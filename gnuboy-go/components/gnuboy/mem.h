#ifndef __MEM_H__
#define __MEM_H__

#include "defs.h"

#define MBC_NONE 0
#define MBC_MBC1 1
#define MBC_MBC2 2
#define MBC_MBC3 3
#define MBC_MBC5 5
#define MBC_RUMBLE 15
#define MBC_HUC1 0xC1
#define MBC_HUC3 0xC3

struct mbc
{
	int type;
	int model;
	int rombank;
	int rambank;
	int romsize;
	int ramsize;
	int enableram;
	int batt;
	int rtc;
	byte *rmap[0x10], *wmap[0x10];
};

struct rom
{
	byte* bank[512];
	char name[20];
	int length;
	int checksum;
};

struct ram
{
	byte hi[256];
	byte ibank[8][4096];
	byte (*sbank)[8192];
	byte sram_dirty;
};


extern struct mbc mbc;
extern struct rom rom;
extern struct ram ram;


void mem_updatemap();
void mem_write(word a, byte b);
byte mem_read(word a);
void mbc_reset();

static inline byte readb(word a)
{
	byte *p = mbc.rmap[a>>12];
	if (p) return p[a];
	return mem_read(a);
}

static inline void writeb(word a, byte b)
{
	byte *p = mbc.wmap[a>>12];
	if (p) p[a] = b;
	else mem_write(a, b);
}

static inline word readw(word a)
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

static inline void writew(word a, word w)
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
