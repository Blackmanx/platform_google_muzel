# SPDX-License-Identifier: CC0-1.0

if get_option('android').enabled() and get_option('android_platform') == 'cros'
  tracepoint_files = files([
      'pipeline.perfetto',
      'request.perfetto',
  ])
else
  # enum files must go first
  tracepoint_files = files([
      'buffer_enums.tp',
      'request_enums.tp',
  ])

  tracepoint_files += files([
      'pipeline.tp',
      'request.tp',
  ])
endif
