// converted from asm to c by Jonof

#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "fixedPoint_math.h"
#include "esp_attr.h"

IRAM_ATTR void clearbuf(void *d, int32_t c, int32_t a)
{
	int32_t *p = (int32_t*)d;
	while ((c--) > 0) *(p++) = a;
}

IRAM_ATTR void clearbufbyte(void *D, int32_t c, int32_t a)
{
	uint8_t *p = (uint8_t *)D;
	uint32_t pattern = (uint32_t)a;

	if (c <= 0)
		return;

	// The common clear-to-one-colour case is best handled by the optimized
	// libc routine. Otherwise preserve Build's repeating four-byte pattern.
	const uint8_t byte = (uint8_t)pattern;
	if (pattern == (uint32_t)byte * 0x01010101u)
	{
		memset(p, byte, (size_t)c);
		return;
	}

	// Align the destination while rotating the little-endian byte pattern so
	// the first word store continues exactly where the byte loop left off.
	while (c > 0 && ((uintptr_t)p & 3u))
	{
		*p++ = (uint8_t)pattern;
		pattern = (pattern >> 8) | (pattern << 24);
		c--;
	}

	uint32_t *words = (uint32_t *)p;
	while (c >= 4)
	{
		*words++ = pattern;
		c -= 4;
	}

	p = (uint8_t *)words;
	while (c-- > 0)
	{
		*p++ = (uint8_t)pattern;
		pattern = (pattern >> 8) | (pattern << 24);
	}
}

IRAM_ATTR void copybuf(void *s, void *d, int32_t c)
{
	int32_t *p = (int32_t*)s, *q = (int32_t*)d;
	while ((c--) > 0) *(q++) = *(p++);
}

IRAM_ATTR void copybufbyte(void *S, void *D, int32_t c)
{
	uint8_t  *p = (uint8_t *)S, *q = (uint8_t *)D;
	while((c--) > 0) *(q++) = *(p++);
}

IRAM_ATTR void copybufreverse(void *S, void *D, int32_t c)
{
	uint8_t  *p = (uint8_t *)S, *q = (uint8_t *)D;
	while((c--) > 0) *(q++) = *(p--);
}

IRAM_ATTR void qinterpolatedown16(int32_t* bufptr, int32_t num, int32_t val, int32_t add)
{ // gee, I wonder who could have provided this...
    int32_t i, *lptr = bufptr;
    for(i=0;i<num;i++) { lptr[i] = (val>>16); val += add; }
}

IRAM_ATTR void qinterpolatedown16short(int32_t* bufptr, int32_t num, int32_t val, int32_t add)
{ // ...maybe the same person who provided this too?
    int32_t i; short *sptr = (short *)bufptr;
    for(i=0;i<num;i++) { sptr[i] = (short)(val>>16); val += add; }
}
