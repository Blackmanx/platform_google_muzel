/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utils.h"

#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "android-base/file.h"
#include "android-base/stringprintf.h"
#include "android-base/strings.h"
#include "base/mem_map.h"
#include "base/stl_util.h"
#include "bit_utils.h"
#include "os.h"

#if defined(__APPLE__)
#include <crt_externs.h>
// NOLINTNEXTLINE - inclusion of syscall is dependent on arch
#include <sys/syscall.h>
#include "AvailabilityMacros.h"  // For MAC_OS_X_VERSION_MAX_ALLOWED
#endif

#if defined(__linux__)
// NOLINTNEXTLINE - inclusion of syscall is dependent on arch
#include <sys/syscall.h>
#endif

#if defined(_WIN32)
#include <windows.h>
// This include needs to be here due to our coding conventions.  Unfortunately
// it drags in the definition of the dread ERROR macro.
#ifdef ERROR
#undef ERROR
#endif
#endif

namespace art {

using android::base::ReadFileToString;  // NOLINT - ReadFileToString is actually used
using android::base::StringPrintf;

#if defined(__arm__)

namespace {

// Bitmap of caches to flush for cacheflush(2). Must be zero for ARM.
static constexpr int kCacheFlushFlags = 0x0;

// Number of retry attempts when flushing cache ranges.
static constexpr size_t kMaxFlushAttempts = 4;

int CacheFlush(uintptr_t start, uintptr_t limit) {
  // The signature of cacheflush(2) seems to vary by source. On ARM the system call wrapper
  //    (bionic/SYSCALLS.TXT) has the form: int cacheflush(long start, long end, long flags);
  int r = cacheflush(start, limit, kCacheFlushFlags);
  if (r == -1) {
    CHECK_NE(errno, EINVAL);
  }
  return r;
}

bool TouchAndFlushCacheLinesWithinPage(uintptr_t start, uintptr_t limit, size_t attempts,
                                       size_t page_size) {
  CHECK_LT(start, limit);
  CHECK_EQ(RoundDown(start, page_size), RoundDown(limit - 1, page_size)) << "range spans pages";
  // Declare a volatile variable so the compiler does not elide reads from the page being touched.
  [[maybe_unused]] volatile uint8_t v = 0;
  for (size_t i = 0; i < attempts; ++i) {
    // Touch page to maximize chance page is resident.
    v = *reinterpret_cast<uint8_t*>(start);

    if (LIKELY(CacheFlush(start, limit) == 0)) {
      return true;
    }
  }
  return false;
}

}  // namespace

bool FlushCpuCaches(void* begin, void* end) {
  // This method is specialized for ARM as the generic implementation below uses the
  // __builtin___clear_cache() intrinsic which is declared as void. On ARMv7 flushing the CPU
  // caches is a privileged operation. The Linux kernel allows these operations to fail when they
  // trigger a fault (e.g. page not resident). We use a wrapper for the ARM specific cacheflush()
  // system call to detect the failure and potential erroneous state of the data and instruction
  // caches.
  //
  // The Android bug for this is b/132205399 and there's a similar discussion on
  // https://reviews.llvm.org/D37788. This is primarily an issue for the dual view JIT where the
  // pages where code is executed are only ever RX and never RWX. When attempting to invalidate
  // instruction cache lines in the RX mapping after writing fresh code in the RW mapping, the
  // page may not be resident (due to memory pressure), and this means that a fault is raised in
  // the midst of a cacheflush() call and the instruction cache lines are not invalidated and so
  // have stale code.
  //
  // Other architectures fair better for reasons such as:
  //
  // (1) stronger coherence between the data and instruction caches.
  //
  // (2) fault handling that allows flushing/invalidation to continue after
  //     a missing page has been faulted in.

  const size_t page_size = MemMap::GetPageSize();

  uintptr_t start = reinterpret_cast<uintptr_t>(begin);
  const uintptr_t limit = reinterpret_cast<uintptr_t>(end);
  if (LIKELY(CacheFlush(start, limit) == 0)) {
    return true;
  }

  // A rare failure has occurred implying that part of the range (begin, end] has been swapped
  // out. Retry flushing but this time grouping cache-line flushes on individual pages and
  // touching each page before flushing.
  uintptr_t next_page = RoundUp(start + 1, page_size);
  while (start < limit) {
    uintptr_t boundary = std::min(next_page, limit);
    if (!TouchAndFlushCacheLinesWithinPage(start, boundary, kMaxFlushAttempts, page_size)) {
      return false;
    }
    start = boundary;
    next_page += page_size;
  }
  return true;
}

#else

bool FlushCpuCaches(void* begin, void* end) {
  __builtin___clear_cache(reinterpret_cast<char*>(begin), reinterpret_cast<char*>(end));
  return true;
}

#endif

#if defined(__linux__)
bool IsKernelVersionAtLeast(int reqd_major, int reqd_minor) {
  static auto version = []() -> std::pair<int, int> {
    struct utsname uts;
    int res, major, minor;
    res = uname(&uts);
    CHECK_EQ(res, 0);
    CHECK_EQ(strcmp(uts.sysname, "Linux"), 0);
    res = sscanf(uts.release, "%d.%d:", &major, &minor);
    CHECK_EQ(res, 2);
    return std::make_pair(major, minor);
  }();
  return version >= std::make_pair(reqd_major, reqd_minor);
}
#endif

bool CacheOperationsMaySegFault() {
#if defined(__linux__) && defined(__aarch64__)
  // Avoid issue on older ARM64 kernels where data cache operations could be classified as writes
  // and cause segmentation faults. This was fixed in Linux 3.11rc2:
  //
  // https://github.com/torvalds/linux/commit/db6f41063cbdb58b14846e600e6bc3f4e4c2e888
  //
  // This behaviour means we should avoid the dual view JIT on the device. This is just
  // an issue when running tests on devices that have an old kernel.
  return !IsKernelVersionAtLeast(3, 12);
#else
  return false;
#endif
}

bool RunningOnVM() {
  const char* on_vm = getenv("ART_TEST_ON_VM");
  return on_vm != nullptr && std::strcmp("true", on_vm) == 0;
}

bool RunningOnSBC() {
  const char* on_sbc = getenv("ART_TEST_ON_SBC");
  return on_sbc != nullptr && std::strcmp("true", on_sbc) == 0;
}

uint32_t GetTid() {
#if defined(__APPLE__)
  uint64_t owner;
  CHECK_PTHREAD_CALL(pthread_threadid_np, (nullptr, &owner), __FUNCTION__);  // Requires Mac OS 10.6
  return owner;
#elif defined(__BIONIC__)
  return gettid();
#elif defined(_WIN32)
  return static_cast<pid_t>(::GetCurrentThreadId());
#else
  return syscall(__NR_gettid);
#endif
}

std::string GetThreadName(pid_t tid) {
  std::string result;
#ifdef _WIN32
  UNUSED(tid);
  result = "<unknown>";
#else
  // TODO: make this less Linux-specific.
  if (ReadFileToString(StringPrintf("/proc/self/task/%d/comm", tid), &result)) {
    result.resize(result.size() - 1);  // Lose the trailing '\n'.
  } else {
    result = "<unknown>";
  }
#endif
  return result;
}

std::string PrettySize(uint64_t byte_count) {
  // The byte thresholds at which we display amounts.  A byte count is displayed
  // in unit U when kUnitThresholds[U] <= bytes < kUnitThresholds[U+1].
  static const uint64_t kUnitThresholds[] = {
    0,        // B up to...
    10*KB,    // KB up to...
    10*MB,    // MB up to...
    10ULL*GB  // GB from here.
  };
  static const uint64_t kBytesPerUnit[] = { 1, KB, MB, GB };
  static const char* const kUnitStrings[] = { "B", "KB", "MB", "GB" };
  int i = arraysize(kUnitThresholds);
  while (--i > 0) {
    if (byte_count >= kUnitThresholds[i]) {
      break;
    }
  }
  return StringPrintf("%" PRIu64 "%s",
                      byte_count / kBytesPerUnit[i], kUnitStrings[i]);
}

template <typename StrIn, typename Str>
void Split(const StrIn& s, char separator, std::vector<Str>* out_result) {
  auto split = SplitString(std::string_view(s), separator);
  for (std::string_view p : split) {
    if (p.empty()) {
      continue;
    }
    out_result->push_back(Str(p));
  }
}

template void Split(const char *const& s, char separator, std::vector<std::string>* out_result);
template void Split(const std::string& s, char separator, std::vector<std::string>* out_result);
template void Split(const char *const& s, char separator, std::vector<std::string_view>* out_result);
template void Split(const std::string_view& s,
                    char separator,
                    std::vector<std::string_view>* out_result);
template void Split(const std::string_view& s,
                    char separator,
                    std::vector<std::string>* out_result);
template void Split(const std::string& s,
                    char separator,
                    std::vector<std::string_view>* out_result);

template <typename Str>
void Split(const Str& s, char separator, size_t len, Str* out_result) {
  Str* last = out_result + len;
  auto split = SplitString(std::string_view(s), separator);
  for (std::string_view p : split) {
    if (p.empty()) {
      continue;
    }
    if (out_result == last) {
      return;
    }
    *out_result++ = Str(p);
  }
}

template void Split(const std::string& s, char separator, size_t len, std::string* out_result);
template void Split(const std::string_view& s,
                    char separator,
                    size_t len,
                    std::string_view* out_result);

void SetThreadName(pthread_t thr, const char* thread_name) {
  bool hasAt = false;
  bool hasDot = false;
  const char* s = thread_name;
  while (*s) {
    if (*s == '.') {
      hasDot = true;
    } else if (*s == '@') {
      hasAt = true;
    }
    s++;
  }
  int len = s - thread_name;
  if (len < 15 || hasAt || !hasDot) {
    s = thread_name;
  } else {
    s = thread_name + len - 15;
  }
#if defined(__linux__) || defined(_WIN32)
  // pthread_setname_np fails rather than truncating long strings.
  char buf[16];       // MAX_TASK_COMM_LEN=16 is hard-coded in the kernel.
  strncpy(buf, s, sizeof(buf)-1);
  buf[sizeof(buf)-1] = '\0';
  errno = pthread_setname_np(thr, buf);
  if (errno != 0) {
    PLOG(WARNING) << "Unable to set the name of current thread to '" << buf << "'";
  }
#else  // __APPLE__
  if (pthread_equal(thr, pthread_self())) {
    pthread_setname_np(thread_name);
  } else {
    PLOG(WARNING) << "Unable to set the name of another thread to '" << thread_name << "'";
  }
#endif
}

void SetThreadName(const char* thread_name) { SetThreadName(pthread_self(), thread_name); }

void GetTaskStats(pid_t tid, char* state, int* utime, int* stime, int* task_cpu) {
  *utime = *stime = *task_cpu = 0;
#ifdef _WIN32
  // TODO: implement this.
  UNUSED(tid);
  *state = 'S';
#else
  std::string stats;
  // TODO: make this less Linux-specific.
  if (!ReadFileToString(StringPrintf("/proc/self/task/%d/stat", tid), &stats)) {
    return;
  }
  // Skip the command, which may contain spaces.
  stats = stats.substr(stats.find(')') + 2);
  // Extract the three fields we care about.
  std::vector<std::string> fields;
  Split(stats, ' ', &fields);
  *state = fields[0][0];
  *utime = strtoull(fields[11].c_str(), nullptr, 10);
  *stime = strtoull(fields[12].c_str(), nullptr, 10);
  *task_cpu = strtoull(fields[36].c_str(), nullptr, 10);
#endif
}

void SleepForever() {
  while (true) {
    sleep(100000000);
  }
}

std::string GetProcessStatus(const char* key) {
  // Build search pattern of key and separator.
  std::string pattern(key);
  pattern.push_back(':');

  // Search for status lines starting with pattern.
  std::ifstream fs("/proc/self/status");
  std::string line;
  while (std::getline(fs, line)) {
    if (strncmp(pattern.c_str(), line.c_str(), pattern.size()) == 0) {
      // Skip whitespace in matching line (if any).
      size_t pos = line.find_first_not_of(" \t", pattern.size());
      if (UNLIKELY(pos == std::string::npos)) {
        break;
      }
      return std::string(line, pos);
    }
  }
  return "<unknown>";
}

size_t GetOsThreadStat(pid_t tid, char* buf, size_t len) {
#if defined(__linux__)
  static constexpr int NAME_BUF_SIZE = 60;
  char file_name_buf[NAME_BUF_SIZE];
  // We don't use just /proc/<pid>/stat since, in spite of some documentation to the contrary,
  // those report utime and stime values for the whole process, not just the thread.
  snprintf(file_name_buf, NAME_BUF_SIZE, "/proc/%d/task/%d/stat", getpid(), tid);
  android::base::unique_fd stat_fd(open(file_name_buf, O_RDONLY | O_CLOEXEC));
  if (stat_fd.ok()) {
    ssize_t bytes_read = TEMP_FAILURE_RETRY(read(stat_fd.get(), buf, len - 1));
    CHECK_GT(bytes_read, 0) << strerror(errno);
    buf[bytes_read] = '\0';
    return bytes_read;
  }
#else
  UNUSED(tid);
  UNUSED(buf);
  UNUSED(len);
#endif
  return 0;
}

std::string GetOsThreadStatQuick(pid_t tid) {
#if defined(__linux__)
  static constexpr int BUF_SIZE = 100;
  char buf[BUF_SIZE];
  if (GetOsThreadStat(tid, buf, BUF_SIZE) == 0) {
    snprintf(buf, BUF_SIZE, "Unknown state: %d", tid);
  }
  return buf;
#else
  UNUSED(tid);
  return "Unknown state";
#endif
}

char GetStateFromStatString(const std::string& stat_output) {
  size_t rparen_pos = stat_output.find(")");
  if (rparen_pos == std::string::npos || rparen_pos >= stat_output.length() - 3) {
    return '?';
  }
  size_t state_pos = stat_output.find_first_not_of(" ", rparen_pos + 1);
  if (rparen_pos == std::string::npos) {
    return '?';
  }
  return stat_output[state_pos];
}

std::string GetOtherThreadOsStats() {
#if defined(__linux__)
  DIR* dir = opendir("/proc/self/task");
  if (dir == nullptr) {
    return std::string("Failed to open /proc/self/task: ") + strerror(errno);
  }
  pid_t me = GetTid();
  struct dirent* de;
  std::string result;
  bool found_me = false;
  errno = 0;
  while ((de = readdir(dir)) != nullptr) {
    if (de->d_name[0] == '.') {
      continue;
    }
    pid_t tid = atoi(de->d_name);
    if (tid == me) {
      found_me = true;
    } else {
      if (!result.empty()) {
        result += "; ";
      }
      result += tid == 0 ? std::string("bad tid: ") + de->d_name : GetOsThreadStatQuick(tid);
    }
  }
  closedir(dir);
  if (errno == EBADF) {
    result += "(Bad directory)";
  }
  if (!found_me) {
    result += "(Failed to find requestor)";
  }
  return result;
#else
  return "Can't get other threads";
#endif
}

#if defined(__linux__)

// Copy nfields single-blank-separated fields from the line referenced by src.
// Return a pointer to one past the copy on success, nullptr on failure.
static char* memcpy_fields(char* dest, const char* src, const char* src_end, size_t nfields) {
  size_t nblanks = 0;
  while (src < src_end) {
    char c = *src++;
    if (c == '\n') {
      return nullptr;
    }
    if (c == ' ') {
      ++nblanks;
      if (nblanks == nfields) {
        return dest;
      }
    }
    *dest++ = c;
  }
  return nullptr;
}

// Return a pointer to the start of the field_no'th blank-separated field in the line at src.
// field_no = 0 corresponds to the first field. Leading blanks are ignored.
static const char* find_nth(const char* src, const char* src_end, size_t field_no) {
  auto skip_blanks = [&src, src_end]() {
    while (src < src_end && *src == ' ') {
      ++src;
    }
  };
  skip_blanks();
  while (src < src_end) {
    if (*src == '\n') {
      return nullptr;  // Didn't find it.
    }
    if (*src == ' ') {
      DCHECK_NE(field_no, 0ul);
      --field_no;
      skip_blanks();
    }
    if (field_no == 0) {
      return src;
    }
    while (src < src_end && *src != ' ') {
      ++src;
    }
  }
  return nullptr;
}

#endif  // defined(__linux__)
// Otherwise memcpy_fields and find_nth are unused.

// Retrieve the first 3 fields of each of the sum and full lines, and combine them into a string.
// Return an empty string if something goes wrong, e.g. if we don't have permission to read the
// /proc file.
std::string GetOSPressureIOSummary() {
#if defined(__linux__)
  android::base::unique_fd io_fd(open("/proc/pressure/io", O_RDONLY | O_CLOEXEC));
  if (!io_fd.ok()) {
    return "";
  }
  static constexpr size_t kBufSize = 150;
  char tmp_buf[kBufSize + 1];
  // Read the entire file, typically 110 characters.
  ssize_t bytes_read = TEMP_FAILURE_RETRY(read(io_fd.get(), tmp_buf, kBufSize));
  CHECK_GT(bytes_read, 0) << strerror(errno);
  char buf[kBufSize];
  char* out = buf;
  const char* in = tmp_buf;
  DCHECK_EQ(0, strncmp(in, "some ", strlen("some ")));
  out = memcpy_fields(out, in, tmp_buf + kBufSize, 3);
  if (out == nullptr) {
    return "";
  }
  in += out - buf;
  *out++ = ',';
  *out++ = ' ';
  while (*in++ != '\n') {
    if (in >= tmp_buf + kBufSize) {
      return "";
    }
  }
  DCHECK_EQ(0, strncmp(in, "full ", strlen("full ")));
  out = memcpy_fields(out, in, tmp_buf + kBufSize, 3);
  if (out == nullptr) {
    return "";
  }
  *out++ = '\0';
  return std::string(buf);
#else
  return "";
#endif
}

size_t GetOSDiskStats(const char* disk_name, char* buf, size_t len) {
  // This is theoretically easier to get from /sys/block/sda/stat, but the selinux permission
  // issues there look harder.
#if defined(__linux__)
  android::base::unique_fd stats_fd(open("/proc/diskstats", O_RDONLY | O_CLOEXEC));
  if (!stats_fd.ok()) {
    return 0;
  }
  static constexpr size_t kBufSize = 20'000;
  std::unique_ptr<char[]> tmp_buf_ptr(new char[kBufSize]);
  char* tmp_buf = tmp_buf_ptr.get();
  // Read the entire file, typically 10K characters.
  ssize_t bytes_read = TEMP_FAILURE_RETRY(read(stats_fd.get(), tmp_buf, kBufSize));
  if (bytes_read <= 0) {
    return 0;
  }
  const char* line_p = tmp_buf;
  const char* const tmp_buf_end = tmp_buf + bytes_read;
  const size_t disk_name_len = strlen(disk_name);
  while (line_p < tmp_buf_end) {
    static constexpr size_t kNamePos = 2;  // Position of disk name in diskstats line, 0-based.
    const char* name_etc = find_nth(line_p, tmp_buf_end, kNamePos);
    if (name_etc != nullptr && name_etc + disk_name_len < tmp_buf_end &&
        strncmp(name_etc, disk_name, disk_name_len) == 0) {
      size_t out_index = 0;
      for (const char* p = name_etc; p < tmp_buf_end && *p != '\n'; ++p) {
        if (out_index >= len - 1) {
          break;
        }
        buf[out_index++] = *p;
      }
      buf[out_index] = '\0';
      return out_index;
    }
    while (line_p < tmp_buf_end && *line_p != '\n') {
      ++line_p;
    }
    ++line_p;
  }
#else
  UNUSED(buf);
  UNUSED(len);
  UNUSED(disk_name);
#endif
  return 0;
}

ConciseDiskStats::ConciseDiskStats(const char* disk_name)
    : write_millis_(0), io_millis_(0), flush_millis_(0), in_progress_(0) {
#if defined(__linux__)
  if (disk_name == nullptr) {
    return;
  }
  static constexpr size_t kBufSize = 300;
  char tmp_buf[kBufSize];
  int bytes = GetOSDiskStats(disk_name, tmp_buf, kBufSize);
  if (bytes == 0) {
    return;
  }
  // Could do this with sscanf, but that seems more prone to counting errors,
  // and the man page points to a slightly troubling UB issue.

  static constexpr int kWriteMillisOffset = 8;
  static constexpr int kInProgressOffset = 9;
  static constexpr int kIOMillisOffset = 10;
  static constexpr int kFlushMillisOffset = 17;
  const char* p = tmp_buf;
  const char* const buf_end = tmp_buf + bytes;

  // Buf has disk name as zeroth field. Field numbers match iostats.rst.
  p = find_nth(p, buf_end, kWriteMillisOffset);
  if (p == nullptr) {
    return;
  }
  write_millis_ = static_cast<unsigned int>(strtoul(p, nullptr, 10));
  p = find_nth(p, buf_end, kInProgressOffset - kWriteMillisOffset);
  if (p == nullptr) {
    return;
  }
  in_progress_ = static_cast<unsigned int>(strtoul(p, nullptr, 10));
  p = find_nth(p, buf_end, kIOMillisOffset - kInProgressOffset);
  if (p == nullptr) {
    return;
  }
  io_millis_ = static_cast<unsigned int>(strtoul(p, nullptr, 10));
  p = find_nth(p, buf_end, kFlushMillisOffset - kIOMillisOffset);
  if (p == nullptr) {
    return;
  }
  flush_millis_ = static_cast<unsigned int>(strtoul(p, nullptr, 10));
#else
  UNUSED(disk_name);
#endif
}

std::string ConciseDiskStats::SummarizeDiff(ConciseDiskStats earlier) {
#if defined(__linux__)
  if (io_millis_ == 0) {
    return "";
  }
  std::stringstream output;
  output << "ioms: " << (io_millis_ - earlier.io_millis_) << ", ";
  output << "wrms: " << (write_millis_ - earlier.write_millis_) << ", ";
  if (flush_millis_ != 0) {
    output << "flms: " << (flush_millis_ - earlier.flush_millis_) << ", ";
  }
  output << "in progress: " << earlier.in_progress_ << "->" << in_progress_;
  return output.str();
#else
  UNUSED(earlier);
  return "";
#endif
}

}  // namespace art
