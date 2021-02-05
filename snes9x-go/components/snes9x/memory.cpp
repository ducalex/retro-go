/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <string>
#include "snes9x.h"
#include "memory.h"
#include "apu/apu.h"
#include "controls.h"
#include "display.h"

CMemory		Memory;
uint32		OpenBus = 0;


extern uint32 crc32_le(uint32 crc, uint8 const * buf, uint32 len);

static int First512BytesCountZeroes(const uint8 *buf)
{
	int zeroCount = 0;
	for (int i = 0; i < 512; i++)
	{
		if (buf[i] == 0)
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

#define match_nn(str) (strncmp(Memory.ROMName, (str), strlen((str))) == 0)

// allocation and deallocation

bool8 CMemory::Init (void)
{
	FillRAM = (uint8 *) calloc(1, 0x2800);
    RAM	 = (uint8 *) calloc(1, 0x20000);
    VRAM = (uint8 *) calloc(1, 0x10000);
    SRAM = (uint8 *) calloc(1, 0x8000);
    ROM  = (uint8 *) calloc(1, ROM_BUFFER_SIZE + 0x200);

	IPPU.TileCacheData = (uint8 *) calloc(4096, 64);

	if (!FillRAM || !RAM || !SRAM || !VRAM || !ROM || !IPPU.TileCacheData)
    {
		Deinit();
		return (FALSE);
    }

	// We never access 0x0000 - 0x2000 so we shift our offset to save 8K of ram
	FillRAM -= 0x2000;

	return (TRUE);
}

void CMemory::Deinit (void)
{
	if (FillRAM)
	{
		FillRAM += 0x2000;
		free(FillRAM);
		FillRAM = NULL;
	}

	free(RAM);
	free(SRAM);
	free(VRAM);
	free(ROM);
	free(IPPU.TileCacheData);

	RAM = NULL;
	SRAM = NULL;
	VRAM = NULL;
	ROM = NULL;
	IPPU.TileCacheData = NULL;
}

// file management and ROM detection

int CMemory::ScoreHiROM (bool8 skip_header, int32 romoff)
{
	uint8	*buf = ROM + 0xff00 + romoff + (skip_header ? 0x200 : 0);
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

	if (CalculatedSize > 1024 * 1024 * 3)
		score += 4;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

int CMemory::ScoreLoROM (bool8 skip_header, int32 romoff)
{
	uint8	*buf = ROM + 0x7f00 + romoff + (skip_header ? 0x200 : 0);
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

	if (CalculatedSize <= 1024 * 1024 * 16)
		score += 2;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

bool8 CMemory::LoadROMMem (const uint8 *source, uint32 sourceSize)
{
	if (source == 0 || sourceSize > ROM_BUFFER_SIZE)
		return FALSE;

	memset(ROM, 0, ROM_BUFFER_SIZE);
	memcpy(ROM, source, sourceSize);

	ROM_SIZE = sourceSize;

	return InitROM();
}

bool8 CMemory::LoadROM (const char *filename)
{
	STREAM stream = OPEN_STREAM(filename, "rb");
	if (!stream)
		return (FALSE);

	REVERT_STREAM(stream, 0, SEEK_END);

	ROM_SIZE = FIND_STREAM(stream);

	REVERT_STREAM(stream, 0, SEEK_SET);
	READ_STREAM(ROM, ROM_BUFFER_SIZE + 0x200, stream);

	CLOSE_STREAM(stream);

	return InitROM();
}

bool8 CMemory::LoadROMBlock (int b)
{
	return FALSE;
}

bool8 CMemory::InitROM ()
{
	if (!ROM || ROM_SIZE < 0x200)
		return (FALSE);

	Settings.DisplayColor = BUILD_PIXEL(31, 31, 31);
	SET_UI_COLOR(255, 255, 255);

	if ((ROM_SIZE & 0x7FF) == 512 || First512BytesCountZeroes(ROM) > 400)
	{
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "Found ROM file header (and ignored it).");
		memmove(ROM, ROM + 512, ROM_BUFFER_SIZE - 512);
		ROM_SIZE -= 512;
	}

	CalculatedSize = ((ROM_SIZE + 0x1fff) / 0x2000) * 0x2000;

	// these two games fail to be detected
	if (strncmp((char *) &ROM[0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0 ||
		(strncmp((char *) &ROM[0xffc0], "BATMAN--REVENGE JOKER",  21) == 0))
	{
		LoROM = TRUE;
		HiROM = FALSE;
	}
	else
	{
		LoROM = (ScoreLoROM(FALSE) >= ScoreHiROM(FALSE));
		HiROM = !LoROM;
	}

	//// Parse ROM header and read ROM informatoin
	ParseSNESHeader(ROM + (HiROM ? 0xFFB0 : 0x7FB0));

	// Detect DSP 1 & 2
	if (ROMType == 0x03 || (ROMType == 0x05 && ROMSpeed != 0x20))
	{
		Settings.DSP = 1;
		if (HiROM)
		{
			DSP0.boundary = 0x7000;
			DSP0.maptype = M_DSP1_HIROM;
		}
		else
		if (CalculatedSize > 0x100000)
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
	if (ROMType == 0x05 && ROMSpeed == 0x20)
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

	//// Map memory
	if (HiROM)
		Map_HiROMMap();
    else
		Map_LoROMMap();

	// ROM Region
	Settings.PAL = (Settings.ForcePAL && !Settings.ForceNTSC)
						|| ((ROMRegion >= 2) && (ROMRegion <= 12));

	if (Settings.PAL)
	{
		Settings.FrameTime = 20000;
		Settings.FrameRate = 50;
	}
	else
	{
		Settings.FrameTime = 16667;
		Settings.FrameRate = 60;
	}

	// SRAM size
	SRAMMask = SRAMSize ? ((1 << (SRAMSize + 3)) * 128) - 1 : 0;
	SRAMBytes = SRAMSize ? ((1 << (SRAMSize + 3)) * 128) : 0;

	if (SRAMBytes > 0x8000)
	{
		printf("\n\nWARNING: Default SRAM size too small!, need %d bytes\n\n", SRAMBytes);
	}

	// Checksums
	Checksum_Calculate();

	bool8 isChecksumOK = (ROMChecksum + ROMComplementChecksum == 0xffff) &
						 (ROMChecksum == CalculatedChecksum);

	ROMCRC32 = crc32_le(0, ROM, CalculatedSize);

	if (!isChecksumOK || ((uint32) CalculatedSize > (uint32) (((1 << (ROMSize - 7)) * 128) * 1024)))
	{
		Settings.DisplayColor = BUILD_PIXEL(31, 31, 0);
		SET_UI_COLOR(255, 255, 0);
	}

	ROMIsPatched = false;

	// CheckAnyPatch();

	// Use slight blue tint to indicate ROM was patched.
	if (ROMIsPatched)
	{
		Settings.DisplayColor = BUILD_PIXEL(26, 26, 31);
		SET_UI_COLOR(216, 216, 255);
	}

	//// Hack games
	ApplyROMFixes();

	//// Show ROM information
	sprintf(String, "\"%s\" [%s] %s, %s, %s, %s, SRAM:%s, ID:%s, CRC32:%08X",
		ROMName, isChecksumOK ? "checksum ok" : "bad checksum",
		MapType(), Size(), KartContents(), Settings.PAL ? "PAL" : "NTSC", StaticRAMSize(), ROMId, ROMCRC32);
	S9xMessage(S9X_INFO, S9X_ROM_INFO, String);

	// Reset system, then we're ready
	S9xReset();

    return (TRUE);
}

void CMemory::ClearSRAM (bool8 onlyNonSavedSRAM)
{
	if (SRAMBytes > 0)
		memset(SRAM, 0x00, SRAMBytes);
}

bool8 CMemory::LoadSRAM (const char *filename)
{
	return (TRUE);
}

bool8 CMemory::SaveSRAM (const char *filename)
{
	return (FALSE);
}

// initialization

void CMemory::ParseSNESHeader (uint8 *RomHeader)
{
	strncpy(ROMName, (char *) &RomHeader[0x10], ROM_NAME_LEN - 1);
	sanitize(ROMName, ROM_NAME_LEN);

	ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen(ROMName))
	{
		char *p = ROMName + strlen(ROMName);
		if (p > ROMName + 21 && ROMName[20] == ' ')
			p = ROMName + 21;
		while (p > ROMName && *(p - 1) == ' ')
			p--;
		*p = 0;
	}

	ROMSize   = RomHeader[0x27];
	SRAMSize  = RomHeader[0x28];
	ROMSpeed  = RomHeader[0x25];
	ROMType   = RomHeader[0x26];
	ROMRegion = RomHeader[0x29];

	ROMChecksum           = RomHeader[0x2E] + (RomHeader[0x2F] << 8);
	ROMComplementChecksum = RomHeader[0x2C] + (RomHeader[0x2D] << 8);

	memmove(ROMId, &RomHeader[0x02], 4);
	sanitize(ROMId, 4);

	CompanyId = -1;

	if (RomHeader[0x2A] != 0x33)
		CompanyId = ((RomHeader[0x2A] >> 4) & 0x0F) * 36 + (RomHeader[0x2A] & 0x0F);
	else
	if (isalnum(RomHeader[0x00]) && isalnum(RomHeader[0x01]))
	{
		int	l, r, l2, r2;
		l = toupper(RomHeader[0x00]);
		r = toupper(RomHeader[0x01]);
		l2 = (l > '9') ? l - '7' : l - '0';
		r2 = (r > '9') ? r - '7' : r - '0';
		CompanyId = l2 * 36 + r2;
	}
}

// memory map

uint32 CMemory::map_mirror (uint32 size, uint32 pos)
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

void CMemory::map_lorom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = (c & 0x7f) * 0x8000;
			ReadMap[p] = ROM + map_mirror(size, addr) - (i & 0x8000);
		}
	}
}

void CMemory::map_hirom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = c << 16;
			ReadMap[p] = ROM + map_mirror(size, addr);
		}
	}
}

void CMemory::map_space (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint8 *data)
{
	uint32	c, i, p;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			ReadMap[p] = data;
		}
	}
}

void CMemory::map_index (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, int index, int type)
{
	uint32	c, i, p;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			ReadMap[p] = (uint8 *) (pint) index;
		}
	}
}

void CMemory::Map_Initialize (void)
{
	for (int c = 0; c < MEMMAP_NUM_BLOCKS; c++)
	{
		ReadMap[c]  = (uint8 *) MAP_NONE;
		WriteMap[c] = (uint8 *) MAP_NONE;
	}
}

void CMemory::Map_WriteProtectROM (void)
{
	memmove((void *) WriteMap, (void *) ReadMap, sizeof(ReadMap));

	for (int c = 0; c < MEMMAP_NUM_BLOCKS; c++)
	{
		if (WriteMap[c] >= ROM && WriteMap[c] <= (ROM + ROM_SIZE + 0x8000))
		{
			WriteMap[c] = (uint8 *) MAP_NONE;
		}
	}
}

void CMemory::Map_System (void)
{
	// will be overwritten
	map_space(0x00, 0x3f, 0x0000, 0x1fff, RAM);
	map_index(0x00, 0x3f, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x00, 0x3f, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
	map_space(0x80, 0xbf, 0x0000, 0x1fff, RAM);
	map_index(0x80, 0xbf, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
}

void CMemory::Map_WRAM (void)
{
	// will overwrite others
	map_space(0x7e, 0x7e, 0x0000, 0xffff, RAM);
	map_space(0x7f, 0x7f, 0x0000, 0xffff, RAM + 0x10000);
}

void CMemory::Map_LoROMSRAM (void)
{
	uint32 hi;

	if (SRAMSize == 0)
		return;

	if (ROMSize > 11 || SRAMSize > 5)
		hi = 0x7fff;
	else
		hi = 0xffff;

	map_index(0x70, 0x7d, 0x0000, hi, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	map_index(0xf0, 0xff, 0x0000, hi, MAP_LOROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::Map_HiROMSRAM (void)
{
	map_index(0x20, 0x3f, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	map_index(0xa0, 0xbf, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::Map_LoROMMap (void)
{
	printf("Map_LoROMMap\n");
	Map_Initialize();
	Map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	if (Settings.DSP)
		Map_DSP();

    Map_LoROMSRAM();
	Map_WRAM();

	Map_WriteProtectROM();
}

void CMemory::Map_HiROMMap (void)
{
	printf("Map_HiROMMap\n");
	Map_Initialize();
	Map_System();

	map_hirom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_hirom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	if (Settings.DSP)
		Map_DSP();

	Map_HiROMSRAM();
	Map_WRAM();

	Map_WriteProtectROM();
}

void CMemory::Map_DSP (void)
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

// checksum

uint16 CMemory::checksum_calc_sum (uint8 *data, uint32 length)
{
	uint16	sum = 0;

	for (size_t i = 0; i < length; i++)
		sum += data[i];

	return (sum);
}

uint16 CMemory::checksum_mirror_sum (uint8 *start, uint32 &length, uint32 mask)
{
	// from NSRT
	while (!(length & mask) && mask)
		mask >>= 1;

	uint16	part1 = checksum_calc_sum(start, mask);
	uint16	part2 = 0;

	uint32	next_length = length - mask;
	if (next_length)
	{
		part2 = checksum_mirror_sum(start + mask, next_length, mask >> 1);

		while (next_length < mask)
		{
			next_length += next_length;
			part2 += part2;
		}

		length = mask + mask;
	}

	return (part1 + part2);
}

void CMemory::Checksum_Calculate (void)
{
	// from NSRT
	uint16	sum = 0;

	if (CalculatedSize & 0x7fff)
		sum = checksum_calc_sum(ROM, CalculatedSize);
	else
	{
		uint32	length = CalculatedSize;
		sum = checksum_mirror_sum(ROM, length);
	}

	CalculatedChecksum = sum;
}

// information

const char * CMemory::MapType (void)
{
	return (HiROM ? "HiROM" : "LoROM");
}

const char * CMemory::StaticRAMSize (void)
{
	static char	str[20];

	if (SRAMSize > 16)
		strcpy(str, "Corrupt");
	else
		sprintf(str, "%dKbits", 8 * (SRAMMask + 1) / 1024);

	return (str);
}

const char * CMemory::Size (void)
{
	static char	str[20];

	if (ROMSize < 7 || ROMSize - 7 > 23)
		strcpy(str, "Corrupt");
	else
		sprintf(str, "%dMbits", 1 << (ROMSize - 7));

	return (str);
}

const char * CMemory::Revision (void)
{
	static char	str[20];

	sprintf(str, "1.%d", HiROM ? ROM[0xffdb] : ROM[0x7fdb]);

	return (str);
}

const char * CMemory::KartContents (void)
{
	static char	str[64];
	const char	*contents[3] = { "ROM", "ROM+RAM", "ROM+RAM+BAT" };

	strcpy(str, contents[(ROMType & 0xf) % 3]);

	if (Settings.DSP == 1)
		strcat(str, "+DSP-1");
	else if (Settings.DSP == 2)
		strcat(str, "+DSP-2");

	return (str);
}

const char * CMemory::Country (void)
{
	switch (ROMRegion)
	{
		case 0:		return("Japan");
		case 1:		return("USA and Canada");
		case 2:		return("Oceania, Europe and Asia");
		case 3:		return("Sweden");
		case 4:		return("Finland");
		case 5:		return("Denmark");
		case 6:		return("France");
		case 7:		return("Holland");
		case 8:		return("Spain");
		case 9:		return("Germany, Austria and Switzerland");
		case 10:	return("Italy");
		case 11:	return("Hong Kong and China");
		case 12:	return("Indonesia");
		case 13:	return("South Korea");
		default:	return("Unknown");
	}
}

const char * CMemory::PublishingCompany (void)
{
	return ("Unknown");
}

void CMemory::MakeRomInfoText (char *romtext)
{
	char	temp[256];

	romtext[0] = 0;

	sprintf(temp,   "            Cart Name: %s", ROMName);
	strcat(romtext, temp);
	sprintf(temp, "\n            Game Code: %s", ROMId);
	strcat(romtext, temp);
	sprintf(temp, "\n             Contents: %s", KartContents());
	strcat(romtext, temp);
	sprintf(temp, "\n                  Map: %s", MapType());
	strcat(romtext, temp);
	sprintf(temp, "\n                Speed: 0x%02X (%s)", ROMSpeed, (ROMSpeed & 0x10) ? "FastROM" : "SlowROM");
	strcat(romtext, temp);
	sprintf(temp, "\n                 Type: 0x%02X", ROMType);
	strcat(romtext, temp);
	sprintf(temp, "\n    Size (calculated): %dMbits", CalculatedSize / 0x20000);
	strcat(romtext, temp);
	sprintf(temp, "\n        Size (header): %s", Size());
	strcat(romtext, temp);
	sprintf(temp, "\n            SRAM size: %s", StaticRAMSize());
	strcat(romtext, temp);
	sprintf(temp, "\nChecksum (calculated): 0x%04X", CalculatedChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n    Checksum (header): 0x%04X", ROMChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n  Complement (header): 0x%04X", ROMComplementChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n         Video Output: %s", (ROMRegion > 12 || ROMRegion < 2) ? "NTSC 60Hz" : "PAL 50Hz");
	strcat(romtext, temp);
	sprintf(temp, "\n             Revision: %s", Revision());
	strcat(romtext, temp);
	sprintf(temp, "\n             Licensee: %s", PublishingCompany());
	strcat(romtext, temp);
	sprintf(temp, "\n               Region: %s", Country());
	strcat(romtext, temp);
	sprintf(temp, "\n                CRC32: 0x%08X", ROMCRC32);
	strcat(romtext, temp);
}

// hack

void CMemory::ApplyROMFixes (void)
{
	Settings.UniracersHack = FALSE;
	Settings.DMACPUSyncHack = FALSE;

	//// Warnings

	// Reject strange hacked games
	if ((ROMCRC32 == 0x6810aa95) ||
		(ROMCRC32 == 0x340f23e5) ||
		(ROMCRC32 == 0x77fd806a) ||
		(match_nn("HIGHWAY BATTLE 2")) ||
		(match_nn("FX SKIING NINTENDO 96") && (ROM[0x7fda] == 0)) ||
		(match_nn("HONKAKUHA IGO GOSEI")   && (ROM[0xffd5] != 0x31)))
	{
		Settings.DisplayColor = BUILD_PIXEL(31, 0, 0);
		SET_UI_COLOR(255, 0, 0);
	}

	// Always exec because it sets PAL mode too
	S9xAPUTimingSetSpeedup(0);

	if (!Settings.DisableGameSpecificHacks)
	{
		//// APU timing hacks :(
		if (match_nn("CIRCUIT USA "))
			S9xAPUTimingSetSpeedup(3);

		// SRAM not correctly detected
		if (match_nn("HITOMI3 "))
		{
			SRAMSize = 1;
			SRAMMask = ((1 << (SRAMSize + 3)) * 128) - 1;
		}

		// The delay to sync CPU and DMA which Snes9x cannot emulate.
		// Some games need really severe delay timing...
		Settings.DMACPUSyncHack = (match_nn("BATTLE GRANDPRIX") || match_nn("KORYU NO MIMI ENG"));

		// OAM hacks because we don't fully understand the behavior of the SNES.
		// Totally wacky display in 2P mode...
		// seems to need a disproven behavior, so we're definitely overlooking some other bug?
		Settings.UniracersHack = match_nn("UNIRACERS");
	}
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
			if (ofs + len > CMemory::ROM_MAX_SIZE)
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

			if (ofs + rlen > CMemory::ROM_MAX_SIZE)
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