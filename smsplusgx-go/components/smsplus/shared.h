#ifndef _SHARED_H_
#define _SHARED_H_

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;

typedef signed char int8;
typedef signed short int int16;
typedef signed long int int32;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <limits.h>
#include <math.h>

#ifdef RETRO_GO
#include <rg_system.h>
#define LOG_PRINTF(level, x...) rg_system_log(RG_LOG_USER, NULL, x)
#else
#define LOG_PRINTF(level, x...) printf(x)
#define DRAM_ATTR
#define IRAM_ATTR
#define crc32_le(a, b, c) (0)
#endif

#define MESSAGE_ERROR(x, ...) LOG_PRINTF(1, "!! %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_WARN(x, ...)  LOG_PRINTF(2, "** %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_INFO(x, ...)  LOG_PRINTF(3, " * %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_DEBUG(x, ...) LOG_PRINTF(4, ">> %s: " x, __func__, ## __VA_ARGS__)

#include "z80.h"
#include "sms.h"
#include "pio.h"
#include "memz80.h"
#include "vdp.h"
#include "render.h"
#include "tms.h"
#include "sn76489.h"
#include "emu2413.h"
#include "ym2413.h"
#include "fmintf.h"
#include "sound.h"
#include "system.h"
#include "loadrom.h"
// #include "config.h"
#include "state.h"

#endif /* _SHARED_H_ */
