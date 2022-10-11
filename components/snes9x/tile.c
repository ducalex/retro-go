/* This file is part of Snes9x. See LICENSE file. */

#include "snes9x.h"

#include "memmap.h"
#include "ppu.h"
#include "display.h"
#include "gfx.h"
#include "tile.h"

extern uint32_t HeadMask [4];
extern uint32_t TailMask [5];

static uint8_t ConvertTile(uint8_t* pCache, uint32_t TileAddr)
{
   uint8_t* tp = &Memory.VRAM[TileAddr];
   uint32_t* p = (uint32_t*) pCache;
   uint32_t non_zero = 0;
   uint8_t line;
   uint32_t p1;
   uint32_t p2;
   uint8_t pix;

   switch (BG.BitShift)
   {
   case 8:
      for (line = 8; line != 0; line--, tp += 2)
      {
         p1 = p2 = 0;
         if((pix = tp[0]))
         {
            p1 |= odd_high[0][pix >> 4];
            p2 |= odd_low[0][pix & 0xf];
         }
         if((pix = tp[1]))
         {
            p1 |= even_high[0][pix >> 4];
            p2 |= even_low[0][pix & 0xf];
         }
         if((pix = tp[16]))
         {
            p1 |= odd_high[1][pix >> 4];
            p2 |= odd_low[1][pix & 0xf];
         }
         if((pix = tp[17]))
         {
            p1 |= even_high[1][pix >> 4];
            p2 |= even_low[1][pix & 0xf];
         }
         if((pix = tp[32]))
         {
            p1 |= odd_high[2][pix >> 4];
            p2 |= odd_low[2][pix & 0xf];
         }
         if((pix = tp[33]))
         {
            p1 |= even_high[2][pix >> 4];
            p2 |= even_low[2][pix & 0xf];
         }
         if((pix = tp[48]))
         {
            p1 |= odd_high[3][pix >> 4];
            p2 |= odd_low[3][pix & 0xf];
         }
         if((pix = tp[49]))
         {
            p1 |= even_high[3][pix >> 4];
            p2 |= even_low[3][pix & 0xf];
         }
         *p++ = p1;
         *p++ = p2;
         non_zero |= p1 | p2;
      }
      break;
   case 4:
      for (line = 8; line != 0; line--, tp += 2)
      {
         p1 = p2 = 0;
         if((pix = tp[0]))
         {
            p1 |= odd_high[0][pix >> 4];
            p2 |= odd_low[0][pix & 0xf];
         }
         if((pix = tp[1]))
         {
            p1 |= even_high[0][pix >> 4];
            p2 |= even_low[0][pix & 0xf];
         }
         if((pix = tp[16]))
         {
            p1 |= odd_high[1][pix >> 4];
            p2 |= odd_low[1][pix & 0xf];
         }
         if((pix = tp[17]))
         {
            p1 |= even_high[1][pix >> 4];
            p2 |= even_low[1][pix & 0xf];
         }
         *p++ = p1;
         *p++ = p2;
         non_zero |= p1 | p2;
      }
      break;
   case 2:
      for (line = 8; line != 0; line--, tp += 2)
      {
         p1 = p2 = 0;
         if((pix = tp[0]))
         {
            p1 |= odd_high[0][pix >> 4];
            p2 |= odd_low[0][pix & 0xf];
         }
         if((pix = tp[1]))
         {
            p1 |= even_high[0][pix >> 4];
            p2 |= even_low[0][pix & 0xf];
         }
         *p++ = p1;
         *p++ = p2;
         non_zero |= p1 | p2;
      }
      break;
   }
   return non_zero ? (0x10|BG.Depth) : BLANK_TILE;
}

#define PLOT_PIXEL(screen, pixel) (pixel)

static INLINE void WRITE_4PIXELS16(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
#if defined(__MIPSEL) && defined(__GNUC__) && !defined(NO_ASM)
	uint16_t *Screen = (uint16_t *) GFX.S + Offset;
	uint8_t  *Depth = GFX.DB + Offset;
	uint8_t  Pixel_A, Pixel_B, Pixel_C, Pixel_D;
	uint8_t  Depth_A, Depth_B, Depth_C, Depth_D;
	uint8_t  Cond;
	uint32_t Temp;
	__asm__ __volatile__ (
		".set noreorder                        \n"
		"   lbu   %[In8A], 0(%[In8])           \n"
		"   lbu   %[In8B], 1(%[In8])           \n"
		"   lbu   %[In8C], 2(%[In8])           \n"
		"   lbu   %[In8D], 3(%[In8])           \n"
		"   lbu   %[ZA], 0(%[Z])               \n"
		"   lbu   %[ZB], 1(%[Z])               \n"
		"   lbu   %[ZC], 2(%[Z])               \n"
		"   lbu   %[ZD], 3(%[Z])               \n"
		/* If In8A is non-zero (opaque) and ZCompare > ZA, write the pixel to
		 * the screen from the palette. */
		"   sltiu %[Temp], %[In8A], 1          \n"
		"   sltu  %[Cond], %[ZCompare], %[ZA]  \n"
		"   or    %[Cond], %[Cond], %[Temp]    \n"
		/* Otherwise skip to the next pixel, B. */
		"   bne   %[Cond], $0, 2f              \n"
		/* Load the address of the palette entry (16-bit) corresponding to
		 * this pixel (partially in the delay slot). */
		"   sll   %[In8A], %[In8A], 1          \n"
		"   addu  %[Temp], %[Palette], %[In8A] \n"
		/* Load the palette entry. While that's being done, store the new
		 * depth for this pixel. Then store to the screen. */
		"   lhu   %[Temp], 0(%[Temp])          \n"
		"   sb    %[ZSet], 0(%[Z])             \n"
		"   sh    %[Temp], 0(%[Out16])         \n"
		/* Now do the same for pixel B. */
		"2: sltiu %[Temp], %[In8B], 1          \n"
		"   sltu  %[Cond], %[ZCompare], %[ZB]  \n"
		"   or    %[Cond], %[Cond], %[Temp]    \n"
		"   bne   %[Cond], $0, 3f              \n"
		"   sll   %[In8B], %[In8B], 1          \n"
		"   addu  %[Temp], %[Palette], %[In8B] \n"
		"   lhu   %[Temp], 0(%[Temp])          \n"
		"   sb    %[ZSet], 1(%[Z])             \n"
		"   sh    %[Temp], 2(%[Out16])         \n"
		/* Now do the same for pixel C. */
		"3: sltiu %[Temp], %[In8C], 1          \n"
		"   sltu  %[Cond], %[ZCompare], %[ZC]  \n"
		"   or    %[Cond], %[Cond], %[Temp]    \n"
		"   bne   %[Cond], $0, 4f              \n"
		"   sll   %[In8C], %[In8C], 1          \n"
		"   addu  %[Temp], %[Palette], %[In8C] \n"
		"   lhu   %[Temp], 0(%[Temp])          \n"
		"   sb    %[ZSet], 2(%[Z])             \n"
		"   sh    %[Temp], 4(%[Out16])         \n"
		/* Now do the same for pixel D. */
		"4: sltiu %[Temp], %[In8D], 1          \n"
		"   sltu  %[Cond], %[ZCompare], %[ZD]  \n"
		"   or    %[Cond], %[Cond], %[Temp]    \n"
		"   bne   %[Cond], $0, 5f              \n"
		"   sll   %[In8D], %[In8D], 1          \n"
		"   addu  %[Temp], %[Palette], %[In8D] \n"
		"   lhu   %[Temp], 0(%[Temp])          \n"
		"   sb    %[ZSet], 3(%[Z])             \n"
		"   sh    %[Temp], 6(%[Out16])         \n"
		"5:                                    \n"
		".set reorder                          \n"
		: /* output */  [In8A] "=&r" (Pixel_A), [In8B] "=&r" (Pixel_B), [In8C] "=&r" (Pixel_C), [In8D] "=&r" (Pixel_D), [ZA] "=&r" (Depth_A), [ZB] "=&r" (Depth_B), [ZC] "=&r" (Depth_C), [ZD] "=&r" (Depth_D), [Cond] "=&r" (Cond), [Temp] "=&r" (Temp)
		: /* input */   [Out16] "r" (Screen), [Z] "r" (Depth), [In8] "r" (Pixels), [Palette] "r" (ScreenColors), [ZCompare] "r" (GFX.Z1), [ZSet] "r" (GFX.Z2)
		: /* clobber */ "memory"
	);
#else
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.DB + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N]))
      {
         Screen [N] = ScreenColors [Pixel];
         Depth [N] = GFX.Z2;
      }
   }
#endif
}

static INLINE void WRITE_4PIXELS16_FLIPPED(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
#if defined(__MIPSEL) && defined(__GNUC__) && !defined(NO_ASM)
	uint16_t *Screen = (uint16_t *) GFX.S + Offset;
	uint8_t  *Depth = GFX.DB + Offset;
	uint8_t  Pixel_A, Pixel_B, Pixel_C, Pixel_D;
	uint8_t  Depth_A, Depth_B, Depth_C, Depth_D;
	uint8_t  Cond;
	uint32_t Temp;
	__asm__ __volatile__ (
		".set noreorder                        \n"
		"   lbu   %[In8A], 3(%[In8])           \n"
		"   lbu   %[In8B], 2(%[In8])           \n"
		"   lbu   %[In8C], 1(%[In8])           \n"
		"   lbu   %[In8D], 0(%[In8])           \n"
		"   lbu   %[ZA], 0(%[Z])               \n"
		"   lbu   %[ZB], 1(%[Z])               \n"
		"   lbu   %[ZC], 2(%[Z])               \n"
		"   lbu   %[ZD], 3(%[Z])               \n"
		/* If In8A is non-zero (opaque) and ZCompare > ZA, write the pixel to
		 * the screen from the palette. */
		"   sltiu %[Temp], %[In8A], 1          \n"
		"   sltu  %[Cond], %[ZCompare], %[ZA]  \n"
		"   or    %[Cond], %[Cond], %[Temp]    \n"
		/* Otherwise skip to the next pixel, B. */
		"   bne   %[Cond], $0, 2f              \n"
		/* Load the address of the palette entry (16-bit) corresponding to
		 * this pixel (partially in the delay slot). */
		"   sll   %[In8A], %[In8A], 1          \n"
		"   addu  %[Temp], %[Palette], %[In8A] \n"
		/* Load the palette entry. While that's being done, store the new
		 * depth for this pixel. Then store to the screen. */
		"   lhu   %[Temp], 0(%[Temp])          \n"
		"   sb    %[ZSet], 0(%[Z])             \n"
		"   sh    %[Temp], 0(%[Out16])         \n"
		/* Now do the same for pixel B. */
		"2: sltiu %[Temp], %[In8B], 1          \n"
		"   sltu  %[Cond], %[ZCompare], %[ZB]  \n"
		"   or    %[Cond], %[Cond], %[Temp]    \n"
		"   bne   %[Cond], $0, 3f              \n"
		"   sll   %[In8B], %[In8B], 1          \n"
		"   addu  %[Temp], %[Palette], %[In8B] \n"
		"   lhu   %[Temp], 0(%[Temp])          \n"
		"   sb    %[ZSet], 1(%[Z])             \n"
		"   sh    %[Temp], 2(%[Out16])         \n"
		/* Now do the same for pixel C. */
		"3: sltiu %[Temp], %[In8C], 1          \n"
		"   sltu  %[Cond], %[ZCompare], %[ZC]  \n"
		"   or    %[Cond], %[Cond], %[Temp]    \n"
		"   bne   %[Cond], $0, 4f              \n"
		"   sll   %[In8C], %[In8C], 1          \n"
		"   addu  %[Temp], %[Palette], %[In8C] \n"
		"   lhu   %[Temp], 0(%[Temp])          \n"
		"   sb    %[ZSet], 2(%[Z])             \n"
		"   sh    %[Temp], 4(%[Out16])         \n"
		/* Now do the same for pixel D. */
		"4: sltiu %[Temp], %[In8D], 1          \n"
		"   sltu  %[Cond], %[ZCompare], %[ZD]  \n"
		"   or    %[Cond], %[Cond], %[Temp]    \n"
		"   bne   %[Cond], $0, 5f              \n"
		"   sll   %[In8D], %[In8D], 1          \n"
		"   addu  %[Temp], %[Palette], %[In8D] \n"
		"   lhu   %[Temp], 0(%[Temp])          \n"
		"   sb    %[ZSet], 3(%[Z])             \n"
		"   sh    %[Temp], 6(%[Out16])         \n"
		"5:                                    \n"
		".set reorder                          \n"
		: /* output */  [In8A] "=&r" (Pixel_A), [In8B] "=&r" (Pixel_B), [In8C] "=&r" (Pixel_C), [In8D] "=&r" (Pixel_D), [ZA] "=&r" (Depth_A), [ZB] "=&r" (Depth_B), [ZC] "=&r" (Depth_C), [ZD] "=&r" (Depth_D), [Cond] "=&r" (Cond), [Temp] "=&r" (Temp)
		: /* input */   [Out16] "r" (Screen), [Z] "r" (Depth), [In8] "r" (Pixels), [Palette] "r" (ScreenColors), [ZCompare] "r" (GFX.Z1), [ZSet] "r" (GFX.Z2)
		: /* clobber */ "memory"
	);
#else
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.DB + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N]))
      {
         Screen [N] = ScreenColors [Pixel];
         Depth [N] = GFX.Z2;
      }
   }
#endif
}

static void WRITE_4PIXELS16_HALFWIDTH(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.DB + Offset;

   for (N = 0; N < 4; N += 2)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N]))
      {
         Screen [N >> 1] = ScreenColors [Pixel];
         Depth [N >> 1] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPED_HALFWIDTH(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.DB + Offset;

   for (N = 0; N < 4; N += 2)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[2 - N]))
      {
         Screen [N >> 1] = ScreenColors [Pixel];
         Depth [N >> 1] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16x2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.DB + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[N]))
      {
         Screen [N * 2] = Screen [N * 2 + 1] = ScreenColors [Pixel];
         Depth [N * 2] = Depth [N * 2 + 1] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPEDx2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.DB + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[3 - N]))
      {
         Screen [N * 2] = Screen [N * 2 + 1] = ScreenColors [Pixel];
         Depth [N * 2] = Depth [N * 2 + 1] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16x2x2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.DB + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[N]))
      {
         Screen [N * 2] = Screen [N * 2 + 1] = Screen [(GFX.RealPitch >> 1) + N * 2] = Screen [(GFX.RealPitch >> 1) + N * 2 + 1] = ScreenColors [Pixel];
         Depth [N * 2] = Depth [N * 2 + 1] = Depth [(GFX.RealPitch >> 1) + N * 2] = Depth [(GFX.RealPitch >> 1) + N * 2 + 1] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPEDx2x2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.DB + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N * 2] && (Pixel = Pixels[3 - N]))
      {
         Screen [N * 2] = Screen [N * 2 + 1] = Screen [(GFX.RealPitch >> 1) + N * 2] = Screen [(GFX.RealPitch >> 1) + N * 2 + 1] = ScreenColors [Pixel];
         Depth [N * 2] = Depth [N * 2 + 1] = Depth [(GFX.RealPitch >> 1) + N * 2] = Depth [(GFX.RealPitch >> 1) + N * 2 + 1] = GFX.Z2;
      }
   }
}

void DrawTile16(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16, WRITE_4PIXELS16_FLIPPED, 4);
}

void DrawClippedTile16(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16, WRITE_4PIXELS16_FLIPPED, 4);
}

void DrawTile16HalfWidth(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16_HALFWIDTH, WRITE_4PIXELS16_FLIPPED_HALFWIDTH, 2);
}

void DrawClippedTile16HalfWidth(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16_HALFWIDTH, WRITE_4PIXELS16_FLIPPED_HALFWIDTH, 2);
}

void DrawTile16x2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16x2, WRITE_4PIXELS16_FLIPPEDx2, 8);
}

void DrawClippedTile16x2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16x2, WRITE_4PIXELS16_FLIPPEDx2, 8);
}

void DrawTile16x2x2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16x2x2, WRITE_4PIXELS16_FLIPPEDx2x2, 8);
}

void DrawClippedTile16x2x2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16x2x2, WRITE_4PIXELS16_FLIPPEDx2x2, 8);
}

void DrawLargePixel16(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount)
{
   uint16_t pixel;
   uint16_t *sp;
   uint8_t *Depth;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   sp = (uint16_t*) GFX.S + Offset;
   Depth = GFX.DB + Offset;
   RENDER_TILE_LARGE(ScreenColors [pixel], PLOT_PIXEL);
}

void DrawLargePixel16HalfWidth(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount)
{
   uint16_t pixel;
   uint16_t *sp;
   uint8_t *Depth;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   sp = (uint16_t*) GFX.S + Offset;
   Depth = GFX.DB + Offset;
   RENDER_TILE_LARGE_HALFWIDTH(ScreenColors [pixel], PLOT_PIXEL);
}

static void WRITE_4PIXELS16_ADD(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N]))
      {
         switch (SubDepth [N])
         {
         case 0:
            Screen [N] = ScreenColors [Pixel];
            break;
         case 1:
            Screen [N] = COLOR_ADD(ScreenColors [Pixel], GFX.FixedColour);
            break;
         default:
            Screen [N] = COLOR_ADD(ScreenColors [Pixel], Screen [GFX.Delta + N]);
            break;
         }
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPED_ADD(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N]))
      {
         switch (SubDepth [N])
         {
         case 0:
            Screen [N] = ScreenColors [Pixel];
            break;
         case 1:
            Screen [N] = COLOR_ADD(ScreenColors [Pixel], GFX.FixedColour);
            break;
         default:
            Screen [N] = COLOR_ADD(ScreenColors [Pixel], Screen [GFX.Delta + N]);
            break;
         }
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_ADD1_2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N]))
      {
         switch (SubDepth [N])
         {
         case 0:
            Screen [N] = ScreenColors [Pixel];
            break;
         case 1:
            Screen [N] = COLOR_ADD(ScreenColors [Pixel], GFX.FixedColour);
            break;
         default:
            Screen [N] = (uint16_t)(COLOR_ADD1_2(ScreenColors [Pixel], Screen [GFX.Delta + N]));
            break;
         }
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPED_ADD1_2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N]))
      {
         switch (SubDepth [N])
         {
         case 0:
            Screen [N] = ScreenColors [Pixel];
            break;
         case 1:
            Screen [N] = COLOR_ADD(ScreenColors [Pixel], GFX.FixedColour);
            break;
         default:
            Screen [N] = (uint16_t)(COLOR_ADD1_2(ScreenColors [Pixel], Screen [GFX.Delta + N]));
            break;
         }
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_SUB(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N]))
      {
         switch (SubDepth [N])
         {
         case 0:
            Screen [N] = ScreenColors [Pixel];
            break;
         case 1:
            Screen [N] = (uint16_t) COLOR_SUB(ScreenColors [Pixel], GFX.FixedColour);
            break;
         default:
            Screen [N] = (uint16_t) COLOR_SUB(ScreenColors [Pixel], Screen [GFX.Delta + N]);
            break;
         }
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPED_SUB(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N]))
      {
         switch (SubDepth [N])
         {
         case 0:
            Screen [N] = ScreenColors [Pixel];
            break;
         case 1:
            Screen [N] = (uint16_t) COLOR_SUB(ScreenColors [Pixel], GFX.FixedColour);
            break;
         default:
            Screen [N] = (uint16_t) COLOR_SUB(ScreenColors [Pixel], Screen [GFX.Delta + N]);
            break;
         }
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_SUB1_2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N]))
      {
         switch (SubDepth [N])
         {
         case 0:
            Screen [N] = ScreenColors [Pixel];
            break;
         case 1:
            Screen [N] = (uint16_t) COLOR_SUB(ScreenColors [Pixel], GFX.FixedColour);
            break;
         default:
            Screen [N] = (uint16_t) COLOR_SUB1_2(ScreenColors [Pixel], Screen [GFX.Delta + N]);
            break;
         }
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPED_SUB1_2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N]))
      {
         switch (SubDepth [N])
         {
         case 0:
            Screen [N] = ScreenColors [Pixel];
            break;
         case 1:
            Screen [N] = (uint16_t) COLOR_SUB(ScreenColors [Pixel], GFX.FixedColour);
            break;
         default:
            Screen [N] = (uint16_t) COLOR_SUB1_2(ScreenColors [Pixel], Screen [GFX.Delta + N]);
            break;
         }
         Depth [N] = GFX.Z2;
      }
   }
}


void DrawTile16Add(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   uint8_t Pixel;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t* Depth = GFX.ZBuffer + Offset;
   uint8_t* SubDepth = GFX.SubZBuffer + Offset;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();

   switch (Tile & (V_FLIP | H_FLIP))
   {
   case 0:
      bp = pCache + StartLine;
      for (l = LineCount; l != 0; l--, bp += 8, Screen += GFX.PPL, Depth += GFX.PPL, SubDepth += GFX.PPL)
      {
         uint8_t N;
         for (N = 0; N < 8; N++)
         {
            if (GFX.Z1 > Depth [N] && (Pixel = bp[N]))
            {
               switch (SubDepth [N])
               {
               case 0:
                  Screen [N] = ScreenColors [Pixel];
                  break;
               case 1:
                  Screen [N] = COLOR_ADD(ScreenColors [Pixel], GFX.FixedColour);
                  break;
               default:
                  Screen [N] = COLOR_ADD(ScreenColors [Pixel], Screen [GFX.Delta + N]);
                  break;
               }
               Depth [N] = GFX.Z2;
            }
         }
      }
      break;
   case H_FLIP:
      bp = pCache + StartLine;
      for (l = LineCount; l != 0; l--, bp += 8, Screen += GFX.PPL, Depth += GFX.PPL, SubDepth += GFX.PPL)
      {
         uint8_t N;
         for (N = 0; N < 8; N++)
         {
            if (GFX.Z1 > Depth [N] && (Pixel = bp[7 - N]))
            {
               switch (SubDepth [N])
               {
               case 0:
                  Screen [N] = ScreenColors [Pixel];
                  break;
               case 1:
                  Screen [N] = COLOR_ADD(ScreenColors [Pixel], GFX.FixedColour);
                  break;
               default:
                  Screen [N] = COLOR_ADD(ScreenColors [Pixel], Screen [GFX.Delta + N]);
                  break;
               }
               Depth [N] = GFX.Z2;
            }
         }
      }
      break;
   case H_FLIP | V_FLIP:
      bp = pCache + 56 - StartLine;
      for (l = LineCount; l != 0; l--, bp -= 8, Screen += GFX.PPL, Depth += GFX.PPL, SubDepth += GFX.PPL)
      {
         uint8_t N;
         for (N = 0; N < 8; N++)
         {
            if (GFX.Z1 > Depth [N] && (Pixel = bp[7 - N]))
            {
               switch (SubDepth [N])
               {
               case 0:
                  Screen [N] = ScreenColors [Pixel];
                  break;
               case 1:
                  Screen [N] = COLOR_ADD(ScreenColors [Pixel], GFX.FixedColour);
                  break;
               default:
                  Screen [N] = COLOR_ADD(ScreenColors [Pixel], Screen [GFX.Delta + N]);
                  break;
               }
               Depth [N] = GFX.Z2;
            }
         }
      }
      break;
   case V_FLIP:
      bp = pCache + 56 - StartLine;
      for (l = LineCount; l != 0; l--, bp -= 8, Screen += GFX.PPL, Depth += GFX.PPL, SubDepth += GFX.PPL)
      {
         uint8_t N;
         for (N = 0; N < 8; N++)
         {
            if (GFX.Z1 > Depth [N] && (Pixel = bp[N]))
            {
               switch (SubDepth [N])
               {
               case 0:
                  Screen [N] = ScreenColors [Pixel];
                  break;
               case 1:
                  Screen [N] = COLOR_ADD(ScreenColors [Pixel], GFX.FixedColour);
                  break;
               default:
                  Screen [N] = COLOR_ADD(ScreenColors [Pixel], Screen [GFX.Delta + N]);
                  break;
               }
               Depth [N] = GFX.Z2;
            }
         }
      }
      break;
   default:
      break;
   }
}

void DrawClippedTile16Add(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16_ADD, WRITE_4PIXELS16_FLIPPED_ADD, 4);
}

void DrawTile16Add1_2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16_ADD1_2, WRITE_4PIXELS16_FLIPPED_ADD1_2, 4);
}

void DrawClippedTile16Add1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16_ADD1_2, WRITE_4PIXELS16_FLIPPED_ADD1_2, 4);
}

void DrawTile16Sub(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16_SUB, WRITE_4PIXELS16_FLIPPED_SUB, 4);
}

void DrawClippedTile16Sub(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width,  uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16_SUB, WRITE_4PIXELS16_FLIPPED_SUB, 4);
}

void DrawTile16Sub1_2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16_SUB1_2, WRITE_4PIXELS16_FLIPPED_SUB1_2, 4);
}

void DrawClippedTile16Sub1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16_SUB1_2, WRITE_4PIXELS16_FLIPPED_SUB1_2, 4);
}

static void WRITE_4PIXELS16_ADDF1_2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N]))
      {
         if (SubDepth [N] == 1)
            Screen [N] = (uint16_t)(COLOR_ADD1_2(ScreenColors [Pixel], GFX.FixedColour));
         else
            Screen [N] = ScreenColors [Pixel];
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPED_ADDF1_2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N]))
      {
         if (SubDepth [N] == 1)
            Screen [N] = (uint16_t)(COLOR_ADD1_2(ScreenColors [Pixel], GFX.FixedColour));
         else
            Screen [N] = ScreenColors [Pixel];
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_SUBF1_2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[N]))
      {
         if (SubDepth [N] == 1)
            Screen [N] = (uint16_t) COLOR_SUB1_2(ScreenColors [Pixel], GFX.FixedColour);
         else
            Screen [N] = ScreenColors [Pixel];
         Depth [N] = GFX.Z2;
      }
   }
}

static void WRITE_4PIXELS16_FLIPPED_SUBF1_2(int32_t Offset, uint8_t* Pixels, uint16_t* ScreenColors)
{
   uint8_t  Pixel, N;
   uint16_t* Screen = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint8_t*  SubDepth = GFX.SubZBuffer + Offset;

   for (N = 0; N < 4; N++)
   {
      if (GFX.Z1 > Depth [N] && (Pixel = Pixels[3 - N]))
      {
         if (SubDepth [N] == 1)
            Screen [N] = (uint16_t) COLOR_SUB1_2(ScreenColors [Pixel], GFX.FixedColour);
         else
            Screen [N] = ScreenColors [Pixel];
         Depth [N] = GFX.Z2;
      }
   }
}

void DrawTile16FixedAdd1_2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16_ADDF1_2, WRITE_4PIXELS16_FLIPPED_ADDF1_2, 4);
}

void DrawClippedTile16FixedAdd1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width,  uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16_ADDF1_2, WRITE_4PIXELS16_FLIPPED_ADDF1_2, 4);
}

void DrawTile16FixedSub1_2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();
   RENDER_TILE(WRITE_4PIXELS16_SUBF1_2, WRITE_4PIXELS16_FLIPPED_SUBF1_2, 4);
}

void DrawClippedTile16FixedSub1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount)
{
   uint8_t* bp;
   TILE_PREAMBLE_VARS();
   TILE_CLIP_PREAMBLE_VARS();
   RENDER_CLIPPED_TILE_VARS();
   TILE_PREAMBLE_CODE();
   TILE_CLIP_PREAMBLE_CODE();
   RENDER_CLIPPED_TILE_CODE(WRITE_4PIXELS16_SUBF1_2, WRITE_4PIXELS16_FLIPPED_SUBF1_2, 4);
}

void DrawLargePixel16Add(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount)
{
   uint16_t* sp = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint16_t pixel;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();

#define LARGE_ADD_PIXEL(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
                COLOR_ADD (p, *(s + GFX.Delta)) : \
                COLOR_ADD (p, GFX.FixedColour)) : p)

   RENDER_TILE_LARGE(ScreenColors [pixel], LARGE_ADD_PIXEL);
}

void DrawLargePixel16Add1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount)
{
   uint16_t* sp = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint16_t pixel;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();

#define LARGE_ADD_PIXEL1_2(s, p) \
((uint16_t) (Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
                COLOR_ADD1_2 (p, *(s + GFX.Delta)) : \
                COLOR_ADD (p, GFX.FixedColour)) : p))

   RENDER_TILE_LARGE(ScreenColors [pixel], LARGE_ADD_PIXEL1_2);
}

void DrawLargePixel16Sub(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount)
{
   uint16_t* sp = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint16_t pixel;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();

#define LARGE_SUB_PIXEL(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
                COLOR_SUB (p, *(s + GFX.Delta)) : \
                COLOR_SUB (p, GFX.FixedColour)) : p)

   RENDER_TILE_LARGE(ScreenColors [pixel], LARGE_SUB_PIXEL);
}

void DrawLargePixel16Sub1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount)
{
   uint16_t* sp = (uint16_t*) GFX.S + Offset;
   uint8_t*  Depth = GFX.ZBuffer + Offset;
   uint16_t pixel;
   TILE_PREAMBLE_VARS();
   TILE_PREAMBLE_CODE();

#define LARGE_SUB_PIXEL1_2(s, p) \
(Depth [z + GFX.DepthDelta] ? (Depth [z + GFX.DepthDelta] != 1 ? \
                COLOR_SUB1_2 (p, *(s + GFX.Delta)) : \
                COLOR_SUB (p, GFX.FixedColour)) : p)

   RENDER_TILE_LARGE(ScreenColors [pixel], LARGE_SUB_PIXEL1_2);
}
