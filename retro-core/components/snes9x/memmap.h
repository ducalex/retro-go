/* This file is part of Snes9x. See LICENSE file. */

#ifndef _memmap_h_
#define _memmap_h_

#include "snes9x.h"

#ifdef FAST_LSB_WORD_ACCESS
#define READ_WORD(s)      (*(uint16_t *) (s))
#define READ_DWORD(s)     (*(uint32_t *) (s))
#define WRITE_WORD(s, d)  (*(uint16_t *) (s)) = (d)
#define WRITE_DWORD(s, d) (*(uint32_t *) (s)) = (d)
#define READ_3WORD(s)     ((*(uint32_t *) (s)) & 0x00ffffff)
#define WRITE_3WORD(s, d) *(uint16_t *) (s) = (uint16_t)(d), *((uint8_t *) (s) + 2) = (uint8_t) ((d) >> 16)
#else
#define READ_WORD(s)      (*(uint8_t *) (s) | (*((uint8_t *) (s) + 1) << 8))
#define READ_DWORD(s)     (*(uint8_t *) (s) | (*((uint8_t *) (s) + 1) << 8) | (*((uint8_t *) (s) + 2) << 16) | (*((uint8_t *) (s) + 3) << 24))
#define WRITE_WORD(s, d)   *(uint8_t *) (s) = (d), *((uint8_t *) (s) + 1) = (d) >> 8
#define WRITE_DWORD(s, d)  *(uint8_t *) (s) = (uint8_t) (d), *((uint8_t *) (s) + 1) = (uint8_t) ((d) >> 8), *((uint8_t *) (s) + 2) = (uint8_t) ((d) >> 16), *((uint8_t *) (s) + 3) = (uint8_t) ((d) >> 24)
#define WRITE_3WORD(s, d)  *(uint8_t *) (s) = (uint8_t) (d), *((uint8_t *) (s) + 1) = (uint8_t) ((d) >> 8), *((uint8_t *) (s) + 2) = (uint8_t) ((d) >> 16)
#define READ_3WORD(s)     (*(uint8_t *) (s) | (*((uint8_t *) (s) + 1) << 8) | (*((uint8_t *) (s) + 2) << 16))
#endif

#define MEMMAP_BLOCK_SIZE (0x1000)
#define MEMMAP_NUM_BLOCKS (0x1000000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_BLOCKS_PER_BANK (0x10000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_SHIFT 12
#define MEMMAP_MASK (MEMMAP_BLOCK_SIZE - 1)

/* Extended ROM Formats */
#define NOPE 0
#define YEAH 1
#define BIGFIRST 2
#define SMALLFIRST 3

bool LoadROM(const char*);
void InitROM(bool);
bool S9xInitMemory(void);
void S9xDeinitMemory(void);
void FreeSDD1Data(void);

void WriteProtectROM(void);
void FixROMSpeed(void);
void MapRAM(void);
void MapExtraRAM(void);
void ResetSpeedMap(void);

void BSLoROMMap(void);
void JumboLoROMMap(bool);
void LoROMMap(void);
void LoROM24MBSMap(void);
void SRAM512KLoROMMap(void);
void SRAM1024KLoROMMap(void);
void SufamiTurboLoROMMap(void);
void HiROMMap(void);
void SuperFXROMMap(void);
void TalesROMMap(bool);
void AlphaROMMap(void);
void SA1ROMMap(void);
void BSHiROMMap(void);
void SPC7110HiROMMap(void);
void SPC7110Sram(uint8_t);
void SetaDSPMap(void);
void ApplyROMFixes(void);
void ApplyROMPatches(void);
void DSPMap(void);
void CapcomProtectLoROMMap(void);

const char* TVStandard(void);
const char* Speed(void);
const char* MapType(void);
const char* Headers(void);
const char* ROMID(void);
const char* CompanyID(void);

enum
{
   MAP_PPU, MAP_CPU, MAP_DSP, MAP_LOROM_SRAM, MAP_HIROM_SRAM,
   MAP_NONE, MAP_DEBUG, MAP_C4, MAP_BWRAM, MAP_BWRAM_BITMAP,
   MAP_BWRAM_BITMAP2, MAP_SA1RAM, MAP_SPC7110_ROM, MAP_SPC7110_DRAM,
   MAP_RONLY_SRAM, MAP_OBC_RAM, MAP_SETA_DSP, MAP_SETA_RISC, MAP_LAST
};

enum
{
   MAX_ROM_SIZE = 0x240000,
   RAM_SIZE = 0x20000,
   SRAM_SIZE = 0x10000, // 0x20000,
   VRAM_SIZE = 0x10000,
   FILLRAM_SIZE = 0x8000,
};

enum
{
   MAP_TYPE_I_O = 1,
   MAP_TYPE_ROM = 2,
   MAP_TYPE_RAM = 3
};

typedef struct
{
   uint8_t Speed:5;
   uint8_t Type:3;
} SMapInfo;

typedef struct
{
   uint8_t* RAM;
   uint8_t* ROM;
   uint8_t* VRAM;
   uint8_t* SRAM;
   uint8_t* FillRAM;
   uint8_t* C4RAM;
   bool     HiROM;
   bool     LoROM;
   uint16_t SRAMMask;
   uint8_t  SRAMSize;
   uint8_t**Map; // [MEMMAP_NUM_BLOCKS];
   SMapInfo*MapInfo;// [MEMMAP_NUM_BLOCKS];
   char     ROMName     [ROM_NAME_LEN + 1];
   char     ROMId       [4 + 1];
   char     CompanyId   [2 + 1];
   uint8_t  ROMSpeed;
   uint8_t  ROMType;
   uint8_t  ROMSize;
   int32_t  ROMFramesPerSecond;
   int32_t  HeaderCount;
   uint32_t CalculatedSize;
   uint32_t CalculatedChecksum;
   uint32_t ROMChecksum;
   uint32_t ROMComplementChecksum;
   // char     ROMFilename [_MAX_PATH];
   uint8_t  ROMRegion;
   uint8_t  ExtendedFormat;
   size_t   ROM_AllocSize; // size of *ROM content
   size_t   ROM_Offset;
} CMemory;

uint8_t S9xGetByte(uint32_t Address);
uint16_t S9xGetWord(uint32_t Address);
void S9xSetByte(uint8_t Byte, uint32_t Address);
void S9xSetWord(uint16_t Byte, uint32_t Address);
void S9xSetPCBase(uint32_t Address);
uint8_t* S9xGetMemPointer(uint32_t Address);
uint8_t* GetBasePointer(uint32_t Address);

extern CMemory Memory;
extern uint8_t OpenBus;

#endif /* _memmap_h_ */
