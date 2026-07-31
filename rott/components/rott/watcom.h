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
#ifndef _watcom_h_public
#define _watcom_h_public
fixed FixedMul(fixed a, fixed b);
fixed FixedDiv2(fixed a, fixed b);
fixed FixedScale(fixed orig, fixed factor, fixed divisor);
fixed FixedMulShift(fixed a, fixed b, fixed shift);
#ifdef __WATCOMC__
#pragma aux FixedMul =  \
        "imul ebx",                     \
        "add  eax, 8000h"        \
        "adc  edx,0h"            \
        "shrd eax,edx,16"       \
        parm    [eax] [ebx] \
        value   [eax]           \
        modify exact [eax edx]

#pragma aux FixedMulShift =  \
        "imul ebx",                     \
        "shrd eax,edx,cl"       \
        parm    [eax] [ebx] [ecx]\
        value   [eax]           \
        modify exact [eax edx]

#pragma aux FixedDiv2 = \
        "cdq",                          \
        "shld edx,eax,16",      \
        "sal eax,16",           \
        "idiv ebx"                      \
        parm    [eax] [ebx] \
        value   [eax]           \
        modify exact [eax edx]
#pragma aux FixedScale = \
        "imul ebx",                     \
        "idiv ecx"                      \
        parm    [eax] [ebx] [ecx]\
        value   [eax]           \
        modify exact [eax edx]
#endif

#endif
