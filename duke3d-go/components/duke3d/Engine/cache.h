/*
 * Cache1d declarations.
 *
 *  Written by Ryan C. Gordon. (icculus@clutteredmind.org)
 *
 * Please do NOT harrass Ken Silverman about any code modifications
 *  (including this file) to BUILD.
 */

/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file has been modified from Ken Silverman's original release
 */

#ifndef _INCLUDE_CACHE1D_H_
#define _INCLUDE_CACHE1D_H_

void initcache(uint8_t* dacachestart, int32_t dacachesize);
void allocache (uint8_t* *newhandle, int32_t newbytes, uint8_t  *newlockptr);
void suckcache (int32_t *suckptr);
void agecache(void);


void reportandexit(char  *errormessage);


//Filesystem



#endif
