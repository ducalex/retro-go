/* This file is part of Snes9x. See LICENSE file. */

/* CPU Byte Order Utilities */

/* snes_spc 0.9.0 */
#ifndef BLARGG_ENDIAN
#define BLARGG_ENDIAN

/* Uncomment to enable platform-specific (and possibly non-portable) optimizations */
#if !defined(EMSCRIPTEN) && !defined(__mips__)
#define BLARGG_NONPORTABLE 1
#endif

/* PS3 - if SNC compiler is used - enable platform-specific optimizations  */
#ifdef __SNC__
#include <ppu_intrinsics.h>
#define BLARGG_NONPORTABLE 1
#endif

#include <stdbool.h>
#include <stdint.h>

/* BLARGG_CPU_CISC: Defined if CPU has very few general-purpose registers (< 16) */
#if defined (_M_IX86) || defined (_M_IA64) || defined (__i486__) || \
      defined (__x86_64__) || defined (__ia64__) || defined (__i386__) || defined(ANDROID_X86)
   #define BLARGG_CPU_X86 1
   #define BLARGG_CPU_CISC 1
#endif

#if defined (__powerpc__) || defined (__ppc__) || defined (__POWERPC__) || defined (__powerc)
   #define BLARGG_CPU_POWERPC 1
   #define BLARGG_CPU_RISC 1
#endif

#if BLARGG_NONPORTABLE
   /* Optimized implementation if byte order is known */

#ifdef MSB_FIRST
#if BLARGG_CPU_POWERPC
   /* PowerPC has special byte-reversed instructions */
#if defined (__SNC__)
#define GET_LE16( addr )        (__builtin_lhbrx(addr, 0))
#define GET_LE32( addr )        (__builtin_lwbrx(addr, 0))
#define SET_LE16( addr, in )    (__builtin_sthbrx(in, addr, 0))
#define SET_LE32( addr, in )    (__builtin_stwbrx(in, addr, 0))
#elif defined (_XBOX360)
#include <PPCIntrinsics.h>
#define GET_LE16( addr )        (__loadshortbytereverse(0, addr))
#define GET_LE32( addr )        (__loadwordbytereverse(0, addr))
#define SET_LE16( addr, in )    (__storeshortbytereverse(in, 0, addr))
#define SET_LE32( addr, in )    (__storewordbytereverse(in, 0, addr))
#elif defined (__MWERKS__)
#define GET_LE16( addr )        (__lhbrx( addr, 0 ))
#define GET_LE32( addr )        (__lwbrx( addr, 0 ))
#define SET_LE16( addr, in )    (__sthbrx( in, addr, 0 ))
#define SET_LE32( addr, in )    (__stwbrx( in, addr, 0 ))
#elif defined (__GNUC__)
#define GET_LE16( addr )        ({unsigned ppc_lhbrx_; asm( "lhbrx %0,0,%1" : "=r" (ppc_lhbrx_) : "r" (addr), "0" (ppc_lhbrx_) ); ppc_lhbrx_;})
#define GET_LE32( addr )        ({unsigned ppc_lwbrx_; asm( "lwbrx %0,0,%1" : "=r" (ppc_lwbrx_) : "r" (addr), "0" (ppc_lwbrx_) ); ppc_lwbrx_;})
#define SET_LE16( addr, in )    ({asm( "sthbrx %0,0,%1" : : "r" (in), "r" (addr) );})
#define SET_LE32( addr, in )    ({asm( "stwbrx %0,0,%1" : : "r" (in), "r" (addr) );})
#endif
#endif
#else
#define GET_LE16( addr )        (*(uint16_t*) (addr))
#define GET_LE32( addr )        (*(uint32_t*) (addr))
#define SET_LE16( addr, data )  (void) (*(uint16_t*) (addr) = (data))
#define SET_LE32( addr, data )  (void) (*(uint32_t*) (addr) = (data))
#endif
#else
static inline uint32_t get_le16( void const* p )
{
   return (uint32_t) ((uint8_t const*) p) [1] << 8 | (uint32_t) ((uint8_t const*) p) [0];
}

static inline uint32_t get_le32( void const* p )
{
   return (uint32_t) ((uint8_t const*) p) [3] << 24 |
      (uint32_t) ((uint8_t const*) p) [2] << 16 |
      (uint32_t) ((uint8_t const*) p) [1] << 8 |
      (uint32_t) ((uint8_t const*) p) [0];
}

static inline void set_le16( void* p, uint32_t n )
{
   ((uint8_t*) p) [1] = (uint8_t) (n >> 8);
   ((uint8_t*) p) [0] = (uint8_t) n;
}

static inline void set_le32( void* p, uint32_t n )
{
   ((uint8_t*) p) [0] = (uint8_t) n;
   ((uint8_t*) p) [1] = (uint8_t) (n >> 8);
   ((uint8_t*) p) [2] = (uint8_t) (n >> 16);
   ((uint8_t*) p) [3] = (uint8_t) (n >> 24);
}
#endif

#ifndef GET_LE16
   #define GET_LE16( addr )        get_le16( addr )
   #define SET_LE16( addr, data )  set_le16( addr, data )
#endif

#ifndef GET_LE32
   #define GET_LE32( addr )        get_le32( addr )
   #define SET_LE32( addr, data )  set_le32( addr, data )
#endif

#define GET_LE16SA( addr )      ((int16_t) GET_LE16( addr ))
#define GET_LE16A( addr )       GET_LE16( addr )
#define SET_LE16A( addr, data ) SET_LE16( addr, data )

#endif
