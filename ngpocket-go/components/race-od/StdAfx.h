//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

#ifndef STDAFX_H
#define STDAFX_H

#ifdef TARGET_OD
#include <stdio.h>

#define DWORD unsigned int
#define byte unsigned char

#define _u8   unsigned char
//#define Uint8 unsigned char
#define _u16  unsigned short
#define _u32  unsigned int

#define BOOL  int
#define FALSE 0
#define TRUE  (!0)
#define HWND void*

#else /* ifdef TARGET_OD */

#ifdef TARGET_PSP

#include <stdio.h>

#define DWORD unsigned int
#define byte unsigned char

#define _u8   unsigned char
#define Uint8 unsigned char
#define _u16  unsigned short
#define _u32  unsigned int

#define BOOL  int
#define FALSE 0
#define TRUE  (!0)
#define HWND void*

#else /* ifdef TARGET_PSP */

#ifdef TARGET_GP2X

//extern "C" void gp2x_memcpy(unsigned char *, unsigned char *, unsigned int);
#define memcpy gp2x_memcpy
#define memcmp gp2x_memcmp
//#define memset gp2x_memset

#define GENREGSPC_AS_REG  //comment out to do it as a regular mem loc
#define GENREGSSR_AS_REG  //comment out to do it as a regular mem loc

register unsigned char *my_pc asm ("r9");
register unsigned long opcode asm ("r6");
#ifdef GENREGSPC_AS_REG
register unsigned long gen_regsPC asm("r8");
#endif
#ifdef GENREGSSR_AS_REG
register unsigned long gen_regsSR asm("r7");
#endif
#endif


#include "StdAfx.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>


typedef unsigned int UINT32;
typedef unsigned int DWORD;
typedef void * HRESULT;
typedef unsigned char byte;

typedef unsigned char _u8;
typedef unsigned short _u16;
typedef unsigned int _u32;






#define MB_OK 1

typedef bool BOOL;
#define FALSE false
#define TRUE true

#ifndef _MAX_PATH
#define _MAX_PATH 150
#endif

#ifdef TARGET_WIN
#define DEBUG
#endif

#ifdef DEBUG
#define dbg_printf printf
#else
#define dbg_printf if(0) printf
#endif
#define dbg_print dbg_printf

void AfxMessageBox(char *a, int b, int c);
void AfxMessageBox(char *a, int b);
void AfxMessageBox(char *a);

#define HWND void*
#define BITMAP

#endif /* ifdef TARGET_PSP */

#endif /* ifdef TARGET_OD */

#endif //STDAFX_H
