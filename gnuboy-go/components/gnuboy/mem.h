#ifndef __MEM_H__
#define __MEM_H__

#include "emu.h"

enum {
	MBC_NONE = 0,
	MBC_MBC1,
	MBC_MBC2,
	MBC_MBC3,
	MBC_MBC5,
	MBC_MBC6,
	MBC_MBC7,
	MBC_HUC1,
	MBC_HUC3,
	MBC_MMM01,
};

// This must be a power of 2
#define SRAM_SECTOR_SIZE 1024

struct mbc
{
	int type;
	int model;
	int rombank;
	int rambank;
	int romsize;
	int ramsize;
	int enableram;
	int rumble;
	int sensor;
	int batt;
	int rtc;
	byte *rmap[0x10];
	byte *wmap[0x10];
};

struct rom
{
	byte *bank[512];
	char name[20];
	un16 checksum;
};

struct ram
{
	byte hi[256];
	byte ibank[8][4096];
	union {
		byte (*sbank)[8192];
		byte *sram;
	};
	un32 sram_dirty;
	byte sram_dirty_sector[256];
};


extern struct mbc mbc;
extern struct rom rom;
extern struct ram ram;


void mem_updatemap();
void mem_write(addr_t a, byte b);
byte mem_read(addr_t a);
void mem_reset(bool hard);

static inline byte readb(addr_t a)
{
	byte *p = mbc.rmap[a>>12];
	if (p) return p[a];
	return mem_read(a);
}

static inline void writeb(addr_t a, byte b)
{
	byte *p = mbc.wmap[a>>12];
	if (p) p[a] = b;
	else mem_write(a, b);
}

static inline word readw(addr_t a)
{
#ifdef IS_LITTLE_ENDIAN
	if ((a & 0xFFF) == 0xFFF)
#endif
	{
		return readb(a) | (readb(a + 1) << 8);
	}
	byte *p = mbc.rmap[a >> 12];
	if (p)
	{
		return *(word *)(p + a);
	}
	return mem_read(a) | (mem_read(a + 1) << 8);
}

static inline void writew(addr_t a, word w)
{
#ifdef IS_LITTLE_ENDIAN
	if ((a & 0xFFF) == 0xFFF)
#endif
	{
		writeb(a, w);
		writeb(a + 1, w >> 8);
		return;
	}
	byte *p = mbc.wmap[a >> 12];
	if (p)
	{
		*(word *)(p + a) = w;
		return;
	}
	mem_write(a, w);
	mem_write(a + 1, w >> 8);
}

#endif
