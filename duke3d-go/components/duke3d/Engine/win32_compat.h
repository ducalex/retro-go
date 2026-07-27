/*
 * win32 compatibility header. Takes care of some legacy code issues
 *  and incompatibilities at the source level.
 *
 *  Written by Ryan C. Gordon (icculus@clutteredmind.org)
 *
 * Please do NOT harrass Ken Silverman about any code modifications
 *  (including this file) to BUILD.
 */

/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file IS NOT A PART OF Ken Silverman's original release
 */


  
#ifndef _INCLUDE_WIN32_COMPAT_H_
#define _INCLUDE_WIN32_COMPAT_H_

#if (!defined PLATFORM_WIN32)
#error PLATFORM_WIN32 is not defined.
#endif

#define PLATFORM_SUPPORTS_SDL

#include <stdio.h>

#if (!defined _MSC_VER)
	#include <unistd.h>
#else
	#include <fcntl.h>
    #include <SYS\STAT.H>
#endif

#include <stdlib.h>
#include <io.h>
#include <direct.h>
#include <conio.h>
#include <dos.h>
#include <assert.h>
#include <string.h>

#define kmalloc(x) malloc(x)
#define kkmalloc(x) malloc(x)
#define kfree(x) free(x)
#define kkfree(x) free(x)

#ifdef FP_OFF
#undef FP_OFF
#endif

#define FP_OFF(x) ((long) (x))

/* !!! This might be temporary. */
#define printext16 printext256
#define printext16_noupdate printext256_noupdate

#ifndef max
#define max(x, y)  (((x) > (y)) ? (x) : (y))
#endif

#ifndef min
#define min(x, y)  (((x) < (y)) ? (x) : (y))
#endif

#if (defined __WATCOMC__)
#define inline
#pragma intrinsic(min);
#pragma intrinsic(max);
#define __int64 long long
#endif

#if (defined _MSC_VER)
#if ((!defined _INTEGRAL_MAX_BITS) || (_INTEGRAL_MAX_BITS < 64))
#error __int64 type not supported
#endif

#define open _open
#define O_BINARY _O_BINARY
#define O_RDONLY _O_RDONLY
#define O_WRONLY _O_WRONLY
#define O_RDWR _O_RDWR
#define O_TRUNC _O_TRUNC
#define O_CREAT _O_CREAT
#define S_IREAD _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IRDWR _S_IRDWR
#endif /* defined _MSC_VER */

#define snprintf _snprintf

#define strncasecmp strnicmp

//Damn you Microsoft, how hard would it REALLY be to support C99 ?!??!?!
#define inline 
#include "windows/inttypes.h"

#endif

/* end of win32_compat.h ... */



