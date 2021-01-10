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

struct CMemory
{
	enum
	{ MAX_ROM_SIZE = 0x200000 };

	enum file_formats
	{ FILE_ZIP, FILE_JMA, FILE_DEFAULT };

	enum
	{ NOPE, YEAH, BIGFIRST, SMALLFIRST };

	enum
	{ MAP_TYPE_I_O, MAP_TYPE_ROM, MAP_TYPE_RAM };

	enum
	{
		MAP_CPU,
		MAP_PPU,
		MAP_LOROM_SRAM,
		MAP_HIROM_SRAM,
		MAP_DSP,
		MAP_BWRAM,
		MAP_BWRAM_BITMAP,
		MAP_BWRAM_BITMAP2,
		MAP_RONLY_SRAM,
		MAP_C4,
		MAP_NONE,
		MAP_LAST
	};

	uint8	NSRTHeader[32];
	int32	HeaderCount;

	uint8	*RAM;
	uint8	*ROM;
	uint8	*SRAM;
	uint8	*VRAM;
	uint8	*FillRAM;
	uint8	*BWRAM;
	uint8	*C4RAM;

	uint8	*Map[MEMMAP_NUM_BLOCKS];		// 16384
	uint8	*WriteMap[MEMMAP_NUM_BLOCKS];	// 16384
	uint8	BlockType[MEMMAP_NUM_BLOCKS];	// 4096
	uint8	ExtendedFormat;

	char	ROMFilename[PATH_MAX + 1];
	char	ROMName[ROM_NAME_LEN];
	char	ROMId[5];
	int32	CompanyId;
	uint8	ROMRegion;
	uint8	ROMSpeed;
	uint8	ROMType;
	uint8	ROMSize;
	uint32	ROMChecksum;
	uint32	ROMComplementChecksum;
	uint32	ROMCRC32;
	int32	ROMFramesPerSecond;

	bool8	HiROM;
	bool8	LoROM;
	uint8	SRAMSize;
	uint32	SRAMMask;
	uint32	CalculatedSize;
	uint32	CalculatedChecksum;

	bool8	Init (void);
	void	Deinit (void);

	int		ScoreHiROM (bool8, int32 romoff = 0);
	int		ScoreLoROM (bool8, int32 romoff = 0);
	int		First512BytesCountZeroes() const;
	uint32	HeaderRemove (uint32, uint8 *);
	uint32	FileLoader (uint8 *, const char *, uint32);
    bool8   LoadROMMem (const uint8 *, uint32);
	bool8	LoadROM (const char *);
    bool8	LoadROMInt (int32);
	bool8	LoadSRAM (const char *);
	bool8	SaveSRAM (const char *);
	void	ClearSRAM (bool8 onlyNonSavedSRAM = 0);

	char *	Safe (const char *);
	char *	SafeANK (const char *);
	void	ParseSNESHeader (uint8 *);
	void	InitROM (void);

	uint32	map_mirror (uint32, uint32);
	void	map_lorom (uint32, uint32, uint32, uint32, uint32);
	void	map_hirom (uint32, uint32, uint32, uint32, uint32);
	void	map_lorom_offset (uint32, uint32, uint32, uint32, uint32, uint32);
	void	map_hirom_offset (uint32, uint32, uint32, uint32, uint32, uint32);
	void	map_space (uint32, uint32, uint32, uint32, uint8 *);
	void	map_index (uint32, uint32, uint32, uint32, int, int);
	void	map_System (void);
	void	map_WRAM (void);
	void	map_LoROMSRAM (void);
	void	map_HiROMSRAM (void);
	void	map_DSP (void);
	void	map_C4 (void);
	void	map_WriteProtectROM (void);
	void	Map_Initialize (void);
	void	Map_LoROMMap (void);
	void	Map_NoMAD1LoROMMap (void);
	void	Map_JumboLoROMMap (void);
	void	Map_ROM24MBSLoROMMap (void);
	void	Map_SRAM512KLoROMMap (void);
	void	Map_HiROMMap (void);
	void	Map_ExtendedHiROMMap (void);

	uint16	checksum_calc_sum (uint8 *, uint32);
	uint16	checksum_mirror_sum (uint8 *, uint32 &, uint32 mask = 0x800000);
	void	Checksum_Calculate (void);

	bool8	match_na (const char *);
	bool8	match_nn (const char *);
	bool8	match_nc (const char *);
	bool8	match_id (const char *);
	void	ApplyROMFixes (void);
	void	CheckForAnyPatch (const char *, bool8, int32 &);

	void	MakeRomInfoText (char *);

	const char *	MapType (void);
	const char *	StaticRAMSize (void);
	const char *	Size (void);
	const char *	Revision (void);
	const char *	KartContents (void);
	const char *	Country (void);
	const char *	PublishingCompany (void);
};

extern CMemory	Memory;

void S9xAutoSaveSRAM (void);
bool8 LoadZip(const char *, uint32 *, uint8 *);

enum s9xwrap_t
{
	WRAP_NONE,
	WRAP_BANK,
	WRAP_PAGE
};

enum s9xwriteorder_t
{
	WRITE_01,
	WRITE_10
};

#include "getset.h"

#endif
