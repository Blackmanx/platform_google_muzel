# SPDX-License-Identifier: CC0-1.0

ipa_name = 'ipa_mtkisp7'

mtkisp7_ipa_sources = files([
  'mtkisp7.cpp',
  'face_detect/parser.cpp',
  'hal3a/hal_3a.cpp',
  'hal3a/utils_3a.cpp',
  'halisp/bss.cpp',
  'halisp/hal_isp.cpp',
  'halisp/swme.cpp',
])

libcamalgo_fdft = cc.find_library('camalgo.fdft', required : false)
libcam_hal3a = cc.find_library('libcam.hal3a', required : true)
libcam_hal3a_v3_nvram_v4l2 = cc.find_library('libcam.hal3a.v3.nvram.v4l2', required : true)
libmtkcam_metadata_container = cc.find_library('libmtkcam_metadata.container', required : true, static : true)
libmtkcam_ulog = cc.find_library('libmtkcam_ulog', required : true)
libmtk_platform_log = cc.find_library('libmtk_platform_log', required : true)
libmtkcam_debugwrapper = cc.find_library('libmtkcam.debugwrapper', required : true)
libcam_halsensor = cc.find_library('libcam.halsensor', required : true)
libcam_halisp_utils_pub = cc.find_library('libcam.halisp.utils.pub', required : true)

libcam_halisp_imp_v4l2 = cc.find_library('cam.halisp.imp.v4l2', required : true)
libcam_halisp_tuning_provider = cc.find_library('cam.halisp.TuningDataProvider', required : true)

libmtkcam_tuning_utils = cc.find_library('libmtkcam_tuning_utils', required : true)
libmtkcam_stdutils = cc.find_library('libmtkcam_stdutils', required : true)

libcamalgo_bss = cc.find_library('camalgo.bss', required : true)
libcamalgo_mfnr = cc.find_library('camalgo.mfnr', required : true)

libcam_halisp_plugin = cc.find_library('cam.halisp.plugin', required : true)

mtkisp7_deps = [
    libcamalgo_fdft,
    libcam_hal3a,
    libcam_hal3a_v3_nvram_v4l2,
    libmtkcam_metadata_container,
    libmtkcam_ulog,
    libmtk_platform_log,
    libmtkcam_debugwrapper,
    libcam_halsensor,
    libcam_halisp_utils_pub,
    libcam_halisp_imp_v4l2,
    libcam_halisp_tuning_provider,
    libmtkcam_tuning_utils,
    libmtkcam_stdutils,
    libcamalgo_bss,
    libcamalgo_mfnr,
    libcamera_private,
    libcam_halisp_plugin
]

mod = shared_module(ipa_name,
                    [mtkisp7_ipa_sources, libcamera_generated_ipa_headers],
                    name_prefix : '',
                    include_directories : [ipa_includes, libipa_includes],
                    dependencies : mtkisp7_deps,
                    link_with : libipa,
                    install : true,
                    install_dir : ipa_install_dir)

if ipa_sign_module
    custom_target(ipa_name + '.so.sign',
                  input : mod,
                  output : ipa_name + '.so.sign',
                  command : [ipa_sign, ipa_priv_key, '@INPUT@', '@OUTPUT@'],
                  install : false,
                  build_by_default : true)
endif

ipa_names += ipa_name
