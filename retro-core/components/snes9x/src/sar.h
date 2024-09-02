/* This file is part of Snes9x. See LICENSE file. */

#ifndef _SAR_H_
#define _SAR_H_

#include <stdint.h>

#include "port.h"

#ifdef RIGHTSHIFT_IS_SAR
#define SAR8(b, n)  ((b) >> (n))
#define SAR16(b, n) ((b) >> (n))
#define SAR32(b, n) ((b) >> (n))
#define SAR64(b, n) ((b) >> (n))
#else

static INLINE int8_t SAR8(const int8_t b, const int32_t n)
{
#ifndef RIGHTSHIFT_INT8_IS_SAR
   if (b < 0)
      return (b >> n) | (~0u << (8 - n));
#endif
   return b >> n;
}

static INLINE int16_t SAR16(const int16_t b, const int32_t n)
{
#ifndef RIGHTSHIFT_INT16_IS_SAR
   if (b < 0)
      return (b >> n) | (~0u << (16 - n));
#endif
   return b >> n;
}

static INLINE int32_t SAR32(const int32_t b, const int32_t n)
{
#ifndef RIGHTSHIFT_INT32_IS_SAR
   if (b < 0)
      return (b >> n) | (~0u << (32 - n));
#endif
   return b >> n;
}

static INLINE int64_t SAR64(const int64_t b, const int32_t n)
{
#ifndef RIGHTSHIFT_INT64_IS_SAR
   if (b < 0)
      return (b >> n) | (~0u << (64 - n));
#endif
   return b >> n;
}
#endif
#endif
