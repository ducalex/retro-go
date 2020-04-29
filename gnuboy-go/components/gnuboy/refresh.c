#include "defs.h"
#include "lcd.h"


void refresh_1(byte *dest, byte *src, byte *pal, int cnt)
{
	while(cnt--) *(dest++) = pal[*(src++)];
}

void refresh_2(un16 *dest, byte *src, un16 *pal, int cnt)
{
	while (cnt--) *(dest++) = pal[*(src++)];
}

void refresh_3(byte *dest, byte *src, un32 *pal, int cnt)
{
	un32 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		*(dest++) = c;
		*(dest++) = c>>8;
		*(dest++) = c>>16;
	}
}

void refresh_4(un32 *dest, byte *src, un32 *pal, int cnt)
{
	while (cnt--) *(dest++) = pal[*(src++)];
}

void refresh_1_2x(byte *dest, byte *src, byte *pal, int cnt)
{
	byte c;
	while (cnt--)
	{
		c = pal[*(src++)];
		*(dest++) = c;
		*(dest++) = c;
	}
}

void refresh_2_2x(un16 *dest, byte *src, un16 *pal, int cnt)
{
	un16 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		*(dest++) = c;
		*(dest++) = c;
	}
}

void refresh_3_2x(byte *dest, byte *src, un32 *pal, int cnt)
{
	un32 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		dest[0] = dest[3] = c;
		dest[1] = dest[4] = c>>8;
		dest[2] = dest[5] = c>>16;
		dest += 6;
	}
}

void refresh_4_2x(un32 *dest, byte *src, un32 *pal, int cnt)
{
	un32 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		*(dest++) = c;
		*(dest++) = c;
	}
}

void refresh_2_3x(un16 *dest, byte *src, un16 *pal, int cnt)
{
	un16 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		*(dest++) = c;
		*(dest++) = c;
		*(dest++) = c;
	}
}

void refresh_3_3x(byte *dest, byte *src, un32 *pal, int cnt)
{
	un32 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		dest[0] = dest[3] = dest[6] = c;
		dest[1] = dest[4] = dest[7] = c>>8;
		dest[2] = dest[5] = dest[8] = c>>16;
		dest += 9;
	}
}

void refresh_4_3x(un32 *dest, byte *src, un32 *pal, int cnt)
{
	un32 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		*(dest++) = c;
		*(dest++) = c;
		*(dest++) = c;
	}
}

void refresh_3_4x(byte *dest, byte *src, un32 *pal, int cnt)
{
	un32 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		dest[0] = dest[3] = dest[6] = dest[9] = c;
		dest[1] = dest[4] = dest[7] = dest[10] = c>>8;
		dest[2] = dest[5] = dest[8] = dest[11] = c>>16;
		dest += 12;
	}
}

void refresh_4_4x(un32 *dest, byte *src, un32 *pal, int cnt)
{
	un32 c;
	while (cnt--)
	{
		c = pal[*(src++)];
		*(dest++) = c;
		*(dest++) = c;
		*(dest++) = c;
		*(dest++) = c;
	}
}
