/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <assert.h>
#include "snes9x.h"
#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"
#include "snapshot.h"
#include "language.h"
#include "controls.h"
#include "gfx.h"

#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

typedef struct __attribute__ ((packed))
{
	int			offset:13;
	int			offset2:1; // unused in retro-go, 13 otherwise
	int			size:13;
	int			type:5;
	uint8		debuted_in;
	uint8		deleted_in;
	const char	*name;
}	FreezeData;

enum
{
	INT_V,
	uint8_ARRAY_V,
	uint16_ARRAY_V,
	uint32_ARRAY_V,
	uint8_INDIR_ARRAY_V,
	uint16_INDIR_ARRAY_V,
	uint32_INDIR_ARRAY_V,
	POINTER_V
};

#define COUNT(ARRAY)				(sizeof(ARRAY) / sizeof(ARRAY[0]))
#define Offset(field, structure)	((int) (((char *) (&(((structure) NULL)->field))) - ((char *) NULL)))
#define OFFSET(f)					Offset(f, STRUCT *)
#define DUMMY(f)					Offset(f, struct Obsolete *)
#define DELETED(f)					(-1)

#define INT_ENTRY(save_version_introduced, field) \
{ \
	OFFSET(field), \
	0, \
	sizeof(((STRUCT *) NULL)->field), \
	INT_V, \
	save_version_introduced, \
	99, \
	#field \
}

#define ARRAY_ENTRY(save_version_introduced, field, count, elemType) \
{ \
	OFFSET(field), \
	0, \
	count, \
	elemType, \
	save_version_introduced, \
	99, \
	#field \
}

#define POINTER_ENTRY(save_version_introduced, field, relativeToField) \
{ \
	OFFSET(field), \
	OFFSET(relativeToField), \
	4, \
	POINTER_V, \
	save_version_introduced, \
	99, \
	#field \
}

#define DELETED_INT_ENTRY(save_version_introduced, save_version_removed, field, size) \
{ \
	DELETED(field), \
	0, \
	size, \
	INT_V, \
	save_version_introduced, \
	save_version_removed, \
	#field \
}

#define DELETED_ARRAY_ENTRY(save_version_introduced, save_version_removed, field, count, elemType) \
{ \
	DELETED(field), \
	0, \
	count, \
	elemType, \
	save_version_introduced, \
	save_version_removed, \
	#field \
}

#define DELETED_POINTER_ENTRY(save_version_introduced, save_version_removed, field, relativeToField) \
{ \
	DELETED(field), \
	DELETED(relativeToField), \
	4, \
	POINTER_V, \
	save_version_introduced, \
	save_version_removed, \
	#field \
}

struct SDMASnapshot
{
	struct SDMA	dma[8];
};

static struct Obsolete
{
	uint8	CPU_IRQActive;
}	Obsolete;

#define STRUCT	struct SCPUState

static const FreezeData	SnapCPU[] =
{
	INT_ENTRY(6, Cycles),
	INT_ENTRY(6, V_Counter),
	INT_ENTRY(6, Flags),
	INT_ENTRY(6, IRQPending),
	INT_ENTRY(6, MemSpeed),
	INT_ENTRY(6, MemSpeedx2),
	INT_ENTRY(6, FastROMSpeed),
	INT_ENTRY(6, InDMA),
	INT_ENTRY(6, InHDMA),
	INT_ENTRY(6, InDMAorHDMA),
	INT_ENTRY(6, InWRAMDMAorHDMA),
	INT_ENTRY(6, HDMARanInDMA),
	INT_ENTRY(6, WhichEvent),
	INT_ENTRY(6, NextEvent),
	INT_ENTRY(6, WaitingForInterrupt),
	INT_ENTRY(7, NMIPending),
	INT_ENTRY(7, IRQLine),
	INT_ENTRY(7, IRQLastState),
	INT_ENTRY(6, NMITriggerPos),
	INT_ENTRY(6, IRQFlagChanging),
	INT_ENTRY(6, NextIRQTimer)
};

#undef STRUCT
#define STRUCT	struct SRegisters

static const FreezeData	SnapRegisters[] =
{
	INT_ENTRY(6, PB),
	INT_ENTRY(6, DB),
	INT_ENTRY(6, P.W),
	INT_ENTRY(6, A.W),
	INT_ENTRY(6, D.W),
	INT_ENTRY(6, S.W),
	INT_ENTRY(6, X.W),
	INT_ENTRY(6, Y.W),
	INT_ENTRY(6, PCw)
};

#undef STRUCT
#define STRUCT	struct SPPU

static const FreezeData	SnapPPU[] =
{
	INT_ENTRY(6, VMA.High),
	INT_ENTRY(6, VMA.Increment),
	INT_ENTRY(6, VMA.Address),
	INT_ENTRY(6, VMA.Mask1),
	INT_ENTRY(6, VMA.FullGraphicCount),
	INT_ENTRY(6, VMA.Shift),
	INT_ENTRY(6, WRAM),
#define O(N) \
	INT_ENTRY(6, BG[N].SCBase), \
	INT_ENTRY(6, BG[N].HOffset), \
	INT_ENTRY(6, BG[N].VOffset), \
	INT_ENTRY(6, BG[N].BGSize), \
	INT_ENTRY(6, BG[N].NameBase), \
	INT_ENTRY(6, BG[N].SCSize)
	O(0), O(1), O(2), O(3),
#undef O
	INT_ENTRY(6, BGMode),
	INT_ENTRY(6, BG3Priority),
	INT_ENTRY(6, CGFLIP),
	INT_ENTRY(6, CGFLIPRead),
	INT_ENTRY(6, CGADD),
	INT_ENTRY(11, CGSavedByte),
	ARRAY_ENTRY(6, CGDATA, 256, uint16_ARRAY_V),
#define O(N) \
	INT_ENTRY(6, OBJ[N].HPos), \
	INT_ENTRY(6, OBJ[N].VPos), \
	INT_ENTRY(6, OBJ[N].HFlip), \
	INT_ENTRY(6, OBJ[N].VFlip), \
	INT_ENTRY(6, OBJ[N].Name), \
	INT_ENTRY(6, OBJ[N].Priority), \
	INT_ENTRY(6, OBJ[N].Palette), \
	INT_ENTRY(6, OBJ[N].Size)
	O(  0), O(  1), O(  2), O(  3), O(  4), O(  5), O(  6), O(  7),
	O(  8), O(  9), O( 10), O( 11), O( 12), O( 13), O( 14), O( 15),
	O( 16), O( 17), O( 18), O( 19), O( 20), O( 21), O( 22), O( 23),
	O( 24), O( 25), O( 26), O( 27), O( 28), O( 29), O( 30), O( 31),
	O( 32), O( 33), O( 34), O( 35), O( 36), O( 37), O( 38), O( 39),
	O( 40), O( 41), O( 42), O( 43), O( 44), O( 45), O( 46), O( 47),
	O( 48), O( 49), O( 50), O( 51), O( 52), O( 53), O( 54), O( 55),
	O( 56), O( 57), O( 58), O( 59), O( 60), O( 61), O( 62), O( 63),
	O( 64), O( 65), O( 66), O( 67), O( 68), O( 69), O( 70), O( 71),
	O( 72), O( 73), O( 74), O( 75), O( 76), O( 77), O( 78), O( 79),
	O( 80), O( 81), O( 82), O( 83), O( 84), O( 85), O( 86), O( 87),
	O( 88), O( 89), O( 90), O( 91), O( 92), O( 93), O( 94), O( 95),
	O( 96), O( 97), O( 98), O( 99), O(100), O(101), O(102), O(103),
	O(104), O(105), O(106), O(107), O(108), O(109), O(110), O(111),
	O(112), O(113), O(114), O(115), O(116), O(117), O(118), O(119),
	O(120), O(121), O(122), O(123), O(124), O(125), O(126), O(127),
#undef O
	INT_ENTRY(6, OBJNameBase),
	INT_ENTRY(6, OBJNameSelect),
	INT_ENTRY(6, OBJSizeSelect),
	INT_ENTRY(6, OAMAddr),
	INT_ENTRY(6, SavedOAMAddr),
	INT_ENTRY(6, OAMPriorityRotation),
	INT_ENTRY(6, OAMFlip),
	INT_ENTRY(6, OAMReadFlip),
	INT_ENTRY(6, OAMTileAddress),
	INT_ENTRY(6, OAMWriteRegister),
	ARRAY_ENTRY(6, OAMData, 512 + 32, uint8_ARRAY_V),
	INT_ENTRY(6, FirstSprite),
	INT_ENTRY(6, LastSprite),
	INT_ENTRY(6, HTimerEnabled),
	INT_ENTRY(6, VTimerEnabled),
	INT_ENTRY(6, HTimerPosition),
	INT_ENTRY(6, VTimerPosition),
	INT_ENTRY(6, IRQHBeamPos),
	INT_ENTRY(6, IRQVBeamPos),
	INT_ENTRY(6, HBeamFlip),
	INT_ENTRY(6, VBeamFlip),
	INT_ENTRY(6, HBeamPosLatched),
	INT_ENTRY(6, VBeamPosLatched),
	INT_ENTRY(6, HVBeamCounterLatched),
	INT_ENTRY(6, Mode7HFlip),
	INT_ENTRY(6, Mode7VFlip),
	INT_ENTRY(6, Mode7Repeat),
	INT_ENTRY(6, MatrixA),
	INT_ENTRY(6, MatrixB),
	INT_ENTRY(6, MatrixC),
	INT_ENTRY(6, MatrixD),
	INT_ENTRY(6, CentreX),
	INT_ENTRY(6, CentreY),
	INT_ENTRY(6, M7HOFS),
	INT_ENTRY(6, M7VOFS),
	INT_ENTRY(6, Mosaic),
	INT_ENTRY(6, MosaicStart),
	ARRAY_ENTRY(6, BGMosaic, 4, uint8_ARRAY_V),
	INT_ENTRY(6, Window1Left),
	INT_ENTRY(6, Window1Right),
	INT_ENTRY(6, Window2Left),
	INT_ENTRY(6, Window2Right),
	INT_ENTRY(6, RecomputeClipWindows),
#define O(N) \
	INT_ENTRY(6, ClipCounts[N]), \
	INT_ENTRY(6, ClipWindowOverlapLogic[N]), \
	INT_ENTRY(6, ClipWindow1Enable[N]), \
	INT_ENTRY(6, ClipWindow2Enable[N]), \
	INT_ENTRY(6, ClipWindow1Inside[N]), \
	INT_ENTRY(6, ClipWindow2Inside[N])
	O(0), O(1), O(2), O(3), O(4), O(5),
#undef O
	INT_ENTRY(6, ForcedBlanking),
	INT_ENTRY(6, FixedColourRed),
	INT_ENTRY(6, FixedColourGreen),
	INT_ENTRY(6, FixedColourBlue),
	INT_ENTRY(6, Brightness),
	INT_ENTRY(6, ScreenHeight),
	INT_ENTRY(6, Need16x8Mulitply),
	INT_ENTRY(6, BGnxOFSbyte),
	INT_ENTRY(6, M7byte),
	INT_ENTRY(6, HDMA),
	INT_ENTRY(6, HDMAEnded),
	INT_ENTRY(6, OpenBus1),
	INT_ENTRY(6, OpenBus2),
	INT_ENTRY(11, VRAMReadBuffer)
};

#undef STRUCT
#define STRUCT	struct SDMASnapshot

static const FreezeData	SnapDMA[] =
{
#define O(N) \
	INT_ENTRY(6, dma[N].ReverseTransfer), \
	INT_ENTRY(6, dma[N].HDMAIndirectAddressing), \
	INT_ENTRY(6, dma[N].UnusedBit43x0), \
	INT_ENTRY(6, dma[N].AAddressFixed), \
	INT_ENTRY(6, dma[N].AAddressDecrement), \
	INT_ENTRY(6, dma[N].TransferMode), \
	INT_ENTRY(6, dma[N].BAddress), \
	INT_ENTRY(6, dma[N].AAddress), \
	INT_ENTRY(6, dma[N].ABank), \
	INT_ENTRY(6, dma[N].DMACount_Or_HDMAIndirectAddress), \
	INT_ENTRY(6, dma[N].IndirectBank), \
	INT_ENTRY(6, dma[N].Address), \
	INT_ENTRY(6, dma[N].Repeat), \
	INT_ENTRY(6, dma[N].LineCount), \
	INT_ENTRY(6, dma[N].UnknownByte), \
	INT_ENTRY(6, dma[N].DoTransfer)
	O(0), O(1), O(2), O(3), O(4), O(5), O(6), O(7)
#undef O
};

#undef STRUCT
#define STRUCT	struct SDSP1

static const FreezeData	SnapDSP1[] =
{
	INT_ENTRY(6, waiting4command),
	INT_ENTRY(6, first_parameter),
	INT_ENTRY(6, command),
	INT_ENTRY(6, in_count),
	INT_ENTRY(6, in_index),
	INT_ENTRY(6, out_count),
	INT_ENTRY(6, out_index),
	ARRAY_ENTRY(6, parameters, 512, uint8_ARRAY_V),
	ARRAY_ENTRY(6, output, 512, uint8_ARRAY_V),
	INT_ENTRY(6, CentreX),
	INT_ENTRY(6, CentreY),
	INT_ENTRY(6, VOffset),
	INT_ENTRY(6, VPlane_C),
	INT_ENTRY(6, VPlane_E),
	INT_ENTRY(6, SinAas),
	INT_ENTRY(6, CosAas),
	INT_ENTRY(6, SinAzs),
	INT_ENTRY(6, CosAzs),
	INT_ENTRY(6, SinAZS),
	INT_ENTRY(6, CosAZS),
	INT_ENTRY(6, SecAZS_C1),
	INT_ENTRY(6, SecAZS_E1),
	INT_ENTRY(6, SecAZS_C2),
	INT_ENTRY(6, SecAZS_E2),
	INT_ENTRY(6, Nx),
	INT_ENTRY(6, Ny),
	INT_ENTRY(6, Nz),
	INT_ENTRY(6, Gx),
	INT_ENTRY(6, Gy),
	INT_ENTRY(6, Gz),
	INT_ENTRY(6, C_Les),
	INT_ENTRY(6, E_Les),
	INT_ENTRY(6, G_Les),
#define O(N) \
	ARRAY_ENTRY(6, matrixA[N], 3, uint16_ARRAY_V), \
	ARRAY_ENTRY(6, matrixB[N], 3, uint16_ARRAY_V), \
	ARRAY_ENTRY(6, matrixC[N], 3, uint16_ARRAY_V)
	O(0), O(1), O(2),
#undef O
	INT_ENTRY(6, Op00Multiplicand),
	INT_ENTRY(6, Op00Multiplier),
	INT_ENTRY(6, Op00Result),
	INT_ENTRY(6, Op20Multiplicand),
	INT_ENTRY(6, Op20Multiplier),
	INT_ENTRY(6, Op20Result),
	INT_ENTRY(6, Op10Coefficient),
	INT_ENTRY(6, Op10Exponent),
	INT_ENTRY(6, Op10CoefficientR),
	INT_ENTRY(6, Op10ExponentR),
	INT_ENTRY(6, Op04Angle),
	INT_ENTRY(6, Op04Radius),
	INT_ENTRY(6, Op04Sin),
	INT_ENTRY(6, Op04Cos),
	INT_ENTRY(6, Op0CA),
	INT_ENTRY(6, Op0CX1),
	INT_ENTRY(6, Op0CY1),
	INT_ENTRY(6, Op0CX2),
	INT_ENTRY(6, Op0CY2),
	INT_ENTRY(6, Op02FX),
	INT_ENTRY(6, Op02FY),
	INT_ENTRY(6, Op02FZ),
	INT_ENTRY(6, Op02LFE),
	INT_ENTRY(6, Op02LES),
	INT_ENTRY(6, Op02AAS),
	INT_ENTRY(6, Op02AZS),
	INT_ENTRY(6, Op02VOF),
	INT_ENTRY(6, Op02VVA),
	INT_ENTRY(6, Op02CX),
	INT_ENTRY(6, Op02CY),
	INT_ENTRY(6, Op0AVS),
	INT_ENTRY(6, Op0AA),
	INT_ENTRY(6, Op0AB),
	INT_ENTRY(6, Op0AC),
	INT_ENTRY(6, Op0AD),
	INT_ENTRY(6, Op06X),
	INT_ENTRY(6, Op06Y),
	INT_ENTRY(6, Op06Z),
	INT_ENTRY(6, Op06H),
	INT_ENTRY(6, Op06V),
	INT_ENTRY(6, Op06M),
	INT_ENTRY(6, Op01m),
	INT_ENTRY(6, Op01Zr),
	INT_ENTRY(6, Op01Xr),
	INT_ENTRY(6, Op01Yr),
	INT_ENTRY(6, Op11m),
	INT_ENTRY(6, Op11Zr),
	INT_ENTRY(6, Op11Xr),
	INT_ENTRY(6, Op11Yr),
	INT_ENTRY(6, Op21m),
	INT_ENTRY(6, Op21Zr),
	INT_ENTRY(6, Op21Xr),
	INT_ENTRY(6, Op21Yr),
	INT_ENTRY(6, Op0DX),
	INT_ENTRY(6, Op0DY),
	INT_ENTRY(6, Op0DZ),
	INT_ENTRY(6, Op0DF),
	INT_ENTRY(6, Op0DL),
	INT_ENTRY(6, Op0DU),
	INT_ENTRY(6, Op1DX),
	INT_ENTRY(6, Op1DY),
	INT_ENTRY(6, Op1DZ),
	INT_ENTRY(6, Op1DF),
	INT_ENTRY(6, Op1DL),
	INT_ENTRY(6, Op1DU),
	INT_ENTRY(6, Op2DX),
	INT_ENTRY(6, Op2DY),
	INT_ENTRY(6, Op2DZ),
	INT_ENTRY(6, Op2DF),
	INT_ENTRY(6, Op2DL),
	INT_ENTRY(6, Op2DU),
	INT_ENTRY(6, Op03F),
	INT_ENTRY(6, Op03L),
	INT_ENTRY(6, Op03U),
	INT_ENTRY(6, Op03X),
	INT_ENTRY(6, Op03Y),
	INT_ENTRY(6, Op03Z),
	INT_ENTRY(6, Op13F),
	INT_ENTRY(6, Op13L),
	INT_ENTRY(6, Op13U),
	INT_ENTRY(6, Op13X),
	INT_ENTRY(6, Op13Y),
	INT_ENTRY(6, Op13Z),
	INT_ENTRY(6, Op23F),
	INT_ENTRY(6, Op23L),
	INT_ENTRY(6, Op23U),
	INT_ENTRY(6, Op23X),
	INT_ENTRY(6, Op23Y),
	INT_ENTRY(6, Op23Z),
	INT_ENTRY(6, Op14Zr),
	INT_ENTRY(6, Op14Xr),
	INT_ENTRY(6, Op14Yr),
	INT_ENTRY(6, Op14U),
	INT_ENTRY(6, Op14F),
	INT_ENTRY(6, Op14L),
	INT_ENTRY(6, Op14Zrr),
	INT_ENTRY(6, Op14Xrr),
	INT_ENTRY(6, Op14Yrr),
	INT_ENTRY(6, Op0EH),
	INT_ENTRY(6, Op0EV),
	INT_ENTRY(6, Op0EX),
	INT_ENTRY(6, Op0EY),
	INT_ENTRY(6, Op0BX),
	INT_ENTRY(6, Op0BY),
	INT_ENTRY(6, Op0BZ),
	INT_ENTRY(6, Op0BS),
	INT_ENTRY(6, Op1BX),
	INT_ENTRY(6, Op1BY),
	INT_ENTRY(6, Op1BZ),
	INT_ENTRY(6, Op1BS),
	INT_ENTRY(6, Op2BX),
	INT_ENTRY(6, Op2BY),
	INT_ENTRY(6, Op2BZ),
	INT_ENTRY(6, Op2BS),
	INT_ENTRY(6, Op28X),
	INT_ENTRY(6, Op28Y),
	INT_ENTRY(6, Op28Z),
	INT_ENTRY(6, Op28R),
	INT_ENTRY(6, Op1CX),
	INT_ENTRY(6, Op1CY),
	INT_ENTRY(6, Op1CZ),
	INT_ENTRY(6, Op1CXBR),
	INT_ENTRY(6, Op1CYBR),
	INT_ENTRY(6, Op1CZBR),
	INT_ENTRY(6, Op1CXAR),
	INT_ENTRY(6, Op1CYAR),
	INT_ENTRY(6, Op1CZAR),
	INT_ENTRY(6, Op1CX1),
	INT_ENTRY(6, Op1CY1),
	INT_ENTRY(6, Op1CZ1),
	INT_ENTRY(6, Op1CX2),
	INT_ENTRY(6, Op1CY2),
	INT_ENTRY(6, Op1CZ2),
	INT_ENTRY(6, Op0FRamsize),
	INT_ENTRY(6, Op0FPass),
	INT_ENTRY(6, Op2FUnknown),
	INT_ENTRY(6, Op2FSize),
	INT_ENTRY(6, Op08X),
	INT_ENTRY(6, Op08Y),
	INT_ENTRY(6, Op08Z),
	INT_ENTRY(6, Op08Ll),
	INT_ENTRY(6, Op08Lh),
	INT_ENTRY(6, Op18X),
	INT_ENTRY(6, Op18Y),
	INT_ENTRY(6, Op18Z),
	INT_ENTRY(6, Op18R),
	INT_ENTRY(6, Op18D),
	INT_ENTRY(6, Op38X),
	INT_ENTRY(6, Op38Y),
	INT_ENTRY(6, Op38Z),
	INT_ENTRY(6, Op38R),
	INT_ENTRY(6, Op38D)
};

#undef STRUCT
#define STRUCT	struct SDSP2

static const FreezeData	SnapDSP2[] =
{
	INT_ENTRY(6, waiting4command),
	INT_ENTRY(6, command),
	INT_ENTRY(6, in_count),
	INT_ENTRY(6, in_index),
	INT_ENTRY(6, out_count),
	INT_ENTRY(6, out_index),
	ARRAY_ENTRY(6, parameters, 512, uint8_ARRAY_V),
	ARRAY_ENTRY(6, output, 512, uint8_ARRAY_V),
	INT_ENTRY(6, Op05HasLen),
	INT_ENTRY(6, Op05Len),
	INT_ENTRY(6, Op05Transparent),
	INT_ENTRY(6, Op06HasLen),
	INT_ENTRY(6, Op06Len),
	INT_ENTRY(6, Op09Word1),
	INT_ENTRY(6, Op09Word2),
	INT_ENTRY(6, Op0DHasLen),
	INT_ENTRY(6, Op0DOutLen),
	INT_ENTRY(6, Op0DInLen)
};

#undef STRUCT

static int UnfreezeBlock (FILE *, const char *, uint8 *, int);
static int UnfreezeBlockCopy (FILE *, const char *, uint8 **, int);
static int UnfreezeStruct (FILE *, const char *, void *, const FreezeData *, int, int);
static int UnfreezeStructCopy (FILE *, const char *, uint8 **, const FreezeData *, int, int);
static void UnfreezeStructFromCopy (void *, const FreezeData *, int, uint8 *, int);
static void FreezeBlock (FILE *, const char *, uint8 *, int);
static void FreezeStruct (FILE *, const char *, void *, const FreezeData *, int);
static bool CheckBlockName(FILE *, const char *, int *);
static void SkipBlockWithName(FILE *, const char *);

// QuickSave

bool8 S9xFreezeGame (const char *filename)
{
	FILE *stream = fopen(filename, "wb");

	if (!stream)
	{
		return (FILE_NOT_FOUND);
	}

	// We don't have enough RAM to fit the sound snapshot at the moment
	// So we're naughty and used the tile cache data which is about 256K.
	uint8 *soundsnapshot = (uint8 *)IPPU.TileCacheData; // new uint8[SPC_SAVE_STATE_BLOCK_SIZE];
	memset(IPPU.TileCache, 0, sizeof(IPPU.TileCache));

	sprintf(String, "%s:%04d\n", SNAPSHOT_MAGIC, SNAPSHOT_VERSION);
	fwrite(String, strlen(String), 1, stream);

	sprintf(String, "NAM:%06d:%s%c", (int) strlen(Memory.ROMName) + 1, Memory.ROMName, 0);
	fwrite(String, strlen(String) + 1, 1,stream);

	FreezeStruct(stream, "CPU", &CPU, SnapCPU, COUNT(SnapCPU));

	FreezeStruct(stream, "REG", &Registers, SnapRegisters, COUNT(SnapRegisters));

	FreezeStruct(stream, "PPU", &PPU, SnapPPU, COUNT(SnapPPU));

	struct SDMASnapshot	dma_snap;
	for (int d = 0; d < 8; d++)
		dma_snap.dma[d] = DMA[d];
	FreezeStruct(stream, "DMA", &dma_snap, SnapDMA, COUNT(SnapDMA));

	FreezeBlock (stream, "VRA", Memory.VRAM, 0x10000);

	FreezeBlock (stream, "RAM", Memory.RAM, 0x20000);

	if (Memory.SRAMSize > 0)
		FreezeBlock(stream, "SRA", Memory.SRAM, Memory.SRAMBytes);

	FreezeBlock (stream, "FI1", Memory.CPU_IO, 0x400);
	FreezeBlock (stream, "FI2", Memory.PPU_IO, 0x200);

	S9xAPUSaveState(soundsnapshot);
	FreezeBlock (stream, "SND", soundsnapshot, SPC_SAVE_STATE_BLOCK_SIZE);

	if (Settings.DSP == 1)
		FreezeStruct(stream, "DP1", &DSP1, SnapDSP1, COUNT(SnapDSP1));

	if (Settings.DSP == 2)
		FreezeStruct(stream, "DP2", &DSP2, SnapDSP2, COUNT(SnapDSP2));

	fclose(stream);

	sprintf(String, SAVE_INFO_SNAPSHOT " %s", S9xBasename(filename));
	S9xMessage(S9X_INFO, S9X_FREEZE_FILE_INFO, String);

	return (SUCCESS);
}

// QuickLoad

bool8 S9xUnfreezeGame (const char *filename)
{
	FILE *stream = fopen(filename, "rb");

	if (!stream)
	{
		sprintf(String, SAVE_ERR_SAVE_NOT_FOUND, S9xBasename(filename));
		S9xMessage(S9X_INFO, S9X_FREEZE_FILE_INFO, String);
		return (FILE_NOT_FOUND);
	}

	int		result = SUCCESS;
	int		version, len;
	char	buffer[PATH_MAX + 1];

	len = strlen(SNAPSHOT_MAGIC) + 1 + 4 + 1;
	if (fread(buffer, 1, len, stream) != (unsigned int ) len)
		return (WRONG_FORMAT);

	if (strncmp(buffer, SNAPSHOT_MAGIC, strlen(SNAPSHOT_MAGIC)) != 0)
		return (WRONG_FORMAT);

	version = atoi(&buffer[strlen(SNAPSHOT_MAGIC) + 1]);
	if (version > SNAPSHOT_VERSION)
		return (WRONG_VERSION);

	result = UnfreezeBlock(stream, "NAM", (uint8 *) buffer, PATH_MAX);
	if (result != SUCCESS)
		return (result);

	uint32 old_flags     = CPU.Flags;
	struct SDMASnapshot	dma_snap;

	// We don't have enough RAM to fit the sound snapshot at the moment
	// So we're naughty and used the tile cache data which is about 256K.
	uint8 *soundsnapshot = (uint8 *)IPPU.TileCacheData;
	memset(IPPU.TileCache, 0, sizeof(IPPU.TileCache));

	do
	{
		result = UnfreezeStruct(stream, "CPU", &CPU, SnapCPU, COUNT(SnapCPU), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeStruct(stream, "REG", &Registers, SnapRegisters, COUNT(SnapRegisters), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeStruct(stream, "PPU", &PPU, SnapPPU, COUNT(SnapPPU), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeStruct(stream, "DMA", &dma_snap, SnapDMA, COUNT(SnapDMA), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeBlock(stream, "VRA", Memory.VRAM, 0x10000);
		if (result != SUCCESS)
			break;

		result = UnfreezeBlock(stream, "RAM", Memory.RAM, 0x20000);
		if (result != SUCCESS)
			break;

		result = UnfreezeBlock(stream, "SRA", Memory.SRAM, Memory.SRAMBytes);
		if (result != SUCCESS && Memory.SRAMSize > 0)
			break;

		result = UnfreezeBlock(stream, "FI1", Memory.CPU_IO, 0x400);
		if (result != SUCCESS)
			break;

		result = UnfreezeBlock(stream, "FI2", Memory.PPU_IO, 0x200);
		if (result != SUCCESS)
			break;

		result = UnfreezeBlock (stream, "SND", soundsnapshot, SPC_SAVE_STATE_BLOCK_SIZE);
		if (result != SUCCESS)
			break;

		result = UnfreezeStruct(stream, "DP1", &DSP1, SnapDSP1, COUNT(SnapDSP1), version);
		if (result != SUCCESS && Settings.DSP == 1)
			break;

		result = UnfreezeStruct(stream, "DP2", &DSP2, SnapDSP2, COUNT(SnapDSP2), version);
		if (result != SUCCESS && Settings.DSP == 2)
			break;

		result = SUCCESS;
	} while (false);

	if (result == SUCCESS)
	{
		S9xResetPPUFast();
		//Do not call this if you have written directly to "Memory." arrays
		// S9xReset();

		S9xAPULoadState(soundsnapshot);

		CPU.Flags |= old_flags & (DEBUG_MODE_FLAG | TRACE_FLAG | SINGLE_STEP_FLAG | FRAME_ADVANCE_FLAG);
		ICPU.ShiftedPB = Registers.PB << 16;
		ICPU.ShiftedDB = Registers.DB << 16;
		ICPU.S9xOpcodes = S9xOpcodesSlow;
		S9xSetPCBase(Registers.PBPC);

		for (int d = 0; d < 8; d++)
			DMA[d] = dma_snap.dma[d];
		// TODO: these should already be correct since they are stored in the snapshot
		CPU.InDMA = CPU.InHDMA = FALSE;
		CPU.InDMAorHDMA = CPU.InWRAMDMAorHDMA = FALSE;
		CPU.HDMARanInDMA = 0;

		IPPU.OBJChanged = TRUE;
		IPPU.RenderThisFrame = TRUE;

		S9xGraphicsScreenResize();
	}

	fclose(stream);

	if (result != SUCCESS)
	{
		switch (result)
		{
			case WRONG_FORMAT:
				S9xMessage(S9X_ERROR, S9X_WRONG_FORMAT, SAVE_ERR_WRONG_FORMAT);
				break;

			case WRONG_VERSION:
				S9xMessage(S9X_ERROR, S9X_WRONG_VERSION, SAVE_ERR_WRONG_VERSION);
				break;

			case FILE_NOT_FOUND:
			default:
				sprintf(String, SAVE_ERR_ROM_NOT_FOUND, S9xBasename(filename));
				S9xMessage(S9X_ERROR, S9X_ROM_NOT_FOUND, String);
				break;
		}

		return (FALSE);
	}

	sprintf(String, SAVE_INFO_LOAD " %s", S9xBasename(filename));
	S9xMessage(S9X_INFO, S9X_FREEZE_FILE_INFO, String);

	return (SUCCESS);
}

static int FreezeSize (int size, int type)
{
	switch (type)
	{
		case uint32_ARRAY_V:
		case uint32_INDIR_ARRAY_V:
			return (size * 4);

		case uint16_ARRAY_V:
		case uint16_INDIR_ARRAY_V:
			return (size * 2);

		default:
			return (size);
	}
}

static void FreezeStruct (FILE *stream, const char *name, void *base, const FreezeData *fields, int num_fields)
{
	int	len = 0;
	int	i, j;

	for (i = 0; i < num_fields; i++)
	{
		if (SNAPSHOT_VERSION < fields[i].debuted_in)
		{
			fprintf(stderr, "%s[%p]: field has bad debuted_in value %d, > %d.", name, (void *) fields, fields[i].debuted_in, SNAPSHOT_VERSION);
			continue;
		}

		if (SNAPSHOT_VERSION < fields[i].deleted_in)
			len += FreezeSize(fields[i].size, fields[i].type);
	}

	uint8	*block = (uint8*)calloc(1, len);
	uint8	*ptr = block;
	uint8	*addr;
	uint16	word;
	uint32	dword;
	int64	qaword;
	int		relativeAddr;

	for (i = 0; i < num_fields; i++)
	{
		if (SNAPSHOT_VERSION >= fields[i].deleted_in || SNAPSHOT_VERSION < fields[i].debuted_in)
			continue;

		addr = (uint8 *) base + fields[i].offset;

		// determine real address of indirect-type fields
		// (where the structure contains a pointer to an array rather than the array itself)
		if (fields[i].type == uint8_INDIR_ARRAY_V || fields[i].type == uint16_INDIR_ARRAY_V || fields[i].type == uint32_INDIR_ARRAY_V)
			addr = (uint8 *) (*((pint *) addr));

		// convert pointer-type saves from absolute to relative pointers
		if (fields[i].type == POINTER_V)
		{
			uint8	*pointer    = (uint8 *) *((pint *) ((uint8 *) base + fields[i].offset));
			uint8	*relativeTo = (uint8 *) *((pint *) ((uint8 *) base + fields[i].offset2));
			relativeAddr = pointer - relativeTo;
			addr = (uint8 *) &relativeAddr;
		}

		switch (fields[i].type)
		{
			case INT_V:
			case POINTER_V:
				switch (fields[i].size)
				{
					case 1:
						*ptr++ = *(addr);
						break;

					case 2:
						word = *((uint16 *) (addr));
						*ptr++ = (uint8) (word >> 8);
						*ptr++ = (uint8) word;
						break;

					case 4:
						dword = *((uint32 *) (addr));
						*ptr++ = (uint8) (dword >> 24);
						*ptr++ = (uint8) (dword >> 16);
						*ptr++ = (uint8) (dword >> 8);
						*ptr++ = (uint8) dword;
						break;

					case 8:
						qaword = *((int64 *) (addr));
						*ptr++ = (uint8) (qaword >> 56);
						*ptr++ = (uint8) (qaword >> 48);
						*ptr++ = (uint8) (qaword >> 40);
						*ptr++ = (uint8) (qaword >> 32);
						*ptr++ = (uint8) (qaword >> 24);
						*ptr++ = (uint8) (qaword >> 16);
						*ptr++ = (uint8) (qaword >> 8);
						*ptr++ = (uint8) qaword;
						break;
				}

				break;

			case uint8_ARRAY_V:
			case uint8_INDIR_ARRAY_V:
				memmove(ptr, addr, fields[i].size);
				ptr += fields[i].size;

				break;

			case uint16_ARRAY_V:
			case uint16_INDIR_ARRAY_V:
				for (j = 0; j < fields[i].size; j++)
				{
					word = *((uint16 *) (addr + j * 2));
					*ptr++ = (uint8) (word >> 8);
					*ptr++ = (uint8) word;
				}

				break;

			case uint32_ARRAY_V:
			case uint32_INDIR_ARRAY_V:
				for (j = 0; j < fields[i].size; j++)
				{
					dword = *((uint32 *) (addr + j * 4));
					*ptr++ = (uint8) (dword >> 24);
					*ptr++ = (uint8) (dword >> 16);
					*ptr++ = (uint8) (dword >> 8);
					*ptr++ = (uint8) dword;
				}

				break;
		}
	}

	FreezeBlock(stream, name, block, len);
	free(block);
}

static void FreezeBlock (FILE *stream, const char *name, uint8 *block, int size)
{
	char	buffer[20];

	// check if it fits in 6 digits. (letting it go over and using strlen isn't safe)
	if (size <= 999999)
		sprintf(buffer, "%s:%06d:", name, size);
	else
	{
		// to make it fit, pack it in the bytes instead of as digits
		sprintf(buffer, "%s:------:", name);
		buffer[6] = (unsigned char) ((unsigned) size >> 24);
		buffer[7] = (unsigned char) ((unsigned) size >> 16);
		buffer[8] = (unsigned char) ((unsigned) size >> 8);
		buffer[9] = (unsigned char) ((unsigned) size >> 0);
	}

	buffer[11] = 0;

	fwrite(buffer, 11, 1, stream);
	fwrite(block, size, 1, stream);
}

static bool CheckBlockName(FILE *stream, const char *name, int *len)
{
	char	buffer[16] = {0};
	size_t	l = fread(buffer, 1, 11, stream);

	fseek(stream, ftell(stream) - l, 0);

	if (buffer[4] == '-')
	{
		*len = (((unsigned char)buffer[6]) << 24)
			| (((unsigned char)buffer[7]) << 16)
			| (((unsigned char)buffer[8]) << 8)
			| (((unsigned char)buffer[9]) << 0);
	}
	else
		*len = atoi(buffer + 4);

	if (l != 11 || strncmp(buffer, name, 3) != 0 || buffer[3] != ':')
	{
		return false;
	}

	return (*len > 0);
}

static void SkipBlockWithName(FILE *stream, const char *name)
{
	int len;
	bool matchesName = CheckBlockName(stream, name, &len);
	if (matchesName)
	{
		long rewind = ftell(stream);
		rewind += len + 11;
		fseek(stream, rewind, 0);
	}
}

static int UnfreezeBlock (FILE *stream, const char *name, uint8 *block, int size)
{
	char	buffer[20];
	int		len = 0, rem = 0;
	long	rewind = ftell(stream);

	size_t	l = fread(buffer, 1, 11, stream);
	buffer[l] = 0;

	if (l != 11 || strncmp(buffer, name, 3) != 0 || buffer[3] != ':')
	{
	err:
#ifdef DEBUGGER
		fprintf(stdout, "absent: %s(%d); next: '%.11s'\n", name, size, buffer);
#endif
		fseek(stream, ftell(stream) - l, 0);
		return (WRONG_FORMAT);
	}

	if (buffer[4] == '-')
	{
		len = (((unsigned char) buffer[6]) << 24)
			| (((unsigned char) buffer[7]) << 16)
			| (((unsigned char) buffer[8]) << 8)
			| (((unsigned char) buffer[9]) << 0);
	}
	else
		len = atoi(buffer + 4);

	if (len <= 0)
		goto err;

	if (len > size)
	{
		rem = len - size;
		len = size;
	}

	memset(block, 0, size);

	if (fread(block, 1, len, stream) != (unsigned int) len)
	{
		fseek(stream, rewind, 0);
		return (WRONG_FORMAT);
	}

	if (rem)
	{
		if (fseek(stream, ftell(stream) + rem, 0) != 0)
		{
			fseek(stream, rewind, 0);
			return (WRONG_FORMAT);
		}
	}

	return (SUCCESS);
}

static int UnfreezeBlockCopy (FILE *stream, const char *name, uint8 **block, int size)
{
	int	result;

	//check name first to avoid memory allocation
	int blockLength;
	if (!CheckBlockName(stream, name, &blockLength))
	{
		return 0;
	}

	*block = (uint8*)calloc(1, size);

	result = UnfreezeBlock(stream, name, *block, size);
	if (result != SUCCESS)
	{
		free(*block);
		*block = NULL;
		return (result);
	}

	return (SUCCESS);
}

static int UnfreezeStruct (FILE *stream, const char *name, void *base, const FreezeData *fields, int num_fields, int version)
{
	int		result;
	uint8	*block = NULL;

	result = UnfreezeStructCopy(stream, name, &block, fields, num_fields, version);
	if (result != SUCCESS)
	{
		free(block);
		return (result);
	}

	UnfreezeStructFromCopy(base, fields, num_fields, block, version);
	free(block);

	return (SUCCESS);
}

static int UnfreezeStructCopy (FILE *stream, const char *name, uint8 **block, const FreezeData *fields, int num_fields, int version)
{
	int	len = 0;

	for (int i = 0; i < num_fields; i++)
	{
		if (version >= fields[i].debuted_in && version < fields[i].deleted_in)
			len += FreezeSize(fields[i].size, fields[i].type);
	}

	return (UnfreezeBlockCopy(stream, name, block, len));
}

static void UnfreezeStructFromCopy (void *sbase, const FreezeData *fields, int num_fields, uint8 *block, int version)
{
	uint8	*ptr = block;
	uint16	word;
	uint32	dword;
	int64	qaword;
	uint8	*addr;
	void	*base;
	int		relativeAddr;
	int		i, j;

	for (i = 0; i < num_fields; i++)
	{
		if (version < fields[i].debuted_in || version >= fields[i].deleted_in)
			continue;

		base = (SNAPSHOT_VERSION >= fields[i].deleted_in) ? ((void *) &Obsolete) : sbase;
		addr = (uint8 *) base + fields[i].offset;

		if (fields[i].type == uint8_INDIR_ARRAY_V || fields[i].type == uint16_INDIR_ARRAY_V || fields[i].type == uint32_INDIR_ARRAY_V)
			addr = (uint8 *) (*((pint *) addr));

		switch (fields[i].type)
		{
			case INT_V:
			case POINTER_V:
				switch (fields[i].size)
				{
					case 1:
						if (fields[i].offset < 0)
						{
							ptr++;
							break;
						}

						*(addr) = *ptr++;
						break;

					case 2:
						if (fields[i].offset < 0)
						{
							ptr += 2;
							break;
						}

						word  = *ptr++ << 8;
						word |= *ptr++;
						*((uint16 *) (addr)) = word;
						break;

					case 4:
						if (fields[i].offset < 0)
						{
							ptr += 4;
							break;
						}

						dword  = *ptr++ << 24;
						dword |= *ptr++ << 16;
						dword |= *ptr++ << 8;
						dword |= *ptr++;
						*((uint32 *) (addr)) = dword;
						break;

					case 8:
						if (fields[i].offset < 0)
						{
							ptr += 8;
							break;
						}

						qaword  = (int64) *ptr++ << 56;
						qaword |= (int64) *ptr++ << 48;
						qaword |= (int64) *ptr++ << 40;
						qaword |= (int64) *ptr++ << 32;
						qaword |= (int64) *ptr++ << 24;
						qaword |= (int64) *ptr++ << 16;
						qaword |= (int64) *ptr++ << 8;
						qaword |= (int64) *ptr++;
						*((int64 *) (addr)) = qaword;
						break;

					default:
						assert(0);
						break;
				}

				break;

			case uint8_ARRAY_V:
			case uint8_INDIR_ARRAY_V:
				if (fields[i].offset >= 0)
					memmove(addr, ptr, fields[i].size);
				ptr += fields[i].size;

				break;

			case uint16_ARRAY_V:
			case uint16_INDIR_ARRAY_V:
				if (fields[i].offset < 0)
				{
					ptr += fields[i].size * 2;
					break;
				}

				for (j = 0; j < fields[i].size; j++)
				{
					word  = *ptr++ << 8;
					word |= *ptr++;
					*((uint16 *) (addr + j * 2)) = word;
				}

				break;

			case uint32_ARRAY_V:
			case uint32_INDIR_ARRAY_V:
				if (fields[i].offset < 0)
				{
					ptr += fields[i].size * 4;
					break;
				}

				for (j = 0; j < fields[i].size; j++)
				{
					dword  = *ptr++ << 24;
					dword |= *ptr++ << 16;
					dword |= *ptr++ << 8;
					dword |= *ptr++;
					*((uint32 *) (addr + j * 4)) = dword;
				}

				break;
		}

		if (fields[i].type == POINTER_V)
		{
			relativeAddr = (int) *((pint *) ((uint8 *) base + fields[i].offset));
			uint8	*relativeTo = (uint8 *) *((pint *) ((uint8 *) base + fields[i].offset2));
			*((pint *) (addr)) = (pint) (relativeTo + relativeAddr);
		}
	}
}
