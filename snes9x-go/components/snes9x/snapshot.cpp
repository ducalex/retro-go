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
#include "controls.h"
#include "display.h"
#include "language.h"
#include "gfx.h"

#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

typedef struct
{
	int			offset;
	int			offset2;
	int			size;
	int			type;
	uint16		debuted_in;
	uint16		deleted_in;
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
	9999, \
	#field \
}

#define ARRAY_ENTRY(save_version_introduced, field, count, elemType) \
{ \
	OFFSET(field), \
	0, \
	count, \
	elemType, \
	save_version_introduced, \
	9999, \
	#field \
}

#define POINTER_ENTRY(save_version_introduced, field, relativeToField) \
{ \
	OFFSET(field), \
	OFFSET(relativeToField), \
	4, \
	POINTER_V, \
	save_version_introduced, \
	9999, \
	#field \
}

#define OBSOLETE_INT_ENTRY(save_version_introduced, save_version_removed, field) \
{ \
	DUMMY(field), \
	0, \
	sizeof(((struct Obsolete *) NULL)->field), \
	INT_V, \
	save_version_introduced, \
	save_version_removed, \
	#field \
}

#define OBSOLETE_ARRAY_ENTRY(save_version_introduced, save_version_removed, field, count, elemType) \
{ \
	DUMMY(field), \
	0, \
	count, \
	elemType, \
	save_version_introduced, \
	save_version_removed, \
	#field \
}

#define OBSOLETE_POINTER_ENTRY(save_version_introduced, save_version_removed, field, relativeToField) \
{ \
	DUMMY(field), \
	DUMMY(relativeToField), \
	4, \
	POINTER_V, \
	save_version_introduced, \
	save_version_removed, \
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

struct SnapshotMovieInfo
{
	uint32	MovieInputDataSize;
};

static struct Obsolete
{
	uint8	CPU_IRQActive;
}	Obsolete;

// Disable snapshots for now
#if 0

#define STRUCT	struct SCPUState

static FreezeData	SnapCPU[] =
{
	INT_ENTRY(6, Cycles),
	INT_ENTRY(6, PrevCycles),
	INT_ENTRY(6, V_Counter),
	INT_ENTRY(6, Flags),
	OBSOLETE_INT_ENTRY(6, 7, CPU_IRQActive),
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
	DELETED_INT_ENTRY(6, 7, WaitAddress, 4),
	DELETED_INT_ENTRY(6, 7, WaitCounter, 4),
	DELETED_INT_ENTRY(6, 7, PBPCAtOpcodeStart, 4),
	INT_ENTRY(7, NMIPending),
	INT_ENTRY(7, IRQLine),
	INT_ENTRY(7, IRQTransition),
	INT_ENTRY(7, IRQLastState),
	INT_ENTRY(7, IRQExternal)
};

#undef STRUCT
#define STRUCT	struct SRegisters

static FreezeData	SnapRegisters[] =
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

static FreezeData	SnapPPU[] =
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
	INT_ENTRY(6, OBJThroughMain),
	INT_ENTRY(6, OBJThroughSub),
	INT_ENTRY(6, OBJAddition),
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
	INT_ENTRY(6, GunHLatch),
	INT_ENTRY(6, GunVLatch),
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

static FreezeData	SnapDMA[] =
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
#define STRUCT	struct SControlSnapshot

static FreezeData	SnapControls[] =
{
	INT_ENTRY(6, ver),
	ARRAY_ENTRY(6, port1_read_idx, 2, uint8_ARRAY_V),
	ARRAY_ENTRY(6, dummy1, 4, uint8_ARRAY_V),
	ARRAY_ENTRY(6, port2_read_idx, 2, uint8_ARRAY_V),
	ARRAY_ENTRY(6, dummy2, 4, uint8_ARRAY_V),
	INT_ENTRY(6, pad_read),
	INT_ENTRY(6, pad_read_last),
	ARRAY_ENTRY(6, internal, 60, uint8_ARRAY_V),
	ARRAY_ENTRY(10, internal_macs, 5, uint8_ARRAY_V)
};

#undef STRUCT
#define STRUCT	struct STimings

static FreezeData	SnapTimings[] =
{
	INT_ENTRY(6, H_Max_Master),
	INT_ENTRY(6, H_Max),
	INT_ENTRY(6, V_Max_Master),
	INT_ENTRY(6, V_Max),
	INT_ENTRY(6, HBlankStart),
	INT_ENTRY(6, HBlankEnd),
	INT_ENTRY(6, HDMAInit),
	INT_ENTRY(6, HDMAStart),
	INT_ENTRY(6, NMITriggerPos),
	INT_ENTRY(6, WRAMRefreshPos),
	INT_ENTRY(6, RenderPos),
	INT_ENTRY(6, InterlaceField),
	INT_ENTRY(6, DMACPUSync),
	INT_ENTRY(6, NMIDMADelay),
	INT_ENTRY(6, IRQFlagChanging),
	INT_ENTRY(6, APUSpeedup),
	INT_ENTRY(7, IRQTriggerCycles),
	INT_ENTRY(7, APUAllowTimeOverflow),
	INT_ENTRY(11, NextIRQTimer)
};

#undef STRUCT
#define STRUCT	struct SDSP1

static FreezeData	SnapDSP1[] =
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

static FreezeData	SnapDSP2[] =
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

#else

static struct Dummy
{
	uint32 dummy;
}	Dummy;
#define STRUCT struct Dummy
static FreezeData	SnapCPU[] = {INT_ENTRY(1, dummy)};
static FreezeData	SnapRegisters[] = {INT_ENTRY(1, dummy)};
static FreezeData	SnapPPU[] = {INT_ENTRY(1, dummy)};
static FreezeData	SnapDMA[] = {INT_ENTRY(1, dummy)};
static FreezeData	SnapControls[] = {INT_ENTRY(1, dummy)};
static FreezeData	SnapTimings[] = {INT_ENTRY(1, dummy)};
static FreezeData	SnapDSP1[] = {INT_ENTRY(1, dummy)};
static FreezeData	SnapDSP2[] = {INT_ENTRY(1, dummy)};

#endif

#undef STRUCT

static int UnfreezeBlock (STREAM, const char *, uint8 *, int);
static int UnfreezeBlockCopy (STREAM, const char *, uint8 **, int);
static int UnfreezeStruct (STREAM, const char *, void *, FreezeData *, int, int);
static int UnfreezeStructCopy (STREAM, const char *, uint8 **, FreezeData *, int, int);
static void UnfreezeStructFromCopy (void *, FreezeData *, int, uint8 *, int);
static void FreezeBlock (STREAM, const char *, uint8 *, int);
static void FreezeStruct (STREAM, const char *, void *, FreezeData *, int);
static bool CheckBlockName(STREAM stream, const char *name, int &len);
static void SkipBlockWithName(STREAM stream, const char *name);

// QuickSave

uint32 S9xFreezeSize()
{
    nulStream stream;
    S9xFreezeToStream(&stream);
    return stream.size();
}

bool8 S9xFreezeGameMem (uint8 *buf, uint32 bufSize)
{
    memStream mStream(buf, bufSize);
	S9xFreezeToStream(&mStream);

	return (TRUE);
}

bool8 S9xFreezeGame (const char *filename)
{
	STREAM	stream = NULL;

	if (S9xOpenSnapshotFile(filename, FALSE, &stream))
	{
		S9xFreezeToStream(stream);
		S9xCloseSnapshotFile(stream);

		const char *base = S9xBasename(filename);
		sprintf(String, SAVE_INFO_SNAPSHOT " %s", base);

		S9xMessage(S9X_INFO, S9X_FREEZE_FILE_INFO, String);

		return (TRUE);
	}

	return (FALSE);
}

// QuickLoad

int S9xUnfreezeGameMem (const uint8 *buf, uint32 bufSize)
{
    memStream stream(buf, bufSize);
	int result = S9xUnfreezeFromStream(&stream);

	return result;
}

bool8 S9xUnfreezeGame (const char *filename)
{
	STREAM	stream = NULL;
	char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], def[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	const char	*base = S9xBasename(filename);

	_splitpath(filename, drive, dir, def, ext);

	if (S9xOpenSnapshotFile(filename, TRUE, &stream))
	{
		int	result;

		result = S9xUnfreezeFromStream(stream);
		S9xCloseSnapshotFile(stream);

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

				case WRONG_MOVIE_SNAPSHOT:
					S9xMessage(S9X_ERROR, S9X_WRONG_MOVIE_SNAPSHOT, MOVIE_ERR_SNAPSHOT_WRONG_MOVIE);
					break;

				case NOT_A_MOVIE_SNAPSHOT:
					S9xMessage(S9X_ERROR, S9X_NOT_A_MOVIE_SNAPSHOT, MOVIE_ERR_SNAPSHOT_NOT_MOVIE);
					break;

				case SNAPSHOT_INCONSISTENT:
					S9xMessage(S9X_ERROR, S9X_SNAPSHOT_INCONSISTENT, MOVIE_ERR_SNAPSHOT_INCONSISTENT);
					break;

				case FILE_NOT_FOUND:
				default:
					sprintf(String, SAVE_ERR_ROM_NOT_FOUND, base);
					S9xMessage(S9X_ERROR, S9X_ROM_NOT_FOUND, String);
					break;
			}

			return (FALSE);
		}

		sprintf(String, SAVE_INFO_LOAD " %s", base);

		S9xMessage(S9X_INFO, S9X_FREEZE_FILE_INFO, String);

		return (TRUE);
	}

	sprintf(String, SAVE_ERR_SAVE_NOT_FOUND, base);
	S9xMessage(S9X_INFO, S9X_FREEZE_FILE_INFO, String);

	return (FALSE);
}

void S9xFreezeToStream (STREAM stream)
{
	char	buffer[8192];
	uint8	*soundsnapshot = new uint8[SPC_SAVE_STATE_BLOCK_SIZE];

	sprintf(buffer, "%s:%04d\n", SNAPSHOT_MAGIC, SNAPSHOT_VERSION);
	WRITE_STREAM(buffer, strlen(buffer), stream);

	sprintf(buffer, "NAM:%06d:%s%c", (int) strlen(Memory.ROMFilename) + 1, Memory.ROMFilename, 0);
	WRITE_STREAM(buffer, strlen(buffer) + 1, stream);

	FreezeStruct(stream, "CPU", &CPU, SnapCPU, COUNT(SnapCPU));

	FreezeStruct(stream, "REG", &Registers, SnapRegisters, COUNT(SnapRegisters));

	FreezeStruct(stream, "PPU", &PPU, SnapPPU, COUNT(SnapPPU));

	struct SDMASnapshot	dma_snap;
	for (int d = 0; d < 8; d++)
		dma_snap.dma[d] = DMA[d];
	FreezeStruct(stream, "DMA", &dma_snap, SnapDMA, COUNT(SnapDMA));

	FreezeBlock (stream, "VRA", Memory.VRAM, 0x10000);

	FreezeBlock (stream, "RAM", Memory.RAM, 0x20000);

	FreezeBlock (stream, "SRA", Memory.SRAM, 0x20000);

	FreezeBlock (stream, "FIL", Memory.FillRAM, 0x8000);

	S9xAPUSaveState(soundsnapshot);
	FreezeBlock (stream, "SND", soundsnapshot, SPC_SAVE_STATE_BLOCK_SIZE);

	struct SControlSnapshot	ctl_snap;
	S9xControlPreSaveState(&ctl_snap);
	FreezeStruct(stream, "CTL", &ctl_snap, SnapControls, COUNT(SnapControls));

	FreezeStruct(stream, "TIM", &Timings, SnapTimings, COUNT(SnapTimings));

	if (Settings.DSP == 1)
		FreezeStruct(stream, "DP1", &DSP1, SnapDSP1, COUNT(SnapDSP1));

	if (Settings.DSP == 2)
		FreezeStruct(stream, "DP2", &DSP2, SnapDSP2, COUNT(SnapDSP2));

	if (Settings.C4)
		FreezeBlock (stream, "CX4", Memory.C4RAM, 8192);

	delete [] soundsnapshot;
}

int S9xUnfreezeFromStream (STREAM stream)
{
	const bool8 fast = Settings.FastSavestates;

	int		result = SUCCESS;
	int		version, len;
	char	buffer[PATH_MAX + 1];

	len = strlen(SNAPSHOT_MAGIC) + 1 + 4 + 1;
	if (READ_STREAM(buffer, len, stream) != (unsigned int ) len)
		return (WRONG_FORMAT);

	if (strncmp(buffer, SNAPSHOT_MAGIC, strlen(SNAPSHOT_MAGIC)) != 0)
		return (WRONG_FORMAT);

	version = atoi(&buffer[strlen(SNAPSHOT_MAGIC) + 1]);
	if (version > SNAPSHOT_VERSION)
		return (WRONG_VERSION);

	result = UnfreezeBlock(stream, "NAM", (uint8 *) buffer, PATH_MAX);
	if (result != SUCCESS)
		return (result);

	uint8	*local_cpu           = NULL;
	uint8	*local_registers     = NULL;
	uint8	*local_ppu           = NULL;
	uint8	*local_dma           = NULL;
	uint8	*local_vram          = NULL;
	uint8	*local_ram           = NULL;
	uint8	*local_sram          = NULL;
	uint8	*local_fillram       = NULL;
	uint8	*local_apu_sound     = NULL;
	uint8	*local_control_data  = NULL;
	uint8	*local_timing_data   = NULL;
	uint8	*local_dsp1          = NULL;
	uint8	*local_dsp2          = NULL;
	uint8	*local_cx4_data      = NULL;

	do
	{
		result = UnfreezeStructCopy(stream, "CPU", &local_cpu, SnapCPU, COUNT(SnapCPU), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeStructCopy(stream, "REG", &local_registers, SnapRegisters, COUNT(SnapRegisters), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeStructCopy(stream, "PPU", &local_ppu, SnapPPU, COUNT(SnapPPU), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeStructCopy(stream, "DMA", &local_dma, SnapDMA, COUNT(SnapDMA), version);
		if (result != SUCCESS)
			break;

		if (fast)
			result = UnfreezeBlock(stream, "VRA", Memory.VRAM, 0x10000);
		else
			result = UnfreezeBlockCopy(stream, "VRA", &local_vram, 0x10000);
		if (result != SUCCESS)
			break;

		if (fast)
			result = UnfreezeBlock(stream, "RAM", Memory.RAM, 0x20000);
		else
			result = UnfreezeBlockCopy(stream, "RAM", &local_ram, 0x20000);
		if (result != SUCCESS)
			break;

		if (fast)
			result = UnfreezeBlock(stream, "SRA", Memory.SRAM, 0x20000);
		else
			result = UnfreezeBlockCopy (stream, "SRA", &local_sram, 0x20000);
		if (result != SUCCESS)
			break;

		if (fast)
			result = UnfreezeBlock(stream, "FIL", Memory.FillRAM, 0x8000);
		else
			result = UnfreezeBlockCopy(stream, "FIL", &local_fillram, 0x8000);
		if (result != SUCCESS)
			break;

		result = UnfreezeBlockCopy (stream, "SND", &local_apu_sound, SPC_SAVE_STATE_BLOCK_SIZE);
		if (result != SUCCESS)
			break;

		result = UnfreezeStructCopy(stream, "CTL", &local_control_data, SnapControls, COUNT(SnapControls), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeStructCopy(stream, "TIM", &local_timing_data, SnapTimings, COUNT(SnapTimings), version);
		if (result != SUCCESS)
			break;

		result = UnfreezeStructCopy(stream, "DP1", &local_dsp1, SnapDSP1, COUNT(SnapDSP1), version);
		if (result != SUCCESS && Settings.DSP == 1)
			break;

		result = UnfreezeStructCopy(stream, "DP2", &local_dsp2, SnapDSP2, COUNT(SnapDSP2), version);
		if (result != SUCCESS && Settings.DSP == 2)
			break;

		SkipBlockWithName(stream, "DP3");
		SkipBlockWithName(stream, "DP4");

		if (Settings.C4)
		{
			if (fast)
				result = UnfreezeBlock(stream, "CX4", Memory.C4RAM, 8192);
			else
				result = UnfreezeBlockCopy(stream, "CX4", &local_cx4_data, 8192);
			if (result != SUCCESS)
				break;
		}
		else
		{
			SkipBlockWithName(stream, "CX4");
		}

		SkipBlockWithName(stream, "ST0");
		SkipBlockWithName(stream, "OBC");
		SkipBlockWithName(stream, "OBM");
		SkipBlockWithName(stream, "S71");
		SkipBlockWithName(stream, "SRT");
		SkipBlockWithName(stream, "CLK");
		SkipBlockWithName(stream, "BSX");

		result = SUCCESS;
	} while (false);

	if (result == SUCCESS)
	{
		uint32 old_flags     = CPU.Flags;

		if (fast)
		{
			S9xResetPPUFast();
		}
		else
		{
			//Do not call this if you have written directly to "Memory." arrays
			S9xReset();
		}

		UnfreezeStructFromCopy(&CPU, SnapCPU, COUNT(SnapCPU), local_cpu, version);

		UnfreezeStructFromCopy(&Registers, SnapRegisters, COUNT(SnapRegisters), local_registers, version);

		UnfreezeStructFromCopy(&PPU, SnapPPU, COUNT(SnapPPU), local_ppu, version);

		struct SDMASnapshot	dma_snap;
		UnfreezeStructFromCopy(&dma_snap, SnapDMA, COUNT(SnapDMA), local_dma, version);

		if (local_vram)
			memcpy(Memory.VRAM, local_vram, 0x10000);

		if (local_ram)
			memcpy(Memory.RAM, local_ram, 0x20000);

		if (local_sram)
			memcpy(Memory.SRAM, local_sram, 0x20000);

		if (local_fillram)
			memcpy(Memory.FillRAM, local_fillram, 0x8000);

        if(version < SNAPSHOT_VERSION_BAPU) {
            printf("Using Blargg APU snapshot loading (snapshot version %d, current is %d)\n...", version, SNAPSHOT_VERSION);
            S9xAPULoadBlarggState(local_apu_sound);
        } else
		    S9xAPULoadState(local_apu_sound);

		struct SControlSnapshot	ctl_snap;
		UnfreezeStructFromCopy(&ctl_snap, SnapControls, COUNT(SnapControls), local_control_data, version);

		UnfreezeStructFromCopy(&Timings, SnapTimings, COUNT(SnapTimings), local_timing_data, version);

		if (local_dsp1)
			UnfreezeStructFromCopy(&DSP1, SnapDSP1, COUNT(SnapDSP1), local_dsp1, version);

		if (local_dsp2)
			UnfreezeStructFromCopy(&DSP2, SnapDSP2, COUNT(SnapDSP2), local_dsp2, version);

		if (local_cx4_data)
			memcpy(Memory.C4RAM, local_cx4_data, 8192);

		if (version < SNAPSHOT_VERSION_IRQ)
		{
			printf("Converting old snapshot version %d to %d\n...", version, SNAPSHOT_VERSION);

			CPU.NMIPending = (CPU.Flags & (1 <<  7)) ? TRUE : FALSE;
			CPU.IRQLine = (CPU.Flags & (1 << 11)) ? TRUE : FALSE;
			CPU.IRQTransition = FALSE;
			CPU.IRQLastState = FALSE;
			CPU.IRQExternal = (Obsolete.CPU_IRQActive & ~(1 << 1)) ? TRUE : FALSE;

			switch (CPU.WhichEvent)
			{
				case 12:	case   1:	CPU.WhichEvent = 1; break;
				case  2:	case   3:	CPU.WhichEvent = 2; break;
				case  4:	case   5:	CPU.WhichEvent = 3; break;
				case  6:	case   7:	CPU.WhichEvent = 4; break;
				case  8:	case   9:	CPU.WhichEvent = 5; break;
				case 10:	case  11:	CPU.WhichEvent = 6; break;
			}
		}

		CPU.Flags |= old_flags & (DEBUG_MODE_FLAG | TRACE_FLAG | SINGLE_STEP_FLAG | FRAME_ADVANCE_FLAG);
		ICPU.ShiftedPB = Registers.PB << 16;
		ICPU.ShiftedDB = Registers.DB << 16;
		S9xSetPCBase(Registers.PBPC);
		S9xUnpackStatus();
		if(version < SNAPSHOT_VERSION_IRQ_2018)
			S9xUpdateIRQPositions(false); // calculate the new trigger pos from saved PPU data
		S9xFixCycles();

		for (int d = 0; d < 8; d++)
			DMA[d] = dma_snap.dma[d];
		// TODO: these should already be correct since they are stored in the snapshot
		CPU.InDMA = CPU.InHDMA = FALSE;
		CPU.InDMAorHDMA = CPU.InWRAMDMAorHDMA = FALSE;
		CPU.HDMARanInDMA = 0;

		S9xFixColourBrightness();
		S9xBuildDirectColourMaps();
		IPPU.ColorsChanged = TRUE;
		IPPU.OBJChanged = TRUE;
		IPPU.RenderThisFrame = TRUE;

		GFX.InterlaceFrame = Timings.InterlaceField;
		GFX.DoInterlace = 0;

		S9xGraphicsScreenResize();

		if (Settings.FastSavestates == 0)
			memset(GFX.Screen,0,GFX.Pitch * MAX_SNES_HEIGHT);

		// TODO: this seems to be a relic from 1.43 changes, completely remove if no issues in the future
		/*uint8 hdma_byte = Memory.FillRAM[0x420c];
		S9xSetCPU(hdma_byte, 0x420c);*/

		S9xControlPostLoadState(&ctl_snap);
	}

	if (local_cpu)				delete [] local_cpu;
	if (local_registers)		delete [] local_registers;
	if (local_ppu)				delete [] local_ppu;
	if (local_dma)				delete [] local_dma;
	if (local_vram)				delete [] local_vram;
	if (local_ram)				delete [] local_ram;
	if (local_sram)				delete [] local_sram;
	if (local_fillram)			delete [] local_fillram;
	if (local_apu_sound)		delete [] local_apu_sound;
	if (local_control_data)		delete [] local_control_data;
	if (local_timing_data)		delete [] local_timing_data;
	if (local_dsp1)				delete [] local_dsp1;
	if (local_dsp2)				delete [] local_dsp2;
	if (local_cx4_data)			delete [] local_cx4_data;

	return (result);
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

static void FreezeStruct (STREAM stream, const char *name, void *base, FreezeData *fields, int num_fields)
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

	uint8	*block = new uint8[len];
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
	delete [] block;
}

static void FreezeBlock (STREAM stream, const char *name, uint8 *block, int size)
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

	WRITE_STREAM(buffer, 11, stream);
	WRITE_STREAM(block, size, stream);
}

static bool CheckBlockName(STREAM stream, const char *name, int &len)
{
	char	buffer[16];
	len = 0;

	size_t	l = READ_STREAM(buffer, 11, stream);
	buffer[l] = 0;
	REVERT_STREAM(stream, FIND_STREAM(stream) - l, 0);

	if (buffer[4] == '-')
	{
		len = (((unsigned char)buffer[6]) << 24)
			| (((unsigned char)buffer[7]) << 16)
			| (((unsigned char)buffer[8]) << 8)
			| (((unsigned char)buffer[9]) << 0);
	}
	else
		len = atoi(buffer + 4);

	if (l != 11 || strncmp(buffer, name, 3) != 0 || buffer[3] != ':')
	{
		return false;
	}

	if (len <= 0)
	{
		return false;
	}

	return true;
}

static void SkipBlockWithName(STREAM stream, const char *name)
{
	int len;
	bool matchesName = CheckBlockName(stream, name, len);
	if (matchesName)
	{
		long rewind = FIND_STREAM(stream);
		rewind += len + 11;
		REVERT_STREAM(stream, rewind, 0);
	}
}

static int UnfreezeBlock (STREAM stream, const char *name, uint8 *block, int size)
{
	char	buffer[20];
	int		len = 0, rem = 0;
	long	rewind = FIND_STREAM(stream);

	size_t	l = READ_STREAM(buffer, 11, stream);
	buffer[l] = 0;

	if (l != 11 || strncmp(buffer, name, 3) != 0 || buffer[3] != ':')
	{
	err:
#ifdef DEBUGGER
		fprintf(stdout, "absent: %s(%d); next: '%.11s'\n", name, size, buffer);
#endif
		REVERT_STREAM(stream, FIND_STREAM(stream) - l, 0);
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

	if (!Settings.FastSavestates)
	{
		memset(block, 0, size);
	}

	if (READ_STREAM(block, len, stream) != (unsigned int) len)
	{
		REVERT_STREAM(stream, rewind, 0);
		return (WRONG_FORMAT);
	}

	if (rem)
	{
		char	*junk = new char[rem];
		len = READ_STREAM(junk, rem, stream);
		delete [] junk;
		if (len != rem)
		{
			REVERT_STREAM(stream, rewind, 0);
			return (WRONG_FORMAT);
		}
	}

	return (SUCCESS);
}

static int UnfreezeBlockCopy (STREAM stream, const char *name, uint8 **block, int size)
{
	int	result;

	//check name first to avoid memory allocation
	int blockLength;
	if (!CheckBlockName(stream, name, blockLength))
	{
		return 0;
	}

	*block = new uint8[size];

	result = UnfreezeBlock(stream, name, *block, size);
	if (result != SUCCESS)
	{
		delete [] (*block);
		*block = NULL;
		return (result);
	}

	return (SUCCESS);
}

static int UnfreezeStruct (STREAM stream, const char *name, void *base, FreezeData *fields, int num_fields, int version)
{
	int		result;
	uint8	*block = NULL;

	result = UnfreezeStructCopy(stream, name, &block, fields, num_fields, version);
	if (result != SUCCESS)
	{
		if (block != NULL)
			delete [] block;
		return (result);
	}

	UnfreezeStructFromCopy(base, fields, num_fields, block, version);
	delete [] block;

	return (SUCCESS);
}

static int UnfreezeStructCopy (STREAM stream, const char *name, uint8 **block, FreezeData *fields, int num_fields, int version)
{
	int	len = 0;

	for (int i = 0; i < num_fields; i++)
	{
		if (version >= fields[i].debuted_in && version < fields[i].deleted_in)
			len += FreezeSize(fields[i].size, fields[i].type);
	}

	return (UnfreezeBlockCopy(stream, name, block, len));
}

static void UnfreezeStructFromCopy (void *sbase, FreezeData *fields, int num_fields, uint8 *block, int version)
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
