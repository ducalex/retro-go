/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _PORT_H_
#define _PORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>

#ifndef PIXEL_FORMAT
#define PIXEL_FORMAT RGB565
#endif

// typedef unsigned char		bool8;
typedef bool				bool8;
typedef intptr_t			pint;
typedef int8_t				int8;
typedef uint8_t				uint8;
typedef int16_t				int16;
typedef uint16_t			uint16;
typedef int32_t				int32;
typedef uint32_t			uint32;
typedef int64_t				int64;
typedef uint64_t			uint64;

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#undef PATH_MAX
#define PATH_MAX 512

#define _MAX_DRIVE	1
#define _MAX_DIR	PATH_MAX
#define _MAX_FNAME	PATH_MAX
#define _MAX_EXT	PATH_MAX
#define _MAX_PATH	PATH_MAX

#ifndef TITLE
#define TITLE "Snes9x"
#endif

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) || defined(__x86_64__) || defined(__alpha__) || defined(__MIPSEL__) || defined(_M_IX86) || defined(_M_X64) || defined(_XBOX1) || defined(__arm__) || defined(ANDROID) || defined(__aarch64__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER == __ORDER_LITTLE_ENDIAN__) || defined(IS_LITTLE_ENDIAN)
#define LSB_FIRST
#define FAST_LSB_WORD_ACCESS
#else
#define MSB_FIRST
#endif

#ifdef FAST_LSB_WORD_ACCESS
#define READ_WORD(s)		(*(uint16 *) (s))
#define READ_3WORD(s)		(*(uint32 *) (s) & 0x00ffffff)
#define READ_DWORD(s)		(*(uint32 *) (s))
#define WRITE_WORD(s, d)	*(uint16 *) (s) = (d)
#define WRITE_3WORD(s, d)	*(uint16 *) (s) = (uint16) (d), *((uint8 *) (s) + 2) = (uint8) ((d) >> 16)
#define WRITE_DWORD(s, d)	*(uint32 *) (s) = (d)
#else
#define READ_WORD(s)		(*(uint8 *) (s) | (*((uint8 *) (s) + 1) << 8))
#define READ_3WORD(s)		(*(uint8 *) (s) | (*((uint8 *) (s) + 1) << 8) | (*((uint8 *) (s) + 2) << 16))
#define READ_DWORD(s)		(*(uint8 *) (s) | (*((uint8 *) (s) + 1) << 8) | (*((uint8 *) (s) + 2) << 16) | (*((uint8 *) (s) + 3) << 24))
#define WRITE_WORD(s, d)	*(uint8 *) (s) = (uint8) (d), *((uint8 *) (s) + 1) = (uint8) ((d) >> 8)
#define WRITE_3WORD(s, d)	*(uint8 *) (s) = (uint8) (d), *((uint8 *) (s) + 1) = (uint8) ((d) >> 8), *((uint8 *) (s) + 2) = (uint8) ((d) >> 16)
#define WRITE_DWORD(s, d)	*(uint8 *) (s) = (uint8) (d), *((uint8 *) (s) + 1) = (uint8) ((d) >> 8), *((uint8 *) (s) + 2) = (uint8) ((d) >> 16), *((uint8 *) (s) + 3) = (uint8) ((d) >> 24)
#endif

#define SWAP_WORD(s)		(s) = (((s) & 0xff) <<  8) | (((s) & 0xff00) >> 8)
#define SWAP_DWORD(s)		(s) = (((s) & 0xff) << 24) | (((s) & 0xff00) << 8) | (((s) & 0xff0000) >> 8) | (((s) & 0xff000000) >> 24)


#ifndef _PIXFORM_H_
#define _PIXFORM_H_

/* RGB565 format */
#define BUILD_PIXEL_RGB565(R, G, B)  (((int)(R) << 11) | ((int)(G) << 6) | (((int)(G) & 0x10) << 1) | (int)(B))
#define BUILD_PIXEL2_RGB565(R, G, B) (((int)(R) << 11) | ((int)(G) << 5) | (int)(B))
#define DECOMPOSE_PIXEL_RGB565(PIX, R, G, B) \
    {                                        \
        (R) = (PIX) >> 11;                   \
        (G) = ((PIX) >> 6) & 0x1f;           \
        (B) = (PIX)&0x1f;                    \
    }
#define SPARE_RGB_BIT_MASK_RGB565 (1 << 5)

#define MAX_RED_RGB565            31
#define MAX_GREEN_RGB565          63
#define MAX_BLUE_RGB565           31
#define RED_SHIFT_BITS_RGB565     11
#define GREEN_SHIFT_BITS_RGB565   6
#define RED_LOW_BIT_MASK_RGB565   0x0800
#define GREEN_LOW_BIT_MASK_RGB565 0x0020
#define BLUE_LOW_BIT_MASK_RGB565  0x0001
#define RED_HI_BIT_MASK_RGB565    0x8000
#define GREEN_HI_BIT_MASK_RGB565  0x0400
#define BLUE_HI_BIT_MASK_RGB565   0x0010
#define FIRST_COLOR_MASK_RGB565   0xF800
#define SECOND_COLOR_MASK_RGB565  0x07E0
#define THIRD_COLOR_MASK_RGB565   0x001F
#define ALPHA_BITS_MASK_RGB565    0x0000

/* RGB555 format */
#define BUILD_PIXEL_RGB555(R, G, B)  (((int)(R) << 10) | ((int)(G) << 5) | (int)(B))
#define BUILD_PIXEL2_RGB555(R, G, B) (((int)(R) << 10) | ((int)(G) << 5) | (int)(B))
#define DECOMPOSE_PIXEL_RGB555(PIX, R, G, B) \
    {                                        \
        (R) = (PIX) >> 10;                   \
        (G) = ((PIX) >> 5) & 0x1f;           \
        (B) = (PIX)&0x1f;                    \
    }
#define SPARE_RGB_BIT_MASK_RGB555 (1 << 15)

#define MAX_RED_RGB555            31
#define MAX_GREEN_RGB555          31
#define MAX_BLUE_RGB555           31
#define RED_SHIFT_BITS_RGB555     10
#define GREEN_SHIFT_BITS_RGB555   5
#define RED_LOW_BIT_MASK_RGB555   0x0400
#define GREEN_LOW_BIT_MASK_RGB555 0x0020
#define BLUE_LOW_BIT_MASK_RGB555  0x0001
#define RED_HI_BIT_MASK_RGB555    0x4000
#define GREEN_HI_BIT_MASK_RGB555  0x0200
#define BLUE_HI_BIT_MASK_RGB555   0x0010
#define FIRST_COLOR_MASK_RGB555   0x7C00
#define SECOND_COLOR_MASK_RGB555  0x03E0
#define THIRD_COLOR_MASK_RGB555   0x001F
#define ALPHA_BITS_MASK_RGB555    0x0000

#define CONCAT(X, Y) X##Y

// C pre-processor needs a two stage macro define to enable it to concat
// to macro names together to form the name of another macro.
#define BUILD_PIXEL_D(F, R, G, B)          CONCAT(BUILD_PIXEL_, F) (R, G, B)
#define BUILD_PIXEL2_D(F, R, G, B)         CONCAT(BUILD_PIXEL2_, F) (R, G, B)
#define DECOMPOSE_PIXEL_D(F, PIX, R, G, B) CONCAT(DECOMPOSE_PIXEL_, F) (PIX, R, G, B)

#define BUILD_PIXEL(R, G, B)               BUILD_PIXEL_D(PIXEL_FORMAT, R, G, B)
#define BUILD_PIXEL2(R, G, B)              BUILD_PIXEL2_D(PIXEL_FORMAT, R, G, B)
#define DECOMPOSE_PIXEL(PIX, R, G, B)      DECOMPOSE_PIXEL_D(PIXEL_FORMAT, PIX, R, G, B)

#define MAX_RED_D(F)            CONCAT(MAX_RED_, F)
#define MAX_GREEN_D(F)          CONCAT(MAX_GREEN_, F)
#define MAX_BLUE_D(F)           CONCAT(MAX_BLUE_, F)
#define RED_SHIFT_BITS_D(F)     CONCAT(RED_SHIFT_BITS_, F)
#define GREEN_SHIFT_BITS_D(F)   CONCAT(GREEN_SHIFT_BITS_, F)
#define RED_LOW_BIT_MASK_D(F)   CONCAT(RED_LOW_BIT_MASK_, F)
#define GREEN_LOW_BIT_MASK_D(F) CONCAT(GREEN_LOW_BIT_MASK_, F)
#define BLUE_LOW_BIT_MASK_D(F)  CONCAT(BLUE_LOW_BIT_MASK_, F)
#define RED_HI_BIT_MASK_D(F)    CONCAT(RED_HI_BIT_MASK_, F)
#define GREEN_HI_BIT_MASK_D(F)  CONCAT(GREEN_HI_BIT_MASK_, F)
#define BLUE_HI_BIT_MASK_D(F)   CONCAT(BLUE_HI_BIT_MASK_, F)
#define FIRST_COLOR_MASK_D(F)   CONCAT(FIRST_COLOR_MASK_, F)
#define SECOND_COLOR_MASK_D(F)  CONCAT(SECOND_COLOR_MASK_, F)
#define THIRD_COLOR_MASK_D(F)   CONCAT(THIRD_COLOR_MASK_, F)
#define ALPHA_BITS_MASK_D(F)    CONCAT(ALPHA_BITS_MASK_, F)

#define MAX_RED            MAX_RED_D(PIXEL_FORMAT)
#define MAX_GREEN          MAX_GREEN_D(PIXEL_FORMAT)
#define MAX_BLUE           MAX_BLUE_D(PIXEL_FORMAT)
#define RED_SHIFT_BITS     RED_SHIFT_BITS_D(PIXEL_FORMAT)
#define GREEN_SHIFT_BITS   GREEN_SHIFT_BITS_D(PIXEL_FORMAT)
#define RED_LOW_BIT_MASK   RED_LOW_BIT_MASK_D(PIXEL_FORMAT)
#define GREEN_LOW_BIT_MASK GREEN_LOW_BIT_MASK_D(PIXEL_FORMAT)
#define BLUE_LOW_BIT_MASK  BLUE_LOW_BIT_MASK_D(PIXEL_FORMAT)
#define RED_HI_BIT_MASK    RED_HI_BIT_MASK_D(PIXEL_FORMAT)
#define GREEN_HI_BIT_MASK  GREEN_HI_BIT_MASK_D(PIXEL_FORMAT)
#define BLUE_HI_BIT_MASK   BLUE_HI_BIT_MASK_D(PIXEL_FORMAT)
#define FIRST_COLOR_MASK   FIRST_COLOR_MASK_D(PIXEL_FORMAT)
#define SECOND_COLOR_MASK  SECOND_COLOR_MASK_D(PIXEL_FORMAT)
#define THIRD_COLOR_MASK   THIRD_COLOR_MASK_D(PIXEL_FORMAT)
#define ALPHA_BITS_MASK    ALPHA_BITS_MASK_D(PIXEL_FORMAT)

#define GREEN_HI_BIT               ((MAX_GREEN + 1) >> 1)
#define RGB_LOW_BITS_MASK          (RED_LOW_BIT_MASK | GREEN_LOW_BIT_MASK | BLUE_LOW_BIT_MASK)
#define RGB_HI_BITS_MASK           (RED_HI_BIT_MASK | GREEN_HI_BIT_MASK | BLUE_HI_BIT_MASK)
#define RGB_HI_BITS_MASKx2         ((RED_HI_BIT_MASK | GREEN_HI_BIT_MASK | BLUE_HI_BIT_MASK) << 1)
#define RGB_REMOVE_LOW_BITS_MASK   (~RGB_LOW_BITS_MASK)
#define FIRST_THIRD_COLOR_MASK     (FIRST_COLOR_MASK | THIRD_COLOR_MASK)
#define TWO_LOW_BITS_MASK          (RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 1))
#define HIGH_BITS_SHIFTED_TWO_MASK (((FIRST_COLOR_MASK | SECOND_COLOR_MASK | THIRD_COLOR_MASK) & ~TWO_LOW_BITS_MASK) >> 2)

#endif // _PIXFORM_H_

#endif
