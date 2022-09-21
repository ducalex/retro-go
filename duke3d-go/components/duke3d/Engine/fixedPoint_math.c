// converted from asm to c by Jonof

#include <stdio.h>
#include "platform.h"
#include "fixedPoint_math.h"
#include "esp_attr.h"

IRAM_ATTR void clearbuf(void *d, int32_t c, int32_t a)
{
	int32_t *p = (int32_t*)d;
	while ((c--) > 0) *(p++) = a;
}

IRAM_ATTR void clearbufbyte(void *D, int32_t c, int32_t a)
{ // Cringe City
	uint8_t  *p = (uint8_t *)D;
	int32_t m[4] = { 0xffl,0xff00l,0xff0000l,0xff000000l };
	int32_t n[4] = { 0,8,16,24 };
	int32_t z=0;
	while ((c--) > 0) {
		*(p++) = (uint8_t )((a & m[z])>>n[z]);
		z=(z+1)&3;
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

