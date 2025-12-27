#ifndef __ADL_META_H__
#define __ADL_META_H__

#include <stdint.h>

typedef struct slb_info {
  uintptr_t slb_ping_offset;    // SLB ping buffer base
  uintptr_t slb_pong_offset;    // SLB pong buffer base
  int32_t   slb_stride;       // SLB buffer stride (i.e. bytes)
  int32_t   slb_line_count;   // SLB buffer N line count
} slb_info;

/**
 * @brief ctrl meta usage for traw driver
 */
typedef struct adl_ctrl {
  int32_t   apu_tile_wdith;

  uintptr_t dram_base;

  /*
   * 0 for Bayer or Y channel
   * 1 for UV channel
   */
  slb_info  slb_channel[2];
} adl_ctrl;

#endif  // __ADL_META_H__
