/* This file is part of Snes9x. See LICENSE file. */

#include "snes9x.h"

#include "memmap.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "gfx.h"
#include "apu.h"

static const uint8_t BitShifts[8][4] =
{
   {2, 2, 2, 2}, /* 0 */
   {4, 4, 2, 0}, /* 1 */
   {4, 4, 0, 0}, /* 2 */
   {8, 4, 0, 0}, /* 3 */
   {8, 2, 0, 0}, /* 4 */
   {4, 2, 0, 0}, /* 5 */
   {4, 0, 0, 0}, /* 6 */
   {8, 0, 0, 0}  /* 7 */
};
static const uint8_t TileShifts[8][4] =
{
   {4, 4, 4, 4}, /* 0 */
   {5, 5, 4, 0}, /* 1 */
   {5, 5, 0, 0}, /* 2 */
   {6, 5, 0, 0}, /* 3 */
   {6, 4, 0, 0}, /* 4 */
   {5, 4, 0, 0}, /* 5 */
   {5, 0, 0, 0}, /* 6 */
   {6, 0, 0, 0}  /* 7 */
};
static const uint8_t PaletteShifts[8][4] =
{
   {2, 2, 2, 2}, /* 0 */
   {4, 4, 2, 0}, /* 1 */
   {4, 4, 0, 0}, /* 2 */
   {0, 4, 0, 0}, /* 3 */
   {0, 2, 0, 0}, /* 4 */
   {4, 2, 0, 0}, /* 5 */
   {4, 0, 0, 0}, /* 6 */
   {0, 0, 0, 0}  /* 7 */
};
static const uint8_t PaletteMasks[8][4] =
{
   {7, 7, 7, 7}, /* 0 */
   {7, 7, 7, 0}, /* 1 */
   {7, 7, 0, 0}, /* 2 */
   {0, 7, 0, 0}, /* 3 */
   {0, 7, 0, 0}, /* 4 */
   {7, 7, 0, 0}, /* 5 */
   {7, 0, 0, 0}, /* 6 */
   {0, 0, 0, 0}  /* 7 */
};
static const uint8_t Depths[8][4] =
{
   {TILE_2BIT, TILE_2BIT, TILE_2BIT, TILE_2BIT}, /* 0 */
   {TILE_4BIT, TILE_4BIT, TILE_2BIT, 0},         /* 1 */
   {TILE_4BIT, TILE_4BIT, 0,         0},         /* 2 */
   {TILE_8BIT, TILE_4BIT, 0,         0},         /* 3 */
   {TILE_8BIT, TILE_2BIT, 0,         0},         /* 4 */
   {TILE_4BIT, TILE_2BIT, 0,         0},         /* 5 */
   {TILE_4BIT, 0,         0,         0},         /* 6 */
   {0,         0,         0,         0}          /* 7 */
};

static NormalTileRenderer  DrawTilePtr;
static ClippedTileRenderer DrawClippedTilePtr;
static NormalTileRenderer  DrawHiResTilePtr;
static ClippedTileRenderer DrawHiResClippedTilePtr;
static LargePixelRenderer  DrawLargePixelPtr;
static uint8_t  Mode7Depths [2];

static struct {
   SLineData LineData[240];
   SLineMatrixData LineMatrixData[240];
   SOBJLines OBJLines[SNES_HEIGHT_EXTENDED];
} *LocalState;

#define LineData LocalState->LineData
#define LineMatrixData LocalState->LineMatrixData

#define CLIP_10_BIT_SIGNED(a) \
   ((a) & ((1 << 10) - 1)) + (((((a) & (1 << 13)) ^ (1 << 13)) - (1 << 13)) >> 3)

#define ON_MAIN(N) \
(GFX.r212c & (1 << (N)))

#define SUB_OR_ADD(N) \
(GFX.r2131 & (1 << (N)))

#define ON_SUB(N) \
((GFX.r2130 & 0x30) != 0x30 && \
 (GFX.r2130 & 2) && \
 (GFX.r212d & (1 << N)))

#define ANYTHING_ON_SUB \
((GFX.r2130 & 0x30) != 0x30 && \
 (GFX.r2130 & 2) && \
 (GFX.r212d & 0x1f))

#define ADD_OR_SUB_ON_ANYTHING \
(GFX.r2131 & 0x3f)

#define FIX_INTERLACE(SCREEN, DO_DEPTH, DEPTH) \
    { \
    uint32_t y; \
    if (IPPU.DoubleHeightPixels && ((PPU.BGMode != 5 && PPU.BGMode != 6) || !IPPU.Interlace)) \
    { \
        for (y = GFX.StartY; y <= GFX.EndY; y++) \
        { \
            /* memmove converted: Same malloc, non-overlapping addresses [Neb] */ \
            memcpy (SCREEN + (y * 2 + 1) * GFX.Pitch2, \
                    SCREEN + y * 2 * GFX.Pitch2, \
                    GFX.Pitch2); \
            if(DO_DEPTH){ \
                /* memmove required: Same malloc, potentially overlapping addresses [Neb] */ \
                memmove (DEPTH + (y * 2 + 1) * (GFX.PPLx2>>1), \
                         DEPTH + y * GFX.PPL, \
                         GFX.PPLx2>>1); \
            } \
        } \
    } \
    }

#define BLACK BUILD_PIXEL(0,0,0)
#define M7 19

void ComputeClipWindows(void);

void DrawTile16(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawTile16HalfWidth(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16HalfWidth(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawTile16x2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16x2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawTile16x2x2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16x2x2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawLargePixel16(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount);
void DrawLargePixel16HalfWidth(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount);
void DrawTile16Add(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16Add(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawTile16Add1_2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16Add1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawTile16FixedAdd1_2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16FixedAdd1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawTile16Sub(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16Sub(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawTile16Sub1_2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16Sub1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawTile16FixedSub1_2(uint32_t Tile, int32_t Offset, uint32_t StartLine, uint32_t LineCount);
void DrawClippedTile16FixedSub1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Width, uint32_t StartLine, uint32_t LineCount);
void DrawLargePixel16Add(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount);
void DrawLargePixel16Add1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount);
void DrawLargePixel16Sub(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount);
void DrawLargePixel16Sub1_2(uint32_t Tile, int32_t Offset, uint32_t StartPixel, uint32_t Pixels, uint32_t StartLine, uint32_t LineCount);

bool S9xInitGFX(void)
{
   LocalState = calloc(1, sizeof(*LocalState));
   if (!LocalState)
      return false;

   GFX.OBJLines = LocalState->OBJLines;
   GFX.RealPitch = GFX.Pitch2 = GFX.Pitch;
   GFX.ZPitch = GFX.Pitch;
   GFX.ZPitch >>= 1;
   GFX.Delta = (GFX.SubScreen - GFX.Screen) >> 1;
   GFX.DepthDelta = GFX.SubZBuffer - GFX.ZBuffer;

   IPPU.OBJChanged = true;

   GFX.PixSize = 1;
   DrawTilePtr = DrawTile16;
   DrawClippedTilePtr = DrawClippedTile16;
   DrawLargePixelPtr = DrawLargePixel16;
   DrawHiResTilePtr = DrawTile16;
   DrawHiResClippedTilePtr = DrawClippedTile16;
   GFX.PPL = GFX.Pitch >> 1;
   GFX.PPLx2 = GFX.Pitch;
   S9xFixColourBrightness();

#ifndef NO_ZERO_LUT
   if (!(GFX.ZERO = (uint16_t*) malloc(sizeof(uint16_t) * 0x10000)))
      return false;

   /* Build a lookup table that if the top bit of the color value is zero
    * then the value is zero, otherwise its just the value. */
   for (uint32_t r = 0; r <= MAX_RED; r++)
   {
      uint32_t r2 = r;
      if ((r2 & 0x10) == 0)
         r2 = 0;
      else
         r2 &= ~0x10;

      for (uint32_t g = 0; g <= MAX_GREEN; g++)
      {
         uint32_t g2 = g;
         if ((g2 & GREEN_HI_BIT) == 0)
            g2 = 0;
         else
            g2 &= ~GREEN_HI_BIT;
         for (uint32_t b = 0; b <= MAX_BLUE; b++)
         {
            uint32_t b2 = b;
            if ((b2 & 0x10) == 0)
               b2 = 0;
            else
               b2 &= ~0x10;

            GFX.ZERO [BUILD_PIXEL2(r, g, b)] = BUILD_PIXEL2(r2, g2, b2);
            GFX.ZERO [BUILD_PIXEL2(r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2(r2, g2, b2);
         }
      }
   }
#endif
   return true;
}

void S9xDeinitGFX(void)
{
   /* Free any memory allocated in S9xInitGFX */
   if (GFX.ZERO)
   {
      free(GFX.ZERO);
      GFX.ZERO = NULL;
   }
   if (LocalState)
   {
      free(LocalState);
      LocalState = NULL;
   }
}

void S9xStartScreenRefresh(void)
{
   if (IPPU.RenderThisFrame)
   {
      IPPU.PreviousLine = IPPU.CurrentLine = 0;

      if (PPU.BGMode == 5 || PPU.BGMode == 6)
         IPPU.Interlace = (Memory.FillRAM[0x2133] & 1);
      if (PPU.BGMode == 5 || PPU.BGMode == 6 || IPPU.Interlace)
      {
         IPPU.RenderedScreenWidth = 512;
         IPPU.DoubleWidthPixels = true;
         IPPU.HalfWidthPixels = false;

         if (IPPU.Interlace)
         {
            IPPU.RenderedScreenHeight = PPU.ScreenHeight << 1;
            IPPU.DoubleHeightPixels = true;
            GFX.Pitch2 = GFX.RealPitch;
            GFX.Pitch = GFX.RealPitch * 2;
            GFX.PPL = GFX.PPLx2 = GFX.RealPitch;
         }
         else
         {
            IPPU.RenderedScreenHeight = PPU.ScreenHeight;
            GFX.Pitch2 = GFX.Pitch = GFX.RealPitch;
            IPPU.DoubleHeightPixels = false;
            GFX.PPL = GFX.Pitch >> 1;
            GFX.PPLx2 = GFX.PPL << 1;
         }
      }
      else
      {
         IPPU.RenderedScreenWidth = 256;
         IPPU.RenderedScreenHeight = PPU.ScreenHeight;
         IPPU.DoubleWidthPixels = false;
         IPPU.HalfWidthPixels = false;
         IPPU.DoubleHeightPixels = false;
         {
            GFX.Pitch2 = GFX.Pitch = GFX.RealPitch;
            GFX.PPL = GFX.PPLx2 >> 1;
            GFX.ZPitch = GFX.RealPitch;
            GFX.ZPitch >>= 1;
         }
      }

      PPU.RecomputeClipWindows = true;
      GFX.DepthDelta = GFX.SubZBuffer - GFX.ZBuffer;
      GFX.Delta = (GFX.SubScreen - GFX.Screen) >> 1;
   }

   if (++IPPU.FrameCount == (uint32_t)Memory.ROMFramesPerSecond)
      IPPU.FrameCount = 0;
}

void RenderLine(uint8_t C)
{
   if (IPPU.RenderThisFrame)
   {
      LineData[C].BG[0].VOffset = PPU.BG[0].VOffset + 1;
      LineData[C].BG[0].HOffset = PPU.BG[0].HOffset;
      LineData[C].BG[1].VOffset = PPU.BG[1].VOffset + 1;
      LineData[C].BG[1].HOffset = PPU.BG[1].HOffset;

      if (PPU.BGMode == 7)
      {
         SLineMatrixData* p = &LineMatrixData [C];
         p->MatrixA = PPU.MatrixA;
         p->MatrixB = PPU.MatrixB;
         p->MatrixC = PPU.MatrixC;
         p->MatrixD = PPU.MatrixD;
         p->CentreX = PPU.CentreX;
         p->CentreY = PPU.CentreY;
      }
      else
      {
         if (Settings.StarfoxHack && PPU.BG[2].VOffset == 0 && PPU.BG[2].HOffset == 0xe000)
         {
            LineData[C].BG[2].VOffset = 0xe1;
            LineData[C].BG[2].HOffset = 0;
         }
         else
         {
            LineData[C].BG[2].VOffset = PPU.BG[2].VOffset + 1;
            LineData[C].BG[2].HOffset = PPU.BG[2].HOffset;
            LineData[C].BG[3].VOffset = PPU.BG[3].VOffset + 1;
            LineData[C].BG[3].HOffset = PPU.BG[3].HOffset;
         }
      }
      IPPU.CurrentLine = C + 1;
   }
   else
   {
      /* if we're not rendering this frame, we still need to update this */
      /* XXX: Check ForceBlank? Or anything else? */
      if (IPPU.OBJChanged)
         S9xSetupOBJ();
      PPU.RangeTimeOver |= GFX.OBJLines[C].RTOFlags;
   }
}

void S9xEndScreenRefresh(void)
{
   if (IPPU.RenderThisFrame)
   {
      FLUSH_REDRAW();
      if (IPPU.ColorsChanged)
      {
         uint32_t saved = PPU.CGDATA[0];
         IPPU.ColorsChanged = false;
         PPU.CGDATA[0] = saved;
      }

      GFX.Pitch = GFX.Pitch2 = GFX.RealPitch;
      GFX.PPL = GFX.PPLx2 >> 1;
   }

   if (CPU.SRAMModified)
      CPU.SRAMModified = false;
}

static INLINE void SelectTileRenderer(bool normal)
{
   if (normal)
   {
      if (IPPU.HalfWidthPixels)
      {
         DrawTilePtr = DrawTile16HalfWidth;
         DrawClippedTilePtr = DrawClippedTile16HalfWidth;
         DrawLargePixelPtr = DrawLargePixel16HalfWidth;
      }
      else
      {
         DrawTilePtr = DrawTile16;
         DrawClippedTilePtr = DrawClippedTile16;
         DrawLargePixelPtr = DrawLargePixel16;
      }
   }
   else
   {
      switch (GFX.r2131 & 0xC0)
      {
         case 0x00:
            DrawTilePtr = DrawTile16Add;
            DrawClippedTilePtr = DrawClippedTile16Add;
            DrawLargePixelPtr = DrawLargePixel16Add;
            break;
         case 0x40:
            if (GFX.r2130 & 2)
            {
               DrawTilePtr = DrawTile16Add1_2;
               DrawClippedTilePtr = DrawClippedTile16Add1_2;
            }
            else
            {
               /* Fixed colour addition */
               DrawTilePtr = DrawTile16FixedAdd1_2;
               DrawClippedTilePtr = DrawClippedTile16FixedAdd1_2;
            }
            DrawLargePixelPtr = DrawLargePixel16Add1_2;
            break;
         case 0x80:
            DrawTilePtr = DrawTile16Sub;
            DrawClippedTilePtr = DrawClippedTile16Sub;
            DrawLargePixelPtr = DrawLargePixel16Sub;
            break;
         case 0xC0:
            if (GFX.r2130 & 2)
            {
               DrawTilePtr = DrawTile16Sub1_2;
               DrawClippedTilePtr = DrawClippedTile16Sub1_2;
            }
            else
            {
               /* Fixed colour substraction */
               DrawTilePtr = DrawTile16FixedSub1_2;
               DrawClippedTilePtr = DrawClippedTile16FixedSub1_2;
            }
            DrawLargePixelPtr = DrawLargePixel16Sub1_2;
            break;
      }
   }
}

void S9xSetupOBJ(void)
{
   int32_t Height;
   uint8_t S;
   int32_t SmallWidth, SmallHeight;
   int32_t LargeWidth, LargeHeight;

   switch (PPU.OBJSizeSelect)
   {
      case 0:
         SmallWidth = SmallHeight = 8;
         LargeWidth = LargeHeight = 16;
         break;
      case 1:
         SmallWidth = SmallHeight = 8;
         LargeWidth = LargeHeight = 32;
         break;
      case 2:
         SmallWidth = SmallHeight = 8;
         LargeWidth = LargeHeight = 64;
         break;
      case 3:
         SmallWidth = SmallHeight = 16;
         LargeWidth = LargeHeight = 32;
         break;
      case 4:
         SmallWidth = SmallHeight = 16;
         LargeWidth = LargeHeight = 64;
         break;
      default:
      case 5:
         SmallWidth = SmallHeight = 32;
         LargeWidth = LargeHeight = 64;
         break;
      case 6:
         SmallWidth = 16;
         SmallHeight = 32;
         LargeWidth = 32;
         LargeHeight = 64;
         break;
      case 7:
         SmallWidth = 16;
         SmallHeight = 32;
         LargeWidth = LargeHeight = 32;
         break;
   }
   /* OK, we have three cases here. Either there's no priority, priority is
    * normal FirstSprite, or priority is FirstSprite+Y. The first two are
    * easy, the last is somewhat more ... interesting. So we split them up. */

   if (!PPU.OAMPriorityRotation || !(PPU.OAMFlip & PPU.OAMAddr & 1))
   {
      int32_t Y;
      int32_t i;
      uint8_t FirstSprite;
      /* normal case */
      uint8_t LineOBJ[SNES_HEIGHT_EXTENDED];
      memset(LineOBJ, 0, sizeof(LineOBJ));
      for (i = 0; i < SNES_HEIGHT_EXTENDED; i++)
      {
         GFX.OBJLines[i].RTOFlags = 0;
         GFX.OBJLines[i].Tiles = SNES_SPRITE_TILE_PER_LINE;
      }
      FirstSprite = PPU.FirstSprite;
      S = FirstSprite;
      do
      {
         int32_t HPos;
         if (PPU.OBJ[S].Size)
         {
            GFX.OBJWidths[S] = LargeWidth;
            Height = LargeHeight;
         }
         else
         {
            GFX.OBJWidths[S] = SmallWidth;
            Height = SmallHeight;
         }
         HPos = PPU.OBJ[S].HPos;
         if (HPos == -256)
            HPos = 256;
         if (HPos > -GFX.OBJWidths[S] && HPos <= 256)
         {
            uint8_t line, Y;
            if (HPos < 0)
               GFX.OBJVisibleTiles[S] = (GFX.OBJWidths[S] + HPos + 7) >> 3;
            else if (HPos + GFX.OBJWidths[S] >= 257)
               GFX.OBJVisibleTiles[S] = (257 - HPos + 7) >> 3;
            else
               GFX.OBJVisibleTiles[S] = GFX.OBJWidths[S] >> 3;
            for (line = 0, Y = (uint8_t)(PPU.OBJ[S].VPos & 0xff); line < Height; Y++, line++)
            {
               if (Y >= SNES_HEIGHT_EXTENDED)
                  continue;
               if (LineOBJ[Y] >= 32)
               {
                  GFX.OBJLines[Y].RTOFlags |= 0x40;
                  continue;
               }
               GFX.OBJLines[Y].Tiles -= GFX.OBJVisibleTiles[S];
               if (GFX.OBJLines[Y].Tiles < 0)
                  GFX.OBJLines[Y].RTOFlags |= 0x80;
               GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Sprite = S;
               if (PPU.OBJ[S].VFlip)
               {
                  /* Yes, Width not Height. It so happens that the
                   * sprites with H = 2 * W flip as two W * W sprites. */
                  GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Line = line ^ (GFX.OBJWidths[S] - 1);
               }
               else
                  GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Line = line;
               LineOBJ[Y]++;
            }
         }
         S = (S + 1) & 0x7F;
      } while (S != FirstSprite);

      for (Y = 0; Y < SNES_HEIGHT_EXTENDED; Y++)
         if (LineOBJ[Y] < 32) /* Add the sentinel */
            GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Sprite = -1;
      for (Y = 1; Y < SNES_HEIGHT_EXTENDED; Y++)
         GFX.OBJLines[Y].RTOFlags |= GFX.OBJLines[Y - 1].RTOFlags;
   }
   else /* evil FirstSprite+Y case */
   {
      int32_t j, Y;
      /* First, find out which sprites are on which lines */
      // uint8_t OBJOnLine[SNES_HEIGHT_EXTENDED][128];

      // We can't afford 30K on the stack. But maybe we have a better buffer to abuse?
      uint8_t (*OBJOnLine)[128] = (void *)GFX.SubScreen;

      /* We only initialise this per line, as needed. [Neb]
       * Bonus: We can quickly avoid looping if a line has no OBJs. */
      bool AnyOBJOnLine[SNES_HEIGHT_EXTENDED];
      memset(AnyOBJOnLine, false, sizeof(AnyOBJOnLine));

      for (S = 0; S < 128; S++)
      {
         int32_t HPos;
         if (PPU.OBJ[S].Size)
         {
            GFX.OBJWidths[S] = LargeWidth;
            Height = LargeHeight;
         }
         else
         {
            GFX.OBJWidths[S] = SmallWidth;
            Height = SmallHeight;
         }
         HPos = PPU.OBJ[S].HPos;
         if (HPos == -256)
            HPos = 256;
         if (HPos > -GFX.OBJWidths[S] && HPos <= 256)
         {
            uint8_t line, Y;
            if (HPos < 0)
               GFX.OBJVisibleTiles[S] = (GFX.OBJWidths[S] + HPos + 7) >> 3;
            else if (HPos + GFX.OBJWidths[S] >= 257)
               GFX.OBJVisibleTiles[S] = (257 - HPos + 7) >> 3;
            else
               GFX.OBJVisibleTiles[S] = GFX.OBJWidths[S] >> 3;
            for (line = 0, Y = (uint8_t)(PPU.OBJ[S].VPos & 0xff); line < Height; Y++, line++)
            {
               if (Y >= SNES_HEIGHT_EXTENDED)
                  continue;
               if (!AnyOBJOnLine[Y])
               {
                  memset(OBJOnLine[Y], 0, 128);
                  AnyOBJOnLine[Y] = true;
               }
               if (PPU.OBJ[S].VFlip)
               {
                  /* Yes, Width not Height. It so happens that the
                   * sprites with H=2*W flip as two WxW sprites. */
                  OBJOnLine[Y][S] = (line ^ (GFX.OBJWidths[S] - 1)) | 0x80;
               }
               else
                  OBJOnLine[Y][S] = line | 0x80;
            }
         }
      }

      /* Now go through and pull out those OBJ that are actually visible. */
      for (Y = 0; Y < SNES_HEIGHT_EXTENDED; Y++)
      {
         GFX.OBJLines[Y].RTOFlags = Y ? GFX.OBJLines[Y - 1].RTOFlags : 0;
         GFX.OBJLines[Y].Tiles = SNES_SPRITE_TILE_PER_LINE;
         j = 0;
         if (AnyOBJOnLine[Y])
         {
            uint8_t FirstSprite = (PPU.FirstSprite + Y) & 0x7F;
            S = FirstSprite;
            do
            {
               if (OBJOnLine[Y][S])
               {
                  if (j >= 32)
                  {
                     GFX.OBJLines[Y].RTOFlags |= 0x40;
                     break;
                  }
                  GFX.OBJLines[Y].Tiles -= GFX.OBJVisibleTiles[S];
                  if (GFX.OBJLines[Y].Tiles < 0)
                     GFX.OBJLines[Y].RTOFlags |= 0x80;
                  GFX.OBJLines[Y].OBJ[j].Sprite = S;
                  GFX.OBJLines[Y].OBJ[j++].Line = OBJOnLine[Y][S] & ~0x80;
               }
               S = (S + 1) & 0x7F;
            } while (S != FirstSprite);
         }
         if (j < 32)
            GFX.OBJLines[Y].OBJ[j].Sprite = -1;
      }
   }

   IPPU.OBJChanged = false;
}

static void DrawOBJS(bool OnMain, uint8_t D)
{
   struct
   {
      uint16_t Pos;
      bool     Value;
   } Windows[7];

   int32_t clipcount;
   uint32_t Y, Offset;
   BG.BitShift = 4;
   BG.TileShift = 5;
   BG.TileAddress = PPU.OBJNameBase;
   BG.StartPalette = 128;
   BG.PaletteShift = 4;
   BG.PaletteMask = 7;
   BG.Depth = TILE_4BIT;
   BG.Buffer = IPPU.TileCache;
   BG.Buffered = IPPU.TileCached;
   BG.NameSelect = PPU.OBJNameSelect;
   BG.DirectColourMode = false;
   GFX.PixSize = 1;
   clipcount = GFX.pCurrentClip->Count [4];

   if (!clipcount)
   {
      Windows[0].Pos = 0;
      Windows[0].Value = true;
      Windows[1].Pos = 256;
      Windows[1].Value = false;
      Windows[2].Pos = 1000;
      Windows[2].Value = false;
   }
   else
   {
      int32_t clip, i;
      Windows[0].Pos = 1000;
      Windows[0].Value = false;
      for (clip = 0, i = 1; clip < clipcount; clip++)
      {
         int32_t j;
         if (GFX.pCurrentClip->Right[clip][4] <= GFX.pCurrentClip->Left[clip][4])
            continue;
         for (j = 0; j < i && Windows[j].Pos < GFX.pCurrentClip->Left[clip][4]; j++);
         if (j < i && Windows[j].Pos == GFX.pCurrentClip->Left[clip][4])
            Windows[j].Value = true;
         else
         {
            /* memmove required: Overlapping addresses [Neb] */
            if (j < i)
               memmove(&Windows[j + 1], &Windows[j], sizeof(Windows[0]) * (i - j));
            Windows[j].Pos = GFX.pCurrentClip->Left[clip][4];
            Windows[j].Value = true;
            i++;
         }
         for (j = 0; j < i && Windows[j].Pos < GFX.pCurrentClip->Right[clip][4]; j++);
         if (j >= i || Windows[j].Pos != GFX.pCurrentClip->Right[clip][4])
         {
            /* memmove required: Overlapping addresses [Neb] */
            if (j < i)
               memmove(&Windows[j + 1], &Windows[j], sizeof(Windows[0]) * (i - j));
            Windows[j].Pos = GFX.pCurrentClip->Right[clip][4];
            Windows[j].Value = false;
            i++;
         }
      }
   }

   if (PPU.BGMode == 5 || PPU.BGMode == 6)
   {
      /* Bah, OnMain is never used except to determine if calling
       * SelectTileRenderer is necessary. So let's hack it to false here
       * to stop SelectTileRenderer from being called when it causes
       * problems. */
      OnMain = false;
      GFX.PixSize = 2;
      if (IPPU.DoubleHeightPixels)
      {
         DrawTilePtr = DrawTile16x2x2;
         DrawClippedTilePtr = DrawClippedTile16x2x2;
      }
      else
      {
         DrawTilePtr = DrawTile16x2;
         DrawClippedTilePtr = DrawClippedTile16x2;
      }
   }
   else
   {
      DrawTilePtr = DrawTile16;
      DrawClippedTilePtr = DrawClippedTile16;
   }

   GFX.Z1 = D + 2;

   for (Y = GFX.StartY, Offset = Y * GFX.PPL; Y <= GFX.EndY; Y++, Offset += GFX.PPL)
   {
      int32_t I = 0;
      int32_t tiles = GFX.OBJLines[Y].Tiles;
      int32_t S;
      for (S = GFX.OBJLines[Y].OBJ[I].Sprite; S >= 0 && I < 32; S = GFX.OBJLines[Y].OBJ[++I].Sprite)
      {
         int32_t TileInc = 1;
         int32_t TileLine;
         int32_t TileX;
         int32_t BaseTile;
         bool WinStat = true;
         int32_t WinIdx = 0, NextPos = -1000;
         int32_t t, O;
         int32_t X;

         tiles += GFX.OBJVisibleTiles[S];
         if (tiles <= 0)
            continue;

         if (OnMain && SUB_OR_ADD(4))
            SelectTileRenderer(!GFX.Pseudo && PPU.OBJ [S].Palette < 4);

         BaseTile = (((GFX.OBJLines[Y].OBJ[I].Line << 1) + (PPU.OBJ[S].Name & 0xf0)) & 0xf0) | (PPU.OBJ[S].Name & 0x100) | (PPU.OBJ[S].Palette << 10);
         TileX = PPU.OBJ[S].Name & 0x0f;
         TileLine = (GFX.OBJLines[Y].OBJ[I].Line & 7) * 8;

         if (PPU.OBJ[S].HFlip)
         {
            TileX = (TileX + (GFX.OBJWidths[S] >> 3) - 1) & 0x0f;
            BaseTile |= H_FLIP;
            TileInc = -1;
         }

         GFX.Z2 = (PPU.OBJ[S].Priority + 1) * 4 + D;

         X = PPU.OBJ[S].HPos;
         if (X == -256)
            X = 256;
         for (t = tiles, O = Offset + X * GFX.PixSize; X <= 256 && X < PPU.OBJ[S].HPos + GFX.OBJWidths[S]; TileX = (TileX + TileInc) & 0x0f, X += 8, O += 8 * GFX.PixSize)
         {
            if (X < -7 || --t < 0 || X == 256)
               continue;
            if (X >= NextPos)
            {
               for (; WinIdx < 7 && Windows[WinIdx].Pos <= X; WinIdx++);
               if (WinIdx == 0)
                  WinStat = false;
               else
                  WinStat = Windows[WinIdx - 1].Value;
               NextPos = (WinIdx < 7) ? Windows[WinIdx].Pos : 1000;
            }

            if (X + 8 < NextPos)
            {
               if (WinStat)
                  (*DrawTilePtr)(BaseTile | TileX, O, TileLine, 1);
            }
            else
            {
               int32_t x = X;
               while (x < X + 8)
               {
                  if (WinStat)
                     (*DrawClippedTilePtr)(BaseTile | TileX, O, x - X, NextPos - x, TileLine, 1);
                  x = NextPos;
                  for (; WinIdx < 7 && Windows[WinIdx].Pos <= x; WinIdx++);
                  if (WinIdx == 0)
                     WinStat = false;
                  else
                     WinStat = Windows[WinIdx - 1].Value;
                  NextPos = (WinIdx < 7) ? Windows[WinIdx].Pos : 1000;
                  if (NextPos > X + 8)
                     NextPos = X + 8;
               }
            }
         }
      }
   }
}

static void DrawBackgroundMosaic(uint32_t BGMode, uint32_t bg, uint8_t Z1, uint8_t Z2)
{
   uint32_t Lines;
   uint32_t OffsetMask;
   uint32_t OffsetShift;
   uint32_t Y;
   int32_t m5;
   uint32_t Tile;
   uint16_t* SC0;
   uint16_t* SC1;
   uint16_t* SC2;
   uint16_t* SC3;
   uint8_t depths [2];
   depths[0] = Z1;
   depths[1] = Z2;

   if (BGMode == 0)
      BG.StartPalette = bg << 5;
   else
      BG.StartPalette = 0;

   SC0 = (uint16_t*) &Memory.VRAM[PPU.BG[bg].SCBase << 1];

   if (PPU.BG[bg].SCSize & 1)
      SC1 = SC0 + 1024;
   else
      SC1 = SC0;

   if (((uint8_t*)SC1 - Memory.VRAM) >= 0x10000)
      SC1 -= 0x08000;


   if (PPU.BG[bg].SCSize & 2)
      SC2 = SC1 + 1024;
   else
      SC2 = SC0;

   if (((uint8_t*)SC2 - Memory.VRAM) >= 0x10000)
      SC2 -= 0x08000;


   if (PPU.BG[bg].SCSize & 1)
      SC3 = SC2 + 1024;
   else
      SC3 = SC2;

   if (((uint8_t*)SC3 - Memory.VRAM) >= 0x10000)
      SC3 -= 0x08000;

   if (BG.TileSize == 16)
   {
      OffsetMask = 0x3ff;
      OffsetShift = 4;
   }
   else
   {
      OffsetMask = 0x1ff;
      OffsetShift = 3;
   }

   m5 = (BGMode == 5 || BGMode == 6) ? 1 : 0;

   for (Y = GFX.StartY; Y <= GFX.EndY; Y += Lines)
   {
      uint16_t* b1;
      uint16_t* b2;
      uint32_t MosaicLine;
      uint32_t VirtAlign;
      uint32_t ScreenLine, Rem16;
      uint16_t* t;
      uint32_t Left = 0;
      uint32_t Right;
      uint32_t clip;
      uint32_t ClipCount, HPos, PixWidth;
      uint32_t VOffset = LineData [Y].BG[bg].VOffset;
      uint32_t HOffset = LineData [Y].BG[bg].HOffset;
      uint32_t MosaicOffset = Y % PPU.Mosaic;

      for (Lines = 1; Lines < PPU.Mosaic - MosaicOffset; Lines++)
         if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) || (HOffset != LineData [Y + Lines].BG[bg].HOffset))
            break;

      MosaicLine = VOffset + Y - MosaicOffset;

      if (Y + Lines > GFX.EndY)
         Lines = GFX.EndY + 1 - Y;

      VirtAlign = (MosaicLine & 7) << 3;
      ScreenLine = MosaicLine >> OffsetShift;
      Rem16 = MosaicLine & 15;

      if (ScreenLine & 0x20)
         b1 = SC2, b2 = SC3;
      else
         b1 = SC0, b2 = SC1;

      b1 += (ScreenLine & 0x1f) << 5;
      b2 += (ScreenLine & 0x1f) << 5;
      Right = 256 << m5;

      HOffset <<= m5;

      ClipCount = GFX.pCurrentClip->Count [bg];
      HPos = HOffset;
      PixWidth = (PPU.Mosaic << m5);

      if (!ClipCount)
         ClipCount = 1;

      for (clip = 0; clip < ClipCount; clip++)
      {
         uint32_t s, x;
         if (GFX.pCurrentClip->Count [bg])
         {
            uint32_t r;
            Left = GFX.pCurrentClip->Left [clip][bg] << m5;
            Right = GFX.pCurrentClip->Right [clip][bg] << m5;

            r    = Left % (PPU.Mosaic << m5);
            HPos = HOffset + Left;
            PixWidth = (PPU.Mosaic << m5) - r;
         }
         s = Y * GFX.PPL + Left * GFX.PixSize;
         for (x = Left; x < Right; x += PixWidth, s += (IPPU.HalfWidthPixels ? PixWidth >> 1 : PixWidth) * GFX.PixSize, HPos += PixWidth, PixWidth = (PPU.Mosaic << m5))
         {
            uint32_t Quot = (HPos & OffsetMask) >> 3;

            if (x + PixWidth >= Right)
               PixWidth = Right - x;

            if (BG.TileSize == 8 && !m5)
            {
               if (Quot > 31)
                  t = b2 + (Quot & 0x1f);
               else
                  t = b1 + Quot;
            }
            else
            {
               if (Quot > 63)
                  t = b2 + ((Quot >> 1) & 0x1f);
               else
                  t = b1 + (Quot >> 1);
            }

            Tile = READ_2BYTES(t);
            GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];

            /* Draw tile... */
            if (BG.TileSize != 8)
            {
               if (Tile & H_FLIP)
               {
                  /* Horizontal flip, but what about vertical flip ? */
                  if (Tile & V_FLIP)
                  {
                     /* Both horzontal & vertical flip */
                     if (Rem16 < 8)
                        (*DrawLargePixelPtr)(Tile + 17 - (Quot & 1), s, HPos & 7, PixWidth, VirtAlign, Lines);
                     else
                        (*DrawLargePixelPtr)(Tile + 1 - (Quot & 1), s, HPos & 7, PixWidth, VirtAlign, Lines);
                  }
                  else
                  {
                     /* Horizontal flip only */
                     if (Rem16 > 7)
                        (*DrawLargePixelPtr)(Tile + 17 - (Quot & 1), s, HPos & 7, PixWidth, VirtAlign, Lines);
                     else
                        (*DrawLargePixelPtr)(Tile + 1 - (Quot & 1), s, HPos & 7, PixWidth, VirtAlign, Lines);
                  }
               }
               else
               {
                  /* No horizontal flip, but is there a vertical flip ? */
                  if (Tile & V_FLIP)
                  {
                     /* Vertical flip only */
                     if (Rem16 < 8)
                        (*DrawLargePixelPtr)(Tile + 16 + (Quot & 1), s, HPos & 7, PixWidth, VirtAlign, Lines);
                     else
                        (*DrawLargePixelPtr)(Tile + (Quot & 1), s, HPos & 7, PixWidth, VirtAlign, Lines);
                  }
                  else
                  {
                     /* Normal unflipped */
                     if (Rem16 > 7)
                        (*DrawLargePixelPtr)(Tile + 16 + (Quot & 1), s, HPos & 7, PixWidth, VirtAlign, Lines);
                     else
                        (*DrawLargePixelPtr)(Tile + (Quot & 1), s, HPos & 7, PixWidth, VirtAlign, Lines);
                  }
               }
            }
            else
               (*DrawLargePixelPtr)(Tile + (Quot & 1) * m5, s, HPos & 7, PixWidth, VirtAlign, Lines);
         }
      }
   }
}

static void DrawBackgroundOffset(uint32_t BGMode, uint32_t bg, uint8_t Z1, uint8_t Z2)
{
   uint32_t Tile;
   uint16_t* SC0;
   uint16_t* SC1;
   uint16_t* SC2;
   uint16_t* SC3;
   uint16_t* BPS0;
   uint16_t* BPS1;
   uint16_t* BPS2;
   uint16_t* BPS3;
   uint32_t Width;
   uint32_t Y;
   int32_t OffsetEnableMask;
   static const int32_t Lines = 1;
   int32_t OffsetMask;
   int32_t OffsetShift;
   int32_t VOffsetOffset = BGMode == 4 ? 0 : 32;
   uint8_t depths [2];

   depths[0] = Z1;
   depths[1] = Z2;
   BG.StartPalette = 0;
   BPS0 = (uint16_t*) &Memory.VRAM[PPU.BG[2].SCBase << 1];

   if (PPU.BG[2].SCSize & 1)
      BPS1 = BPS0 + 1024;
   else
      BPS1 = BPS0;

   if (PPU.BG[2].SCSize & 2)
      BPS2 = BPS1 + 1024;
   else
      BPS2 = BPS0;

   if (PPU.BG[2].SCSize & 1)
      BPS3 = BPS2 + 1024;
   else
      BPS3 = BPS2;

   SC0 = (uint16_t*) &Memory.VRAM[PPU.BG[bg].SCBase << 1];

   if (PPU.BG[bg].SCSize & 1)
      SC1 = SC0 + 1024;
   else
      SC1 = SC0;

   if (((uint8_t*)SC1 - Memory.VRAM) >= 0x10000)
      SC1 -= 0x08000;


   if (PPU.BG[bg].SCSize & 2)
      SC2 = SC1 + 1024;
   else
      SC2 = SC0;

   if (((uint8_t*)SC2 - Memory.VRAM) >= 0x10000)
      SC2 -= 0x08000;


   if (PPU.BG[bg].SCSize & 1)
      SC3 = SC2 + 1024;
   else
      SC3 = SC2;

   if (((uint8_t*)SC3 - Memory.VRAM) >= 0x10000)
      SC3 -= 0x08000;

   OffsetEnableMask = 1 << (bg + 13);

   if (BG.TileSize == 16)
   {
      OffsetMask = 0x3ff;
      OffsetShift = 4;
   }
   else
   {
      OffsetMask = 0x1ff;
      OffsetShift = 3;
   }

   for (Y = GFX.StartY; Y <= GFX.EndY; Y++)
   {
      uint32_t VOff = LineData [Y].BG[2].VOffset - 1;
      uint32_t HOff = LineData [Y].BG[2].HOffset;
      int32_t VirtAlign;
      int32_t ScreenLine = VOff >> 3;
      int32_t t1;
      int32_t t2;
      uint16_t* s0;
      uint16_t* s1;
      uint16_t* s2;
      int32_t clipcount;
      int32_t clip;

      if (ScreenLine & 0x20)
         s1 = BPS2, s2 = BPS3;
      else
         s1 = BPS0, s2 = BPS1;

      s1 += (ScreenLine & 0x1f) << 5;
      s2 += (ScreenLine & 0x1f) << 5;

      if (BGMode != 4)
      {
         if ((ScreenLine & 0x1f) == 0x1f)
         {
            if (ScreenLine & 0x20)
               VOffsetOffset = BPS0 - BPS2 - 0x1f * 32;
            else
               VOffsetOffset = BPS2 - BPS0 - 0x1f * 32;
         }
         else
            VOffsetOffset = 32;
      }

      clipcount = GFX.pCurrentClip->Count [bg];
      if (!clipcount)
         clipcount = 1;

      for (clip = 0; clip < clipcount; clip++)
      {
         uint32_t Left;
         uint32_t Right;
         uint32_t VOffset;
         uint32_t HOffset;
         uint32_t Offset;
         uint32_t HPos;
         uint32_t Quot;
         uint32_t Count;
         uint16_t* t;
         uint32_t Quot2;
         uint32_t VCellOffset;
         uint32_t HCellOffset;
         uint16_t* b1;
         uint16_t* b2;
         uint32_t TotalCount = 0;
         uint32_t MaxCount = 8;
         uint32_t LineHOffset, s;
         bool left_hand_edge;

         if (!GFX.pCurrentClip->Count [bg])
         {
            Left = 0;
            Right = 256;
         }
         else
         {
            Left = GFX.pCurrentClip->Left [clip][bg];
            Right = GFX.pCurrentClip->Right [clip][bg];

            if (Right <= Left)
               continue;
         }


         LineHOffset    = LineData [Y].BG[bg].HOffset;
         s              = Left * GFX.PixSize + Y * GFX.PPL;
         left_hand_edge = (Left == 0);
         Width          = Right - Left;

         if (Left & 7)
            MaxCount = 8 - (Left & 7);

         while (Left < Right)
         {
            if (left_hand_edge)
            {
               /* The SNES offset-per-tile background mode has a
                * hardware limitation that the offsets cannot be set
                * for the tile at the left-hand edge of the screen. */
               VOffset = LineData [Y].BG[bg].VOffset;
               HOffset = LineHOffset;
               left_hand_edge = false;
            }
            else
            {
               /* All subsequent offset tile data is shifted left by one,
                * hence the - 1 below. */
               Quot2 = ((HOff + Left - 1) & OffsetMask) >> 3;

               if (Quot2 > 31)
                  s0 = s2 + (Quot2 & 0x1f);
               else
                  s0 = s1 + Quot2;

               HCellOffset = READ_2BYTES(s0);

               if (BGMode == 4)
               {
                  VOffset = LineData [Y].BG[bg].VOffset;
                  HOffset = LineHOffset;
                  if ((HCellOffset & OffsetEnableMask))
                  {
                     if (HCellOffset & 0x8000)
                        VOffset = HCellOffset + 1;
                     else
                        HOffset = HCellOffset;
                  }
               }
               else
               {
                  VCellOffset = READ_2BYTES(s0 + VOffsetOffset);
                  if ((VCellOffset & OffsetEnableMask))
                     VOffset = VCellOffset + 1;
                  else
                     VOffset = LineData [Y].BG[bg].VOffset;

                  /* MKendora Strike Gunner fix */
                  if ((HCellOffset & OffsetEnableMask))
                     HOffset = (HCellOffset & ~7) | (LineHOffset & 7);
                  else
                     HOffset = LineHOffset;
                  /* end MK */
               }
            }
            VirtAlign = ((Y + VOffset) & 7) << 3;
            ScreenLine = (VOffset + Y) >> OffsetShift;

            if (((VOffset + Y) & 15) > 7)
            {
               t1 = 16;
               t2 = 0;
            }
            else
            {
               t1 = 0;
               t2 = 16;
            }

            if (ScreenLine & 0x20)
               b1 = SC2, b2 = SC3;
            else
               b1 = SC0, b2 = SC1;

            b1 += (ScreenLine & 0x1f) << 5;
            b2 += (ScreenLine & 0x1f) << 5;

            HPos = (HOffset + Left) & OffsetMask;

            Quot = HPos >> 3;

            if (BG.TileSize == 8)
            {
               if (Quot > 31)
                  t = b2 + (Quot & 0x1f);
               else
                  t = b1 + Quot;
            }
            else
            {
               if (Quot > 63)
                  t = b2 + ((Quot >> 1) & 0x1f);
               else
                  t = b1 + (Quot >> 1);
            }

            if (MaxCount + TotalCount > Width)
               MaxCount = Width - TotalCount;

            Offset = HPos & 7;

            Count = 8 - Offset;
            if (Count > MaxCount)
               Count = MaxCount;

            s -= (IPPU.HalfWidthPixels ? Offset >> 1 : Offset) * GFX.PixSize;
            Tile = READ_2BYTES(t);
            GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];

            if (BG.TileSize == 8)
               (*DrawClippedTilePtr)(Tile, s, Offset, Count, VirtAlign, Lines);
            else
            {
               if (!(Tile & (V_FLIP | H_FLIP))) /* Normal, unflipped */
                  (*DrawClippedTilePtr)(Tile + t1 + (Quot & 1), s, Offset, Count, VirtAlign, Lines);
               else if (Tile & H_FLIP)
               {
                  if (Tile & V_FLIP) /* H & V flip */
                     (*DrawClippedTilePtr)(Tile + t2 + 1 - (Quot & 1), s, Offset, Count, VirtAlign, Lines);
                  else /* H flip only */
                     (*DrawClippedTilePtr)(Tile + t1 + 1 - (Quot & 1), s, Offset, Count, VirtAlign, Lines);
               }
               else /* V flip only */
                  (*DrawClippedTilePtr)(Tile + t2 + (Quot & 1), s, Offset, Count, VirtAlign, Lines);
            }

            Left += Count;
            TotalCount += Count;
            s += (IPPU.HalfWidthPixels ? (Offset + Count) >> 1 : (Offset + Count)) * GFX.PixSize;
            MaxCount = 8;
         }
      }
   }
}

static void DrawBackgroundMode5(uint32_t bg, uint8_t Z1, uint8_t Z2)
{
   uint32_t Tile;
   uint16_t* SC0;
   uint16_t* SC1;
   uint16_t* SC2;
   uint16_t* SC3;
   uint32_t Width;
   int32_t Lines;
   int32_t VOffsetShift;
   int32_t Y;
   int32_t endy;
   uint8_t depths[2];

   if (IPPU.Interlace)
   {
      GFX.Pitch = GFX.RealPitch;
      GFX.PPL = GFX.PPLx2 >> 1;
   }

   GFX.PixSize = 1;
   depths[0]       = Z1;
   depths[1]       = Z2;
   BG.StartPalette = 0;

   SC0 = (uint16_t*) &Memory.VRAM[PPU.BG[bg].SCBase << 1];

   if ((PPU.BG[bg].SCSize & 1))
      SC1 = SC0 + 1024;
   else
      SC1 = SC0;

   if ((SC1 - (uint16_t*)Memory.VRAM) > 0x10000)
      SC1 = (uint16_t*)&Memory.VRAM[(((uint8_t*)SC1) - Memory.VRAM) % 0x10000];

   if ((PPU.BG[bg].SCSize & 2))
      SC2 = SC1 + 1024;
   else
      SC2 = SC0;

   if (((uint8_t*)SC2 - Memory.VRAM) >= 0x10000)
      SC2 -= 0x08000;

   if ((PPU.BG[bg].SCSize & 1))
      SC3 = SC2 + 1024;
   else
      SC3 = SC2;

   if (((uint8_t*)SC3 - Memory.VRAM) >= 0x10000)
      SC3 -= 0x08000;

   if (BG.TileSize == 16)
      VOffsetShift = 4;
   else
      VOffsetShift = 3;

   endy = IPPU.Interlace ? 1 + (GFX.EndY << 1) : GFX.EndY;

   for (Y = IPPU.Interlace ? GFX.StartY << 1 : GFX.StartY; Y <= endy; Y += Lines)
   {
      int32_t ScreenLine;
      int32_t t1;
      int32_t t2;
      uint16_t* b1;
      uint16_t* b2;
      int32_t clipcount;
      int32_t clip;
      int32_t y = IPPU.Interlace ? (Y >> 1) : Y;
      uint32_t VOffset = LineData [y].BG[bg].VOffset;
      uint32_t HOffset = LineData [y].BG[bg].HOffset;
      int32_t VirtAlign = (Y + VOffset) & 7;

      for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
         if ((VOffset != LineData [y + Lines].BG[bg].VOffset) || (HOffset != LineData [y + Lines].BG[bg].HOffset))
            break;

      HOffset <<= 1;
      if (Y + Lines > endy)
         Lines = endy + 1 - Y;
      VirtAlign <<= 3;
      ScreenLine = (VOffset + Y) >> VOffsetShift;

      if (((VOffset + Y) & 15) > 7)
      {
         t1 = 16;
         t2 = 0;
      }
      else
      {
         t1 = 0;
         t2 = 16;
      }

      if (ScreenLine & 0x20)
         b1 = SC2, b2 = SC3;
      else
         b1 = SC0, b2 = SC1;

      b1 += (ScreenLine & 0x1f) << 5;
      b2 += (ScreenLine & 0x1f) << 5;

      clipcount = GFX.pCurrentClip->Count [bg];
      if (!clipcount)
         clipcount = 1;

      for (clip = 0; clip < clipcount; clip++)
      {
         int32_t C;
         uint32_t Count = 0;
         uint16_t* t;
         int32_t Left;
         int32_t Right;
         uint32_t s;
         int32_t Middle;
         uint32_t HPos, Quot;

         if (!GFX.pCurrentClip->Count [bg])
         {
            Left = 0;
            Right = 512;
         }
         else
         {
            Left = GFX.pCurrentClip->Left [clip][bg] * 2;
            Right = GFX.pCurrentClip->Right [clip][bg] * 2;

            if (Right <= Left)
               continue;
         }

         s = (IPPU.HalfWidthPixels ? Left >> 1 : Left) * GFX.PixSize + Y * GFX.PPL;
         HPos = (HOffset + Left * GFX.PixSize) & 0x3ff;
         Quot = HPos >> 3;

         if (Quot > 63)
            t = b2 + ((Quot >> 1) & 0x1f);
         else
            t = b1 + (Quot >> 1);

         Width = Right - Left;
         /* Left hand edge clipped tile */
         if (HPos & 7)
         {
            int32_t Offset = (HPos & 7);
            Count = 8 - Offset;
            if (Count > Width)
               Count = Width;
            s -= (IPPU.HalfWidthPixels ? Offset >> 1 : Offset);
            Tile = READ_2BYTES(t);
            GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];

            if (BG.TileSize == 8)
            {
               if (!(Tile & H_FLIP)) /* Normal, unflipped */
                  (*DrawHiResClippedTilePtr)(Tile + (Quot & 1), s, Offset, Count, VirtAlign, Lines);
               else /* H flip */
                  (*DrawHiResClippedTilePtr)(Tile + 1 - (Quot & 1), s, Offset, Count, VirtAlign, Lines);
            }
            else
            {
               if (!(Tile & (V_FLIP | H_FLIP))) /* Normal, unflipped */
                  (*DrawHiResClippedTilePtr)(Tile + t1 + (Quot & 1), s, Offset, Count, VirtAlign, Lines);
               else if (Tile & H_FLIP)
               {
                  if (Tile & V_FLIP) /* H & V flip */
                     (*DrawHiResClippedTilePtr)(Tile + t2 + 1 - (Quot & 1), s, Offset, Count, VirtAlign, Lines);
                  else /* H flip only */
                     (*DrawHiResClippedTilePtr)(Tile + t1 + 1 - (Quot & 1), s, Offset, Count, VirtAlign, Lines);
               }
               else /* V flip only */
                  (*DrawHiResClippedTilePtr)(Tile + t2 + (Quot & 1), s, Offset, Count, VirtAlign, Lines);
            }

            t += Quot & 1;
            if (Quot == 63)
               t = b2;
            else if (Quot == 127)
               t = b1;
            Quot++;
            s += (IPPU.HalfWidthPixels ? 4 : 8);
         }

         /* Middle, unclipped tiles */
         Count = Width - Count;
         Middle = Count >> 3;
         Count &= 7;

         for (C = Middle; C > 0; s += (IPPU.HalfWidthPixels ? 4 : 8), Quot++, C--)
         {
            Tile = READ_2BYTES(t);
            GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
            if (BG.TileSize == 8)
            {
               if (!(Tile & H_FLIP)) /* Normal, unflipped */
                  (*DrawHiResTilePtr)(Tile + (Quot & 1), s, VirtAlign, Lines);
               else /* H flip */
                  (*DrawHiResTilePtr)(Tile + 1 - (Quot & 1), s, VirtAlign, Lines);
            }
            else
            {
               if (!(Tile & (V_FLIP | H_FLIP))) /* Normal, unflipped */
                  (*DrawHiResTilePtr)(Tile + t1 + (Quot & 1), s, VirtAlign, Lines);
               else if (Tile & H_FLIP)
               {
                  if (Tile & V_FLIP) /* H & V flip */
                     (*DrawHiResTilePtr)(Tile + t2 + 1 - (Quot & 1), s, VirtAlign, Lines);
                  else /* H flip only */
                     (*DrawHiResTilePtr)(Tile + t1 + 1 - (Quot & 1), s, VirtAlign, Lines);
               }
               else /* V flip only */
                  (*DrawHiResTilePtr)(Tile + t2 + (Quot & 1), s, VirtAlign, Lines);
            }

            t += Quot & 1;
            if (Quot == 63)
               t = b2;
            else if (Quot == 127)
               t = b1;
         }

         /* Right-hand edge clipped tiles */
         if (Count)
         {
            Tile = READ_2BYTES(t);
            GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];
            if (BG.TileSize == 8)
            {
               if (!(Tile & H_FLIP)) /* Normal, unflipped */
                  (*DrawHiResClippedTilePtr)(Tile + (Quot & 1), s, 0, Count, VirtAlign, Lines);
               else /* H flip */
                  (*DrawHiResClippedTilePtr)(Tile + 1 - (Quot & 1), s, 0, Count, VirtAlign, Lines);
            }
            else
            {
               if (!(Tile & (V_FLIP | H_FLIP))) /* Normal, unflipped */
                  (*DrawHiResClippedTilePtr)(Tile + t1 + (Quot & 1), s, 0, Count, VirtAlign, Lines);
               else if (Tile & H_FLIP)
               {
                  if (Tile & V_FLIP) /* H & V flip */
                     (*DrawHiResClippedTilePtr)(Tile + t2 + 1 - (Quot & 1), s, 0, Count, VirtAlign, Lines);
                  else /* H flip only */
                     (*DrawHiResClippedTilePtr)(Tile + t1 + 1 - (Quot & 1), s, 0, Count, VirtAlign, Lines);
               }
               else /* V flip only */
                  (*DrawHiResClippedTilePtr)(Tile + t2 + (Quot & 1), s, 0, Count, VirtAlign, Lines);
            }
         }
      }
   }
   GFX.Pitch = IPPU.DoubleHeightPixels ? GFX.RealPitch * 2 : GFX.RealPitch;
   GFX.PPL = IPPU.DoubleHeightPixels ? GFX.PPLx2 : (GFX.PPLx2 >> 1);
}

static void DrawBackground(uint32_t BGMode, uint32_t bg, uint8_t Z1, uint8_t Z2)
{
   uint32_t Tile;
   uint16_t* SC0;
   uint16_t* SC1;
   uint16_t* SC2;
   uint16_t* SC3;
   uint32_t Width;
   uint8_t depths[2];
   uint32_t Y;
   int32_t Lines;
   int32_t OffsetMask;
   int32_t OffsetShift;
   GFX.PixSize = 1;

   BG.TileSize = 8 << (PPU.BG[bg].BGSize);
   BG.BitShift = BitShifts[BGMode][bg];
   BG.TileShift = TileShifts[BGMode][bg];
   BG.TileAddress = PPU.BG[bg].NameBase << 1;
   BG.NameSelect = 0;
   BG.Depth = Depths [BGMode][bg];
   BG.Buffer = IPPU.TileCache;
   BG.Buffered = IPPU.TileCached;
   BG.PaletteShift = PaletteShifts[BGMode][bg];
   BG.PaletteMask = PaletteMasks[BGMode][bg];
   BG.DirectColourMode = (BGMode == 3 || BGMode == 4) && bg == 0 && (GFX.r2130 & 1);

   if (PPU.BGMosaic [bg] && PPU.Mosaic > 1)
   {
      DrawBackgroundMosaic(BGMode, bg, Z1, Z2);
      return;
   }
   switch (BGMode)
   {
      case 2:
      case 4: /* Used by Puzzle Bobble */
         DrawBackgroundOffset(BGMode, bg, Z1, Z2);
         return;

      case 5:
      case 6: /* XXX: is also offset per tile. */
         DrawBackgroundMode5(bg, Z1, Z2);
         return;
   }

   depths [0] = Z1;
   depths [1] = Z2;

   if (BGMode == 0)
      BG.StartPalette = bg << 5;
   else
      BG.StartPalette = 0;

   SC0 = (uint16_t*) &Memory.VRAM[PPU.BG[bg].SCBase << 1];

   if (PPU.BG[bg].SCSize & 1)
      SC1 = SC0 + 1024;
   else
      SC1 = SC0;

   if (SC1 >= (uint16_t*)(Memory.VRAM + 0x10000))
      SC1 = (uint16_t*)&Memory.VRAM[((uint8_t*)SC1 - &Memory.VRAM[0]) % 0x10000];

   if (PPU.BG[bg].SCSize & 2)
      SC2 = SC1 + 1024;
   else
      SC2 = SC0;

   if (((uint8_t*)SC2 - Memory.VRAM) >= 0x10000)
      SC2 -= 0x08000;

   if (PPU.BG[bg].SCSize & 1)
      SC3 = SC2 + 1024;
   else
      SC3 = SC2;

   if (((uint8_t*)SC3 - Memory.VRAM) >= 0x10000)
      SC3 -= 0x08000;

   if (BG.TileSize == 16)
   {
      OffsetMask = 0x3ff;
      OffsetShift = 4;
   }
   else
   {
      OffsetMask = 0x1ff;
      OffsetShift = 3;
   }

   for (Y = GFX.StartY; Y <= GFX.EndY; Y += Lines)
   {
      uint32_t ScreenLine;
      uint32_t t1;
      uint32_t t2;
      uint16_t* b1;
      uint16_t* b2;
      int32_t clip;
      int32_t clipcount;
      uint32_t VOffset = LineData [Y].BG[bg].VOffset;
      uint32_t HOffset = LineData [Y].BG[bg].HOffset;
      int32_t VirtAlign = (Y + VOffset) & 7;

      for (Lines = 1; Lines < 8 - VirtAlign; Lines++)
         if ((VOffset != LineData [Y + Lines].BG[bg].VOffset) || (HOffset != LineData [Y + Lines].BG[bg].HOffset))
            break;

      if (Y + Lines > GFX.EndY)
         Lines = GFX.EndY + 1 - Y;

      VirtAlign <<= 3;
      ScreenLine = (VOffset + Y) >> OffsetShift;

      if (((VOffset + Y) & 15) > 7)
      {
         t1 = 16;
         t2 = 0;
      }
      else
      {
         t1 = 0;
         t2 = 16;
      }

      if (ScreenLine & 0x20)
         b1 = SC2, b2 = SC3;
      else
         b1 = SC0, b2 = SC1;

      b1 += (ScreenLine & 0x1f) << 5;
      b2 += (ScreenLine & 0x1f) << 5;

      clipcount = GFX.pCurrentClip->Count [bg];
      if (!clipcount)
         clipcount = 1;
      for (clip = 0; clip < clipcount; clip++)
      {
         uint32_t Left;
         uint32_t Right;
         uint32_t Count = 0;
         uint16_t* t;
         uint32_t s, HPos, Quot;
         int32_t C;
         int32_t Middle;

         if (!GFX.pCurrentClip->Count [bg])
         {
            Left = 0;
            Right = 256;
         }
         else
         {
            Left = GFX.pCurrentClip->Left [clip][bg];
            Right = GFX.pCurrentClip->Right [clip][bg];

            if (Right <= Left)
               continue;
         }

         s = Left * GFX.PixSize + Y * GFX.PPL;
         HPos = (HOffset + Left) & OffsetMask;
         Quot = HPos >> 3;

         if (BG.TileSize == 8)
         {
            if (Quot > 31)
               t = b2 + (Quot & 0x1f);
            else
               t = b1 + Quot;
         }
         else
         {
            if (Quot > 63)
               t = b2 + ((Quot >> 1) & 0x1f);
            else
               t = b1 + (Quot >> 1);
         }

         Width = Right - Left;
         /* Left hand edge clipped tile */
         if (HPos & 7)
         {
            uint32_t Offset = (HPos & 7);
            Count = 8 - Offset;
            if (Count > Width)
               Count = Width;
            s -= Offset * GFX.PixSize;
            Tile = READ_2BYTES(t);
            GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];

            if (BG.TileSize == 8)
               (*DrawClippedTilePtr)(Tile, s, Offset, Count, VirtAlign, Lines);
            else if (!(Tile & (V_FLIP | H_FLIP))) /* Normal, unflipped */
               (*DrawClippedTilePtr)(Tile + t1 + (Quot & 1), s, Offset, Count, VirtAlign, Lines);
            else if (Tile & H_FLIP)
            {
               if (Tile & V_FLIP) /* H & V flip */
                  (*DrawClippedTilePtr)(Tile + t2 + 1 - (Quot & 1), s, Offset, Count, VirtAlign, Lines);
               else /* H flip only */
                  (*DrawClippedTilePtr)(Tile + t1 + 1 - (Quot & 1), s, Offset, Count, VirtAlign, Lines);
            }
            else /* V flip only */
               (*DrawClippedTilePtr)(Tile + t2 + (Quot & 1), s, Offset, Count, VirtAlign, Lines);

            if (BG.TileSize == 8)
            {
               t++;
               if (Quot == 31)
                  t = b2;
               else if (Quot == 63)
                  t = b1;
            }
            else
            {
               t += Quot & 1;
               if (Quot == 63)
                  t = b2;
               else if (Quot == 127)
                  t = b1;
            }
            Quot++;
            s += (IPPU.HalfWidthPixels ? 4 : 8) * GFX.PixSize;
         }

         /* Middle, unclipped tiles */
         Count = Width - Count;
         Middle = Count >> 3;
         Count &= 7;

         for (C = Middle; C > 0; s += (IPPU.HalfWidthPixels ? 4 : 8) * GFX.PixSize, Quot++, C--)
         {
            Tile = READ_2BYTES(t);
            GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];

            if (BG.TileSize != 8)
            {
               if (Tile & H_FLIP) /* Horizontal flip, but what about vertical flip? */
               {
                  if (Tile & V_FLIP) /* Both horzontal & vertical flip */
                     (*DrawTilePtr)(Tile + t2 + 1 - (Quot & 1), s, VirtAlign, Lines);
                  else /* Horizontal flip only */
                     (*DrawTilePtr)(Tile + t1 + 1 - (Quot & 1), s, VirtAlign, Lines);
               }
               else if (Tile & V_FLIP) /* Vertical flip only */
                  (*DrawTilePtr)(Tile + t2 + (Quot & 1), s, VirtAlign, Lines);
               else /* Normal unflipped */
                  (*DrawTilePtr)(Tile + t1 + (Quot & 1), s, VirtAlign, Lines);
            }
            else
               (*DrawTilePtr)(Tile, s, VirtAlign, Lines);

            if (BG.TileSize == 8)
            {
               t++;
               if (Quot == 31)
                  t = b2;
               else if (Quot == 63)
                  t = b1;
            }
            else
            {
               t += Quot & 1;
               if (Quot == 63)
                  t = b2;
               else if (Quot == 127)
                  t = b1;
            }
         }
         /* Right-hand edge clipped tiles */
         if (Count)
         {
            Tile = READ_2BYTES(t);
            GFX.Z1 = GFX.Z2 = depths [(Tile & 0x2000) >> 13];

            if (BG.TileSize == 8)
               (*DrawClippedTilePtr)(Tile, s, 0, Count, VirtAlign, Lines);
            else
            {
               if (!(Tile & (V_FLIP | H_FLIP))) /* Normal, unflipped */
                  (*DrawClippedTilePtr)(Tile + t1 + (Quot & 1), s, 0, Count, VirtAlign, Lines);
               else if (Tile & H_FLIP)
               {
                  if (Tile & V_FLIP) /* H & V flip */
                     (*DrawClippedTilePtr)(Tile + t2 + 1 - (Quot & 1), s, 0, Count, VirtAlign, Lines);
                  else /* H flip only */
                     (*DrawClippedTilePtr)(Tile + t1 + 1 - (Quot & 1), s, 0, Count, VirtAlign, Lines);
               }
               else /* V flip only */
                  (*DrawClippedTilePtr)(Tile + t2 + (Quot & 1), s, 0, Count, VirtAlign, Lines);
            }
         }
      }
   }
}

#define RENDER_BACKGROUND_MODE7(TYPE,FUNC) \
    uint32_t clip; \
    int32_t aa, cc; \
    int32_t dir; \
    int32_t startx, endx; \
    uint32_t Left = 0; \
    uint32_t Right = 256; \
    uint32_t ClipCount; \
    uint16_t* ScreenColors; \
    uint8_t* VRAM1; \
    uint32_t Line; \
    uint8_t* Depth; \
    SLineMatrixData* l; \
    (void)ScreenColors; \
\
    VRAM1 = Memory.VRAM + 1; \
    if (GFX.r2130 & 1) \
        ScreenColors = IPPU.DirectColors; \
    else \
        ScreenColors = IPPU.ScreenColors; \
\
    ClipCount = GFX.pCurrentClip->Count [bg]; \
\
    if (!ClipCount) \
   ClipCount = 1; \
\
    Screen += GFX.StartY * GFX.Pitch; \
    Depth = GFX.DB + GFX.StartY * GFX.PPL; \
    l = &LineMatrixData [GFX.StartY]; \
\
    for (Line = GFX.StartY; Line <= GFX.EndY; Line++, Screen += GFX.Pitch, Depth += GFX.PPL, l++) \
    { \
    int32_t yy; \
    int32_t BB,DD; \
\
    int32_t HOffset = ((int32_t) LineData [Line].BG[0].HOffset << M7) >> M7; \
    int32_t VOffset = ((int32_t) LineData [Line].BG[0].VOffset << M7) >> M7; \
\
    int32_t CentreX = ((int32_t) l->CentreX << M7) >> M7; \
    int32_t CentreY = ((int32_t) l->CentreY << M7) >> M7; \
\
    if (PPU.Mode7VFlip) \
       yy = 255 - (int32_t) Line; \
    else \
       yy = Line; \
\
    yy += CLIP_10_BIT_SIGNED(VOffset - CentreY); \
\
    BB = l->MatrixB * yy + (CentreX << 8); \
    DD = l->MatrixD * yy + (CentreY << 8); \
\
    for (clip = 0; clip < ClipCount; clip++) \
    { \
       TYPE *p; \
       uint8_t *d; \
       int32_t xx, AA, CC; \
       if (GFX.pCurrentClip->Count [bg]) \
       { \
      Left = GFX.pCurrentClip->Left [clip][bg]; \
      Right = GFX.pCurrentClip->Right [clip][bg]; \
      if (Right <= Left) \
          continue; \
       } \
       p = (TYPE *) Screen + Left; \
       d = Depth + Left; \
\
       if (PPU.Mode7HFlip) \
       { \
      startx = Right - 1; \
      endx = Left - 1; \
      dir = -1; \
      aa = -l->MatrixA; \
      cc = -l->MatrixC; \
       } \
       else \
       { \
      startx = Left; \
      endx = Right; \
      dir = 1; \
      aa = l->MatrixA; \
      cc = l->MatrixC; \
       } \
\
   xx = startx + CLIP_10_BIT_SIGNED(HOffset - CentreX); \
       AA = l->MatrixA * xx; \
       CC = l->MatrixC * xx; \
\
       if (!PPU.Mode7Repeat) \
       { \
      int32_t x; \
      for (x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
      { \
          int32_t X = ((AA + BB) >> 8) & 0x3ff; \
          int32_t Y = ((CC + DD) >> 8) & 0x3ff; \
          uint8_t *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
          uint32_t b = TileData[((Y & 7) << 4) + ((X & 7) << 1)]; \
          GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
          if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
          { \
         *p = (FUNC); \
         *d = GFX.Z1; \
          } \
      } \
       } \
       else \
       { \
      int32_t x; \
      for (x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
      { \
          int32_t X = ((AA + BB) >> 8); \
          int32_t Y = ((CC + DD) >> 8); \
\
          if (((X | Y) & ~0x3ff) == 0) \
          { \
         uint8_t *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
         uint32_t b = TileData[((Y & 7) << 4) + ((X & 7) << 1)]; \
         GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
         if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
         { \
             *p = (FUNC); \
             *d = GFX.Z1; \
         } \
          } \
          else \
          { \
         if (PPU.Mode7Repeat == 3) \
         { \
             uint32_t b; \
             X = (x + HOffset) & 7; \
             Y = (yy + CentreY) & 7; \
             b = VRAM1[((Y & 7) << 4) + ((X & 7) << 1)]; \
             GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
             if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
             { \
            *p = (FUNC); \
            *d = GFX.Z1; \
             } \
         } \
          } \
      } \
       } \
   } \
    }

static void DrawBGMode7Background(uint8_t* Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7(uint8_t, (uint8_t) (b & GFX.Mode7Mask))
}

static void DrawBGMode7Background16(uint8_t* Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7(uint16_t, ScreenColors [b & GFX.Mode7Mask]);
}

static void DrawBGMode7Background16Add(uint8_t * Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7(uint16_t, *(d + GFX.DepthDelta) ? (*(d + GFX.DepthDelta) != 1 ? COLOR_ADD(ScreenColors[b & GFX.Mode7Mask], p[GFX.Delta]) : COLOR_ADD(ScreenColors[b & GFX.Mode7Mask], GFX.FixedColour)) : ScreenColors[b & GFX.Mode7Mask]);
}

static void DrawBGMode7Background16Add1_2(uint8_t * Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7(uint16_t, *(d + GFX.DepthDelta) ? (*(d + GFX.DepthDelta) != 1 ? COLOR_ADD1_2(ScreenColors[b & GFX.Mode7Mask], p[GFX.Delta]) : COLOR_ADD(ScreenColors[b & GFX.Mode7Mask], GFX.FixedColour)) : ScreenColors[b & GFX.Mode7Mask]);
}

static void DrawBGMode7Background16Sub(uint8_t * Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7(uint16_t, *(d + GFX.DepthDelta) ? (*(d + GFX.DepthDelta) != 1 ? COLOR_SUB(ScreenColors[b & GFX.Mode7Mask], p[GFX.Delta]) : COLOR_SUB(ScreenColors[b & GFX.Mode7Mask], GFX.FixedColour)) : ScreenColors[b & GFX.Mode7Mask]);
}

static void DrawBGMode7Background16Sub1_2(uint8_t * Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7(uint16_t, *(d + GFX.DepthDelta) ? (*(d + GFX.DepthDelta) != 1 ? COLOR_SUB1_2(ScreenColors[b & GFX.Mode7Mask], p[GFX.Delta]) : COLOR_SUB(ScreenColors[b & GFX.Mode7Mask], GFX.FixedColour)) : ScreenColors[b & GFX.Mode7Mask]);
}

#define RENDER_BACKGROUND_MODE7_i(TYPE,FUNC,COLORFUNC) \
    int32_t aa, cc; \
    uint32_t clip; \
    int32_t dir; \
    int32_t startx, endx; \
    uint32_t ClipCount; \
    uint16_t *ScreenColors; \
    uint8_t *Depth; \
    uint32_t Line; \
    SLineMatrixData *l; \
    uint32_t Left = 0; \
    uint32_t Right = 256; \
    bool allowSimpleCase = false; \
    uint8_t *VRAM1 = Memory.VRAM + 1; \
    uint32_t b; \
    if (GFX.r2130 & 1) \
        ScreenColors = IPPU.DirectColors; \
    else \
        ScreenColors = IPPU.ScreenColors; \
    \
    ClipCount = GFX.pCurrentClip->Count [bg]; \
    \
    if (!ClipCount) \
        ClipCount = 1; \
    \
    Screen += GFX.StartY * GFX.Pitch; \
    Depth = GFX.DB + GFX.StartY * GFX.PPL; \
    l = &LineMatrixData [GFX.StartY]; \
    if (!l->MatrixB && !l->MatrixC && (l->MatrixA == 0x0100) && (l->MatrixD == 0x0100) \
        && !LineMatrixData[GFX.EndY].MatrixB && !LineMatrixData[GFX.EndY].MatrixC \
        && (LineMatrixData[GFX.EndY].MatrixA == 0x0100) && (LineMatrixData[GFX.EndY].MatrixD == 0x0100) \
        ) \
        allowSimpleCase = true;  \
    \
    for (Line = GFX.StartY; Line <= GFX.EndY; Line++, Screen += GFX.Pitch, Depth += GFX.PPL, l++) \
    { \
        bool simpleCase = false; \
        int32_t BB; \
        int32_t DD; \
        int32_t yy; \
        \
        int32_t HOffset = ((int32_t) LineData [Line].BG[0].HOffset << M7) >> M7; \
        int32_t VOffset = ((int32_t) LineData [Line].BG[0].VOffset << M7) >> M7; \
        \
        int32_t CentreX = ((int32_t) l->CentreX << M7) >> M7; \
        int32_t CentreY = ((int32_t) l->CentreY << M7) >> M7; \
        \
        if (PPU.Mode7VFlip) \
            yy = 255 - (int32_t) Line; \
        else \
            yy = Line; \
        \
   \
       yy += CLIP_10_BIT_SIGNED(VOffset - CentreY); \
        /* Make a special case for the identity matrix, since it's a common case and */ \
        /* can be done much more quickly without special effects */ \
        if (allowSimpleCase && !l->MatrixB && !l->MatrixC && (l->MatrixA == 0x0100) && (l->MatrixD == 0x0100)) \
        { \
            BB = CentreX << 8; \
            DD = (yy + CentreY) << 8; \
            simpleCase = true; \
        } \
        else \
        { \
            BB = l->MatrixB * yy + (CentreX << 8); \
            DD = l->MatrixD * yy + (CentreY << 8); \
        } \
        \
        for (clip = 0; clip < ClipCount; clip++) \
        { \
           TYPE *p; \
           uint8_t *d; \
            int32_t xx; \
            int32_t AA, CC = 0; \
            if (GFX.pCurrentClip->Count [bg]) \
            { \
                Left = GFX.pCurrentClip->Left [clip][bg]; \
                Right = GFX.pCurrentClip->Right [clip][bg]; \
                if (Right <= Left) \
                    continue; \
            } \
            p = (TYPE *) Screen + Left; \
            d = Depth + Left; \
            \
            if (PPU.Mode7HFlip) \
            { \
                startx = Right - 1; \
                endx = Left - 1; \
                dir = -1; \
                aa = -l->MatrixA; \
                cc = -l->MatrixC; \
            } \
            else \
            { \
                startx = Left; \
                endx = Right; \
                dir = 1; \
                aa = l->MatrixA; \
                cc = l->MatrixC; \
            } \
   \
         xx = startx + CLIP_10_BIT_SIGNED(HOffset - CentreX); \
            if (simpleCase) \
            { \
                AA = xx << 8; \
            } \
            else \
            { \
                AA = l->MatrixA * xx; \
                CC = l->MatrixC * xx; \
            } \
            if (simpleCase) \
            { \
                if (!PPU.Mode7Repeat) \
                { \
                    int32_t x = startx; \
                    do \
                    { \
                        int32_t X = ((AA + BB) >> 8) & 0x3ff; \
                        int32_t Y = (DD >> 8) & 0x3ff; \
                        uint8_t *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
                        b = TileData[((Y & 7) << 4) + ((X & 7) << 1)]; \
         GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                        if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
                        { \
                            TYPE theColor = COLORFUNC; \
                            *p = (FUNC) | ALPHA_BITS_MASK; \
                            *d = GFX.Z1; \
                        } \
                        AA += aa, p++, d++; \
         x += dir; \
                    } while (x != endx); \
                } \
                else \
                { \
                    int32_t x = startx; \
                    do { \
                        int32_t X = (AA + BB) >> 8; \
                        int32_t Y = DD >> 8; \
\
                        if (((X | Y) & ~0x3ff) == 0) \
                        { \
                            uint8_t *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
             b = TileData[((Y & 7) << 4) + ((X & 7) << 1)]; \
             GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                            if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
                            { \
                                TYPE theColor = COLORFUNC; \
                                *p = (FUNC) | ALPHA_BITS_MASK; \
                                *d = GFX.Z1; \
                            } \
                        } \
                        else if (PPU.Mode7Repeat == 3) \
                        { \
                           uint8_t *TileData; \
                           uint32_t b; \
                            X = (x + HOffset) & 7; \
                            Y = (yy + CentreY) & 7; \
             TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
             b = TileData[((Y & 7) << 4) + ((X & 7) << 1)]; \
             GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                            if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
                            { \
                                TYPE theColor = COLORFUNC; \
                                *p = (FUNC) | ALPHA_BITS_MASK; \
                                *d = GFX.Z1; \
                            } \
                        } \
                        AA += aa; p++; d++; \
         x += dir; \
                    } while (x != endx); \
                } \
            } \
            else if (!PPU.Mode7Repeat) \
            { \
                /* The bilinear interpolator: get the colors at the four points surrounding */ \
                /* the location of one point in the _sampled_ image, and weight them according */ \
                /* to their (city block) distance.  It's very smooth, but blurry with "close up" */ \
                /* points. */ \
                \
                /* 460 (slightly less than 2 source pixels per displayed pixel) is an educated */ \
                /* guess for where bilinear filtering will become a poor method for averaging. */ \
                /* (When reducing the image, the weighting used by a bilinear filter becomes */ \
                /* arbitrary, and a simple mean is a better way to represent the source image.) */ \
                /* You can think of this as a kind of mipmapping. */ \
                if ((aa < 460 && aa > -460) && (cc < 460 && cc > -460)) \
                {\
                    int32_t x; \
                    for (x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
                    { \
                        uint32_t xPos = AA + BB; \
                        uint32_t xPix = xPos >> 8; \
                        uint32_t yPos = CC + DD; \
                        uint32_t yPix = yPos >> 8; \
                        uint32_t X = xPix & 0x3ff; \
                        uint32_t Y = yPix & 0x3ff; \
                        uint8_t *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
         b = TileData[((Y & 7) << 4) + ((X & 7) << 1)]; \
         GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                        if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
                        { \
                           TYPE theColor; \
                           uint32_t p1, p2, p3, p4; \
                           uint32_t Xdel, Ydel, XY, area1, area2, area3, area4, tempColor; \
                            /* X10 and Y01 are the X and Y coordinates of the next source point over. */ \
                            uint32_t X10 = (xPix + dir) & 0x3ff; \
                            uint32_t Y01 = (yPix + (PPU.Mode7VFlip?-1:1)) & 0x3ff; \
                            uint8_t *TileData10 = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8_t *TileData11 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8_t *TileData01 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
                            p1 = COLORFUNC; \
                            p1 = (p1 & FIRST_THIRD_COLOR_MASK) | ((p1 & SECOND_COLOR_MASK) << 16); \
                            b = TileData10[((Y & 7) << 4) + ((X10 & 7) << 1)]; \
                            p2 = COLORFUNC; \
                            p2 = (p2 & FIRST_THIRD_COLOR_MASK) | ((p2 & SECOND_COLOR_MASK) << 16); \
                            b = TileData11[((Y01 & 7) << 4) + ((X10 & 7) << 1)]; \
                            p4 = COLORFUNC; \
                            p4 = (p4 & FIRST_THIRD_COLOR_MASK) | ((p4 & SECOND_COLOR_MASK) << 16); \
                            b = TileData01[((Y01 & 7) << 4) + ((X & 7) << 1)]; \
                            p3 = COLORFUNC; \
                            p3 = (p3 & FIRST_THIRD_COLOR_MASK) | ((p3 & SECOND_COLOR_MASK) << 16); \
                            /* Xdel, Ydel: position (in 1/32nds) between the points */ \
                            Xdel = (xPos >> 3) & 0x1F; \
                            Ydel = (yPos >> 3) & 0x1F; \
                            XY = (Xdel*Ydel) >> 5; \
                            area1 = 0x20 + XY - Xdel - Ydel; \
                            area2 = Xdel - XY; \
                            area3 = Ydel - XY; \
                            area4 = XY; \
                            if(PPU.Mode7HFlip){ \
                                uint32_t tmp=area1; area1=area2; area2=tmp; \
                                tmp=area3; area3=area4; area4=tmp; \
                            } \
                            if(PPU.Mode7VFlip){ \
                                uint32_t tmp=area1; area1=area3; area3=tmp; \
                                tmp=area2; area2=area4; area4=tmp; \
                            } \
                            tempColor = ((area1 * p1) + \
                                                (area2 * p2) + \
                                                (area3 * p3) + \
                                                (area4 * p4)) >> 5; \
                            theColor  = (tempColor & FIRST_THIRD_COLOR_MASK) | ((tempColor >> 16) & SECOND_COLOR_MASK); \
                            *p        = (FUNC) | ALPHA_BITS_MASK; \
                            *d = GFX.Z1; \
                        } \
                    } \
                } \
                else \
                    /* The oversampling method: get the colors at four corners of a square */ \
                    /* in the _displayed_ image, and average them.  It's sharp and clean, but */ \
                    /* gives the usual huge pixels when the source image gets "close." */ \
                { \
                   uint32_t BB10, BB01, BB11, DD10, DD01, DD11; \
                   int32_t x; \
                    /* Find the dimensions of the square in the source image whose corners will be examined. */ \
                    uint32_t aaDelX = aa >> 1; \
                    uint32_t ccDelX = cc >> 1; \
                    uint32_t bbDelY = l->MatrixB >> 1; \
                    uint32_t ddDelY = l->MatrixD >> 1; \
                    /* Offset the location within the source image so that the four sampled points */ \
                    /* center around where the single point would otherwise have been drawn. */ \
                    BB -= (bbDelY >> 1); \
                    DD -= (ddDelY >> 1); \
                    AA -= (aaDelX >> 1); \
                    CC -= (ccDelX >> 1); \
                    BB10 = BB + aaDelX; \
                    BB01 = BB + bbDelY; \
                    BB11 = BB + aaDelX + bbDelY; \
                    DD10 = DD + ccDelX; \
                    DD01 = DD + ddDelY; \
                    DD11 = DD + ccDelX + ddDelY; \
                    for (x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
                    { \
                        uint32_t X = ((AA + BB) >> 8) & 0x3ff; \
                        uint32_t Y = ((CC + DD) >> 8) & 0x3ff; \
                        uint8_t *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
         b = TileData[((Y & 7) << 4) + ((X & 7) << 1)]; \
         GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                        if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
                        { \
                           TYPE p1, p2, p3, p4, theColor; \
                            /* X, Y, X10, Y10, etc. are the coordinates of the four pixels within the */ \
                            /* source image that we're going to examine. */ \
                            uint32_t X10 = ((AA + BB10) >> 8) & 0x3ff; \
                            uint32_t Y10 = ((CC + DD10) >> 8) & 0x3ff; \
                            uint32_t X01 = ((AA + BB01) >> 8) & 0x3ff; \
                            uint32_t Y01 = ((CC + DD01) >> 8) & 0x3ff; \
                            uint32_t X11 = ((AA + BB11) >> 8) & 0x3ff; \
                            uint32_t Y11 = ((CC + DD11) >> 8) & 0x3ff; \
                            uint8_t *TileData10 = VRAM1 + (Memory.VRAM[((Y10 & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8_t *TileData01 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X01 >> 2) & ~1)] << 7); \
                            uint8_t *TileData11 = VRAM1 + (Memory.VRAM[((Y11 & ~7) << 5) + ((X11 >> 2) & ~1)] << 7); \
                            p1 = COLORFUNC; \
                            b = TileData10[((Y10 & 7) << 4) + ((X10 & 7) << 1)]; \
                            p2 = COLORFUNC; \
                            b = TileData01[((Y01 & 7) << 4) + ((X01 & 7) << 1)]; \
                            p3 = COLORFUNC; \
                            b = TileData11[((Y11 & 7) << 4) + ((X11 & 7) << 1)]; \
                            p4 = COLORFUNC; \
                            theColor = Q_INTERPOLATE(p1, p2, p3, p4); \
                            *p = (FUNC) | ALPHA_BITS_MASK; \
                            *d = GFX.Z1; \
                        } \
                    } \
                } \
            } \
            else \
            { \
                int32_t x; \
                for (x = startx; x != endx; x += dir, AA += aa, CC += cc, p++, d++) \
                { \
                    uint32_t xPos = AA + BB; \
                    uint32_t xPix = xPos >> 8; \
                    uint32_t yPos = CC + DD; \
                    uint32_t yPix = yPos >> 8; \
                    uint32_t X = xPix; \
                    uint32_t Y = yPix; \
                    \
\
                    if (((X | Y) & ~0x3ff) == 0) \
                    { \
                        uint8_t *TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
         b = TileData[((Y & 7) << 4) + ((X & 7) << 1)]; \
         GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                        if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
                        { \
                           uint32_t p1, p2, p4, p3; \
                           uint32_t Xdel, Ydel, XY, area1, area2, area3, area4; \
                           uint32_t tempColor; \
                           TYPE theColor; \
                            /* X10 and Y01 are the X and Y coordinates of the next source point over. */ \
                            uint32_t X10 = (xPix + dir) & 0x3ff; \
                            uint32_t Y01 = (yPix + dir) & 0x3ff; \
                            uint8_t *TileData10 = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8_t *TileData11 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X10 >> 2) & ~1)] << 7); \
                            uint8_t *TileData01 = VRAM1 + (Memory.VRAM[((Y01 & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
                            p1 = COLORFUNC; \
                            p1 = (p1 & FIRST_THIRD_COLOR_MASK) | ((p1 & SECOND_COLOR_MASK) << 16); \
                            b = TileData10[((Y & 7) << 4) + ((X10 & 7) << 1)]; \
                            p2 = COLORFUNC; \
                            p2 = (p2 & FIRST_THIRD_COLOR_MASK) | ((p2 & SECOND_COLOR_MASK) << 16); \
                            b = TileData11[((Y01 & 7) << 4) + ((X10 & 7) << 1)]; \
                            p4 = COLORFUNC; \
                            p4 = (p4 & FIRST_THIRD_COLOR_MASK) | ((p4 & SECOND_COLOR_MASK) << 16); \
                            b = TileData01[((Y01 & 7) << 4) + ((X & 7) << 1)]; \
                            p3 = COLORFUNC; \
                            p3 = (p3 & FIRST_THIRD_COLOR_MASK) | ((p3 & SECOND_COLOR_MASK) << 16); \
                            /* Xdel, Ydel: position (in 1/32nds) between the points */ \
                           Xdel = (xPos >> 3) & 0x1F; \
                           Ydel = (yPos >> 3) & 0x1F; \
                           XY = (Xdel*Ydel) >> 5; \
                           area1 = 0x20 + XY - Xdel - Ydel; \
                           area2 = Xdel - XY; \
                           area3 = Ydel - XY; \
                           area4 = XY; \
                           tempColor = ((area1 * p1) + \
                                                (area2 * p2) + \
                                                (area3 * p3) + \
                                                (area4 * p4)) >> 5; \
                            theColor = (tempColor & FIRST_THIRD_COLOR_MASK) | ((tempColor >> 16) & SECOND_COLOR_MASK); \
                            *p = (FUNC) | ALPHA_BITS_MASK; \
                            *d = GFX.Z1; \
                        } \
                    } \
                    else \
                    { \
                        if (PPU.Mode7Repeat == 3) \
                        { \
                            X = (x + HOffset) & 7; \
                            Y = (yy + CentreY) & 7; \
             b = VRAM1[((Y & 7) << 4) + ((X & 7) << 1)]; \
             GFX.Z1 = Mode7Depths [(b & GFX.Mode7PriorityMask) >> 7]; \
                            if (GFX.Z1 > *d && (b & GFX.Mode7Mask) ) \
                            { \
                                TYPE theColor = COLORFUNC; \
                                *p = (FUNC) | ALPHA_BITS_MASK; \
                                *d = GFX.Z1; \
                            } \
                        } \
                    } \
                } \
            } \
        } \
    }

static uint32_t Q_INTERPOLATE(uint32_t A, uint32_t B, uint32_t C, uint32_t D)
{
   uint32_t x = ((A >> 2) & HIGH_BITS_SHIFTED_TWO_MASK) + ((B >> 2) & HIGH_BITS_SHIFTED_TWO_MASK) + ((C >> 2) & HIGH_BITS_SHIFTED_TWO_MASK) + ((D >> 2) & HIGH_BITS_SHIFTED_TWO_MASK);
   uint32_t y = (A & TWO_LOW_BITS_MASK) + (B & TWO_LOW_BITS_MASK) + (C & TWO_LOW_BITS_MASK) + (D & TWO_LOW_BITS_MASK);
   y = (y >> 2) & TWO_LOW_BITS_MASK;
   return x + y;
}

static void DrawBGMode7Background16_i(uint8_t* Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7_i(uint16_t, theColor, (ScreenColors[b & GFX.Mode7Mask]));
}

static void DrawBGMode7Background16Add_i(uint8_t* Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7_i(uint16_t, *(d + GFX.DepthDelta) ? (*(d + GFX.DepthDelta) != 1 ? (COLOR_ADD(theColor, p[GFX.Delta])) : (COLOR_ADD(theColor, GFX.FixedColour))) : theColor, (ScreenColors[b & GFX.Mode7Mask]));
}

static void DrawBGMode7Background16Add1_2_i(uint8_t* Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7_i(uint16_t, *(d + GFX.DepthDelta) ? (*(d + GFX.DepthDelta) != 1 ? COLOR_ADD1_2(theColor, p[GFX.Delta]) : COLOR_ADD(theColor, GFX.FixedColour)) : theColor, (ScreenColors[b & GFX.Mode7Mask]));
}

static void DrawBGMode7Background16Sub_i(uint8_t* Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7_i(uint16_t, *(d + GFX.DepthDelta) ? (*(d + GFX.DepthDelta) != 1 ? COLOR_SUB(theColor, p[GFX.Delta]) : COLOR_SUB(theColor, GFX.FixedColour)) : theColor, (ScreenColors[b & GFX.Mode7Mask]));
}

static void DrawBGMode7Background16Sub1_2_i(uint8_t* Screen, int32_t bg)
{
   RENDER_BACKGROUND_MODE7_i(uint16_t, *(d + GFX.DepthDelta) ? (*(d + GFX.DepthDelta) != 1 ? COLOR_SUB1_2(theColor, p[GFX.Delta]) : COLOR_SUB(theColor, GFX.FixedColour)) : theColor, (ScreenColors[b & GFX.Mode7Mask]));
}

static void RenderScreen(uint8_t* Screen, bool sub, bool force_no_add, uint8_t D)
{
   bool BG0;
   bool BG1;
   bool BG2;
   bool BG3;
   bool OB;

   GFX.S = Screen;

   if (!sub)
   {
      GFX.pCurrentClip = &IPPU.Clip [0];
      BG0 = ON_MAIN(0);
      BG1 = ON_MAIN(1);
      BG2 = ON_MAIN(2);
      BG3 = ON_MAIN(3);
      OB  = ON_MAIN(4);
   }
   else
   {
      GFX.pCurrentClip = &IPPU.Clip [1];
      BG0 = ON_SUB(0);
      BG1 = ON_SUB(1);
      BG2 = ON_SUB(2);
      BG3 = ON_SUB(3);
      OB  = ON_SUB(4);
   }

   sub |= force_no_add;

   switch (PPU.BGMode)
   {
      case 0:
      case 1:
         if (OB)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(4));
            DrawOBJS(!sub, D);
         }
         if (BG0)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(0));
            DrawBackground(PPU.BGMode, 0, D + 10, D + 14);
         }
         if (BG1)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(1));
            DrawBackground(PPU.BGMode, 1, D + 9, D + 13);
         }
         if (BG2)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(2));
            DrawBackground(PPU.BGMode, 2, D + 3, PPU.BG3Priority ? D + 17 : D + 6);
         }
         if (BG3 && PPU.BGMode == 0)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(3));
            DrawBackground(PPU.BGMode, 3, D + 2, D + 5);
         }
         break;
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
         if (OB)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(4));
            DrawOBJS(!sub, D);
         }
         if (BG0)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(0));
            DrawBackground(PPU.BGMode, 0, D + 5, D + 13);
         }
         if (BG1 && PPU.BGMode != 6)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(1));
            DrawBackground(PPU.BGMode, 1, D + 2, D + 9);
         }
         break;
      case 7:
         if (OB)
         {
            SelectTileRenderer(sub || !SUB_OR_ADD(4));
            DrawOBJS(!sub, D);
         }
         if (BG0 || ((Memory.FillRAM [0x2133] & 0x40) && BG1))
         {
            int32_t bg;

            if ((Memory.FillRAM [0x2133] & 0x40) && BG1)
            {
               GFX.Mode7Mask = 0x7f;
               GFX.Mode7PriorityMask = 0x80;
               Mode7Depths [0] = (BG0 ? 5 : 1) + D;
               Mode7Depths [1] = 9 + D;
               bg = 1;
            }
            else
            {
               GFX.Mode7Mask = 0xff;
               GFX.Mode7PriorityMask = 0;
               Mode7Depths [0] = 5 + D;
               Mode7Depths [1] = 5 + D;
               bg = 0;
            }
            if (sub || !SUB_OR_ADD(0))
               DrawBGMode7Background16(Screen, bg);
            else
            {
               if (GFX.r2131 & 0x80)
               {
                  if (GFX.r2131 & 0x40)
                     DrawBGMode7Background16Sub1_2(Screen, bg);
                  else
                     DrawBGMode7Background16Sub(Screen, bg);
               }
               else
               {
                  if (GFX.r2131 & 0x40)
                     DrawBGMode7Background16Add1_2(Screen, bg);
                  else
                     DrawBGMode7Background16Add(Screen, bg);
               }
            }
         }
         break;
      default:
         break;
   }
}

void S9xUpdateScreen(void)
{
   int32_t x2 = 1;
   uint32_t starty, endy, black;

   GFX.S = GFX.Screen;
   GFX.r2131 = Memory.FillRAM [0x2131];
   GFX.r212c = Memory.FillRAM [0x212c];
   GFX.r212d = Memory.FillRAM [0x212d];
   GFX.r2130 = Memory.FillRAM [0x2130];
   GFX.Pseudo = Memory.FillRAM [0x2133] & 8;

   if (IPPU.OBJChanged)
      S9xSetupOBJ();

   if (PPU.RecomputeClipWindows)
   {
      ComputeClipWindows();
      PPU.RecomputeClipWindows = false;
   }

   GFX.StartY = IPPU.PreviousLine;
   if ((GFX.EndY = IPPU.CurrentLine - 1) >= PPU.ScreenHeight)
      GFX.EndY = PPU.ScreenHeight - 1;

   /* XXX: Check ForceBlank? Or anything else? */
   PPU.RangeTimeOver |= GFX.OBJLines[GFX.EndY].RTOFlags;

   starty = GFX.StartY;
   endy   = GFX.EndY;

   if (PPU.BGMode == 5 || PPU.BGMode == 6 || IPPU.Interlace || IPPU.DoubleHeightPixels)
   {
      if (PPU.BGMode == 5 || PPU.BGMode == 6 || IPPU.Interlace)
      {
         IPPU.RenderedScreenWidth = 512;
         x2 = 2;
      }

      if (IPPU.DoubleHeightPixels)
      {
         starty = GFX.StartY * 2;
         endy = GFX.EndY * 2 + 1;
      }

      if ((PPU.BGMode == 5 || PPU.BGMode == 6) && !IPPU.DoubleWidthPixels)
      {
         /* The game has switched from lo-res to hi-res mode part way down
          * the screen. Scale any existing lo-res pixels on screen */
         uint32_t y;
         for (y = 0; y < starty; y++)
         {
            int32_t x;
            uint16_t* p = (uint16_t*) (GFX.Screen + y * GFX.Pitch2) + 255;
            uint16_t* q = (uint16_t*) p + 255;
            for (x = 255; x >= 0; x--, p--, q -= 2)
               q[0] = q[1] = p[0];
         }
         IPPU.DoubleWidthPixels = true;
         IPPU.HalfWidthPixels = false;
      }
      /* BJ: And we have to change the height if Interlace gets set,
       *     too. */
      if (IPPU.Interlace && !IPPU.DoubleHeightPixels)
      {
         int32_t y;

         starty                    = GFX.StartY * 2;
         endy                      = GFX.EndY * 2 + 1;
         IPPU.RenderedScreenHeight = PPU.ScreenHeight << 1;
         IPPU.DoubleHeightPixels   = true;
         GFX.Pitch2                = GFX.RealPitch;
         GFX.Pitch                 = GFX.RealPitch * 2;
         GFX.PPL                   = GFX.RealPitch;
         GFX.PPLx2                 = GFX.RealPitch;

         /* The game has switched from non-interlaced to interlaced mode
          * part way down the screen. Scale everything. */
         for (y = (int32_t) GFX.StartY - 1; y >= 0; y--)
         {
            /* memmove converted: Same malloc, different addresses, and identical addresses at line 0 [Neb]
             * DS2 DMA notes: This code path is unused [Neb] */
            memcpy(GFX.Screen + y * 2 * GFX.Pitch2, GFX.Screen + y * GFX.Pitch2, GFX.Pitch2);
            /* memmove converted: Same malloc, different addresses [Neb] */
            memcpy(GFX.Screen + (y * 2 + 1) * GFX.Pitch2, GFX.Screen + y * GFX.Pitch2, GFX.Pitch2);
         }
      }
   }

   black = BLACK | (BLACK << 16);

   if (GFX.Pseudo)
   {
      GFX.r2131 = 0x5f;
      GFX.r212c &= (Memory.FillRAM [0x212d] | 0xf0);
      GFX.r212d |= (Memory.FillRAM [0x212c] & 0x0f);
      GFX.r2130 |= 2;
   }

   if (!PPU.ForcedBlanking && ADD_OR_SUB_ON_ANYTHING && (GFX.r2130 & 0x30) != 0x30 && !((GFX.r2130 & 0x30) == 0x10 && IPPU.Clip[1].Count[5] == 0))
   {
      ClipData* pClip;

      GFX.FixedColour = BUILD_PIXEL(IPPU.XB [PPU.FixedColourRed], IPPU.XB [PPU.FixedColourGreen], IPPU.XB [PPU.FixedColourBlue]);

      /* Clear the z-buffer, marking areas 'covered' by the fixed
       * colour as depth 1. */
      pClip = &IPPU.Clip [1];

      /* Clear the z-buffer */
      if (pClip->Count [5])
      {
         /* Colour window enabled. */
         uint32_t y;
         for (y = starty; y <= endy; y++)
         {
            uint32_t c;

            memset(GFX.SubZBuffer + y * GFX.ZPitch, 0, IPPU.RenderedScreenWidth);
            memset(GFX.ZBuffer + y * GFX.ZPitch, 0, IPPU.RenderedScreenWidth);

            if (IPPU.Clip [0].Count [5])
            {
               uint32_t* p = (uint32_t*)(GFX.SubScreen + y * GFX.Pitch2);
               uint32_t* q = (uint32_t*)((uint16_t*) p + IPPU.RenderedScreenWidth);
               while (p < q)
                  *p++ = black;
            }

            for (c = 0; c < pClip->Count [5]; c++)
            {
               if (pClip->Right [c][5] > pClip->Left [c][5])
               {
                  memset(GFX.SubZBuffer + y * GFX.ZPitch + pClip->Left [c][5] * x2, 1, (pClip->Right [c][5] - pClip->Left [c][5]) * x2);

                  if (IPPU.Clip [0].Count [5])
                  {
                     /* Blast, have to clear the sub-screen to the fixed-colour
                      * because there is a colour window in effect clipping
                      * the main screen that will allow the sub-screen
                      * 'underneath' to show through. */

                     uint16_t* p = (uint16_t*)(GFX.SubScreen + y * GFX.Pitch2);
                     uint16_t* q = p + pClip->Right [c][5] * x2;
                     p += pClip->Left [c][5] * x2;

                     while (p < q)
                        *p++ = (uint16_t) GFX.FixedColour;
                  }
               }
            }
         }
      }
      else
      {
         uint32_t y;
         for (y = starty; y <= endy; y++)
         {
            memset(GFX.ZBuffer + y * GFX.ZPitch, 0, IPPU.RenderedScreenWidth);
            memset(GFX.SubZBuffer + y * GFX.ZPitch, 1, IPPU.RenderedScreenWidth);

            if (IPPU.Clip [0].Count [5])
            {
               /* Blast, have to clear the sub-screen to the fixed-colour
                * because there is a colour window in effect clipping
                * the main screen that will allow the sub-screen
                * 'underneath' to show through. */
               uint32_t b = GFX.FixedColour | (GFX.FixedColour << 16);
               uint32_t* p = (uint32_t*)(GFX.SubScreen + y * GFX.Pitch2);
               uint32_t* q = (uint32_t*)((uint16_t*) p + IPPU.RenderedScreenWidth);

               while (p < q)
                  *p++ = b;
            }
         }
      }

      if (ANYTHING_ON_SUB)
      {
         GFX.DB = GFX.SubZBuffer;
         RenderScreen(GFX.SubScreen, true, true, SUB_SCREEN_DEPTH);
      }

      if (IPPU.Clip [0].Count [5])
      {
         uint32_t y;
         for (y = starty; y <= endy; y++)
         {
            uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2);
            uint8_t* d = GFX.SubZBuffer + y * GFX.ZPitch;
            uint8_t* e = d + IPPU.RenderedScreenWidth;

            while (d < e)
            {
               if (*d > 1)
                  *p = *(p + GFX.Delta);
               else
                  *p = BLACK;
               d++;
               p++;
            }
         }
      }

      GFX.DB = GFX.ZBuffer;
      RenderScreen(GFX.Screen, false, false, MAIN_SCREEN_DEPTH);

      if (SUB_OR_ADD(5))
      {
         uint32_t y;
         uint32_t back = IPPU.ScreenColors [0];
         uint32_t Left = 0;
         uint32_t Right = 256;
         uint32_t Count;

         pClip = &IPPU.Clip [0];
         for (y = starty; y <= endy; y++)
         {
            uint32_t b;
            if (!(Count = pClip->Count [5]))
            {
               Left = 0;
               Right = 256 * x2;
               Count = 1;
            }

            for (b = 0; b < Count; b++)
            {
               if (pClip->Count [5])
               {
                  Left = pClip->Left [b][5] * x2;
                  Right = pClip->Right [b][5] * x2;
                  if (Right <= Left)
                     continue;
               }

               if (GFX.r2131 & 0x80)
               {
                  if (GFX.r2131 & 0x40)
                  {
                     /* Subtract, halving the result. */
                     uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2) + Left;
                     uint8_t* d = GFX.ZBuffer + y * GFX.ZPitch;
                     uint8_t* s = GFX.SubZBuffer + y * GFX.ZPitch + Left;
                     uint8_t* e = d + Right;
                     uint16_t back_fixed = COLOR_SUB(back, GFX.FixedColour);

                     d += Left;
                     while (d < e)
                     {
                        if (*d == 0)
                        {
                           if (*s)
                           {
                              if (*s != 1)
                                 *p = COLOR_SUB1_2(back, *(p + GFX.Delta));
                              else
                                 *p = back_fixed;
                           }
                           else
                              *p = (uint16_t) back;
                        }
                        d++;
                        p++;
                        s++;
                     }
                  }
                  else
                  {
                     /* Subtract */
                     uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2) + Left;
                     uint8_t* s = GFX.SubZBuffer + y * GFX.ZPitch + Left;
                     uint8_t* d = GFX.ZBuffer + y * GFX.ZPitch;
                     uint8_t* e = d + Right;
                     uint16_t back_fixed = COLOR_SUB(back, GFX.FixedColour);

                     d += Left;
                     while (d < e)
                     {
                        if (*d == 0)
                        {
                           if (*s)
                           {
                              if (*s != 1)
                                 *p = COLOR_SUB(back, *(p + GFX.Delta));
                              else
                                 *p = back_fixed;
                           }
                           else
                              *p = (uint16_t) back;
                        }
                        d++;
                        p++;
                        s++;
                     }
                  }
               }
               else if (GFX.r2131 & 0x40)
               {
                  uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2) + Left;
                  uint8_t* d = GFX.ZBuffer + y * GFX.ZPitch;
                  uint8_t* s = GFX.SubZBuffer + y * GFX.ZPitch + Left;
                  uint8_t* e = d + Right;
                  uint16_t back_fixed = COLOR_ADD(back, GFX.FixedColour);
                  d += Left;
                  while (d < e)
                  {
                     if (*d == 0)
                     {
                        if (*s)
                        {
                           if (*s != 1)
                              *p = COLOR_ADD1_2(back, *(p + GFX.Delta));
                           else
                              *p = back_fixed;
                        }
                        else
                           *p = (uint16_t) back;
                     }
                     d++;
                     p++;
                     s++;
                  }
               }
               else if (back != 0)
               {
                  uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2) + Left;
                  uint8_t* d = GFX.ZBuffer + y * GFX.ZPitch;
                  uint8_t* s = GFX.SubZBuffer + y * GFX.ZPitch + Left;
                  uint8_t* e = d + Right;
                  uint16_t back_fixed = COLOR_ADD(back, GFX.FixedColour);
                  d += Left;
                  while (d < e)
                  {
                     if (*d == 0)
                     {
                        if (*s)
                        {
                           if (*s != 1)
                              *p = COLOR_ADD(back, *(p + GFX.Delta));
                           else
                              *p = back_fixed;
                        }
                        else
                           *p = (uint16_t) back;
                     }
                     d++;
                     p++;
                     s++;
                  }
               }
               else
               {
                  if (!pClip->Count [5])
                  {
                     /* The backdrop has not been cleared yet - so
                      * copy the sub-screen to the main screen
                      * or fill it with the back-drop colour if the
                      * sub-screen is clear. */
                     uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2) + Left;
                     uint8_t* d = GFX.ZBuffer + y * GFX.ZPitch;
                     uint8_t* s = GFX.SubZBuffer + y * GFX.ZPitch + Left;
                     uint8_t* e = d + Right;
                     d += Left;
                     while (d < e)
                     {
                        if (*d == 0)
                        {
                           if (*s)
                           {
                              if (*s != 1)
                                 *p = *(p + GFX.Delta);
                              else
                                 *p = GFX.FixedColour;
                           }
                           else
                              *p = (uint16_t) back;
                        }
                        d++;
                        p++;
                        s++;
                     }
                  }
               }
            }
         }
      } /* --if (SUB_OR_ADD(5)) */
      else
      {
         uint32_t y;
         /* Subscreen not being added to back */
         uint32_t back = IPPU.ScreenColors [0] | (IPPU.ScreenColors [0] << 16);
         pClip = &IPPU.Clip [0];

         if (pClip->Count [5])
         {
            for (y = starty; y <= endy; y++)
            {
               uint32_t b;
               for (b = 0; b < pClip->Count [5]; b++)
               {
                  uint32_t Left = pClip->Left [b][5] * x2;
                  uint32_t Right = pClip->Right [b][5] * x2;
                  uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2) + Left;
                  uint8_t* d = GFX.ZBuffer + y * GFX.ZPitch;
                  uint8_t* e = d + Right;
                  d += Left;

                  while (d < e)
                  {
                     if (*d == 0)
                        *p = (int16_t) back;
                     d++;
                     p++;
                  }
               }
            }
         }
         else
         {
            for (y = starty; y <= endy; y++)
            {
               uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2);
               uint8_t* d = GFX.ZBuffer + y * GFX.ZPitch;
               uint8_t* e = d + 256 * x2;

               while (d < e)
               {
                  if (*d == 0)
                     *p = (int16_t) back;
                  d++;
                  p++;
               }
            }
         }
      }
   } /* force blanking */
   else
   {
      /* 16bit and transparency but currently no transparency effects in
       * operation. */

      uint32_t back = IPPU.ScreenColors [0] | (IPPU.ScreenColors [0] << 16);

      if (PPU.ForcedBlanking)
         back = black;

      if (IPPU.Clip [0].Count[5])
      {
         uint32_t y;
         for (y = starty; y <= endy; y++)
         {
            uint32_t c;
            uint32_t* p = (uint32_t*)(GFX.Screen + y * GFX.Pitch2);
            uint32_t* q = (uint32_t*)((uint16_t*) p + IPPU.RenderedScreenWidth);

            while (p < q)
               *p++ = black;

            for (c = 0; c < IPPU.Clip [0].Count [5]; c++)
            {
               if (IPPU.Clip [0].Right [c][5] > IPPU.Clip [0].Left [c][5])
               {
                  uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2);
                  uint16_t* q = p + IPPU.Clip [0].Right [c][5] * x2;
                  p += IPPU.Clip [0].Left [c][5] * x2;

                  while (p < q)
                     *p++ = (uint16_t) back;
               }
            }
         }
      }
      else
      {
         uint32_t y;
         for (y = starty; y <= endy; y++)
         {
            uint32_t* p = (uint32_t*)(GFX.Screen + y * GFX.Pitch2);
            uint32_t* q = (uint32_t*)((uint16_t*) p + IPPU.RenderedScreenWidth);
            while (p < q)
               *p++ = back;
         }
      }

      if (!PPU.ForcedBlanking)
      {
         uint32_t y;
         for (y = starty; y <= endy; y++)
            memset(GFX.ZBuffer + y * GFX.ZPitch, 0, IPPU.RenderedScreenWidth);
         GFX.DB = GFX.ZBuffer;
         RenderScreen(GFX.Screen, false, true, SUB_SCREEN_DEPTH);
      }
   }

   if (PPU.BGMode != 5 && PPU.BGMode != 6 && IPPU.DoubleWidthPixels)
   {
      /* Mixture of background modes used on screen - scale width
       * of all non-mode 5 and 6 pixels. */
      uint32_t y;
      for (y = starty; y <= endy; y++)
      {
         int32_t x;
         uint16_t* p = (uint16_t*)(GFX.Screen + y * GFX.Pitch2) + 255;
         uint16_t* q = p + 255;
         for (x = 255; x >= 0; x--, p--, q -= 2)
            q[0] = q[1] = p[0];
      }
   }

   /* Double the height of the pixels just drawn */
   FIX_INTERLACE(GFX.Screen, false, GFX.ZBuffer);

   IPPU.PreviousLine = IPPU.CurrentLine;
}
