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
#define LOG_TAG "BufferAdapter"

#include "BufferAdapter.h"

#include <libcamera/base/log.h>

#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <inttypes.h>
#include <memory>
#include <string>
#include <cutils/properties.h>

using namespace libcamera;

LOG_DECLARE_CATEGORY(HAL)

#define HANDLE_EINTR(x)                                         \
    ({                                                          \
        decltype(x) eintr_wrapper_result;                       \
        do                                                      \
        {                                                       \
            eintr_wrapper_result = (x);                         \
        } while (eintr_wrapper_result == -1 && errno == EINTR); \
        eintr_wrapper_result;                                   \
    })

/* This enumeration must match the one in <gralloc_drm.h>.
 * The functions supported by this gralloc's temporary private API are listed
 * below. Use of these functions is highly discouraged and should only be
 * reserved for cases where no alternative to get same information (such as
 * querying ANativeWindow) exists.
 */
// clang-format off
enum {
        GRALLOC_DRM_GET_STRIDE,
        GRALLOC_DRM_GET_FORMAT,
        GRALLOC_DRM_GET_DIMENSIONS,
        GRALLOC_DRM_GET_BACKING_STORE,
        GRALLOC_DRM_GET_BUFFER_INFO,
        GRALLOC_DRM_GET_USAGE,
};

namespace {

const struct SupportedFormat {
    int halFormat;
    // |drmFormat| is set to 0 for multi-planar formats. The actual format will be resolved in
    // |SupportedPlanarFormat|.
    uint32_t drmFormat;
} kSupportedFormats[] = {
        {HAL_PIXEL_FORMAT_BGRA_8888, DRM_FORMAT_ARGB8888},
        {HAL_PIXEL_FORMAT_RGBA_8888, DRM_FORMAT_ABGR8888},
        {HAL_PIXEL_FORMAT_RGBX_8888, DRM_FORMAT_XBGR8888},
        {HAL_PIXEL_FORMAT_YCbCr_422_I, DRM_FORMAT_YUYV},
        // There is no exact match for HAL_PIXEL_FORMAT_BLOB in DRM format. Use DRM_FORMAT_R8
        // because it is one byte per pixel.
        {HAL_PIXEL_FORMAT_BLOB, DRM_FORMAT_R8},
        {HAL_PIXEL_FORMAT_YV12, 0},
        {HAL_PIXEL_FORMAT_YCbCr_420_888, 0},
        {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, 0},
        // Add more buffer formats when needed.
};

const struct SupportedPlanarFormat {
    int halFormat;
    bool crcb;
    bool semiplanar;
    uint32_t drmFormat;
} kSupportedPlanarFormats[] = {
        {HAL_PIXEL_FORMAT_YV12, true, false, DRM_FORMAT_YVU420},
        {HAL_PIXEL_FORMAT_YCbCr_420_888, false, false, DRM_FORMAT_YUV420},
        {HAL_PIXEL_FORMAT_YCbCr_420_888, true, false, DRM_FORMAT_YVU420},
        {HAL_PIXEL_FORMAT_YCbCr_420_888, false, true, DRM_FORMAT_NV12},
        {HAL_PIXEL_FORMAT_YCbCr_420_888, true, true, DRM_FORMAT_NV21},
        {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, false, false, DRM_FORMAT_YUV420},
        {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, true, false, DRM_FORMAT_YVU420},
        {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, false, true, DRM_FORMAT_NV12},
        {HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED, true, true, DRM_FORMAT_NV21},
        // Add more buffer formats when needed.
};

static inline int getBpp(uint32_t drmFormat) {
    switch (drmFormat) {
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_XBGR8888:
        return 4;
    case DRM_FORMAT_BGR888:
        return 3;
    case DRM_FORMAT_RGB565:
    case DRM_FORMAT_YUYV:
    case DRM_FORMAT_MTISP_SXYZW10:
        return 2;
    case DRM_FORMAT_YVU420:
    case DRM_FORMAT_NV16:
    case DRM_FORMAT_NV21:
    case DRM_FORMAT_NV12:
    case DRM_FORMAT_R8:
        return 1;
    default:
        LOG(HAL, Error) << "Unsupported DRM fourcc " << drmFormat
                        << ".Forgot to add switch case here?";
        return 0;
    }
}

// A helper function to get the singleton instance of the gralloc module.
static const gralloc_module_t* GetGrallocModule() {
    static const gralloc_module_t* sGrallocModule;
    static std::mutex sGrallocModuleLock;

    std::unique_lock<std::mutex> l(sGrallocModuleLock);
    if (!sGrallocModule) {
        if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID,
                          reinterpret_cast<const hw_module_t**>(&sGrallocModule))) {
            LOG(HAL, Error) << "Failed to initialize gralloc module";
            return nullptr;
        }
    }
    return sGrallocModule;
}

}  // end of anonymous namespace

namespace android {

camera3_stream_buffer_t BufferAdapter::prepareStreamBufferPtr(const camera3_stream_buffer* buffer) {
    std::unique_lock<std::mutex> bufferLock(mBufferHandlesLock);
    buffer_handle_t* nativeHandle = buffer->buffer;
    // Use the pointer value of buffer_handle_t as buffer ID.
    uint64_t bufferId = reinterpret_cast<uint64_t>(nativeHandle);
    uint32_t drmFormat = 0;
    uint32_t width, height;
    uint32_t numPlanes;
    std::vector<uint32_t> strides;
    std::vector<uint32_t> offsets;

    AdaptedHandle &adaptedBuffer = mAdaptedBufferHandles[bufferId];
    adaptedBuffer.buffer_handle = reinterpret_cast<buffer_handle_t>(&adaptedBuffer.camera_buffer_handle);

    camera_buffer_handle_t *handle_ptr = &adaptedBuffer.camera_buffer_handle;

    // Resolve DRM format fourcc from Android format.
    int androidFormat;
    int ret;
    ret = mGrallocModule->perform(mGrallocModule, GRALLOC_DRM_GET_FORMAT, *nativeHandle,
                                  &androidFormat);
    if (ret) {
        LOG(HAL, Error) << "Failed to get format";
        return *buffer;
    }
    auto value = std::find_if(
            std::begin(kSupportedFormats), std::end(kSupportedFormats),
            [androidFormat](struct SupportedFormat f) { return f.halFormat == androidFormat; });
    if (value == std::end(kSupportedFormats)) {
        LOG(HAL, Error) << "Unsupported android format: " << androidFormat;
        return *buffer;
    }
    drmFormat = value->drmFormat;
    if (drmFormat) {
        // Single-planar buffer.
        numPlanes = 1;
        uint32_t stride;
        ret = mGrallocModule->perform(mGrallocModule, GRALLOC_DRM_GET_STRIDE, *nativeHandle,
                                      &stride);
        if (ret) {
            LOG(HAL, Error) << "Failed to get stride with format: " << androidFormat;
            return *buffer;
        }
        strides.push_back(stride * getBpp(drmFormat));
        offsets.push_back(0);
    } else {
        // Multi-planar buffer.
        drmFormat = resolveFormat(*nativeHandle, androidFormat, buffer->stream->usage, &numPlanes,
                                  &strides, &offsets);
    }

    ret = mGrallocModule->perform(mGrallocModule, GRALLOC_DRM_GET_DIMENSIONS, *nativeHandle, &width,
                                  &height);
    if (ret) {
        LOG(HAL, Error) << "Failed to get dimensions with format: " << androidFormat;
        return *buffer;
    }
    const uint32_t numFds = static_cast<uint32_t>((*nativeHandle)->numFds);

    if (!strcmp(mGrallocModule->common.name, "CrOS Gralloc")) {
        // minigbm cros gralloc always have the first numPlanes count of fds to match each
        // plane buffer. The numFds can be (numPlanes + 1) for mapper4 metadata.

        if (numFds < numPlanes) {
            LOG(HAL, Error) << "numFds(" << numFds << ") must be at least numPlanes(" << numPlanes << ") for CrOS Gralloc";
            return *buffer;
        }
        for (uint32_t i = 0; i < numPlanes; i++) {
            int dupFd = HANDLE_EINTR(dup((*nativeHandle)->data[i]));
            if (dupFd < 0) {
                LOG(HAL, Error) << "Failed to dup buffer fd: " << strerror(errno);
                return *buffer;
            }
            handle_ptr->fds[i] = dupFd;
        }
    } else if (numFds == 1) {
        // We assume all planes use the same fd if there's only one fd in the buffer handle (i.e.
        // in drm_gralloc case).  We can remove this code path once we have fully transitioned to
        // cros_gralloc.
        for (size_t i = 0; i < numPlanes; ++i) {
            int dupFd = HANDLE_EINTR(dup((*nativeHandle)->data[0]));
            if (dupFd == -1) {
                LOG(HAL, Error) << "Failed to dup buffer fd: " << strerror(errno);
                return *buffer;
            }
            handle_ptr->fds[i] = dupFd;
        }
    } else {
        if (numFds != numPlanes) {
            LOG(HAL, Error) << "numFds does not match the number of planes";
            return *buffer;
        }
        for (size_t i = 0; i < numPlanes; ++i) {
            int dupFd = HANDLE_EINTR(dup((*nativeHandle)->data[i]));
            if (dupFd == -1) {
                LOG(HAL, Error) << "Failed to dup buffer fd: " << strerror(errno);
                return *buffer;
            }
            handle_ptr->fds[i] = dupFd;
        }
    }

    // When we return the buffer in process_capture_result, the frameworks expect to see the
    // same buffer_handle_t that it passed down before.  Store the received buffer_handle_t
    // so we can restore the |buffer| field in camera3_stream_buffer_t to the original value
    // later in process_capture_result.
    mBufferHandles[bufferId] = nativeHandle;

    camera3_stream_buffer_t new_camera3_stream_buffer = {};
    new_camera3_stream_buffer = (*buffer);

    handle_ptr->buffer_id = bufferId;
    handle_ptr->drm_format = drmFormat;
    handle_ptr->hal_pixel_format = androidFormat;
    handle_ptr->width = width;
    handle_ptr->height = height;
    std::copy(strides.begin(), strides.end(), handle_ptr->strides);
    std::copy(offsets.begin(), offsets.end(), handle_ptr->offsets);

    new_camera3_stream_buffer.buffer = &adaptedBuffer.buffer_handle;

    return new_camera3_stream_buffer;
}

bool BufferAdapter::init() {

    mGrallocModule = GetGrallocModule();
    if (!mGrallocModule) {
        return false;
    }
    return true;
}

int BufferAdapter::decodeStreamBufferPtr(const camera3_stream_buffer_t* ptr,
                                         camera3_stream_buffer_t* outBuffer) {

    std::unique_lock<std::mutex> bufferLock(mBufferHandlesLock);

    camera_buffer_handle_t *reint_cast_from_buffer_handle_t
        = const_cast<camera_buffer_handle_t*>(reinterpret_cast<const camera_buffer_handle_t*>(*ptr->buffer));
    uint64_t bufferId = reint_cast_from_buffer_handle_t->buffer_id;
    if (mBufferHandles.count(bufferId) == 0) {
        LOG(HAL, Error) << "Invalid buffer ID: " << reint_cast_from_buffer_handle_t->buffer_id;
        return -EINVAL;
    }

    if (!mBufferHandles.contains(bufferId)) {
        LOG(HAL, Error) << "cannot find buffer " << bufferId;
        return -EINVAL;
    }

    outBuffer->buffer = mBufferHandles[bufferId];

    for (int i = 0; i < kMaxPlanes; i++) {
        if (reint_cast_from_buffer_handle_t->fds[i] == -1) {
            continue;
        }
        if (close(reint_cast_from_buffer_handle_t->fds[i])) {
            LOG(HAL, Error) << "Failed to close FD: " << reint_cast_from_buffer_handle_t->fds[i];
        } else {
            reint_cast_from_buffer_handle_t->fds[i] = -1;
        }
    }

    mBufferHandles.erase(bufferId);
    mAdaptedBufferHandles.erase(bufferId);

    return 0;
}

uint32_t BufferAdapter::resolveFormat(buffer_handle_t buffer, int androidFormat, uint32_t usage,
                                      uint32_t* outNumPlanes, std::vector<uint32_t>* outStrides,
                                      std::vector<uint32_t>* outOffsets) {
    struct android_ycbcr ycbcr;
    memset(&ycbcr, 0, sizeof(ycbcr));
    // When we call lock_ycbcr() with zero usage, the offsets are filled in |ycbcr.{y|cb|cr}|.
    int ret = mGrallocModule->lock_ycbcr(mGrallocModule, buffer, 0, 0, 0, 0, 0, &ycbcr);
    if (ret && androidFormat == HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED) {
        // HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED can be single-planar RGBX8888 or any multi-planar
        // YUV 420 formats.  Currently we rely on cros_gralloc to only give us YUV 420 formats since
        // we don't have a way to tell 420 from 422 with lock_ycbcr().  If we fail to lock_ycbcr()
        // the implementation defined buffer then we assume it's RGBX8888; otherwise we leave it to
        // resolveFormat to resolve the exact YUV 420 format. (b/32077885)
        uint32_t drmFormat = DRM_FORMAT_XBGR8888;
        // Try to override it to a private format for reprocessing if usage flags include
        // HW_CAMERA_READ.
        if (usage & GRALLOC_USAGE_HW_CAMERA_READ) {
            uint32_t privateFormat = getPrivateFormat();
            if (privateFormat) {
                drmFormat = privateFormat;
            }
        }
        *outNumPlanes = 1;
        uint32_t stride;
        ret = mGrallocModule->perform(mGrallocModule, GRALLOC_DRM_GET_STRIDE, buffer, &stride);
        if (ret) {
            LOG(HAL, Error) << "Failed to get stride with format: " << androidFormat;
            return 0;
        }
        outStrides->push_back(stride * getBpp(drmFormat));
        outOffsets->push_back(0);
        return drmFormat;
    }
    if (ret) {
        LOG(HAL, Error) << "Failed to lock_ycbcr with format: " << androidFormat;
        return 0;
    }
    mGrallocModule->unlock(mGrallocModule, buffer);

    uint32_t offsets[3] = {0, 0, 0};
    uint32_t strides[3] = {0, 0, 0};

    offsets[0] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ycbcr.y));
    offsets[1] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ycbcr.cb));
    offsets[2] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ycbcr.cr));

    strides[0] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ycbcr.ystride));
    strides[1] = strides[2] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ycbcr.cstride));

    bool crcb = false;
    if (offsets[1] > offsets[2]) {
        crcb = true;
        std::swap(offsets[1], offsets[2]);
    }

    bool semiplanar = true;
    *outNumPlanes = 2;
    outStrides->push_back(strides[0]);
    outStrides->push_back(strides[1]);
    outOffsets->push_back(offsets[0]);
    outOffsets->push_back(offsets[1]);
    if (ycbcr.chroma_step == 1) {
        semiplanar = false;
        *outNumPlanes = 3;
        outStrides->push_back(strides[2]);
        outOffsets->push_back(offsets[2]);
    }

    auto value = std::find_if(
            std::begin(kSupportedPlanarFormats), std::end(kSupportedPlanarFormats),
            [androidFormat, crcb, semiplanar](struct SupportedPlanarFormat f) {
                return f.halFormat == androidFormat && f.crcb == crcb && f.semiplanar == semiplanar;
            });
    if (value == std::end(kSupportedPlanarFormats)) {
        LOG(HAL, Error) << "Unsupported planar format:  " << androidFormat << " (crcb=" << crcb << ", semiplanar=" << semiplanar << ")";
        return 0;
    }
    return value->drmFormat;
}

uint32_t BufferAdapter::getPrivateFormat() {
    const char* prop_key = "ro.product.device";
    std::unique_ptr<char[]> device(new char[PROPERTY_VALUE_MAX]);
    if (property_get(prop_key, device.get(), "") == 0) {
        LOG(HAL, Error) << "Failed to read system property: " << prop_key;
    }
    // The value of ro.product.device is ${BOARD}_(cheets|bertha)
    // See
    // http://cs/chromeos_internal/src/private-overlays/project-cheets-private/scripts/board_specific_setup.sh?l=917&rcl=59fdaf4641708433c5ecdcf34fa15e9e36e5aa7e
    std::string device_string(device.get());
    size_t pos = device_string.rfind("_");
    if (pos == std::string::npos) {
        LOG(HAL, Error) << prop_key << " has unexpected format: " << device_string.c_str();
        return 1;
    }
    std::string board = device_string.substr(0, pos);
    if (board == "kukui") {
        return DRM_FORMAT_MTISP_SXYZW10;
    }
    return 0;
}

}  // end of namespace android
