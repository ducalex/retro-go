/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file has been modified from Ken Silverman's original release
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "platform.h"
#include "display.h"

#include "fixedPoint_math.h"
#include "cache.h"
#include "build.h"

#include "cvar_defs.h"

#include "types.h"

#include "esp_attr.h"

/*
 *   This module keeps track of a standard linear cacheing system.
 *   To use this module, here's all you need to do:
 *
 *   Step 1: Allocate a nice BIG buffer, like from 1MB-4MB and
 *           Call initcache(int32_t cachestart, int32_t cachesize) where
 *
 *              cachestart = (int32_t )(pointer to start of BIG buffer)
 *              cachesize = length of BIG buffer
 *
 *   Step 2: Call allocache(int32_t *bufptr, int32_t bufsiz, uint8_t  *lockptr)
 *              whenever you need to allocate a buffer, where:
 *
 *              *bufptr = pointer to 4-byte pointer to buffer
 *                 Confused?  Using this method, cache2d can remove
 *                 previously allocated things from the cache safely by
 *                 setting the 4-byte pointer to 0.
 *              bufsiz = number of bytes to allocate
 *              *lockptr = pointer to locking uint8_t  which tells whether
 *                 the region can be removed or not.  If *lockptr = 0 then
 *                 the region is not locked else its locked.
 *
 *   Step 3: If you need to remove everything from the cache, or every
 *           unlocked item from the cache, you can call uninitcache();
 *              Call uninitcache(0) to remove all unlocked items, or
 *              Call uninitcache(1) to remove everything.
 *           After calling uninitcache, it is still ok to call allocache
 *           without first calling initcache.
 */

#define MAXCACHEOBJECTS 9216

static int32_t cachesize = 0;
int32_t cachecount = 0;
uint8_t  zerochar = 0;
uint8_t* cachestart = NULL;
int32_t cacnum = 0, agecount = 0;

typedef struct { 
    uint8_t** hand;
    int32_t leng;
    uint8_t  *lock; } 
cactype;

EXT_RAM_ATTR cactype cac[MAXCACHEOBJECTS];
EXT_RAM_ATTR int32_t lockrecip[200];

// TC game directory
char  game_dir[512] = { "/sd/duke3d\0" };

void initcache(uint8_t* dacachestart, int32_t dacachesize)
{
	printf("Initcache: %d bytes, at: %p\n",dacachesize, dacachestart);
	int32_t i;

	for(i=1;i<200;i++) lockrecip[i] = (1<<28)/(200-i);

	cachestart = dacachestart;
	cachesize = dacachesize;

	cac[0].leng = cachesize;
	cac[0].lock = &zerochar;
	cacnum = 1;
}

void allocache (uint8_t** newhandle, int32_t newbytes, uint8_t  *newlockptr)
{
	int32_t i, z, zz, bestz=0, daval, bestval, besto=0, o1, o2, sucklen, suckz;

	newbytes = newbytes+15;

	if ((uint32_t)newbytes > (uint32_t)cachesize)
	{
		printf("Cachesize: %d\n",cachesize);
		printf("*Newhandle: 0x%x, Newbytes: %d, *Newlock: %d\n",(unsigned int)newhandle,newbytes,*newlockptr);
		reportandexit("BUFFER TOO BIG TO FIT IN CACHE!\n");
	}

	if (*newlockptr == 0)
	{
		reportandexit("ALLOCACHE CALLED WITH LOCK OF 0!\n");
	}

		/* Find best place */
	bestval = 0x7fffffff; o1 = cachesize;
	for(z=cacnum-1;z>=0;z--)
	{
		o1 -= cac[z].leng;
		o2 = o1+newbytes; if (o2 > cachesize) continue;

		daval = 0;
		for(i=o1,zz=z;i<o2;i+=cac[zz++].leng)
		{
			if (*cac[zz].lock == 0) continue;
			if (*cac[zz].lock >= 200) { daval = 0x7fffffff; break; }
			daval += (int32_t ) mulscale32(cac[zz].leng+65536,lockrecip[*cac[zz].lock]);
			if (daval >= bestval) break;
		}
		if (daval < bestval)
		{
			bestval = daval; besto = o1; bestz = z;
			if (bestval == 0) break;
		}
	}

	/*printf("%ld %ld %ld\n",besto,newbytes,*newlockptr);*/

	if (bestval == 0x7fffffff)
		reportandexit("CACHE SPACE ALL LOCKED UP!\n");

		/* Suck things out */
	for(sucklen=-newbytes,suckz=bestz;sucklen<0;sucklen+=cac[suckz++].leng)
		if (*cac[suckz].lock) *cac[suckz].hand = 0;

		/* Remove all blocks except 1 */
	suckz -= (bestz+1); cacnum -= suckz;
	copybufbyte(&cac[bestz+suckz],&cac[bestz],(cacnum-bestz)*sizeof(cactype));
	cac[bestz].hand = newhandle;
    *newhandle = cachestart+besto;
	cac[bestz].leng = newbytes;
	cac[bestz].lock = newlockptr;
	cachecount++;

		/* Add new empty block if necessary */
	if (sucklen <= 0) return;

	bestz++;
	if (bestz == cacnum)
	{
		cacnum++; if (cacnum > MAXCACHEOBJECTS) reportandexit("Too many objects in cache! (cacnum > MAXCACHEOBJECTS)\n");
		cac[bestz].leng = sucklen;
		cac[bestz].lock = &zerochar;
		return;
	}

	if (*cac[bestz].lock == 0) { cac[bestz].leng += sucklen; return; }

	cacnum++; if (cacnum > MAXCACHEOBJECTS) reportandexit("Too many objects in cache! (cacnum > MAXCACHEOBJECTS)\n");
	for(z=cacnum-1;z>bestz;z--) cac[z] = cac[z-1];
	cac[bestz].leng = sucklen;
	cac[bestz].lock = &zerochar;
}

void suckcache (int32_t *suckptr)
{
	int32_t i;

		/* Can't exit early, because invalid pointer might be same even though lock = 0 */
	for(i=0;i<cacnum;i++)
		if ((int32_t )(*cac[i].hand) == (int32_t )suckptr)
		{
			if (*cac[i].lock) *cac[i].hand = 0;
			cac[i].lock = &zerochar;
			cac[i].hand = 0;

				/* Combine empty blocks */
			if ((i > 0) && (*cac[i-1].lock == 0))
			{
				cac[i-1].leng += cac[i].leng;
				cacnum--; copybuf(&cac[i+1],&cac[i],(cacnum-i)*sizeof(cactype));
			}
			else if ((i < cacnum-1) && (*cac[i+1].lock == 0))
			{
				cac[i+1].leng += cac[i].leng;
				cacnum--; copybuf(&cac[i+1],&cac[i],(cacnum-i)*sizeof(cactype));
			}
		}
}

IRAM_ATTR void agecache(void)
{
	int32_t cnt;
	uint8_t  ch;

	if (agecount >= cacnum) agecount = cacnum-1;
	assert(agecount >= 0);

	for(cnt=(cacnum>>4);cnt>=0;cnt--)
	{
		ch = (*cac[agecount].lock);
		if (((ch-2)&255) < 198)
			(*cac[agecount].lock) = (uint8_t ) (ch-1);

		agecount--; if (agecount < 0) agecount = cacnum-1;
	}
}

void reportandexit(char  *errormessage)
{
	int32_t i, j;

	setvmode(0x3);
	printf("Cachesize = %d\n",cachesize);
	printf("Cacnum = %d\n",cacnum);
	printf("ERROR: %s",errormessage);
	j = 0;
	for(i=0;i<cacnum;i++)
	{
		printf("%d- ",i);
		if(cac[i].hand != NULL)
		{
			printf("ptr: 0x%x, ",(int8_t)*cac[i].hand);
			printf("leng: %d, ",cac[i].leng);
			printf("lock: %d\n",*cac[i].lock);
			j += cac[i].leng;
		}
	}
	printf("Cache length sum = %d\n",j);
	Error(EXIT_FAILURE, "");
}

