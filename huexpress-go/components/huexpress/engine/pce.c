// pce.c - Entry file to start/stop/reset/save emulation
//
#include "pce.h"
#include "romdb.h"

struct host_machine host;

const char SAVESTATE_HEADER[8] = "PCE_V003";

/**
 * Describes what is saved in a save state. Changing the order will break
 * previous saves so add a place holder if necessary. Eventually we could use
 * the keys to make order irrelevant...
 */

const svar_t SaveStateVars[] =
{
	// Arrays
	SVAR_A("RAM", PCE.RAM),      SVAR_A("VRAM", PCE.VRAM),  SVAR_A("SPRAM", PCE.SPRAM),
	SVAR_A("PAL", PCE.Palette),  SVAR_A("MMR", PCE.MMR),

	// CPU registers
	SVAR_2("CPU.PC", reg_pc),    SVAR_1("CPU.A", reg_a),    SVAR_1("CPU.X", reg_x),
	SVAR_1("CPU.Y", reg_y),      SVAR_1("CPU.P", reg_p),    SVAR_1("CPU.S", reg_s),

	// Misc
	SVAR_2("Cycles", Cycles),                   SVAR_1("SF2", PCE.SF2),

	// IRQ
	SVAR_1("irq_mask", PCE.irq_mask),           SVAR_1("irq_status", PCE.irq_status),

	// PSG
	SVAR_A("PSG", PCE.PSG.regs),                SVAR_A("PSG_WAVE", PCE.PSG.wave),
	SVAR_A("psg_da_data", PCE.PSG.da_data),     SVAR_A("psg_da_count", PCE.PSG.da_count),
	SVAR_A("psg_da_index", PCE.PSG.da_index),   SVAR_1("psg_ch", PCE.PSG.ch),
	SVAR_1("psg_volume", PCE.PSG.volume),       SVAR_1("psg_lfo_freq", PCE.PSG.lfo_freq),
	SVAR_1("psg_lfo_ctrl", PCE.PSG.lfo_ctrl),

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
	PCE.ROM = osd_alloc(fsize);

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

	uint IDX = 0;
	uint ROM_MASK = 1;

	while (ROM_MASK < PCE.ROM_SIZE) ROM_MASK <<= 1;
	ROM_MASK--;

	MESSAGE_INFO("ROM LOADED: OFFSET=%d, BANKS=%d, MASK=%03X, CRC=%08X\n",
		offset, PCE.ROM_SIZE, ROM_MASK, PCE.ROM_CRC);

	for (int index = 0; index < KNOWN_ROM_COUNT; index++) {
		if (PCE.ROM_CRC == romFlags[index].CRC) {
			IDX = index;
			break;
		}
	}

	MESSAGE_INFO("Game Name: %s\n", romFlags[IDX].Name);
	MESSAGE_INFO("Game Region: %s\n", (romFlags[IDX].Flags & JAP) ? "Japan" : "USA");

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
				MemoryMapR[i] = PCE.ROM_DATA + (i & ROM_MASK) * 0x2000;
				break;
			case 0x20:
			case 0x60:
				MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x20) & ROM_MASK) * 0x2000;
				break;
			case 0x30:
			case 0x70:
				MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x10) & ROM_MASK) * 0x2000;
				break;
			case 0x40:
				MemoryMapR[i] = PCE.ROM_DATA + ((i - 0x20) & ROM_MASK) * 0x2000;
				break;
			}
		} else {
			MemoryMapR[i] = PCE.ROM_DATA + (i & ROM_MASK) * 0x2000;
		}
		MemoryMapW[i] = PCE.NULLRAM;
	}

	// Allocate the card's onboard ram
	if (romFlags[IDX].Flags & ONBOARD_RAM) {
		PCE.ExRAM = PCE.ExRAM ?: osd_alloc(0x8000);
		MemoryMapR[0x40] = MemoryMapW[0x40] = PCE.ExRAM;
		MemoryMapR[0x41] = MemoryMapW[0x41] = PCE.ExRAM + 0x2000;
		MemoryMapR[0x42] = MemoryMapW[0x42] = PCE.ExRAM + 0x4000;
		MemoryMapR[0x43] = MemoryMapW[0x43] = PCE.ExRAM + 0x6000;
	}

	// Mapper for roms >= 1.5MB (SF2, homebrews)
	if (PCE.ROM_SIZE >= 192)
		MemoryMapW[0x00] = PCE.IOAREA;

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
		pce_bank_set(i, PCE.MMR[i]);
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
