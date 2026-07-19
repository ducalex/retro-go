#ifndef __NDS_COMPAT_H__
#define __NDS_COMPAT_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rg_system.h>
#include "stella_types.h"

// System Cycle counter (Declared globally in System.hxx with C++ linkage)
extern Int32 gSystemCycles;

#ifdef __cplusplus
extern "C" {
#endif

// Memory Sections
#define SECTION_ITCM IRAM_ATTR
#define ITCM_CODE    IRAM_ATTR
#define SECTION_DTCM 
#define DTCM_DATA    

// NDS Types
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;

// NDS Constants
#define NTSC 0
#define PAL  1

// Stub out NDS hardware access
extern uint8_t *BG_GFX;
#define VRAM_A (uint8_t*)0
#define VRAM_B (uint8_t*)0

// Memory Copy
#define dmaCopyWordsAsynch(c, s, d, l) memcpy(d, s, l)

// DSi Detection
static inline bool isDSiMode() { return true; }

// Alignment/Packing
#define PACKED __attribute__((packed))

// Timer access stub
#define TIMER0_DATA 0

#ifdef __cplusplus
}
#endif

#endif // __NDS_COMPAT_H__
