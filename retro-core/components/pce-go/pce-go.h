#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#ifdef RETRO_GO
#include <rg_system.h>
#define LOG_PRINTF(level, x...) rg_system_log(RG_LOG_PRINTF, NULL, x)
#define crc32_le(a, b, c) rg_crc32(a, b, c)
#else
#define LOG_PRINTF(level, x...) printf(x)
#define crc32_le(a, b, c) (0)
#endif

#define MESSAGE_ERROR(x...) LOG_PRINTF(1, "!! " x)
#define MESSAGE_WARN(x...)  LOG_PRINTF(2, " ! " x)
#define MESSAGE_INFO(x...)  LOG_PRINTF(3, " * " x)
#define MESSAGE_TRACE(tag, x...) LOG_PRINTF(4, " & (" tag ") " x)
#if ENABLE_DEBUG
#define MESSAGE_DEBUG(x...) LOG_PRINTF(4, " > " x)
#else
#define MESSAGE_DEBUG(x...) {}
#endif

#if ENABLE_SPR_TRACING
#define TRACE_SPR(x...) MESSAGE_TRACE("SPR", x)
#else
#define TRACE_SPR(x...) {}
#endif

#if ENABLE_GFX_TRACING
#define TRACE_GFX(x...) MESSAGE_TRACE("GFX", x)
#else
#define TRACE_GFX(x...) {}
#endif

#if ENABLE_IO_TRACING
#define TRACE_IO(x...) MESSAGE_TRACE("IO", x)
#else
#define TRACE_IO(x...) {}
#endif

#if ENABLE_CPU_TRACING
#define TRACE_CPU(x...) MESSAGE_TRACE("CPU", x)
#else
#define TRACE_CPU(x...) {}
#endif

#undef MIN
#define MIN(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#undef MAX
#define MAX(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })

#define JOY_A       0x01
#define JOY_B       0x02
#define JOY_SELECT  0x04
#define JOY_RUN     0x08
#define JOY_UP      0x10
#define JOY_RIGHT   0x20
#define JOY_DOWN    0x40
#define JOY_LEFT    0x80

// We need 16 bytes of scratch area on both side of each line. The 16 bytes can be shared by adjacent lines.
// The buffer should look like [16 bytes] [line 1] [16 bytes] ... [16 bytes] [line 242] [16 bytes]
#define XBUF_WIDTH 	(352 + 16)
#define	XBUF_HEIGHT	(242)

int InitPCE(int samplerate, bool stereo);
void ResetPCE(bool hard);
void InputPCE(int port, uint32_t value);
void RunPCE(bool draw);
void ShutdownPCE(void);

int LoadState(const char *name);
int SaveState(const char *name);
int LoadCard(uint8_t *data, size_t size);
int LoadFile(const char *name);
void *PalettePCE(int bitdepth);

extern uint8_t *osd_gfx_framebuffer(int width, int height);
