/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "controls.h"
#include "hacks.h"

CMemory		Memory;
uint32		OpenBus = 0;

#define match_nn(str) (strncmp(Memory.ROMName, (str), strlen((str))) == 0)

static int First512BytesCountZeroes(void)
{
	int zeroCount = 0;
	for (int i = 0; i < 512; i++)
	{
		if (Memory.ROM[i] == 0)
			zeroCount++;
	}
	return zeroCount;
}

static void sanitize(char *b, int size)
{
	for (int pos = 0; pos < size; pos++)
	{
		if (b[pos] == 0)
			continue;
		if (b[pos] < 32 || b[pos] > 126)
			b[pos] = '_';
	}
	b[size - 1] = 0;
}

static bool8 allASCII (uint8 *b, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (b[i] < 32 || b[i] > 126)
			return (FALSE);
	}

	return (TRUE);
}

static void ShowInfoString (void)
{
	const char *contents[3] = { "ROM", "ROM+RAM", "ROM+RAM+BAT" };
	char contentstr[20], sizestr[20], sramstr[20];

	bool8 isChecksumOK = (Memory.ROMChecksum + Memory.ROMComplementChecksum == 0xffff) &
						 (Memory.ROMChecksum == Memory.CalculatedChecksum);

	if (!isChecksumOK || ((uint32) Memory.CalculatedSize > (uint32) (((1 << (Memory.ROMSize - 7)) * 128) * 1024)))
	{
		// Use yellow tint to indicate corrupted ROM.
		Settings.DisplayColor = BUILD_PIXEL(31, 31, 0);
	}
	else if (Memory.ROMIsPatched)
	{
		// Use slight blue tint to indicate ROM was patched.
		Settings.DisplayColor = BUILD_PIXEL(26, 26, 31);
	}
	else
	{
		Settings.DisplayColor = BUILD_PIXEL(31, 31, 31);
	}

	strcpy(contentstr, contents[(Memory.ROMType & 0xf) % 3]);

	if (Settings.DSP == 1)
		strcat(contentstr, "+DSP-1");
	else if (Settings.DSP == 2)
		strcat(contentstr, "+DSP-2");

	if (Memory.ROMSize < 7 || Memory.ROMSize - 7 > 23)
		strcpy(sizestr, "Corrupt");
	else
		sprintf(sizestr, "%dMbits", 1 << (Memory.ROMSize - 7));

	if (Memory.SRAMSize > 16)
		strcpy(sramstr, "Corrupt");
	else
		sprintf(sramstr, "%dKbits", 8 * (Memory.SRAMMask + 1) / 1024);

	sprintf(String, "\"%s\" [%s] %s, %s, %s, %s, SRAM:%s, ID:%s, CRC32:%08X",
		Memory.ROMName,
		isChecksumOK ? "checksum ok" : "bad checksum",
		Memory.HiROM ? "HiROM" : "LoROM",
		sizestr,
		contentstr,
		(Settings.Region == S9X_PAL) ? "PAL" : "NTSC",
		sramstr,
		Memory.ROMId,
		Memory.ROMCRC32);

	S9xMessage(S9X_INFO, S9X_ROM_INFO, String);
}

static uint32 map_mirror (uint32 size, uint32 pos)
{
	// from bsnes
	if (size == 0)
		return (0);
	if (pos < size)
		return (pos);

	uint32	mask = 1 << 31;
	while (!(pos & mask))
		mask >>= 1;

	if (size <= (pos & mask))
		return (map_mirror(size, pos - mask));
	else
		return (mask + map_mirror(size - mask, pos - mask));
}

static void map_lorom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = (c & 0x7f) * 0x8000;
			Memory.ReadMap[p] = Memory.ROM + map_mirror(size, addr) - (i & 0x8000);
		}
	}
}

static void map_hirom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = c << 16;
			Memory.ReadMap[p] = Memory.ROM + map_mirror(size, addr);
		}
	}
}

static void map_space (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint8 *data)
{
	uint32	c, i, p;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			Memory.ReadMap[p] = data;
			Memory.WriteMap[p] = data;
		}
	}
}

static void map_index (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, int index, int type)
{
	uint32	c, i, p;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			Memory.ReadMap[p] = (uint8 *) (pint) index;
			Memory.WriteMap[p] = (uint8 *) (pint) index;
		}
	}
}

static void Map_Initialize (void)
{
	for (int c = 0; c < MEMMAP_NUM_BLOCKS; c++)
	{
		Memory.ReadMap[c]  = (uint8 *) MAP_NONE;
		Memory.WriteMap[c] = (uint8 *) MAP_NONE;
	}

	// will be overwritten
	map_space(0x00, 0x3f, 0x0000, 0x1fff, Memory.RAM);
	map_index(0x00, 0x3f, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x00, 0x3f, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
	map_space(0x80, 0xbf, 0x0000, 0x1fff, Memory.RAM);
	map_index(0x80, 0xbf, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
}

static void Map_Finalize (void)
{
	// Map Work RAM
	map_space(0x7e, 0x7e, 0x0000, 0xffff, Memory.RAM);
	map_space(0x7f, 0x7f, 0x0000, 0xffff, Memory.RAM + 0x10000);
}

static void Map_DSP (void)
{
	switch (DSP0.maptype)
	{
		case M_DSP1_LOROM_S:
			map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP1_LOROM_L:
			map_index(0x60, 0x6f, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xe0, 0xef, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP1_HIROM:
			map_index(0x00, 0x1f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0x80, 0x9f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP2_LOROM:
			map_index(0x20, 0x3f, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0x20, 0x3f, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
			break;
	}
}

static void Map_LoROMMap (void)
{
	Map_Initialize();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, Memory.CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, Memory.CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, Memory.CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, Memory.CalculatedSize);

	if (Settings.DSP)
		Map_DSP();

	if (Memory.SRAMSize > 0)
	{
		uint32 hi = (Memory.ROMSize > 11 || Memory.SRAMSize > 5) ? 0x7fff : 0xffff;
		map_index(0x70, 0x7d, 0x0000, hi, MAP_LOROM_SRAM, MAP_TYPE_RAM);
		map_index(0xf0, 0xff, 0x0000, hi, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	}

	Map_Finalize();
}

static void Map_HiROMMap (void)
{
	Map_Initialize();

	map_hirom(0x00, 0x3f, 0x8000, 0xffff, Memory.CalculatedSize);
	map_hirom(0x40, 0x7f, 0x0000, 0xffff, Memory.CalculatedSize);
	map_hirom(0x80, 0xbf, 0x8000, 0xffff, Memory.CalculatedSize);
	map_hirom(0xc0, 0xff, 0x0000, 0xffff, Memory.CalculatedSize);

	if (Settings.DSP)
		Map_DSP();

	map_index(0x20, 0x3f, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	map_index(0xa0, 0xbf, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);

	Map_Finalize();
}

static uint16 checksum_calc_sum (uint8 *data, uint32 length)
{
	uint16	sum = 0;

	for (size_t i = 0; i < length; i++)
		sum += data[i];

	return (sum);
}

static uint16 checksum_mirror_sum (uint8 *start, uint32 *length, uint32 mask)
{
	// from NSRT
	while (!(*length & mask) && mask)
		mask >>= 1;

	uint16	part1 = checksum_calc_sum(start, mask);
	uint16	part2 = 0;

	uint32	next_length = *length - mask;
	if (next_length)
	{
		part2 = checksum_mirror_sum(start + mask, &next_length, mask >> 1);

		while (next_length < mask)
		{
			next_length += next_length;
			part2 += part2;
		}

		*length = mask + mask;
	}

	return (part1 + part2);
}

static uint16 CalcChecksum (uint8 *data, uint32 length)
{
	if (length & 0x7fff)
		return checksum_calc_sum(data, length);
	else
		return checksum_mirror_sum(data, &length, 0x800000);
}

static int ScoreHiROM (void)
{
	uint8	*buf = Memory.ROM + 0xff00;
	int		score = 0;

	if (buf[0xd5] & 0x1)
		score += 2;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score -= 2;

	if (buf[0xd4] == 0x20)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (0 != (buf[0xde] + (buf[0xdf] << 8)))
			score++;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced after looking at a scan by Cowering

	if (Memory.CalculatedSize > 1024 * 1024 * 3)
		score += 4;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

static int ScoreLoROM (void)
{
	uint8	*buf = Memory.ROM + 0x7f00;
	int		score = 0;

	if (!(buf[0xd5] & 0x1))
		score += 3;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (0 != (buf[0xde] + (buf[0xdf] << 8)))
			score++;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced per Cowering suggestion

	if (Memory.CalculatedSize <= 1024 * 1024 * 16)
		score += 2;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

int ApplyGameSpecificHacks (void)
{
	int applied = 0;

	for (const s9x_hack_t *hack = &GameHacks[0]; hack->checksum; hack++)
	{
		if (hack->checksum == Memory.ROMCRC32)
		{
			printf("Applying patch %s: '%s'\n", hack->name, hack->patch);

			char *ptr = (char*)hack->patch;

			while (ptr && *ptr)
			{
				int offset = strtol(ptr, &ptr, 16);
				int value = strtol(ptr + 1, &ptr, 16);

				if ((ptr = strchr(ptr, ',')))
					ptr++;

				// We only care about opcode 0x42 for now
				if ((value != 0x42) && ((value >> 8) != 0x42))
				{
					printf(" - Warning: Ignoring patch %X=%X...\n", offset, value);
				}
				else if (offset < 1 || offset >= Memory.CalculatedSize)
				{
					printf(" - Warning: Offset %d (%X) is out of range...\n", offset, offset);
				}
				else if (value & 0xFF00) // 16 bit replacement
				{
					printf(" - Applying 16bit patch %X=%X\n", offset, value);
					// Memory.ROM[offset] = (value >> 8) & 0xFF;
					// Memory.ROM[offset + 1] = value & 0xFF;
					applied++;
				}
				else // 8 bit replacement
				{
					printf(" - Applying 8bit patch %X=%X\n", offset, value);
					// Memory.ROM[offset] = value;
					applied++;
				}
			}
		}
	}

	return applied;
}

static bool8 InitROM ()
{
	if (!Memory.ROM || Memory.ROM_SIZE < 0x200)
		return (FALSE);

	if ((Memory.ROM_SIZE & 0x7FF) == 512 || First512BytesCountZeroes() > 400)
	{
		printf("Found ROM file header (and ignored it).\n");
		memmove(Memory.ROM, Memory.ROM + 512, Memory.ROM_SIZE - 512);
		Memory.ROM_SIZE -= 512;
	}

	Memory.CalculatedSize = ((Memory.ROM_SIZE + 0x1fff) / 0x2000) * 0x2000;

	//// these two games fail to be detected
	if (strncmp((char *) &Memory.ROM[0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0 ||
		(strncmp((char *) &Memory.ROM[0xffc0], "BATMAN--REVENGE JOKER",  21) == 0))
	{
		Memory.LoROM = TRUE;
		Memory.HiROM = FALSE;
	}
	else
	{
		Memory.LoROM = (ScoreLoROM() >= ScoreHiROM());
		Memory.HiROM = !Memory.LoROM;
	}

	//// Parse ROM header and read ROM informatoin

	uint8 *RomHeader = Memory.ROM + (Memory.HiROM ? 0xFFB0 : 0x7FB0);

	strncpy(Memory.ROMName, (char *) &RomHeader[0x10], ROM_NAME_LEN - 1);
	sanitize(Memory.ROMName, ROM_NAME_LEN);

	Memory.ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen(Memory.ROMName))
	{
		char *p = Memory.ROMName + strlen(Memory.ROMName);
		if (p > Memory.ROMName + 21 && Memory.ROMName[20] == ' ')
			p = Memory.ROMName + 21;
		while (p > Memory.ROMName && *(p - 1) == ' ')
			p--;
		*p = 0;
	}

	Memory.ROMSize   = RomHeader[0x27];
	Memory.SRAMSize  = RomHeader[0x28];
	Memory.ROMSpeed  = RomHeader[0x25];
	Memory.ROMType   = RomHeader[0x26];
	Memory.ROMRegion = RomHeader[0x29];
	Memory.ROMChecksum           = RomHeader[0x2E] + (RomHeader[0x2F] << 8);
	Memory.ROMComplementChecksum = RomHeader[0x2C] + (RomHeader[0x2D] << 8);

	memmove(Memory.ROMId, &RomHeader[0x02], 4);
	sanitize(Memory.ROMId, 4);

	//// Detect DSP 1 & 2
	if (Memory.ROMType == 0x03 || (Memory.ROMType == 0x05 && Memory.ROMSpeed != 0x20))
	{
		Settings.DSP = 1;
		if (Memory.HiROM)
		{
			DSP0.boundary = 0x7000;
			DSP0.maptype = M_DSP1_HIROM;
		}
		else
		if (Memory.CalculatedSize > 0x100000)
		{
			DSP0.boundary = 0x4000;
			DSP0.maptype = M_DSP1_LOROM_L;
		}
		else
		{
			DSP0.boundary = 0xc000;
			DSP0.maptype = M_DSP1_LOROM_S;
		}

		SetDSP = &DSP1SetByte;
		GetDSP = &DSP1GetByte;
	}
	else
	if (Memory.ROMType == 0x05 && Memory.ROMSpeed == 0x20)
	{
		Settings.DSP = 2;
		DSP0.boundary = 0x10000;
		DSP0.maptype = M_DSP2_LOROM;
		SetDSP = &DSP2SetByte;
		GetDSP = &DSP2GetByte;
	}
	else
	{
		Settings.DSP = 0;
		SetDSP = NULL;
		GetDSP = NULL;
	}

	//// SRAM size
	if (strncmp("HITOMI3 ", Memory.ROMName, 8) == 0)
	{
		Memory.SRAMSize = 1;
	}
	Memory.SRAMMask = Memory.SRAMSize ? ((1 << (Memory.SRAMSize + 3)) * 128) - 1 : 0;
	Memory.SRAMBytes = Memory.SRAMSize ? ((1 << (Memory.SRAMSize + 3)) * 128) : 0;
	if (Memory.SRAMBytes > 0x8000)
	{
		printf("\n\nWARNING: Default SRAM size too small!, need %d bytes\n\n", Memory.SRAMBytes);
	}

	//// Map memory
	if (Memory.HiROM)
	{
		Map_HiROMMap();
	}
    else
	{
		Map_LoROMMap();
	}

	//// Checksums
	Memory.ROMCRC32 = crc32_le(0, Memory.ROM, Memory.CalculatedSize);
	Memory.CalculatedChecksum = CalcChecksum(Memory.ROM, Memory.CalculatedSize);

	//// ROM Region
	if ((Memory.ROMRegion >= 2) && (Memory.ROMRegion <= 12))
	{
		Settings.Region = S9X_PAL;
		Settings.FrameRate = 50;
	}
	else
	{
		Settings.Region = S9X_NTSC;
		Settings.FrameRate = 60;
	}

	// Setup APU timings
	S9xAPUTimingSetSpeedup(0);

	//// Apply Game hacks
	if (!Settings.DisableGameSpecificHacks)
	{
		ApplyGameSpecificHacks();
	}

	//// Show ROM information
	ShowInfoString();

	//// Reset system, then we're ready
	S9xReset();

    return (TRUE);
}


// Public functions

bool8 S9xMemoryInit (void)
{
	memset(&Memory, 0, sizeof(Memory));

	Memory.RAM  = (uint8 *) calloc(1, 0x20000);
	Memory.VRAM = (uint8 *) calloc(1, 0x10000);
	Memory.SRAM = (uint8 *) calloc(1, 0x8000);
	Memory.ROM  = (uint8 *) calloc(1, Memory.ROM_MAX_SIZE = 0x300000);
	// Note: we don't care if ROM alloc fails. It's just to grab a large heap block
	//       before it gets fragmented. The actual useful alloc is done in S9xLoadROM()

	if (!Memory.RAM || !Memory.SRAM || !Memory.VRAM)
	{
		S9xMemoryDeinit();
		return (FALSE);
	}

	return (TRUE);
}

void S9xMemoryDeinit (void)
{
	free(Memory.RAM);
	free(Memory.SRAM);
	free(Memory.VRAM);
	free(Memory.ROM);

	Memory.RAM = NULL;
	Memory.SRAM = NULL;
	Memory.VRAM = NULL;
	Memory.ROM = NULL;
}

bool8 S9xLoadROM (const char *filename)
{
	FILE *stream = fopen(filename, "rb");
	if (!stream)
		return (FALSE);

	fseek(stream, 0, SEEK_END);
	Memory.ROM_SIZE = ftell(stream);

	// We shrink if possible because we need all the memory we can get for savestates and buffers
	uint8 *temp = (uint8 *)realloc(Memory.ROM, Memory.ROM_SIZE);
	if (temp)
	{
		Memory.ROM_MAX_SIZE = Memory.ROM_SIZE;
		Memory.ROM = temp;
	}

	if (!Memory.ROM || Memory.ROM_MAX_SIZE < Memory.ROM_SIZE)
	{
		fclose(stream);
		return (FALSE);
	}

	fseek(stream, 0, SEEK_SET);
	fread(Memory.ROM, Memory.ROM_SIZE, 1, stream);
	fclose(stream);

	return InitROM();
}

// BPS % UPS % IPS
#if 0

static long ReadInt (Stream *r, unsigned nbytes)
{
	long	v = 0;

	while (nbytes--)
	{
		int	c = r->get_char();
		if (c == EOF)
			return (-1);
		v = (v << 8) | (c & 0xFF);
	}

	return (v);
}

static bool8 ReadIPSPatch (Stream *r, long offset, int32 &rom_size)
{
	const int32	IPS_EOF = 0x00454F46l;
	int32		ofs;
	char		fname[6];

	fname[5] = 0;
	for (int i = 0; i < 5; i++)
	{
		int	c = r->get_char();
		if (c == EOF)
			return (0);
		fname[i] = (char) c;
	}

	if (strncmp(fname, "PATCH", 5))
		return (0);

	for (;;)
	{
		long	len, rlen;
		int		rchar;

		ofs = ReadInt(r, 3);
		if (ofs == -1)
			return (0);

		if (ofs == IPS_EOF)
			break;

		ofs -= offset;

		len = ReadInt(r, 2);
		if (len == -1)
			return (0);

		if (len)
		{
			if (ofs + len > Memory.ROM_SIZE)
				return (0);

			while (len--)
			{
				rchar = r->get_char();
				if (rchar == EOF)
					return (0);
				Memory.ROM[ofs++] = (uint8) rchar;
			}

			if (ofs > rom_size)
				rom_size = ofs;
		}
		else
		{
			rlen = ReadInt(r, 2);
			if (rlen == -1)
				return (0);

			rchar = r->get_char();
			if (rchar == EOF)
				return (0);

			if (ofs + rlen > Memory.ROM_SIZE)
				return (0);

			while (rlen--)
				Memory.ROM[ofs++] = (uint8) rchar;

			if (ofs > rom_size)
				rom_size = ofs;
		}
	}

	ofs = ReadInt(r, 3);
	if (ofs != -1 && ofs - offset < rom_size)
		rom_size = ofs - offset;

	ROMIsPatched = 1;
	return (1);
}

void CMemory::CheckForAnyPatch (const char *rom_filename, bool8 header, int32 &rom_size)
{
	ROMIsPatched = false;

	FSTREAM		patch_file  = NULL;
	uint32		i;
	long		offset = header ? 512 : 0;
	int			ret;
	bool		flag;
	char		dir[_MAX_DIR + 1], drive[_MAX_DRIVE + 1], name[_MAX_FNAME + 1], ext[_MAX_EXT + 1], ips[_MAX_EXT + 3], fname[PATH_MAX + 1];
	const char	*n;

	_splitpath(rom_filename, drive, dir, name, ext);

	// IPS

	_makepath(fname, drive, dir, name, "ips");

	if ((patch_file = OPEN_FSTREAM(fname, "rb")) != NULL)
	{
		printf("Using IPS patch %s", fname);

        Stream *s = new fStream(patch_file);
		ret = ReadIPSPatch(s, offset, rom_size);
        s->closeStream();

		if (ret)
		{
			printf("!\n");
			return;
		}
		else
			printf(" failed!\n");
	}

	if (_MAX_EXT > 6)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, 8, "%03d.ips", i);
			_makepath(fname, drive, dir, name, ips);

			if (!(patch_file = OPEN_FSTREAM(fname, "rb")))
				break;

			printf("Using IPS patch %s", fname);

            Stream *s = new fStream(patch_file);
			ret = ReadIPSPatch(s, offset, rom_size);
            s->closeStream();

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i < 1000);

		if (flag)
			return;
	}

	if (_MAX_EXT > 3)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, _MAX_EXT + 2, "ips%d", i);
			if (strlen(ips) > _MAX_EXT)
				break;
			_makepath(fname, drive, dir, name, ips);

			if (!(patch_file = OPEN_FSTREAM(fname, "rb")))
				break;

			printf("Using IPS patch %s", fname);

            Stream *s = new fStream(patch_file);
			ret = ReadIPSPatch(s, offset, rom_size);
            s->closeStream();

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i != 0);

		if (flag)
			return;
	}

	if (_MAX_EXT > 2)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, 4, "ip%d", i);
			_makepath(fname, drive, dir, name, ips);

			if (!(patch_file = OPEN_FSTREAM(fname, "rb")))
				break;

			printf("Using IPS patch %s", fname);

            Stream *s = new fStream(patch_file);
			ret = ReadIPSPatch(s, offset, rom_size);
            s->closeStream();

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i < 10);

		if (flag)
			return;
	}

	n = S9xGetFilename(".ips", PATCH_DIR);

	if ((patch_file = OPEN_FSTREAM(n, "rb")) != NULL)
	{
		printf("Using IPS patch %s", n);

        Stream *s = new fStream(patch_file);
		ret = ReadIPSPatch(s, offset, rom_size);
        s->closeStream();

		if (ret)
		{
			printf("!\n");
			return;
		}
		else
			printf(" failed!\n");
	}

	if (_MAX_EXT > 6)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, 9, ".%03d.ips", i);
			n = S9xGetFilename(ips, PATCH_DIR);

			if (!(patch_file = OPEN_FSTREAM(n, "rb")))
				break;

			printf("Using IPS patch %s", n);

            Stream *s = new fStream(patch_file);
			ret = ReadIPSPatch(s, offset, rom_size);
            s->closeStream();

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i < 1000);

		if (flag)
			return;
	}

	if (_MAX_EXT > 3)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, _MAX_EXT + 3, ".ips%d", i);
			if (strlen(ips) > _MAX_EXT + 1)
				break;
			n = S9xGetFilename(ips, PATCH_DIR);

			if (!(patch_file = OPEN_FSTREAM(n, "rb")))
				break;

			printf("Using IPS patch %s", n);

            Stream *s = new fStream(patch_file);
			ret = ReadIPSPatch(s, offset, rom_size);
            s->closeStream();

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i != 0);

		if (flag)
			return;
	}

	if (_MAX_EXT > 2)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, 5, ".ip%d", i);
			n = S9xGetFilename(ips, PATCH_DIR);

			if (!(patch_file = OPEN_FSTREAM(n, "rb")))
				break;

			printf("Using IPS patch %s", n);

            Stream *s = new fStream(patch_file);
			ret = ReadIPSPatch(s, offset, rom_size);
            s->closeStream();

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i < 10);

		if (flag)
			return;
	}
}

#endif
