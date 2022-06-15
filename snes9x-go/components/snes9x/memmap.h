/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _MEMMAP_H_
#define _MEMMAP_H_

#define MEMMAP_BLOCK_SIZE	(0x1000)
#define MEMMAP_NUM_BLOCKS	(0x1000000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_SHIFT		(12)
#define MEMMAP_MASK			(MEMMAP_BLOCK_SIZE - 1)

enum
{
	MAP_TYPE_I_O,
	MAP_TYPE_ROM,
	MAP_TYPE_RAM
};

enum
{
	MAP_CPU,
	MAP_PPU,
	MAP_LOROM_SRAM,
	MAP_HIROM_SRAM,
	MAP_DSP,
	MAP_RONLY_SRAM,
	MAP_NONE,
	MAP_LAST
};

typedef enum
{
	WRAP_NONE,
	WRAP_BANK,
	WRAP_PAGE
} s9xwrap_t;

typedef enum
{
	WRITE_01,
	WRITE_10
} s9xwriteorder_t;

typedef struct
{
	uint8	CPU_IO[0x400];
	uint8	PPU_IO[0x200];
	uint8	*RAM;
	uint8	*ROM;
	uint8	*SRAM;
	uint8	*VRAM;
	uint8	*OBC1RAM;

#if RETRO_COMBINED_MEMORY_MAP
	union {
	uint8	*ReadMap[MEMMAP_NUM_BLOCKS];
	uint8	*WriteMap[MEMMAP_NUM_BLOCKS];
	};
#else
	uint8	*ReadMap[MEMMAP_NUM_BLOCKS];
	uint8	*WriteMap[MEMMAP_NUM_BLOCKS];
#endif

	char	ROMName[ROM_NAME_LEN];
	char	ROMId[5];
	uint8	ROMRegion;
	uint8	ROMSpeed;
	uint8	ROMType;
	uint8	ROMSize;
	uint32	ROMChecksum;
	uint32	ROMComplementChecksum;
	uint32	ROMCRC32;
	bool8	ROMIsPatched;

	bool8	HiROM;
	bool8	LoROM;
	uint32	SRAMSize;
	uint32	SRAMBytes;
	uint32	SRAMMask;
	uint32	CalculatedSize;
	uint32	CalculatedChecksum;

	// uint32	ROM_BLOCKS[ROM_MAX_SIZE / ROM_BLOCK_SIZE];
	uint32	ROM_MAX_SIZE;
	uint32	ROM_SIZE;
} CMemory;

extern CMemory	Memory;
extern uint32	OpenBus;

bool8 S9xLoadROMMem (const uint8 *);
bool8 S9xLoadROM (const char *);
bool8 S9xMemoryInit (void);
void S9xMemoryDeinit (void);


#include "cpu.h"
#include "dsp.h"


#if RETRO_LESS_ACCURATE_MEM
#define addCyclesInMemoryAccess \
	if (!CPU.InDMAorHDMA) \
	{ \
		CPU.Cycles += speed; \
	}
#define addCyclesInMemoryAccess_x2 \
	if (!CPU.InDMAorHDMA) \
	{ \
		CPU.Cycles += speed << 1; \
	}
#else
#define addCyclesInMemoryAccess \
	if (!CPU.InDMAorHDMA) \
	{ \
		CPU.Cycles += speed; \
		if (CPU.Cycles >= CPU.NextEvent) \
			S9xDoHEventProcessing(); \
	}
#define addCyclesInMemoryAccess_x2 \
	if (!CPU.InDMAorHDMA) \
	{ \
		CPU.Cycles += speed << 1; \
		if (CPU.Cycles >= CPU.NextEvent) \
			S9xDoHEventProcessing(); \
	}
#endif

// This is just to track where ROM memory access occurs. If the trapping isn't too costly, this info
// will eventually be used it to handle large roms through esp_himem_*
#define CHECK_ROM_MAPPING(a) 	// if ((a) >= Memory.ROM && (a) - Memory.ROM < Memory.ROM_MAX_SIZE) { printf("ROM in %s at %p\n", __func__, (a)); }


static inline int32 memory_speed (uint32 address)
{
	if (address & 0x408000)
	{
		if (address & 0x800000)
			return (CPU.FastROMSpeed);

		return (SLOW_ONE_CYCLE);
	}

	if ((address + 0x6000) & 0x4000)
		return (SLOW_ONE_CYCLE);

	if ((address - 0x4000) & 0x7e00)
		return (ONE_CYCLE);

	return (TWO_CYCLES);
}

inline uint8 S9xGetByte (uint32 Address)
{
	uint8	*GetAddress = Memory.ReadMap[(Address & 0xffffff) >> MEMMAP_SHIFT];
	uint32	speed = memory_speed(Address);
	uint32	byte;

	if (GetAddress >= (uint8 *) MAP_LAST)
	{
		CHECK_ROM_MAPPING(GetAddress + (Address & 0xffff));
		byte = *(GetAddress + (Address & 0xffff));
		addCyclesInMemoryAccess;
		return (byte);
	}

	switch ((pint) GetAddress)
	{
		case MAP_CPU:
			byte = S9xGetCPU(Address & 0xffff);
			addCyclesInMemoryAccess;
			return (byte);

		case MAP_PPU:
			if (CPU.InDMAorHDMA && (Address & 0xff00) == 0x2100)
				return (OpenBus);

			byte = S9xGetPPU(Address & 0xffff);
			addCyclesInMemoryAccess;
			return (byte);

		case MAP_LOROM_SRAM:
			// Address & 0x7fff   : offset into bank
			// Address & 0xff0000 : bank
			// bank >> 1 | offset : SRAM address, unbound
			// unbound & SRAMMask : SRAM offset
			byte = *(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask));
			addCyclesInMemoryAccess;
			return (byte);

		case MAP_HIROM_SRAM:
		case MAP_RONLY_SRAM:
			byte = *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask));
			addCyclesInMemoryAccess;
			return (byte);

		case MAP_DSP:
			byte = S9xGetDSP(Address & 0xffff);
			addCyclesInMemoryAccess;
			return (byte);

		case MAP_NONE:
		default:
			byte = OpenBus;
			addCyclesInMemoryAccess;
			return (byte);
	}
}

inline uint16 S9xGetWord (uint32 Address, s9xwrap_t w = WRAP_NONE)
{
	uint32	mask = MEMMAP_MASK & (w == WRAP_PAGE ? 0xff : (w == WRAP_BANK ? 0xffff : 0xffffff));
	uint32	word;

	if ((Address & mask) == mask)
	{
		PC_t	a;

		word = OpenBus = S9xGetByte(Address);

		switch (w)
		{
			case WRAP_PAGE:
				a.xPBPC = Address;
				a.B.xPCl++;
				return (word | (S9xGetByte(a.xPBPC) << 8));

			case WRAP_BANK:
				a.xPBPC = Address;
				a.W.xPC++;
				return (word | (S9xGetByte(a.xPBPC) << 8));

			case WRAP_NONE:
			default:
				return (word | (S9xGetByte(Address + 1) << 8));
		}
	}

	uint8	*GetAddress = Memory.ReadMap[(Address & 0xffffff) >> MEMMAP_SHIFT];
	uint32	speed = memory_speed(Address);

	if (GetAddress >= (uint8 *) MAP_LAST)
	{
		CHECK_ROM_MAPPING(GetAddress + (Address & 0xffff));
		word = READ_WORD(GetAddress + (Address & 0xffff));
		addCyclesInMemoryAccess_x2;
		return (word);
	}

	switch ((pint) GetAddress)
	{
		case MAP_CPU:
			word  = S9xGetCPU(Address & 0xffff);
			word |= S9xGetCPU((Address + 1) & 0xffff) << 8;
			addCyclesInMemoryAccess_x2;
			return (word);

		case MAP_PPU:
			if (CPU.InDMAorHDMA)
			{
				word = OpenBus = S9xGetByte(Address);
				return (word | (S9xGetByte(Address + 1) << 8));
			}

			word  = S9xGetPPU(Address & 0xffff);
			word |= S9xGetPPU((Address + 1) & 0xffff) << 8;
			addCyclesInMemoryAccess_x2;
			return (word);

		case MAP_LOROM_SRAM:
			if (Memory.SRAMMask >= MEMMAP_MASK)
				word = READ_WORD(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask));
			else
				word = (*(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask))) |
					  ((*(Memory.SRAM + (((((Address + 1) & 0xff0000) >> 1) | ((Address + 1) & 0x7fff)) & Memory.SRAMMask))) << 8);
			addCyclesInMemoryAccess_x2;
			return (word);

		case MAP_HIROM_SRAM:
		case MAP_RONLY_SRAM:
			if (Memory.SRAMMask >= MEMMAP_MASK)
				word = READ_WORD(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask));
			else
				word = (*(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) |
					   (*(Memory.SRAM + ((((Address + 1) & 0x7fff) - 0x6000 + (((Address + 1) & 0xf0000) >> 3)) & Memory.SRAMMask)) << 8));
			addCyclesInMemoryAccess_x2;
			return (word);

		case MAP_DSP:
			word  = S9xGetDSP(Address & 0xffff);
			word |= S9xGetDSP((Address + 1) & 0xffff) << 8;
			addCyclesInMemoryAccess_x2;
			return (word);

		case MAP_NONE:
		default:
			word = OpenBus | (OpenBus << 8);
			addCyclesInMemoryAccess_x2;
			return (word);
	}
}

inline void S9xSetByte (uint8 Byte, uint32 Address)
{
	uint8	*SetAddress = Memory.WriteMap[(Address & 0xffffff) >> MEMMAP_SHIFT];
	uint32	speed = memory_speed(Address);

	if (SetAddress >= (uint8 *) MAP_LAST)
	{
		*(SetAddress + (Address & 0xffff)) = Byte;
		addCyclesInMemoryAccess;
		return;
	}

	switch ((pint) SetAddress)
	{
		case MAP_CPU:
			S9xSetCPU(Byte, Address & 0xffff);
			addCyclesInMemoryAccess;
			return;

		case MAP_PPU:
			if (CPU.InDMAorHDMA && (Address & 0xff00) == 0x2100)
				return;

			S9xSetPPU(Byte, Address & 0xffff);
			addCyclesInMemoryAccess;
			return;

		case MAP_LOROM_SRAM:
			if (Memory.SRAMMask)
			{
				*(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask)) = Byte;
				CPU.SRAMModified = TRUE;
			}

			addCyclesInMemoryAccess;
			return;

		case MAP_HIROM_SRAM:
			if (Memory.SRAMMask)
			{
				*(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) = Byte;
				CPU.SRAMModified = TRUE;
			}

			addCyclesInMemoryAccess;
			return;

		case MAP_DSP:
			S9xSetDSP(Byte, Address & 0xffff);
			addCyclesInMemoryAccess;
			return;

		case MAP_NONE:
		default:
			addCyclesInMemoryAccess;
			return;
	}
}

inline void S9xSetWord (uint16 Word, uint32 Address, s9xwrap_t w, s9xwriteorder_t o)
{
	uint32	mask = MEMMAP_MASK & (w == WRAP_PAGE ? 0xff : (w == WRAP_BANK ? 0xffff : 0xffffff));

	if ((Address & mask) == mask)
	{
		PC_t	a;

		if (!o)
			S9xSetByte((uint8) Word, Address);

		switch (w)
		{
			case WRAP_PAGE:
				a.xPBPC = Address;
				a.B.xPCl++;
				S9xSetByte(Word >> 8, a.xPBPC);
				break;

			case WRAP_BANK:
				a.xPBPC = Address;
				a.W.xPC++;
				S9xSetByte(Word >> 8, a.xPBPC);
				break;

			case WRAP_NONE:
			default:
				S9xSetByte(Word >> 8, Address + 1);
				break;
		}

		if (o)
			S9xSetByte((uint8) Word, Address);

		return;
	}

	uint8	*SetAddress = Memory.WriteMap[(Address & 0xffffff) >> MEMMAP_SHIFT];
	uint32	speed = memory_speed(Address);

	if (SetAddress >= (uint8 *) MAP_LAST)
	{
		WRITE_WORD(SetAddress + (Address & 0xffff), Word);
		addCyclesInMemoryAccess_x2;
		return;
	}

	switch ((pint) SetAddress)
	{
		case MAP_CPU:
			if (o)
			{
				S9xSetCPU(Word >> 8, (Address + 1) & 0xffff);
				S9xSetCPU((uint8) Word, Address & 0xffff);
			}
			else
			{
				S9xSetCPU((uint8) Word, Address & 0xffff);
				S9xSetCPU(Word >> 8, (Address + 1) & 0xffff);
			}
			addCyclesInMemoryAccess_x2;
			return;

		case MAP_PPU:
			if (CPU.InDMAorHDMA)
			{
				if ((Address & 0xff00) != 0x2100)
					S9xSetPPU((uint8) Word, Address & 0xffff);
				if (((Address + 1) & 0xff00) != 0x2100)
					S9xSetPPU(Word >> 8, (Address + 1) & 0xffff);
				return;
			}

			if (o)
			{
				S9xSetPPU(Word >> 8, (Address + 1) & 0xffff);
				S9xSetPPU((uint8) Word, Address & 0xffff);
			}
			else
			{
				S9xSetPPU((uint8) Word, Address & 0xffff);
				S9xSetPPU(Word >> 8, (Address + 1) & 0xffff);
			}
			addCyclesInMemoryAccess_x2;
			return;

		case MAP_LOROM_SRAM:
			if (Memory.SRAMMask)
			{
				if (Memory.SRAMMask >= MEMMAP_MASK)
					WRITE_WORD(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask), Word);
				else
				{
					*(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask)) = (uint8) Word;
					*(Memory.SRAM + (((((Address + 1) & 0xff0000) >> 1) | ((Address + 1) & 0x7fff)) & Memory.SRAMMask)) = Word >> 8;
				}

				CPU.SRAMModified = TRUE;
			}

			addCyclesInMemoryAccess_x2;
			return;

		case MAP_HIROM_SRAM:
			if (Memory.SRAMMask)
			{
				if (Memory.SRAMMask >= MEMMAP_MASK)
					WRITE_WORD(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask), Word);
				else
				{
					*(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) = (uint8) Word;
					*(Memory.SRAM + ((((Address + 1) & 0x7fff) - 0x6000 + (((Address + 1) & 0xf0000) >> 3)) & Memory.SRAMMask)) = Word >> 8;
				}

				CPU.SRAMModified = TRUE;
			}

			addCyclesInMemoryAccess_x2;
			return;

		case MAP_DSP:
			if (o)
			{
				S9xSetDSP(Word >> 8, (Address + 1) & 0xffff);
				S9xSetDSP((uint8) Word, Address & 0xffff);
			}
			else
			{
				S9xSetDSP((uint8) Word, Address & 0xffff);
				S9xSetDSP(Word >> 8, (Address + 1) & 0xffff);
			}
			addCyclesInMemoryAccess_x2;
			return;

		case MAP_NONE:
		default:
			addCyclesInMemoryAccess_x2;
			return;
	}
}

inline uint8 * S9xGetBasePointer (uint32 Address)
{
	uint8	*GetAddress = Memory.ReadMap[(Address & 0xffffff) >> MEMMAP_SHIFT];

	if ((pint) GetAddress >= MAP_LAST)
	{
		CHECK_ROM_MAPPING(GetAddress);
		return (GetAddress);
	}
	else if ((pint) GetAddress == MAP_LOROM_SRAM)
	{
		if ((Memory.SRAMMask & MEMMAP_MASK) == MEMMAP_MASK)
			return (Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask) - (Address & 0xffff));
	}
	else if ((pint) GetAddress == MAP_HIROM_SRAM)
	{
		if ((Memory.SRAMMask & MEMMAP_MASK) == MEMMAP_MASK)
			return (Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask) - (Address & 0xffff));
	}

	return (NULL);
}

inline void S9xSetPCBase (uint32 Address)
{
	Registers.PBPC = Address & 0xffffff;
	ICPU.ShiftedPB = Address & 0xff0000;

	CPU.MemSpeed = memory_speed(Address);
	CPU.MemSpeedx2 = CPU.MemSpeed << 1;

	CPU.PCBase = S9xGetBasePointer(Address);
}

#endif
