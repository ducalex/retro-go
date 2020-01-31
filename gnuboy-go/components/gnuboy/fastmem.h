
#ifndef __FASTMEM_H__
#define __FASTMEM_H__


#include "defs.h"
#include "mem.h"


inline static byte readb(int a)
{
	byte *p = mbc.rmap[a>>12];
	if (p)
	{
		return p[a];
	}
	else
	{
		 return mem_read(a);
	 }
}

inline static void writeb(int a, byte b)
{
	byte *p = mbc.wmap[a>>12];
	if (p) p[a] = b;
	else mem_write(a, b);
}

inline static int readw(int a)
{
	if ((a+1) & 0xfff)
	{
		byte *p = mbc.rmap[a>>12];
		if (p)
		{
#ifdef IS_LITTLE_ENDIAN
#ifndef ALLOW_UNALIGNED_IO
			if (a&1) return p[a] | (p[a+1]<<8);
#endif
			return *(word *)(p+a);
#else
			return p[a] | (p[a+1]<<8);
#endif
		}
	}
	return mem_read(a) | (mem_read(a+1)<<8);
}

inline static void writew(int a, int w)
{
	if ((a+1) & 0xfff)
	{
		byte *p = mbc.wmap[a>>12];
		if (p)
		{
#ifdef IS_LITTLE_ENDIAN
#ifndef ALLOW_UNALIGNED_IO
			if (a&1)
			{
				p[a] = w;
				p[a+1] = w >> 8;
				return;
			}
#endif
			*(word *)(p+a) = w;
			return;
#else
			p[a] = w;
			p[a+1] = w >> 8;
			return;
#endif
		}
	}
	mem_write(a, w);
	mem_write(a+1, w>>8);
}

inline static byte readhi(int a)
{
	return readb(a | 0xff00);
}

inline static void writehi(int a, byte b)
{
	writeb(a | 0xff00, b);
}


#endif
