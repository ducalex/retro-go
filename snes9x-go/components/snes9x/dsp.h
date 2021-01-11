/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _DSP1_H_
#define _DSP1_H_

enum
{
	M_DSP1_LOROM_S,
	M_DSP1_LOROM_L,
	M_DSP1_HIROM,
	M_DSP2_LOROM,
};

struct SDSP0
{
	uint32	maptype;
	uint32	boundary;
};

struct SDSP1
{
	bool8	waiting4command;
	bool8	first_parameter;
	uint8	command;
	uint32	in_count;
	uint32	in_index;
	uint32	out_count;
	uint32	out_index;
	uint8	parameters[512];
	uint8	output[512];

	int16	CentreX;
	int16	CentreY;
	int16	VOffset;

	int16	VPlane_C;
	int16	VPlane_E;

	// Azimuth and Zenith angles
	int16	SinAas;
	int16	CosAas;
	int16	SinAzs;
	int16	CosAzs;

	// Clipped Zenith angle
	int16	SinAZS;
	int16	CosAZS;
	int16	SecAZS_C1;
	int16	SecAZS_E1;
	int16	SecAZS_C2;
	int16	SecAZS_E2;

	int16	Nx;
	int16	Ny;
	int16	Nz;
	int16	Gx;
	int16	Gy;
	int16	Gz;
	int16	C_Les;
	int16	E_Les;
	int16	G_Les;

	int16	matrixA[3][3];
	int16	matrixB[3][3];
	int16	matrixC[3][3];

	int16	Op00Multiplicand;
	int16	Op00Multiplier;
	int16	Op00Result;

	int16	Op20Multiplicand;
	int16	Op20Multiplier;
	int16	Op20Result;

	int16	Op10Coefficient;
	int16	Op10Exponent;
	int16	Op10CoefficientR;
	int16	Op10ExponentR;

	int16	Op04Angle;
	int16	Op04Radius;
	int16	Op04Sin;
	int16	Op04Cos;

	int16	Op0CA;
	int16	Op0CX1;
	int16	Op0CY1;
	int16	Op0CX2;
	int16	Op0CY2;

	int16	Op02FX;
	int16	Op02FY;
	int16	Op02FZ;
	int16	Op02LFE;
	int16	Op02LES;
	int16	Op02AAS;
	int16	Op02AZS;
	int16	Op02VOF;
	int16	Op02VVA;
	int16	Op02CX;
	int16	Op02CY;

	int16	Op0AVS;
	int16	Op0AA;
	int16	Op0AB;
	int16	Op0AC;
	int16	Op0AD;

	int16	Op06X;
	int16	Op06Y;
	int16	Op06Z;
	int16	Op06H;
	int16	Op06V;
	int16	Op06M;

	int16	Op01m;
	int16	Op01Zr;
	int16	Op01Xr;
	int16	Op01Yr;

	int16	Op11m;
	int16	Op11Zr;
	int16	Op11Xr;
	int16	Op11Yr;

	int16	Op21m;
	int16	Op21Zr;
	int16	Op21Xr;
	int16	Op21Yr;

	int16	Op0DX;
	int16	Op0DY;
	int16	Op0DZ;
	int16	Op0DF;
	int16	Op0DL;
	int16	Op0DU;

	int16	Op1DX;
	int16	Op1DY;
	int16	Op1DZ;
	int16	Op1DF;
	int16	Op1DL;
	int16	Op1DU;

	int16	Op2DX;
	int16	Op2DY;
	int16	Op2DZ;
	int16	Op2DF;
	int16	Op2DL;
	int16	Op2DU;

	int16	Op03F;
	int16	Op03L;
	int16	Op03U;
	int16	Op03X;
	int16	Op03Y;
	int16	Op03Z;

	int16	Op13F;
	int16	Op13L;
	int16	Op13U;
	int16	Op13X;
	int16	Op13Y;
	int16	Op13Z;

	int16	Op23F;
	int16	Op23L;
	int16	Op23U;
	int16	Op23X;
	int16	Op23Y;
	int16	Op23Z;

	int16	Op14Zr;
	int16	Op14Xr;
	int16	Op14Yr;
	int16	Op14U;
	int16	Op14F;
	int16	Op14L;
	int16	Op14Zrr;
	int16	Op14Xrr;
	int16	Op14Yrr;

	int16	Op0EH;
	int16	Op0EV;
	int16	Op0EX;
	int16	Op0EY;

	int16	Op0BX;
	int16	Op0BY;
	int16	Op0BZ;
	int16	Op0BS;

	int16	Op1BX;
	int16	Op1BY;
	int16	Op1BZ;
	int16	Op1BS;

	int16	Op2BX;
	int16	Op2BY;
	int16	Op2BZ;
	int16	Op2BS;

	int16	Op28X;
	int16	Op28Y;
	int16	Op28Z;
	int16	Op28R;

	int16	Op1CX;
	int16	Op1CY;
	int16	Op1CZ;
	int16	Op1CXBR;
	int16	Op1CYBR;
	int16	Op1CZBR;
	int16	Op1CXAR;
	int16	Op1CYAR;
	int16	Op1CZAR;
	int16	Op1CX1;
	int16	Op1CY1;
	int16	Op1CZ1;
	int16	Op1CX2;
	int16	Op1CY2;
	int16	Op1CZ2;

	uint16	Op0FRamsize;
	uint16	Op0FPass;

	int16	Op2FUnknown;
	int16	Op2FSize;

	int16	Op08X;
	int16	Op08Y;
	int16	Op08Z;
	int16	Op08Ll;
	int16	Op08Lh;

	int16	Op18X;
	int16	Op18Y;
	int16	Op18Z;
	int16	Op18R;
	int16	Op18D;

	int16	Op38X;
	int16	Op38Y;
	int16	Op38Z;
	int16	Op38R;
	int16	Op38D;
};

struct SDSP2
{
	bool8	waiting4command;
	uint8	command;
	uint32	in_count;
	uint32	in_index;
	uint32	out_count;
	uint32	out_index;
	uint8	parameters[512];
	uint8	output[512];

	bool8	Op05HasLen;
	int32	Op05Len;
	uint8	Op05Transparent;

	bool8	Op06HasLen;
	int32	Op06Len;

	uint16	Op09Word1;
	uint16	Op09Word2;

	bool8	Op0DHasLen;
	int32	Op0DOutLen;
	int32	Op0DInLen;
};

extern struct SDSP0	DSP0;
extern struct SDSP1	DSP1;
extern struct SDSP2	DSP2;

uint8 S9xGetDSP (uint16);
void S9xSetDSP (uint8, uint16);
void S9xResetDSP (void);
uint8 DSP1GetByte (uint16);
void DSP1SetByte (uint8, uint16);
uint8 DSP2GetByte (uint16);
void DSP2SetByte (uint8, uint16);

extern uint8 (*GetDSP) (uint16);
extern void (*SetDSP) (uint8, uint16);

#endif
