/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// This file includes itself multiple times.
// The other option would be to have 4 files, where A includes B, and B includes C 3 times, and C includes D 5 times.
// Look for the following marker to find where the divisions are.

// Top-level compilation.

#ifndef _NEWTILE_CPP
#define _NEWTILE_CPP

// #pragma GCC optimize("Os")

#include "snes9x.h"
#include "ppu.h"
#include "tile.h"

static uint32	pixbit[8][16];
static uint8	hrbit_odd[256];
static uint8	hrbit_even[256];
static uint16	BlackColourMap[256];
static uint16	DirectColourMaps[8][256];
static uint8	brightness_cap[64];
static struct SLineMatrixData LineMatrixData[240];
static uint8 mul_brightness[16][32] =
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 },
	{ 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04 },
	{ 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
	  0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06 },
	{ 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04,
	  0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08 },
	{ 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05,
	  0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a },
	{ 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06,
	  0x06, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c },
	{ 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
	  0x07, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e },
	{ 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08,
	  0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x11 },
	{ 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x08, 0x09,
	  0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x11, 0x12, 0x13 },
	{ 0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x09, 0x0a,
	  0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x11, 0x12, 0x13, 0x13, 0x14, 0x15 },
	{ 0x00, 0x01, 0x01, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b,
	  0x0c, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17 },
	{ 0x00, 0x01, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c,
	  0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x16, 0x16, 0x17, 0x18, 0x19 },
	{ 0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d,
	  0x0e, 0x0f, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x17, 0x18, 0x19, 0x1a, 0x1b },
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	  0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d },
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f }
};


void S9xInitTileRenderer (void)
{
	memset(BlackColourMap, 0, 256 * sizeof(uint16));

	S9xBuildDirectColourMaps();
	S9xFixColourBrightness();

	int	i;

	for (i = 0; i < 16; i++)
	{
		uint32	b = 0;

	#ifdef LSB_FIRST
		if (i & 8)
			b |= 1;
		if (i & 4)
			b |= 1 << 8;
		if (i & 2)
			b |= 1 << 16;
		if (i & 1)
			b |= 1 << 24;
	#else
		if (i & 8)
			b |= 1 << 24;
		if (i & 4)
			b |= 1 << 16;
		if (i & 2)
			b |= 1 << 8;
		if (i & 1)
			b |= 1;
	#endif

		for (uint8 bitshift = 0; bitshift < 8; bitshift++)
			pixbit[bitshift][i] = b << bitshift;
	}

	for (i = 0; i < 256; i++)
	{
		uint8	m = 0;
		uint8	s = 0;

		if (i & 0x80)
			s |= 8;
		if (i & 0x40)
			m |= 8;
		if (i & 0x20)
			s |= 4;
		if (i & 0x10)
			m |= 4;
		if (i & 0x08)
			s |= 2;
		if (i & 0x04)
			m |= 2;
		if (i & 0x02)
			s |= 1;
		if (i & 0x01)
			m |= 1;

		hrbit_odd[i]  = m;
		hrbit_even[i] = s;
	}
}

void S9xBuildDirectColourMaps (void)
{
	IPPU.XB = (uint8 *)mul_brightness[PPU.Brightness];

	for (int p = 0; p < 8; p++)
	{
		for (int c = 0; c < 256; c++)
		{
			uint8 r = IPPU.XB[((c & 7) << 2) | ((p & 1) << 1)];
			uint8 g = IPPU.XB[((c & 0x38) >> 1) | (p & 2)];
			uint8 b = IPPU.XB[((c & 0xc0) >> 3) | (p & 4)];
			DirectColourMaps[p][c] = BUILD_PIXEL(r, g, b);
		}
	}
}

void S9xFixColourBrightness (void)
{
	IPPU.XB = (uint8 *)mul_brightness[PPU.Brightness];

	for (int i = 0; i < 64; i++)
	{
		if (i > IPPU.XB[0x1f])
			brightness_cap[i] = IPPU.XB[0x1f];
		else
			brightness_cap[i] = i;
	}

	for (int i = 0; i < 256; i++)
	{
		uint8 r = IPPU.XB[(PPU.CGDATA[i])       & 0x1f];
		uint8 g = IPPU.XB[(PPU.CGDATA[i] >>  5) & 0x1f];
		uint8 b = IPPU.XB[(PPU.CGDATA[i] >> 10) & 0x1f];
		IPPU.ScreenColors[i] = BUILD_PIXEL(r, g, b);
	}
}

void S9UpdateLineMatrix(int line)
{
	struct SLineMatrixData *p = &LineMatrixData[line];
	p->MatrixA = PPU.MatrixA;
	p->MatrixB = PPU.MatrixB;
	p->MatrixC = PPU.MatrixC;
	p->MatrixD = PPU.MatrixD;
	p->CentreX = PPU.CentreX;
	p->CentreY = PPU.CentreY;
	p->M7HOFS  = PPU.M7HOFS;
	p->M7VOFS  = PPU.M7VOFS;
}

#define COLOR_ADD1_2(C1, C2) \
	((((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
	((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	((C1) & (C2) & RGB_LOW_BITS_MASK)) | ALPHA_BITS_MASK)
#define COLOR_ADD_BRIGHTNESS1_2 COLOR_ADD1_2

static inline uint16 COLOR_ADD_BRIGHTNESS(uint32 C1, uint32 C2)
{
    return ((brightness_cap[ (C1 >> RED_SHIFT_BITS)           +  (C2 >> RED_SHIFT_BITS)          ] << RED_SHIFT_BITS)   |
            (brightness_cap[((C1 >> GREEN_SHIFT_BITS) & 0x1f) + ((C2 >> GREEN_SHIFT_BITS) & 0x1f)] << GREEN_SHIFT_BITS) |
// Proper 15->16bit color conversion moves the high bit of green into the low bit.
#if GREEN_SHIFT_BITS == 6
           ((brightness_cap[((C1 >> 6) & 0x1f) + ((C2 >> 6) & 0x1f)] & 0x10) << 1) |
#endif
            (brightness_cap[ (C1                      & 0x1f) +  (C2                      & 0x1f)]      ));
}

static inline uint16 COLOR_ADD(uint32 C1, uint32 C2)
{
	const uint32 RED_MASK   = 0x1F << RED_SHIFT_BITS;
	const uint32 GREEN_MASK = 0x1F << GREEN_SHIFT_BITS;
	const uint32 BLUE_MASK  = 0x1F;

	int rb = C1 & (RED_MASK | BLUE_MASK);
	rb += C2 & (RED_MASK | BLUE_MASK);
	int rbcarry = rb & ((0x20 << RED_SHIFT_BITS) | (0x20 << 0));
	int g = (C1 & (GREEN_MASK)) + (C2 & (GREEN_MASK));
	int rgbsaturate = (((g & (0x20 << GREEN_SHIFT_BITS)) | rbcarry) >> 5) * 0x1f;
	uint16 retval = (rb & (RED_MASK | BLUE_MASK)) | (g & GREEN_MASK) | rgbsaturate;
#if GREEN_SHIFT_BITS == 6
	retval |= (retval & 0x0400) >> 5;
#endif
	return retval;
}

#define COLOR_SUB1_2(C1, C2) \
	GFX.ZERO[(((C1) | RGB_HI_BITS_MASKx2) - \
	((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1]

static inline uint16 COLOR_SUB (uint32 C1, uint32 C2)
{
	int rb1 = (C1 & (THIRD_COLOR_MASK | FIRST_COLOR_MASK)) | ((0x20 << 0) | (0x20 << RED_SHIFT_BITS));
	int rb2 = C2 & (THIRD_COLOR_MASK | FIRST_COLOR_MASK);
	int rb = rb1 - rb2;
	int rbcarry = rb & ((0x20 << RED_SHIFT_BITS) | (0x20 << 0));
	int g = ((C1 & (SECOND_COLOR_MASK)) | (0x20 << GREEN_SHIFT_BITS)) - (C2 & (SECOND_COLOR_MASK));
	int rgbsaturate = (((g & (0x20 << GREEN_SHIFT_BITS)) | rbcarry) >> 5) * 0x1f;
	uint16 retval = ((rb & (THIRD_COLOR_MASK | FIRST_COLOR_MASK)) | (g & SECOND_COLOR_MASK)) & rgbsaturate;
#if GREEN_SHIFT_BITS == 6
	retval |= (retval & 0x0400) >> 5;
#endif
	return retval;
}

// Here are the tile converters, selected by S9xSelectTileConverter().
// Really, except for the definition of DOBIT and the number of times it is called, they're all the same.

#define DOBIT(n, i) \
	if ((pix = *(tp + (n)))) \
	{ \
		p1 |= pixbit[(i)][pix >> 4]; \
		p2 |= pixbit[(i)][pix & 0xf]; \
	}

static uint8 ConvertTile2 (uint8 *pCache, uint32 TileAddr, uint32)
{
	uint8	*tp      = &Memory.VRAM[TileAddr];
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;

	for (int line = 8; line != 0; line--, tp += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		uint32	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4 (uint8 *pCache, uint32 TileAddr, uint32)
{
	uint8	*tp      = &Memory.VRAM[TileAddr];
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;

	for (int line = 8; line != 0; line--, tp += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		uint32	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		DOBIT(16, 2);
		DOBIT(17, 3);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile8 (uint8 *pCache, uint32 TileAddr, uint32)
{
	uint8	*tp      = &Memory.VRAM[TileAddr];
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;

	for (int line = 8; line != 0; line--, tp += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		uint32	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		DOBIT(16, 2);
		DOBIT(17, 3);
		DOBIT(32, 4);
		DOBIT(33, 5);
		DOBIT(48, 6);
		DOBIT(49, 7);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

#define DOBIT(n, i) \
	if ((pix = hrbit_odd[*(tp1 + (n))])) \
		p1 |= pixbit[(i)][pix]; \
	if ((pix = hrbit_odd[*(tp2 + (n))])) \
		p2 |= pixbit[(i)][pix];

static uint8 ConvertTile2h_odd (uint8 *pCache, uint32 TileAddr, uint32 Tile)
{
	uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;

	if (Tile == 0x3ff)
		tp2 = tp1 - (0x3ff << 4);
	else
		tp2 = tp1 + (1 << 4);

	for (int line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		uint32	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4h_odd (uint8 *pCache, uint32 TileAddr, uint32 Tile)
{
	uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;

	if (Tile == 0x3ff)
		tp2 = tp1 - (0x3ff << 5);
	else
		tp2 = tp1 + (1 << 5);

	for (int line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		uint8	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		DOBIT(16, 2);
		DOBIT(17, 3);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

#define DOBIT(n, i) \
	if ((pix = hrbit_even[*(tp1 + (n))])) \
		p1 |= pixbit[(i)][pix]; \
	if ((pix = hrbit_even[*(tp2 + (n))])) \
		p2 |= pixbit[(i)][pix];

static uint8 ConvertTile2h_even (uint8 *pCache, uint32 TileAddr, uint32 Tile)
{
	uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;

	if (Tile == 0x3ff)
		tp2 = tp1 - (0x3ff << 4);
	else
		tp2 = tp1 + (1 << 4);

	for (int line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		uint32	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4h_even (uint8 *pCache, uint32 TileAddr, uint32 Tile)
{
	uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;

	if (Tile == 0x3ff)
		tp2 = tp1 - (0x3ff << 5);
	else
		tp2 = tp1 + (1 << 5);

	for (int line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		uint32	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		DOBIT(16, 2);
		DOBIT(17, 3);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

// First-level include: Get all the renderers.
#include "tile.cpp"

// Functions to select which converter and renderer to use.

void S9xSelectTileRenderers (int BGMode, bool8 sub, bool8 obj)
{
	void	(**DT)		(uint32, uint32, uint32, uint32);
	void	(**DCT)		(uint32, uint32, uint32, uint32, uint32, uint32);
	void	(**DMP)		(uint32, uint32, uint32, uint32, uint32, uint32);
	void	(**DB)		(uint32, uint32, uint32);
	void	(**DM7BG1)	(uint32, uint32, int);
	void	(**DM7BG2)	(uint32, uint32, int);
	bool8	M7M1, M7M2;

	M7M1 = PPU.BGMosaic[0] && PPU.Mosaic > 1;
	M7M2 = PPU.BGMosaic[1] && PPU.Mosaic > 1;

	DT     = Renderers_DrawTile16Normal1x1;
	DCT    = Renderers_DrawClippedTile16Normal1x1;
	DMP    = Renderers_DrawMosaicPixel16Normal1x1;
	DB     = Renderers_DrawBackdrop16Normal1x1;
	DM7BG1 = M7M1 ? Renderers_DrawMode7MosaicBG1Normal1x1 : Renderers_DrawMode7BG1Normal1x1;
	DM7BG2 = M7M2 ? Renderers_DrawMode7MosaicBG2Normal1x1 : Renderers_DrawMode7BG2Normal1x1;

	GFX.DrawTileNomath        = DT[0];
	GFX.DrawClippedTileNomath = DCT[0];
	GFX.DrawMosaicPixelNomath = DMP[0];
	GFX.DrawBackdropNomath    = DB[0];
	GFX.DrawMode7BG1Nomath    = DM7BG1[0];
	GFX.DrawMode7BG2Nomath    = DM7BG2[0];

	int	i = 0;

	if (Settings.Transparency)
	{
		i = (Memory.PPU_IO[0x131] & 0x80) ? 4 : 1;
		if (Memory.PPU_IO[0x131] & 0x40)
		{
			i++;
			if (Memory.PPU_IO[0x130] & 2)
				i++;
		}
		if (IPPU.MaxBrightness != 0xf)
		{
			if (i == 1)
				i = 7;
			else if (i == 3)
				i = 8;
		}

	}

	GFX.DrawTileMath        = DT[i];
	GFX.DrawClippedTileMath = DCT[i];
	GFX.DrawMosaicPixelMath = DMP[i];
	GFX.DrawBackdropMath    = DB[i];
	GFX.DrawMode7BG1Math    = DM7BG1[i];
	GFX.DrawMode7BG2Math    = DM7BG2[i];
}

void S9xSelectTileConverter (int depth, bool8 hires, bool8 sub, bool8 mosaic)
{
	switch (depth)
	{
		case 8:
			BG.ConvertTile      = BG.ConvertTileFlip = ConvertTile8;
			BG.Buffered         = BG.BufferedFlip    = TILE_8BIT;
			BG.TileShift        = 6;
			BG.PaletteShift     = 0;
			BG.PaletteMask      = 0;
			BG.DirectColourMode = Memory.PPU_IO[0x130] & 1;

			break;

		case 4:
			if (hires)
			{
				if (sub || mosaic)
				{
					BG.ConvertTile     = ConvertTile4h_even;
					BG.Buffered        = TILE_4BIT_EVEN;
					BG.ConvertTileFlip = ConvertTile4h_odd;
					BG.BufferedFlip    = TILE_4BIT_ODD;
				}
				else
				{
					BG.ConvertTile     = ConvertTile4h_odd;
					BG.Buffered        = TILE_4BIT_ODD;
					BG.ConvertTileFlip = ConvertTile4h_even;
					BG.BufferedFlip    = TILE_4BIT_EVEN;
				}
			}
			else
			{
				BG.ConvertTile = BG.ConvertTileFlip = ConvertTile4;
				BG.Buffered    = BG.BufferedFlip    = TILE_4BIT;
			}

			BG.TileShift        = 5;
			BG.PaletteShift     = 10 - 4;
			BG.PaletteMask      = 7 << 4;
			BG.DirectColourMode = FALSE;

			break;

		case 2:
			if (hires)
			{
				if (sub || mosaic)
				{
					BG.ConvertTile     = ConvertTile2h_even;
					BG.Buffered        = TILE_2BIT_EVEN;
					BG.ConvertTileFlip = ConvertTile2h_odd;
					BG.BufferedFlip    = TILE_2BIT_ODD;
				}
				else
				{
					BG.ConvertTile     = ConvertTile2h_odd;
					BG.Buffered        = TILE_2BIT_ODD;
					BG.ConvertTileFlip = ConvertTile2h_even;
					BG.BufferedFlip    = TILE_2BIT_EVEN;
				}
			}
			else
			{
				BG.ConvertTile = BG.ConvertTileFlip = ConvertTile2;
				BG.Buffered    = BG.BufferedFlip    = TILE_2BIT;
			}

			BG.TileShift        = 4;
			BG.PaletteShift     = 10 - 2;
			BG.PaletteMask      = 7 << 2;
			BG.DirectColourMode = FALSE;

			break;
	}
}

/*****************************************************************************/
#else
#ifndef NAME1 // First-level: Get all the renderers.
/*****************************************************************************/

#define GET_CACHED_TILE() \
	uint32	TileNumber; \
	uint32	TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift); \
	if (Tile & 0x100) \
		TileAddr += BG.NameSelect; \
	TileAddr &= 0xffff; \
	TileNumber = TileAddr >> BG.TileShift; \
	pCache = &IPPU.TileCacheData[TileNumber << 6]; \
	if (Tile & H_FLIP) \
	{ \
		if (IPPU.TileCache[TileNumber] != (BG.BufferedFlip|TILE_CACHE_VALID|TILE_CACHE_FLIP)) { \
			if (BG.ConvertTileFlip(pCache, TileAddr, Tile & 0x3ff) == BLANK_TILE) \
				IPPU.TileCache[TileNumber] = BG.BufferedFlip|TILE_CACHE_VALID|TILE_CACHE_BLANK|TILE_CACHE_FLIP; \
			else \
				IPPU.TileCache[TileNumber] = BG.BufferedFlip|TILE_CACHE_VALID|TILE_CACHE_FLIP; \
		} \
	} \
	else \
	{ \
		if (IPPU.TileCache[TileNumber] != (BG.Buffered|TILE_CACHE_VALID)) { \
			if (BG.ConvertTile(pCache, TileAddr, Tile & 0x3ff) == BLANK_TILE) \
				IPPU.TileCache[TileNumber] = BG.Buffered|TILE_CACHE_VALID|TILE_CACHE_BLANK; \
			else \
				IPPU.TileCache[TileNumber] = BG.Buffered|TILE_CACHE_VALID; \
		} \
	}

#define IS_BLANK_TILE() \
	(IPPU.TileCache[TileNumber] & TILE_CACHE_BLANK)

#define SELECT_PALETTE() \
	if (BG.DirectColourMode) \
	{ \
		GFX.RealScreenColors = DirectColourMaps[(Tile >> 10) & 7]; \
	} \
	else \
		GFX.RealScreenColors = &IPPU.ScreenColors[((Tile >> BG.PaletteShift) & BG.PaletteMask) + BG.StartPalette]; \
	GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors

#define NOMATH(Op, Main, Sub, SD) \
	(Main)

#define REGMATH(Op, Main, Sub, SD) \
	(COLOR_##Op((Main), ((SD) & 0x20) ? (Sub) : GFX.FixedColour))

#define MATHF1_2(Op, Main, Sub, SD) \
	(GFX.ClipColors ? (COLOR_##Op((Main), GFX.FixedColour)) : (COLOR_##Op##1_2((Main), GFX.FixedColour)))

#define MATHS1_2(Op, Main, Sub, SD) \
	(GFX.ClipColors ? REGMATH(Op, Main, Sub, SD) : (((SD) & 0x20) ? COLOR_##Op##1_2((Main), (Sub)) : COLOR_##Op((Main), GFX.FixedColour)))

// Basic routine to render an unclipped tile.
// Input parameters:
//     DRAW_PIXEL(N, M) is a routine to actually draw the pixel. N is the pixel in the row to draw,
//     and M is a test which if false means the pixel should be skipped.
//     Z1 is the "draw if Z1 > cur_depth".
//     Z2 is the "cur_depth = new_depth". OBJ need the two separate.
//     Pix is the pixel to draw.

#define Z1	GFX.Z1
#define Z2	GFX.Z2

#define DRAW_TILE() \
	uint8			*pCache; \
	int32	l; \
	uint8	*bp, Pix; \
	\
	GET_CACHED_TILE(); \
	if (IS_BLANK_TILE()) \
		return; \
	SELECT_PALETTE(); \
	\
	if (!(Tile & (V_FLIP | H_FLIP))) \
	{ \
		bp = pCache + StartLine; \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, bp += 8, Offset += GFX.PPL) \
		{ \
			for (int x = 0; x < 8; x++) { \
				DRAW_PIXEL(x, Pix = bp[x]); \
			} \
		} \
	} \
	else \
	if (!(Tile & V_FLIP)) \
	{ \
		bp = pCache + StartLine; \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, bp += 8, Offset += GFX.PPL) \
		{ \
			for (int x = 0; x < 8; x++) { \
				DRAW_PIXEL(x, Pix = bp[7 - x]); \
			} \
		} \
	} \
	else \
	if (!(Tile & H_FLIP)) \
	{ \
		bp = pCache + 56 - StartLine; \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, bp -= 8, Offset += GFX.PPL) \
		{ \
			for (int x = 0; x < 8; x++) { \
				DRAW_PIXEL(x, Pix = bp[x]); \
			} \
		} \
	} \
	else \
	{ \
		bp = pCache + 56 - StartLine; \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, bp -= 8, Offset += GFX.PPL) \
		{ \
			for (int x = 0; x < 8; x++) { \
				DRAW_PIXEL(x, Pix = bp[7 - x]); \
			} \
		} \
	}

#define NAME1	DrawTile16
#define ARGS	uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount

// Second-level include: Get the DrawTile16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render a clipped tile. Inputs same as above.

#define Z1	GFX.Z1
#define Z2	GFX.Z2

#define DRAW_TILE() \
	uint8			*pCache; \
	int32	l; \
	uint8	*bp, Pix, w; \
	\
	GET_CACHED_TILE(); \
	if (IS_BLANK_TILE()) \
		return; \
	SELECT_PALETTE(); \
	\
	if (!(Tile & (V_FLIP | H_FLIP))) \
	{ \
		bp = pCache + StartLine; \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, bp += 8 , Offset += GFX.PPL) \
		{ \
			w = Width; \
			switch (StartPixel) \
			{ \
				case 0: DRAW_PIXEL(0, Pix = bp[0]); if (!--w) break; /* Fall through */ \
				case 1: DRAW_PIXEL(1, Pix = bp[1]); if (!--w) break; /* Fall through */ \
				case 2: DRAW_PIXEL(2, Pix = bp[2]); if (!--w) break; /* Fall through */ \
				case 3: DRAW_PIXEL(3, Pix = bp[3]); if (!--w) break; /* Fall through */ \
				case 4: DRAW_PIXEL(4, Pix = bp[4]); if (!--w) break; /* Fall through */ \
				case 5: DRAW_PIXEL(5, Pix = bp[5]); if (!--w) break; /* Fall through */ \
				case 6: DRAW_PIXEL(6, Pix = bp[6]); if (!--w) break; /* Fall through */ \
				case 7: DRAW_PIXEL(7, Pix = bp[7]); break; \
			} \
		} \
	} \
	else \
	if (!(Tile & V_FLIP)) \
	{ \
		bp = pCache + StartLine; \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, bp += 8, Offset += GFX.PPL) \
		{ \
			w = Width; \
			switch (StartPixel) \
			{ \
				case 0: DRAW_PIXEL(0, Pix = bp[7]); if (!--w) break; /* Fall through */ \
				case 1: DRAW_PIXEL(1, Pix = bp[6]); if (!--w) break; /* Fall through */ \
				case 2: DRAW_PIXEL(2, Pix = bp[5]); if (!--w) break; /* Fall through */ \
				case 3: DRAW_PIXEL(3, Pix = bp[4]); if (!--w) break; /* Fall through */ \
				case 4: DRAW_PIXEL(4, Pix = bp[3]); if (!--w) break; /* Fall through */ \
				case 5: DRAW_PIXEL(5, Pix = bp[2]); if (!--w) break; /* Fall through */ \
				case 6: DRAW_PIXEL(6, Pix = bp[1]); if (!--w) break; /* Fall through */ \
				case 7: DRAW_PIXEL(7, Pix = bp[0]); break; \
			} \
		} \
	} \
	else \
	if (!(Tile & H_FLIP)) \
	{ \
		bp = pCache + 56 - StartLine; \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, bp -= 8, Offset += GFX.PPL) \
		{ \
			w = Width; \
			switch (StartPixel) \
			{ \
				case 0: DRAW_PIXEL(0, Pix = bp[0]); if (!--w) break; /* Fall through */ \
				case 1: DRAW_PIXEL(1, Pix = bp[1]); if (!--w) break; /* Fall through */ \
				case 2: DRAW_PIXEL(2, Pix = bp[2]); if (!--w) break; /* Fall through */ \
				case 3: DRAW_PIXEL(3, Pix = bp[3]); if (!--w) break; /* Fall through */ \
				case 4: DRAW_PIXEL(4, Pix = bp[4]); if (!--w) break; /* Fall through */ \
				case 5: DRAW_PIXEL(5, Pix = bp[5]); if (!--w) break; /* Fall through */ \
				case 6: DRAW_PIXEL(6, Pix = bp[6]); if (!--w) break; /* Fall through */ \
				case 7: DRAW_PIXEL(7, Pix = bp[7]); break; \
			} \
		} \
	} \
	else \
	{ \
		bp = pCache + 56 - StartLine; \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, bp -= 8, Offset += GFX.PPL) \
		{ \
			w = Width; \
			switch (StartPixel) \
			{ \
				case 0: DRAW_PIXEL(0, Pix = bp[7]); if (!--w) break; /* Fall through */ \
				case 1: DRAW_PIXEL(1, Pix = bp[6]); if (!--w) break; /* Fall through */ \
				case 2: DRAW_PIXEL(2, Pix = bp[5]); if (!--w) break; /* Fall through */ \
				case 3: DRAW_PIXEL(3, Pix = bp[4]); if (!--w) break; /* Fall through */ \
				case 4: DRAW_PIXEL(4, Pix = bp[3]); if (!--w) break; /* Fall through */ \
				case 5: DRAW_PIXEL(5, Pix = bp[2]); if (!--w) break; /* Fall through */ \
				case 6: DRAW_PIXEL(6, Pix = bp[1]); if (!--w) break; /* Fall through */ \
				case 7: DRAW_PIXEL(7, Pix = bp[0]); break; \
			} \
		} \
	}

#define NAME1	DrawClippedTile16
#define ARGS	uint32 Tile, uint32 Offset, uint32 StartPixel, uint32 Width, uint32 StartLine, uint32 LineCount

// Second-level include: Get the DrawClippedTile16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render a single mosaic pixel.
// DRAW_PIXEL, Z1, Z2 and Pix are the same as above

#define Z1	GFX.Z1
#define Z2	GFX.Z2

#define DRAW_TILE() \
	uint8			*pCache; \
	int32	l, w; \
	uint8	Pix; \
	\
	GET_CACHED_TILE(); \
	if (IS_BLANK_TILE()) \
		return; \
	SELECT_PALETTE(); \
	\
	if (Tile & H_FLIP) \
		StartPixel = 7 - StartPixel; \
	\
	if (Tile & V_FLIP) \
		Pix = pCache[56 - StartLine + StartPixel]; \
	else \
		Pix = pCache[StartLine + StartPixel]; \
	\
	if (Pix) \
	{ \
		OFFSET_IN_LINE; \
		for (l = LineCount; l > 0; l--, Offset += GFX.PPL) \
		{ \
			for (w = Width - 1; w >= 0; w--) \
				DRAW_PIXEL(w, 1); \
		} \
	}

#define NAME1	DrawMosaicPixel16
#define ARGS	uint32 Tile, uint32 Offset, uint32 StartLine, uint32 StartPixel, uint32 Width, uint32 LineCount

// Second-level include: Get the DrawMosaicPixel16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render the backdrop.
// DRAW_PIXEL is the same as above
// (or interlace at all, really).
// The backdrop is always depth = 1, so Z1 = Z2 = 1. And backdrop is always color 0.

#define Z1				1
#define Z2				1
#define Pix				0

#define DRAW_TILE() \
	uint32	l, x; \
	\
	GFX.RealScreenColors = IPPU.ScreenColors; \
	GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
	\
	OFFSET_IN_LINE; \
	for (l = GFX.StartY; l <= GFX.EndY; l++, Offset += GFX.PPL) \
	{ \
		for (x = Left; x < Right; x++) \
			DRAW_PIXEL(x, 1); \
	}

#define NAME1	DrawBackdrop16
#define ARGS	uint32 Offset, uint32 Left, uint32 Right

// Second-level include: Get the DrawBackdrop16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Pix
#undef Z1
#undef Z2

// Basic routine to render a chunk of a Mode 7 BG.
// We get some new parameters, so we can use the same DRAW_TILE to do BG1 or BG2:
//     DCMODE tests if Direct Color should apply.
//     BG is the BG, so we use the right clip window.
//     MASK is 0xff or 0x7f, the 'color' portion of the pixel.
// We define Z1/Z2 to either be constant 5 or to vary depending on the 'priority' portion of the pixel.

#define CLIP_10_BIT_SIGNED(a)	(((a) & 0x2000) ? ((a) | ~0x3ff) : ((a) & 0x3ff))

#define Z1				(D + 7)
#define Z2				(D + 7)
#define MASK			0xff
#define DCMODE			(Memory.PPU_IO[0x130] & 1)
#define BG				0

#define DRAW_TILE_NORMAL() \
	uint8	*VRAM1 = Memory.VRAM + 1; \
	\
	if (DCMODE) \
	{ \
		GFX.RealScreenColors = DirectColourMaps[0]; \
	} \
	else \
		GFX.RealScreenColors = IPPU.ScreenColors; \
	\
	GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
	\
	int	aa, cc; \
	int	startx; \
	\
	uint32	Offset = GFX.StartY * GFX.PPL; \
	struct SLineMatrixData	*l = &LineMatrixData[GFX.StartY]; \
	\
	OFFSET_IN_LINE; \
	for (uint32 Line = GFX.StartY; Line <= GFX.EndY; Line++, Offset += GFX.PPL, l++) \
	{ \
		int	yy, starty; \
		\
		int32	HOffset = ((int32) l->M7HOFS  << 19) >> 19; \
		int32	VOffset = ((int32) l->M7VOFS  << 19) >> 19; \
		\
		int32	CentreX = ((int32) l->CentreX << 19) >> 19; \
		int32	CentreY = ((int32) l->CentreY << 19) >> 19; \
		\
		if (PPU.Mode7VFlip) \
			starty = 255 - (int) (Line + 1); \
		else \
			starty = Line + 1; \
		\
		yy = CLIP_10_BIT_SIGNED(VOffset - CentreY); \
		\
		int	BB = ((l->MatrixB * starty) & ~63) + ((l->MatrixB * yy) & ~63) + (CentreX << 8); \
		int	DD = ((l->MatrixD * starty) & ~63) + ((l->MatrixD * yy) & ~63) + (CentreY << 8); \
		\
		if (PPU.Mode7HFlip) \
		{ \
			startx = Right - 1; \
			aa = -l->MatrixA; \
			cc = -l->MatrixC; \
		} \
		else \
		{ \
			startx = Left; \
			aa = l->MatrixA; \
			cc = l->MatrixC; \
		} \
		\
		int	xx = CLIP_10_BIT_SIGNED(HOffset - CentreX); \
		int	AA = l->MatrixA * startx + ((l->MatrixA * xx) & ~63); \
		int	CC = l->MatrixC * startx + ((l->MatrixC * xx) & ~63); \
		\
		uint8	Pix; \
		\
		if (!PPU.Mode7Repeat) \
		{ \
			for (uint32 x = Left; x < Right; x++, AA += aa, CC += cc) \
			{ \
				int	X = ((AA + BB) >> 8) & 0x3ff; \
				int	Y = ((CC + DD) >> 8) & 0x3ff; \
				\
				uint8	*TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
				uint8	b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				\
				DRAW_PIXEL(x, Pix = (b & MASK)); \
			} \
		} \
		else \
		{ \
			for (uint32 x = Left; x < Right; x++, AA += aa, CC += cc) \
			{ \
				int	X = ((AA + BB) >> 8); \
				int	Y = ((CC + DD) >> 8); \
				\
				uint8	b; \
				\
				if (((X | Y) & ~0x3ff) == 0) \
				{ \
					uint8	*TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				} \
				else \
				if (PPU.Mode7Repeat == 3) \
					b = *(VRAM1    + ((Y & 7) << 4) + ((X & 7) << 1)); \
				else \
					continue; \
				\
				DRAW_PIXEL(x, Pix = (b & MASK)); \
			} \
		} \
	}

#define DRAW_TILE_MOSAIC() \
	uint8	*VRAM1 = Memory.VRAM + 1; \
	\
	if (DCMODE) \
	{ \
		GFX.RealScreenColors = DirectColourMaps[0]; \
	} \
	else \
		GFX.RealScreenColors = IPPU.ScreenColors; \
	\
	GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
	\
	int	aa, cc; \
	int	startx, StartY = GFX.StartY; \
	\
	int		HMosaic = 1, VMosaic = 1, MosaicStart = 0; \
	int32	MLeft = Left, MRight = Right; \
	\
	if (PPU.BGMosaic[0]) \
	{ \
		VMosaic = PPU.Mosaic; \
		MosaicStart = ((uint32) GFX.StartY - PPU.MosaicStart) % VMosaic; \
		StartY -= MosaicStart; \
	} \
	\
	if (PPU.BGMosaic[BG]) \
	{ \
		HMosaic = PPU.Mosaic; \
		MLeft  -= MLeft  % HMosaic; \
		MRight += HMosaic - 1; \
		MRight -= MRight % HMosaic; \
	} \
	\
	uint32	Offset = StartY * GFX.PPL; \
	struct SLineMatrixData	*l = &LineMatrixData[StartY]; \
	\
	OFFSET_IN_LINE; \
	for (uint32 Line = StartY; Line <= GFX.EndY; Line += VMosaic, Offset += VMosaic * GFX.PPL, l += VMosaic) \
	{ \
		if (Line + VMosaic > GFX.EndY) \
			VMosaic = GFX.EndY - Line + 1; \
		\
		int	yy, starty; \
		\
		int32	HOffset = ((int32) l->M7HOFS  << 19) >> 19; \
		int32	VOffset = ((int32) l->M7VOFS  << 19) >> 19; \
		\
		int32	CentreX = ((int32) l->CentreX << 19) >> 19; \
		int32	CentreY = ((int32) l->CentreY << 19) >> 19; \
		\
		if (PPU.Mode7VFlip) \
			starty = 255 - (int) (Line + 1); \
		else \
			starty = Line + 1; \
		\
		yy = CLIP_10_BIT_SIGNED(VOffset - CentreY); \
		\
		int	BB = ((l->MatrixB * starty) & ~63) + ((l->MatrixB * yy) & ~63) + (CentreX << 8); \
		int	DD = ((l->MatrixD * starty) & ~63) + ((l->MatrixD * yy) & ~63) + (CentreY << 8); \
		\
		if (PPU.Mode7HFlip) \
		{ \
			startx = MRight - 1; \
			aa = -l->MatrixA; \
			cc = -l->MatrixC; \
		} \
		else \
		{ \
			startx = MLeft; \
			aa = l->MatrixA; \
			cc = l->MatrixC; \
		} \
		\
		int	xx = CLIP_10_BIT_SIGNED(HOffset - CentreX); \
		int	AA = l->MatrixA * startx + ((l->MatrixA * xx) & ~63); \
		int	CC = l->MatrixC * startx + ((l->MatrixC * xx) & ~63); \
		\
		uint8	Pix; \
		uint8	ctr = 1; \
		\
		if (!PPU.Mode7Repeat) \
		{ \
			for (int32 x = MLeft; x < MRight; x++, AA += aa, CC += cc) \
			{ \
				if (--ctr) \
					continue; \
				ctr = HMosaic; \
				\
				int	X = ((AA + BB) >> 8) & 0x3ff; \
				int	Y = ((CC + DD) >> 8) & 0x3ff; \
				\
				uint8	*TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
				uint8	b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				\
				if ((Pix = (b & MASK))) \
				{ \
					for (int32 h = MosaicStart; h < VMosaic; h++) \
					{ \
						for (int32 w = x + HMosaic - 1; w >= x; w--) \
							DRAW_PIXEL(w + h * GFX.PPL, (w >= (int32) Left && w < (int32) Right)); \
					} \
				} \
			} \
		} \
		else \
		{ \
			for (int32 x = MLeft; x < MRight; x++, AA += aa, CC += cc) \
			{ \
				if (--ctr) \
					continue; \
				ctr = HMosaic; \
				\
				int	X = ((AA + BB) >> 8); \
				int	Y = ((CC + DD) >> 8); \
				\
				uint8	b; \
				\
				if (((X | Y) & ~0x3ff) == 0) \
				{ \
					uint8	*TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				} \
				else \
				if (PPU.Mode7Repeat == 3) \
					b = *(VRAM1    + ((Y & 7) << 4) + ((X & 7) << 1)); \
				else \
					continue; \
				\
				if ((Pix = (b & MASK))) \
				{ \
					for (int32 h = MosaicStart; h < VMosaic; h++) \
					{ \
						for (int32 w = x + HMosaic - 1; w >= x; w--) \
							DRAW_PIXEL(w + h * GFX.PPL, (w >= (int32) Left && w < (int32) Right)); \
					} \
				} \
			} \
		} \
		\
		MosaicStart = 0; \
	}

#define DRAW_TILE()	DRAW_TILE_NORMAL()
#define NAME1		DrawMode7BG1
#define ARGS		uint32 Left, uint32 Right, int D

// Second-level include: Get the DrawMode7BG1 renderers.

#include "tile.cpp"

#undef NAME1
#undef DRAW_TILE

#define DRAW_TILE()	DRAW_TILE_MOSAIC()
#define NAME1		DrawMode7MosaicBG1

// Second-level include: Get the DrawMode7MosaicBG1 renderers.

#include "tile.cpp"

#undef DRAW_TILE
#undef NAME1
#undef Z1
#undef Z2
#undef MASK
#undef DCMODE
#undef BG

#define NAME1		DrawMode7BG2
#define DRAW_TILE()	DRAW_TILE_NORMAL()
#define Z1			(D + ((b & 0x80) ? 11 : 3))
#define Z2			(D + ((b & 0x80) ? 11 : 3))
#define MASK		0x7f
#define DCMODE		0
#define BG			1

// Second-level include: Get the DrawMode7BG2 renderers.

#include "tile.cpp"

#undef NAME1
#undef DRAW_TILE

#define DRAW_TILE()	DRAW_TILE_MOSAIC()
#define NAME1		DrawMode7MosaicBG2

// Second-level include: Get the DrawMode7MosaicBG2 renderers.

#include "tile.cpp"

#undef MASK
#undef DCMODE
#undef BG
#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef DRAW_TILE_NORMAL
#undef DRAW_TILE_MOSAIC
#undef Z1
#undef Z2

/*****************************************************************************/
#else
#ifndef NAME2 // Second-level: Get all the NAME1 renderers.
/*****************************************************************************/

// The 1x1 pixel plotter, for speedhacking modes.

#define OFFSET_IN_LINE
#define DRAW_PIXEL(N, M) \
	if (Z1 > GFX.DB[Offset + N] && (M)) \
	{ \
		GFX.S[Offset + N] = MATH(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + N], GFX.SubZBuffer[Offset + N]); \
		GFX.DB[Offset + N] = Z2; \
	}

#define NAME2	Normal1x1

// Third-level include: Get the Normal1x1 renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL

/*****************************************************************************/
#else // Third-level: Renderers for each math mode for NAME1 + NAME2.
/*****************************************************************************/

#define CONCAT3(A, B, C)	A##B##C
#define MAKENAME(A, B, C)	CONCAT3(A, B, C)

static void MAKENAME(NAME1, _, NAME2) (ARGS)
{
#define MATH(A, B, C)	NOMATH(x, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, Add_, NAME2) (ARGS)
{
#define MATH(A, B, C)	REGMATH(ADD, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, Add_Brightness_, NAME2) (ARGS)
{
#define MATH(A, B, C)	REGMATH(ADD_BRIGHTNESS, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, AddF1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHF1_2(ADD, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, AddS1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHS1_2(ADD, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, AddS1_2_Brightness_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHS1_2(ADD_BRIGHTNESS, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, Sub_, NAME2) (ARGS)
{
#define MATH(A, B, C)	REGMATH(SUB, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, SubF1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHF1_2(SUB, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, SubS1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHS1_2(SUB, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void (*MAKENAME(Renderers_, NAME1, NAME2)[9]) (ARGS) =
{
	MAKENAME(NAME1, _, NAME2),
	MAKENAME(NAME1, Add_, NAME2),
	MAKENAME(NAME1, AddF1_2_, NAME2),
	MAKENAME(NAME1, AddS1_2_, NAME2),
	MAKENAME(NAME1, Sub_, NAME2),
	MAKENAME(NAME1, SubF1_2_, NAME2),
	MAKENAME(NAME1, SubS1_2_, NAME2),
	MAKENAME(NAME1, Add_Brightness_, NAME2),
	MAKENAME(NAME1, AddS1_2_Brightness_, NAME2)
};

#undef MAKENAME
#undef CONCAT3

#endif
#endif
#endif
