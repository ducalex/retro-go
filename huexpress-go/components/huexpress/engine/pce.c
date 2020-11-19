// pce.c - Entry file to start/stop/reset/save emulation
//
#include "pce.h"
#include "romdb.h"

struct host_machine host;

const char SAVESTATE_HEADER[8] = "PCE_V002";

/**
 * Describes what is saved in a save state. Changing the order will break
 * previous saves so add a place holder if necessary. Eventually we could use
 * the keys to make order irrelevant...
 */

const svar_t SaveStateVars[] =
{
	// Arrays
	SVAR_A("RAM", RAM),       SVAR_A("BRAM", BackupRAM), SVAR_A("VRAM", VRAM),
	SVAR_A("SPRAM", SPRAM),   SVAR_A("PAL", Palette),    SVAR_A("MMR", MMR),

	// CPU registers
	SVAR_2("CPU.PC", reg_pc), SVAR_1("CPU.A", reg_a),    SVAR_1("CPU.X", reg_x),
	SVAR_1("CPU.Y", reg_y),   SVAR_1("CPU.P", reg_p),    SVAR_1("CPU.S", reg_s),

	// Misc
	SVAR_2("Cycles", Cycles),                  SVAR_1("SF2", SF2),

	// PSG
	SVAR_A("PSG", io.PSG),                     SVAR_A("PSG_WAVE", io.PSG_WAVE),
	SVAR_A("psg_da_data", io.psg_da_data),     SVAR_A("psg_da_count", io.psg_da_count),
	SVAR_A("psg_da_index", io.psg_da_index),   SVAR_1("psg_ch", io.psg_ch),
	SVAR_1("psg_volume", io.psg_volume),       SVAR_1("psg_lfo_freq", io.psg_lfo_freq),
	SVAR_1("psg_lfo_ctrl", io.psg_lfo_ctrl),

	// IO
	SVAR_A("VCE", io.VCE),                     SVAR_2("vce_reg", io.vce_reg),

	SVAR_A("VDC", io.VDC),                     SVAR_1("vdc_reg", io.vdc_reg),
	SVAR_1("vdc_status", io.vdc_status),       SVAR_1("vdc_satb", io.vdc_satb),
	SVAR_4("vdc_irq_queue", io.vdc_irq_queue),

	SVAR_1("timer_reload", io.timer_reload),   SVAR_1("timer_running", io.timer_running),
	SVAR_1("timer_counter", io.timer_counter), SVAR_4("timer_next", io.timer_cycles_counter),

	SVAR_1("irq_mask", io.irq_mask),           SVAR_1("irq_status", io.irq_status),

	SVAR_END
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

	if (ROM != NULL) {
		free(ROM);
	}

	// find file size
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	offset = fsize & 0x1fff;

	// read ROM
	ROM = osd_alloc(fsize);

	if (ROM == NULL)
	{
		MESSAGE_ERROR("Failed to allocate ROM buffer!\n");
		return -1;
	}

	fseek(fp, 0, SEEK_SET);
	fread(ROM, 1, fsize, fp);

	fclose(fp);

	ROM_SIZE = (fsize - offset) / 0x2000;
	ROM_PTR = ROM + offset;
	ROM_CRC = crc32_le(0, ROM, fsize);

	uint IDX = 0;
	uint ROM_MASK = 1;

	while (ROM_MASK < ROM_SIZE) ROM_MASK <<= 1;
	ROM_MASK--;

	MESSAGE_INFO("ROM LOADED: OFFSET=%d, BANKS=%d, MASK=%03X, CRC=%08X\n",
		offset, ROM_SIZE, ROM_MASK, ROM_CRC);

	for (int index = 0; index < KNOWN_ROM_COUNT; index++) {
		if (ROM_CRC == romFlags[index].CRC) {
			IDX = index;
			break;
		}
	}

	MESSAGE_INFO("Game Name: %s\n", romFlags[IDX].Name);
	MESSAGE_INFO("Game Region: %s\n", (romFlags[IDX].Flags & JAP) ? "Japan" : "USA");

	// US Encrypted
	if ((romFlags[IDX].Flags & US_ENCODED) || ROM_PTR[0x1FFF] < 0xE0)
	{
		MESSAGE_INFO("This rom is probably US encrypted, decrypting...\n");

		unsigned char inverted_nibble[16] = {
			0, 8, 4, 12, 2, 10, 6, 14,
			1, 9, 5, 13, 3, 11, 7, 15
		};

		for (int x = 0; x < ROM_SIZE * 0x2000; x++) {
			unsigned char temp = ROM_PTR[x] & 15;

			ROM_PTR[x] &= ~0x0F;
			ROM_PTR[x] |= inverted_nibble[ROM_PTR[x] >> 4];

			ROM_PTR[x] &= ~0xF0;
			ROM_PTR[x] |= inverted_nibble[temp] << 4;
		}
	}

	// For example with Devil Crush 512Ko
	if (romFlags[IDX].Flags & TWO_PART_ROM)
		ROM_SIZE = 0x30;

	// Game ROM
	for (int i = 0; i < 0x80; i++) {
		if (ROM_SIZE == 0x30) {
			switch (i & 0x70) {
			case 0x00:
			case 0x10:
			case 0x50:
				MemoryMapR[i] = ROM_PTR + (i & ROM_MASK) * 0x2000;
				break;
			case 0x20:
			case 0x60:
				MemoryMapR[i] = ROM_PTR + ((i - 0x20) & ROM_MASK) * 0x2000;
				break;
			case 0x30:
			case 0x70:
				MemoryMapR[i] = ROM_PTR + ((i - 0x10) & ROM_MASK) * 0x2000;
				break;
			case 0x40:
				MemoryMapR[i] = ROM_PTR + ((i - 0x20) & ROM_MASK) * 0x2000;
				break;
			}
		} else {
			MemoryMapR[i] = ROM_PTR + (i & ROM_MASK) * 0x2000;
		}
		MemoryMapW[i] = NULLRAM;
	}

	// Allocate the card's onboard ram
	if (romFlags[IDX].Flags & ONBOARD_RAM) {
		ExtraRAM = ExtraRAM ?: osd_alloc(0x8000);
		MemoryMapR[0x40] = MemoryMapW[0x40] = ExtraRAM;
		MemoryMapR[0x41] = MemoryMapW[0x41] = ExtraRAM + 0x2000;
		MemoryMapR[0x42] = MemoryMapW[0x42] = ExtraRAM + 0x4000;
		MemoryMapR[0x43] = MemoryMapW[0x43] = ExtraRAM + 0x6000;
	}

	// Mapper for roms >= 1.5MB (SF2, homebrews)
	if (ROM_SIZE >= 192)
		MemoryMapW[0x00] = IOAREA;

	return 0;
}


/**
 * Reset the emulator
 */
void
ResetPCE(bool hard)
{
	gfx_clear_cache();
	pce_reset();
}


/**
 * Initialize the emulator (allocate memory, call osd_init* functions)
 */
int
InitPCE(const char *name)
{
	if (osd_input_init())
		return 1;

	if (gfx_init())
		return 1;

	if (snd_init())
		return 1;

	if (pce_init())
		return 1;

	if (LoadCard(name))
		return 1;

	ResetPCE(0);

	return 0;
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
LoadState(char *name)
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
		pce_bank_set(i, MMR[i]);
	}

	gfx_clear_cache();

	osd_gfx_set_mode(IO_VDC_SCREEN_WIDTH, IO_VDC_SCREEN_HEIGHT);

	fclose(fp);

	return 0;
}


/**
 * Save current state
 */
int
SaveState(char *name)
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
	snd_term();
	pce_term();

	exit(0);
}
