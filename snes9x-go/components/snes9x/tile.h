/* This file is part of Snes9x. See LICENSE file. */

#ifndef _TILE_H_
#define _TILE_H_

#define TILE_PREAMBLE_VARS() \
    uint32_t l; \
    uint16_t *ScreenColors; \
    uint8_t *pCache; \
    uint32_t TileNumber; \
    uint32_t TileAddr

#define TILE_PREAMBLE_CODE() \
    TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift); \
    if ((Tile & 0x1ff) >= 256) \
       TileAddr += BG.NameSelect; \
    TileAddr &= 0xffff; \
    pCache = &BG.Buffer[(TileNumber = (TileAddr >> BG.TileShift)) << 6]; \
    if (BG.Buffered [TileNumber] != (0x10|BG.Depth)) \
    { \
      BG.Buffered[TileAddr >> 4] = BG.Buffered[TileAddr >> 5] = BG.Buffered[TileAddr >> 6] = false; \
      BG.Buffered[TileNumber] = ConvertTile (pCache, TileAddr); \
    } \
    if (BG.Buffered [TileNumber] == BLANK_TILE) \
       return; \
    if (BG.DirectColourMode) \
       ScreenColors = &IPPU.DirectColors [((Tile >> 10) & BG.PaletteMask) << 8]; \
    else \
       ScreenColors = &IPPU.ScreenColors [(((Tile >> 10) & BG.PaletteMask) << BG.PaletteShift) + BG.StartPalette]

#define RENDER_TILE(NORMAL, FLIPPED, N) \
    switch (Tile & (V_FLIP | H_FLIP)) \
    { \
    case 0: \
       bp = pCache + StartLine; \
       for (l = LineCount; l != 0; l--, bp += 8, Offset += GFX.PPL) \
       { \
          NORMAL (Offset, bp, ScreenColors); \
          NORMAL (Offset + N, bp + 4, ScreenColors); \
       } \
       break; \
    case H_FLIP: \
       bp = pCache + StartLine; \
       for (l = LineCount; l != 0; l--, bp += 8, Offset += GFX.PPL) \
       { \
          FLIPPED (Offset, bp + 4, ScreenColors); \
          FLIPPED (Offset + N, bp, ScreenColors); \
       } \
       break; \
    case H_FLIP | V_FLIP: \
       bp = pCache + 56 - StartLine; \
       for (l = LineCount; l != 0; l--, bp -= 8, Offset += GFX.PPL) \
       { \
          FLIPPED (Offset, bp + 4, ScreenColors); \
          FLIPPED (Offset + N, bp, ScreenColors); \
       } \
       break; \
    case V_FLIP: \
       bp = pCache + 56 - StartLine; \
       for (l = LineCount; l != 0; l--, bp -= 8, Offset += GFX.PPL) \
       { \
          NORMAL (Offset, bp, ScreenColors); \
          NORMAL (Offset + N, bp + 4, ScreenColors); \
       } \
       break; \
    default: \
       break; \
    }

#define TILE_CLIP_PREAMBLE_VARS() \
    uint32_t d1; \
    uint32_t d2

#define TILE_CLIP_PREAMBLE_CODE() \
    if (StartPixel < 4) \
    { \
       d1 = HeadMask [StartPixel]; \
       if (StartPixel + Width < 4) \
          d1 &= TailMask [StartPixel + Width]; \
    } \
    else \
       d1 = 0; \
    if (StartPixel + Width > 4) \
    { \
       if (StartPixel > 4) \
          d2 = HeadMask [StartPixel - 4]; \
       else \
          d2 = 0xffffffff; \
       d2 &= TailMask [(StartPixel + Width - 4)]; \
    } \
    else \
       d2 = 0

#define RENDER_CLIPPED_TILE_VARS() \
    uint32_t dd

#define RENDER_CLIPPED_TILE_CODE(NORMAL, FLIPPED, N) \
    switch (Tile & (V_FLIP | H_FLIP)) \
    { \
    case 0: \
       bp = pCache + StartLine; \
       for (l = LineCount; l != 0; l--, bp += 8, Offset += GFX.PPL) \
       { \
          /* This is perfectly OK, regardless of endianness. The tiles are \
           * cached in leftmost-endian order (when not horiz flipped) by \
           * the ConvertTile function. */ \
          if ((dd = (*(uint32_t *) bp) & d1)) \
             NORMAL (Offset, (uint8_t *) &dd, ScreenColors); \
          if ((dd = (*(uint32_t *) (bp + 4)) & d2)) \
             NORMAL (Offset + N, (uint8_t *) &dd, ScreenColors); \
       } \
       break; \
    case H_FLIP: \
       bp = pCache + StartLine; \
       SWAP_DWORD (d1); \
       SWAP_DWORD (d2); \
       for (l = LineCount; l != 0; l--, bp += 8, Offset += GFX.PPL) \
       { \
          if ((dd = *(uint32_t *) (bp + 4) & d1)) \
             FLIPPED (Offset, (uint8_t *) &dd, ScreenColors); \
          if ((dd = *(uint32_t *) bp & d2)) \
             FLIPPED (Offset + N, (uint8_t *) &dd, ScreenColors); \
       } \
       break; \
    case H_FLIP | V_FLIP: \
       bp = pCache + 56 - StartLine; \
       SWAP_DWORD (d1); \
       SWAP_DWORD (d2); \
       for (l = LineCount; l != 0; l--, bp -= 8, Offset += GFX.PPL) \
       { \
          if ((dd = *(uint32_t *) (bp + 4) & d1)) \
             FLIPPED (Offset, (uint8_t *) &dd, ScreenColors); \
          if ((dd = *(uint32_t *) bp & d2)) \
             FLIPPED (Offset + N, (uint8_t *) &dd, ScreenColors); \
       } \
       break; \
    case V_FLIP: \
       bp = pCache + 56 - StartLine; \
       for (l = LineCount; l != 0; l--, bp -= 8, Offset += GFX.PPL) \
       { \
          if ((dd = (*(uint32_t *) bp) & d1)) \
             NORMAL (Offset, (uint8_t *) &dd, ScreenColors); \
          if ((dd = (*(uint32_t *) (bp + 4)) & d2)) \
             NORMAL (Offset + N, (uint8_t *) &dd, ScreenColors); \
       } \
       break; \
    default: \
       break; \
    }

#define RENDER_TILE_LARGE(PIXEL, FUNCTION) \
    switch (Tile & (V_FLIP | H_FLIP)) \
    { \
    case H_FLIP: \
       StartPixel = 7 - StartPixel; \
       /* fallthrough for no-flip case - above was a horizontal flip */ \
    case 0: \
       if((pixel = pCache[StartLine + StartPixel])) \
       { \
          pixel = PIXEL; \
          for (l = LineCount; l != 0; l--, sp += GFX.PPL, Depth += GFX.PPL) \
          { \
             int32_t z; \
             for (z = Pixels - 1; z >= 0; z--) \
             { \
                if (GFX.Z1 > Depth [z]) \
                { \
                   sp [z] = FUNCTION(sp + z, pixel); \
                   Depth [z] = GFX.Z2; \
                }\
             } \
          } \
       } \
       break; \
    case H_FLIP | V_FLIP: \
       StartPixel = 7 - StartPixel; \
       /* fallthrough for V_FLIP-only case - above was a horizontal flip */ \
    case V_FLIP: \
       if((pixel = pCache[56 - StartLine + StartPixel])) \
       { \
          pixel = PIXEL; \
          for (l = LineCount; l != 0; l--, sp += GFX.PPL, Depth += GFX.PPL) \
          { \
             int32_t z; \
             for (z = Pixels - 1; z >= 0; z--) \
             { \
                if (GFX.Z1 > Depth [z]) \
                { \
                   sp [z] = FUNCTION(sp + z, pixel); \
                   Depth [z] = GFX.Z2; \
                }\
             } \
          } \
       } \
       break; \
    default: \
       break; \
    }

#define RENDER_TILE_LARGE_HALFWIDTH(PIXEL, FUNCTION) \
    switch (Tile & (V_FLIP | H_FLIP)) \
    { \
    case H_FLIP: \
       StartPixel = 7 - StartPixel; \
       /* fallthrough for no-flip case - above was a horizontal flip */ \
    case 0: \
       if((pixel = pCache[StartLine + StartPixel])) \
       { \
          pixel = PIXEL; \
          for (l = LineCount; l != 0; l--, sp += GFX.PPL, Depth += GFX.PPL) \
          { \
             int32_t z; \
             for (z = Pixels - 2; z >= 0; z -= 2) \
             { \
                if (GFX.Z1 > Depth [z]) \
                { \
                   sp [z >> 1] = FUNCTION(sp + z, pixel); \
                   Depth [z >> 1] = GFX.Z2; \
                } \
             } \
          } \
       } \
       break; \
    case H_FLIP | V_FLIP: \
       StartPixel = 7 - StartPixel; \
       /* fallthrough for V_FLIP-only case - above was a horizontal flip */ \
    case V_FLIP: \
       if((pixel = pCache[56 - StartLine + StartPixel])) \
       { \
          pixel = PIXEL; \
          for (l = LineCount; l != 0; l--, sp += GFX.PPL, Depth += GFX.PPL) \
          { \
             int32_t z; \
             for (z = Pixels - 2; z >= 0; z -= 2) \
             { \
                if (GFX.Z1 > Depth [z]) \
                { \
                   sp [z >> 1] = FUNCTION(sp + z, pixel); \
                   Depth [z >> 1] = GFX.Z2; \
                } \
             } \
          } \
       } \
       break; \
    default: \
       break; \
    }
#endif
