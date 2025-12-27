/*
 * Copyright 2024 The Android Open Source Project
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

#ifndef ANDROID_DEVICE_CAMERA_BUFFER_ADAPTER_H
#define ANDROID_DEVICE_CAMERA_BUFFER_ADAPTER_H

#include <mutex>
#include <drm/drm_fourcc.h>
#include <hardware/camera3.h>
#include <hardware/gralloc.h>

#include <unordered_map>
#include <vector>

#include "camera_buffer_handle.h"
// A 10-bit private bayer format used by MediaTek ISP. The define here should be
// kept in sync with cs/chromeos_public/src/platform/minigbm/drv.h
#define DRM_FORMAT_MTISP_SXYZW10 fourcc_code('M', 'B', '1', '0')

namespace android {

class Camera3DeviceOpsDelegate;

struct AdaptedHandle {
    camera_buffer_handle_t camera_buffer_handle;
    buffer_handle_t buffer_handle;
};

class BufferAdapter {
public:
    bool init();

    // Creates a Camera3StreamBufferPtr out of |buffer|.  The buffer will be assigned a positive
    // buffer id which can be used in IPC calls to identify |buffer|.
    // Returns 0 on success; otherwise returns the error code returned by HAL adapter.
    camera3_stream_buffer_t prepareStreamBufferPtr(
            const camera3_stream_buffer_t* buffer);

    // buffers the function converts the received handle ID to the original buffer handles which
    // were passed down when the frameworks called CameraDevice::processCaptureRequest.
    int decodeStreamBufferPtr(const camera3_stream_buffer_t* ptr,
                              camera3_stream_buffer_t* outBuffer);

private:
    // Resolves the multi-planar |androidFormat| using ways that do not depend on the binary format
    // of |buffer|.
    // Returns the resolved DRM format.
    uint32_t resolveFormat(buffer_handle_t buffer, int androidFormat, uint32_t usage,
                           uint32_t* numPlanes, std::vector<uint32_t>* strides,
                           std::vector<uint32_t>* offsets);

    // Gets the private format of the current board.
    // Returns the private DRM format on success, 0 when none is found.
    uint32_t getPrivateFormat();

    // Set of currently active buffer handles.  Used to sanitize and convert IPC calls.  We need to
    // store the pointer to buffer_handle_t in processCaptureRequest and restore it in
    // processCaptureResult because the frameworks requires the same pointer to buffer_handle_t to
    // calculate the address of the ANativeWindowBuffer_t that contains the handle.
    std::unordered_map<uint64_t, buffer_handle_t*> mBufferHandles;
    std::unordered_map<uint64_t, AdaptedHandle> mAdaptedBufferHandles;

    // Lock to guard mBufferHandles.
    std::mutex mBufferHandlesLock;

    const gralloc_module_t* mGrallocModule;
};

}  // end of namespace android

#endif  // ANDROID_DEVICE_CAMERA_BUFFER_ADAPTER_H
