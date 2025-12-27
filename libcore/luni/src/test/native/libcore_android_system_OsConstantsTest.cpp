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

#define LOG_TAG "OsConstantsTest"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include <unistd.h>

#include <linux/if_addr.h>
#include <linux/rtnetlink.h>

#include <nativehelper/JNIHelp.h>
#include <nativehelper/jni_macros.h>

#include "android-base/logging.h"

#if defined(__GLIBC__)
// MADV_SOFT_OFFLINE is otherwise unavailable from glibc.
#include <asm-generic/mman-common.h>
#endif

// Taken from Portability.h
#if __has_include(<linux/vm_sockets.h>)
#include <linux/vm_sockets.h>
#else  // __has_include(<linux/vm_sockets.h>)
// the platform does not support virtio-vsock
#define AF_VSOCK (-1)
#define VMADDR_PORT_ANY (-1)
#define VMADDR_CID_ANY (-1)
#define VMADDR_CID_LOCAL (-1)
#define VMADDR_CID_HOST (-1)
#endif  // __has_include(<linux/vm_sockets.h>)


// Constants whose value does not depend on architecture and is set in Java code.
#define JAVA_INITIALIZED_FIELDS(V) \
  V(AF_INET) \
  V(AF_INET6) \
  V(AF_NETLINK) \
  V(AF_PACKET) \
  V(AF_UNIX) \
  V(AF_VSOCK) \
  V(AF_UNSPEC) \
  V(ARPHRD_ETHER) \
  V(ARPHRD_LOOPBACK) \
  V(VMADDR_PORT_ANY) \
  V(VMADDR_CID_ANY) \
  V(VMADDR_CID_LOCAL) \
  V(VMADDR_CID_HOST) \
  V(CAP_AUDIT_CONTROL) \
  V(CAP_AUDIT_WRITE) \
  V(CAP_BLOCK_SUSPEND) \
  V(CAP_CHOWN) \
  V(CAP_DAC_OVERRIDE) \
  V(CAP_DAC_READ_SEARCH) \
  V(CAP_FOWNER) \
  V(CAP_FSETID) \
  V(CAP_IPC_LOCK) \
  V(CAP_IPC_OWNER) \
  V(CAP_KILL) \
  V(CAP_LAST_CAP) \
  V(CAP_LEASE) \
  V(CAP_LINUX_IMMUTABLE) \
  V(CAP_MAC_ADMIN) \
  V(CAP_MAC_OVERRIDE) \
  V(CAP_MKNOD) \
  V(CAP_NET_ADMIN) \
  V(CAP_NET_BIND_SERVICE) \
  V(CAP_NET_BROADCAST) \
  V(CAP_NET_RAW) \
  V(CAP_SETFCAP) \
  V(CAP_SETGID) \
  V(CAP_SETPCAP) \
  V(CAP_SETUID) \
  V(CAP_SYS_ADMIN) \
  V(CAP_SYS_BOOT) \
  V(CAP_SYS_CHROOT) \
  V(CAP_SYSLOG) \
  V(CAP_SYS_MODULE) \
  V(CAP_SYS_NICE) \
  V(CAP_SYS_PACCT) \
  V(CAP_SYS_PTRACE) \
  V(CAP_SYS_RAWIO) \
  V(CAP_SYS_RESOURCE) \
  V(CAP_SYS_TIME) \
  V(CAP_SYS_TTY_CONFIG) \
  V(CAP_WAKE_ALARM) \
  V(_LINUX_CAPABILITY_VERSION_3) \
  V(STDIN_FILENO) \
  V(STDOUT_FILENO) \
  V(STDERR_FILENO) \
  V(E2BIG) \
  V(EACCES) \
  V(EADDRINUSE) \
  V(EADDRNOTAVAIL) \
  V(EAFNOSUPPORT) \
  V(EAGAIN) \
  V(EALREADY) \
  V(EBADF) \
  V(EBADMSG) \
  V(EBUSY) \
  V(ECANCELED) \
  V(ECHILD) \
  V(ECONNABORTED) \
  V(ECONNREFUSED) \
  V(ECONNRESET) \
  V(EDEADLK) \
  V(EDESTADDRREQ) \
  V(EDOM) \
  V(EDQUOT) \
  V(EEXIST) \
  V(EFAULT) \
  V(EFBIG) \
  V(EHOSTUNREACH) \
  V(EIDRM) \
  V(EILSEQ) \
  V(EINPROGRESS) \
  V(EINTR) \
  V(EINVAL) \
  V(EIO) \
  V(EISCONN) \
  V(EISDIR) \
  V(ELOOP) \
  V(EMFILE) \
  V(EMLINK) \
  V(EMSGSIZE) \
  V(EMULTIHOP) \
  V(ENAMETOOLONG) \
  V(ENETDOWN) \
  V(ENETRESET) \
  V(ENETUNREACH) \
  V(ENFILE) \
  V(ENOBUFS) \
  V(ENODATA) \
  V(ENODEV) \
  V(ENOENT) \
  V(ENOEXEC) \
  V(ENOLCK) \
  V(ENOLINK) \
  V(ENOMEM) \
  V(ENOMSG) \
  V(ENONET) \
  V(ENOPROTOOPT) \
  V(ENOSPC) \
  V(ENOSR) \
  V(ENOSTR) \
  V(ENOSYS) \
  V(ENOTCONN) \
  V(ENOTDIR) \
  V(ENOTEMPTY) \
  V(ENOTSOCK) \
  V(ENOTSUP) \
  V(ENOTTY) \
  V(ENXIO) \
  V(EOPNOTSUPP) \
  V(EOVERFLOW) \
  V(EPERM) \
  V(EPIPE) \
  V(EPROTO) \
  V(EPROTONOSUPPORT) \
  V(EPROTOTYPE) \
  V(ERANGE) \
  V(EROFS) \
  V(ESPIPE) \
  V(ESRCH) \
  V(ESTALE) \
  V(ETIME) \
  V(ETIMEDOUT) \
  V(ETXTBSY) \
  V(EUSERS) \
  V(EXDEV) \
  V(EXIT_FAILURE) \
  V(EXIT_SUCCESS) \
  V(ETH_P_ALL) \
  V(ETH_P_ARP) \
  V(ETH_P_IP) \
  V(ETH_P_IPV6) \
  V(FD_CLOEXEC) \
  V(FIONREAD) \
  V(F_DUPFD) \
  V(F_DUPFD_CLOEXEC) \
  V(F_GETFD) \
  V(F_GETFL) \
  V(F_GETOWN) \
  V(F_OK) \
  V(R_OK) \
  V(F_RDLCK) \
  V(F_SETFD) \
  V(F_SETFL) \
  V(F_SETOWN) \
  V(F_UNLCK) \
  V(F_WRLCK) \
  V(ICMP_ECHO) \
  V(ICMP_ECHOREPLY) \
  V(ICMP6_ECHO_REQUEST) \
  V(ICMP6_ECHO_REPLY) \
  V(IFA_F_DADFAILED) \
  V(IFA_F_DEPRECATED) \
  V(IFA_F_HOMEADDRESS) \
  V(IFA_F_MANAGETEMPADDR) \
  V(IFA_F_NODAD) \
  V(IFA_F_NOPREFIXROUTE) \
  V(IFA_F_OPTIMISTIC) \
  V(IFA_F_PERMANENT) \
  V(IFA_F_SECONDARY) \
  V(IFA_F_TEMPORARY) \
  V(IFA_F_TENTATIVE) \
  V(IFF_ALLMULTI) \
  V(IFF_AUTOMEDIA) \
  V(IFF_BROADCAST) \
  V(IFF_DEBUG) \
  V(IFF_DYNAMIC) \
  V(IFF_LOOPBACK) \
  V(IFF_MASTER) \
  V(IFF_MULTICAST) \
  V(IFF_NOARP) \
  V(IFF_NOTRAILERS) \
  V(IFF_POINTOPOINT) \
  V(IFF_PORTSEL) \
  V(IFF_PROMISC) \
  V(IFF_RUNNING) \
  V(IFF_SLAVE) \
  V(IFF_UP) \
  V(IPPROTO_ICMP) \
  V(IPPROTO_ICMPV6) \
  V(IPPROTO_IP) \
  V(IPPROTO_IPV6) \
  V(IPPROTO_RAW) \
  V(IPPROTO_TCP) \
  V(IPPROTO_UDP) \
  V(IPPROTO_ESP) \
  V(IPV6_CHECKSUM) \
  V(IPV6_MULTICAST_HOPS) \
  V(IPV6_MULTICAST_IF) \
  V(IPV6_MULTICAST_LOOP) \
  V(IPV6_PKTINFO) \
  V(IPV6_RECVDSTOPTS) \
  V(IPV6_RECVHOPLIMIT) \
  V(IPV6_RECVHOPOPTS) \
  V(IPV6_RECVPKTINFO) \
  V(IPV6_RECVRTHDR) \
  V(IPV6_RECVTCLASS) \
  V(IPV6_TCLASS) \
  V(IPV6_UNICAST_HOPS) \
  V(IPV6_V6ONLY) \
  V(IP_MULTICAST_ALL) \
  V(IP_MULTICAST_IF) \
  V(IP_MULTICAST_LOOP) \
  V(IP_MULTICAST_TTL) \
  V(IP_RECVTOS) \
  V(IP_TOS) \
  V(IP_TTL) \
  V(MADV_NORMAL) \
  V(MADV_RANDOM) \
  V(MADV_SEQUENTIAL) \
  V(MADV_WILLNEED) \
  V(MADV_DONTNEED) \
  V(MADV_REMOVE) \
  V(MADV_DONTFORK) \
  V(MADV_DOFORK) \
  V(MADV_HWPOISON) \
  V(MADV_MERGEABLE) \
  V(MADV_UNMERGEABLE) \
  V(MADV_SOFT_OFFLINE) \
  V(MADV_HUGEPAGE) \
  V(MADV_NOHUGEPAGE) \
  V(MADV_COLLAPSE) \
  V(MADV_DONTDUMP) \
  V(MADV_DODUMP) \
  V(MADV_FREE) \
  V(MADV_WIPEONFORK) \
  V(MADV_KEEPONFORK) \
  V(MADV_COLD) \
  V(MADV_PAGEOUT) \
  V(MADV_POPULATE_READ) \
  V(MADV_POPULATE_WRITE) \
  V(MAP_FIXED) \
  V(MAP_ANONYMOUS) \
  V(MAP_POPULATE) \
  V(MAP_PRIVATE) \
  V(MAP_SHARED) \
  V(MCAST_JOIN_GROUP) \
  V(MCAST_LEAVE_GROUP) \
  V(MCAST_JOIN_SOURCE_GROUP) \
  V(MCAST_LEAVE_SOURCE_GROUP) \
  V(MCAST_BLOCK_SOURCE) \
  V(MCAST_UNBLOCK_SOURCE) \
  V(MCL_CURRENT) \
  V(MCL_FUTURE) \
  V(MFD_CLOEXEC) \
  V(MSG_CTRUNC) \
  V(MSG_DONTROUTE) \
  V(MSG_EOR) \
  V(MSG_OOB) \
  V(MSG_PEEK) \
  V(MSG_TRUNC) \
  V(MSG_WAITALL) \
  V(MS_ASYNC) \
  V(MS_INVALIDATE) \
  V(MS_SYNC) \
  V(NETLINK_NETFILTER) \
  V(NETLINK_ROUTE) \
  V(NETLINK_INET_DIAG) \
  V(NETLINK_XFRM) \
  V(NI_DGRAM) \
  V(O_ACCMODE) \
  V(O_APPEND) \
  V(O_CLOEXEC) \
  V(O_CREAT) \
  V(O_EXCL) \
  V(O_NOCTTY) \
  V(O_NONBLOCK) \
  V(O_RDONLY) \
  V(O_RDWR) \
  V(O_SYNC) \
  V(O_DSYNC) \
  V(O_TRUNC) \
  V(O_WRONLY) \
  V(POLLERR) \
  V(POLLHUP) \
  V(POLLIN) \
  V(POLLNVAL) \
  V(POLLOUT) \
  V(POLLPRI) \
  V(POLLRDBAND) \
  V(POLLRDNORM) \
  V(POLLWRBAND) \
  V(POLLWRNORM) \
  V(PR_CAP_AMBIENT) \
  V(PR_CAP_AMBIENT_RAISE) \
  V(PR_GET_DUMPABLE) \
  V(PR_SET_DUMPABLE) \
  V(PR_SET_NO_NEW_PRIVS) \
  V(PROT_EXEC) \
  V(PROT_NONE) \
  V(PROT_READ) \
  V(PROT_WRITE) \
  V(RLIMIT_NOFILE) \
  V(RLIMIT_RTPRIO) \
  V(RT_SCOPE_HOST) \
  V(RT_SCOPE_LINK) \
  V(RT_SCOPE_NOWHERE) \
  V(RT_SCOPE_SITE) \
  V(RT_SCOPE_UNIVERSE) \
  V(RTMGRP_IPV4_IFADDR) \
  V(RTMGRP_IPV4_MROUTE) \
  V(RTMGRP_IPV4_ROUTE) \
  V(RTMGRP_IPV4_RULE) \
  V(RTMGRP_IPV6_IFADDR) \
  V(RTMGRP_IPV6_IFINFO) \
  V(RTMGRP_IPV6_MROUTE) \
  V(RTMGRP_IPV6_PREFIX) \
  V(RTMGRP_IPV6_ROUTE) \
  V(RTMGRP_LINK) \
  V(RTMGRP_NEIGH) \
  V(RTMGRP_NOTIFY) \
  V(RTMGRP_TC) \
  V(SEEK_CUR) \
  V(SEEK_END) \
  V(SEEK_SET) \
  V(SHUT_RD) \
  V(SHUT_RDWR) \
  V(SHUT_WR) \
  V(SIGABRT) \
  V(SIGALRM) \
  V(SIGBUS) \
  V(SIGCHLD) \
  V(SIGCONT) \
  V(SIGFPE) \
  V(SIGHUP) \
  V(SIGILL) \
  V(SIGINT) \
  V(SIGIO) \
  V(SIGKILL) \
  V(SIGPIPE) \
  V(SIGPROF) \
  V(SIGPWR) \
  V(SIGQUIT) \
  V(SIGRTMAX) \
  V(SIGSEGV) \
  V(SIGSTKFLT) \
  V(SIGSTOP) \
  V(SIGSYS) \
  V(SIGTERM) \
  V(SIGTRAP) \
  V(SIGTSTP) \
  V(SIGTTIN) \
  V(SIGTTOU) \
  V(SIGURG) \
  V(SIGUSR1) \
  V(SIGUSR2) \
  V(SIGVTALRM) \
  V(SIGWINCH) \
  V(SIGXCPU) \
  V(SIGXFSZ) \
  V(SIOCGIFADDR) \
  V(SIOCGIFBRDADDR) \
  V(SIOCGIFDSTADDR) \
  V(SIOCGIFNETMASK) \
  V(SOCK_CLOEXEC) \
  V(SOCK_DGRAM) \
  V(SOCK_NONBLOCK) \
  V(SOCK_RAW) \
  V(SOCK_SEQPACKET) \
  V(SOCK_STREAM) \
  V(SOL_SOCKET) \
  V(SOL_UDP) \
  V(SOL_PACKET) \
  V(SO_BINDTODEVICE) \
  V(SO_BROADCAST) \
  V(SO_DEBUG) \
  V(SO_DOMAIN) \
  V(SO_DONTROUTE) \
  V(SO_ERROR) \
  V(SO_KEEPALIVE) \
  V(SO_LINGER) \
  V(SO_OOBINLINE) \
  V(SO_PASSCRED) \
  V(SO_PEERCRED) \
  V(SO_PROTOCOL) \
  V(SO_RCVBUF) \
  V(SO_RCVLOWAT) \
  V(SO_RCVTIMEO) \
  V(SO_REUSEADDR) \
  V(SO_SNDBUF) \
  V(SO_SNDLOWAT) \
  V(SO_SNDTIMEO) \
  V(SO_TYPE) \
  V(PACKET_IGNORE_OUTGOING) \
  V(SPLICE_F_MOVE) \
  V(SPLICE_F_NONBLOCK) \
  V(SPLICE_F_MORE) \
  V(ST_MANDLOCK) \
  V(ST_NOATIME) \
  V(ST_NODEV) \
  V(ST_NODIRATIME) \
  V(ST_NOEXEC) \
  V(ST_NOSUID) \
  V(ST_RDONLY) \
  V(ST_RELATIME) \
  V(ST_SYNCHRONOUS) \
  V(S_IFBLK) \
  V(S_IFCHR) \
  V(S_IFDIR) \
  V(S_IFIFO) \
  V(S_IFLNK) \
  V(S_IFMT) \
  V(S_IFREG) \
  V(S_IFSOCK) \
  V(S_IRGRP) \
  V(S_IROTH) \
  V(S_IRUSR) \
  V(S_IRWXG) \
  V(S_IRWXO) \
  V(S_IRWXU) \
  V(S_ISGID) \
  V(S_ISUID) \
  V(S_ISVTX) \
  V(S_IWGRP) \
  V(S_IWOTH) \
  V(S_IWUSR) \
  V(S_IXGRP) \
  V(S_IXOTH) \
  V(S_IXUSR) \
  V(TCP_NODELAY) \
  V(TCP_USER_TIMEOUT) \
  V(UDP_GRO) \
  V(UDP_SEGMENT) \
  V(TIOCOUTQ) \
  V(UDP_ENCAP) \
  V(UDP_ENCAP_ESPINUDP_NON_IKE) \
  V(UDP_ENCAP_ESPINUDP) \
  V(UNIX_PATH_MAX) \
  V(WCONTINUED) \
  V(WEXITED) \
  V(WNOHANG) \
  V(WNOWAIT) \
  V(WSTOPPED) \
  V(WUNTRACED) \
  V(W_OK) \
  V(XATTR_CREATE) \
  V(XATTR_REPLACE) \
  V(X_OK) \

// glibc's sys/un.h does not have UNIX_MAX_PATH and just hardcodes 108 in the sockaddr_un struct.
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX sizeof(sockaddr_un::sun_path)
#endif

static void validateConstant(JNIEnv* env, jclass c, const char* fieldName, int value) {
    jfieldID field = env->GetStaticFieldID(c, fieldName, "I");
    int actual = env->GetStaticIntField(c, field);
    if (actual != value) {
        LOG(FATAL) << "Value of " << fieldName << " is wrong. "
                   << "Expected " << value << ", got " << actual;
    }
}

static void validateConstants(JNIEnv* env, jclass c) {
#define VALIDATE_CONSTANTS(Name) \
    validateConstant(env, c, # Name, Name);
    JAVA_INITIALIZED_FIELDS(VALIDATE_CONSTANTS)
#undef VALIDATE_CONSTANTS
}

extern "C"
JNIEXPORT void JNICALL Java_libcore_android_system_OsConstantsTest_checkConsistency(JNIEnv* env,
                                                                                    jclass) {
    jclass os_constants_class = env->FindClass("android/system/OsConstants");
    if (os_constants_class == nullptr) {
        LOG(FATAL) << "Could not find OsConstants";
    }
    validateConstants(env, os_constants_class);
}

#define ONE_PLUS(Name) 1 +
static constexpr size_t kJavaInitializedFieldsCount = JAVA_INITIALIZED_FIELDS(ONE_PLUS) 0;
#undef ONE_PLUS

extern "C"
JNIEXPORT jint JNICALL Java_libcore_android_system_OsConstantsTest_initializedInJavaCount(
        JNIEnv*,
        jclass) {
    return kJavaInitializedFieldsCount;
}
