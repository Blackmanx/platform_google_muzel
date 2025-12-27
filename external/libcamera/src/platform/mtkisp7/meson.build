# SPDX-License-Identifier: CC0-1.0

mtkisp7_platform_sources = files([
  'single_device_helper.cpp',
  'Tuning_Helper.cpp',
  'imgsensor_info_helper.cpp',
  'cam_cal_func.cpp',
  'cam_cal_helper.cpp',
  'platform_utils.cpp'
])

libcam_halsensor = cc.find_library('libcam.halsensor', required : true)

platform_mtkisp7_libs = shared_library('libcamera-mtkisp7-platform',
                                       mtkisp7_platform_sources,
                                       name_prefix : '',
                                       include_directories : libcamera_includes,
                                       dependencies : [libcam_halsensor],
                                       install : true)
