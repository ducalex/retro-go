//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

// memory.cpp: implementation of the memory class.
//
// Quick & dirty implementation; no rom protection etc.
//
// TODO:
//
//////////////////////////////////////////////////////////////////////

#ifndef __GP32__
#include "StdAfx.h"
#endif
#include "main.h"
#include "memory.h"
#include "input.h"	  // for Gameboy Input
#include "graphics.h" // for i/o ports of the game gear
//#include "mainemu.h"
#include "sound.h"
//#include "z80.h"
#include "tlcs900h.h"
#include "koyote_bin.h"

#ifdef DRZ80
#include "DrZ80_support.h"
/*#else
#if defined(CZ80)
#include "cz80_support.h"
#else
#include "z80.h"*/
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

unsigned char *mainram;
unsigned char *mainrom;
unsigned char *cpurom;
unsigned char *cpuram;

//
// preliminary address map:
//
// 0x00000000 - 0x000000ff : cpuram
// 0x00004000 - 0x0000ffff : ram
// 0x00200000 - 0x003fffff : rom (lower 16Mbit)
// 0x00800000 - 0x009fffff : rom (higher 16Mbit)
// 0x00fffe00 - 0x00ffffff : other ram/rom
//

///////////////////////////////////////////////////////////////////
//
//// z80 memory instructions
//
///////////////////////////////////////////////////////////////////

// function place holders
unsigned char (*z80MemReadB)(unsigned short addr);
unsigned short (*z80MemReadW)(unsigned short addr);
void (*z80MemWriteB)(unsigned short addr, unsigned char data);
void (*z80MemWriteW)(unsigned short addr, unsigned short data);
void (*z80PortWriteB)(unsigned char port, unsigned char data);
unsigned char (*z80PortReadB)(unsigned char port);

////////////////////////////////////////////////////////////////////////////////////
///
/// Neogeo Pocket z80 functions
///
////////////////////////////////////////////////////////////////////////////////////

unsigned char z80ngpMemReadB(unsigned short addr)
{
	unsigned char temp;
	if (addr < 0x4000)
	{
		return mainram[0x3000 + addr];
	}
	switch (addr)
	{
	case 0x4000: // sound chip read
		break;
	case 0x4001:
		break;
	case 0x8000:
		temp = cpuram[0xBC];
		return temp;
	case 0xC000:
		break;
	}
	return 0x00;
}

unsigned short z80ngpMemReadW(unsigned short addr)
{
	return (z80ngpMemReadB(addr + 1) << 8) | z80ngpMemReadB(addr);
}

void z80ngpMemWriteB(unsigned short addr, unsigned char data)
{
	if (addr < 0x4000)
	{
		mainram[0x3000 + addr] = data;
		return;
	}

	switch (addr)
	{
	case 0x4000:
		Write_SoundChipNoise(data); //Flavor SN76496Write(0, data);
		return;
	case 0x4001:
		Write_SoundChipTone(data); //Flavor SN76496Write(0, data);
		return;
	case 0x8000:
		cpuram[0xBC] = data;
		return;
	case 0xC000:
		tlcs_interrupt_wrapper(0x03);
		return;
	}
}

void z80ngpMemWriteW(unsigned short addr, unsigned short data)
{
	if (addr < 0x4000)
	{
		mainram[0x3000 + addr] = data & 0xFF;
		mainram[0x3000 + addr + 1] = data >> 8;
		return;
	}

	switch (addr)
	{
	case 0x4000:
		Write_SoundChipNoise(data & 0xFF); //Flavor SN76496Write(0, data);
		Write_SoundChipNoise(data >> 8);   //Flavor SN76496Write(0, data);
		return;
	case 0x4001:
		Write_SoundChipTone(data & 0xFF); //Flavor SN76496Write(0, data);
		Write_SoundChipTone(data >> 8);	  //Flavor SN76496Write(0, data);
		return;
	case 0x8000:
		cpuram[0xBC] = data & 0xFF;
		cpuram[0xBC] = data >> 8;
		return;
	case 0xC000:
		tlcs_interrupt_wrapper(0x03);
		tlcs_interrupt_wrapper(0x03);
		return;
	}
}

void z80ngpPortWriteB(unsigned char port, unsigned char data)
{
	// acknowledge interrupt, any port works
}

unsigned char z80ngpPortReadB(unsigned char port)
{
	return 0xFF;
}

#if defined(DRZ80) || defined(CZ80)
void DrZ80ngpMemWriteB(unsigned char data, unsigned short addr)
{
	if (addr < 0x4000)
	{
		mainram[0x3000 + addr] = data;
		return;
	}
	switch (addr)
	{
	case 0x4000:
		Write_SoundChipNoise(data); //Flavor SN76496Write(0, data);
		return;
	case 0x4001:
		Write_SoundChipTone(data); //Flavor SN76496Write(0, data);
		return;
	case 0x8000:
		cpuram[0xBC] = data;
		return;
	case 0xC000:
		tlcs_interrupt_wrapper(0x03);
		return;
	}
}

void DrZ80ngpMemWriteW(unsigned short data, unsigned short addr)
{
	if (addr < 0x4000)
	{
		mainram[0x3000 + addr] = data & 0xFF;
		mainram[0x3000 + addr + 1] = data >> 8;
		return;
	}

	switch (addr)
	{
	case 0x4000:
		Write_SoundChipNoise(data & 0xFF); //Flavor SN76496Write(0, data);
		Write_SoundChipNoise(data >> 8);   //Flavor SN76496Write(0, data);
		return;
	case 0x4001:
		Write_SoundChipTone(data & 0xFF); //Flavor SN76496Write(0, data);
		Write_SoundChipTone(data >> 8);	  //Flavor SN76496Write(0, data);
		return;
	case 0x8000:
		cpuram[0xBC] = data & 0xFF;
		cpuram[0xBC] = data >> 8;
		return;
	case 0xC000:
		tlcs_interrupt_wrapper(0x03);
		tlcs_interrupt_wrapper(0x03);
		return;
	}
}

void DrZ80ngpPortWriteB(unsigned short port, unsigned char data)
{
	// acknowledge interrupt, any port works
}

unsigned char DrZ80ngpPortReadB(unsigned short port)
{
	return 0xFF;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
//// General Functions                                                            ///
/////////////////////////////////////////////////////////////////////////////////////

const unsigned char ngpcpuram[256] = {
	// 0x00
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0xFF, 0xFF,
	// 0x10
	0x34, 0x3C, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x3F, 0xFF, 0x2D, 0x01, 0xFF, 0xFF, 0x03, 0xB2,
	// 0x20
	0x80, 0x00, 0x01, 0x90, 0x03, 0xB0, 0x90, 0x62, 0x05, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x4C, 0x4C,
	// 0x30
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x20, 0xFF, 0x80, 0x7F,
	// 0x40
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x50
	0x00, 0x20, 0x69, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	// 0x60
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x17, 0x03, 0x03, 0x02, 0x00, 0x00, 0x00,
	// 0x70 (0x70-0x7A: interrupt level settings)
	0x02, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char ngpInterruptCode[] = {
	0x07,					// RETI
	0xD1, 0xBA, 0x6F, 0x04, // PUSHW (6FBA)	FFF801	SWI3
	0xD1, 0xB8, 0x6F, 0x04, // PUSHW (6FB8)
	0x0E,					// RET
	0xD1, 0xBE, 0x6F, 0x04, // PUSHW (6FBE) FFF80A	SWI4
	0xD1, 0xBC, 0x6F, 0x04, // PUSHW (6FBC)
	0x0E,					// RET
	0xD1, 0xC2, 0x6F, 0x04, // PUSHW (6FC2)	FFF813	SWI5
	0xD1, 0xC0, 0x6F, 0x04, // PUSHW (6FC0)
	0x0E,					// RET
	0xD1, 0xC6, 0x6F, 0x04, // PUSHW (6FC6)	FFF81C	SWI6
	0xD1, 0xC4, 0x6F, 0x04, // PUSHW (6FC4)
	0x0E,					// RET
	0xD1, 0xCA, 0x6F, 0x04, // PUSHW (6FCA)	FFF825	RTC Alarm
	0xD1, 0xC8, 0x6F, 0x04, // PUSHW (6FC8)
	0x0E,					// RET
	0xD1, 0xCE, 0x6F, 0x04, // PUSHW (6FCE)	FFF82E	VBlank
	0xD1, 0xCC, 0x6F, 0x04, // PUSHW (6FCC)
	0x0E,					// RET
	0xD1, 0xD2, 0x6F, 0x04, // PUSHW (6FD2)	FFF837	interrupt from z80
	0xD1, 0xD0, 0x6F, 0x04, // PUSHW (6FD0)
	0x0E,					// RET
	0xD1, 0xD6, 0x6F, 0x04, // PUSHW (6FD6)	FFF840	INTT0
	0xD1, 0xD4, 0x6F, 0x04, // PUSHW (6FD4)
	0x0E,					// RET
	0xD1, 0xDA, 0x6F, 0x04, // PUSHW (6FDA)	FFF849	INTT1
	0xD1, 0xD8, 0x6F, 0x04, // PUSHW (6FD8)
	0x0E,					// RET
	0xD1, 0xDE, 0x6F, 0x04, // PUSHW (6FDE)	FFF852	INTT2
	0xD1, 0xDC, 0x6F, 0x04, // PUSHW (6FDC)
	0x0E,					// RET
	0xD1, 0xE2, 0x6F, 0x04, // PUSHW (6FE2)	FFF85B	INTT3 (interrupt to z80)
	0xD1, 0xE0, 0x6F, 0x04, // PUSHW (6FE0)
	0x0E,					// RET
	0xD1, 0xE6, 0x6F, 0x04, // PUSHW (6FE6)	FFF864	Serial Receive
	0xD1, 0xE4, 0x6F, 0x04, // PUSHW (6FE4)
	0x0E,					// RET
	0xD1, 0xEA, 0x6F, 0x04, // PUSHW (6FEA)	FFF86D	Serial Communication
	0xD1, 0xE8, 0x6F, 0x04, // PUSHW (6FE8)
	0x0E,					// RET
	0xD1, 0xEE, 0x6F, 0x04, // PUSHW (6FEE)	FFF876	Reserved
	0xD1, 0xEC, 0x6F, 0x04, // PUSHW (6FEC)
	0x0E,					// RET
	0xD1, 0xF2, 0x6F, 0x04, // PUSHW (6FF2)	FFF87F	End DMA Channel 0
	0xD1, 0xF0, 0x6F, 0x04, // PUSHW (6FF0)
	0x0E,					// RET
	0xD1, 0xF6, 0x6F, 0x04, // PUSHW (6FF6)	FFF888	End DMA Channel 1
	0xD1, 0xF4, 0x6F, 0x04, // PUSHW (6FF4)
	0x0E,					// RET
	0xD1, 0xFA, 0x6F, 0x04, // PUSHW (6FFA)	FFF891	End DMA Channel 2
	0xD1, 0xF8, 0x6F, 0x04, // PUSHW (6FF8)
	0x0E,					// RET
	0xD1, 0xFE, 0x6F, 0x04, // PUSHW (6FFE)	FFF89A	End DMA Channel 3
	0xD1, 0xFC, 0x6F, 0x04, // PUSHW (6FFC)
	0x0E,					// RET
};

const unsigned long ngpVectors[0x21] = {
	0x00FFF800, 0x00FFF000, 0x00FFF800, 0x00FFF801, // 00, 04, 08, 0C
	0x00FFF80A, 0x00FFF813, 0x00FFF81C, 0x00FFF800, // 10, 14, 18, 1C
	0x00FFF800, 0x00FFF800, 0x00FFF825, 0x00FFF82E, // 20, 24, 28, 2C
	0x00FFF837, 0x00FFF800, 0x00FFF800, 0x00FFF800, // 30, 34, 38, 3C
	0x00FFF840, 0x00FFF849, 0x00FFF852, 0x00FFF85B, // 40, 44, 48, 4C
	0x00FFF800, 0x00FFF800, 0x00FFF800, 0x00FFF800, // 50, 54, 58, 5C
	0x00FFF864, 0x00FFF86D, 0x00FFF800, 0x00FFF800, // 60, 64, 68, 6C
	0x00FFF800, 0x00FFF87F, 0x00FFF888, 0x00FFF891, // 70, 74, 78, 7C
	0x00FFF89A										// 80
};

void mem_init()
{
	int x;

	mainram = (unsigned char *)rg_alloc((64 + 32 + 128) * 1024, MEM_SLOW);
	mainrom = (unsigned char *)rg_alloc(2 * 1024 * 1024, MEM_SLOW);
	cpurom = (unsigned char *)rg_alloc(256 * 1024, MEM_SLOW);
	cpuram = &mainram[128 * 1024];

	//in theory, if we've loaded the BIOS, we shouldn't need much of this, but good luck with that.
	// setup fake jump table for the additional bios calls
	for (int i = 0; i < 0x40; i++)
	{
		// using the 1A (reg) dummy instruction to emulate the bios
		cpurom[0xe000 + 0x40 * i] = 0xc8;
		cpurom[0xe001 + 0x40 * i] = 0x1a;
		cpurom[0xe002 + 0x40 * i] = i;
		cpurom[0xe003 + 0x40 * i] = 0x0e;
		*((unsigned long *)(&cpurom[0xfe00 + 4 * i])) = (unsigned long)0x00ffe000 + 0x40 * i;
	}

	// setup SWI 1 code & vector
	x = 0xf000;
	cpurom[x] = 0x17;
	x++;
	cpurom[x] = 0x03;
	x++; // ldf 3
	cpurom[x] = 0x3C;
	x++; // push XIX
	cpurom[x] = 0xC8;
	x++;
	cpurom[x] = 0xCC;
	x++; // and w,1F
	cpurom[x] = 0x1F;
	x++;
	cpurom[x] = 0xC8;
	x++;
	cpurom[x] = 0x80;
	x++; // add w,w
	cpurom[x] = 0xC8;
	x++;
	cpurom[x] = 0x80;
	x++; // add w,w
	cpurom[x] = 0x44;
	x++;
	cpurom[x] = 0x00;
	x++; // ld XIX,0x00FFFE00
	cpurom[x] = 0xFE;
	x++;
	cpurom[x] = 0xFF;
	x++; //
	cpurom[x] = 0x00;
	x++; //
	cpurom[x] = 0xE3;
	x++;
	cpurom[x] = 0x03;
	x++; // ld XIX,(XIX+W)
	cpurom[x] = 0xF0;
	x++;
	cpurom[x] = 0xE1;
	x++; //
	cpurom[x] = 0x24;
	x++; //
	cpurom[x] = 0xB4;
	x++;
	cpurom[x] = 0xE8;
	x++; // call XIX
	cpurom[x] = 0x5C;
	x++; // pop XIX
	cpurom[x] = 0x07;
	x++; // reti
	*((unsigned long *)(&cpurom[0xff04])) = (unsigned long)0x00fff000;
	// setup interrupt code
	for (int i = 0; i < sizeof(ngpInterruptCode); i++)
	{
		cpurom[0xf800 + i] = ngpInterruptCode[i];
	}
	// setup interrupt vectors
	for (int i = 0; i < sizeof(ngpVectors) / 4; i++)
	{
		*((unsigned long *)(&cpurom[0xff00 + 4 * i])) = ngpVectors[i];
	}

	// setup the additional CPU ram
	// interrupt priorities, timer settings, transfer settings, etc
	memcpy(cpuram, ngpcpuram, 256);

	//koyote.bin handling
	memcpy(mainram, koyote_bin, KOYOTE_BIN_SIZE /*12*1024*/);

	// setup interrupt vectors in RAM
	for (int i = 0; i < 18; i++)
	{
		*((unsigned long *)(&mainram[0x2FB8 + 4 * i])) = (unsigned long)0x00FFF800;
	}
	mainram[0x6F80 - 0x4000] = 0xFF; //Lots of battery power!
	mainram[0x6F81 - 0x4000] = 0x03;
	mainram[0x6F95 - 0x4000] = 0x10;
	mainram[0x6F91 - 0x4000] = 0x10; // Colour bios
	mainram[0x6F84 - 0x4000] = 0x40; // "Power On" startup
	mainram[0x6F85 - 0x4000] = 0x00; // No shutdown request
	mainram[0x6F86 - 0x4000] = 0x00; // No user answer (?)
	mainram[0x6F87 - 0x4000] = 0x01; //English

	mainram[0x4000] = 0xC0; // Enable generation of VBlanks by default
	mainram[0x4004] = 0xFF;
	mainram[0x4005] = 0xFF;
	mainram[0x4006] = 0xC6;
	for (int i = 0; i < 5; i++)
	{
		mainram[0x4101 + 4 * i] = 0x07;
		mainram[0x4102 + 4 * i] = 0x07;
		mainram[0x4103 + 4 * i] = 0x07;
	}
	mainram[0x4118] = 0x07;
	mainram[0x43E0] = mainram[0x43F0] = 0xFF;
	mainram[0x43E1] = mainram[0x43F1] = 0x0F;
	mainram[0x43E2] = mainram[0x43F2] = 0xDD;
	mainram[0x43E3] = mainram[0x43F3] = 0x0D;
	mainram[0x43E4] = mainram[0x43F4] = 0xBB;
	mainram[0x43E5] = mainram[0x43F5] = 0x0B;
	mainram[0x43E6] = mainram[0x43F6] = 0x99;
	mainram[0x43E7] = mainram[0x43F7] = 0x09;
	mainram[0x43E8] = mainram[0x43F8] = 0x77;
	mainram[0x43E9] = mainram[0x43F9] = 0x07;
	mainram[0x43EA] = mainram[0x43FA] = 0x44;
	mainram[0x43EB] = mainram[0x43FB] = 0x04;
	mainram[0x43EC] = mainram[0x43FC] = 0x33;
	mainram[0x43ED] = mainram[0x43FD] = 0x03;
	mainram[0x43EE] = mainram[0x43FE] = 0x00;
	mainram[0x43EF] = mainram[0x43FF] = 0x00;

	// Setup z80 functions
	z80MemReadB = z80ngpMemReadB;
	z80MemReadW = z80ngpMemReadW;
	z80MemWriteB = z80ngpMemWriteB;
	z80MemWriteW = z80ngpMemWriteW;
	z80PortWriteB = z80ngpPortWriteB;
	z80PortReadB = z80ngpPortReadB;
}

void mem_dump_ram(FILE *output)
{
	fwrite(mainram, 1, sizeof(mainram), output);
}
