# SPDX-License-Identifier: CC0-1.0

libmtkcam_metadata_container = cc.find_library('libmtkcam_metadata.container', required : true, static : true)
libmtkcam_ulog = cc.find_library('libmtkcam_ulog', required : true)
libmtk_platform_log = cc.find_library('libmtk_platform_log', required : true)
libmtkcam_debugwrapper = cc.find_library('libmtkcam.debugwrapper', required : true)
libcam_halsensor = cc.find_library('libcam.halsensor', required : true)
libcam_halisp_utils_pub = cc.find_library('libcam.halisp.utils.pub', required : true)

# TODO: move to IPA
libmtkcam_tuning_utils = cc.find_library('libmtkcam_tuning_utils', required : true)
libmtkcam_stdutils = cc.find_library('libmtkcam_stdutils', required : true)

libcamera_deps += [
    libmtkcam_metadata_container,
    libmtkcam_ulog,
    libmtk_platform_log,
    libmtkcam_debugwrapper,
    libcam_halsensor,
    libcam_halisp_utils_pub,
    libmtkcam_tuning_utils,
    libmtkcam_stdutils,
]

libcamera_sources += files([
    'camsys/camsys.cpp',
    'camsys/capture.cpp',
    'face_detect/aie.cpp',
    'face_detect/detector.cpp',
    'hal3a/aaa.cpp',
    'halisp/imgsys_task.cpp',
    'halisp/lpnr_tun.cpp',
    'halisp/mcnr_tun.cpp',
    'halisp/mfnr_tun.cpp',
    'imgsys/lpnr.cpp',
    'imgsys/imgsys.cpp',
    'imgsys/mcnr.cpp',
    'imgsys/mfnr.cpp',
    'imgsys/single_device.cpp',
    'ipa/ipa_delegate.cpp',
    'odt/imagiq_adapter/static_metadata/static_strings.cpp',
    'odt/imagiq_adapter/dump.cpp',
    'odt/imagiq_adapter/imagiq_adapter.cpp',
    'odt/camsys_driver_debug.cpp',
    'odt/imgsys_driver_debug.cpp',
    'odt/on_device_tuner.cpp',
    'utils/img_meta_request_data_helper.cpp',
    'mtkisp7.cpp'
])

platform_libs += platform_mtkisp7_libs
