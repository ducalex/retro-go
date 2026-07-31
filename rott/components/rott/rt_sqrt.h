/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef _rt_sqrt_public
#define _rt_sqrt_public

/*
FUNCTION:
    Fixed32 FixedSqrtHP(Fixed32 n);
DESCRIPTION:
    This does a high-precision square root of a Fixed32.  It has
    8.16 bit accuracy.  For more speed use FixedSqrtLP().


FUNCTION:
    Fixed32 FixedSqrtLP(Fixed32 n);
DESCRIPTION:
    This does a low-precision square root of a Fixed32.  It has
    8.8 bit accuracy.  For more accuracy use FixedSqrtHP().
*/



long FixedSqrtLP(long n);  // Low  Precision (8.8)
long FixedSqrtHP(long n);  // High Precision (8.16)

#ifdef __WATCOMC__
#pragma aux FixedSqrtLP =            \
    "         xor eax, eax"          \
    "         mov ebx, 40000000h"    \
    "sqrtLP1: mov edx, ecx"          \
    "         sub edx, ebx"          \
    "         jl  sqrtLP2"           \
    "         sub edx, eax"          \
    "         jl  sqrtLP2"           \
    "         mov ecx,edx"           \
    "         shr eax, 1"            \
    "         or  eax, ebx"          \
    "         shr ebx, 2"            \
    "         jnz sqrtLP1"           \
    "         shl eax, 8"            \
    "         jmp sqrtLP3"           \
    "sqrtLP2: shr eax, 1"            \
    "         shr ebx, 2"            \
    "         jnz sqrtLP1"           \
    "         shl eax, 8"            \
    "sqrtLP3: nop"                   \
    parm caller [ecx]                \
    value [eax]                      \
    modify [eax ebx ecx edx];


#pragma aux FixedSqrtHP =            \
    "         xor eax, eax"          \
    "         mov ebx, 40000000h"    \
    "sqrtHP1: mov edx, ecx"          \
    "         sub edx, ebx"          \
    "         jb  sqrtHP2"           \
    "         sub edx, eax"          \
    "         jb  sqrtHP2"           \
    "         mov ecx,edx"           \
    "         shr eax, 1"            \
    "         or  eax, ebx"          \
    "         shr ebx, 2"            \
    "         jnz sqrtHP1"           \
    "         jz  sqrtHP5"           \
    "sqrtHP2: shr eax, 1"            \
    "         shr ebx, 2"            \
    "         jnz sqrtHP1"           \
    "sqrtHP5: mov ebx, 00004000h"    \
    "         shl eax, 16"           \
    "         shl ecx, 16"           \
    "sqrtHP3: mov edx, ecx"          \
    "         sub edx, ebx"          \
    "         jb  sqrtHP4"           \
    "         sub edx, eax"          \
    "         jb  sqrtHP4"           \
    "         mov ecx, edx"          \
    "         shr eax, 1"            \
    "         or  eax, ebx"          \
    "         shr ebx, 2"            \
    "         jnz sqrtHP3"           \
    "         jmp sqrtHP6"           \
    "sqrtHP4: shr eax, 1"            \
    "         shr ebx, 2"            \
    "         jnz sqrtHP3"           \
    "sqrtHP6: nop"                   \
    parm caller [ecx]                \
    value [eax]                      \
    modify [eax ebx ecx edx];
#endif

#endif
