/* This file is part of Snes9x. See LICENSE file. */

#ifndef _GFX_H_
#define _GFX_H_

#include "port.h"
#include "ppu.h"
#include "snes9x.h"


void S9xStartScreenRefresh(void);
void S9xDrawScanLine(uint8_t Line);
void S9xEndScreenRefresh(void);
void S9xSetupOBJ(void);
void S9xUpdateScreen(void);
void RenderLine(uint8_t line);

bool S9xInitGFX(void);
void S9xDeinitGFX(void);

typedef struct
{
   uint8_t RTOFlags;
   int16_t Tiles;
   struct
   {
      int8_t  Sprite;
      uint8_t Line;
   } OBJ[32];
} SOBJLines;

typedef struct
{
   uint8_t*    Screen;
   uint8_t*    SubScreen;
   uint8_t*    ZBuffer;
   uint8_t*    SubZBuffer;
   uint32_t    Pitch;

   int32_t     Delta;
   uint16_t*   ZERO;
   uint32_t    RealPitch;  /* True pitch of Screen buffer. */
   uint32_t    Pitch2;     /* Same as RealPitch except while using speed up hack for Glide. */
   uint32_t    ZPitch;     /* Pitch of ZBuffer */
   uint32_t    PPL;        /* Number of pixels on each of Screen buffer */
   uint32_t    PPLx2;
   uint32_t    PixSize;
   uint8_t*    S;
   uint8_t*    DB;
   ptrdiff_t   DepthDelta;
   uint8_t     Z1;         /* Depth for comparison */
   uint8_t     Z2;         /* Depth to save */
   uint32_t    FixedColour;
   uint32_t    StartY;
   uint32_t    EndY;
   ClipData*   pCurrentClip;
   uint32_t    Mode7Mask;
   uint32_t    Mode7PriorityMask;
   uint8_t     OBJWidths[128];
   uint8_t     OBJVisibleTiles[128];
   SOBJLines   *OBJLines; // [SNES_HEIGHT_EXTENDED];
   uint8_t     r212c;
   uint8_t     r212d;
   uint8_t     r2130;
   uint8_t     r2131;
   bool        Pseudo;
} SGFX;

/* External port interface which must be implemented or initialised for each port. */
extern SGFX GFX;

typedef struct
{
   struct
   {
      uint16_t VOffset;
      uint16_t HOffset;
   } BG [4];
} SLineData;

#define H_FLIP 0x4000
#define V_FLIP 0x8000
#define BLANK_TILE 2

typedef struct
{
   uint32_t TileSize;
   uint32_t BitShift;
   uint32_t TileShift;
   uint32_t TileAddress;
   uint32_t NameSelect;
   uint32_t SCBase;
   uint32_t StartPalette;
   uint32_t PaletteShift;
   uint32_t PaletteMask;
   uint8_t  Depth;
   uint8_t* Buffer;
   uint8_t* Buffered;
   bool     DirectColourMode;
} SBG;

typedef struct
{
   int16_t MatrixA;
   int16_t MatrixB;
   int16_t MatrixC;
   int16_t MatrixD;
   int16_t CentreX;
   int16_t CentreY;
} SLineMatrixData;

extern SBG BG;

/* Could use BSWAP instruction on Intel port... */
#define SWAP_DWORD(dword) dword = ((((dword) & 0x000000ff) << 24) \
                                |  (((dword) & 0x0000ff00) <<  8) \
                                |  (((dword) & 0x00ff0000) >>  8) \
                                |  (((dword) & 0xff000000) >> 24))

#ifdef FAST_LSB_WORD_ACCESS
#define READ_2BYTES(s)     (*(uint16_t *) (s))
#define WRITE_2BYTES(s, d)  *(uint16_t *) (s) = (d)
#elif defined(MSB_FIRST)
#define READ_2BYTES(s)     (*(uint8_t *) (s) | (*((uint8_t *) (s) + 1) << 8))
#define WRITE_2BYTES(s, d)  *(uint8_t *) (s) = (d), *((uint8_t *) (s) + 1) = (d) >> 8
#else
#define READ_2BYTES(s)     (*(uint16_t *) (s))
#define WRITE_2BYTES(s, d)  *(uint16_t *) (s) = (d)
#endif

#define SUB_SCREEN_DEPTH 0
#define MAIN_SCREEN_DEPTH 32

static INLINE uint16_t COLOR_ADD(uint16_t C1, uint16_t C2)
{
	const int RED_MASK   = 0x1F << RED_SHIFT_BITS;
	const int GREEN_MASK = 0x1F << GREEN_SHIFT_BITS;
	const int BLUE_MASK  = 0x1F;

	int rb          = (C1 & (RED_MASK | BLUE_MASK)) + (C2 & (RED_MASK | BLUE_MASK));
	int rbcarry     = rb & ((0x20 << RED_SHIFT_BITS) | (0x20 << 0));
	int g           = (C1 & (GREEN_MASK)) + (C2 & (GREEN_MASK));
	int rgbsaturate = (((g & (0x20 << GREEN_SHIFT_BITS)) | rbcarry) >> 5) * 0x1f;
	uint16_t retval = (rb & (RED_MASK | BLUE_MASK)) | (g & GREEN_MASK) | rgbsaturate;

#if GREEN_SHIFT_BITS == 6
	retval         |= (retval & 0x0400) >> 5;
#endif

	return retval;
}

#define COLOR_ADD1_2(C1, C2) \
(((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
          ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
         (((C1) & (C2) & RGB_LOW_BITS_MASK) | ALPHA_BITS_MASK))

static INLINE uint16_t COLOR_SUB(uint16_t C1, uint16_t C2)
{
	int rb1         = (C1 & (THIRD_COLOR_MASK | FIRST_COLOR_MASK)) | ((0x20 << 0) | (0x20 << RED_SHIFT_BITS));
	int rb2         = C2 & (THIRD_COLOR_MASK | FIRST_COLOR_MASK);
	int rb          = rb1 - rb2;
	int rbcarry     = rb & ((0x20 << RED_SHIFT_BITS) | (0x20 << 0));
	int g           = ((C1 & (SECOND_COLOR_MASK)) | (0x20 << GREEN_SHIFT_BITS)) - (C2 & (SECOND_COLOR_MASK));
	int rgbsaturate = (((g & (0x20 << GREEN_SHIFT_BITS)) | rbcarry) >> 5) * 0x1f;
	uint16_t retval = ((rb & (THIRD_COLOR_MASK | FIRST_COLOR_MASK)) | (g & SECOND_COLOR_MASK)) & rgbsaturate;

#if GREEN_SHIFT_BITS == 6
	retval         |= (retval & 0x0400) >> 5;
#endif

	return retval;
}

#ifdef NO_ZERO_LUT
static INLINE uint16_t COLOR_SUB1_2(uint16_t C1, uint16_t C2)
{
   // if the top bit of the color value is zero then the value is zero, otherwise its just the value
   int C = (((C1) | RGB_HI_BITS_MASKx2) - ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1;
   int r, g, b;
   DECOMPOSE_PIXEL(C, r, g, b);
   if (r & 0x10)
      r = 0;
   if (g & GREEN_HI_BIT)
      g = 0;
   if (b & 0x10)
      b = 0;
   return BUILD_PIXEL(r, g, b);
}
#else
#define COLOR_SUB1_2(C1, C2) \
GFX.ZERO [(((C1) | RGB_HI_BITS_MASKx2) - \
           ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1]
#endif

typedef void (*NormalTileRenderer)(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
typedef void (*ClippedTileRenderer)(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
typedef void (*LargePixelRenderer)(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount);
#endif
