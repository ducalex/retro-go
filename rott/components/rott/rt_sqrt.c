
#include "rt_def.h"
#include "rt_sqrt.h"

/* 
  C version of fixed-point Square Root functions 
 */

long FixedSqrtLP(long n)  // Low  Precision (8.8)
{
	/* 
	   Not used, so unimplemented.
	   If it matters, just copy the first half of the HP version,
	   and multiply the answer by 256.
	 */
	STUB_FUNCTION;
	
	return 0;
}

long FixedSqrtHP(long n)  // High Precision (8.16)
{
	/* This is more or less a direct C transliteration of the asm code.
	   I've replaced right shifting with division, since right shifting
	   signed values is undefined in ANSI C (though it usually works).
	   ROTT does not use this routine heavily. */
	unsigned long root, mask, val;
	signed long d;

	root = 0;
	mask = 0x40000000;
	val = (unsigned long)n;
 hp1:
	d = val;
	d -= mask;
	if (d < 0)
	    goto hp2;
	d -= root;
	if (d < 0)
	    goto hp2;
	
	val = d;
	root /= 2;
	root |= mask;
	mask /= 4;
	if (mask != 0)
	    goto hp1;
	else
	    goto hp5;
 hp2:
	root /= 2;
	mask /= 4;
	if (mask != 0)
	    goto hp1;

 hp5:
	mask = 0x00004000;
	root <<= 16;
	val <<= 16;
 hp3:
	/* use add here to properly emulate the asm - SBF */
	if ((root+mask) > val)
	    goto hp4;
	
	val -= (root+mask);
	   
	root /= 2;
	root |= mask;
	mask /= 4;
	if (mask != 0)
	    goto hp3;
	else
	    goto hp6;
 hp4:
	root /= 2;
	mask /= 4;
	if (mask != 0)
	    goto hp3;
 hp6:

	return (long)root;
}
