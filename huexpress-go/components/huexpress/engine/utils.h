/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _UTILS_C
#define _UTILS_C

#include <stdint.h>
#include <stdbool.h>

#define MESSAGE_ERROR(x...) osd_log("!! " x)
#define MESSAGE_INFO(x...) osd_log(" * " x)
#if DEBUG_ENABLED
#define MESSAGE_DEBUG(x...) osd_log(" ~ " x)
#else
#define MESSAGE_DEBUG(x...) {}
#endif

#if ENABLE_SPR_TRACING
#define TRACE_SPR(x...) osd_log(" & (SPR) " x)
#else
#define TRACE_SPR(x...) {}
#endif

#if ENABLE_GFX_TRACING
#define TRACE_GFX(x...) osd_log(" & (GFX) " x)
#else
#define TRACE_GFX(x...) {}
#endif

#if ENABLE_GFX2_TRACING
#define TRACE_GFX2(x...) osd_log(" & (GFX2) " x)
#else
#define TRACE_GFX2(x...) {}
#endif

#if ENABLE_IO_TRACING
#define TRACE_IO(x...) osd_log(" & (IO) " x)
#else
#define TRACE_IO(x...) {}
#endif

#if ENABLE_CPU_TRACING
#define TRACE_CPU(x...) osd_log(" & (CPU) " x)
#else
#define TRACE_CPU(x...) {}
#endif

#undef MIN
#define MIN(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#undef MAX
#define MAX(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })

#endif
