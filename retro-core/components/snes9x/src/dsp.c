/* This file is part of Snes9x. See LICENSE file. */

#include <string.h>
#include "snes9x.h"
#include "memmap.h"
#include "dsp.h"

static uint8_t (*GetDSP)(uint16_t) = NULL;
static void (*SetDSP)(uint8_t, uint16_t) = NULL;

static struct SDSP1 *DSP1;
static struct SDSP2 *DSP2;
static struct SDSP3	*DSP3;
static struct SDSP4 *DSP4;

void S9xInitDSP(void)
{
	switch (Settings.DSP)
	{
	case 1: /* DSP1 */
		DSP1 = malloc(sizeof(*DSP1));
		SetDSP = &DSP1SetByte;
		GetDSP = &DSP1GetByte;
		break;
	case 2: /* DSP2 */
		DSP2 = malloc(sizeof(*DSP2));
		SetDSP = &DSP2SetByte;
		GetDSP = &DSP2GetByte;
		break;
	case 3: /* DSP3 */
		DSP3 = malloc(sizeof(*DSP3));
		SetDSP = &DSP3SetByte;
		GetDSP = &DSP3GetByte;
		break;
	case 4: /* DSP4 */
		DSP4 = malloc(sizeof(*DSP4));
		SetDSP = &DSP4SetByte;
		GetDSP = &DSP4GetByte;
		break;
	default:
		SetDSP = NULL;
		GetDSP = NULL;
		break;
	}
}

void S9xResetDSP(void)
{
	switch (Settings.DSP)
	{
	case 1: /* DSP1 */
		memset(DSP1, 0, sizeof(*DSP1));
		DSP1->waiting4command = true;
		DSP1->first_parameter = true;
		break;
	case 2: /* DSP2 */
		memset(DSP2, 0, sizeof(*DSP2));
		DSP2->waiting4command = true;
		break;
	case 3: /* DSP3 */
		memset(DSP3, 0, sizeof(*DSP3));
		DSP3_Reset();
		break;
	case 4: /* DSP4 */
		memset(DSP4, 0, sizeof(*DSP4));
		DSP4->waiting4command = true;
		break;
	}
}

uint8_t S9xGetDSP (uint16_t address)
{
	return ((*GetDSP)(address));
}

void S9xSetDSP (uint8_t byte, uint16_t address)
{
	(*SetDSP)(byte, address);
}

/***********************************************************************************
 DSP1
***********************************************************************************/

static const uint16_t	DSP1ROM[1024] =
{
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0001,  0x0002,  0x0004,  0x0008,  0x0010,  0x0020,
	 0x0040,  0x0080,  0x0100,  0x0200,  0x0400,  0x0800,  0x1000,  0x2000,
	 0x4000,  0x7fff,  0x4000,  0x2000,  0x1000,  0x0800,  0x0400,  0x0200,
	 0x0100,  0x0080,  0x0040,  0x0020,  0x0001,  0x0008,  0x0004,  0x0002,
	 0x0001,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x8000,  0xffe5,  0x0100,  0x7fff,  0x7f02,  0x7e08,
	 0x7d12,  0x7c1f,  0x7b30,  0x7a45,  0x795d,  0x7878,  0x7797,  0x76ba,
	 0x75df,  0x7507,  0x7433,  0x7361,  0x7293,  0x71c7,  0x70fe,  0x7038,
	 0x6f75,  0x6eb4,  0x6df6,  0x6d3a,  0x6c81,  0x6bca,  0x6b16,  0x6a64,
	 0x69b4,  0x6907,  0x685b,  0x67b2,  0x670b,  0x6666,  0x65c4,  0x6523,
	 0x6484,  0x63e7,  0x634c,  0x62b3,  0x621c,  0x6186,  0x60f2,  0x6060,
	 0x5fd0,  0x5f41,  0x5eb5,  0x5e29,  0x5d9f,  0x5d17,  0x5c91,  0x5c0c,
	 0x5b88,  0x5b06,  0x5a85,  0x5a06,  0x5988,  0x590b,  0x5890,  0x5816,
	 0x579d,  0x5726,  0x56b0,  0x563b,  0x55c8,  0x5555,  0x54e4,  0x5474,
	 0x5405,  0x5398,  0x532b,  0x52bf,  0x5255,  0x51ec,  0x5183,  0x511c,
	 0x50b6,  0x5050,  0x4fec,  0x4f89,  0x4f26,  0x4ec5,  0x4e64,  0x4e05,
	 0x4da6,  0x4d48,  0x4cec,  0x4c90,  0x4c34,  0x4bda,  0x4b81,  0x4b28,
	 0x4ad0,  0x4a79,  0x4a23,  0x49cd,  0x4979,  0x4925,  0x48d1,  0x487f,
	 0x482d,  0x47dc,  0x478c,  0x473c,  0x46ed,  0x469f,  0x4651,  0x4604,
	 0x45b8,  0x456c,  0x4521,  0x44d7,  0x448d,  0x4444,  0x43fc,  0x43b4,
	 0x436d,  0x4326,  0x42e0,  0x429a,  0x4255,  0x4211,  0x41cd,  0x4189,
	 0x4146,  0x4104,  0x40c2,  0x4081,  0x4040,  0x3fff,  0x41f7,  0x43e1,
	 0x45bd,  0x478d,  0x4951,  0x4b0b,  0x4cbb,  0x4e61,  0x4fff,  0x5194,
	 0x5322,  0x54a9,  0x5628,  0x57a2,  0x5914,  0x5a81,  0x5be9,  0x5d4a,
	 0x5ea7,  0x5fff,  0x6152,  0x62a0,  0x63ea,  0x6530,  0x6672,  0x67b0,
	 0x68ea,  0x6a20,  0x6b53,  0x6c83,  0x6daf,  0x6ed9,  0x6fff,  0x7122,
	 0x7242,  0x735f,  0x747a,  0x7592,  0x76a7,  0x77ba,  0x78cb,  0x79d9,
	 0x7ae5,  0x7bee,  0x7cf5,  0x7dfa,  0x7efe,  0x7fff,  0x0000,  0x0324,
	 0x0647,  0x096a,  0x0c8b,  0x0fab,  0x12c8,  0x15e2,  0x18f8,  0x1c0b,
	 0x1f19,  0x2223,  0x2528,  0x2826,  0x2b1f,  0x2e11,  0x30fb,  0x33de,
	 0x36ba,  0x398c,  0x3c56,  0x3f17,  0x41ce,  0x447a,  0x471c,  0x49b4,
	 0x4c3f,  0x4ebf,  0x5133,  0x539b,  0x55f5,  0x5842,  0x5a82,  0x5cb4,
	 0x5ed7,  0x60ec,  0x62f2,  0x64e8,  0x66cf,  0x68a6,  0x6a6d,  0x6c24,
	 0x6dca,  0x6f5f,  0x70e2,  0x7255,  0x73b5,  0x7504,  0x7641,  0x776c,
	 0x7884,  0x798a,  0x7a7d,  0x7b5d,  0x7c29,  0x7ce3,  0x7d8a,  0x7e1d,
	 0x7e9d,  0x7f09,  0x7f62,  0x7fa7,  0x7fd8,  0x7ff6,  0x7fff,  0x7ff6,
	 0x7fd8,  0x7fa7,  0x7f62,  0x7f09,  0x7e9d,  0x7e1d,  0x7d8a,  0x7ce3,
	 0x7c29,  0x7b5d,  0x7a7d,  0x798a,  0x7884,  0x776c,  0x7641,  0x7504,
	 0x73b5,  0x7255,  0x70e2,  0x6f5f,  0x6dca,  0x6c24,  0x6a6d,  0x68a6,
	 0x66cf,  0x64e8,  0x62f2,  0x60ec,  0x5ed7,  0x5cb4,  0x5a82,  0x5842,
	 0x55f5,  0x539b,  0x5133,  0x4ebf,  0x4c3f,  0x49b4,  0x471c,  0x447a,
	 0x41ce,  0x3f17,  0x3c56,  0x398c,  0x36ba,  0x33de,  0x30fb,  0x2e11,
	 0x2b1f,  0x2826,  0x2528,  0x2223,  0x1f19,  0x1c0b,  0x18f8,  0x15e2,
	 0x12c8,  0x0fab,  0x0c8b,  0x096a,  0x0647,  0x0324,  0x7fff,  0x7ff6,
	 0x7fd8,  0x7fa7,  0x7f62,  0x7f09,  0x7e9d,  0x7e1d,  0x7d8a,  0x7ce3,
	 0x7c29,  0x7b5d,  0x7a7d,  0x798a,  0x7884,  0x776c,  0x7641,  0x7504,
	 0x73b5,  0x7255,  0x70e2,  0x6f5f,  0x6dca,  0x6c24,  0x6a6d,  0x68a6,
	 0x66cf,  0x64e8,  0x62f2,  0x60ec,  0x5ed7,  0x5cb4,  0x5a82,  0x5842,
	 0x55f5,  0x539b,  0x5133,  0x4ebf,  0x4c3f,  0x49b4,  0x471c,  0x447a,
	 0x41ce,  0x3f17,  0x3c56,  0x398c,  0x36ba,  0x33de,  0x30fb,  0x2e11,
	 0x2b1f,  0x2826,  0x2528,  0x2223,  0x1f19,  0x1c0b,  0x18f8,  0x15e2,
	 0x12c8,  0x0fab,  0x0c8b,  0x096a,  0x0647,  0x0324,  0x0000,  0xfcdc,
	 0xf9b9,  0xf696,  0xf375,  0xf055,  0xed38,  0xea1e,  0xe708,  0xe3f5,
	 0xe0e7,  0xdddd,  0xdad8,  0xd7da,  0xd4e1,  0xd1ef,  0xcf05,  0xcc22,
	 0xc946,  0xc674,  0xc3aa,  0xc0e9,  0xbe32,  0xbb86,  0xb8e4,  0xb64c,
	 0xb3c1,  0xb141,  0xaecd,  0xac65,  0xaa0b,  0xa7be,  0xa57e,  0xa34c,
	 0xa129,  0x9f14,  0x9d0e,  0x9b18,  0x9931,  0x975a,  0x9593,  0x93dc,
	 0x9236,  0x90a1,  0x8f1e,  0x8dab,  0x8c4b,  0x8afc,  0x89bf,  0x8894,
	 0x877c,  0x8676,  0x8583,  0x84a3,  0x83d7,  0x831d,  0x8276,  0x81e3,
	 0x8163,  0x80f7,  0x809e,  0x8059,  0x8028,  0x800a,  0x6488,  0x0080,
	 0x03ff,  0x0116,  0x0002,  0x0080,  0x4000,  0x3fd7,  0x3faf,  0x3f86,
	 0x3f5d,  0x3f34,  0x3f0c,  0x3ee3,  0x3eba,  0x3e91,  0x3e68,  0x3e40,
	 0x3e17,  0x3dee,  0x3dc5,  0x3d9c,  0x3d74,  0x3d4b,  0x3d22,  0x3cf9,
	 0x3cd0,  0x3ca7,  0x3c7f,  0x3c56,  0x3c2d,  0x3c04,  0x3bdb,  0x3bb2,
	 0x3b89,  0x3b60,  0x3b37,  0x3b0e,  0x3ae5,  0x3abc,  0x3a93,  0x3a69,
	 0x3a40,  0x3a17,  0x39ee,  0x39c5,  0x399c,  0x3972,  0x3949,  0x3920,
	 0x38f6,  0x38cd,  0x38a4,  0x387a,  0x3851,  0x3827,  0x37fe,  0x37d4,
	 0x37aa,  0x3781,  0x3757,  0x372d,  0x3704,  0x36da,  0x36b0,  0x3686,
	 0x365c,  0x3632,  0x3609,  0x35df,  0x35b4,  0x358a,  0x3560,  0x3536,
	 0x350c,  0x34e1,  0x34b7,  0x348d,  0x3462,  0x3438,  0x340d,  0x33e3,
	 0x33b8,  0x338d,  0x3363,  0x3338,  0x330d,  0x32e2,  0x32b7,  0x328c,
	 0x3261,  0x3236,  0x320b,  0x31df,  0x31b4,  0x3188,  0x315d,  0x3131,
	 0x3106,  0x30da,  0x30ae,  0x3083,  0x3057,  0x302b,  0x2fff,  0x2fd2,
	 0x2fa6,  0x2f7a,  0x2f4d,  0x2f21,  0x2ef4,  0x2ec8,  0x2e9b,  0x2e6e,
	 0x2e41,  0x2e14,  0x2de7,  0x2dba,  0x2d8d,  0x2d60,  0x2d32,  0x2d05,
	 0x2cd7,  0x2ca9,  0x2c7b,  0x2c4d,  0x2c1f,  0x2bf1,  0x2bc3,  0x2b94,
	 0x2b66,  0x2b37,  0x2b09,  0x2ada,  0x2aab,  0x2a7c,  0x2a4c,  0x2a1d,
	 0x29ed,  0x29be,  0x298e,  0x295e,  0x292e,  0x28fe,  0x28ce,  0x289d,
	 0x286d,  0x283c,  0x280b,  0x27da,  0x27a9,  0x2777,  0x2746,  0x2714,
	 0x26e2,  0x26b0,  0x267e,  0x264c,  0x2619,  0x25e7,  0x25b4,  0x2581,
	 0x254d,  0x251a,  0x24e6,  0x24b2,  0x247e,  0x244a,  0x2415,  0x23e1,
	 0x23ac,  0x2376,  0x2341,  0x230b,  0x22d6,  0x229f,  0x2269,  0x2232,
	 0x21fc,  0x21c4,  0x218d,  0x2155,  0x211d,  0x20e5,  0x20ad,  0x2074,
	 0x203b,  0x2001,  0x1fc7,  0x1f8d,  0x1f53,  0x1f18,  0x1edd,  0x1ea1,
	 0x1e66,  0x1e29,  0x1ded,  0x1db0,  0x1d72,  0x1d35,  0x1cf6,  0x1cb8,
	 0x1c79,  0x1c39,  0x1bf9,  0x1bb8,  0x1b77,  0x1b36,  0x1af4,  0x1ab1,
	 0x1a6e,  0x1a2a,  0x19e6,  0x19a1,  0x195c,  0x1915,  0x18ce,  0x1887,
	 0x183f,  0x17f5,  0x17ac,  0x1761,  0x1715,  0x16c9,  0x167c,  0x162e,
	 0x15df,  0x158e,  0x153d,  0x14eb,  0x1497,  0x1442,  0x13ec,  0x1395,
	 0x133c,  0x12e2,  0x1286,  0x1228,  0x11c9,  0x1167,  0x1104,  0x109e,
	 0x1036,  0x0fcc,  0x0f5f,  0x0eef,  0x0e7b,  0x0e04,  0x0d89,  0x0d0a,
	 0x0c86,  0x0bfd,  0x0b6d,  0x0ad6,  0x0a36,  0x098d,  0x08d7,  0x0811,
	 0x0736,  0x063e,  0x0519,  0x039a,  0x0000,  0x7fff,  0x0100,  0x0080,
	 0x021d,  0x00c8,  0x00ce,  0x0048,  0x0a26,  0x277a,  0x00ce,  0x6488,
	 0x14ac,  0x0001,  0x00f9,  0x00fc,  0x00ff,  0x00fc,  0x00f9,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff
};

static const int16_t	DSP1_MulTable[256] =
{
	 0x0000,  0x0003,  0x0006,  0x0009,  0x000c,  0x000f,  0x0012,  0x0015,
	 0x0019,  0x001c,  0x001f,  0x0022,  0x0025,  0x0028,  0x002b,  0x002f,
	 0x0032,  0x0035,  0x0038,  0x003b,  0x003e,  0x0041,  0x0045,  0x0048,
	 0x004b,  0x004e,  0x0051,  0x0054,  0x0057,  0x005b,  0x005e,  0x0061,
	 0x0064,  0x0067,  0x006a,  0x006d,  0x0071,  0x0074,  0x0077,  0x007a,
	 0x007d,  0x0080,  0x0083,  0x0087,  0x008a,  0x008d,  0x0090,  0x0093,
	 0x0096,  0x0099,  0x009d,  0x00a0,  0x00a3,  0x00a6,  0x00a9,  0x00ac,
	 0x00af,  0x00b3,  0x00b6,  0x00b9,  0x00bc,  0x00bf,  0x00c2,  0x00c5,
	 0x00c9,  0x00cc,  0x00cf,  0x00d2,  0x00d5,  0x00d8,  0x00db,  0x00df,
	 0x00e2,  0x00e5,  0x00e8,  0x00eb,  0x00ee,  0x00f1,  0x00f5,  0x00f8,
	 0x00fb,  0x00fe,  0x0101,  0x0104,  0x0107,  0x010b,  0x010e,  0x0111,
	 0x0114,  0x0117,  0x011a,  0x011d,  0x0121,  0x0124,  0x0127,  0x012a,
	 0x012d,  0x0130,  0x0133,  0x0137,  0x013a,  0x013d,  0x0140,  0x0143,
	 0x0146,  0x0149,  0x014d,  0x0150,  0x0153,  0x0156,  0x0159,  0x015c,
	 0x015f,  0x0163,  0x0166,  0x0169,  0x016c,  0x016f,  0x0172,  0x0175,
	 0x0178,  0x017c,  0x017f,  0x0182,  0x0185,  0x0188,  0x018b,  0x018e,
	 0x0192,  0x0195,  0x0198,  0x019b,  0x019e,  0x01a1,  0x01a4,  0x01a8,
	 0x01ab,  0x01ae,  0x01b1,  0x01b4,  0x01b7,  0x01ba,  0x01be,  0x01c1,
	 0x01c4,  0x01c7,  0x01ca,  0x01cd,  0x01d0,  0x01d4,  0x01d7,  0x01da,
	 0x01dd,  0x01e0,  0x01e3,  0x01e6,  0x01ea,  0x01ed,  0x01f0,  0x01f3,
	 0x01f6,  0x01f9,  0x01fc,  0x0200,  0x0203,  0x0206,  0x0209,  0x020c,
	 0x020f,  0x0212,  0x0216,  0x0219,  0x021c,  0x021f,  0x0222,  0x0225,
	 0x0228,  0x022c,  0x022f,  0x0232,  0x0235,  0x0238,  0x023b,  0x023e,
	 0x0242,  0x0245,  0x0248,  0x024b,  0x024e,  0x0251,  0x0254,  0x0258,
	 0x025b,  0x025e,  0x0261,  0x0264,  0x0267,  0x026a,  0x026e,  0x0271,
	 0x0274,  0x0277,  0x027a,  0x027d,  0x0280,  0x0284,  0x0287,  0x028a,
	 0x028d,  0x0290,  0x0293,  0x0296,  0x029a,  0x029d,  0x02a0,  0x02a3,
	 0x02a6,  0x02a9,  0x02ac,  0x02b0,  0x02b3,  0x02b6,  0x02b9,  0x02bc,
	 0x02bf,  0x02c2,  0x02c6,  0x02c9,  0x02cc,  0x02cf,  0x02d2,  0x02d5,
	 0x02d8,  0x02db,  0x02df,  0x02e2,  0x02e5,  0x02e8,  0x02eb,  0x02ee,
	 0x02f1,  0x02f5,  0x02f8,  0x02fb,  0x02fe,  0x0301,  0x0304,  0x0307,
	 0x030b,  0x030e,  0x0311,  0x0314,  0x0317,  0x031a,  0x031d,  0x0321
};

static const int16_t	DSP1_SinTable[256] =
{
	 0x0000,  0x0324,  0x0647,  0x096a,  0x0c8b,  0x0fab,  0x12c8,  0x15e2,
	 0x18f8,  0x1c0b,  0x1f19,  0x2223,  0x2528,  0x2826,  0x2b1f,  0x2e11,
	 0x30fb,  0x33de,  0x36ba,  0x398c,  0x3c56,  0x3f17,  0x41ce,  0x447a,
	 0x471c,  0x49b4,  0x4c3f,  0x4ebf,  0x5133,  0x539b,  0x55f5,  0x5842,
	 0x5a82,  0x5cb4,  0x5ed7,  0x60ec,  0x62f2,  0x64e8,  0x66cf,  0x68a6,
	 0x6a6d,  0x6c24,  0x6dca,  0x6f5f,  0x70e2,  0x7255,  0x73b5,  0x7504,
	 0x7641,  0x776c,  0x7884,  0x798a,  0x7a7d,  0x7b5d,  0x7c29,  0x7ce3,
	 0x7d8a,  0x7e1d,  0x7e9d,  0x7f09,  0x7f62,  0x7fa7,  0x7fd8,  0x7ff6,
	 0x7fff,  0x7ff6,  0x7fd8,  0x7fa7,  0x7f62,  0x7f09,  0x7e9d,  0x7e1d,
	 0x7d8a,  0x7ce3,  0x7c29,  0x7b5d,  0x7a7d,  0x798a,  0x7884,  0x776c,
	 0x7641,  0x7504,  0x73b5,  0x7255,  0x70e2,  0x6f5f,  0x6dca,  0x6c24,
	 0x6a6d,  0x68a6,  0x66cf,  0x64e8,  0x62f2,  0x60ec,  0x5ed7,  0x5cb4,
	 0x5a82,  0x5842,  0x55f5,  0x539b,  0x5133,  0x4ebf,  0x4c3f,  0x49b4,
	 0x471c,  0x447a,  0x41ce,  0x3f17,  0x3c56,  0x398c,  0x36ba,  0x33de,
	 0x30fb,  0x2e11,  0x2b1f,  0x2826,  0x2528,  0x2223,  0x1f19,  0x1c0b,
	 0x18f8,  0x15e2,  0x12c8,  0x0fab,  0x0c8b,  0x096a,  0x0647,  0x0324,
	-0x0000, -0x0324, -0x0647, -0x096a, -0x0c8b, -0x0fab, -0x12c8, -0x15e2,
	-0x18f8, -0x1c0b, -0x1f19, -0x2223, -0x2528, -0x2826, -0x2b1f, -0x2e11,
	-0x30fb, -0x33de, -0x36ba, -0x398c, -0x3c56, -0x3f17, -0x41ce, -0x447a,
	-0x471c, -0x49b4, -0x4c3f, -0x4ebf, -0x5133, -0x539b, -0x55f5, -0x5842,
	-0x5a82, -0x5cb4, -0x5ed7, -0x60ec, -0x62f2, -0x64e8, -0x66cf, -0x68a6,
	-0x6a6d, -0x6c24, -0x6dca, -0x6f5f, -0x70e2, -0x7255, -0x73b5, -0x7504,
	-0x7641, -0x776c, -0x7884, -0x798a, -0x7a7d, -0x7b5d, -0x7c29, -0x7ce3,
	-0x7d8a, -0x7e1d, -0x7e9d, -0x7f09, -0x7f62, -0x7fa7, -0x7fd8, -0x7ff6,
	-0x7fff, -0x7ff6, -0x7fd8, -0x7fa7, -0x7f62, -0x7f09, -0x7e9d, -0x7e1d,
	-0x7d8a, -0x7ce3, -0x7c29, -0x7b5d, -0x7a7d, -0x798a, -0x7884, -0x776c,
	-0x7641, -0x7504, -0x73b5, -0x7255, -0x70e2, -0x6f5f, -0x6dca, -0x6c24,
	-0x6a6d, -0x68a6, -0x66cf, -0x64e8, -0x62f2, -0x60ec, -0x5ed7, -0x5cb4,
	-0x5a82, -0x5842, -0x55f5, -0x539b, -0x5133, -0x4ebf, -0x4c3f, -0x49b4,
	-0x471c, -0x447a, -0x41ce, -0x3f17, -0x3c56, -0x398c, -0x36ba, -0x33de,
	-0x30fb, -0x2e11, -0x2b1f, -0x2826, -0x2528, -0x2223, -0x1f19, -0x1c0b,
	-0x18f8, -0x15e2, -0x12c8, -0x0fab, -0x0c8b, -0x096a, -0x0647, -0x0324
};


static void DSP1_Op00 (void)
{
	DSP1->Op00Result = DSP1->Op00Multiplicand * DSP1->Op00Multiplier >> 15;

}

static void DSP1_Op20 (void)
{
	DSP1->Op20Result = DSP1->Op20Multiplicand * DSP1->Op20Multiplier >> 15;
	DSP1->Op20Result++;

}

static void DSP1_Inverse (int16_t Coefficient, int16_t Exponent, int16_t *iCoefficient, int16_t *iExponent)
{
	/* Step One: Division by Zero */
	if (Coefficient == 0x0000)
	{
		*iCoefficient = 0x7fff;
		*iExponent    = 0x002f;
	}
	else
	{
		int16_t	Sign = 1;

		/* Step Two: Remove Sign */
		if (Coefficient < 0)
		{
			if (Coefficient < -32767)
				Coefficient = -32767;
			Coefficient = -Coefficient;
			Sign = -1;
		}

		/* Step Three: Normalize*/
#ifdef __GNUC__
		{
			const int shift = __builtin_clz(Coefficient) - (8 * sizeof(int) - 15);
			Coefficient <<= shift;
			Exponent -= shift;
		}
#else
		while (Coefficient < 0x4000)
		{
			Coefficient <<= 1;
			Exponent--;
		}
#endif

		/* Step Four: Special Case*/
		if (Coefficient == 0x4000)
		{
			if (Sign == 1)
				*iCoefficient =  0x7fff;
			else
			{
				*iCoefficient = -0x4000;
				Exponent--;
			}
		}
		else
		{
			/* Step Five: Initial Guess*/
			int16_t	i = DSP1ROM[((Coefficient - 0x4000) >> 7) + 0x0065];

			/* Step Six: Iterate "estimated" Newton's Method*/
			i = (i + (-i * (Coefficient * i >> 15) >> 15)) << 1;
			i = (i + (-i * (Coefficient * i >> 15) >> 15)) << 1;

			*iCoefficient = i * Sign;
		}

		*iExponent = 1 - Exponent;
	}
}

static void DSP1_Op10 (void)
{
	DSP1_Inverse(DSP1->Op10Coefficient, DSP1->Op10Exponent, &DSP1->Op10CoefficientR, &DSP1->Op10ExponentR);

}

static int16_t DSP1_Sin (int16_t Angle)
{
	int32_t	S;

	if (Angle < 0)
	{
		if (Angle == -32768)
			return (0);

		return (-DSP1_Sin(-Angle));
	}

	S = DSP1_SinTable[Angle >> 8] + (DSP1_MulTable[Angle & 0xff] * DSP1_SinTable[0x40 + (Angle >> 8)] >> 15);
	if (S > 32767)
		S = 32767;

	return ((int16_t) S);
}

static int16_t DSP1_Cos (int16_t Angle)
{
	int32_t	S;

	if (Angle < 0)
	{
		if (Angle == -32768)
			return (-32768);

		Angle = -Angle;
	}

	S = DSP1_SinTable[0x40 + (Angle >> 8)] - (DSP1_MulTable[Angle & 0xff] * DSP1_SinTable[Angle >> 8] >> 15);
	if (S < -32768)
		S = -32767;

	return ((int16_t) S);
}

static void DSP1_Normalize (int16_t m, int16_t *Coefficient, int16_t *Exponent)
{
	int16_t i, e;

	e = 0;

#ifdef __GNUC__
	i = m < 0 ? ~m : m;

	if (i == 0)
		e = 15;
	else
		e = __builtin_clz(i) - (8 * sizeof(int) - 15);
#else
	i = 0x4000;

	if (m < 0)
	{
		while ((m & i) && i)
		{
			i >>= 1;
			e++;
		}
	}
	else
	{
		while (!(m & i) && i)
		{
			i >>= 1;
			e++;
		}
	}
#endif

	if (e > 0)
		*Coefficient = m * DSP1ROM[0x21 + e] << 1;
	else
		*Coefficient = m;

	*Exponent -= e;
}

static void DSP1_NormalizeDouble (int32_t Product, int16_t *Coefficient, int16_t *Exponent)
{
	int16_t n, m, i,e;

	n = Product & 0x7fff;
	m = Product >> 15;
	e = 0;

#ifdef __GNUC__
	i = m < 0 ? ~m : m;

	if (i == 0)
		e = 15;
	else
		e = __builtin_clz(i) - (8 * sizeof(int) - 15);
#else
	i = 0x4000;

	if (m < 0)
	{
		while ((m & i) && i)
		{
			i >>= 1;
			e++;
		}
	}
	else
	{
		while (!(m & i) && i)
		{
			i >>= 1;
			e++;
		}
	}
#endif

	if (e > 0)
	{
		*Coefficient = m * DSP1ROM[0x0021 + e] << 1;

		if (e < 15)
			*Coefficient += n * DSP1ROM[0x0040 - e] >> 15;
		else
		{
#ifdef __GNUC__
			i = m < 0 ? ~(n | 0x8000) : n;

			if (i == 0)
				e += 15;
			else
				e += __builtin_clz(i) - (8 * sizeof(int) - 15);
#else
			i = 0x4000;

			if (m < 0)
			{
				while ((n & i) && i)
				{
					i >>= 1;
					e++;
				}
			}
			else
			{
				while (!(n & i) && i)
				{
					i >>= 1;
					e++;
				}
			}
#endif

			if (e > 15)
				*Coefficient = n * DSP1ROM[0x0012 + e] << 1;
			else
				*Coefficient += n;
		}
	}
	else
		*Coefficient = m;

	*Exponent = e;
}

static int16_t DSP1_Truncate (int16_t C, int16_t E)
{
	if (E > 0)
	{
		if (C > 0)
			return (32767);
		else
		if (C < 0)
			return (-32767);
	}
	else
	{
		if (E < 0)
			return (C * DSP1ROM[0x0031 + E] >> 15);
	}

	return (C);
}

static void DSP1_Op04 (void)
{
	DSP1->Op04Sin = DSP1_Sin(DSP1->Op04Angle) * DSP1->Op04Radius >> 15;
	DSP1->Op04Cos = DSP1_Cos(DSP1->Op04Angle) * DSP1->Op04Radius >> 15;
}

static void DSP1_Op0C (void)
{
	DSP1->Op0CX2 = (DSP1->Op0CY1 * DSP1_Sin(DSP1->Op0CA) >> 15) + (DSP1->Op0CX1 * DSP1_Cos(DSP1->Op0CA) >> 15);
	DSP1->Op0CY2 = (DSP1->Op0CY1 * DSP1_Cos(DSP1->Op0CA) >> 15) - (DSP1->Op0CX1 * DSP1_Sin(DSP1->Op0CA) >> 15);
}

static void DSP1_Parameter (int16_t Fx, int16_t Fy, int16_t Fz, int16_t Lfe, int16_t Les, int16_t Aas, int16_t Azs, int16_t *Vof, int16_t *Vva, int16_t *Cx, int16_t *Cy)
{
	static const int16_t	MaxAZS_Exp[16] =
	{
		0x38b4, 0x38b7, 0x38ba, 0x38be, 0x38c0, 0x38c4, 0x38c7, 0x38ca,
		0x38ce,	0x38d0, 0x38d4, 0x38d7, 0x38da, 0x38dd, 0x38e0, 0x38e4
	};

	int16_t	CSec, C, E, MaxAZS, Aux;
	int16_t	LfeNx, LfeNy, LfeNz;
	int16_t	LesNx, LesNy, LesNz;
	int16_t	CentreZ, AZS;

	/* Copy Zenith angle for clipping*/
	AZS = Azs;

	/* Store Sine and Cosine of Azimuth and Zenith angle*/
	DSP1->SinAas = DSP1_Sin(Aas);
	DSP1->CosAas = DSP1_Cos(Aas);
	DSP1->SinAzs = DSP1_Sin(Azs);
	DSP1->CosAzs = DSP1_Cos(Azs);

	DSP1->Nx = DSP1->SinAzs * -DSP1->SinAas >> 15;
	DSP1->Ny = DSP1->SinAzs *  DSP1->CosAas >> 15;
	DSP1->Nz = DSP1->CosAzs *  0x7fff >> 15;

	LfeNx = Lfe * DSP1->Nx >> 15;
	LfeNy = Lfe * DSP1->Ny >> 15;
	LfeNz = Lfe * DSP1->Nz >> 15;

	/* Center of Projection*/
	DSP1->CentreX = Fx + LfeNx;
	DSP1->CentreY = Fy + LfeNy;
	CentreZ = Fz + LfeNz;

	LesNx = Les * DSP1->Nx >> 15;
	LesNy = Les * DSP1->Ny >> 15;
	LesNz = Les * DSP1->Nz >> 15;

	DSP1->Gx = DSP1->CentreX - LesNx;
	DSP1->Gy = DSP1->CentreY - LesNy;
	DSP1->Gz = CentreZ - LesNz;

	DSP1->E_Les = 0;
	DSP1_Normalize(Les, &DSP1->C_Les, &DSP1->E_Les);
	DSP1->G_Les = Les;

	E = 0;
	DSP1_Normalize(CentreZ, &C, &E);

	DSP1->VPlane_C = C;
	DSP1->VPlane_E = E;

	/* Determine clip boundary and clip Zenith angle if necessary*/
	MaxAZS = MaxAZS_Exp[-E];

	if (AZS < 0)
	{
		MaxAZS = -MaxAZS;
		if (AZS < MaxAZS + 1)
			AZS = MaxAZS + 1;
	}
	else
	{
		if (AZS > MaxAZS)
			AZS = MaxAZS;
	}

	/* Store Sine and Cosine of clipped Zenith angle*/
	DSP1->SinAZS = DSP1_Sin(AZS);
	DSP1->CosAZS = DSP1_Cos(AZS);

	DSP1_Inverse(DSP1->CosAZS, 0, &DSP1->SecAZS_C1, &DSP1->SecAZS_E1);
	DSP1_Normalize(C * DSP1->SecAZS_C1 >> 15, &C, &E);
	E += DSP1->SecAZS_E1;

	C = DSP1_Truncate(C, E) * DSP1->SinAZS >> 15;

	DSP1->CentreX += C * DSP1->SinAas >> 15;
	DSP1->CentreY -= C * DSP1->CosAas >> 15;

	*Cx = DSP1->CentreX;
	*Cy = DSP1->CentreY;

	/* Raster number of imaginary center and horizontal line*/
	*Vof = 0;

	if ((Azs != AZS) || (Azs == MaxAZS))
	{
		if (Azs == -32768)
			Azs = -32767;

		C = Azs - MaxAZS;
		if (C >= 0)
			C--;
		Aux = ~(C << 2);

		C = Aux * DSP1ROM[0x0328] >> 15;
		C = (C * Aux >> 15) + DSP1ROM[0x0327];
		*Vof -= (C * Aux >> 15) * Les >> 15;

		C = Aux * Aux >> 15;
		Aux = (C * DSP1ROM[0x0324] >> 15) + DSP1ROM[0x0325];
		DSP1->CosAZS += (C * Aux >> 15) * DSP1->CosAZS >> 15;
	}

	DSP1->VOffset = Les * DSP1->CosAZS >> 15;

	DSP1_Inverse(DSP1->SinAZS, 0, &CSec, &E);
	DSP1_Normalize(DSP1->VOffset, &C, &E);
	DSP1_Normalize(C * CSec >> 15, &C, &E);

	if (C == -32768)
	{
		C >>= 1;
		E++;
	}

	*Vva = DSP1_Truncate(-C, E);

	/* Store Secant of clipped Zenith angle*/
	DSP1_Inverse(DSP1->CosAZS, 0, &DSP1->SecAZS_C2, &DSP1->SecAZS_E2);
}

static void DSP1_Raster (int16_t Vs, int16_t *An, int16_t *Bn, int16_t *Cn, int16_t *Dn)
{
	int16_t	C, E, C1, E1;

	DSP1_Inverse((Vs * DSP1->SinAzs >> 15) + DSP1->VOffset, 7, &C, &E);
	E += DSP1->VPlane_E;

	C1 = C * DSP1->VPlane_C >> 15;
	E1 = E + DSP1->SecAZS_E2;

	DSP1_Normalize(C1, &C, &E);

	C = DSP1_Truncate(C, E);

	*An = C *  DSP1->CosAas >> 15;
	*Cn = C *  DSP1->SinAas >> 15;

	DSP1_Normalize(C1 * DSP1->SecAZS_C2 >> 15, &C, &E1);

	C = DSP1_Truncate(C, E1);

	*Bn = C * -DSP1->SinAas >> 15;
	*Dn = C *  DSP1->CosAas >> 15;
}

static void DSP1_Op02 (void)
{
	DSP1_Parameter(DSP1->Op02FX, DSP1->Op02FY, DSP1->Op02FZ, DSP1->Op02LFE, DSP1->Op02LES, DSP1->Op02AAS, DSP1->Op02AZS, &DSP1->Op02VOF, &DSP1->Op02VVA, &DSP1->Op02CX, &DSP1->Op02CY);
}

static void DSP1_Op0A (void)
{
	DSP1_Raster(DSP1->Op0AVS, &DSP1->Op0AA, &DSP1->Op0AB, &DSP1->Op0AC, &DSP1->Op0AD);
	DSP1->Op0AVS++;
}

static int16_t DSP1_ShiftR (int16_t C, int16_t E)
{
	return (C * DSP1ROM[0x0031 + E] >> 15);
}

static void DSP1_Project (int16_t X, int16_t Y, int16_t Z, int16_t *H, int16_t *V, int16_t *M)
{
	int32_t	aux, aux4;
	int16_t	E, E2, E3, E4, E5, refE, E6, E7;
	int16_t	C2, C4, C6, C8, C9, C10, C11, C12, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26;
	int16_t	Px, Py, Pz;

	E4 = E3 = E2 = E = E5 = 0;

	DSP1_NormalizeDouble((int32_t) X - DSP1->Gx, &Px, &E4);
	DSP1_NormalizeDouble((int32_t) Y - DSP1->Gy, &Py, &E );
	DSP1_NormalizeDouble((int32_t) Z - DSP1->Gz, &Pz, &E3);
	Px >>= 1; /* to avoid overflows when calculating the scalar products*/
	E4--;
	Py >>= 1;
	E--;
	Pz >>= 1;
	E3--;

	refE = (E < E3) ? E : E3;
	refE = (refE < E4) ? refE : E4;

	Px = DSP1_ShiftR(Px, E4 - refE); /* normalize them to the same exponent*/
	Py = DSP1_ShiftR(Py, E  - refE);
	Pz = DSP1_ShiftR(Pz, E3 - refE);

	C11 =- (Px * DSP1->Nx >> 15);
	C8  =- (Py * DSP1->Ny >> 15);
	C9  =- (Pz * DSP1->Nz >> 15);
	C12 = C11 + C8 + C9; /* this cannot overflow!*/

	aux4 = C12; /* de-normalization with 32-bits arithmetic*/
	refE = 16 - refE; /* refE can be up to 3*/
	if (refE >= 0)
		aux4 <<=  (refE);
	else
		aux4 >>= -(refE);
	if (aux4 == -1)
		aux4 = 0; /* why?*/
	aux4 >>= 1;

	aux = ((uint16_t) DSP1->G_Les) + aux4; /* Les - the scalar product of P with the normal vector of the screen*/
	DSP1_NormalizeDouble(aux, &C10, &E2);
	E2 = 15 - E2;

	DSP1_Inverse(C10, 0, &C4, &E4);
	C2 = C4 * DSP1->C_Les >> 15; /* scale factor*/

	/* H*/
	E7 = 0;
	C16 = Px * ( DSP1->CosAas *  0x7fff >> 15) >> 15;
	C20 = Py * ( DSP1->SinAas *  0x7fff >> 15) >> 15;
	C17 = C16 + C20; /* scalar product of P with the normalized horizontal vector of the screen...*/

	C18 = C17 * C2 >> 15; /* ... multiplied by the scale factor*/
	DSP1_Normalize(C18, &C19, &E7);
	*H = DSP1_Truncate(C19, DSP1->E_Les - E2 + refE + E7);

	/* V*/
	E6 = 0;
	C21 = Px * ( DSP1->CosAzs * -DSP1->SinAas >> 15) >> 15;
	C22 = Py * ( DSP1->CosAzs *  DSP1->CosAas >> 15) >> 15;
	C23 = Pz * (-DSP1->SinAzs *  0x7fff >> 15) >> 15;
	C24 = C21 + C22 + C23; /* scalar product of P with the normalized vertical vector of the screen...*/

	C26 = C24 * C2 >> 15; /* ... multiplied by the scale factor*/
	DSP1_Normalize(C26, &C25, &E6);
	*V = DSP1_Truncate(C25, DSP1->E_Les - E2 + refE + E6);

	/* M*/
	DSP1_Normalize(C2, &C6, &E4);
	*M = DSP1_Truncate(C6, E4 + DSP1->E_Les - E2 - 7); /* M is the scale factor divided by 2^7*/
}

static void DSP1_Op06 (void)
{
	DSP1_Project(DSP1->Op06X, DSP1->Op06Y, DSP1->Op06Z, &DSP1->Op06H, &DSP1->Op06V, &DSP1->Op06M);
}

static void DSP1_Op01 (void)
{
	int16_t SinAz, CosAz, SinAy, CosAy, SinAx, CosAx;

	SinAz = DSP1_Sin(DSP1->Op01Zr);
	CosAz = DSP1_Cos(DSP1->Op01Zr);
	SinAy = DSP1_Sin(DSP1->Op01Yr);
	CosAy = DSP1_Cos(DSP1->Op01Yr);
	SinAx = DSP1_Sin(DSP1->Op01Xr);
	CosAx = DSP1_Cos(DSP1->Op01Xr);

	DSP1->Op01m >>= 1;

	DSP1->matrixA[0][0] =   (DSP1->Op01m * CosAz >> 15) * CosAy >> 15;
	DSP1->matrixA[0][1] = -((DSP1->Op01m * SinAz >> 15) * CosAy >> 15);
	DSP1->matrixA[0][2] =    DSP1->Op01m * SinAy >> 15;

	DSP1->matrixA[1][0] =  ((DSP1->Op01m * SinAz >> 15) * CosAx >> 15) + (((DSP1->Op01m * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1->matrixA[1][1] =  ((DSP1->Op01m * CosAz >> 15) * CosAx >> 15) - (((DSP1->Op01m * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1->matrixA[1][2] = -((DSP1->Op01m * SinAx >> 15) * CosAy >> 15);

	DSP1->matrixA[2][0] =  ((DSP1->Op01m * SinAz >> 15) * SinAx >> 15) - (((DSP1->Op01m * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1->matrixA[2][1] =  ((DSP1->Op01m * CosAz >> 15) * SinAx >> 15) + (((DSP1->Op01m * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1->matrixA[2][2] =   (DSP1->Op01m * CosAx >> 15) * CosAy >> 15;
}

static void DSP1_Op11 (void)
{
	int16_t SinAz, CosAz, SinAy, CosAy, SinAx, CosAx;

	SinAz = DSP1_Sin(DSP1->Op11Zr);
	CosAz = DSP1_Cos(DSP1->Op11Zr);
	SinAy = DSP1_Sin(DSP1->Op11Yr);
	CosAy = DSP1_Cos(DSP1->Op11Yr);
	SinAx = DSP1_Sin(DSP1->Op11Xr);
	CosAx = DSP1_Cos(DSP1->Op11Xr);

	DSP1->Op11m >>= 1;

	DSP1->matrixB[0][0] =   (DSP1->Op11m * CosAz >> 15) * CosAy >> 15;
	DSP1->matrixB[0][1] = -((DSP1->Op11m * SinAz >> 15) * CosAy >> 15);
	DSP1->matrixB[0][2] =    DSP1->Op11m * SinAy >> 15;

	DSP1->matrixB[1][0] =  ((DSP1->Op11m * SinAz >> 15) * CosAx >> 15) + (((DSP1->Op11m * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1->matrixB[1][1] =  ((DSP1->Op11m * CosAz >> 15) * CosAx >> 15) - (((DSP1->Op11m * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1->matrixB[1][2] = -((DSP1->Op11m * SinAx >> 15) * CosAy >> 15);

	DSP1->matrixB[2][0] =  ((DSP1->Op11m * SinAz >> 15) * SinAx >> 15) - (((DSP1->Op11m * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1->matrixB[2][1] =  ((DSP1->Op11m * CosAz >> 15) * SinAx >> 15) + (((DSP1->Op11m * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1->matrixB[2][2] =   (DSP1->Op11m * CosAx >> 15) * CosAy >> 15;
}

static void DSP1_Op21 (void)
{
	int16_t SinAz, CosAz, SinAy, CosAy, SinAx, CosAx;

	SinAz = DSP1_Sin(DSP1->Op21Zr);
	CosAz = DSP1_Cos(DSP1->Op21Zr);
	SinAy = DSP1_Sin(DSP1->Op21Yr);
	CosAy = DSP1_Cos(DSP1->Op21Yr);
	SinAx = DSP1_Sin(DSP1->Op21Xr);
	CosAx = DSP1_Cos(DSP1->Op21Xr);

	DSP1->Op21m >>= 1;

	DSP1->matrixC[0][0] =   (DSP1->Op21m * CosAz >> 15) * CosAy >> 15;
	DSP1->matrixC[0][1] = -((DSP1->Op21m * SinAz >> 15) * CosAy >> 15);
	DSP1->matrixC[0][2] =    DSP1->Op21m * SinAy >> 15;

	DSP1->matrixC[1][0] =  ((DSP1->Op21m * SinAz >> 15) * CosAx >> 15) + (((DSP1->Op21m * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1->matrixC[1][1] =  ((DSP1->Op21m * CosAz >> 15) * CosAx >> 15) - (((DSP1->Op21m * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1->matrixC[1][2] = -((DSP1->Op21m * SinAx >> 15) * CosAy >> 15);

	DSP1->matrixC[2][0] =  ((DSP1->Op21m * SinAz >> 15) * SinAx >> 15) - (((DSP1->Op21m * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1->matrixC[2][1] =  ((DSP1->Op21m * CosAz >> 15) * SinAx >> 15) + (((DSP1->Op21m * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1->matrixC[2][2] =   (DSP1->Op21m * CosAx >> 15) * CosAy >> 15;
}

static void DSP1_Op0D (void)
{
	DSP1->Op0DF = (DSP1->Op0DX * DSP1->matrixA[0][0] >> 15) + (DSP1->Op0DY * DSP1->matrixA[0][1] >> 15) + (DSP1->Op0DZ * DSP1->matrixA[0][2] >> 15);
	DSP1->Op0DL = (DSP1->Op0DX * DSP1->matrixA[1][0] >> 15) + (DSP1->Op0DY * DSP1->matrixA[1][1] >> 15) + (DSP1->Op0DZ * DSP1->matrixA[1][2] >> 15);
	DSP1->Op0DU = (DSP1->Op0DX * DSP1->matrixA[2][0] >> 15) + (DSP1->Op0DY * DSP1->matrixA[2][1] >> 15) + (DSP1->Op0DZ * DSP1->matrixA[2][2] >> 15);

}

static void DSP1_Op1D (void)
{
	DSP1->Op1DF = (DSP1->Op1DX * DSP1->matrixB[0][0] >> 15) + (DSP1->Op1DY * DSP1->matrixB[0][1] >> 15) + (DSP1->Op1DZ * DSP1->matrixB[0][2] >> 15);
	DSP1->Op1DL = (DSP1->Op1DX * DSP1->matrixB[1][0] >> 15) + (DSP1->Op1DY * DSP1->matrixB[1][1] >> 15) + (DSP1->Op1DZ * DSP1->matrixB[1][2] >> 15);
	DSP1->Op1DU = (DSP1->Op1DX * DSP1->matrixB[2][0] >> 15) + (DSP1->Op1DY * DSP1->matrixB[2][1] >> 15) + (DSP1->Op1DZ * DSP1->matrixB[2][2] >> 15);

}

static void DSP1_Op2D (void)
{
	DSP1->Op2DF = (DSP1->Op2DX * DSP1->matrixC[0][0] >> 15) + (DSP1->Op2DY * DSP1->matrixC[0][1] >> 15) + (DSP1->Op2DZ * DSP1->matrixC[0][2] >> 15);
	DSP1->Op2DL = (DSP1->Op2DX * DSP1->matrixC[1][0] >> 15) + (DSP1->Op2DY * DSP1->matrixC[1][1] >> 15) + (DSP1->Op2DZ * DSP1->matrixC[1][2] >> 15);
	DSP1->Op2DU = (DSP1->Op2DX * DSP1->matrixC[2][0] >> 15) + (DSP1->Op2DY * DSP1->matrixC[2][1] >> 15) + (DSP1->Op2DZ * DSP1->matrixC[2][2] >> 15);

}

static void DSP1_Op03 (void)
{
	DSP1->Op03X = (DSP1->Op03F * DSP1->matrixA[0][0] >> 15) + (DSP1->Op03L * DSP1->matrixA[1][0] >> 15) + (DSP1->Op03U * DSP1->matrixA[2][0] >> 15);
	DSP1->Op03Y = (DSP1->Op03F * DSP1->matrixA[0][1] >> 15) + (DSP1->Op03L * DSP1->matrixA[1][1] >> 15) + (DSP1->Op03U * DSP1->matrixA[2][1] >> 15);
	DSP1->Op03Z = (DSP1->Op03F * DSP1->matrixA[0][2] >> 15) + (DSP1->Op03L * DSP1->matrixA[1][2] >> 15) + (DSP1->Op03U * DSP1->matrixA[2][2] >> 15);

}

static void DSP1_Op13 (void)
{
	DSP1->Op13X = (DSP1->Op13F * DSP1->matrixB[0][0] >> 15) + (DSP1->Op13L * DSP1->matrixB[1][0] >> 15) + (DSP1->Op13U * DSP1->matrixB[2][0] >> 15);
	DSP1->Op13Y = (DSP1->Op13F * DSP1->matrixB[0][1] >> 15) + (DSP1->Op13L * DSP1->matrixB[1][1] >> 15) + (DSP1->Op13U * DSP1->matrixB[2][1] >> 15);
	DSP1->Op13Z = (DSP1->Op13F * DSP1->matrixB[0][2] >> 15) + (DSP1->Op13L * DSP1->matrixB[1][2] >> 15) + (DSP1->Op13U * DSP1->matrixB[2][2] >> 15);

}

static void DSP1_Op23 (void)
{
	DSP1->Op23X = (DSP1->Op23F * DSP1->matrixC[0][0] >> 15) + (DSP1->Op23L * DSP1->matrixC[1][0] >> 15) + (DSP1->Op23U * DSP1->matrixC[2][0] >> 15);
	DSP1->Op23Y = (DSP1->Op23F * DSP1->matrixC[0][1] >> 15) + (DSP1->Op23L * DSP1->matrixC[1][1] >> 15) + (DSP1->Op23U * DSP1->matrixC[2][1] >> 15);
	DSP1->Op23Z = (DSP1->Op23F * DSP1->matrixC[0][2] >> 15) + (DSP1->Op23L * DSP1->matrixC[1][2] >> 15) + (DSP1->Op23U * DSP1->matrixC[2][2] >> 15);

}

static void DSP1_Op14 (void)
{
	int16_t	CSec, ESec, CTan, CSin, C, E;

	DSP1_Inverse(DSP1_Cos(DSP1->Op14Xr), 0, &CSec, &ESec);

	/* Rotation Around Z*/
	DSP1_NormalizeDouble(DSP1->Op14U * DSP1_Cos(DSP1->Op14Yr) - DSP1->Op14F * DSP1_Sin(DSP1->Op14Yr), &C, &E);

	E = ESec - E;

	DSP1_Normalize(C * CSec >> 15, &C, &E);

	DSP1->Op14Zrr = DSP1->Op14Zr + DSP1_Truncate(C, E);

	/* Rotation Around X*/
	DSP1->Op14Xrr = DSP1->Op14Xr + (DSP1->Op14U * DSP1_Sin(DSP1->Op14Yr) >> 15) + (DSP1->Op14F * DSP1_Cos(DSP1->Op14Yr) >> 15);

	/* Rotation Around Y*/
	DSP1_NormalizeDouble(DSP1->Op14U * DSP1_Cos(DSP1->Op14Yr) + DSP1->Op14F * DSP1_Sin(DSP1->Op14Yr), &C, &E);

	E = ESec - E;

	DSP1_Normalize(DSP1_Sin(DSP1->Op14Xr), &CSin, &E);

	CTan = CSec * CSin >> 15;

	DSP1_Normalize(-(C * CTan >> 15), &C, &E);

	DSP1->Op14Yrr = DSP1->Op14Yr + DSP1_Truncate(C, E) + DSP1->Op14L;
}

static void DSP1_Target (int16_t H, int16_t V, int16_t *X, int16_t *Y)
{
	int16_t	C, E, C1, E1;

	DSP1_Inverse((V * DSP1->SinAzs >> 15) + DSP1->VOffset, 8, &C, &E);
	E += DSP1->VPlane_E;

	C1 = C * DSP1->VPlane_C >> 15;
	E1 = E + DSP1->SecAZS_E1;

	H <<= 8;

	DSP1_Normalize(C1, &C, &E);

	C = DSP1_Truncate(C, E) * H >> 15;

	*X = DSP1->CentreX + (C * DSP1->CosAas >> 15);
	*Y = DSP1->CentreY - (C * DSP1->SinAas >> 15);

	V <<= 8;

	DSP1_Normalize(C1 * DSP1->SecAZS_C1 >> 15, &C, &E1);

	C = DSP1_Truncate(C, E1) * V >> 15;

	*X += C * -DSP1->SinAas >> 15;
	*Y += C *  DSP1->CosAas >> 15;
}

static void DSP1_Op0E (void)
{
	DSP1_Target(DSP1->Op0EH, DSP1->Op0EV, &DSP1->Op0EX, &DSP1->Op0EY);
}

static void DSP1_Op0B (void)
{
	DSP1->Op0BS = (DSP1->Op0BX * DSP1->matrixA[0][0] + DSP1->Op0BY * DSP1->matrixA[0][1] + DSP1->Op0BZ * DSP1->matrixA[0][2]) >> 15;

}

static void DSP1_Op1B (void)
{
	DSP1->Op1BS = (DSP1->Op1BX * DSP1->matrixB[0][0] + DSP1->Op1BY * DSP1->matrixB[0][1] + DSP1->Op1BZ * DSP1->matrixB[0][2]) >> 15;

}

static void DSP1_Op2B (void)
{
	DSP1->Op2BS = (DSP1->Op2BX * DSP1->matrixC[0][0] + DSP1->Op2BY * DSP1->matrixC[0][1] + DSP1->Op2BZ * DSP1->matrixC[0][2]) >> 15;

}

static void DSP1_Op08 (void)
{
	int32_t	op08Size = (DSP1->Op08X * DSP1->Op08X + DSP1->Op08Y * DSP1->Op08Y + DSP1->Op08Z * DSP1->Op08Z) << 1;
	DSP1->Op08Ll =  op08Size        & 0xffff;
	DSP1->Op08Lh = (op08Size >> 16) & 0xffff;

}

static void DSP1_Op18 (void)
{
	DSP1->Op18D = (DSP1->Op18X * DSP1->Op18X + DSP1->Op18Y * DSP1->Op18Y + DSP1->Op18Z * DSP1->Op18Z - DSP1->Op18R * DSP1->Op18R) >> 15;

}

static void DSP1_Op38 (void)
{
	DSP1->Op38D = (DSP1->Op38X * DSP1->Op38X + DSP1->Op38Y * DSP1->Op38Y + DSP1->Op38Z * DSP1->Op38Z - DSP1->Op38R * DSP1->Op38R) >> 15;
	DSP1->Op38D++;

}

static void DSP1_Op28 (void)
{
	int32_t	Radius = DSP1->Op28X * DSP1->Op28X + DSP1->Op28Y * DSP1->Op28Y + DSP1->Op28Z * DSP1->Op28Z;

	if (Radius == 0)
		DSP1->Op28R = 0;
	else
	{
		int16_t	C, E, Pos, Node1, Node2;

		DSP1_NormalizeDouble(Radius, &C, &E);
		if (E & 1)
			C = C * 0x4000 >> 15;

		Pos = C * 0x0040 >> 15;

		Node1 = DSP1ROM[0x00d5 + Pos];
		Node2 = DSP1ROM[0x00d6 + Pos];

		DSP1->Op28R = ((Node2 - Node1) * (C & 0x1ff) >> 9) + Node1;
		DSP1->Op28R >>= (E >> 1);
	}

}

static void DSP1_Op1C (void)
{
	/* Rotate Around Op1CZ1*/
	DSP1->Op1CX1 = (DSP1->Op1CYBR * DSP1_Sin(DSP1->Op1CZ) >> 15) + (DSP1->Op1CXBR * DSP1_Cos(DSP1->Op1CZ) >> 15);
	DSP1->Op1CY1 = (DSP1->Op1CYBR * DSP1_Cos(DSP1->Op1CZ) >> 15) - (DSP1->Op1CXBR * DSP1_Sin(DSP1->Op1CZ) >> 15);
	DSP1->Op1CXBR = DSP1->Op1CX1;
	DSP1->Op1CYBR = DSP1->Op1CY1;

	/* Rotate Around Op1CY1*/
	DSP1->Op1CZ1 = (DSP1->Op1CXBR * DSP1_Sin(DSP1->Op1CY) >> 15) + (DSP1->Op1CZBR * DSP1_Cos(DSP1->Op1CY) >> 15);
	DSP1->Op1CX1 = (DSP1->Op1CXBR * DSP1_Cos(DSP1->Op1CY) >> 15) - (DSP1->Op1CZBR * DSP1_Sin(DSP1->Op1CY) >> 15);
	DSP1->Op1CXAR = DSP1->Op1CX1;
	DSP1->Op1CZBR = DSP1->Op1CZ1;

	/* Rotate Around Op1CX1*/
	DSP1->Op1CY1 = (DSP1->Op1CZBR * DSP1_Sin(DSP1->Op1CX) >> 15) + (DSP1->Op1CYBR * DSP1_Cos(DSP1->Op1CX) >> 15);
	DSP1->Op1CZ1 = (DSP1->Op1CZBR * DSP1_Cos(DSP1->Op1CX) >> 15) - (DSP1->Op1CYBR * DSP1_Sin(DSP1->Op1CX) >> 15);
	DSP1->Op1CYAR = DSP1->Op1CY1;
	DSP1->Op1CZAR = DSP1->Op1CZ1;

}

static void DSP1_Op0F (void)
{
	DSP1->Op0FPass = 0x0000;

}

static void DSP1_Op2F (void)
{
	DSP1->Op2FSize = 0x100;
}

void DSP1SetByte (uint8_t byte, uint16_t address)
{
	if ((address & 0xf000) == 0x6000 || (address & 0x7fff) < 0x4000)
	{
		if ((DSP1->command == 0x0A || DSP1->command == 0x1A) && DSP1->out_count != 0)
		{
			DSP1->out_count--;
			DSP1->out_index++;
			return;
		}
		else
		if (DSP1->waiting4command)
		{
			DSP1->command         = byte;
			DSP1->in_index        = 0;
			DSP1->waiting4command = false;
			DSP1->first_parameter = true;

			switch (byte)
			{
				case 0x00: DSP1->in_count = 2; break;
				case 0x30:
				case 0x10: DSP1->in_count = 2; break;
				case 0x20: DSP1->in_count = 2; break;
				case 0x24:
				case 0x04: DSP1->in_count = 2; break;
				case 0x08: DSP1->in_count = 3; break;
				case 0x18: DSP1->in_count = 4; break;
				case 0x28: DSP1->in_count = 3; break;
				case 0x38: DSP1->in_count = 4; break;
				case 0x2c:
				case 0x0c: DSP1->in_count = 3; break;
				case 0x3c:
				case 0x1c: DSP1->in_count = 6; break;
				case 0x32:
				case 0x22:
				case 0x12:
				case 0x02: DSP1->in_count = 7; break;
				case 0x0a: DSP1->in_count = 1; break;
				case 0x3a:
				case 0x2a:
				case 0x1a:
					DSP1->command = 0x1a;
					DSP1->in_count = 1;
					break;
				case 0x16:
				case 0x26:
				case 0x36:
				case 0x06: DSP1->in_count = 3; break;
				case 0x1e:
				case 0x2e:
				case 0x3e:
				case 0x0e: DSP1->in_count = 2; break;
				case 0x05:
				case 0x35:
				case 0x31:
				case 0x01: DSP1->in_count = 4; break;
				case 0x15:
				case 0x11: DSP1->in_count = 4; break;
				case 0x25:
				case 0x21: DSP1->in_count = 4; break;
				case 0x09:
				case 0x39:
				case 0x3d:
				case 0x0d: DSP1->in_count = 3; break;
				case 0x19:
				case 0x1d: DSP1->in_count = 3; break;
				case 0x29:
				case 0x2d: DSP1->in_count = 3; break;
				case 0x33:
				case 0x03: DSP1->in_count = 3; break;
				case 0x13: DSP1->in_count = 3; break;
				case 0x23: DSP1->in_count = 3; break;
				case 0x3b:
				case 0x0b: DSP1->in_count = 3; break;
				case 0x1b: DSP1->in_count = 3; break;
				case 0x2b: DSP1->in_count = 3; break;
				case 0x34:
				case 0x14: DSP1->in_count = 6; break;
				case 0x07:
				case 0x0f: DSP1->in_count = 1; break;
				case 0x27:
				case 0x2F: DSP1->in_count = 1; break;
				case 0x17:
				case 0x37:
				case 0x3F:
					DSP1->command = 0x1f;
				case 0x1f: DSP1->in_count = 1; break;
				default:
				case 0x80:
					DSP1->in_count        = 0;
					DSP1->waiting4command = true;
					DSP1->first_parameter = true;
					break;
			}

			DSP1->in_count <<= 1;
		}
		else
		{
			DSP1->parameters[DSP1->in_index] = byte;
			DSP1->first_parameter = false;
			DSP1->in_index++;
		}

		if (DSP1->waiting4command || (DSP1->first_parameter && byte == 0x80))
		{
			DSP1->waiting4command = true;
			DSP1->first_parameter = false;
		}
		else
		if (DSP1->first_parameter && (DSP1->in_count != 0 || (DSP1->in_count == 0 && DSP1->in_index == 0)))
			;
		else
		{
			if (DSP1->in_count)
			{
				if (--DSP1->in_count == 0)
				{
					/* Actually execute the command*/
					DSP1->waiting4command = true;
					DSP1->out_index       = 0;

					switch (DSP1->command)
					{
						case 0x1f:
							DSP1->out_count = 2048;
							break;

						case 0x00: /* Multiple*/
							DSP1->Op00Multiplicand = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op00Multiplier   = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));

							DSP1_Op00();

							DSP1->out_count = 2;
							DSP1->output[0] =  DSP1->Op00Result       & 0xFF;
							DSP1->output[1] = (DSP1->Op00Result >> 8) & 0xFF;
							break;

						case 0x20: /* Multiple*/
							DSP1->Op20Multiplicand = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op20Multiplier   = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));

							DSP1_Op20();

							DSP1->out_count = 2;
							DSP1->output[0] =  DSP1->Op20Result       & 0xFF;
							DSP1->output[1] = (DSP1->Op20Result >> 8) & 0xFF;
							break;

						case 0x30:
						case 0x10: /* Inverse*/
							DSP1->Op10Coefficient = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op10Exponent    = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));

							DSP1_Op10();

							DSP1->out_count = 4;
							DSP1->output[0] = (uint8_t) ( ((int16_t) DSP1->Op10CoefficientR)       & 0xFF);
							DSP1->output[1] = (uint8_t) ((((int16_t) DSP1->Op10CoefficientR) >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t) ( ((int16_t) DSP1->Op10ExponentR   )       & 0xFF);
							DSP1->output[3] = (uint8_t) ((((int16_t) DSP1->Op10ExponentR   ) >> 8) & 0xFF);
							break;

						case 0x24:
						case 0x04: /* Sin and Cos of angle*/
							DSP1->Op04Angle  = (int16_t)  (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op04Radius = (uint16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));

							DSP1_Op04();

							DSP1->out_count = 4;
							DSP1->output[0] = (uint8_t)  (DSP1->Op04Sin       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op04Sin >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op04Cos       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op04Cos >> 8) & 0xFF);
							break;

						case 0x08: /* Radius*/
							DSP1->Op08X = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op08Y = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op08Z = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op08();

							DSP1->out_count = 4;
							DSP1->output[0] = (uint8_t) ( ((int16_t) DSP1->Op08Ll)       & 0xFF);
							DSP1->output[1] = (uint8_t) ((((int16_t) DSP1->Op08Ll) >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t) ( ((int16_t) DSP1->Op08Lh)       & 0xFF);
							DSP1->output[3] = (uint8_t) ((((int16_t) DSP1->Op08Lh) >> 8) & 0xFF);
							break;

						case 0x18: /* Range*/

							DSP1->Op18X = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op18Y = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op18Z = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));
							DSP1->Op18R = (int16_t) (DSP1->parameters[6] | (DSP1->parameters[7] << 8));

							DSP1_Op18();

							DSP1->out_count = 2;
							DSP1->output[0] = (uint8_t)  (DSP1->Op18D       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op18D >> 8) & 0xFF);
							break;

						case 0x38: /* Range*/

							DSP1->Op38X = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op38Y = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op38Z = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));
							DSP1->Op38R = (int16_t) (DSP1->parameters[6] | (DSP1->parameters[7] << 8));

							DSP1_Op38();

							DSP1->out_count = 2;
							DSP1->output[0] = (uint8_t)  (DSP1->Op38D       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op38D >> 8) & 0xFF);
							break;

						case 0x28: /* Distance (vector length)*/
							DSP1->Op28X = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op28Y = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op28Z = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op28();

							DSP1->out_count = 2;
							DSP1->output[0] = (uint8_t)  (DSP1->Op28R       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op28R >> 8) & 0xFF);
							break;

						case 0x2c:
						case 0x0c: /* Rotate (2D rotate)*/
							DSP1->Op0CA  = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op0CX1 = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op0CY1 = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op0C();

							DSP1->out_count = 4;
							DSP1->output[0] = (uint8_t)  (DSP1->Op0CX2       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op0CX2 >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op0CY2       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op0CY2 >> 8) & 0xFF);
							break;

						case 0x3c:
						case 0x1c: /* Polar (3D rotate)*/
							DSP1->Op1CZ   = (DSP1->parameters[ 0] | (DSP1->parameters[ 1] << 8));
							/*MK: reversed X and Y on neviksti and John's advice.*/
							DSP1->Op1CY   = (DSP1->parameters[ 2] | (DSP1->parameters[ 3] << 8));
							DSP1->Op1CX   = (DSP1->parameters[ 4] | (DSP1->parameters[ 5] << 8));
							DSP1->Op1CXBR = (DSP1->parameters[ 6] | (DSP1->parameters[ 7] << 8));
							DSP1->Op1CYBR = (DSP1->parameters[ 8] | (DSP1->parameters[ 9] << 8));
							DSP1->Op1CZBR = (DSP1->parameters[10] | (DSP1->parameters[11] << 8));

							DSP1_Op1C();

							DSP1->out_count = 6;
							DSP1->output[0] = (uint8_t)  (DSP1->Op1CXAR       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op1CXAR >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op1CYAR       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op1CYAR >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op1CZAR       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op1CZAR >> 8) & 0xFF);
							break;

						case 0x32:
						case 0x22:
						case 0x12:
						case 0x02: /* Parameter (Projection)*/
							DSP1->Op02FX  = (int16_t)  (DSP1->parameters[ 0] | (DSP1->parameters[ 1] << 8));
							DSP1->Op02FY  = (int16_t)  (DSP1->parameters[ 2] | (DSP1->parameters[ 3] << 8));
							DSP1->Op02FZ  = (int16_t)  (DSP1->parameters[ 4] | (DSP1->parameters[ 5] << 8));
							DSP1->Op02LFE = (int16_t)  (DSP1->parameters[ 6] | (DSP1->parameters[ 7] << 8));
							DSP1->Op02LES = (int16_t)  (DSP1->parameters[ 8] | (DSP1->parameters[ 9] << 8));
							DSP1->Op02AAS = (uint16_t) (DSP1->parameters[10] | (DSP1->parameters[11] << 8));
							DSP1->Op02AZS = (uint16_t) (DSP1->parameters[12] | (DSP1->parameters[13] << 8));

							DSP1_Op02();

							DSP1->out_count = 8;
							DSP1->output[0] = (uint8_t)  (DSP1->Op02VOF       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op02VOF >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op02VVA       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op02VVA >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op02CX        & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op02CX  >> 8) & 0xFF);
							DSP1->output[6] = (uint8_t)  (DSP1->Op02CY        & 0xFF);
							DSP1->output[7] = (uint8_t) ((DSP1->Op02CY  >> 8) & 0xFF);
							break;

						case 0x3a:
						case 0x2a:
						case 0x1a: /* Raster mode 7 matrix data*/
						case 0x0a:
							DSP1->Op0AVS = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));

							DSP1_Op0A();

							DSP1->out_count = 8;
							DSP1->output[0] = (uint8_t)  (DSP1->Op0AA       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op0AA >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op0AB       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op0AB >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op0AC       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op0AC >> 8) & 0xFF);
							DSP1->output[6] = (uint8_t)  (DSP1->Op0AD       & 0xFF);
							DSP1->output[7] = (uint8_t) ((DSP1->Op0AD >> 8) & 0xFF);
							DSP1->in_index  = 0;
							break;

						case 0x16:
						case 0x26:
						case 0x36:
						case 0x06: /* Project object*/
							DSP1->Op06X = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op06Y = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op06Z = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op06();

							DSP1->out_count = 6;
							DSP1->output[0] = (uint8_t)  (DSP1->Op06H       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op06H >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op06V       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op06V >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op06M       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op06M >> 8) & 0xFF);
							break;

						case 0x1e:
						case 0x2e:
						case 0x3e:
						case 0x0e: /* Target*/
							DSP1->Op0EH = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op0EV = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));

							DSP1_Op0E();

							DSP1->out_count = 4;
							DSP1->output[0] = (uint8_t)  (DSP1->Op0EX       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op0EX >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op0EY       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op0EY >> 8) & 0xFF);
							break;

							/* Extra commands used by Pilot Wings*/
						case 0x05:
						case 0x35:
						case 0x31:
						case 0x01: /* Set attitude matrix A*/
							DSP1->Op01m  = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op01Zr = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op01Yr = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));
							DSP1->Op01Xr = (int16_t) (DSP1->parameters[6] | (DSP1->parameters[7] << 8));

							DSP1_Op01();
							break;

						case 0x15:
						case 0x11: /* Set attitude matrix B*/
							DSP1->Op11m  = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op11Zr = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op11Yr = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));
							DSP1->Op11Xr = (int16_t) (DSP1->parameters[6] | (DSP1->parameters[7] << 8));

							DSP1_Op11();
							break;

						case 0x25:
						case 0x21: /* Set attitude matrix C*/
							DSP1->Op21m  = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op21Zr = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op21Yr = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));
							DSP1->Op21Xr = (int16_t) (DSP1->parameters[6] | (DSP1->parameters[7] << 8));

							DSP1_Op21();
							break;

						case 0x09:
						case 0x39:
						case 0x3d:
						case 0x0d: /* Objective matrix A*/
							DSP1->Op0DX = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op0DY = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op0DZ = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op0D();

							DSP1->out_count = 6;
							DSP1->output [0] = (uint8_t)  (DSP1->Op0DF       & 0xFF);
							DSP1->output [1] = (uint8_t) ((DSP1->Op0DF >> 8) & 0xFF);
							DSP1->output [2] = (uint8_t)  (DSP1->Op0DL       & 0xFF);
							DSP1->output [3] = (uint8_t) ((DSP1->Op0DL >> 8) & 0xFF);
							DSP1->output [4] = (uint8_t)  (DSP1->Op0DU       & 0xFF);
							DSP1->output [5] = (uint8_t) ((DSP1->Op0DU >> 8) & 0xFF);
							break;

						case 0x19:
						case 0x1d: /* Objective matrix B*/
							DSP1->Op1DX = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op1DY = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op1DZ = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op1D();

							DSP1->out_count = 6;
							DSP1->output[0] = (uint8_t)  (DSP1->Op1DF       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op1DF >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op1DL       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op1DL >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op1DU       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op1DU >> 8) & 0xFF);
							break;

						case 0x29:
						case 0x2d: /* Objective matrix C*/
							DSP1->Op2DX = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op2DY = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op2DZ = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op2D();

							DSP1->out_count = 6;
							DSP1->output[0] = (uint8_t)  (DSP1->Op2DF       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op2DF >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op2DL       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op2DL >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op2DU       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op2DU >> 8) & 0xFF);
							break;

						case 0x33:
						case 0x03: /* Subjective matrix A*/
							DSP1->Op03F = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op03L = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op03U = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op03();

							DSP1->out_count = 6;
							DSP1->output[0] = (uint8_t)  (DSP1->Op03X       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op03X >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op03Y       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op03Y >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op03Z       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op03Z >> 8) & 0xFF);
							break;

						case 0x13: /* Subjective matrix B*/
							DSP1->Op13F = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op13L = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op13U = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op13();

							DSP1->out_count = 6;
							DSP1->output[0] = (uint8_t)  (DSP1->Op13X       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op13X >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op13Y       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op13Y >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op13Z       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op13Z >> 8) & 0xFF);
							break;

						case 0x23: /* Subjective matrix C*/
							DSP1->Op23F = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op23L = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op23U = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op23();

							DSP1->out_count = 6;
							DSP1->output[0] = (uint8_t)  (DSP1->Op23X       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op23X >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op23Y       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op23Y >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op23Z       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op23Z >> 8) & 0xFF);
							break;

						case 0x3b:
						case 0x0b:
							DSP1->Op0BX = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op0BY = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op0BZ = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op0B();

							DSP1->out_count = 2;
							DSP1->output[0] = (uint8_t)  (DSP1->Op0BS       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op0BS >> 8) & 0xFF);
							break;

						case 0x1b:
							DSP1->Op1BX = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op1BY = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op1BZ = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op1B();

							DSP1->out_count = 2;
							DSP1->output[0] = (uint8_t)  (DSP1->Op1BS       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op1BS >> 8) & 0xFF);
							break;

						case 0x2b:
							DSP1->Op2BX = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));
							DSP1->Op2BY = (int16_t) (DSP1->parameters[2] | (DSP1->parameters[3] << 8));
							DSP1->Op2BZ = (int16_t) (DSP1->parameters[4] | (DSP1->parameters[5] << 8));

							DSP1_Op2B();

							DSP1->out_count = 2;
							DSP1->output[0] = (uint8_t)  (DSP1->Op2BS       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op2BS >> 8) & 0xFF);
							break;

						case 0x34:
						case 0x14:
							DSP1->Op14Zr = (int16_t) (DSP1->parameters[ 0] | (DSP1->parameters[ 1] << 8));
							DSP1->Op14Xr = (int16_t) (DSP1->parameters[ 2] | (DSP1->parameters[ 3] << 8));
							DSP1->Op14Yr = (int16_t) (DSP1->parameters[ 4] | (DSP1->parameters[ 5] << 8));
							DSP1->Op14U  = (int16_t) (DSP1->parameters[ 6] | (DSP1->parameters[ 7] << 8));
							DSP1->Op14F  = (int16_t) (DSP1->parameters[ 8] | (DSP1->parameters[ 9] << 8));
							DSP1->Op14L  = (int16_t) (DSP1->parameters[10] | (DSP1->parameters[11] << 8));

							DSP1_Op14();

							DSP1->out_count = 6;
							DSP1->output[0] = (uint8_t)  (DSP1->Op14Zrr       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op14Zrr >> 8) & 0xFF);
							DSP1->output[2] = (uint8_t)  (DSP1->Op14Xrr       & 0xFF);
							DSP1->output[3] = (uint8_t) ((DSP1->Op14Xrr >> 8) & 0xFF);
							DSP1->output[4] = (uint8_t)  (DSP1->Op14Yrr       & 0xFF);
							DSP1->output[5] = (uint8_t) ((DSP1->Op14Yrr >> 8) & 0xFF);
							break;

						case 0x27:
						case 0x2F:
							DSP1->Op2FUnknown = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));

							DSP1_Op2F();

							DSP1->out_count = 2;
							DSP1->output[0] = (uint8_t)  (DSP1->Op2FSize       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op2FSize >> 8) & 0xFF);
							break;


						case 0x07:
						case 0x0F:
							DSP1->Op0FRamsize = (int16_t) (DSP1->parameters[0] | (DSP1->parameters[1] << 8));

							DSP1_Op0F();

							DSP1->out_count = 2;
							DSP1->output[0] = (uint8_t)  (DSP1->Op0FPass       & 0xFF);
							DSP1->output[1] = (uint8_t) ((DSP1->Op0FPass >> 8) & 0xFF);
							break;

						default:
							break;
					}
				}
			}
		}
	}
}

uint8_t DSP1GetByte (uint16_t address)
{
	uint8_t	t;

	if ((address & 0xf000) == 0x6000 || (address & 0x7fff) < 0x4000)
	{
		if (DSP1->out_count)
		{
			t = (uint8_t) DSP1->output[DSP1->out_index];

			DSP1->out_index++;

			if (--DSP1->out_count == 0)
			{
				if (DSP1->command == 0x1a || DSP1->command == 0x0a)
				{
					DSP1_Op0A();
					DSP1->out_count = 8;
					DSP1->out_index = 0;
					DSP1->output[0] =  DSP1->Op0AA       & 0xFF;
					DSP1->output[1] = (DSP1->Op0AA >> 8) & 0xFF;
					DSP1->output[2] =  DSP1->Op0AB       & 0xFF;
					DSP1->output[3] = (DSP1->Op0AB >> 8) & 0xFF;
					DSP1->output[4] =  DSP1->Op0AC       & 0xFF;
					DSP1->output[5] = (DSP1->Op0AC >> 8) & 0xFF;
					DSP1->output[6] =  DSP1->Op0AD       & 0xFF;
					DSP1->output[7] = (DSP1->Op0AD >> 8) & 0xFF;
				}

				if (DSP1->command == 0x1f)
				{
					if ((DSP1->out_index % 2) != 0)
						t = (uint8_t) DSP1ROM[DSP1->out_index >> 1];
					else
						t = DSP1ROM[DSP1->out_index >> 1] >> 8;
				}
			}

			DSP1->waiting4command = true;
		}
		else
			t = 0xff;
	}
	else
		t = 0x80;

	return (t);
}

/***********************************************************************************
 DSP2
***********************************************************************************/

/* convert bitmap to bitplane tile*/
static void DSP2_Op01 (void)
{
	/* Op01 size is always 32 bytes input and output*/
	/* The hardware does strange things if you vary the size*/
	int j;
	uint8_t	c0, c1, c2, c3;
	uint8_t *p1, *p2a, *p2b;

	p1  = DSP2->parameters;
	p2a = DSP2->output;
	p2b = DSP2->output + 16; /* halfway*/

	/* Process 8 blocks of 4 bytes each*/

	for ( j = 0; j < 8; j++)
	{
		c0 = *p1++;
		c1 = *p1++;
		c2 = *p1++;
		c3 = *p1++;

		*p2a++ = (c0 & 0x10) << 3 |
				 (c0 & 0x01) << 6 |
				 (c1 & 0x10) << 1 |
				 (c1 & 0x01) << 4 |
				 (c2 & 0x10) >> 1 |
				 (c2 & 0x01) << 2 |
				 (c3 & 0x10) >> 3 |
				 (c3 & 0x01);

		*p2a++ = (c0 & 0x20) << 2 |
				 (c0 & 0x02) << 5 |
				 (c1 & 0x20)      |
				 (c1 & 0x02) << 3 |
				 (c2 & 0x20) >> 2 |
				 (c2 & 0x02) << 1 |
				 (c3 & 0x20) >> 4 |
				 (c3 & 0x02) >> 1;

		*p2b++ = (c0 & 0x40) << 1 |
				 (c0 & 0x04) << 4 |
				 (c1 & 0x40) >> 1 |
				 (c1 & 0x04) << 2 |
				 (c2 & 0x40) >> 3 |
				 (c2 & 0x04)      |
				 (c3 & 0x40) >> 5 |
				 (c3 & 0x04) >> 2;

		*p2b++ = (c0 & 0x80)      |
				 (c0 & 0x08) << 3 |
				 (c1 & 0x80) >> 2 |
				 (c1 & 0x08) << 1 |
				 (c2 & 0x80) >> 4 |
				 (c2 & 0x08) >> 1 |
				 (c3 & 0x80) >> 6 |
				 (c3 & 0x08) >> 3;
	}
}

/* set transparent color*/
static void DSP2_Op03 (void)
{
	DSP2->Op05Transparent = DSP2->parameters[0];
}

/* replace bitmap using transparent color*/
static void DSP2_Op05 (void)
{
	int32_t n;
	/* Overlay bitmap with transparency.
	/ Input:
	//
	//   Bitmap 1:  i[0] <=> i[size-1]
	//   Bitmap 2:  i[size] <=> i[2*size-1]
	//
	// Output:
	//
	//   Bitmap 3:  o[0] <=> o[size-1]
	//
	// Processing:
	//
	//   Process all 4-bit pixels (nibbles) in the bitmap
	//
	//   if ( BM2_pixel == transparent_color )
	//      pixelout = BM1_pixel
	//   else
	//      pixelout = BM2_pixel

	// The max size bitmap is limited to 255 because the size parameter is a byte
	// I think size=0 is an error.  The behavior of the chip on size=0 is to
	// return the last value written to DR if you read DR on Op05 with
	// size = 0.  I don't think it's worth implementing this quirk unless it's
	// proven necessary. */

	uint8_t	color;
	uint8_t	c1, c2;
	uint8_t *p1, *p2, *p3;

	p1 = DSP2->parameters;
	p2 = DSP2->parameters + DSP2->Op05Len;
	p3 = DSP2->output;

	color = DSP2->Op05Transparent & 0x0f;

	for ( n = 0; n < DSP2->Op05Len; n++)
	{
		c1 = *p1++;
		c2 = *p2++;
		*p3++ = (((c2 >> 4) == color) ? c1 & 0xf0: c2 & 0xf0) | (((c2 & 0x0f) == color) ? c1 & 0x0f: c2 & 0x0f);
	}
}

/* reverse bitmap*/
static void DSP2_Op06 (void)
{
	int32_t i, j;
	/* Input:
	//    size
	//    bitmap */

	for ( i = 0, j = DSP2->Op06Len - 1; i < DSP2->Op06Len; i++, j--)
		DSP2->output[j] = (DSP2->parameters[i] << 4) | (DSP2->parameters[i] >> 4);
}

/* multiply*/
static void DSP2_Op09 (void)
{
	uint32_t temp;
	DSP2->Op09Word1 = DSP2->parameters[0] | (DSP2->parameters[1] << 8);
	DSP2->Op09Word2 = DSP2->parameters[2] | (DSP2->parameters[3] << 8);

	temp = DSP2->Op09Word1 * DSP2->Op09Word2;
	DSP2->output[0] =  temp        & 0xFF;
	DSP2->output[1] = (temp >>  8) & 0xFF;
	DSP2->output[2] = (temp >> 16) & 0xFF;
	DSP2->output[3] = (temp >> 24) & 0xFF;
}

/* scale bitmap*/
static void DSP2_Op0D (void)
{
	int32_t i;
	/* Bit accurate hardware algorithm - uses fixed point math
	// This should match the DSP2 Op0D output exactly
	// I wouldn't recommend using this unless you're doing hardware debug.
	// In some situations it has small visual artifacts that
	// are not readily apparent on a TV screen but show up clearly
	// on a monitor.  Use Overload's scaling instead.
	// This is for hardware verification testing.
	//
	// One note:  the HW can do odd byte scaling but since we divide
	// by two to get the count of bytes this won't work well for
	// odd byte scaling (in any of the current algorithm implementations).
	// So far I haven't seen Dungeon Master use it.
	// If it does we can adjust the parameters and code to work with it */

	uint32_t	multiplier; /* Any size int >= 32-bits*/
	uint32_t	pixloc;	    /* match size of multiplier*/
	uint8_t	pixelarray[512];

	if (DSP2->Op0DInLen <= DSP2->Op0DOutLen)
		multiplier = 0x10000; /* In our self defined fixed point 0x10000 == 1*/
	else
		multiplier = (DSP2->Op0DInLen << 17) / ((DSP2->Op0DOutLen << 1) + 1);

	pixloc = 0;

	for ( i = 0; i < DSP2->Op0DOutLen * 2; i++)
	{
		int32_t	j = pixloc >> 16;

		if (j & 1)
			pixelarray[i] =  DSP2->parameters[j >> 1] & 0x0f;
		else
			pixelarray[i] = (DSP2->parameters[j >> 1] & 0xf0) >> 4;

		pixloc += multiplier;
	}

	for ( i = 0; i < DSP2->Op0DOutLen; i++)
		DSP2->output[i] = (pixelarray[i << 1] << 4) | pixelarray[(i << 1) + 1];
}

/*
static void DSP2_Op0D (void)
{
	// Overload's algorithm - use this unless doing hardware testing

	// One note:  the HW can do odd byte scaling but since we divide
	// by two to get the count of bytes this won't work well for
	// odd byte scaling (in any of the current algorithm implementations).
	// So far I haven't seen Dungeon Master use it.
	// If it does we can adjust the parameters and code to work with it

	int32_t	pixel_offset;
	uint8_t	pixelarray[512];

	for (int32_t i = 0; i < DSP2->Op0DOutLen * 2; i++)
	{
		pixel_offset = (i * DSP2->Op0DInLen) / DSP2->Op0DOutLen;

		if ((pixel_offset & 1) == 0)
			pixelarray[i] = DSP2->parameters[pixel_offset >> 1] >> 4;
		else
			pixelarray[i] = DSP2->parameters[pixel_offset >> 1] & 0x0f;
	}

	for (int32_t i = 0; i < DSP2->Op0DOutLen; i++)
		DSP2->output[i] = (pixelarray[i << 1] << 4) | pixelarray[(i << 1) + 1];
}
*/

void DSP2SetByte (uint8_t byte, uint16_t address)
{
	if ((address & 0xf000) == 0x6000 || (address >= 0x8000 && address < 0xc000))
	{
		if (DSP2->waiting4command)
		{
			DSP2->command         = byte;
			DSP2->in_index        = 0;
			DSP2->waiting4command = false;

			switch (byte)
			{
				case 0x01: DSP2->in_count = 32; break;
				case 0x03: DSP2->in_count =  1; break;
				case 0x05: DSP2->in_count =  1; break;
				case 0x06: DSP2->in_count =  1; break;
				case 0x09: DSP2->in_count =  4; break;
				case 0x0D: DSP2->in_count =  2; break;
				default:
				case 0x0f: DSP2->in_count =  0; break;
			}
		}
		else
		{
			DSP2->parameters[DSP2->in_index] = byte;
			DSP2->in_index++;
		}

		if (DSP2->in_count == DSP2->in_index)
		{
			DSP2->waiting4command = true;
			DSP2->out_index       = 0;

			switch (DSP2->command)
			{
				case 0x01:
					DSP2->out_count = 32;
					DSP2_Op01();
					break;

				case 0x03:
					DSP2_Op03();
					break;

				case 0x05:
					if (DSP2->Op05HasLen)
					{
						DSP2->Op05HasLen = false;
						DSP2->out_count  = DSP2->Op05Len;
						DSP2_Op05();
					}
					else
					{
						DSP2->Op05Len    = DSP2->parameters[0];
						DSP2->in_index   = 0;
						DSP2->in_count   = 2 * DSP2->Op05Len;
						DSP2->Op05HasLen = true;
						if (byte)
							DSP2->waiting4command = false;
					}

					break;

				case 0x06:
					if (DSP2->Op06HasLen)
					{
						DSP2->Op06HasLen = false;
						DSP2->out_count  = DSP2->Op06Len;
						DSP2_Op06();
					}
					else
					{
						DSP2->Op06Len    = DSP2->parameters[0];
						DSP2->in_index   = 0;
						DSP2->in_count   = DSP2->Op06Len;
						DSP2->Op06HasLen = true;
						if (byte)
							DSP2->waiting4command = false;
					}

					break;

				case 0x09:
					DSP2->out_count = 4;
					DSP2_Op09();
					break;

				case 0x0D:
					if (DSP2->Op0DHasLen)
					{
						DSP2->Op0DHasLen = false;
						DSP2->out_count  = DSP2->Op0DOutLen;
						DSP2_Op0D();
					}
					else
					{
						DSP2->Op0DInLen  = DSP2->parameters[0];
						DSP2->Op0DOutLen = DSP2->parameters[1];
						DSP2->in_index   = 0;
						DSP2->in_count   = (DSP2->Op0DInLen + 1) >> 1;
						DSP2->Op0DHasLen = true;
						if (byte)
							DSP2->waiting4command = false;
					}

					break;

				case 0x0f:
				default:
					break;
			}
		}
	}
}

uint8_t DSP2GetByte (uint16_t address)
{
	uint8_t	t;

	if ((address & 0xf000) == 0x6000 || (address >= 0x8000 && address < 0xc000))
	{
		if (DSP2->out_count)
		{
			t = (uint8_t) DSP2->output[DSP2->out_index];
			DSP2->out_index++;
			if (DSP2->out_count == DSP2->out_index)
				DSP2->out_count = 0;
		}
		else
			t = 0xff;
	}
	else
		t = 0x80;

	return (t);
}

/***********************************************************************************
 DSP3
***********************************************************************************/

static void (*SetDSP3) (void);

static const uint16_t	DSP3_DataROM[1024] =
{
	0x8000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100,
	0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001,
	0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100,
	0x0000, 0x000f, 0x0400, 0x0200, 0x0140, 0x0400, 0x0200, 0x0040,
	0x007d, 0x007e, 0x007e, 0x007b, 0x007c, 0x007d, 0x007b, 0x007c,
	0x0002, 0x0020, 0x0030, 0x0000, 0x000d, 0x0019, 0x0026, 0x0032,
	0x003e, 0x004a, 0x0056, 0x0062, 0x006d, 0x0079, 0x0084, 0x008e,
	0x0098, 0x00a2, 0x00ac, 0x00b5, 0x00be, 0x00c6, 0x00ce, 0x00d5,
	0x00dc, 0x00e2, 0x00e7, 0x00ec, 0x00f1, 0x00f5, 0x00f8, 0x00fb,
	0x00fd, 0x00ff, 0x0100, 0x0100, 0x0100, 0x00ff, 0x00fd, 0x00fb,
	0x00f8, 0x00f5, 0x00f1, 0x00ed, 0x00e7, 0x00e2, 0x00dc, 0x00d5,
	0x00ce, 0x00c6, 0x00be, 0x00b5, 0x00ac, 0x00a2, 0x0099, 0x008e,
	0x0084, 0x0079, 0x006e, 0x0062, 0x0056, 0x004a, 0x003e, 0x0032,
	0x0026, 0x0019, 0x000d, 0x0000, 0xfff3, 0xffe7, 0xffdb, 0xffce,
	0xffc2, 0xffb6, 0xffaa, 0xff9e, 0xff93, 0xff87, 0xff7d, 0xff72,
	0xff68, 0xff5e, 0xff54, 0xff4b, 0xff42, 0xff3a, 0xff32, 0xff2b,
	0xff25, 0xff1e, 0xff19, 0xff14, 0xff0f, 0xff0b, 0xff08, 0xff05,
	0xff03, 0xff01, 0xff00, 0xff00, 0xff00, 0xff01, 0xff03, 0xff05,
	0xff08, 0xff0b, 0xff0f, 0xff13, 0xff18, 0xff1e, 0xff24, 0xff2b,
	0xff32, 0xff3a, 0xff42, 0xff4b, 0xff54, 0xff5d, 0xff67, 0xff72,
	0xff7c, 0xff87, 0xff92, 0xff9e, 0xffa9, 0xffb5, 0xffc2, 0xffce,
	0xffda, 0xffe7, 0xfff3, 0x002b, 0x007f, 0x0020, 0x00ff, 0xff00,
	0xffbe, 0x0000, 0x0044, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffc1, 0x0001, 0x0002, 0x0045,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc5, 0x0003, 0x0004, 0x0005, 0x0047, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffca, 0x0006, 0x0007, 0x0008,
	0x0009, 0x004a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffd0, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x004e, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd7, 0x000f, 0x0010, 0x0011,
	0x0012, 0x0013, 0x0014, 0x0053, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffdf, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b,
	0x0059, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffe8, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0021, 0x0022, 0x0023, 0x0060, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xfff2, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a,
	0x002b, 0x002c, 0x0068, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfffd, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0071,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc7, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d,
	0x003e, 0x003f, 0x0040, 0x0041, 0x007b, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd4, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x0044, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffe2, 0x000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012,
	0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0050, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfff1, 0x0019, 0x001a, 0x001b,
	0x001c, 0x001d, 0x001e, 0x001f, 0x0020, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x005d, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffcb, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d,
	0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
	0x006b, 0x0000, 0x0000, 0x0000, 0xffdc, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 0x0044, 0x0000, 0x0000,
	0xffee, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016,
	0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0054, 0x0000, 0xffee, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b,
	0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0065,
	0xffbe, 0x0000, 0xfeac, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffc1, 0x0001, 0x0002, 0xfead,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc5, 0x0003, 0x0004, 0x0005, 0xfeaf, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffca, 0x0006, 0x0007, 0x0008,
	0x0009, 0xfeb2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffd0, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0xfeb6, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd7, 0x000f, 0x0010, 0x0011,
	0x0012, 0x0013, 0x0014, 0xfebb, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffdf, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b,
	0xfec1, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffe8, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0021, 0x0022, 0x0023, 0xfec8, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xfff2, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a,
	0x002b, 0x002c, 0xfed0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfffd, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0xfed9,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc7, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d,
	0x003e, 0x003f, 0x0040, 0x0041, 0xfee3, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd4, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0xfeac, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffe2, 0x000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012,
	0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0xfeb8, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfff1, 0x0019, 0x001a, 0x001b,
	0x001c, 0x001d, 0x001e, 0x001f, 0x0020, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0xfec5, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffcb, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d,
	0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
	0xfed3, 0x0000, 0x0000, 0x0000, 0xffdc, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 0xfeac, 0x0000, 0x0000,
	0xffee, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016,
	0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0xfebc, 0x0000, 0xffee, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b,
	0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0xfecd,
	0x0154, 0x0218, 0x0110, 0x00b0, 0x00cc, 0x00b0, 0x0088, 0x00b0,
	0x0044, 0x00b0, 0x0000, 0x00b0, 0x00fe, 0xff07, 0x0002, 0x00ff,
	0x00f8, 0x0007, 0x00fe, 0x00ee, 0x07ff, 0x0200, 0x00ef, 0xf800,
	0x0700, 0x00ee, 0xffff, 0xffff, 0xffff, 0x0000, 0x0000, 0x0001,
	0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000,
	0x0000, 0xffff, 0xffff, 0x0000, 0xffff, 0x0001, 0x0000, 0x0001,
	0x0001, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0x0000,
	0xffff, 0x0001, 0x0000, 0x0001, 0x0001, 0x0000, 0x0000, 0xffff,
	0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0x0044, 0x0088, 0x00cc,
	0x0110, 0x0154, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

static void DSP3_Command (void);
static void DSP3_Decode_Data (void);
static void DSP3_Decode_Tree (void);
static void DSP3_Decode_Symbols (void);
static void DSP3_Decode (void);
static void DSP3_Decode_A (void);
static void DSP3_Convert (void);
static void DSP3_Convert_A (void);
static void DSP3_OP03 (void);
static void DSP3_OP06 (void);
static void DSP3_OP07 (void);
static void DSP3_OP07_A (void);
static void DSP3_OP07_B (void);
static void DSP3_OP0C (void);
/* static void DSP3_OP0C_A (void); */
static void DSP3_OP10 (void);
static void DSP3_OP1C (void);
static void DSP3_OP1C_A (void);
static void DSP3_OP1C_B (void);
static void DSP3_OP1C_C (void);
static void DSP3_OP1E (void);
static void DSP3_OP1E_A (void);
static void DSP3_OP1E_A1 (void);
static void DSP3_OP1E_A2 (void);
static void DSP3_OP1E_A3 (void);
static void DSP3_OP1E_B (void);
static void DSP3_OP1E_B1 (void);
static void DSP3_OP1E_B2 (void);
static void DSP3_OP1E_C (void);
static void DSP3_OP1E_C1 (void);
static void DSP3_OP1E_C2 (void);
static void DSP3_OP1E_D (int16_t, int16_t *, int16_t *);
static void DSP3_OP1E_D1 (int16_t, int16_t *, int16_t *);
static void DSP3_OP3E (void);


void DSP3_Reset (void)
{
	DSP3->DR = 0x0080;
	DSP3->SR = 0x0084;
	SetDSP3 = &DSP3_Command;
}

static void DSP3_TestMemory (void)
{
	DSP3->DR = 0x0000;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_DumpDataROM (void)
{
	DSP3->DR = DSP3_DataROM[DSP3->MemoryIndex++];
	if (DSP3->MemoryIndex == 1024)
		SetDSP3 = &DSP3_Reset;
}

static void DSP3_MemoryDump (void)
{
	DSP3->MemoryIndex = 0;
	SetDSP3 = &DSP3_DumpDataROM;
	DSP3_DumpDataROM();
}

static void DSP3_OP06 (void)
{
	DSP3->WinLo = (uint8_t) (DSP3->DR);
	DSP3->WinHi = (uint8_t) (DSP3->DR >> 8);
	DSP3_Reset();
}

static void DSP3_OP03 (void)
{
	int16_t Lo, Hi, Ofs;

	Lo  = (uint8_t) (DSP3->DR);
	Hi  = (uint8_t) (DSP3->DR >> 8);
	Ofs = (DSP3->WinLo * Hi << 1) + (Lo << 1);

	DSP3->DR = Ofs >> 1;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP07_B (void)
{
	int16_t Ofs;

	Ofs = (DSP3->WinLo * DSP3->AddHi << 1) + (DSP3->AddLo << 1);

	DSP3->DR = Ofs >> 1;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP07_A (void)
{
	int16_t Lo, Hi;

	Lo = (uint8_t) (DSP3->DR);
	Hi = (uint8_t) (DSP3->DR >> 8);

	if (Lo & 1)
		Hi += (DSP3->AddLo & 1);

	DSP3->AddLo += Lo;
	DSP3->AddHi += Hi;

	if (DSP3->AddLo < 0)
		DSP3->AddLo += DSP3->WinLo;
	else
	if (DSP3->AddLo >= DSP3->WinLo)
		DSP3->AddLo -= DSP3->WinLo;

	if (DSP3->AddHi < 0)
		DSP3->AddHi += DSP3->WinHi;
	else
	if (DSP3->AddHi >= DSP3->WinHi)
		DSP3->AddHi -= DSP3->WinHi;

	DSP3->DR = DSP3->AddLo | (DSP3->AddHi << 8) | ((DSP3->AddHi >> 8) & 0xff);
	SetDSP3 = &DSP3_OP07_B;
}

static void DSP3_OP07 (void)
{
	uint32_t dataOfs;

	dataOfs = ((DSP3->DR << 1) + 0x03b2) & 0x03ff;

	DSP3->AddHi = DSP3_DataROM[dataOfs];
	DSP3->AddLo = DSP3_DataROM[dataOfs + 1];

	SetDSP3 = &DSP3_OP07_A;
	DSP3->SR = 0x0080;
}

static void DSP3_Coordinate (void)
{
	DSP3->Index++;

	switch (DSP3->Index)
	{
		case 3:
			if (DSP3->DR == 0xffff)
				DSP3_Reset();
			break;

		case 4:
			DSP3->X = DSP3->DR;
			break;

		case 5:
			DSP3->Y = DSP3->DR;
			DSP3->DR = 1;
			break;

		case 6:
			DSP3->DR = DSP3->X;
			break;

		case 7:
			DSP3->DR = DSP3->Y;
			DSP3->Index = 0;
			break;
	}
}

static void DSP3_Convert_A (void)
{
	int i, j;
	if (DSP3->BMIndex < 8)
	{
		DSP3->Bitmap[DSP3->BMIndex++] = (uint8_t) (DSP3->DR);
		DSP3->Bitmap[DSP3->BMIndex++] = (uint8_t) (DSP3->DR >> 8);

		if (DSP3->BMIndex == 8)
		{
			for ( i = 0; i < 8; i++)
			{
				for ( j = 0; j < 8; j++)
				{
					DSP3->Bitplane[j] <<= 1;
					DSP3->Bitplane[j] |= (DSP3->Bitmap[i] >> j) & 1;
				}
			}

			DSP3->BPIndex = 0;
			DSP3->Count--;
		}
	}

	if (DSP3->BMIndex == 8)
	{
		if (DSP3->BPIndex == 8)
		{
			if (!DSP3->Count)
				DSP3_Reset();

			DSP3->BMIndex = 0;
		}
		else
		{
			DSP3->DR  = DSP3->Bitplane[DSP3->BPIndex++];
			DSP3->DR |= DSP3->Bitplane[DSP3->BPIndex++] << 8;
		}
	}
}

static void DSP3_Convert (void)
{
	DSP3->Count = DSP3->DR;
	DSP3->BMIndex = 0;
	SetDSP3 = &DSP3_Convert_A;
}

static bool DSP3_GetBits (uint8_t Count)
{
	if (!DSP3->BitsLeft)
	{
		DSP3->BitsLeft = Count;
		DSP3->ReqBits = 0;
	}

	do
	{
		if (!DSP3->BitCount)
		{
			DSP3->SR = 0xC0;
			return (false);
		}

		DSP3->ReqBits <<= 1;
		if (DSP3->ReqData & 0x8000)
			DSP3->ReqBits++;
		DSP3->ReqData <<= 1;

		DSP3->BitCount--;
		DSP3->BitsLeft--;

	}
	while (DSP3->BitsLeft);

	return (true);
}

static void DSP3_Decode_Data (void)
{
	if (!DSP3->BitCount)
	{
		if (DSP3->SR & 0x40)
		{
			DSP3->ReqData = DSP3->DR;
			DSP3->BitCount += 16;
		}
		else
		{
			DSP3->SR = 0xC0;
			return;
		}
	}

	if (DSP3->LZCode == 1)
	{
		if (!DSP3_GetBits(1))
			return;

		if (DSP3->ReqBits)
			DSP3->LZLength = 12;
		else
			DSP3->LZLength = 8;

		DSP3->LZCode++;
	}

	if (DSP3->LZCode == 2)
	{
		if (!DSP3_GetBits(DSP3->LZLength))
			return;

		DSP3->LZCode = 0;
		DSP3->Outwords--;
		if (!DSP3->Outwords)
			SetDSP3 = &DSP3_Reset;

		DSP3->SR = 0x80;
		DSP3->DR = DSP3->ReqBits;
		return;
	}

	if (DSP3->BaseCode == 0xffff)
	{
		if (!DSP3_GetBits(DSP3->BaseLength))
			return;

		DSP3->BaseCode = DSP3->ReqBits;
	}

	if (!DSP3_GetBits(DSP3->CodeLengths[DSP3->BaseCode]))
		return;

	DSP3->Symbol = DSP3->Codes[DSP3->CodeOffsets[DSP3->BaseCode] + DSP3->ReqBits];
	DSP3->BaseCode = 0xffff;

	if (DSP3->Symbol & 0xff00)
	{
		DSP3->Symbol += 0x7f02;
		DSP3->LZCode++;
	}
	else
	{
		DSP3->Outwords--;
		if (!DSP3->Outwords)
			SetDSP3 = &DSP3_Reset;
	}

	DSP3->SR = 0x80;
	DSP3->DR = DSP3->Symbol;
}

static void DSP3_Decode_Tree (void)
{
	if (!DSP3->BitCount)
	{
		DSP3->ReqData = DSP3->DR;
		DSP3->BitCount += 16;
	}

	if (!DSP3->BaseCodes)
	{
		DSP3_GetBits(1);

		if (DSP3->ReqBits)
		{
			DSP3->BaseLength = 3;
			DSP3->BaseCodes = 8;
		}
		else
		{
			DSP3->BaseLength = 2;
			DSP3->BaseCodes = 4;
		}
	}

	while (DSP3->BaseCodes)
	{
		if (!DSP3_GetBits(3))
			return;

		DSP3->ReqBits++;

		DSP3->CodeLengths[DSP3->Index] = (uint8_t) DSP3->ReqBits;
		DSP3->CodeOffsets[DSP3->Index] = DSP3->Symbol;
		DSP3->Index++;

		DSP3->Symbol += 1 << DSP3->ReqBits;
		DSP3->BaseCodes--;
	}

	DSP3->BaseCode = 0xffff;
	DSP3->LZCode = 0;

	SetDSP3 = &DSP3_Decode_Data;
	if (DSP3->BitCount)
		DSP3_Decode_Data();
}

static void DSP3_Decode_Symbols (void)
{
	DSP3->ReqData = DSP3->DR;
	DSP3->BitCount += 16;

	do
	{
		if (DSP3->BitCommand == 0xffff)
		{
			if (!DSP3_GetBits(2))
				return;

			DSP3->BitCommand = DSP3->ReqBits;
		}

		switch (DSP3->BitCommand)
		{
			case 0:
				if (!DSP3_GetBits(9))
					return;
				DSP3->Symbol = DSP3->ReqBits;
				break;

			case 1:
				DSP3->Symbol++;
				break;

			case 2:
				if (!DSP3_GetBits(1))
					return;
				DSP3->Symbol += 2 + DSP3->ReqBits;
				break;

			case 3:
				if (!DSP3_GetBits(4))
					return;
				DSP3->Symbol += 4 + DSP3->ReqBits;
				break;
		}

		DSP3->BitCommand = 0xffff;

		DSP3->Codes[DSP3->Index++] = DSP3->Symbol;
		DSP3->Codewords--;

	}
	while (DSP3->Codewords);

	DSP3->Index = 0;
	DSP3->Symbol = 0;
	DSP3->BaseCodes = 0;

	SetDSP3 = &DSP3_Decode_Tree;
	if (DSP3->BitCount)
		DSP3_Decode_Tree();
}

static void DSP3_Decode_A (void)
{
	DSP3->Outwords = DSP3->DR;
	SetDSP3 = &DSP3_Decode_Symbols;
	DSP3->BitCount = 0;
	DSP3->BitsLeft = 0;
	DSP3->Symbol = 0;
	DSP3->Index = 0;
	DSP3->BitCommand = 0xffff;
	DSP3->SR = 0xC0;
}

static void DSP3_Decode (void)
{
	DSP3->Codewords = DSP3->DR;
	SetDSP3 = &DSP3_Decode_A;
}

/* Opcodes 1E/3E bit-perfect to 'dsp3-intro' log
   src: adapted from SD Gundam X/G-Next */

static void DSP3_OP3E (void)
{
	DSP3-> op3e_x = (uint8_t)  (DSP3->DR & 0x00ff);
	DSP3-> op3e_y = (uint8_t) ((DSP3->DR & 0xff00) >> 8);

	DSP3_OP03();

	DSP3->op1e_terrain[DSP3->DR] = 0x00;
	DSP3->op1e_cost[DSP3->DR]    = 0xff;
	DSP3->op1e_weight[DSP3->DR]  = 0;

	DSP3->op1e_max_search_radius = 0;
	DSP3->op1e_max_path_radius   = 0;
}

static void DSP3_OP1E (void)
{
	int lcv;
	DSP3->op1e_min_radius = (uint8_t)  (DSP3->DR & 0x00ff);
	DSP3->op1e_max_radius = (uint8_t) ((DSP3->DR & 0xff00) >> 8);

	if (DSP3->op1e_min_radius == 0)
		DSP3->op1e_min_radius++;

	if (DSP3->op1e_max_search_radius >= DSP3->op1e_min_radius)
		DSP3->op1e_min_radius = DSP3->op1e_max_search_radius + 1;

	if (DSP3->op1e_max_radius > DSP3->op1e_max_search_radius)
		DSP3->op1e_max_search_radius = DSP3->op1e_max_radius;

	DSP3->op1e_lcv_radius = DSP3->op1e_min_radius;
	DSP3->op1e_lcv_steps = DSP3->op1e_min_radius;

	DSP3->op1e_lcv_turns = 6;
	DSP3->op1e_turn = 0;

	DSP3->op1e_x = DSP3-> op3e_x;
	DSP3->op1e_y = DSP3-> op3e_y;

	for ( lcv = 0; lcv < DSP3->op1e_min_radius; lcv++)
		DSP3_OP1E_D(DSP3->op1e_turn, &DSP3->op1e_x, &DSP3->op1e_y);

	DSP3_OP1E_A();
}

static void DSP3_OP1E_A (void)
{
	int lcv;
	if (DSP3->op1e_lcv_steps == 0)
	{
		DSP3->op1e_lcv_radius++;

		DSP3->op1e_lcv_steps = DSP3->op1e_lcv_radius;

		DSP3->op1e_x = DSP3-> op3e_x;
		DSP3->op1e_y = DSP3-> op3e_y;

		for ( lcv = 0; lcv < DSP3->op1e_lcv_radius; lcv++)
			DSP3_OP1E_D(DSP3->op1e_turn, &DSP3->op1e_x, &DSP3->op1e_y);
	}

	if (DSP3->op1e_lcv_radius > DSP3->op1e_max_radius)
	{
		DSP3->op1e_turn++;
		DSP3->op1e_lcv_turns--;

		DSP3->op1e_lcv_radius = DSP3->op1e_min_radius;
		DSP3->op1e_lcv_steps = DSP3->op1e_min_radius;

		DSP3->op1e_x = DSP3-> op3e_x;
		DSP3->op1e_y = DSP3-> op3e_y;

		for ( lcv = 0; lcv < DSP3->op1e_min_radius; lcv++)
			DSP3_OP1E_D(DSP3->op1e_turn, &DSP3->op1e_x, &DSP3->op1e_y);
	}

	if (DSP3->op1e_lcv_turns == 0)
	{
		DSP3->DR = 0xffff;
		DSP3->SR = 0x0080;
		SetDSP3 = &DSP3_OP1E_B;
		return;
	}

	DSP3->DR = (uint8_t) (DSP3->op1e_x) | ((uint8_t) (DSP3->op1e_y) << 8);
	DSP3_OP03();

	DSP3->op1e_cell = DSP3->DR;

	DSP3->SR = 0x0080;
	SetDSP3 = &DSP3_OP1E_A1;
}

static void DSP3_OP1E_A1 (void)
{
	DSP3->SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_A2;
}

static void DSP3_OP1E_A2 (void)
{
	DSP3->op1e_terrain[DSP3->op1e_cell] = (uint8_t) (DSP3->DR & 0x00ff);

	DSP3->SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_A3;
}

static void DSP3_OP1E_A3 (void)
{
	DSP3->op1e_cost[DSP3->op1e_cell] = (uint8_t) (DSP3->DR & 0x00ff);

	if (DSP3->op1e_lcv_radius == 1)
	{
		if (DSP3->op1e_terrain[DSP3->op1e_cell] & 1)
			DSP3->op1e_weight[DSP3->op1e_cell] = 0xff;
		else
			DSP3->op1e_weight[DSP3->op1e_cell] = DSP3->op1e_cost[DSP3->op1e_cell];
	}
	else
		DSP3->op1e_weight[DSP3->op1e_cell] = 0xff;

	DSP3_OP1E_D((int16_t) (DSP3->op1e_turn + 2), &DSP3->op1e_x, &DSP3->op1e_y);
	DSP3->op1e_lcv_steps--;

	DSP3->SR = 0x0080;
	DSP3_OP1E_A();
}

static void DSP3_OP1E_B (void)
{
	DSP3->op1e_x = DSP3-> op3e_x;
	DSP3->op1e_y = DSP3-> op3e_y;
	DSP3->op1e_lcv_radius = 1;

	DSP3->op1e_search = 0;

	DSP3_OP1E_B1();

	SetDSP3 = &DSP3_OP1E_C;
}

static void DSP3_OP1E_B1 (void)
{
	while (DSP3->op1e_lcv_radius < DSP3->op1e_max_radius)
	{
		DSP3->op1e_y--;

		DSP3->op1e_lcv_turns = 6;
		DSP3->op1e_turn = 5;

		while (DSP3->op1e_lcv_turns)
		{
			DSP3->op1e_lcv_steps = DSP3->op1e_lcv_radius;

			while (DSP3->op1e_lcv_steps)
			{
				DSP3_OP1E_D1(DSP3->op1e_turn, &DSP3->op1e_x, &DSP3->op1e_y);

				if (0 <= DSP3->op1e_y && DSP3->op1e_y < DSP3->WinHi && 0 <= DSP3->op1e_x && DSP3->op1e_x < DSP3->WinLo)
				{
					DSP3->DR = (uint8_t) (DSP3->op1e_x) | ((uint8_t) (DSP3->op1e_y) << 8);
					DSP3_OP03();

					DSP3->op1e_cell = DSP3->DR;
					if (DSP3->op1e_cost[DSP3->op1e_cell] < 0x80 && DSP3->op1e_terrain[DSP3->op1e_cell] < 0x40)
						DSP3_OP1E_B2(); /* end cell perimeter */
				}

				DSP3->op1e_lcv_steps--;
			} /* end search line */

			DSP3->op1e_turn--;
			if (DSP3->op1e_turn == 0)
				DSP3->op1e_turn = 6;

			DSP3->op1e_lcv_turns--;
		} /* end circle search */

		DSP3->op1e_lcv_radius++;
	} /* end radius search */
}

static void DSP3_OP1E_B2 (void)
{
	int16_t	cell, path, x, y, lcv_turns;

	path = 0xff;
	lcv_turns = 6;

	while (lcv_turns)
	{
		x = DSP3->op1e_x;
		y = DSP3->op1e_y;

		DSP3_OP1E_D1(lcv_turns, &x, &y);

		DSP3->DR = (uint8_t) (x) | ((uint8_t) (y) << 8);
		DSP3_OP03();

		cell = DSP3->DR;

		if (0 <= y && y < DSP3->WinHi && 0 <= x && x < DSP3->WinLo)
		{
			if (DSP3->op1e_terrain[cell] < 0x80 || DSP3->op1e_weight[cell] == 0)
			{
				if (DSP3->op1e_weight[cell] < path)
					path = DSP3->op1e_weight[cell];
			}
		} /* end step travel */

		lcv_turns--;
	} /* end while turns */

	if (path != 0xff)
		DSP3->op1e_weight[DSP3->op1e_cell] = path + DSP3->op1e_cost[DSP3->op1e_cell];
}

static void DSP3_OP1E_C (void)
{
	int lcv;
	DSP3->op1e_min_radius = (uint8_t)  (DSP3->DR & 0x00ff);
	DSP3->op1e_max_radius = (uint8_t) ((DSP3->DR & 0xff00) >> 8);

	if (DSP3->op1e_min_radius == 0)
		DSP3->op1e_min_radius = 1;

	if (DSP3->op1e_max_path_radius >= DSP3->op1e_min_radius)
		DSP3->op1e_min_radius = DSP3->op1e_max_path_radius + 1;

	if (DSP3->op1e_max_radius > DSP3->op1e_max_path_radius)
		DSP3->op1e_max_path_radius = DSP3->op1e_max_radius;

	DSP3->op1e_lcv_radius = DSP3->op1e_min_radius;
	DSP3->op1e_lcv_steps = DSP3->op1e_min_radius;

	DSP3->op1e_lcv_turns = 6;
	DSP3->op1e_turn = 0;

	DSP3->op1e_x = DSP3-> op3e_x;
	DSP3->op1e_y = DSP3-> op3e_y;

	for ( lcv = 0; lcv < DSP3->op1e_min_radius; lcv++)
		DSP3_OP1E_D(DSP3->op1e_turn, &DSP3->op1e_x, &DSP3->op1e_y);

	DSP3_OP1E_C1();
}

static void DSP3_OP1E_C1 (void)
{
	int lcv;
	if (DSP3->op1e_lcv_steps == 0)
	{
		DSP3->op1e_lcv_radius++;

		DSP3->op1e_lcv_steps = DSP3->op1e_lcv_radius;

		DSP3->op1e_x = DSP3-> op3e_x;
		DSP3->op1e_y = DSP3-> op3e_y;

		for ( lcv = 0; lcv < DSP3->op1e_lcv_radius; lcv++)
			DSP3_OP1E_D(DSP3->op1e_turn, &DSP3->op1e_x, &DSP3->op1e_y);
	}

	if (DSP3->op1e_lcv_radius > DSP3->op1e_max_radius)
	{
		DSP3->op1e_turn++;
		DSP3->op1e_lcv_turns--;

		DSP3->op1e_lcv_radius = DSP3->op1e_min_radius;
		DSP3->op1e_lcv_steps = DSP3->op1e_min_radius;

		DSP3->op1e_x = DSP3-> op3e_x;
		DSP3->op1e_y = DSP3-> op3e_y;

		for ( lcv = 0; lcv < DSP3->op1e_min_radius; lcv++)
			DSP3_OP1E_D(DSP3->op1e_turn, &DSP3->op1e_x, &DSP3->op1e_y);
	}

	if (DSP3->op1e_lcv_turns == 0)
	{
		DSP3->DR = 0xffff;
		DSP3->SR = 0x0080;
		SetDSP3 = &DSP3_Reset;
		return;
	}

	DSP3->DR = (uint8_t) (DSP3->op1e_x) | ((uint8_t) (DSP3->op1e_y) << 8);
	DSP3_OP03();

	DSP3->op1e_cell = DSP3->DR;

	DSP3->SR = 0x0080;
	SetDSP3 = &DSP3_OP1E_C2;
}

static void DSP3_OP1E_C2 (void)
{
	DSP3->DR = DSP3->op1e_weight[DSP3->op1e_cell];

	DSP3_OP1E_D((int16_t) (DSP3->op1e_turn + 2), &DSP3->op1e_x, &DSP3->op1e_y);
	DSP3->op1e_lcv_steps--;

	DSP3->SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_C1;
}

static void DSP3_OP1E_D (int16_t move, int16_t *lo, int16_t *hi)
{
	uint32_t dataOfs;
	int16_t	Lo;
	int16_t	Hi;

	dataOfs = ((move << 1) + 0x03b2) & 0x03ff;

	DSP3->AddHi = DSP3_DataROM[dataOfs];
	DSP3->AddLo = DSP3_DataROM[dataOfs + 1];

	Lo = (uint8_t) (*lo);
	Hi = (uint8_t) (*hi);

	if (Lo & 1)
		Hi += (DSP3->AddLo & 1);

	DSP3->AddLo += Lo;
	DSP3->AddHi += Hi;

	if (DSP3->AddLo < 0)
		DSP3->AddLo += DSP3->WinLo;
	else
	if (DSP3->AddLo >= DSP3->WinLo)
		DSP3->AddLo -= DSP3->WinLo;

	if (DSP3->AddHi < 0)
		DSP3->AddHi += DSP3->WinHi;
	else
	if (DSP3->AddHi >= DSP3->WinHi)
		DSP3->AddHi -= DSP3->WinHi;

	*lo = DSP3->AddLo;
	*hi = DSP3->AddHi;
}

static void DSP3_OP1E_D1 (int16_t move, int16_t *lo, int16_t *hi)
{
	static const uint16_t	HiAdd[] =
	{
		0x00, 0xFF, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0xFF, 0x00
	};

	static const uint16_t	LoAdd[] =
	{
		0x00, 0x00, 0x01, 0x01, 0x00, 0xFF, 0xFF, 0x00
	};

	int16_t	Lo, Hi;

	if ((*lo) & 1)
		DSP3->AddHi = HiAdd[move + 8];
	else
		DSP3->AddHi = HiAdd[move + 0];

	DSP3->AddLo = LoAdd[move];

	Lo = (uint8_t) (*lo);
	Hi = (uint8_t) (*hi);

	if (Lo & 1)
		Hi += (DSP3->AddLo & 1);

	DSP3->AddLo += Lo;
	DSP3->AddHi += Hi;

	*lo = DSP3->AddLo;
	*hi = DSP3->AddHi;
}

static void DSP3_OP10 (void)
{
	if (DSP3->DR == 0xffff)
		DSP3_Reset();
	else
		DSP3->DR = DSP3->DR;	/* absorb 2 bytes */
}

/*
static void DSP3_OP0C_A (void)
{
	// absorb 2 bytes
	DSP3->DR = 0;
	SetDSP3 = &DSP3_Reset;
}
*/

static void DSP3_OP0C (void)
{
	/* absorb 2 bytes */
	DSP3->DR = 0;
	/* SetDSP3 = &DSP3_OP0C_A; */
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP1C_C (void)
{
	/* return 2 bytes */
	DSP3->DR = 0;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP1C_B (void)
{
	/* return 2 bytes */
	DSP3->DR = 0;
	SetDSP3 = &DSP3_OP1C_C;
}

static void DSP3_OP1C_A (void)
{
	/* absorb 2 bytes */
	SetDSP3 = &DSP3_OP1C_B;
}

static void DSP3_OP1C (void)
{
	/* absorb 2 bytes */
	SetDSP3 = &DSP3_OP1C_A;
}

static void DSP3_Command (void)
{
	if (DSP3->DR < 0x40)
	{
		switch (DSP3->DR)
		{
			case 0x02: SetDSP3 = &DSP3_Coordinate; break;
			case 0x03: SetDSP3 = &DSP3_OP03;       break;
			case 0x06: SetDSP3 = &DSP3_OP06;       break;
			case 0x07: SetDSP3 = &DSP3_OP07;       return;
			case 0x0c: SetDSP3 = &DSP3_OP0C;       break;
			case 0x0f: SetDSP3 = &DSP3_TestMemory; break;
			case 0x10: SetDSP3 = &DSP3_OP10;       break;
			case 0x18: SetDSP3 = &DSP3_Convert;    break;
			case 0x1c: SetDSP3 = &DSP3_OP1C;       break;
			case 0x1e: SetDSP3 = &DSP3_OP1E;       break;
			case 0x1f: SetDSP3 = &DSP3_MemoryDump; break;
			case 0x38: SetDSP3 = &DSP3_Decode;     break;
			case 0x3e: SetDSP3 = &DSP3_OP3E;       break;
			default:
				return;
		}

		DSP3->SR = 0x0080;
		DSP3->Index = 0;
	}
}

void DSP3SetByte (uint8_t byte, uint16_t address)
{
	if (address < 0xC000)
	{
		if (DSP3->SR & 0x04)
		{
			DSP3->DR = (DSP3->DR & 0xff00) + byte;
			(*SetDSP3)();
		}
		else
		{
			DSP3->SR ^= 0x10;

			if (DSP3->SR & 0x10)
				DSP3->DR = (DSP3->DR & 0xff00) + byte;
			else
			{
				DSP3->DR = (DSP3->DR & 0x00ff) + (byte << 8);
				(*SetDSP3)();
			}
		}
	}
}

uint8_t DSP3GetByte (uint16_t address)
{
	if (address < 0xC000)
	{
		uint8_t	byte;

		if (DSP3->SR & 0x04)
		{
			byte = (uint8_t) DSP3->DR;
			(*SetDSP3)();
		}
		else
		{
			DSP3->SR ^= 0x10;

			if (DSP3->SR & 0x10)
				byte = (uint8_t) (DSP3->DR);
			else
			{
				byte = (uint8_t) (DSP3->DR >> 8);
				(*SetDSP3)();
			}
		}

		return (byte);
	}

	return (uint8_t) DSP3->SR;
}

/***********************************************************************************
 DSP4
***********************************************************************************/

/*
  Due recognition and credit are given on Overload's DSP website.
  Thank those contributors for their hard work on this chip.

  Fixed-point math reminder:
  [sign, integer, fraction]
  1.15.00 * 1.15.00 = 2.30.00 -> 1.30.00 (DSP) -> 1.31.00 (LSB is '0')
  1.15.00 * 1.00.15 = 2.15.15 -> 1.15.15 (DSP) -> 1.15.16 (LSB is '0')
*/

#define DSP4_CLEAR_OUT() \
	{ DSP4->out_count = 0; DSP4->out_index = 0; }

#define DSP4_WRITE_BYTE(d) \
	{ WRITE_WORD(DSP4->output + DSP4->out_count, (d)); DSP4->out_count++; }

#define DSP4_WRITE_WORD(d) \
	{ WRITE_WORD(DSP4->output + DSP4->out_count, (d)); DSP4->out_count += 2; }

#ifndef MSB_FIRST
#define DSP4_WRITE_16_WORD(d) \
	{ memcpy(DSP4->output + DSP4->out_count, (d), 32); DSP4->out_count += 32; }
#else
#define DSP4_WRITE_16_WORD(d) \
	{ int p; for ( p = 0; p < 16; p++) DSP4_WRITE_WORD((d)[p]); }
#endif

/* used to wait for dsp i/o */
#define DSP4_WAIT(x) \
	DSP4->in_index = 0; DSP4->Logic = (x); return

/* 1.7.8 -> 1.15.16 */
#define SEX78(a)	(((int32_t) ((int16_t) (a))) << 8)

/* 1.15.0 -> 1.15.16 */
#define DSP_SEX16(a)	(((int32_t) ((int16_t) (a))) << 16)

static int16_t DSP4_READ_WORD (void)
{
	int16_t	out;

	out = READ_WORD(DSP4->parameters + DSP4->in_index);
	DSP4->in_index += 2;

	return (out);
}

static int32_t DSP4_READ_DWORD (void)
{
	int32_t	out;

	out = READ_DWORD(DSP4->parameters + DSP4->in_index);
	DSP4->in_index += 4;

	return (out);
}

static int16_t DSP4_Inverse (int16_t value)
{
	/* Attention: This lookup table is not verified */
	static const uint16_t	div_lut[64] =
	{
		0x0000, 0x8000, 0x4000, 0x2aaa, 0x2000, 0x1999, 0x1555, 0x1249,
		0x1000, 0x0e38, 0x0ccc, 0x0ba2, 0x0aaa, 0x09d8, 0x0924, 0x0888,
		0x0800, 0x0787, 0x071c, 0x06bc, 0x0666, 0x0618, 0x05d1, 0x0590,
		0x0555, 0x051e, 0x04ec, 0x04bd, 0x0492, 0x0469, 0x0444, 0x0421,
		0x0400, 0x03e0, 0x03c3, 0x03a8, 0x038e, 0x0375, 0x035e, 0x0348,
		0x0333, 0x031f, 0x030c, 0x02fa, 0x02e8, 0x02d8, 0x02c8, 0x02b9,
		0x02aa, 0x029c, 0x028f, 0x0282, 0x0276, 0x026a, 0x025e, 0x0253,
		0x0249, 0x023e, 0x0234, 0x022b, 0x0222, 0x0219, 0x0210, 0x0208
	};

	/* saturate bounds */
	if (value < 0)
		value = 0;
	if (value > 63)
		value = 63;

	return (div_lut[value]);
}

static void DSP4_Multiply (int16_t Multiplicand, int16_t Multiplier, int32_t *Product)
{
	*Product = (Multiplicand * Multiplier << 1) >> 1;
}

static void DSP4_OP01 (void)
{
	DSP4->waiting4command = false;

	/* op flow control */
	switch (DSP4->Logic)
   {
      case 1:
         goto resume1;;
      case 2:
         goto resume2;
      case 3:
         goto resume3;
   }

	/* process initial inputs*/

	/* sort inputs*/
	DSP4->world_y           = DSP4_READ_DWORD();
	DSP4->poly_bottom[0][0] = DSP4_READ_WORD();
	DSP4->poly_top[0][0]    = DSP4_READ_WORD();
	DSP4->poly_cx[1][0]     = DSP4_READ_WORD();
	DSP4->viewport_bottom   = DSP4_READ_WORD();
	DSP4->world_x           = DSP4_READ_DWORD();
	DSP4->poly_cx[0][0]     = DSP4_READ_WORD();
	DSP4->poly_ptr[0][0]    = DSP4_READ_WORD();
	DSP4->world_yofs        = DSP4_READ_WORD();
	DSP4->world_dy          = DSP4_READ_DWORD();
	DSP4->world_dx          = DSP4_READ_DWORD();
	DSP4->distance          = DSP4_READ_WORD();
	DSP4_READ_WORD(); /* 0x0000*/
	DSP4->world_xenv        = DSP4_READ_DWORD();
	DSP4->world_ddy         = DSP4_READ_WORD();
	DSP4->world_ddx         = DSP4_READ_WORD();
	DSP4->view_yofsenv      = DSP4_READ_WORD();

	/* initial (x, y, offset) at starting raster line*/
	DSP4->view_x1         = (DSP4->world_x + DSP4->world_xenv) >> 16;
	DSP4->view_y1         = DSP4->world_y >> 16;
	DSP4->view_xofs1      = DSP4->world_x >> 16;
	DSP4->view_yofs1      = DSP4->world_yofs;
	DSP4->view_turnoff_x  = 0;
	DSP4->view_turnoff_dx = 0;

	/* first raster line*/
	DSP4->poly_raster[0][0] = DSP4->poly_bottom[0][0];

	do
	{
		/* process one iteration of projection */

		/* perspective projection of world (x, y, scroll) points*/
		/* based on the current projection lines*/
		DSP4->view_x2    = (((DSP4->world_x + DSP4->world_xenv) >> 16) * DSP4->distance >> 15) + (DSP4->view_turnoff_x * DSP4->distance >> 15);
		DSP4->view_y2    = (DSP4->world_y >> 16) * DSP4->distance >> 15;
		DSP4->view_xofs2 = DSP4->view_x2;
		DSP4->view_yofs2 = (DSP4->world_yofs * DSP4->distance >> 15) + DSP4->poly_bottom[0][0] - DSP4->view_y2;

		/* 1. World x-location before transformation*/
		/* 2. Viewer x-position at the next*/
		/* 3. World y-location before perspective projection*/
		/* 4. Viewer y-position below the horizon*/
		/* 5. Number of raster lines drawn in this iteration*/
		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD((DSP4->world_x + DSP4->world_xenv) >> 16);
		DSP4_WRITE_WORD(DSP4->view_x2);
		DSP4_WRITE_WORD(DSP4->world_y >> 16);
		DSP4_WRITE_WORD(DSP4->view_y2);

		/* SR = 0x00*/

		/* determine # of raster lines used*/
		DSP4->segments = DSP4->poly_raster[0][0] - DSP4->view_y2;

		/* prevent overdraw*/
		if (DSP4->view_y2 >= DSP4->poly_raster[0][0])
			DSP4->segments = 0;
		else
			DSP4->poly_raster[0][0] = DSP4->view_y2;

		/* don't draw outside the window*/
		if (DSP4->view_y2 < DSP4->poly_top[0][0])
		{
			DSP4->segments = 0;

			/* flush remaining raster lines*/
			if (DSP4->view_y1 >= DSP4->poly_top[0][0])
				DSP4->segments = DSP4->view_y1 - DSP4->poly_top[0][0];
		}

		/* SR = 0x80*/

		DSP4_WRITE_WORD(DSP4->segments);

		/* scan next command if no SR check needed*/
		if (DSP4->segments)
		{
			int32_t	px_dx, py_dy;
			int32_t	x_scroll, y_scroll;

			/* SR = 0x00*/

			/* linear interpolation (lerp) between projected points*/
			px_dx = (DSP4->view_xofs2 - DSP4->view_xofs1) * DSP4_Inverse(DSP4->segments) << 1;
			py_dy = (DSP4->view_yofs2 - DSP4->view_yofs1) * DSP4_Inverse(DSP4->segments) << 1;

			/* starting step values*/
			x_scroll = DSP_SEX16(DSP4->poly_cx[0][0] + DSP4->view_xofs1);
			y_scroll = DSP_SEX16(-DSP4->viewport_bottom + DSP4->view_yofs1 + DSP4->view_yofsenv + DSP4->poly_cx[1][0] - DSP4->world_yofs);

			/* SR = 0x80*/

			/* rasterize line*/
			for (DSP4->lcv = 0; DSP4->lcv < DSP4->segments; DSP4->lcv++)
			{
				/* 1. HDMA memory pointer (bg1)*/
				/* 2. vertical scroll offset ($210E)*/
				/* 3. horizontal scroll offset ($210D)*/
				DSP4_WRITE_WORD(DSP4->poly_ptr[0][0]);
				DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
				DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);

				/* update memory address*/
				DSP4->poly_ptr[0][0] -= 4;

				/* update screen values*/
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		/* Post-update*/

		/* update new viewer (x, y, scroll) to last raster line drawn*/
		DSP4->view_x1    = DSP4->view_x2;
		DSP4->view_y1    = DSP4->view_y2;
		DSP4->view_xofs1 = DSP4->view_xofs2;
		DSP4->view_yofs1 = DSP4->view_yofs2;

		/* add deltas for projection lines*/
		DSP4->world_dx += SEX78(DSP4->world_ddx);
		DSP4->world_dy += SEX78(DSP4->world_ddy);

		/* update projection lines*/
		DSP4->world_x += (DSP4->world_dx + DSP4->world_xenv);
		DSP4->world_y += DSP4->world_dy;

		/* update road turnoff position*/
		DSP4->view_turnoff_x += DSP4->view_turnoff_dx;

		/* command check*/

		/* scan next command*/
		DSP4->in_count = 2;
		DSP4_WAIT(1);

		resume1:

		/* check for termination*/
		DSP4->distance = DSP4_READ_WORD();
		if (DSP4->distance == -0x8000)
			break;

		/* road turnoff*/
		if ((uint16_t) DSP4->distance == 0x8001)
		{
			DSP4->in_count = 6;
			DSP4_WAIT(2);

			resume2:

			DSP4->distance        = DSP4_READ_WORD();
			DSP4->view_turnoff_x  = DSP4_READ_WORD();
			DSP4->view_turnoff_dx = DSP4_READ_WORD();

			/* factor in new changes*/
			DSP4->view_x1    += (DSP4->view_turnoff_x * DSP4->distance >> 15);
			DSP4->view_xofs1 += (DSP4->view_turnoff_x * DSP4->distance >> 15);

			/* update stepping values*/
			DSP4->view_turnoff_x += DSP4->view_turnoff_dx;

			DSP4->in_count = 2;
			DSP4_WAIT(1);
		}

		/* already have 2 bytes read*/
		DSP4->in_count = 6;
		DSP4_WAIT(3);

		resume3:

		/* inspect inputs*/
		DSP4->world_ddy    = DSP4_READ_WORD();
		DSP4->world_ddx    = DSP4_READ_WORD();
		DSP4->view_yofsenv = DSP4_READ_WORD();

		/* no envelope here*/
		DSP4->world_xenv = 0;
	}
	while (1);

	/* terminate op*/
	DSP4->waiting4command = true;
}

static void DSP4_OP03 (void)
{
	DSP4->OAM_RowMax = 33;
	memset(DSP4->OAM_Row, 0, 64);
}

static void DSP4_OP05 (void)
{
	DSP4->OAM_index = 0;
	DSP4->OAM_bits = 0;
	memset(DSP4->OAM_attr, 0, 32);
	DSP4->sprite_count = 0;
}

static void DSP4_OP06 (void)
{
	DSP4_CLEAR_OUT();
	DSP4_WRITE_16_WORD(DSP4->OAM_attr);
}

static void DSP4_OP07 (void)
{
	DSP4->waiting4command = false;

	/* op flow control*/
	switch (DSP4->Logic)
	{
		case 1:
         goto resume1;
		case 2:
         goto resume2;
	}

	/* sort inputs*/

	DSP4->world_y           = DSP4_READ_DWORD();
	DSP4->poly_bottom[0][0] = DSP4_READ_WORD();
	DSP4->poly_top[0][0]    = DSP4_READ_WORD();
	DSP4->poly_cx[1][0]     = DSP4_READ_WORD();
	DSP4->viewport_bottom   = DSP4_READ_WORD();
	DSP4->world_x           = DSP4_READ_DWORD();
	DSP4->poly_cx[0][0]     = DSP4_READ_WORD();
	DSP4->poly_ptr[0][0]    = DSP4_READ_WORD();
	DSP4->world_yofs        = DSP4_READ_WORD();
	DSP4->distance          = DSP4_READ_WORD();
	DSP4->view_y2           = DSP4_READ_WORD();
	DSP4->view_dy           = DSP4_READ_WORD() * DSP4->distance >> 15;
	DSP4->view_x2           = DSP4_READ_WORD();
	DSP4->view_dx           = DSP4_READ_WORD() * DSP4->distance >> 15;
	DSP4->view_yofsenv      = DSP4_READ_WORD();

	/* initial (x, y, offset) at starting raster line*/
	DSP4->view_x1    = DSP4->world_x >> 16;
	DSP4->view_y1    = DSP4->world_y >> 16;
	DSP4->view_xofs1 = DSP4->view_x1;
	DSP4->view_yofs1 = DSP4->world_yofs;

	/* first raster line*/
	DSP4->poly_raster[0][0] = DSP4->poly_bottom[0][0];

	do
	{
		/* process one iteration of projection*/

		/* add shaping*/
		DSP4->view_x2 += DSP4->view_dx;
		DSP4->view_y2 += DSP4->view_dy;

		/* vertical scroll calculation*/
		DSP4->view_xofs2 = DSP4->view_x2;
		DSP4->view_yofs2 = (DSP4->world_yofs * DSP4->distance >> 15) + DSP4->poly_bottom[0][0] - DSP4->view_y2;

		/* 1. Viewer x-position at the next*/
		/* 2. Viewer y-position below the horizon*/
		/* 3. Number of raster lines drawn in this iteration*/
		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD(DSP4->view_x2);
		DSP4_WRITE_WORD(DSP4->view_y2);

		/* SR = 0x00*/

		/* determine # of raster lines used*/
		DSP4->segments = DSP4->view_y1 - DSP4->view_y2;

		/* prevent overdraw*/
		if (DSP4->view_y2 >= DSP4->poly_raster[0][0])
			DSP4->segments = 0;
		else
			DSP4->poly_raster[0][0] = DSP4->view_y2;

		/* don't draw outside the window*/
		if (DSP4->view_y2 < DSP4->poly_top[0][0])
		{
			DSP4->segments = 0;

			/* flush remaining raster lines*/
			if (DSP4->view_y1 >= DSP4->poly_top[0][0])
				DSP4->segments = DSP4->view_y1 - DSP4->poly_top[0][0];
		}

		/* SR = 0x80*/

		DSP4_WRITE_WORD(DSP4->segments);

		/* scan next command if no SR check needed*/
		if (DSP4->segments)
		{
			int32_t	px_dx, py_dy;
			int32_t	x_scroll, y_scroll;

			/* SR = 0x00*/

			/* linear interpolation (lerp) between projected points*/
			px_dx = (DSP4->view_xofs2 - DSP4->view_xofs1) * DSP4_Inverse(DSP4->segments) << 1;
			py_dy = (DSP4->view_yofs2 - DSP4->view_yofs1) * DSP4_Inverse(DSP4->segments) << 1;

			/* starting step values*/
			x_scroll = DSP_SEX16(DSP4->poly_cx[0][0] + DSP4->view_xofs1);
			y_scroll = DSP_SEX16(-DSP4->viewport_bottom + DSP4->view_yofs1 + DSP4->view_yofsenv + DSP4->poly_cx[1][0] - DSP4->world_yofs);

			/* SR = 0x80*/

			/* rasterize line*/
			for (DSP4->lcv = 0; DSP4->lcv < DSP4->segments; DSP4->lcv++)
			{
				/* 1. HDMA memory pointer (bg2)*/
				/* 2. vertical scroll offset ($2110)*/
				/* 3. horizontal scroll offset ($210F)*/
				DSP4_WRITE_WORD(DSP4->poly_ptr[0][0]);
				DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
				DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);

				/* update memory address*/
				DSP4->poly_ptr[0][0] -= 4;

				/* update screen values*/
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		/* Post-update*/

		/* update new viewer (x, y, scroll) to last raster line drawn */
		DSP4->view_x1    = DSP4->view_x2;
		DSP4->view_y1    = DSP4->view_y2;
		DSP4->view_xofs1 = DSP4->view_xofs2;
		DSP4->view_yofs1 = DSP4->view_yofs2;

		/* command check*/

		/* scan next command*/
		DSP4->in_count = 2;
		DSP4_WAIT(1);

		resume1:

		/* check for opcode termination*/
		DSP4->distance = DSP4_READ_WORD();
		if (DSP4->distance == -0x8000)
			break;

		/* already have 2 bytes in queue*/
		DSP4->in_count = 10;
		DSP4_WAIT(2);

		resume2:

		/* inspect inputs*/
		DSP4->view_y2      = DSP4_READ_WORD();
		DSP4->view_dy      = DSP4_READ_WORD() * DSP4->distance >> 15;
		DSP4->view_x2      = DSP4_READ_WORD();
		DSP4->view_dx      = DSP4_READ_WORD() * DSP4->distance >> 15;
		DSP4->view_yofsenv = DSP4_READ_WORD();
	}
	while (1);

	DSP4->waiting4command = true;
}

static void DSP4_OP08 (void)
{
	int16_t	win_left, win_right;
	int16_t	view_x[2], view_y[2];
	int16_t	envelope[2][2];

	DSP4->waiting4command = false;

	/* op flow control*/
	switch (DSP4->Logic)
	{
		case 1:
         goto resume1;
		case 2:
         goto resume2;
	}

	/* process initial inputs for two polygons*/

	/* clip values*/
	DSP4->poly_clipRt[0][0] = DSP4_READ_WORD();
	DSP4->poly_clipRt[0][1] = DSP4_READ_WORD();
	DSP4->poly_clipRt[1][0] = DSP4_READ_WORD();
	DSP4->poly_clipRt[1][1] = DSP4_READ_WORD();

	DSP4->poly_clipLf[0][0] = DSP4_READ_WORD();
	DSP4->poly_clipLf[0][1] = DSP4_READ_WORD();
	DSP4->poly_clipLf[1][0] = DSP4_READ_WORD();
	DSP4->poly_clipLf[1][1] = DSP4_READ_WORD();

	/* unknown (constant) (ex. 1P/2P = $00A6, $00A6, $00A6, $00A6)*/
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();

	/* unknown (constant) (ex. 1P/2P = $00A5, $00A5, $00A7, $00A7)*/
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();

	/* polygon centering (left, right)*/
	DSP4->poly_cx[0][0] = DSP4_READ_WORD();
	DSP4->poly_cx[0][1] = DSP4_READ_WORD();
	DSP4->poly_cx[1][0] = DSP4_READ_WORD();
	DSP4->poly_cx[1][1] = DSP4_READ_WORD();

	/* HDMA pointer locations*/
	DSP4->poly_ptr[0][0] = DSP4_READ_WORD();
	DSP4->poly_ptr[0][1] = DSP4_READ_WORD();
	DSP4->poly_ptr[1][0] = DSP4_READ_WORD();
	DSP4->poly_ptr[1][1] = DSP4_READ_WORD();

	/* starting raster line below the horizon*/
	DSP4->poly_bottom[0][0] = DSP4_READ_WORD();
	DSP4->poly_bottom[0][1] = DSP4_READ_WORD();
	DSP4->poly_bottom[1][0] = DSP4_READ_WORD();
	DSP4->poly_bottom[1][1] = DSP4_READ_WORD();

	/* top boundary line to clip*/
	DSP4->poly_top[0][0] = DSP4_READ_WORD();
	DSP4->poly_top[0][1] = DSP4_READ_WORD();
	DSP4->poly_top[1][0] = DSP4_READ_WORD();
	DSP4->poly_top[1][1] = DSP4_READ_WORD();

	/* unknown*/
	/* (ex. 1P = $2FC8, $0034, $FF5C, $0035)*/
/*	*/
	/* (ex. 2P = $3178, $0034, $FFCC, $0035)*/
	/* (ex. 2P = $2FC8, $0034, $FFCC, $0035)*/
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();

	/* look at guidelines for both polygon shapes*/
	DSP4->distance = DSP4_READ_WORD();
	view_x[0] = DSP4_READ_WORD();
	view_y[0] = DSP4_READ_WORD();
	view_x[1] = DSP4_READ_WORD();
	view_y[1] = DSP4_READ_WORD();

	/* envelope shaping guidelines (one frame only)*/
	envelope[0][0] = DSP4_READ_WORD();
	envelope[0][1] = DSP4_READ_WORD();
	envelope[1][0] = DSP4_READ_WORD();
	envelope[1][1] = DSP4_READ_WORD();

	/* starting base values to project from*/
	DSP4->poly_start[0] = view_x[0];
	DSP4->poly_start[1] = view_x[1];

	/* starting raster lines to begin drawing*/
	DSP4->poly_raster[0][0] = view_y[0];
	DSP4->poly_raster[0][1] = view_y[0];
	DSP4->poly_raster[1][0] = view_y[1];
	DSP4->poly_raster[1][1] = view_y[1];

	/* starting distances*/
	DSP4->poly_plane[0] = DSP4->distance;
	DSP4->poly_plane[1] = DSP4->distance;

	/* SR = 0x00*/

	/* re-center coordinates*/
	win_left  = DSP4->poly_cx[0][0] - view_x[0] + envelope[0][0];
	win_right = DSP4->poly_cx[0][1] - view_x[0] + envelope[0][1];

	/* saturate offscreen data for polygon #1*/
	if (win_left  < DSP4->poly_clipLf[0][0])
		win_left  = DSP4->poly_clipLf[0][0];
	if (win_left  > DSP4->poly_clipRt[0][0])
		win_left  = DSP4->poly_clipRt[0][0];
	if (win_right < DSP4->poly_clipLf[0][1])
		win_right = DSP4->poly_clipLf[0][1];
	if (win_right > DSP4->poly_clipRt[0][1])
		win_right = DSP4->poly_clipRt[0][1];

	/* SR = 0x80*/

	/* initial output for polygon #1*/
	DSP4_CLEAR_OUT();
	DSP4_WRITE_BYTE(win_left  & 0xff);
	DSP4_WRITE_BYTE(win_right & 0xff);

	do
	{
		int16_t	polygon;

		/* command check*/

		/* scan next command*/
		DSP4->in_count = 2;
		DSP4_WAIT(1);

		resume1:

		/* terminate op*/
		DSP4->distance = DSP4_READ_WORD();
		if (DSP4->distance == -0x8000)
			break;

		/* already have 2 bytes in queue*/
		DSP4->in_count = 16;
		DSP4_WAIT(2);

		resume2:

		/* look at guidelines for both polygon shapes*/
		view_x[0] = DSP4_READ_WORD();
		view_y[0] = DSP4_READ_WORD();
		view_x[1] = DSP4_READ_WORD();
		view_y[1] = DSP4_READ_WORD();

		/* envelope shaping guidelines (one frame only)*/
		envelope[0][0] = DSP4_READ_WORD();
		envelope[0][1] = DSP4_READ_WORD();
		envelope[1][0] = DSP4_READ_WORD();
		envelope[1][1] = DSP4_READ_WORD();

		/* projection begins*/

		/* init*/
		DSP4_CLEAR_OUT();

		/* solid polygon renderer - 2 shapes*/

		for (polygon = 0; polygon < 2; polygon++)
		{
			int32_t	left_inc, right_inc;
			int16_t	x1_final, x2_final;
			int16_t	env[2][2];
			int16_t	poly;

			/* SR = 0x00*/

			/* # raster lines to draw*/
			DSP4->segments = DSP4->poly_raster[polygon][0] - view_y[polygon];

			/* prevent overdraw*/
			if (DSP4->segments > 0)
			{
				/* bump drawing cursor*/
				DSP4->poly_raster[polygon][0] = view_y[polygon];
				DSP4->poly_raster[polygon][1] = view_y[polygon];
			}
			else
				DSP4->segments = 0;

			/* don't draw outside the window*/
			if (view_y[polygon] < DSP4->poly_top[polygon][0])
			{
				DSP4->segments = 0;

				/* flush remaining raster lines*/
				if (view_y[polygon] >= DSP4->poly_top[polygon][0])
					DSP4->segments = view_y[polygon] - DSP4->poly_top[polygon][0];
			}

			/* SR = 0x80*/

			/* tell user how many raster structures to read in*/
			DSP4_WRITE_WORD(DSP4->segments);

			/* normal parameters*/
			poly = polygon;

			/* scan next command if no SR check needed*/
			if (DSP4->segments)
			{
				int32_t	w_left, w_right;

				/* road turnoff selection*/
				if ((uint16_t) envelope[polygon][0] == (uint16_t) 0xc001)
					poly = 1;
				else
				if (envelope[polygon][1] == 0x3fff)
					poly = 1;

				/* left side of polygon*/

				/* perspective correction on additional shaping parameters*/
				env[0][0] = envelope[polygon][0] * DSP4->poly_plane[poly] >> 15;
				env[0][1] = envelope[polygon][0] * DSP4->distance >> 15;

				/* project new shapes (left side)*/
				x1_final = view_x[poly] + env[0][0];
				x2_final = DSP4->poly_start[poly] + env[0][1];

				/* interpolate between projected points with shaping*/
				left_inc = (x2_final - x1_final) * DSP4_Inverse(DSP4->segments) << 1;
				if (DSP4->segments == 1)
					left_inc = -left_inc;

				/* right side of polygon*/

				/* perspective correction on additional shaping parameters*/
				env[1][0] = envelope[polygon][1] * DSP4->poly_plane[poly] >> 15;
				env[1][1] = envelope[polygon][1] * DSP4->distance >> 15;

				/* project new shapes (right side)*/
				x1_final = view_x[poly] + env[1][0];
				x2_final = DSP4->poly_start[poly] + env[1][1];

				/* interpolate between projected points with shaping*/
				right_inc = (x2_final - x1_final) * DSP4_Inverse(DSP4->segments) << 1;
				if (DSP4->segments == 1)
					right_inc = -right_inc;

				/* update each point on the line*/

				w_left  = DSP_SEX16(DSP4->poly_cx[polygon][0] - DSP4->poly_start[poly] + env[0][0]);
				w_right = DSP_SEX16(DSP4->poly_cx[polygon][1] - DSP4->poly_start[poly] + env[1][0]);

				/* update distance drawn into world*/
				DSP4->poly_plane[polygon] = DSP4->distance;

				/* rasterize line*/
				for (DSP4->lcv = 0; DSP4->lcv < DSP4->segments; DSP4->lcv++)
				{
					int16_t	x_left, x_right;

					/* project new coordinates*/
					w_left  += left_inc;
					w_right += right_inc;

					/* grab integer portion, drop fraction (no rounding)*/
					x_left  = w_left  >> 16;
					x_right = w_right >> 16;

					/* saturate offscreen data*/
					if (x_left  < DSP4->poly_clipLf[polygon][0])
						x_left  = DSP4->poly_clipLf[polygon][0];
					if (x_left  > DSP4->poly_clipRt[polygon][0])
						x_left  = DSP4->poly_clipRt[polygon][0];
					if (x_right < DSP4->poly_clipLf[polygon][1])
						x_right = DSP4->poly_clipLf[polygon][1];
					if (x_right > DSP4->poly_clipRt[polygon][1])
						x_right = DSP4->poly_clipRt[polygon][1];

					/* 1. HDMA memory pointer*/
					/* 2. Left window position ($2126/$2128)*/
					/* 3. Right window position ($2127/$2129)*/
					DSP4_WRITE_WORD(DSP4->poly_ptr[polygon][0]);
					DSP4_WRITE_BYTE(x_left  & 0xff);
					DSP4_WRITE_BYTE(x_right & 0xff);

					/* update memory pointers*/
					DSP4->poly_ptr[polygon][0] -= 4;
					DSP4->poly_ptr[polygon][1] -= 4;
				} /* end rasterize line*/
			}

			/* Post-update*/

			/* new projection spot to continue rasterizing from*/
			DSP4->poly_start[polygon] = view_x[poly];
		} /* end polygon rasterizer*/
	}
	while (1);

	/* unknown output*/
	DSP4_CLEAR_OUT();
	DSP4_WRITE_WORD(0);

	DSP4->waiting4command = true;
}

static void DSP4_OP0B (bool *draw, int16_t sp_x, int16_t sp_y, int16_t sp_attr, bool size, bool stop)
{
	int16_t	Row1, Row2;

	/* SR = 0x00*/

	/* align to nearest 8-pixel row*/
	Row1 = (sp_y >> 3) & 0x1f;
	Row2 = (Row1 + 1)  & 0x1f;

	/* check boundaries*/
	if (!((sp_y < 0) || ((sp_y & 0x01ff) < 0x00eb)))
		*draw = 0;

	if (size)
	{
		if (DSP4->OAM_Row[Row1] + 1 >= DSP4->OAM_RowMax)
			*draw = 0;
		if (DSP4->OAM_Row[Row2] + 1 >= DSP4->OAM_RowMax)
			*draw = 0;
	}
	else
	{
		if (DSP4->OAM_Row[Row1] >= DSP4->OAM_RowMax)
			*draw = 0;
	}

	/* emulator fail-safe (unknown if this really exists)*/
	if (DSP4->sprite_count >= 128)
		*draw = 0;

	/* SR = 0x80*/

	if (*draw)
	{
		/* Row tiles*/
		if (size)
		{
			DSP4->OAM_Row[Row1] += 2;
			DSP4->OAM_Row[Row2] += 2;
		}
		else
			DSP4->OAM_Row[Row1]++;

		/* yield OAM output*/
		DSP4_WRITE_WORD(1);

		/* pack OAM data: x, y, name, attr*/
		DSP4_WRITE_BYTE(sp_x & 0xff);
		DSP4_WRITE_BYTE(sp_y & 0xff);
		DSP4_WRITE_WORD(sp_attr);

		DSP4->sprite_count++;

		/* OAM: size, msb data*/
		/* save post-oam table data for future retrieval*/
		DSP4->OAM_attr[DSP4->OAM_index] |= ((sp_x < 0 || sp_x > 255) << DSP4->OAM_bits);
		DSP4->OAM_bits++;

		DSP4->OAM_attr[DSP4->OAM_index] |= (size << DSP4->OAM_bits);
		DSP4->OAM_bits++;

		/* move to next byte in buffer*/
		if (DSP4->OAM_bits == 16)
		{
			DSP4->OAM_bits = 0;
			DSP4->OAM_index++;
		}
	}
	else
	if (stop)
		/* yield no OAM output*/
		DSP4_WRITE_WORD(0);
}

static void DSP4_OP09 (void)
{
	DSP4->waiting4command = false;

	/* op flow control*/
	switch (DSP4->Logic)
	{
		case 1:
         goto resume1;
		case 2:
         goto resume2;
		case 3:
         goto resume3;
		case 4:
         goto resume4;
		case 5:
         goto resume5;
		case 6:
         goto resume6;
	}

	/* process initial inputs*/

	/* grab screen information*/
	DSP4->viewport_cx     = DSP4_READ_WORD();
	DSP4->viewport_cy     = DSP4_READ_WORD();
	DSP4_READ_WORD(); /* 0x0000*/
	DSP4->viewport_left   = DSP4_READ_WORD();
	DSP4->viewport_right  = DSP4_READ_WORD();
	DSP4->viewport_top    = DSP4_READ_WORD();
	DSP4->viewport_bottom = DSP4_READ_WORD();

	/* starting raster line below the horizon*/
	DSP4->poly_bottom[0][0] = DSP4->viewport_bottom - DSP4->viewport_cy;
	DSP4->poly_raster[0][0] = 0x100;

	do
	{
		/* check for new sprites*/

		DSP4->in_count = 4;
		DSP4_WAIT(1);

		resume1:

		/* raster overdraw check*/

		DSP4->raster = DSP4_READ_WORD();

		/* continue updating the raster line where overdraw begins*/
		if (DSP4->raster < DSP4->poly_raster[0][0])
		{
			DSP4->sprite_clipy = DSP4->viewport_bottom - (DSP4->poly_bottom[0][0] - DSP4->raster);
			DSP4->poly_raster[0][0] = DSP4->raster;
		}

		/* identify sprite*/

		/* op termination*/
		DSP4->distance = DSP4_READ_WORD();
		if (DSP4->distance == -0x8000)
			goto terminate;

		/* no sprite*/
		if (DSP4->distance == 0x0000)
			continue;

		/* process projection information*/

		/* vehicle sprite*/
		if ((uint16_t) DSP4->distance == 0x9000)
		{
			int16_t	car_left, car_right, car_back;
			int16_t	impact_left, impact_back;
			int16_t	world_spx, world_spy;
			int16_t	view_spx, view_spy;
			uint16_t	energy;

			/* we already have 4 bytes we want*/
			DSP4->in_count = 14;
			DSP4_WAIT(2);

			resume2:

			/* filter inputs*/
			energy        = DSP4_READ_WORD();
			impact_back   = DSP4_READ_WORD();
			car_back      = DSP4_READ_WORD();
			impact_left   = DSP4_READ_WORD();
			car_left      = DSP4_READ_WORD();
			DSP4->distance = DSP4_READ_WORD();
			car_right     = DSP4_READ_WORD();

			/* calculate car's world (x, y) values*/
			world_spx = car_right - car_left;
			world_spy = car_back;

			/* add in collision vector [needs bit-twiddling]*/
			world_spx -= energy * (impact_left - car_left) >> 16;
			world_spy -= energy * (car_back - impact_back) >> 16;

			/* perspective correction for world (x, y)*/
			view_spx = world_spx * DSP4->distance >> 15;
			view_spy = world_spy * DSP4->distance >> 15;

			/* convert to screen values*/
			DSP4->sprite_x = DSP4->viewport_cx + view_spx;
			DSP4->sprite_y = DSP4->viewport_bottom - (DSP4->poly_bottom[0][0] - view_spy);

			/* make the car's (x)-coordinate available*/
			DSP4_CLEAR_OUT();
			DSP4_WRITE_WORD(world_spx);

			/* grab a few remaining vehicle values*/
			DSP4->in_count = 4;
			DSP4_WAIT(3);

			resume3:

			/* add vertical lift factor*/
			DSP4->sprite_y += DSP4_READ_WORD();
		}
		/* terrain sprite*/
		else
		{
			int16_t	world_spx, world_spy;
			int16_t	view_spx, view_spy;

			/* we already have 4 bytes we want*/
			DSP4->in_count = 10;
			DSP4_WAIT(4);

			resume4:

			/* sort loop inputs*/
			DSP4->poly_cx[0][0]     = DSP4_READ_WORD();
			DSP4->poly_raster[0][1] = DSP4_READ_WORD();
			world_spx              = DSP4_READ_WORD();
			world_spy              = DSP4_READ_WORD();

			/* compute base raster line from the bottom*/
			DSP4->segments = DSP4->poly_bottom[0][0] - DSP4->raster;

			/* perspective correction for world (x, y)*/
			view_spx = world_spx * DSP4->distance >> 15;
			view_spy = world_spy * DSP4->distance >> 15;

			/* convert to screen values*/
			DSP4->sprite_x = DSP4->viewport_cx + view_spx - DSP4->poly_cx[0][0];
			DSP4->sprite_y = DSP4->viewport_bottom - DSP4->segments + view_spy;
		}

		/* default sprite size: 16x16*/
		DSP4->sprite_size = 1;
		DSP4->sprite_attr = DSP4_READ_WORD();

		/* convert tile data to SNES OAM format*/

		do
		{
			int16_t	sp_x, sp_y, sp_attr, sp_dattr;
			int16_t	sp_dx, sp_dy;
			int16_t	pixels;
			uint16_t	header;
			bool	draw;

			DSP4->in_count = 2;
			DSP4_WAIT(5);

			resume5:

			draw = true;

			/* opcode termination*/
			DSP4->raster = DSP4_READ_WORD();
			if (DSP4->raster == -0x8000)
				goto terminate;

			/* stop code*/
			if (DSP4->raster == 0x0000 && !DSP4->sprite_size)
				break;

			/* toggle sprite size*/
			if (DSP4->raster == 0x0000)
			{
				DSP4->sprite_size = !DSP4->sprite_size;
				continue;
			}

			/* check for valid sprite header*/
			header = DSP4->raster;
			header >>= 8;
			if (header != 0x20 &&
				header != 0x2e && /* This is for attractor sprite*/
				header != 0x40 &&
				header != 0x60 &&
				header != 0xa0 &&
				header != 0xc0 &&
				header != 0xe0)
				break;

			/* read in rest of sprite data*/
			DSP4->in_count = 4;
			DSP4_WAIT(6);

			resume6:

			draw = true;

			/* process tile data*/

			/* sprite deltas*/
			sp_dattr = DSP4->raster;
			sp_dy = DSP4_READ_WORD();
			sp_dx = DSP4_READ_WORD();

			/* update coordinates to screen space*/
			sp_x = DSP4->sprite_x + sp_dx;
			sp_y = DSP4->sprite_y + sp_dy;

			/* update sprite nametable/attribute information*/
			sp_attr = DSP4->sprite_attr + sp_dattr;

			/* allow partially visibile tiles*/
			pixels = DSP4->sprite_size ? 15 : 7;

			DSP4_CLEAR_OUT();

			/* transparent tile to clip off parts of a sprite (overdraw)*/
			if (DSP4->sprite_clipy - pixels <= sp_y && sp_y <= DSP4->sprite_clipy && sp_x >= DSP4->viewport_left - pixels && sp_x <= DSP4->viewport_right && DSP4->sprite_clipy >= DSP4->viewport_top - pixels && DSP4->sprite_clipy <= DSP4->viewport_bottom)
				DSP4_OP0B(&draw, sp_x, DSP4->sprite_clipy, 0x00EE, DSP4->sprite_size, 0);

			/* normal sprite tile*/
			if (sp_x >= DSP4->viewport_left - pixels && sp_x <= DSP4->viewport_right && sp_y >= DSP4->viewport_top - pixels && sp_y <= DSP4->viewport_bottom && sp_y <= DSP4->sprite_clipy)
				DSP4_OP0B(&draw, sp_x, sp_y, sp_attr, DSP4->sprite_size, 0);

			/* no following OAM data*/
			DSP4_OP0B(&draw, 0, 0x0100, 0, 0, 1);
		}
		while (1);
	}
	while (1);

	terminate:
	DSP4->waiting4command = true;
}

static void DSP4_OP0A (int16_t n2, int16_t *o1, int16_t *o2, int16_t *o3, int16_t *o4)
{
	static const uint16_t	OP0A_Values[16] =
	{
		0x0000, 0x0030, 0x0060, 0x0090, 0x00c0, 0x00f0, 0x0120, 0x0150,
		0xfe80, 0xfeb0, 0xfee0, 0xff10, 0xff40, 0xff70, 0xffa0, 0xffd0
	};

	*o4 = OP0A_Values[(n2 & 0x000f)];
	*o3 = OP0A_Values[(n2 & 0x00f0) >> 4];
	*o2 = OP0A_Values[(n2 & 0x0f00) >> 8];
	*o1 = OP0A_Values[(n2 & 0xf000) >> 12];
}


static void DSP4_OP0D (void)
{
	DSP4->waiting4command = false;

	/* op flow control*/
	switch (DSP4->Logic)
	{
		case 1:
         goto resume1;
		case 2:
         goto resume2;
	}

	/* process initial inputs*/

	/* sort inputs*/
	DSP4->world_y           = DSP4_READ_DWORD();
	DSP4->poly_bottom[0][0] = DSP4_READ_WORD();
	DSP4->poly_top[0][0]    = DSP4_READ_WORD();
	DSP4->poly_cx[1][0]     = DSP4_READ_WORD();
	DSP4->viewport_bottom   = DSP4_READ_WORD();
	DSP4->world_x           = DSP4_READ_DWORD();
	DSP4->poly_cx[0][0]     = DSP4_READ_WORD();
	DSP4->poly_ptr[0][0]    = DSP4_READ_WORD();
	DSP4->world_yofs        = DSP4_READ_WORD();
	DSP4->world_dy          = DSP4_READ_DWORD();
	DSP4->world_dx          = DSP4_READ_DWORD();
	DSP4->distance          = DSP4_READ_WORD();
	DSP4_READ_WORD(); /* 0x0000*/
	DSP4->world_xenv        = SEX78(DSP4_READ_WORD());
	DSP4->world_ddy         = DSP4_READ_WORD();
	DSP4->world_ddx         = DSP4_READ_WORD();
	DSP4->view_yofsenv      = DSP4_READ_WORD();

	/* initial (x, y, offset) at starting raster line*/
	DSP4->view_x1    = (DSP4->world_x + DSP4->world_xenv) >> 16;
	DSP4->view_y1    = DSP4->world_y >> 16;
	DSP4->view_xofs1 = DSP4->world_x >> 16;
	DSP4->view_yofs1 = DSP4->world_yofs;

	/* first raster line*/
	DSP4->poly_raster[0][0] = DSP4->poly_bottom[0][0];

	do
	{
		/* process one iteration of projection*/

		/* perspective projection of world (x, y, scroll) points*/
		/* based on the current projection lines*/
		DSP4->view_x2    = (((DSP4->world_x + DSP4->world_xenv) >> 16) * DSP4->distance >> 15) + (DSP4->view_turnoff_x * DSP4->distance >> 15);
		DSP4->view_y2    = (DSP4->world_y >> 16) * DSP4->distance >> 15;
		DSP4->view_xofs2 = DSP4->view_x2;
		DSP4->view_yofs2 = (DSP4->world_yofs * DSP4->distance >> 15) + DSP4->poly_bottom[0][0] - DSP4->view_y2;

		/* 1. World x-location before transformation*/
		/* 2. Viewer x-position at the current*/
		/* 3. World y-location before perspective projection*/
		/* 4. Viewer y-position below the horizon*/
		/* 5. Number of raster lines drawn in this iteration*/
		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD((DSP4->world_x + DSP4->world_xenv) >> 16);
		DSP4_WRITE_WORD(DSP4->view_x2);
		DSP4_WRITE_WORD(DSP4->world_y >> 16);
		DSP4_WRITE_WORD(DSP4->view_y2);


		/* SR = 0x00*/

		/* determine # of raster lines used*/
		DSP4->segments = DSP4->view_y1 - DSP4->view_y2;

		/* prevent overdraw*/
		if (DSP4->view_y2 >= DSP4->poly_raster[0][0])
			DSP4->segments = 0;
		else
			DSP4->poly_raster[0][0] = DSP4->view_y2;

		/* don't draw outside the window*/
		if (DSP4->view_y2 < DSP4->poly_top[0][0])
		{
			DSP4->segments = 0;

			/* flush remaining raster lines*/
			if (DSP4->view_y1 >= DSP4->poly_top[0][0])
				DSP4->segments = DSP4->view_y1 - DSP4->poly_top[0][0];
		}

		/* SR = 0x80*/

		DSP4_WRITE_WORD(DSP4->segments);


		/* scan next command if no SR check needed*/
		if (DSP4->segments)
		{
			int32_t	px_dx, py_dy;
			int32_t	x_scroll, y_scroll;

			/* SR = 0x00*/

			/* linear interpolation (lerp) between projected points*/
			px_dx = (DSP4->view_xofs2 - DSP4->view_xofs1) * DSP4_Inverse(DSP4->segments) << 1;
			py_dy = (DSP4->view_yofs2 - DSP4->view_yofs1) * DSP4_Inverse(DSP4->segments) << 1;

			/* starting step values*/
			x_scroll = DSP_SEX16(DSP4->poly_cx[0][0] + DSP4->view_xofs1);
			y_scroll = DSP_SEX16(-DSP4->viewport_bottom + DSP4->view_yofs1 + DSP4->view_yofsenv + DSP4->poly_cx[1][0] - DSP4->world_yofs);

			/* SR = 0x80*/

			/* rasterize line*/
			for (DSP4->lcv = 0; DSP4->lcv < DSP4->segments; DSP4->lcv++)
			{
				/* 1. HDMA memory pointer (bg1)*/
				/* 2. vertical scroll offset ($210E)*/
				/* 3. horizontal scroll offset ($210D)*/
				DSP4_WRITE_WORD(DSP4->poly_ptr[0][0]);
				DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
				DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);

				/* update memory address*/
				DSP4->poly_ptr[0][0] -= 4;

				/* update screen values*/
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		/* Post-update*/

		/* update new viewer (x, y, scroll) to last raster line drawn*/
		DSP4->view_x1    = DSP4->view_x2;
		DSP4->view_y1    = DSP4->view_y2;
		DSP4->view_xofs1 = DSP4->view_xofs2;
		DSP4->view_yofs1 = DSP4->view_yofs2;

		/* add deltas for projection lines*/
		DSP4->world_dx += SEX78(DSP4->world_ddx);
		DSP4->world_dy += SEX78(DSP4->world_ddy);

		/* update projection lines*/
		DSP4->world_x += (DSP4->world_dx + DSP4->world_xenv);
		DSP4->world_y += DSP4->world_dy;

		/* command check*/

		/* scan next command*/
		DSP4->in_count = 2;
		DSP4_WAIT(1);

		resume1:

		/* inspect input*/
		DSP4->distance = DSP4_READ_WORD();

		/* terminate op*/
		if (DSP4->distance == -0x8000)
			break;

		/* already have 2 bytes in queue*/
		DSP4->in_count = 6;
		DSP4_WAIT(2);

		resume2:

		/* inspect inputs*/
		DSP4->world_ddy    = DSP4_READ_WORD();
		DSP4->world_ddx    = DSP4_READ_WORD();
		DSP4->view_yofsenv = DSP4_READ_WORD();

		/* no envelope here*/
		DSP4->world_xenv = 0;
	}
	while (1);

	DSP4->waiting4command = true;
}

static void DSP4_OP0E (void)
{
	DSP4->OAM_RowMax = 16;
	memset(DSP4->OAM_Row, 0, 64);
}

static void DSP4_OP0F (void)
{
	DSP4->waiting4command = false;

	/* op flow control*/
	switch (DSP4->Logic)
	{
		case 1:
         goto resume1;
		case 2:
         goto resume2;
		case 3:
         goto resume3;
		case 4:
         goto resume4;
	}

	/* process initial inputs*/

	/* sort inputs*/
	DSP4_READ_WORD(); /* 0x0000*/
	DSP4->world_y           = DSP4_READ_DWORD();
	DSP4->poly_bottom[0][0] = DSP4_READ_WORD();
	DSP4->poly_top[0][0]    = DSP4_READ_WORD();
	DSP4->poly_cx[1][0]     = DSP4_READ_WORD();
	DSP4->viewport_bottom   = DSP4_READ_WORD();
	DSP4->world_x           = DSP4_READ_DWORD();
	DSP4->poly_cx[0][0]     = DSP4_READ_WORD();
	DSP4->poly_ptr[0][0]    = DSP4_READ_WORD();
	DSP4->world_yofs        = DSP4_READ_WORD();
	DSP4->world_dy          = DSP4_READ_DWORD();
	DSP4->world_dx          = DSP4_READ_DWORD();
	DSP4->distance          = DSP4_READ_WORD();
	DSP4_READ_WORD(); /* 0x0000*/
	DSP4->world_xenv        = DSP4_READ_DWORD();
	DSP4->world_ddy         = DSP4_READ_WORD();
	DSP4->world_ddx         = DSP4_READ_WORD();
	DSP4->view_yofsenv      = DSP4_READ_WORD();

	/* initial (x, y, offset) at starting raster line*/
	DSP4->view_x1         = (DSP4->world_x + DSP4->world_xenv) >> 16;
	DSP4->view_y1         = DSP4->world_y >> 16;
	DSP4->view_xofs1      = DSP4->world_x >> 16;
	DSP4->view_yofs1      = DSP4->world_yofs;
	DSP4->view_turnoff_x  = 0;
	DSP4->view_turnoff_dx = 0;

	/* first raster line*/
	DSP4->poly_raster[0][0] = DSP4->poly_bottom[0][0];

	do
	{
		/* process one iteration of projection*/

		/* perspective projection of world (x, y, scroll) points*/
		/* based on the current projection lines*/
		DSP4->view_x2    = ((DSP4->world_x + DSP4->world_xenv) >> 16) * DSP4->distance >> 15;
		DSP4->view_y2    = (DSP4->world_y >> 16) * DSP4->distance >> 15;
		DSP4->view_xofs2 = DSP4->view_x2;
		DSP4->view_yofs2 = (DSP4->world_yofs * DSP4->distance >> 15) + DSP4->poly_bottom[0][0] - DSP4->view_y2;

		/* 1. World x-location before transformation*/
		/* 2. Viewer x-position at the next*/
		/* 3. World y-location before perspective projection*/
		/* 4. Viewer y-position below the horizon*/
		/* 5. Number of raster lines drawn in this iteration*/
		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD((DSP4->world_x + DSP4->world_xenv) >> 16);
		DSP4_WRITE_WORD(DSP4->view_x2);
		DSP4_WRITE_WORD(DSP4->world_y >> 16);
		DSP4_WRITE_WORD(DSP4->view_y2);


		/* SR = 0x00*/

		/* determine # of raster lines used*/
		DSP4->segments = DSP4->poly_raster[0][0] - DSP4->view_y2;

		/* prevent overdraw*/
		if (DSP4->view_y2 >= DSP4->poly_raster[0][0])
			DSP4->segments = 0;
		else
			DSP4->poly_raster[0][0] = DSP4->view_y2;

		/* don't draw outside the window*/
		if (DSP4->view_y2 < DSP4->poly_top[0][0])
		{
			DSP4->segments = 0;

			/* flush remaining raster lines*/
			if (DSP4->view_y1 >= DSP4->poly_top[0][0])
				DSP4->segments = DSP4->view_y1 - DSP4->poly_top[0][0];
		}

		/* SR = 0x80*/

		DSP4_WRITE_WORD(DSP4->segments);

		/* scan next command if no SR check needed*/
		if (DSP4->segments)
		{
			int32_t	px_dx, py_dy;
			int32_t	x_scroll, y_scroll;

			for (DSP4->lcv = 0; DSP4->lcv < 4; DSP4->lcv++)
			{
				/* grab inputs*/
				DSP4->in_count = 4;
				DSP4_WAIT(1);

				resume1:

				for (;;)
				{
					int16_t	dist;
					int16_t	color, red, green, blue;

					dist  = DSP4_READ_WORD();
					color = DSP4_READ_WORD();

					/* U1+B5+G5+R5*/
					red   =  color        & 0x1f;
					green = (color >>  5) & 0x1f;
					blue  = (color >> 10) & 0x1f;

					/* dynamic lighting*/
					red   = (red   * dist >> 15) & 0x1f;
					green = (green * dist >> 15) & 0x1f;
					blue  = (blue  * dist >> 15) & 0x1f;
					color = red | (green << 5) | (blue << 10);

					DSP4_CLEAR_OUT();
					DSP4_WRITE_WORD(color);

					break;
				}
			}

			/* SR = 0x00*/

			/* linear interpolation (lerp) between projected points*/
			px_dx = (DSP4->view_xofs2 - DSP4->view_xofs1) * DSP4_Inverse(DSP4->segments) << 1;
			py_dy = (DSP4->view_yofs2 - DSP4->view_yofs1) * DSP4_Inverse(DSP4->segments) << 1;

			/* starting step values*/
			x_scroll = DSP_SEX16(DSP4->poly_cx[0][0] + DSP4->view_xofs1);
			y_scroll = DSP_SEX16(-DSP4->viewport_bottom + DSP4->view_yofs1 + DSP4->view_yofsenv + DSP4->poly_cx[1][0] - DSP4->world_yofs);

			/* SR = 0x80*/

			/* rasterize line*/
			for (DSP4->lcv = 0; DSP4->lcv < DSP4->segments; DSP4->lcv++)
			{
				/* 1. HDMA memory pointer*/
				/* 2. vertical scroll offset ($210E)*/
				/* 3. horizontal scroll offset ($210D)*/
				DSP4_WRITE_WORD(DSP4->poly_ptr[0][0]);
				DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
				DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);

				/* update memory address*/
				DSP4->poly_ptr[0][0] -= 4;

				/* update screen values*/
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		/* Post-update*/

		/* update new viewer (x, y, scroll) to last raster line drawn*/
		DSP4->view_x1    = DSP4->view_x2;
		DSP4->view_y1    = DSP4->view_y2;
		DSP4->view_xofs1 = DSP4->view_xofs2;
		DSP4->view_yofs1 = DSP4->view_yofs2;

		/* add deltas for projection lines*/
		DSP4->world_dx += SEX78(DSP4->world_ddx);
		DSP4->world_dy += SEX78(DSP4->world_ddy);

		/* update projection lines*/
		DSP4->world_x += (DSP4->world_dx + DSP4->world_xenv);
		DSP4->world_y += DSP4->world_dy;

		/* update road turnoff position*/
		DSP4->view_turnoff_x += DSP4->view_turnoff_dx;

		/* command check*/

		/* scan next command*/
		DSP4->in_count = 2;
		DSP4_WAIT(2);

		resume2:

		/* check for termination*/
		DSP4->distance = DSP4_READ_WORD();
		if (DSP4->distance == -0x8000)
			break;

		/* road splice*/
		if ((uint16_t) DSP4->distance == 0x8001)
		{
			DSP4->in_count = 6;
			DSP4_WAIT(3);

			resume3:

			DSP4->distance        = DSP4_READ_WORD();
			DSP4->view_turnoff_x  = DSP4_READ_WORD();
			DSP4->view_turnoff_dx = DSP4_READ_WORD();

			/* factor in new changes*/
			DSP4->view_x1    += (DSP4->view_turnoff_x * DSP4->distance >> 15);
			DSP4->view_xofs1 += (DSP4->view_turnoff_x * DSP4->distance >> 15);

			/* update stepping values*/
			DSP4->view_turnoff_x += DSP4->view_turnoff_dx;

			DSP4->in_count = 2;
			DSP4_WAIT(2);
		}

		/* already have 2 bytes in queue*/
		DSP4->in_count = 6;
		DSP4_WAIT(4);

		resume4:

		/* inspect inputs*/
		DSP4->world_ddy    = DSP4_READ_WORD();
		DSP4->world_ddx    = DSP4_READ_WORD();
		DSP4->view_yofsenv = DSP4_READ_WORD();

		/* no envelope here*/
		DSP4->world_xenv = 0;
	}
	while (1);

	/* terminate op*/
	DSP4->waiting4command = true;
}

static void DSP4_OP10 (void)
{
	DSP4->waiting4command = false;

	/* op flow control*/
	switch (DSP4->Logic)
	{
		case 1:
         goto resume1;
		case 2:
         goto resume2;
		case 3:
         goto resume3;
	}

	/* sort inputs*/

	DSP4_READ_WORD(); /* 0x0000*/
	DSP4->world_y           = DSP4_READ_DWORD();
	DSP4->poly_bottom[0][0] = DSP4_READ_WORD();
	DSP4->poly_top[0][0]    = DSP4_READ_WORD();
	DSP4->poly_cx[1][0]     = DSP4_READ_WORD();
	DSP4->viewport_bottom   = DSP4_READ_WORD();
	DSP4->world_x           = DSP4_READ_DWORD();
	DSP4->poly_cx[0][0]     = DSP4_READ_WORD();
	DSP4->poly_ptr[0][0]    = DSP4_READ_WORD();
	DSP4->world_yofs        = DSP4_READ_WORD();
	DSP4->distance          = DSP4_READ_WORD();
	DSP4->view_y2           = DSP4_READ_WORD();
	DSP4->view_dy           = DSP4_READ_WORD() * DSP4->distance >> 15;
	DSP4->view_x2           = DSP4_READ_WORD();
	DSP4->view_dx           = DSP4_READ_WORD() * DSP4->distance >> 15;
	DSP4->view_yofsenv      = DSP4_READ_WORD();

	/* initial (x, y, offset) at starting raster line*/
	DSP4->view_x1    = DSP4->world_x >> 16;
	DSP4->view_y1    = DSP4->world_y >> 16;
	DSP4->view_xofs1 = DSP4->view_x1;
	DSP4->view_yofs1 = DSP4->world_yofs;

	/* first raster line*/
	DSP4->poly_raster[0][0] = DSP4->poly_bottom[0][0];

	do
	{
		/* process one iteration of projection*/

		/* add shaping*/
		DSP4->view_x2 += DSP4->view_dx;
		DSP4->view_y2 += DSP4->view_dy;

		/* vertical scroll calculation*/
		DSP4->view_xofs2 = DSP4->view_x2;
		DSP4->view_yofs2 = (DSP4->world_yofs * DSP4->distance >> 15) + DSP4->poly_bottom[0][0] - DSP4->view_y2;

		/* 1. Viewer x-position at the next*/
		/* 2. Viewer y-position below the horizon*/
		/* 3. Number of raster lines drawn in this iteration*/
		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD(DSP4->view_x2);
		DSP4_WRITE_WORD(DSP4->view_y2);

		/* SR = 0x00*/

		/* determine # of raster lines used*/
		DSP4->segments = DSP4->view_y1 - DSP4->view_y2;

		/* prevent overdraw*/
		if (DSP4->view_y2 >= DSP4->poly_raster[0][0])
			DSP4->segments = 0;
		else
			DSP4->poly_raster[0][0] = DSP4->view_y2;

		/* don't draw outside the window*/
		if (DSP4->view_y2 < DSP4->poly_top[0][0])
		{
			DSP4->segments = 0;

			/* flush remaining raster lines*/
			if (DSP4->view_y1 >= DSP4->poly_top[0][0])
				DSP4->segments = DSP4->view_y1 - DSP4->poly_top[0][0];
		}

		/* SR = 0x80*/

		DSP4_WRITE_WORD(DSP4->segments);

		/* scan next command if no SR check needed*/
		if (DSP4->segments)
		{
			for (DSP4->lcv = 0; DSP4->lcv < 4; DSP4->lcv++)
			{
				/* grab inputs*/
				DSP4->in_count = 4;
				DSP4_WAIT(1);

				resume1:

				for (;;)
				{
					int16_t	dist;
					int16_t	color, red, green, blue;

					dist  = DSP4_READ_WORD();
					color = DSP4_READ_WORD();

					/* U1+B5+G5+R5*/
					red   =  color        & 0x1f;
					green = (color >>  5) & 0x1f;
					blue  = (color >> 10) & 0x1f;

					/* dynamic lighting*/
					red   = (red   * dist >> 15) & 0x1f;
					green = (green * dist >> 15) & 0x1f;
					blue  = (blue  * dist >> 15) & 0x1f;
					color = red | (green << 5) | (blue << 10);

					DSP4_CLEAR_OUT();
					DSP4_WRITE_WORD(color);

					break;
				}
			}
		}

		/* scan next command if no SR check needed*/
		if (DSP4->segments)
		{
			int32_t	px_dx, py_dy;
			int32_t	x_scroll, y_scroll;

			/* SR = 0x00*/

			/* linear interpolation (lerp) between projected points*/
			px_dx = (DSP4->view_xofs2 - DSP4->view_xofs1) * DSP4_Inverse(DSP4->segments) << 1;
			py_dy = (DSP4->view_yofs2 - DSP4->view_yofs1) * DSP4_Inverse(DSP4->segments) << 1;

			/* starting step values*/
			x_scroll = DSP_SEX16(DSP4->poly_cx[0][0] + DSP4->view_xofs1);
			y_scroll = DSP_SEX16(-DSP4->viewport_bottom + DSP4->view_yofs1 + DSP4->view_yofsenv + DSP4->poly_cx[1][0] - DSP4->world_yofs);

			/* SR = 0x80*/

			/* rasterize line*/
			for (DSP4->lcv = 0; DSP4->lcv < DSP4->segments; DSP4->lcv++)
			{
				/* 1. HDMA memory pointer (bg2)*/
				/* 2. vertical scroll offset ($2110)*/
				/* 3. horizontal scroll offset ($210F)*/
				DSP4_WRITE_WORD(DSP4->poly_ptr[0][0]);
				DSP4_WRITE_WORD((y_scroll + 0x8000) >> 16);
				DSP4_WRITE_WORD((x_scroll + 0x8000) >> 16);

				/* update memory address*/
				DSP4->poly_ptr[0][0] -= 4;

				/* update screen values*/
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		/* Post-update*/

		/* update new viewer (x, y, scroll) to last raster line drawn*/
		DSP4->view_x1    = DSP4->view_x2;
		DSP4->view_y1    = DSP4->view_y2;
		DSP4->view_xofs1 = DSP4->view_xofs2;
		DSP4->view_yofs1 = DSP4->view_yofs2;

		/* command check*/

		/* scan next command*/
		DSP4->in_count = 2;
		DSP4_WAIT(2);

		resume2:

		/* check for opcode termination*/
		DSP4->distance = DSP4_READ_WORD();
		if (DSP4->distance == -0x8000)
			break;

		/* already have 2 bytes in queue*/
		DSP4->in_count = 10;
		DSP4_WAIT(3);

		resume3:

		/* inspect inputs*/
		DSP4->view_y2 = DSP4_READ_WORD();
		DSP4->view_dy = DSP4_READ_WORD() * DSP4->distance >> 15;
		DSP4->view_x2 = DSP4_READ_WORD();
		DSP4->view_dx = DSP4_READ_WORD() * DSP4->distance >> 15;
	}
	while (1);

	DSP4->waiting4command = true;
}

static void DSP4_OP11 (int16_t A, int16_t B, int16_t C, int16_t D, int16_t *M)
{
	/* 0x155 = 341 = Horizontal Width of the Screen*/
	*M = ((A * 0x0155 >> 2) & 0xf000) | ((B * 0x0155 >> 6) & 0x0f00) | ((C * 0x0155 >> 10) & 0x00f0) | ((D * 0x0155 >> 14) & 0x000f);
}

static void DSP4_SetByte (void)
{
	/* clear pending read*/
	if (DSP4->out_index < DSP4->out_count)
	{
		DSP4->out_index++;
		return;
	}

	if (DSP4->waiting4command)
	{
		if (DSP4->half_command)
		{
			DSP4->command |= (DSP4->byte << 8);
			DSP4->in_index        = 0;
			DSP4->waiting4command = false;
			DSP4->half_command    = false;
			DSP4->out_count       = 0;
			DSP4->out_index       = 0;

			DSP4->Logic = 0;

			switch (DSP4->command)
			{
				case 0x0000: DSP4->in_count =  4; break;
				case 0x0001: DSP4->in_count = 44; break;
				case 0x0003: DSP4->in_count =  0; break;
				case 0x0005: DSP4->in_count =  0; break;
				case 0x0006: DSP4->in_count =  0; break;
				case 0x0007: DSP4->in_count = 34; break;
				case 0x0008: DSP4->in_count = 90; break;
				case 0x0009: DSP4->in_count = 14; break;
				case 0x000a: DSP4->in_count =  6; break;
				case 0x000b: DSP4->in_count =  6; break;
				case 0x000d: DSP4->in_count = 42; break;
				case 0x000e: DSP4->in_count =  0; break;
				case 0x000f: DSP4->in_count = 46; break;
				case 0x0010: DSP4->in_count = 36; break;
				case 0x0011: DSP4->in_count =  8; break;
				default:
					DSP4->waiting4command = true;
					break;
			}
		}
		else
		{
			DSP4->command = DSP4->byte;
			DSP4->half_command = true;
		}
	}
	else
	{
		DSP4->parameters[DSP4->in_index] = DSP4->byte;
		DSP4->in_index++;
	}

	if (!DSP4->waiting4command && DSP4->in_count == DSP4->in_index)
	{
		/* Actually execute the command*/
		DSP4->waiting4command = true;
		DSP4->out_index       = 0;
		DSP4->in_index        = 0;

		switch (DSP4->command)
		{
			/* 16-bit multiplication*/
			case 0x0000:
			{
				int16_t	multiplier, multiplicand;
				int32_t	product;

				multiplier   = DSP4_READ_WORD();
				multiplicand = DSP4_READ_WORD();

				DSP4_Multiply(multiplicand, multiplier, &product);

				DSP4_CLEAR_OUT();
				DSP4_WRITE_WORD(product);
				DSP4_WRITE_WORD(product >> 16);

				break;
			}

			/* single-player track projection*/
			case 0x0001:
				DSP4_OP01();
				break;

			/* single-player selection*/
			case 0x0003:
				DSP4_OP03();
				break;

			/* clear OAM*/
			case 0x0005:
				DSP4_OP05();
				break;

			/* transfer OAM*/
			case 0x0006:
				DSP4_OP06();
				break;

			/* single-player track turnoff projection*/
			case 0x0007:
				DSP4_OP07();
				break;

			/* solid polygon projection*/
			case 0x0008:
				DSP4_OP08();
				break;

			/* sprite projection*/
			case 0x0009:
				DSP4_OP09();
				break;

			/* unknown*/
			case 0x000A:
			{
				int16_t in2a, out1a, out2a, out3a, out4a;

				DSP4_READ_WORD();
				in2a = DSP4_READ_WORD();
				DSP4_READ_WORD();

				DSP4_OP0A(in2a, &out2a, &out1a, &out4a, &out3a);

				DSP4_CLEAR_OUT();
				DSP4_WRITE_WORD(out1a);
				DSP4_WRITE_WORD(out2a);
				DSP4_WRITE_WORD(out3a);
				DSP4_WRITE_WORD(out4a);

				break;
			}

			/* set OAM*/
			case 0x000B:
			{
				int16_t	sp_x    = DSP4_READ_WORD();
				int16_t	sp_y    = DSP4_READ_WORD();
				int16_t	sp_attr = DSP4_READ_WORD();
				bool	draw = true;

				DSP4_CLEAR_OUT();
				DSP4_OP0B(&draw, sp_x, sp_y, sp_attr, 0, 1);

				break;
			}

			/* multi-player track projection*/
			case 0x000D:
				DSP4_OP0D();
				break;

			/* multi-player selection*/
			case 0x000E:
				DSP4_OP0E();
				break;

			/* single-player track projection with lighting*/
			case 0x000F:
				DSP4_OP0F();
				break;

			/* single-player track turnoff projection with lighting*/
			case 0x0010:
				DSP4_OP10();
				break;

			/* unknown: horizontal mapping command*/
			case 0x0011:
			{
				int16_t	a, b, c, d, m;

				d = DSP4_READ_WORD();
				c = DSP4_READ_WORD();
				b = DSP4_READ_WORD();
				a = DSP4_READ_WORD();

				DSP4_OP11(a, b, c, d, &m);

				DSP4_CLEAR_OUT();
				DSP4_WRITE_WORD(m);

				break;
			}

			default:
				break;
		}
	}
}

static void DSP4_GetByte (void)
{
	if (DSP4->out_count)
	{
		DSP4->byte = (uint8_t) DSP4->output[DSP4->out_index & 0x1FF];

		DSP4->out_index++;
		if (DSP4->out_count == DSP4->out_index)
			DSP4->out_count = 0;
	}
	else
		DSP4->byte = 0xff;
}

void DSP4SetByte (uint8_t byte, uint16_t address)
{
	if (address < 0xC000)
	{
		DSP4->byte    = byte;
		DSP4->address = address;
		DSP4_SetByte();
	}
}

uint8_t DSP4GetByte (uint16_t address)
{
	if (address < 0xC000)
	{
		DSP4->address = address;
		DSP4_GetByte();
		return (DSP4->byte);
	}

	return (0x80);
}
