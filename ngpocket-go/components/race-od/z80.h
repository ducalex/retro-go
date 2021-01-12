//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

// z80.h: interface for the z80 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_Z80_H__28525BE2_C77A_11D3_8645_00A0241D2A65__INCLUDED_)
#define AFX_Z80_H__28525BE2_C77A_11D3_8645_00A0241D2A65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define Z80NMI	0xFF00

void z80SetRunning(int running);
void z80Init();
void z80Interrupt(int prio);
void z80UnInterrupt(int kind);
int z80Step();
void z80Print(FILE *output);
void z80orIFF(unsigned char bits);


#endif // !defined(AFX_Z80_H__28525BE2_C77A_11D3_8645_00A0241D2A65__INCLUDED_)
