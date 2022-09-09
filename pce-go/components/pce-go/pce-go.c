// pce-go.c - Entry file to start/stop/reset/save emulation
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pce-go.h"
#include "gfx.h"
#include "psg.h"
#include "pce.h"

/**
 * Save state file description.
 */
#define SVAR_1(k, v) { {k, 1, 1}, &v }
#define SVAR_2(k, v) { {k, 2, 2}, &v }
#define SVAR_4(k, v) { {k, 4, 4}, &v }
#define SVAR_A(k, v) { {k, 0, sizeof(v)}, &v }
#define SVAR_N(k, v, n) { {k, 0, n}, &v }
#define SVAR_END { {"END", 0, 0}, NULL }

typedef struct __attribute__((packed))
{
	char key[12];
	uint32_t type:8;
	uint32_t len:24;
} block_hdr_t;

typedef const struct
{
	block_hdr_t desc;
	void *ptr;
} save_var_t;

static const char SAVESTATE_HEADER[8] = "PCE_V010";
static save_var_t SaveStateVars[] =
{
	// Arrays
	SVAR_A("RAM", PCE.RAM),      SVAR_A("VRAM", PCE.VRAM),  SVAR_A("SPRAM", PCE.SPRAM),
	SVAR_A("PAL", PCE.Palette),  SVAR_A("MMR", PCE.MMR),

	// CPU registers
	SVAR_2("CPU.PC", CPU.PC),    SVAR_1("CPU.A", CPU.A),    SVAR_1("CPU.X", CPU.X),
	SVAR_1("CPU.Y", CPU.Y),      SVAR_1("CPU.P", CPU.P),    SVAR_1("CPU.S", CPU.S),

	// Misc
	SVAR_4("Cycles", PCE.Cycles),               SVAR_4("MaxCycles", PCE.MaxCycles),
	SVAR_1("SF2", PCE.SF2),

	// IRQ
	SVAR_1("IRQ.mask", CPU.irq_mask),           SVAR_1("IRQ.lines", CPU.irq_lines),
	SVAR_1("IRQ.m_delay", CPU.irq_mask_delay),

	// PSG
	SVAR_1("PSG.ch", PCE.PSG.ch),               SVAR_1("PSG.vol", PCE.PSG.volume),
	SVAR_1("PSG.lfo_f", PCE.PSG.lfo_freq),      SVAR_1("PSG.lfo_c", PCE.PSG.lfo_ctrl),
	SVAR_N("PSG.ch0", PCE.PSG.chan[0], 40),     SVAR_N("PSG.ch1", PCE.PSG.chan[1], 40),
	SVAR_N("PSG.ch2", PCE.PSG.chan[2], 40),     SVAR_N("PSG.ch3", PCE.PSG.chan[3], 40),
	SVAR_N("PSG.ch4", PCE.PSG.chan[4], 40),     SVAR_N("PSG.ch5", PCE.PSG.chan[5], 40),

	// VCE
	SVAR_A("VCE.regs", PCE.VCE.regs),           SVAR_2("VCE.reg", PCE.VCE.reg),

	// VDC
	SVAR_A("VDC.regs", PCE.VDC.regs),           SVAR_1("VDC.reg", PCE.VDC.reg),
	SVAR_1("VDC.status", PCE.VDC.status),       SVAR_1("VDC.satb", PCE.VDC.satb),
	SVAR_4("VDC.irqs", PCE.VDC.pending_irqs),   SVAR_1("VDC.vram", PCE.VDC.vram),

	// Timer
	SVAR_4("TMR.reload", PCE.Timer.reload),   SVAR_4("TMR.running", PCE.Timer.running),
	SVAR_4("TMR.counter", PCE.Timer.counter), SVAR_4("TMR.next", PCE.Timer.cycles_counter),
	SVAR_4("TMR.freq", PCE.Timer.cycles_per_line),

	SVAR_END
};

#define TWO_PART_ROM 0x0001
#define ONBOARD_RAM  0x0100
#define US_ENCODED   0x0010

static const struct {
	const uint32_t CRC;
	const char *Name;
	const uint32_t Flags;
} romFlags[] = {
	{0xF0ED3094, "Blazing Lazers", TWO_PART_ROM},
	{0xB4A1B0F6, "Blazing Lazers", TWO_PART_ROM},
	{0x55E9630D, "Legend of Hero Tonma", US_ENCODED},
	{0x083C956A, "Populous", ONBOARD_RAM},
	{0x0A9ADE99, "Populous", ONBOARD_RAM},
	{0x00000000, "Unknown", 0},
};

static bool running = false;

/**
 * Load card into memory and set its memory map
 */
int
LoadCard(const char *name)
{
	int fsize, offset;

	MESSAGE_INFO("Opening %s...\n", name);

	FILE *fp = fopen(name, "rb");

	if (fp == NULL)
	{
		MESSAGE_ERROR("Failed to open %s!\n", name);
		return -1;
	}

	if (PCE.ROM != NULL) {
		free(PCE.ROM);
	}

	// find file size
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	offset = fsize & 0x1fff;

	// read ROM
	PCE.ROM = malloc(fsize);

	if (PCE.ROM == NULL)
	{
		MESSAGE_ERROR("Failed to allocate ROM buffer!\n");
		return -1;
	}

	fseek(fp, 0, SEEK_SET);
	fread(PCE.ROM, 1, fsize, fp);

	fclose(fp);

	PCE.ROM_SIZE = (fsize - offset) / 0x2000;
	PCE.ROM_DATA = PCE.ROM + offset;
	PCE.ROM_CRC = crc32_le(0, PCE.ROM, fsize);

	uint32_t IDX = 0;
	uint32_t ROM_MASK = 1;

	while (ROM_MASK < PCE.ROM_SIZE) ROM_MASK <<= 1;
	ROM_MASK--;

	MESSAGE_INFO("ROM LOADED: OFFSET=%d, BANKS=%d, MASK=%03X, CRC=%08X\n",
		offset, PCE.ROM_SIZE, ROM_MASK, PCE.ROM_CRC);

	while (romFlags[IDX].CRC) {
		if (PCE.ROM_CRC == romFlags[IDX].CRC)
			break;
		IDX++;
	}

	MESSAGE_INFO("Game Name: %s\n", romFlags[IDX].Name);

	// US Encrypted
	if ((romFlags[IDX].Flags & US_ENCODED) || PCE.ROM_DATA[0x1FFF] < 0xE0)
	{
		MESSAGE_INFO("This rom is probably US encrypted, decrypting...\n");

		unsigned char inverted_nibble[16] = {
			0, 8, 4, 12, 2, 10, 6, 14,
			1, 9, 5, 13, 3, 11, 7, 15
		};

		for (int x = 0; x < PCE.ROM_SIZE * 0x2000; x++) {
			unsigned char temp = PCE.ROM_DATA[x] & 15;

			PCE.ROM_DATA[x] &= ~0x0F;
			PCE.ROM_DATA[x] |= inverted_nibble[PCE.ROM_DATA[x] >> 4];

			PCE.ROM_DATA[x] &= ~0xF0;
			PCE.ROM_DATA[x] |= inverted_nibble[temp] << 4;
		}
	}

	// For example with Devil Crush 512Ko
	if (romFlags[IDX].Flags & TWO_PART_ROM)
		PCE.ROM_SIZE = 0x30;

	// Game ROM
	for (int i = 0; i < 0x80; i++) {
		if (PCE.ROM_SIZE == 0x30) {
			switch (i & 0x70) {
			case 0x00:
			case 0x10:
			case 0x50:
				PCE.MemoryMapR[i] = PCE.ROM_DATA + (i & ROM_MASK) * 0x2000;
				break;
			case 0x20:
			case 0x60:
				PCE.MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x20) & ROM_MASK) * 0x2000;
				break;
			case 0x30:
			case 0x70:
				PCE.MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x10) & ROM_MASK) * 0x2000;
				break;
			case 0x40:
				PCE.MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x20) & ROM_MASK) * 0x2000;
				break;
			}
		} else {
			PCE.MemoryMapR[i] = PCE.ROM_DATA + (i & ROM_MASK) * 0x2000;
		}
		PCE.MemoryMapW[i] = PCE.NULLRAM;
	}

	// Allocate the card's onboard ram
	if (romFlags[IDX].Flags & ONBOARD_RAM) {
		PCE.ExRAM = PCE.ExRAM ?: malloc(0x8000);
		PCE.MemoryMapR[0x40] = PCE.MemoryMapW[0x40] = PCE.ExRAM;
		PCE.MemoryMapR[0x41] = PCE.MemoryMapW[0x41] = PCE.ExRAM + 0x2000;
		PCE.MemoryMapR[0x42] = PCE.MemoryMapW[0x42] = PCE.ExRAM + 0x4000;
		PCE.MemoryMapR[0x43] = PCE.MemoryMapW[0x43] = PCE.ExRAM + 0x6000;
	}

	// Mapper for roms >= 1.5MB (SF2, homebrews)
	if (PCE.ROM_SIZE >= 192)
		PCE.MemoryMapW[0x00] = PCE.IOAREA;

	return 0;
}


/**
 * Reset the emulator
 */
void
ResetPCE(bool hard)
{
	gfx_reset(hard);
	pce_reset(hard);
}


/**
 * Initialize the emulator (allocate memory, call osd_init* functions)
 */
int
InitPCE(int samplerate, bool stereo, const char *huecard)
{
	if (gfx_init())
		return 1;

	if (psg_init(samplerate, stereo))
		return 1;

	if (pce_init())
		return 1;

	if (huecard && LoadCard(huecard))
		return 1;

	ResetPCE(0);

	return 0;
}


/**
 * Returns a 256 colors palette in the chosen depth
 */
void *
PalettePCE(int bitdepth)
{
	if (bitdepth == 15) {
		uint16_t *palette = malloc(256 * 2);
		// ...
		return palette;
	}

	if (bitdepth == 16) {
		uint16_t *palette = malloc(256 * 2);
		for (int i = 0; i < 255; i++) {
			int r = (i & 0x1C) >> 1;
			int g = (i & 0xe0) >> 4;
			int b = (i & 0x03) << 2;
			palette[i] = (((r << 12) & 0xf800) + ((g << 7) & 0x07e0) + ((b << 1) & 0x001f));
		}
		palette[255] = 0xFFFF;
		return palette;
	}

	if (bitdepth == 24) {
		uint8_t *palette = malloc(256 * 3);
		uint8_t *ptr = palette;
		for (int i = 0; i < 255; i++) {
			*ptr++ = (i & 0x1C) << 2;
			*ptr++ = (i & 0xe0) >> 1;
			*ptr++ = (i & 0x03) << 4;
		}
		*ptr++ = 0xFF;
		*ptr++ = 0xFF;
		*ptr++ = 0xFF;
		return palette;
	}

	return NULL;
}


/**
 * Start the emulation
 */
void
RunPCE(void)
{
	running = true;

	while (running)
	{
		osd_input_read(PCE.Joypad.regs);
		pce_run();
		osd_vsync();
	}
}


/**
 * Load saved state
 */
int
LoadState(const char *name)
{
	MESSAGE_INFO("Loading state from %s...\n", name);

	char buffer[32];
	block_hdr_t block;
	int ret = -1;

	FILE *fp = fopen(name, "rb");
	if (fp == NULL)
		return -1;

	if (!fread(&buffer, 8, 1, fp) || memcmp(&buffer, SAVESTATE_HEADER, 8) != 0)
	{
		MESSAGE_ERROR("Loading state failed: Header mismatch\n");
		goto _cleanup;
	}

	while (fread(&block, sizeof(block), 1, fp))
	{
		size_t block_end = ftell(fp) + block.len;

		for (save_var_t *var = SaveStateVars; var->ptr; var++)
		{
			if (strncmp(var->desc.key, block.key, 12) == 0)
			{
				size_t len = MIN((size_t)var->desc.len, (size_t)block.len);
				if (!fread(var->ptr, len, 1, fp))
				{
					MESSAGE_ERROR("fread error reading block data\n");
					goto _cleanup;
				}
				if (len < var->desc.len)
				{
					memset(var->ptr + len, 0, var->desc.len - len);
				}
				MESSAGE_INFO("Loaded %s\n", var->desc.key);
				break;
			}
		}
		fseek(fp, block_end, SEEK_SET);
	}

	for (int i = 0; i < 8; i++)
		pce_bank_set(i, PCE.MMR[i]);

	gfx_reset(true);
	PCE.VDC.mode_chg = 1;
	ret = 0;

_cleanup:
	fclose(fp);

	return ret;
}


/**
 * Save current state
 */
int
SaveState(const char *name)
{
	MESSAGE_INFO("Saving state to %s...\n", name);

	int ret = -1;

	FILE *fp = fopen(name, "wb");
	if (fp == NULL)
		return -1;

	fwrite(SAVESTATE_HEADER, sizeof(SAVESTATE_HEADER), 1, fp);

	for (save_var_t *var = SaveStateVars; var->ptr; var++)
	{
		if (!fwrite(&var->desc, sizeof(var->desc), 1, fp))
		{
			MESSAGE_ERROR("fwrite error desc\n");
			goto _cleanup;
		}
		if (!fwrite(var->ptr, var->desc.len, 1, fp))
		{
			MESSAGE_ERROR("fwrite error value\n");
			goto _cleanup;
		}
		MESSAGE_INFO("Saved %s\n", var->desc.key);
	}

	ret = 0;

_cleanup:
	fclose(fp);

	return ret;
}


/**
 * Cleanup and quit (not used in retro-go)
 */
void
ShutdownPCE()
{
	gfx_term();
	psg_term();
	pce_term();

	exit(0);
}
