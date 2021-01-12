//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

// z80.cpp: implementation of the z80 class.
//  used for PC version, but DrZ80 can be used for ARM devices
//
// TODO:
//	- CCF:		Implement H flag
//	- CPL:		Implement H flag
//	- LD A,I:	Implement everything
//	- Interrupt Flip Flops: implement these (RETI/RETN/Interrupts)
//	- RLD:		Implement everything
//	- RRD:		Implement everything
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "main.h"
#include "memory.h"
#include "z80.h"
//#include "mainemu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// opslag voor de registers
// AF, BC, DE, HL, IX, IY, SP, PC, AF', BC', DE', HL'
// F,A,B,C,D,E,H,L,IX,IY,SP,F',A',BC',DE',HL'
//
// Flags register: (same of TLCS-900)
// bit 7:	S flag
// bit 6:	Z flag
// bit 5:	undocumented flag
// bit 4:	H flag
// bit 3:	undocumented flag
// bit 2:	P/V flag
// bit 1:	N flag
// bit 0:	C flag
//

int				z80State;		// number of cycles passed
int				z80Running;		// Is z80 running?
unsigned char	z80Opcode;		// last read opcode
unsigned short	z80Regs[12];	// Z80 CPU registers
#define Z80BC	0
#define Z80DE	1
#define Z80HL	2
#define Z80SP	3
#define Z80AF	4
#define Z80PC	5
#define Z80IX	6
#define Z80IY	7
#define Z80AF2	8
#define Z80BC2	9
#define Z80DE2	10
#define Z80HL2	11
unsigned char	*z80RegsB[8]=	// pointers to byte registers
{
	((unsigned char *)&z80Regs[Z80BC])+1,		// B
	((unsigned char *)&z80Regs[Z80BC]),			// C
	((unsigned char *)&z80Regs[Z80DE])+1,		// D
	((unsigned char *)&z80Regs[Z80DE]),			// E
	((unsigned char *)&z80Regs[Z80HL])+1,		// H
	((unsigned char *)&z80Regs[Z80HL]),			// L
	((unsigned char *)&z80Regs[Z80AF]),			// F
	((unsigned char *)&z80Regs[Z80AF])+1		// A
};
#define Z80A	7
#define	Z80F	6
#define Z80B	0
#define Z80C	1
#define Z80D	2
#define Z80E	3
#define	Z80H	4
#define	Z80L	5

#define IM0		0
#define IMUNDEF	1
#define IM1		2
#define IM2		3

unsigned char	Z80R;		// the R register incremented with every opcode fetch
							// bit 7 is never changed
unsigned char	Z80RH;		// used to store high order bit for R registers
unsigned char	Z80IFF;		// the interrupt flip flops
unsigned char	Z80IM;		// the interrupt mode as set by the IM instruction
							// 0 - IM 0
							// 1 - undefined
							// 2 - IM 1
							// 3 - IM 2
int		Z80NMIPending;		// is an NMI interrupt pending (TRUE/FALSE)
int		Z80IntPending;		// is a regular interrupt pending (0x100 | interrupt data)
int		z80HaltState;		// 0 - CPU is not in HALT/STOP state
							// 1 - CPU is in HALT/STOP state
unsigned short	*z80Index;	// used for handling instruction using IX/IY
signed char		z80d8;		// displacement used with the bit operations
int				z80DebugIndex;
int				z80DebugCount;

///// reading from memory for instruction decoding

inline unsigned char z80Readbyte()
{
	unsigned char i = z80MemReadB(z80Regs[Z80PC]);

	z80Regs[Z80PC]+= 1;
	return i;
}

inline unsigned short z80Readword()
{
	unsigned char i = z80MemReadB(z80Regs[Z80PC]);
	z80Regs[Z80PC]+=1;
	unsigned char j = z80MemReadB(z80Regs[Z80PC]);
	z80Regs[Z80PC]+=1;
	return (j<<8)|i;
}

// LD instructions

int z80LDrn()		// LD r,n
{
	*z80RegsB[z80Opcode>>3] = z80Readbyte();	return 4+3;
}

int z80LDrr()		// LD r,r'
{
	*z80RegsB[(z80Opcode>>3)&7] = *z80RegsB[z80Opcode&7];	return 4;
}

int z80LDrHL()		// LD r,(HL)
{
	*z80RegsB[(z80Opcode>>3)&7] = z80MemReadB(z80Regs[Z80HL]);	return 8;
}

int z80LDHLr()		// LD (HL),r
{
	z80MemWriteB(z80Regs[Z80HL],*z80RegsB[z80Opcode&7]);	return 4+3;
}

int z80LDHLn()		// LD (HL),n
{
	z80MemWriteB(z80Regs[Z80HL],z80Readbyte());	return 4+3+3;
}

int z80LDArr()		// LD A,(rr)		(BC/DE)
{
	*z80RegsB[Z80A] = z80MemReadB(z80Regs[(z80Opcode>>4)&1]);	return 4+3;
}

int z80LDAnn()		// LD A,(nn)
{
	*z80RegsB[Z80A] = z80MemReadB(z80Readword());	return 4+3+3+3;
}

int z80LDrrA()		// LD (rr),A		(BC/DE)
{
	z80MemWriteB(z80Regs[(z80Opcode>>4)&1],*z80RegsB[Z80A]);	return 4+3;
}

int z80LDnnA()		// LD (nn),A
{
	z80MemWriteB(z80Readword(),*z80RegsB[Z80A]);	return 4+3+3+3;
}

int z80LDmemHL()	// LD (nn),HL
{
	z80MemWriteW(z80Readword(),z80Regs[Z80HL]);	return 4+3+3+3+3;
}

int z80LDnnrr()		// LD (nn),rr
{
	z80MemWriteW(z80Readword(),z80Regs[(z80Opcode>>4)&3]);	return 4+4+3+3+3+3;
}

int z80LDrrnnm()	// LD rr,(nn)
{
	z80Regs[(z80Opcode>>4)&3] = z80MemReadW(z80Readword());	return 4+4+3+3+3+3;
}

int z80LDHLmem()	// LD HL,(nn)
{
	z80Regs[Z80HL] = z80MemReadW(z80Readword());	return 4+3+3+3+3;
}

int z80LDrrnn()		// LD rr,nn		(BC/DE/HL/SP)
{
	z80Regs[(z80Opcode>>4)&3] = z80Readword();	return 4+3+3;
}

int z80LDSPHL()		// LD SP,HL
{
	z80Regs[Z80SP] = z80Regs[Z80HL];	return 6;
}

int z80LDnnSP()		// LD (nn),SP
{
	z80MemWriteW(z80Readword(),z80Regs[Z80SP]);	return 20;
}

int z80LDXYnn()		// LD Iz,nn
{
	*z80Index = z80Readword();	return 4+3+3;
}

int z80LDXYnnm()	// LD Iz,(nn)
{
	*z80Index = z80MemReadW(z80Readword());	return 4+3+3+3+3;
}

int z80LDXYn()		// LD (Iz+d8),n
{
	signed char d8 = (signed char)z80Readbyte();

	z80MemWriteB(*z80Index+d8,z80Readbyte());	return 4+3+5+3;
}

int z80LDXYr()		// LD (Iz+d8),r
{
	signed char d8 = (signed char)z80Readbyte();

	z80MemWriteB(*z80Index+d8,*z80RegsB[z80Opcode&7]);	return 4+3+5+3;
}

int z80LDXYH()		// LD (Iz+d8),H
{
	signed char d8 = (signed char)z80Readbyte();

	z80MemWriteB(*z80Index+d8,*(((unsigned char *)&z80Regs[Z80HL])+1));	return 4+3+5+3;
}

int z80LDXYL()		// LD (Iz+d8),L
{
	signed char d8 = (signed char)z80Readbyte();

	z80MemWriteB(*z80Index+d8,*((unsigned char *)&z80Regs[Z80HL]));	return 4+3+5+3;
}

int z80LDnnXY()		// LD (nn),Iz
{
	z80MemWriteW(z80Readword(),*z80Index);	return 4+3+3+3+3;
}

int z80LDrXY()		// LD r,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*z80RegsB[(z80Opcode>>3)&7] = z80MemReadB(*z80Index+d8);	return 4+3+5+3;
}

int z80LDHXY()		// LD H,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*(((unsigned char *)&z80Regs[Z80HL])+1) = z80MemReadB(*z80Index+d8);	return 4+3+5+3;
}

int z80LDLXY()		// LD L,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*((unsigned char *)&z80Regs[Z80HL]) = z80MemReadB(*z80Index+d8);	return 4+3+5+3;
}

int z80LDSPXY()		// LD SP,Iz
{
	z80Regs[Z80SP] = *z80Index;	return 6;
}

int z80LDAI()		// LD A,I
{
	unsigned char i = Z80RH | (Z80R & 0x7F);

	*z80RegsB[Z80A] = i;
	*z80RegsB[Z80F] = (i & 0xA8) |
		((i) ? 0 : 0x40) |
		(*z80RegsB[Z80F] & 0x01);
	return 4+5;
}

int z80LDIA()		// LD I,A
{
	return 4+5;
}

int z80LDAR()		// LD A,R
{
	unsigned char i = Z80RH | (Z80R & 0x7F);

	*z80RegsB[Z80A] = i;
	*z80RegsB[Z80F] = (i & 0xA8) |
		((i) ? 0 : 0x40) |
		(*z80RegsB[Z80F] & 0x01);
	return 4+5;
}

int z80LDRA()		// LD R,A
{
	Z80RH = (Z80R = *z80RegsB[Z80A]) & 0x80;
	return 4+5;
}

int z80LDD()		// LDD
{
	z80MemWriteB(z80Regs[Z80DE],z80MemReadB(z80Regs[Z80HL]));
	z80Regs[Z80DE]--;	z80Regs[Z80HL]--;	z80Regs[Z80BC]--;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xE9) |
		((z80Regs[Z80BC]) ? 0 : 0x04);
	return 4+4+3+5;
}

int z80LDDR()		// LDDR
{
	z80MemWriteB(z80Regs[Z80DE],z80MemReadB(z80Regs[Z80HL]));
	z80Regs[Z80DE]--;	z80Regs[Z80HL]--;	z80Regs[Z80BC]--;
	if (!z80Regs[Z80BC])	return 4+4+3+5;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xE9);
	z80Regs[Z80PC]-= 2;		// repeat
	return 4+4+3+5+5;
}

int z80LDI()		// LDI
{
	z80MemWriteB(z80Regs[Z80DE],z80MemReadB(z80Regs[Z80HL]));
	z80Regs[Z80DE]++;	z80Regs[Z80HL]++;	z80Regs[Z80BC]--;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xE9) |
		((z80Regs[Z80BC]) ? 0 : 0x04);
	return 4+4+3+5;
}

int z80LDIR()		// LDIR
{
	z80MemWriteB(z80Regs[Z80DE],z80MemReadB(z80Regs[Z80HL]));
	z80Regs[Z80DE]++;	z80Regs[Z80HL]++;	z80Regs[Z80BC]--;
	if (!z80Regs[Z80BC])	return 4+4+3+5;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xE9);
	z80Regs[Z80PC]-= 2;		// repeat
	return 4+4+3+5+5;
}

// PUSH instructions

int z80PUSHrr()		// PUSH rr (BC/DE/HL)
{
	z80Regs[Z80SP]-= 2;	z80MemWriteW(z80Regs[Z80SP],z80Regs[(z80Opcode>>4)&3]);	return 5+3+3;
}

int z80PUSHAF()		// PUSH AF
{
	z80Regs[Z80SP]-= 2;	z80MemWriteW(z80Regs[Z80SP],z80Regs[Z80AF]);	return 5+3+3;
}

int z80PUSHXY()		// PUSH Iz
{
	z80Regs[Z80SP]-=2;	z80MemWriteW(z80Regs[Z80SP],*z80Index);	return 5+3+3;
}

// POP instructions

int z80POPrr()		// POP rr (BC/DE/HL)
{
	z80Regs[(z80Opcode>>4)&3] = z80MemReadW(z80Regs[Z80SP]);	z80Regs[Z80SP]+= 2;	return 4+3+3;
}

int z80POPAF()		// POP AF
{
	z80Regs[Z80AF] = z80MemReadW(z80Regs[Z80SP]);	z80Regs[Z80SP]+= 2;	return 4+3+3;
}

int z80POPXY()		// POP Iz
{
	*z80Index = z80MemReadW(z80Regs[Z80SP]);	z80Regs[Z80SP]+=2;	return 4+3+3;
}

// ADD instructions

unsigned char z80AddB(unsigned char i, unsigned char j)
{
	unsigned char oldi = i;

	i+= j;
	*z80RegsB[Z80F] = (i&0xA8) |
		((i) ? 0 : 0x40) |
		(((i & 0x0F)<(oldi&0x0F)) ? 0x10 : 0) |
		(((i^oldi) & (i^j) & 0x80) ? 0x04 : 0) |
		((i<oldi) ? 1 : 0);
	return i;
}

unsigned short z80AddW(unsigned short i, unsigned short j)
{
	unsigned short oldi = i;

	i+= j;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xC4) |
		((i&0x2800)>>8) |
		(((i & 0x0FFF)<(oldi&0x0FFF)) ? 0x10 : 0) |
		((i<oldi) ? 1 : 0);
	return i;
}

int z80ADDAr()		// ADD A,r
{
	*z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],*z80RegsB[z80Opcode&7]);	return 4;
}

int z80ADDAHL()		// ADD A,(HL)
{
	*z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));	return 4+3;
}

int z80ADDAn()		// ADD A,n
{
	*z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],z80Readbyte());	return 4+3;
}

int z80ADDHLrr()	// ADD HL,rr
{
	z80Regs[Z80HL] = z80AddW(z80Regs[Z80HL],z80Regs[z80Opcode>>4]);	return 4+4+3;
}

int	z80ADDXYrr()	// ADD Ix,rr
{
	*z80Index = z80AddW(*z80Index,z80Regs[z80Opcode>>4]);	return 4+4+3;
}

int z80ADDAXY()		// ADD A,(Ix+d)
{
	signed char d8 = (signed char)z80Readbyte();

	*z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],z80MemReadB(*z80Index+d8));	return 4+3+5+3;
}

// ADC instructions

unsigned char z80AdcB(unsigned char i, unsigned char j)
{
	unsigned char oldi = i;

	i+= j + (*z80RegsB[Z80F]&1);
	*z80RegsB[Z80F] = (i&0xA8) |
		((i) ? 0 : 0x40) |
		(((i & 0x0F)<(oldi&0x0F)) ? 0x10 : 0) |
		(((i^oldi) & (i^j) & 0x80) ? 0x04 : 0) |
		((i<oldi || (i==oldi && j)) ? 1 : 0);
	return i;
}

unsigned short z80AdcW(unsigned short i, unsigned short j)
{
	unsigned short oldi = i;

	i+= j + (*z80RegsB[Z80F]&1);
	*z80RegsB[Z80F] = ((i&0xA8)>>8) |
		((i) ? 0 : 0x40) |
		(((i & 0x0FFF)<(oldi&0x0FFF)) ? 0x10 : 0) |
		(((i^oldi) & (i^j) & 0x8000) ? 0x04 : 0) |
		((i<oldi || (i==oldi &&j)) ? 1 : 0);
	return i;
}

int z80ADCAr()		// ADC A,r
{
	*z80RegsB[Z80A] = z80AdcB(*z80RegsB[Z80A],*z80RegsB[z80Opcode&7]);	return 4;
}

int z80ADCAHL()		// ADC A,(HL)
{
	*z80RegsB[Z80A] = z80AdcB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));	return 4+3;
}

int z80ADCAn()		// ADC A,n
{
	*z80RegsB[Z80A] = z80AdcB(*z80RegsB[Z80A],z80Readbyte());	return 4+3;
}

int z80ADCHLrr()	// ADC HL,rr
{
	z80Regs[Z80HL] = z80AdcW(z80Regs[Z80HL],z80Regs[(z80Opcode>>4)&3]);	return 4+4+4+3;
}

int z80ADCAXY()		// ADC A,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*z80RegsB[Z80A] = z80AdcB(*z80RegsB[Z80A],*z80Index+d8);	return 4+3+5+3;
}

// SUB instructions

unsigned char z80SubB(unsigned char i, unsigned char j)
{
	unsigned char oldi = i;

	i-= j;
	*z80RegsB[Z80F] = (i & 0xA8) |
		((i) ? 2 : 0x42) |
		(((i & 0x0F)>(oldi&0x0F)) ? 0x10 : 0) |
		(((j^oldi) & (i^oldi) & 0x80) ? 0x04 : 0) |
		((i>oldi) ? 1 : 0);
	return i;
}

int z80SUBAr()		// SUB A,r
{
	*z80RegsB[Z80A] = z80SubB(*z80RegsB[Z80A],*z80RegsB[z80Opcode&7]);	return 4;
}

int z80SUBAHL()		// SUB A,(HL)
{
	*z80RegsB[Z80A] = z80SubB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));	return 4+3;
}

int z80SUBAn()		// SUB A,n
{
	*z80RegsB[Z80A] = z80SubB(*z80RegsB[Z80A],z80Readbyte());	return 4+3;
}

int z80SUBAXY()		// SUB A,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*z80RegsB[Z80A] = z80SubB(*z80RegsB[Z80A],z80MemReadB(*z80Index+d8));	return 4+3+5+3;
}

// SBC instruction

unsigned char z80SbcB(unsigned char i, unsigned char j)
{
	unsigned char oldi = i;

	i = (i - j) - (*z80RegsB[Z80F]&0x01);
	*z80RegsB[Z80F] = (i & 0xA8) |
		((i) ? 2 : 0x42) |
		(((i & 0x0F)>(oldi&0x0F)) ? 0x10 : 0) |
		(((j^oldi) & (i^oldi) & 0x80) ? 0x04 : 0) |
		((i>oldi || (i==oldi && j)) ? 1 : 0);
	return i;
}

unsigned short z80SbcW(unsigned short i, unsigned short j)
{
	unsigned short oldi = i;

	i = (i - j) - (*z80RegsB[Z80F]&1);
	*z80RegsB[Z80F] = ((i&0xA800)>>8) |
		((i) ? 2 : 0x42) |
		(((i & 0x0FFF)>(oldi&0x0FFF)) ? 0x10 : 0) |
		(((i^oldi) & (oldi^j) & 0x8000) ? 0x04 : 0) |
		((i>oldi || (i==oldi && j)) ? 1 : 0);
	return i;
}

int z80SBCAr()		// SBC A,r
{
	*z80RegsB[Z80A] = z80SbcB(*z80RegsB[Z80A],*z80RegsB[z80Opcode&7]);	return 4;
}

int z80SBCAHL()		// SBC A,(HL)
{
	*z80RegsB[Z80A] = z80SbcB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));	return 4+3;
}

int z80SBCAn()		// SBC A,n
{
	*z80RegsB[Z80A] = z80SbcB(*z80RegsB[Z80A],z80Readbyte());	return 4+3;
}

int z80SBCAXY()		// SBC A,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*z80RegsB[Z80A] = z80SbcB(*z80RegsB[Z80A],*z80Index+d8);	return 4+3+5+3;
}

int z80SBCHLrr()	// SBC HL,rr
{
	z80Regs[Z80HL] = z80SbcW(z80Regs[Z80HL],z80Regs[(z80Opcode>>4)&3]);	return 4+4+4+3;
}

// AND instruction

void z80parityB(unsigned char j)
{
	unsigned char k=0, i;

	for (i=0;i<8;i++)
	{
		if (j&1) k++;
		j = j>>1;
	}
	*z80RegsB[Z80F]|= ((k&1) ? 0 : 4);
}

unsigned char z80AndB(unsigned char i, unsigned char j)
{
	i&= j;
	*z80RegsB[Z80F] = (i & 0xA8) |
		((i) ? 0 : 0x40) |
		0x10;
	z80parityB(i);
	return i;
}

int z80ANDAr()		// AND A,r
{
	*z80RegsB[Z80A] = z80AndB(*z80RegsB[Z80A],*z80RegsB[z80Opcode&7]);	return 4;
}

int z80ANDAHL()		// AND A,(HL)
{
	*z80RegsB[Z80A] = z80AndB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));	return 4+3;
}

int z80ANDAn()		// AND A,n
{
	*z80RegsB[Z80A] = z80AndB(*z80RegsB[Z80A],z80Readbyte());	return 4+3;
}

int z80ANDAXY()		// AND A,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*z80RegsB[Z80A] = z80AndB(*z80RegsB[Z80A],z80MemReadB(*z80Index+d8));	return 4+3+5+3;
}

// OR instructions

unsigned char z80OrB(unsigned char i, unsigned char j)
{
	i|= j;
	*z80RegsB[Z80F] = (i&0xA8) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80ORAr()		// OR A,r
{
	*z80RegsB[Z80A] = z80OrB(*z80RegsB[Z80A],*z80RegsB[z80Opcode&7]);	return 4;
}

int z80ORAHL()		// OR A,(HL)
{
	*z80RegsB[Z80A] = z80OrB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));	return 4+3;
}

int z80ORAn()		// OR A,n
{
	*z80RegsB[Z80A] = z80OrB(*z80RegsB[Z80A],z80Readbyte());	return 4+3;
}

int z80ORAXY()		// OR A,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*z80RegsB[Z80A] = z80OrB(*z80RegsB[Z80A],z80MemReadB(*z80Index+d8));	return 4+3+5+3;
}

// XOR instructions

unsigned char z80XorB(unsigned char i, unsigned char j)
{
	i^= j;
	*z80RegsB[Z80F] = (i&0xA8) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80XORAr()		// OR A,r
{
	*z80RegsB[Z80A] = z80XorB(*z80RegsB[Z80A],*z80RegsB[z80Opcode&7]);	return 4;
}

int z80XORAHL()		// XOR A,(HL)
{
	*z80RegsB[Z80A] = z80XorB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));	return 4+3;
}

int z80XORAn()		// XOR A,n
{
	*z80RegsB[Z80A] = z80XorB(*z80RegsB[Z80A],z80Readbyte());	return 4+3;
}

int z80XORAXY()		// XOR A,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	*z80RegsB[Z80A] = z80XorB(*z80RegsB[Z80A],z80MemReadB(*z80Index+d8));	return 4+3+5+3;
}

// CP instructions

int z80CPAr()		// CP A,r
{
	z80SubB(*z80RegsB[Z80A],*z80RegsB[z80Opcode&7]);	return 4;
}

int z80CPAHL()		// CP A,(HL)
{
	z80SubB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));	return 4+3;
}

int z80CPAn()		// CP A,n
{
	z80SubB(*z80RegsB[Z80A],z80Readbyte());	return 4+3;
}

int z80CPAXY()		// CP A,(Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	z80SubB(*z80RegsB[Z80A],z80MemReadB(*z80Index+d8));	return 4+3+5+3;
}

int z80CPD()		// CPD
{
	z80SubB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));
	z80Regs[Z80HL]--;	z80Regs[Z80BC]--;
	if (z80Regs[Z80BC]) {
		*z80RegsB[Z80F]&= ~0x04;
	} else {
		*z80RegsB[Z80F]|= 0x04;
	}
	return 4+4+3+5;
}

int z80CPDR()		// CPDR
{
	z80SubB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));
	z80Regs[Z80HL]--;	z80Regs[Z80BC]--;
	if (z80Regs[Z80BC]) {
		*z80RegsB[Z80F]&= ~0x04;
	} else {
		*z80RegsB[Z80F]|= 0x04;
		return 4+4+3+5;
	}
	if (*z80RegsB[Z80F] & 0x40)
	{
		return 4+4+3+5;
	}
	z80Regs[Z80PC]-= 2;		// repeat
	return 4+4+3+5+5;
}

int z80CPI()		// CPI
{
	z80SubB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));
	z80Regs[Z80HL]++;	z80Regs[Z80BC]--;
	if (z80Regs[Z80BC]) {
		*z80RegsB[Z80F]&= ~0x04;
	} else {
		*z80RegsB[Z80F]|= 0x04;
	}
	return 4+4+3+5;
}

int z80CPIR()		// CPIR
{
	z80SubB(*z80RegsB[Z80A],z80MemReadB(z80Regs[Z80HL]));
	z80Regs[Z80HL]++;	z80Regs[Z80BC]--;
	if (z80Regs[Z80BC]) {
		*z80RegsB[Z80F]&= ~0x04;
	} else {
		*z80RegsB[Z80F]|= 0x04;
		return 4+4+3+5;
	}
	if (*z80RegsB[Z80F] & 0x40)
	{
		return 4+4+3+5;
	}
	z80Regs[Z80PC]-= 2;		// repeat
	return 4+4+3+5+5;
}

// INC instructions

unsigned char z80IncB(unsigned char i)
{
	unsigned char oldi = i;

	i+= 1;
	*z80RegsB[Z80F] = (i&0xA8) |
		(*z80RegsB[Z80F] & 0x01) |
		((i) ? 0 : 0x40) |
		(((oldi^i)&0x80)>>5) |
		(((i & 0x0F)<(oldi&0x0F)) ? 0x10 : 0);
	return i;
}

int z80INCr()		// INC r
{
	*z80RegsB[z80Opcode>>3] = z80IncB(*z80RegsB[z80Opcode>>3]);	return 4;
}

int z80INCHL()		// INC (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80IncB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+3;
}

int z80INCXYm()		// INC (Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	z80MemWriteB(*z80Index+d8,z80IncB(z80MemReadB(*z80Index+d8)));	return 4+3+5+4+3;
}

int z80INCrr()		// INC rr
{
	z80Regs[z80Opcode>>4]+= 1;	return 6;
}

int z80INCXY()		// INC Iz
{
	*z80Index+= 1;	return 6;
}

// DEC instructions

unsigned char z80DecB(unsigned char i)
{
	unsigned char oldi = i;

	i-= 1;
	*z80RegsB[Z80F] = (i & 0xA8) |
		(*z80RegsB[Z80F] & 0x01) |
		((i) ? 0 : 0x40) |
		(((i & 0x0F)>(oldi&0x0F)) ? 0x10 : 0) |
		(((oldi^i)&0x80)>>5) |
		0x02;
	return i;
}

int z80DECr()		// DEC r
{
	*z80RegsB[z80Opcode>>3] = z80DecB(*z80RegsB[z80Opcode>>3]);	return 4;
}

int z80DECHL()		// DEC (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80DecB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+3;
}

int z80DECrr()		// DEC rr
{
	z80Regs[z80Opcode>>4]-= 1;	return 6;
}

int z80DECXYm()		// DEC (Iz+d8)
{
	signed char d8 = (signed char)z80Readbyte();

	z80MemWriteB(*z80Index+d8,z80DecB(z80MemReadB(*z80Index+d8)));	return 4+3+5+4+3;
}

int z80DECXY()		// DEC Iz
{
	*z80Index-= 1;	return 6;
}

// misc instructions

int z80DAA()		// DAA
{
	unsigned char i = (*z80RegsB[Z80A] & 0xf0);
	unsigned char j = (*z80RegsB[Z80A] & 0x0f);

	if (*z80RegsB[Z80F]&2) {	// adjust after SUB, SBC or NEG operation
		if (*z80RegsB[Z80F]&1) {	// C
			if (*z80RegsB[Z80F]&0x10) {	// H
				if ((i>0x50) && (j>0x05)) {
					*z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x9a); *z80RegsB[Z80F]|= 1;
				}
			} else {	// !H
				if ((i>0x60) && (j<0x0a)) {
					*z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0xa0); *z80RegsB[Z80F]|= 1;
				}
			}
		} else {	// !C
			if (*z80RegsB[Z80F]&0x10) {	// H
				if ((i<0x90) && (j>0x05)) {
					*z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0xfa); *z80RegsB[Z80F]&= ~1;
				}
			} else {	// !H
			}
		}
	} else {	// adjust after ADD or ADC operation
		if (*z80RegsB[Z80F]&1) {	// C
			if (*z80RegsB[Z80F]&0x10) {	// H
				if      ((i<0x40) && (j<0x04))	{ *z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x66); *z80RegsB[Z80F]|= 1; }
			} else {	// !H
				if      ((i<0x30) && (j<0x0a))	{ *z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x60); *z80RegsB[Z80F]|= 1; }
				else if ((i<0x30) && (j>0x09))	{ *z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x66); *z80RegsB[Z80F]|= 1; }
			}
		} else {	// !C
			if (*z80RegsB[Z80F]&0x10) {	// H
				if      ((i<0xa0) && (j<0x04))	{ *z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x06); *z80RegsB[Z80F]&= ~1; }
				else if ((i>0x90) && (j<0x04))	{ *z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x66); *z80RegsB[Z80F]|= 1; }
			} else {	// !H
				if      ((i<0x90) && (j>0x09))	{ *z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x06); *z80RegsB[Z80F]&= ~1; }
				else if ((i>0x90) && (j<0x0a))	{ *z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x60); *z80RegsB[Z80F]|= 1; }
				else if ((i>0x80) && (j>0x09))	{ *z80RegsB[Z80A] = z80AddB(*z80RegsB[Z80A],0x66); *z80RegsB[Z80F]|= 1; }
			}
		}
	}
	// calculate parity
	z80parityB(*z80RegsB[Z80A]);
	return 4;
}

int z80CPL()		// CPL
{
	*z80RegsB[Z80A] = ~*z80RegsB[Z80A];
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xED) | 0x12;	return 4;
}

int z80CCF()		// CCF
{
	*z80RegsB[Z80F] = *z80RegsB[Z80F] ^ 0x01;	return 4;
}

int z80SCF()		// SCF
{
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xEC) | 0x01;	return 4;
}

int z80NOP()		// NOP
{
	return 4;
}

int z80HALT()		// HALT
{
	z80HaltState = 1;
	z80Regs[Z80PC]-= 1;
	return 4;
}

int z80IM()			// IM x
{
	Z80IM = (z80Opcode&0x18)>>3;
	return 4+4;
}

int z80DI()			// DI
{
	Z80IFF = 0;
	return 4;
}

int z80EI()			// EI
{
	Z80IFF = 3;
	return 4;
}

int z80DJNZ()		// DJNZ d8
{
	signed char d8 = (signed char)z80Readbyte();

	if (!(*z80RegsB[Z80B]-=1))	return 2;
	z80Regs[Z80PC]+= d8;
	return 5+3+5;
}

int z80NEG()		// NEG
{
	unsigned char oldi = *z80RegsB[Z80A];

	*z80RegsB[Z80A] = z80SubB(0,oldi);
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xFA) |
		((oldi) ? 1 : 0) |
		((oldi == 0x80) ? 4 : 0);
	return 4+4;
}

// RLC instructions

unsigned char z80RlcB(unsigned char i)
{
	i = ((i&0x80) ? (i<<1)|1 : i<<1);
	*z80RegsB[Z80F] = (i&0xA9) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80RLCA()		// RLCA
{
	*z80RegsB[Z80A] = z80RlcB(*z80RegsB[Z80A]);	return 4;
}

int z80RLCr()		// RLC r
{
	*z80RegsB[z80Opcode&7] = z80RlcB(*z80RegsB[z80Opcode&7]);	return 4+4;
}

int z80RLCHL()		// RLC (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80RlcB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+4+3;
}

int z80RLCXY()		// RLC (Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80RlcB(*z80Index+z80d8));	return 4+3+5+4+3;
}

// RL instructions

unsigned char z80RlB(unsigned char i)
{
	if (i&0x80)
	{
		i = (i << 1)|(*z80RegsB[Z80F]&0x01);
		*z80RegsB[Z80F] = 0x01;
	}
	else
	{
		i = (i << 1)|(*z80RegsB[Z80F]&0x01);
		*z80RegsB[Z80F] = 0;
	}
	*z80RegsB[Z80F] = *z80RegsB[Z80F] |
		(i&0xA8) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80RLA()		// RLA
{
	*z80RegsB[Z80A] = z80RlB(*z80RegsB[Z80A]);	return 4;
}

int z80RLr()		// RL r
{
	*z80RegsB[z80Opcode&7] = z80RlB(*z80RegsB[z80Opcode&7]);	return 4+4;
}

int z80RLHL()		// RL (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80RlB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+4+3;
}

int z80RLXY()		// RL (Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80RlB(*z80Index+z80d8));	return 4+3+5+4+3;
}

// RRC instructions

unsigned char z80RrcB(unsigned char i)
{
	i = ((i&1) ? (i>>1)|0x80 : i>>1);
	*z80RegsB[Z80F] = (i&0xA8) |
		((i&0x80)>>7) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80RRCA()		// RRCA
{
	*z80RegsB[Z80A] = z80RrcB(*z80RegsB[Z80A]);	return 4;
}

int z80RRCr()		// RRC r
{
	*z80RegsB[z80Opcode&7] = z80RrcB(*z80RegsB[z80Opcode&7]);	return 4+4;
}

int z80RRCHL()		// RRC (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80RrcB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+4+3;
}

int z80RRCXY()		// RRC (Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80RrcB(z80MemReadB(*z80Index+z80d8)));	return 4+3+5+4+3;
}

// RR instructions

unsigned char z80RrB(unsigned char i)
{
	if (i&1)
	{
		i = (i >> 1)|((*z80RegsB[Z80F]&0x01)<<7);
		*z80RegsB[Z80F] = 0x01;
	}
	else
	{
		i = (i >> 1)|((*z80RegsB[Z80F]&0x01)<<7);
		*z80RegsB[Z80F] = 0;
	}
	*z80RegsB[Z80F] = *z80RegsB[Z80F] |
		(i&0xA8) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80RRA()		// RRA
{
	*z80RegsB[Z80A] = z80RrB(*z80RegsB[Z80A]);	return 4;
}

int z80RRr()		// RR r
{
	*z80RegsB[z80Opcode&7] = z80RrB(*z80RegsB[z80Opcode&7]);	return 4+4;
}

int z80RRHL()		// RR (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80RrB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+4+3;
}

int z80RRXY()		// RR (Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80RrB(z80MemReadB(*z80Index+z80d8)));	return 4+3+5+4+3;
}

// SLA instructions

unsigned char z80SlaB(unsigned char i)
{
	*z80RegsB[Z80F] = ((i&0x80) ? 0x01 : 0);
	i = i << 1;
	*z80RegsB[Z80F] |= (i&0xA8) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80SLAr()		// SLA r
{
	*z80RegsB[z80Opcode&7] = z80SlaB(*z80RegsB[z80Opcode&7]);	return 4+4;
}

int z80SLAHL()		// SLA (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80SlaB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+4+3;
}

int z80SLAXY()		// SLA (Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80SlaB(z80MemReadB(*z80Index+z80d8)));	return 4+3+5+4+3;
}

// SRA instructions

unsigned char z80SraB(unsigned char i)
{
	*z80RegsB[Z80F] = i&1;
	i = (i >> 1)|(i&0x80);
	*z80RegsB[Z80F] |= (i&0xA8) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80SRAr()		// SRA r
{
	*z80RegsB[z80Opcode&7] = z80SraB(*z80RegsB[z80Opcode&7]);	return 4+4;
}

int z80SRAHL()		// SRA (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80SraB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+4+3;
}

int z80SRAXY()		// SRA (Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80SraB(z80MemReadB(*z80Index+z80d8)));	return 4+3+5+4+3;
}

// SLL instructions (undocumented instructions)

unsigned char z80SllB(unsigned char i)
{
	*z80RegsB[Z80F] = ((i&0x80) ? 0x10 : 0);
	i = (i << 1)|1;
	*z80RegsB[Z80F] |= ((i) ? 0 : 0x80);
	return i;
}

int z80SLLr()		// SLA r
{
	*z80RegsB[z80Opcode&7] = z80SllB(*z80RegsB[z80Opcode&7]);	return 8;
}

int z80SLLHL()		// SLA (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80SllB(z80MemReadB(z80Regs[Z80HL])));	return 16;
}

// SRL instructions

unsigned char z80SrlB(unsigned char i)
{
	*z80RegsB[Z80F] = i&1;
	i = i >> 1;
	*z80RegsB[Z80F]|= (i&0xA8) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return i;
}

int z80SRLr()		// SRL r
{
	*z80RegsB[z80Opcode&7] = z80SrlB(*z80RegsB[z80Opcode&7]);	return 4+4;
}

int z80SRLHL()		// SRL (HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80SrlB(z80MemReadB(z80Regs[Z80HL])));	return 4+4+4+3;
}

int z80SRLXY()		// SRL (Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80SrlB(z80MemReadB(*z80Index+z80d8)));	return 4+3+5+4+3;
}

// RLD/RRD instructions

int z80RLD()		// RLD
{
	// docs say sign flag is set if accumulator equals zero
	// assuming that that's an error
	unsigned char i = *z80RegsB[Z80A];
	unsigned char j = z80MemReadB(z80Regs[Z80HL]);

	*z80RegsB[Z80A] = (i&0xF0)|((j&0xF0)>>4);
	z80MemWriteB(z80Regs[Z80HL],(j<<4)|(i&0x0F));
	*z80RegsB[Z80F] = (*z80RegsB[Z80F]&0x01) |
		(*z80RegsB[Z80A]&0xA8) |
		((*z80RegsB[Z80A]) ? 0 : 0x40);
	z80parityB(*z80RegsB[Z80A]);
	return 4+4+3+4+3;
}

int z80RRD()		// RRD
{
	unsigned char i = *z80RegsB[Z80A];
	unsigned char j = z80MemReadB(z80Regs[Z80HL]);

	*z80RegsB[Z80A] = (i&0xF0)|(j&0x0F);
	z80MemWriteB(z80Regs[Z80HL],(j>>4)|((i&0x0F)<<4));
	*z80RegsB[Z80F] = (*z80RegsB[Z80F]&0x01) |
		(*z80RegsB[Z80A]&0xA8) |
		((*z80RegsB[Z80A]) ? 0 : 0x40);
	z80parityB(*z80RegsB[Z80A]);
	return 4+4+3+4+3;
}

// BIT instructions

unsigned char z80Power[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

int z80BITr()		// BIT b,r
{
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xAD) | 0x10;
	if (!(z80Power[(z80Opcode>>3)&7] & *z80RegsB[z80Opcode&7]))	*z80RegsB[Z80F] |= 0x40;
	return 4+4;
}

int z80BITHL()		// BIT b,(HL)
{
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xAD) | 0x10;
	if (!(z80Power[(z80Opcode>>3)&7] & z80MemReadB(z80Regs[Z80HL])))	*z80RegsB[Z80F] |= 0x40;
	return 4+4+4;
}

int z80BITXY()		// BIT b,(Iz+d8)
{
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xAD) | 0x10;
	if (!(z80Power[(z80Opcode>>3)&7] & z80MemReadB(*z80Index+z80d8)))	*z80RegsB[Z80F] |= 0x40;
	return 4+3+5+4;
}

// SET instructions

int z80SETr()		// SET b,r
{
	*z80RegsB[z80Opcode&7] |= z80Power[(z80Opcode>>3)&7];
	return 4+4;
}

int z80SETHL()		// SET b,(HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80MemReadB(z80Regs[Z80HL]) | z80Power[(z80Opcode>>3)&7]);
	return 4+4+4+3;
}

int z80SETXY()		// SET b,(Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80MemReadB(*z80Index+z80d8) | z80Power[(z80Opcode>>3)&7]);
	return 4+3+5+4+3;
}

// RES instructions

int z80RESr()		// RES b,r
{
	*z80RegsB[z80Opcode&7] &= ~z80Power[(z80Opcode>>3)&7];
	return 4+4;
}

int z80RESHL()		// RES b,(HL)
{
	z80MemWriteB(z80Regs[Z80HL],z80MemReadB(z80Regs[Z80HL]) & ~z80Power[(z80Opcode>>3)&7]);
	return 4+4+4+3;
}

int z80RESXY()		// RES b,(Iz+d8)
{
	z80MemWriteB(*z80Index+z80d8,z80MemReadB(*z80Index+z80d8) & ~z80Power[(z80Opcode>>3)&7]);
	return 4+3+5+4+3;
}

// EX instructions

int z80EXAFAF()		// EX AF,AF'
{
	unsigned short i = z80Regs[Z80AF];

	z80Regs[Z80AF] = z80Regs[Z80AF2];
	z80Regs[Z80AF2] = i;
	return 4;
}

int z80EXX()		// EXX
{
	unsigned short i = z80Regs[Z80BC];

	z80Regs[Z80BC] = z80Regs[Z80BC2];	z80Regs[Z80BC2] = i;
	i = z80Regs[Z80DE];
	z80Regs[Z80DE] = z80Regs[Z80DE2];	z80Regs[Z80DE2] = i;
	i = z80Regs[Z80HL];
	z80Regs[Z80HL] = z80Regs[Z80HL2];	z80Regs[Z80HL2] = i;
	return 4;
}

int z80EXDEHL()		// EX DE,HL
{
	unsigned short i = z80Regs[Z80DE];

	z80Regs[Z80DE] = z80Regs[Z80HL];
	z80Regs[Z80HL] = i;
	return 4;
}

int z80EXSPHL()		// EX (SP),HL
{
	unsigned short i = z80Regs[Z80HL];

	z80Regs[Z80HL] = z80MemReadW(z80Regs[Z80SP]);
	z80MemWriteW(z80Regs[Z80SP],i);
	return 4+3+4+3+5;
}

int z80EXSPXY()		// EX (SP),Iz
{
	unsigned short i = *z80Index;

	*z80Index = z80MemReadW(z80Regs[Z80SP]);
	z80MemWriteW(z80Regs[Z80SP],i);
	return 4+3+4+3+5;
}

// JP instructions

int z80JP()			// JP nn
{
	z80Regs[Z80PC] = z80Readword();	return 4+3+3;
}

int z80JPcc()		// JP cc,nn
{
	unsigned short d = z80Readword();

	switch(z80Opcode&0x38)
	{
	case 0x00:		// NZ
		if (*z80RegsB[Z80F]&0x40)		return 4+3+3;
		break;
	case 0x08:		// Z
		if (!(*z80RegsB[Z80F]&0x40))	return 4+3+3;
		break;
	case 0x10:		// NC
		if (*z80RegsB[Z80F]&0x01)		return 4+3+3;
		break;
	case 0x18:		// C
		if (!(*z80RegsB[Z80F]&0x01))	return 4+3+3;
		break;
	case 0x20:		// PO
		if (*z80RegsB[Z80F]&0x04)		return 4+3+3;
		break;
	case 0x28:		// PE
		if (!(*z80RegsB[Z80F]&0x04))	return 4+3+3;
		break;
	case 0x30:		// P
		if (*z80RegsB[Z80F]&0x80)		return 4+3+3;
		break;
	case 0x38:		// M
		if (!(*z80RegsB[Z80F]&0x80))	return 4+3+3;
		break;
	}
	z80Regs[Z80PC] = d;	return 4+3+3;
}

int z80JPHL()		// JP (HL)
{
	z80Regs[Z80PC] = z80Regs[Z80HL];
	return 4;
}

int z80JPXY()		// JP (Iz)
{
	z80Regs[Z80PC] = *z80Index;
	return 4;
}

// JR instructions

int z80JR()			// JR d8
{
	signed char d8 = (signed char)z80Readbyte();

	z80Regs[Z80PC]+= d8;
	return 4+3+5;
}

int z80JRcc()		// JR cc,d8
{
	signed char d8 = (signed char)z80Readbyte();

	switch(z80Opcode&0x18)
	{
	case 0x00:		// NZ
		if (*z80RegsB[Z80F]&0x40)		return 4+3;
		break;
	case 0x08:		// Z
		if (!(*z80RegsB[Z80F]&0x40))	return 4+3;
		break;
	case 0x10:		// NC
		if (*z80RegsB[Z80F]&0x01)		return 4+3;
		break;
	case 0x18:		// C
		if (!(*z80RegsB[Z80F]&0x01))	return 4+3;
		break;
	}
	z80Regs[Z80PC]+= d8;	return 4+3+5;
}

// CALL instructions

int z80CALL()		// CALL nn
{
	unsigned short d = z80Readword();

	z80Regs[Z80SP]-= 2;	z80MemWriteW(z80Regs[Z80SP],z80Regs[Z80PC]);		// push PC
	z80Regs[Z80PC] = d;	return 4+3+4+3+3;
}

int z80CALLcc()		// CALL cc,nn
{
	unsigned short d = z80Readword();

	switch(z80Opcode&0x38)
	{
	case 0x00:		// NZ
		if (*z80RegsB[Z80F]&0x40)		return 4+3+3;
		break;
	case 0x08:		// Z
		if (!(*z80RegsB[Z80F]&0x40))	return 4+3+3;
		break;
	case 0x10:		// NC
		if (*z80RegsB[Z80F]&0x01)		return 4+3+3;
		break;
	case 0x18:		// C
		if (!(*z80RegsB[Z80F]&0x01))	return 4+3+3;
		break;
	case 0x20:		// PO
		if (*z80RegsB[Z80F]&0x04)		return 4+3+3;
		break;
	case 0x28:		// PE
		if (!(*z80RegsB[Z80F]&0x04))	return 4+3+3;
		break;
	case 0x30:		// P
		if (*z80RegsB[Z80F]&0x80)		return 4+3+3;
		break;
	case 0x38:		// M
		if (!(*z80RegsB[Z80F]&0x80))	return 4+3+3;
		break;
	}
	z80Regs[Z80SP]-= 2;	z80MemWriteW(z80Regs[Z80SP],z80Regs[Z80PC]);		// push PC
	z80Regs[Z80PC] = d;	return 4+3+4+3+3;
}

// RST instructions

int z80RST()		// RST n
{
	z80Regs[Z80SP]-= 2;	z80MemWriteW(z80Regs[Z80SP],z80Regs[Z80PC]);		// push PC
	z80Regs[Z80PC] = z80Opcode&0x38;	return 5+3+3;
}

// RET instructions

int z80RET()		// RET
{
	z80Regs[Z80PC] = z80MemReadW(z80Regs[Z80SP]);	z80Regs[Z80SP]+= 2;	// pop PC
	return 4+3+3;
}

int z80RETcc()		// RET cc
{
	switch(z80Opcode&0x38)
	{
	case 0x00:		// NZ
		if (*z80RegsB[Z80F]&0x40)		return 5;
		break;
	case 0x08:		// Z
		if (!(*z80RegsB[Z80F]&0x40))	return 5;
		break;
	case 0x10:		// NC
		if (*z80RegsB[Z80F]&0x01)		return 5;
		break;
	case 0x18:		// C
		if (!(*z80RegsB[Z80F]&0x01))	return 5;
		break;
	case 0x20:		// PO
		if (*z80RegsB[Z80F]&0x04)		return 5;
		break;
	case 0x28:		// PE
		if (!(*z80RegsB[Z80F]&0x04))	return 5;
		break;
	case 0x30:		// P
		if (*z80RegsB[Z80F]&0x80)		return 5;
		break;
	case 0x38:		// M
		if (!(*z80RegsB[Z80F]&0x80))	return 5;
		break;
	}
	z80Regs[Z80PC] = z80MemReadW(z80Regs[Z80SP]);	z80Regs[Z80SP]+= 2;	// pop PC
	return 5+3+3;
}

int z80RETI()		// RETI
{
	z80Regs[Z80PC] = z80MemReadW(z80Regs[Z80SP]);	z80Regs[Z80SP]+= 2;	// pop PC
//	z80IME = 0x0F;
	return 4+4+3+3;
}

int z80RETN()		// RETN
{
	z80Regs[Z80PC] = z80MemReadW(z80Regs[Z80SP]);	z80Regs[Z80SP]+= 2;	// pop PC
	return 4+4+3+3;
}

// OUT instructions

int z80OUTn()		// OUT (n),A
{
	unsigned char i = z80Readbyte();

	z80PortWriteB(i,*z80RegsB[Z80A]);
	return 8;
}

int z80OUTCr()		// OUT (C),r
{
	z80PortWriteB(*z80RegsB[Z80C],*z80RegsB[(z80Opcode>>3)&7]);		// write to output port C
	return 4+4+4;
}

int z80OUTD()			// OUTD
{
	z80PortWriteB(*z80RegsB[Z80C],z80MemReadB(z80Regs[Z80HL]));		// write to output port C
	z80Regs[Z80HL]--;	*z80RegsB[Z80B]-= 1;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xBD) |
		((*z80RegsB[Z80B]) ? 2 : 0x42);
	return 4+5+3+4;
}

int z80OUTI()			// OUTI
{
	z80PortWriteB(*z80RegsB[Z80C],z80MemReadB(z80Regs[Z80HL]));		// write to output port C
	z80Regs[Z80HL]++;	*z80RegsB[Z80B]-= 1;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0xBD) |
		((*z80RegsB[Z80B]) ? 2 : 0x42);
	return 4+5+3+4;
}

int z80OTDR()		// OTDR
{
	z80PortWriteB(*z80RegsB[Z80C],z80MemReadB(z80Regs[Z80HL]));		// write to output port C
	z80Regs[Z80HL]--;	*z80RegsB[Z80B]-= 1;
	if (*z80RegsB[Z80B])
	{
		z80Regs[Z80PC]-= 2;
		return 4+5+3+4+5;
	}
	*z80RegsB[Z80F]|= 0x42;
	return 4+5+3+4;
}

int z80OTIR()		// OTIR
{
	z80PortWriteB(*z80RegsB[Z80C],z80MemReadB(z80Regs[Z80HL]));		// write to output port C
	z80Regs[Z80HL]++;	*z80RegsB[Z80B]-= 1;
	if (*z80RegsB[Z80B])
	{
		z80Regs[Z80PC]-= 2;
		return 4+5+3+4+5;
	}
	*z80RegsB[Z80F]|= 0x42;
	return 4+5+3+4;
}

// IN instructions

int z80INn()		// IN A,(N)
{
	unsigned char i = z80Readbyte();

	*z80RegsB[Z80A] = z80PortReadB(i);	// read from input port N
	return 4+3+4;
}

int z80INrC()		// IN r,(C)
{
	unsigned char i = z80PortReadB(*z80RegsB[Z80C]);	// read from input port C

	*z80RegsB[(z80Opcode>>3)&7] = i;
	*z80RegsB[Z80F] = (i&0xA8) |
		(*z80RegsB[Z80F] & 0x01) |
		((i) ? 0 : 0x40);
	z80parityB(i);
	return 4+4+4;
}

int z80IND()			// IND
{
	unsigned char i = z80PortReadB(*z80RegsB[Z80C]);	// read from input port C

	z80MemWriteB(z80Regs[Z80HL],i);
	z80Regs[Z80HL]--;	*z80RegsB[Z80B]-= 1;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0x95) |
		(i&0x28) |
		((*z80RegsB[Z80B]) ? 2 : 0x42);
	return 4+5+3+4;
}

int z80INI()			// INI
{
	unsigned char i = z80PortReadB(*z80RegsB[Z80C]);	// read from input port C

	z80MemWriteB(z80Regs[Z80HL],i);
	z80Regs[Z80HL]++;	*z80RegsB[Z80B]-= 1;
	*z80RegsB[Z80F] = (*z80RegsB[Z80F] & 0x95) |
		(i&0x28) |
		((*z80RegsB[Z80B]) ? 2 : 0x42);
	return 4+5+3+4;
}

int z80INDR()			// INDR
{
	unsigned char i = z80PortReadB(*z80RegsB[Z80C]);	// read from input port C

	z80MemWriteB(z80Regs[Z80HL],i);
	z80Regs[Z80HL]--;	*z80RegsB[Z80B]-= 1;
	if (*z80RegsB[Z80B])
	{
		z80Regs[Z80PC]-= 2;
		return 4+5+3+4+5;
	}
	*z80RegsB[Z80F]|= 0x42;
	return 4+5+3+4;
}

int z80INIR()			// INIR
{
	unsigned char i = z80PortReadB(*z80RegsB[Z80C]);	// read from input port C

	z80MemWriteB(z80Regs[Z80HL],i);
	z80Regs[Z80HL]++;	*z80RegsB[Z80B]-= 1;
	if (*z80RegsB[Z80B])
	{
		z80Regs[Z80PC]-= 2;
		return 4+5+3+4+5;
	}
	*z80RegsB[Z80F]|= 0x42;
	return 4+5+3+4;
}

// undefined opcode
int z80Udef()
{
	return 0;
}

int (*z80decodeTableDDCB[256])()=
{
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RLCXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RRCXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RLXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RRXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SLAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SRAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SRLXY,	z80Udef,
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80BITXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80BITXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80BITXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80BITXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80BITXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80BITXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80BITXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80BITXY,	z80Udef,
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RESXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RESXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RESXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RESXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RESXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RESXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RESXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80RESXY,	z80Udef,
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SETXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SETXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SETXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SETXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SETXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SETXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SETXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SETXY,	z80Udef,
};

int z80decDDCB()
{
	Z80R++;
	z80d8 = (signed char)z80Readbyte();
	z80Opcode = z80Readbyte();
	return z80decodeTableDDCB[z80Opcode]();
}

int (*z80decodeTableCB[256])()=
{
	z80RLCr,	z80RLCr,	z80RLCr,	z80RLCr,	z80RLCr,	z80RLCr,	z80RLCHL,	z80RLCr,
	z80RRCr,	z80RRCr,	z80RRCr,	z80RRCr,	z80RRCr,	z80RRCr,	z80RRCHL,	z80RRCr,
	z80RLr,		z80RLr,		z80RLr,		z80RLr,		z80RLr,		z80RLr,		z80RLHL,	z80RLr,
	z80RRr,		z80RRr,		z80RRr,		z80RRr,		z80RRr,		z80RRr,		z80RRHL,	z80RRr,
	z80SLAr,	z80SLAr,	z80SLAr,	z80SLAr,	z80SLAr,	z80SLAr,	z80SLAHL,	z80SLAr,
	z80SRAr,	z80SRAr,	z80SRAr,	z80SRAr,	z80SRAr,	z80SRAr,	z80SRAHL,	z80SRAr,
	z80SLLr,	z80SLLr,	z80SLLr,	z80SLLr,	z80SLLr,	z80SLLr,	z80SLLHL,	z80SLLr,
	z80SRLr,	z80SRLr,	z80SRLr,	z80SRLr,	z80SRLr,	z80SRLr,	z80SRLHL,	z80SRLr,
	//
	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITHL,	z80BITr,
	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITHL,	z80BITr,
	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITHL,	z80BITr,
	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITHL,	z80BITr,
	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITHL,	z80BITr,
	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITHL,	z80BITr,
	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITHL,	z80BITr,
	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITr,	z80BITHL,	z80BITr,
	//
	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESHL,	z80RESr,
	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESHL,	z80RESr,
	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESHL,	z80RESr,
	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESHL,	z80RESr,
	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESHL,	z80RESr,
	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESHL,	z80RESr,
	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESHL,	z80RESr,
	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESr,	z80RESHL,	z80RESr,
	//
	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETHL,	z80SETr,
	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETHL,	z80SETr,
	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETHL,	z80SETr,
	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETHL,	z80SETr,
	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETHL,	z80SETr,
	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETHL,	z80SETr,
	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETHL,	z80SETr,
	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETr,	z80SETHL,	z80SETr,
};

int (*z80decodeTableDD[256])()=
{
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80ADDXYrr,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80ADDXYrr,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80LDXYnn,	z80LDnnXY,	z80INCXY,	z80INCr,	z80DECr,	z80LDrn,	z80Udef,
	z80Udef,	z80ADDXYrr,	z80LDXYnnm,	z80DECXY,	z80INCr,	z80DECr,	z80LDrn,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80INCXYm,	z80DECXYm,	z80LDXYn,	z80Udef,
	z80Udef,	z80ADDXYrr,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80LDrr,	z80LDrr,	z80LDrXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80LDrr,	z80LDrr,	z80LDrXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80LDrr,	z80LDrr,	z80LDrXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80LDrr,	z80LDrr,	z80LDrXY,	z80Udef,
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDHXY,	z80LDrr,
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDLXY,	z80LDrr,
	z80LDXYr,	z80LDXYr,	z80LDXYr,	z80LDXYr,	z80LDXYH,	z80LDXYL,	z80Udef,	z80LDXYr,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80LDrr,	z80LDrr,	z80LDrXY,	z80Udef,
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80ADDAr,	z80ADDAr,	z80ADDAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80ADCAr,	z80ADCAr,	z80ADCAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SUBAr,	z80SUBAr,	z80SUBAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80SBCAr,	z80SBCAr,	z80SBCAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80ANDAr,	z80ANDAr,	z80ANDAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80XORAr,	z80XORAr,	z80XORAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80ORAr,	z80ORAr,	z80ORAXY,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80CPAr,	z80CPAr,	z80CPAXY,	z80Udef,
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80decDDCB,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80POPXY,	z80Udef,	z80EXSPXY,	z80Udef,	z80PUSHXY,	z80Udef,	z80Udef,
	z80Udef,	z80JPXY,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80LDSPXY,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
};

int (*z80decodeTableED[256])()=
{
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	//
	z80INrC,	z80OUTCr,	z80SBCHLrr,	z80LDnnrr,	z80NEG,		z80RETN,	z80IM,		z80LDIA,
	z80INrC,	z80OUTCr,	z80ADCHLrr,	z80LDrrnnm,	z80Udef,	z80RETI,	z80Udef,	z80LDRA,
	z80INrC,	z80OUTCr,	z80SBCHLrr,	z80LDnnrr,	z80Udef,	z80Udef,	z80IM,		z80LDAI,
	z80INrC,	z80OUTCr,	z80ADCHLrr,	z80LDrrnnm,	z80Udef,	z80Udef,	z80IM,		z80LDAR,
	z80INrC,	z80OUTCr,	z80SBCHLrr,	z80LDnnrr,	z80Udef,	z80Udef,	z80Udef,	z80RRD,
	z80INrC,	z80OUTCr,	z80ADCHLrr,	z80LDrrnnm,	z80Udef,	z80Udef,	z80Udef,	z80RLD,
	z80Udef,	z80Udef,	z80SBCHLrr,	z80LDnnrr,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80INrC,	z80OUTCr,	z80ADCHLrr,	z80LDrrnnm,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80LDI,		z80CPI,		z80INI,		z80OUTI,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80LDD,		z80CPD,		z80IND,		z80OUTD,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80LDIR,	z80CPIR,	z80INIR,	z80OTIR,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80LDDR,	z80CPDR,	z80INDR,	z80OTDR,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	//
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,	z80Udef,
};

int z80decodeCB()
{
	Z80R++;
	z80Opcode = z80Readbyte();
	return z80decodeTableCB[z80Opcode]();
}

int z80decodeDD()
{
	Z80R++;
	z80Index = &z80Regs[Z80IX];
	z80RegsB[Z80H] = ((unsigned char *)&z80Regs[Z80IX])+1;
	z80RegsB[Z80L] = ((unsigned char *)&z80Regs[Z80IX]);
	z80Opcode = z80Readbyte();
	int retval = z80decodeTableDD[z80Opcode]() + 4;
	z80RegsB[Z80H] = ((unsigned char *)&z80Regs[Z80HL])+1;
	z80RegsB[Z80L] = ((unsigned char *)&z80Regs[Z80HL]);
	return retval;
}

int z80decodeED()
{
	Z80R++;
	z80Opcode = z80Readbyte();
	return z80decodeTableED[z80Opcode]();
}

int z80decodeFD()
{
	Z80R++;
	z80Index = &z80Regs[Z80IY];
	z80RegsB[Z80H] = ((unsigned char *)&z80Regs[Z80IY])+1;
	z80RegsB[Z80L] = ((unsigned char *)&z80Regs[Z80IY]);
	z80Opcode = z80Readbyte();
	int retval = z80decodeTableDD[z80Opcode]() + 4;
	z80RegsB[Z80H] = ((unsigned char *)&z80Regs[Z80HL])+1;
	z80RegsB[Z80L] = ((unsigned char *)&z80Regs[Z80HL]);
	return retval;
}

/// decode tables
int (*z80InstrTable[256])()=
{
	//
	z80NOP,		z80LDrrnn,	z80LDrrA,	z80INCrr,	z80INCr,	z80DECr,	z80LDrn,	z80RLCA,
	z80EXAFAF,	z80ADDHLrr,	z80LDArr,	z80DECrr,	z80INCr,	z80DECr,	z80LDrn,	z80RRCA,
	z80DJNZ,	z80LDrrnn,	z80LDrrA,	z80INCrr,	z80INCr,	z80DECr,	z80LDrn,	z80RLA,
	z80JR,		z80ADDHLrr,	z80LDArr,	z80DECrr,	z80INCr,	z80DECr,	z80LDrn,	z80RRA,
	z80JRcc,	z80LDrrnn,	z80LDmemHL,	z80INCrr,	z80INCr,	z80DECr,	z80LDrn,	z80DAA,
	z80JRcc,	z80ADDHLrr,	z80LDHLmem,	z80DECrr,	z80INCr,	z80DECr,	z80LDrn,	z80CPL,
	z80JRcc,	z80LDrrnn,	z80LDnnA,	z80INCrr,	z80INCHL,	z80DECHL,	z80LDHLn,	z80SCF,
	z80JRcc,	z80ADDHLrr,	z80LDAnn,	z80DECrr,	z80INCr,	z80DECr,	z80LDrn,	z80CCF,
	//
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrHL,	z80LDrr,
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrHL,	z80LDrr,
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrHL,	z80LDrr,
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrHL,	z80LDrr,
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrHL,	z80LDrr,
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrHL,	z80LDrr,
	z80LDHLr,	z80LDHLr,	z80LDHLr,	z80LDHLr,	z80LDHLr,	z80LDHLr,	z80HALT,	z80LDHLr,
	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrr,	z80LDrHL,	z80LDrr,
	//
	z80ADDAr,	z80ADDAr,	z80ADDAr,	z80ADDAr,	z80ADDAr,	z80ADDAr,	z80ADDAHL,	z80ADDAr,
	z80ADCAr,	z80ADCAr,	z80ADCAr,	z80ADCAr,	z80ADCAr,	z80ADCAr,	z80ADCAHL,	z80ADCAr,
	z80SUBAr,	z80SUBAr,	z80SUBAr,	z80SUBAr,	z80SUBAr,	z80SUBAr,	z80SUBAHL,	z80SUBAr,
	z80SBCAr,	z80SBCAr,	z80SBCAr,	z80SBCAr,	z80SBCAr,	z80SBCAr,	z80SBCAHL,	z80SBCAr,
	z80ANDAr,	z80ANDAr,	z80ANDAr,	z80ANDAr,	z80ANDAr,	z80ANDAr,	z80ANDAHL,	z80ANDAr,
	z80XORAr,	z80XORAr,	z80XORAr,	z80XORAr,	z80XORAr,	z80XORAr,	z80XORAHL,	z80XORAr,
	z80ORAr,	z80ORAr,	z80ORAr,	z80ORAr,	z80ORAr,	z80ORAr,	z80ORAHL,	z80ORAr,
	z80CPAr,	z80CPAr,	z80CPAr,	z80CPAr,	z80CPAr,	z80CPAr,	z80CPAHL,	z80CPAr,
	//
	z80RETcc,	z80POPrr,	z80JPcc,	z80JP,		z80CALLcc,	z80PUSHrr,	z80ADDAn,	z80RST,
	z80RETcc,	z80RET,		z80JPcc,	z80decodeCB,z80CALLcc,	z80CALL,	z80ADCAn,	z80RST,
	z80RETcc,	z80POPrr,	z80JPcc,	z80OUTn,	z80CALLcc,	z80PUSHrr,	z80SUBAn,	z80RST,
	z80RETcc,	z80EXX,		z80JPcc,	z80INn,		z80CALLcc,	z80decodeDD,z80SBCAn,	z80RST,
	z80RETcc,	z80POPrr,	z80JPcc,	z80EXSPHL,	z80CALLcc,	z80PUSHrr,	z80ANDAn,	z80RST,
	z80RETcc,	z80JPHL,	z80JPcc,	z80EXDEHL,	z80CALLcc,	z80decodeED,z80XORAn,	z80RST,
	z80RETcc,	z80POPAF,	z80JPcc,	z80DI,		z80CALLcc,	z80PUSHAF,	z80ORAn,	z80RST,
	z80RETcc,	z80LDSPHL,	z80JPcc,	z80EI,		z80CALLcc,	z80decodeFD,z80CPAn,	z80RST,
};

///// main entry funtionality

void z80Init()
{
//	switch (machine)
//	{
//	}
	z80Regs[Z80BC] = 0013;
	z80Regs[Z80DE] = 0x00D8;
	z80Regs[Z80HL] = 0x014D;
	// these ARE needed
	z80Regs[Z80SP] = 0xFFFE;
	z80Regs[Z80PC] = 0x0000;
	Z80IM = IMUNDEF;
	z80Running = 1;
	z80State = 0;
	z80HaltState = 0;
	Z80NMIPending = FALSE;
	Z80IntPending = 0;
	z80DebugIndex = 0;
}

void z80SetRunning(int running) {
	z80Running = running;
}

void z80orIFF(unsigned char bits)
{
    Z80IFF |= bits; // from Thor
}

void z80Interrupt(int kind) {
	if (kind == Z80NMI) {
		Z80NMIPending = TRUE;
	} else {
		if (!kind)
            Z80IntPending = 0x100;
		else
            Z80IntPending = kind;
	}
}

void z80UnInterrupt(int kind) {
	Z80IntPending = (Z80IntPending & ~kind);
}

inline void z80HandleInterrupt() {
	if (Z80NMIPending)
	{
		if (z80HaltState)
		{
			// leave halt state
			z80HaltState = 0;
			z80Regs[Z80PC]+= 1;
		}
		z80Regs[Z80SP]-= 2;
		z80MemWriteW(z80Regs[Z80SP],z80Regs[Z80PC]);
		z80Regs[Z80PC] = 0x0066;
		Z80IFF&= 2;
		Z80NMIPending = FALSE;
	}
	if (Z80IntPending) {
		if (Z80IFF&1) {
			if (z80HaltState)	{
				// leave halt state
				z80HaltState = 0;
				z80Regs[Z80PC]+= 1;
			}
			switch(Z80IM) {
				case IM0:
				// implemented, but untested
				z80Opcode = Z80IntPending & 0xFF;
				z80State+= z80InstrTable[z80Opcode]();
				break;
			case IMUNDEF:
				// undefined, not implemented
				break;
			case IM1:
				z80Regs[Z80SP]-= 2;
				z80MemWriteW(z80Regs[Z80SP],z80Regs[Z80PC]);
				z80Regs[Z80PC] = 0x0038;
				break;
			case IM2:
				// not implemented yet
				break;
			}
			Z80IntPending = 0;
		}
	}
}

#ifdef _DEBUG
int z80Step1()
{
#else
int z80Step()
{
#endif

	if (!z80Running)
        return 4;

	z80HandleInterrupt();
	Z80R++;
	if (z80HaltState)
	{
		return 4;		// NOP
	} else
	{
		z80Opcode = z80Readbyte();
		return z80InstrTable[z80Opcode]();
	}
}

#ifdef _DEBUG
int z80Step() {
#else
int z80Step1() {
#endif
/*	BOOL	res;
	//int		oldpc = z80Regs[Z80PC];

	switch (z80DebugIndex)
	{
	case 0:
		z80DebugCount = 2000000;
		z80DebugIndex = 1;
		return FALSE;
	case 1:
		res = z80Step1();
		z80DebugCount--;
//		if (z80Regs[Z80PC]==0x0302)	z80DebugIndex = 2;
		if (z80Regs[Z80PC]==0x0000) {
//			z80DebugIndex = 97;
		}
//		if (z80Regs[Z80PC]==0xA710)	z80DebugIndex = 97;			// Zool
//		if (ggVRAM[0x4004]==0x00)	z80DebugIndex = 97;
//		if (z80Regs[Z80PC]==0x032D)	z80DebugIndex = 97;
//		if (z80MemReadB(0xCA81)==0xB2)	z80DebugIndex = 97;
//		if (ggVRAM[0x3F18]==0x17)	z80DebugIndex = 97;
//		if (z80Regs[Z80PC] == 0x11ee)	z80DebugIndex = 97;		// columns
//		if (z80Regs[Z80PC] == 0x39c8)	z80DebugIndex = 97;
//		if (z80MemReadW(0xc100) == 0x9F9E)	z80DebugIndex = 97;
//		if (z80Regs[Z80PC] == 0x10b3)	z80DebugIndex = 2;
//		if (z80MemReadB(0xc018) != 0x00)	z80DebugIndex = 2;
//		if (z80Regs[Z80SP] == 0xBF96)	z80DebugIndex = 97;
//		if (ggVRAM[0x3AE5] == 0x19)	z80DebugIndex = 97;
//		if (!z80DebugCount)	z80DebugIndex = 97;
//		if (z80Regs[Z80PC] == 0x4D80) {
//		if (z80MemReadB(0) != 0xF3) {
//			z80DebugIndex = 97;
//		}
		return res;
	case 2:
		z80DebugCount = 2000;
		z80DebugIndex = 3;
		return 1;
	case 3:
		res = z80Step1();
		z80DebugCount--;
//		if (Z80IFF != 3)	z80DebugIndex= 97;
//		if (z80Regs[Z80PC] == 0x0338)	z80DebugIndex = 97;
//		if (z80Regs[Z80SP] <= 0xDF00) {
//		if (z80Regs[Z80PC] == 0x0000) {
//			if (errorLog) {
//				fprintf(errorLog,"enge PC : ");	write16(errorLog,oldpc);
//				fprintf(errorLog,"\n\n");
//			}
//			z80DebugIndex = 97;
//		}
		if (z80Regs[Z80PC] == 0x0A7C) {
			z80DebugIndex = 97;
		}
		return res;
	case 4:
		z80DebugCount = 2000;
		z80DebugIndex = 5;
		return 1;
	case 5:
		res = z80Step1();
		z80DebugCount--;
		if (z80Regs[Z80PC] == 0x10B3)	z80DebugIndex = 97;
		return res;
	case 97:
		z80DebugCount = 10000;
		z80DebugIndex = 98;
		if (errorLog != NULL)	z80Print(errorLog);
		return 1;
	case 98:
		res = z80Step1();
		if (errorLog != NULL)	z80Print(errorLog);
		z80DebugCount--;
		if (!z80DebugCount)	z80DebugIndex = 99;
		return res;
	case 99:
		fclose(errorLog);	errorLog = NULL;
		if (outputRam != NULL)
		{
			fwrite(get_address(0x00004000),1,80*1024,outputRam);	// moet veranderd worden!!!
			fclose(outputRam); outputRam = NULL;
		}
		AfxMessageBox("Done!");
		z80DebugIndex = 100;
		return 1;
	case 100:
		return 1;
	}
	return 1;*/

	return 0;
}

void z80Print(FILE *output)
{
	fprintf(output,"AF : %04X", z80Regs[Z80AF]);
	fprintf(output,"  BC : %04X", z80Regs[Z80BC]);
	fprintf(output,"  DE : %04X", z80Regs[Z80DE]);
	fprintf(output,"  HL : %04X", z80Regs[Z80HL]);
	fprintf(output,"\nAF': %04X", z80Regs[Z80AF2]);
	fprintf(output,"  BC': %04X", z80Regs[Z80BC2]);
	fprintf(output,"  DE': %04X", z80Regs[Z80DE2]);
	fprintf(output,"  HL': %04X", z80Regs[Z80HL2]);
	fprintf(output,"\nIX : %04X", z80Regs[Z80IX]);
	fprintf(output,"  IY : %04X", z80Regs[Z80IY]);
	fprintf(output,"  IFF: %04X", Z80IFF);
	fprintf(output,"  IM : %04X", Z80IM);
	fprintf(output,"\nPC : %04X", z80Regs[Z80PC]);
	fprintf(output,"  SP : %04X", z80Regs[Z80SP]);
	fprintf(output,"\n");
	fprintf(output,"\nNext instruction : %02X%02X%02X"
		, z80MemReadB(z80Regs[Z80PC])
		, z80MemReadB(z80Regs[Z80PC]+1)
		, z80MemReadB(z80Regs[Z80PC]+2));
	fprintf(output,"\n\n");
}

