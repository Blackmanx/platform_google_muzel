# SPDX-License-Identifier: CC0-1.0

platforms = ['mtkisp7']

foreach pipeline : pipelines
    if not platforms.contains(pipeline)
        continue
    endif
    subdir(pipeline)
endforeach
