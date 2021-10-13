// pce-go.c - Entry file to start/stop/reset/save emulation
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pce-go.h"
#include "utils.h"
#include "gfx.h"
#include "psg.h"
#include "pce.h"

/**
 * Describes what is saved in a save state. Changing the order will break
 * previous saves so add a place holder if necessary. Eventually we could use
 * the keys to make order irrelevant...
 */
#define SVAR_1(k, v) { 1, k, &v }
#define SVAR_2(k, v) { 2, k, &v }
#define SVAR_4(k, v) { 4, k, &v }
#define SVAR_A(k, v) { sizeof(v), k, &v }
#define SVAR_N(k, v, n) { n, k, &v }
#define SVAR_END { 0, "\0\0\0\0", 0 }

static const char SAVESTATE_HEADER[8] = "PCE_V004";
static const struct
{
	size_t len;
	char key[16];
	void *ptr;
} SaveStateVars[] =
{
	// Arrays
	SVAR_A("RAM", PCE.RAM),      SVAR_A("VRAM", PCE.VRAM),  SVAR_A("SPRAM", PCE.SPRAM),
	SVAR_A("PAL", PCE.Palette),  SVAR_A("MMR", PCE.MMR),

	// CPU registers
	SVAR_2("CPU.PC", CPU.PC),    SVAR_1("CPU.A", CPU.A),    SVAR_1("CPU.X", CPU.X),
	SVAR_1("CPU.Y", CPU.Y),      SVAR_1("CPU.P", CPU.P),    SVAR_1("CPU.S", CPU.S),

	// Misc
	SVAR_4("Cycles", Cycles),                   SVAR_4("MaxCycles", PCE.MaxCycles),
	SVAR_1("SF2", PCE.SF2),

	// IRQ
	SVAR_1("irq_mask", CPU.irq_mask),           SVAR_1("irq_lines", CPU.irq_lines),

	// PSG
	SVAR_1("psg.ch", PCE.PSG.ch),               SVAR_1("psg.vol", PCE.PSG.volume),
	SVAR_1("psg.lfo_f", PCE.PSG.lfo_freq),      SVAR_1("psg.lfo_c", PCE.PSG.lfo_ctrl),
	SVAR_N("psg.ch0", PCE.PSG.chan[0], 40),     SVAR_N("psg.ch1", PCE.PSG.chan[1], 40),
	SVAR_N("psg.ch2", PCE.PSG.chan[2], 40),     SVAR_N("psg.ch3", PCE.PSG.chan[3], 40),
	SVAR_N("psg.ch4", PCE.PSG.chan[4], 40),     SVAR_N("psg.ch5", PCE.PSG.chan[5], 40),

	// VCE
	SVAR_A("vce_regs", PCE.VCE.regs),           SVAR_2("vce_reg", PCE.VCE.reg),

	// VDC
	SVAR_A("vdc_regs", PCE.VDC.regs),           SVAR_1("vdc_reg", PCE.VDC.reg),
	SVAR_1("vdc_status", PCE.VDC.status),       SVAR_1("vdc_satb", PCE.VDC.satb),
	SVAR_4("vdc_pending_irqs", PCE.VDC.pending_irqs),

	// Timer
	SVAR_4("timer_reload", PCE.Timer.reload),   SVAR_4("timer_running", PCE.Timer.running),
	SVAR_4("timer_counter", PCE.Timer.counter), SVAR_4("timer_next", PCE.Timer.cycles_counter),

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
	pce_run();
}


/**
 * Load saved state
 */
int
LoadState(const char *name)
{
	MESSAGE_INFO("Loading state from %s...\n", name);

	char buffer[512];

	FILE *fp = fopen(name, "rb");
	if (fp == NULL)
		return -1;

	fread(&buffer, 8, 1, fp);

	if (memcmp(&buffer, SAVESTATE_HEADER, 8) != 0)
	{
		MESSAGE_ERROR("Loading state failed: Header mismatch\n");
		fclose(fp);
		return -1;
	}

	for (int i = 0; SaveStateVars[i].len > 0; i++)
	{
		MESSAGE_INFO("Loading %s (%d)\n", SaveStateVars[i].key, SaveStateVars[i].len);
		fread(SaveStateVars[i].ptr, SaveStateVars[i].len, 1, fp);
	}

	for(int i = 0; i < 8; i++)
	{
		pce_bank_set(i, PCE.MMR[i]);
	}

	gfx_reset(true);

	osd_gfx_set_mode(IO_VDC_SCREEN_WIDTH, IO_VDC_SCREEN_HEIGHT);

	fclose(fp);

	return 0;
}


/**
 * Save current state
 */
int
SaveState(const char *name)
{
	MESSAGE_INFO("Saving state to %s...\n", name);

	FILE *fp = fopen(name, "wb");
	if (fp == NULL)
		return -1;

	fwrite(SAVESTATE_HEADER, sizeof(SAVESTATE_HEADER), 1, fp);

	for (int i = 0; SaveStateVars[i].len > 0; i++)
	{
		MESSAGE_INFO("Saving %s (%d)\n", SaveStateVars[i].key, SaveStateVars[i].len);
		fwrite(SaveStateVars[i].ptr, SaveStateVars[i].len, 1, fp);
	}

	fclose(fp);

	return 0;
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
