
#include "rt_def.h"
#include "watcom.h"

/* 
  C versions of watcom.h assembly.
  Uses the '__int64' type (see rt_def.h).
 */

fixed FixedMul(fixed a, fixed b)
{
	__int64 scratch1 = (__int64) a * (__int64) b + (__int64) 0x8000;
	return (scratch1 >> 16) & 0xffffffff;
}

fixed FixedMulShift(fixed a, fixed b, fixed shift)
{
	__int64 x = a;
	__int64 y = b;
	__int64 z = x * y;
	
	return (((unsigned __int64)z) >> shift) & 0xffffffff;
}

fixed FixedDiv2(fixed a, fixed b)
{
	__int64 x = (signed int)a;
	__int64 y = (signed int)b;
	__int64 z = x * 65536 / y;
	
	return (z) & 0xffffffff;
}

fixed FixedScale(fixed orig, fixed factor, fixed divisor)
{
	__int64 x = orig;
	__int64 y = factor;
	__int64 z = divisor;
	
	__int64 w = (x * y) / z;
	
	return (w) & 0xffffffff;
}
