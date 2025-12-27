/*
 * Copyright (C) 2022 MediaTek Inc.
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
#ifndef IPC_WRAPPER_COMMON_INC_IPCCOMMON_H_
#define IPC_WRAPPER_COMMON_INC_IPCCOMMON_H_

// TODO: Commented out by Google.
// #include <IPCLog.h>
#include <stdint.h>
#include <stdio.h>
#include <memory>
#include <string>
#include <utility>

namespace NSCam {
namespace ipc {
namespace wrapper {

#define MAX_SENSOR_SUPPORTED (2)
#define MAX_STRING_LENGTH (255)

using IPC_SHM_TYPE = uint32_t;

struct ShmMemInfo {
  std::string mName;
  IPC_SHM_TYPE mType;
  uint32_t mSize;
  int32_t mFd;
  void* mAddr;
  int32_t mHandle;
  ShmMemInfo()
      : mName(""), mType(0), mSize(0), mFd(-1), mAddr(nullptr), mHandle(-1) {}
  ShmMemInfo(std::string& inName,
             IPC_SHM_TYPE inType,
             int inSize,
             int32_t inFd,
             void* inAddr,
             int32_t inHandle = -1)
      : mName(inName),
        mType(inType),
        mSize(inSize),
        mFd(inFd),
        mAddr(inAddr),
        mHandle(inHandle) {}
};

struct ShmMemAlloc {
  std::string name;
  IPC_SHM_TYPE shmMemType;
  int size;
};

struct ShmMemDeAlloc {
  IPC_SHM_TYPE shmMemType;
  int fd;

  ShmMemDeAlloc(IPC_SHM_TYPE type, int fileDesc)
      : shmMemType(type), fd(fileDesc) {}
};

enum IPC_PLATFORM_TYPE {
  IPC_PLATFORM_TYPE_MTK = 0,
  IPC_PLATFORM_TYPE_CHROMIUM,
};

enum IPC_USER {
  IPC_USER_UNKNOWN = 0,
  IPC_USER_DEFAULT,
  IPC_USER_HAL3A,
  IPC_USER_HALISP,
  IPC_USER_NVBUF,
  IPC_USER_FD,
  IPC_USER_FWMVP,
  IPC_USER_TDP,

  IPC_USER_MAX = (1 << 16),
};

/*
 * IPC Command Format
 +-----------------------------------------+
 |       SYS CMD      |       USER CMD     |
 |       16 bits      |        16 bits     |
 +-----------------------------------------+
 */

#define GEN_IPC_CMD(usr_cmd, sys_cmd, ipc_cmd)                             \
  do {                                                                     \
    ipc_cmd = ((((uint32_t)sys_cmd & 0xFFFF) << 16) + (usr_cmd & 0xFFFF)); \
  } while (0)

#define GET_USER_CMD(ipc_cmd, usr_cmd) \
  do {                                 \
    usr_cmd = (ipc_cmd & 0xFFFF);      \
  } while (0)

#define GET_SYS_CMD(ipc_cmd, sys_cmd)                  \
  do {                                                 \
    sys_cmd = (IPC_CMD)((ipc_cmd & 0xFFFF0000) >> 16); \
  } while (0)

enum IPC_CMD {
  IPC_CMD_NONE = 0,
  IPC_CMD_GET_USER = 1,
  IPC_CMD_GET_SHM_TYPE = 2,
  IPC_CMD_GET_GROUP = 3,
  IPC_CMD_CREATE_INSTANCE = 4,
  IPC_CMD_DESTROY_INSTANCE = 5,
};

template <typename T, typename Args>
std::unique_ptr<T> make_unique(Args&& args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)));
}

struct ShmMemBase {
  uint32_t instanceIdx;
  int32_t sensorIdx;
  int32_t sensorDev;
  uint64_t userId;

  ShmMemBase() : instanceIdx(0), sensorIdx(-1), sensorDev(0), userId(0) {}
};

struct ShmMemCreateInstance : public ShmMemBase {
  ShmMemCreateInstance() {}
};

struct ShmMemDestroyInstance : public ShmMemBase {
  ShmMemDestroyInstance() {}
};

struct ShmMeGetUser : public ShmMemBase {
  ShmMeGetUser() {}
};

}  // namespace wrapper
}  // namespace ipc
}  // namespace NSCam

#endif  // IPC_WRAPPER_COMMON_INC_IPCCOMMON_H_
