//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------


#ifndef _MEMORYH_
#define _MEMORYH_

#include "StdAfx.h"
#include "neopopsound.h"
#include "sound.h"
#include "input.h"
#include "graphics.h"
#include "main.h"
#include "flash.h"

#ifdef DRZ80
#include "DrZ80_support.h"
#else
#ifdef CZ80
#include "cz80_support.h"
#else
#include "z80.h"
#endif
#endif


extern unsigned char *mainram;			// All RAM areas
extern unsigned char *mainrom;			// ROM image area
extern unsigned char *cpurom;			// Bios ROM image area
extern unsigned char *cpuram;
//extern unsigned long cartAddrMask;					// Mask for reading/writing mainrom

/// tlcs 900h memory
unsigned char tlcsMemReadB(unsigned long addr);
unsigned short tlcsMemReadW(unsigned long addr);
unsigned long tlcsMemReadL(unsigned long addr);
void tlcsMemWriteB(unsigned long addr, unsigned char data);
void tlcsMemWriteW(unsigned long addr, unsigned short data);
void tlcsMemWriteL(unsigned long addr, unsigned long data);
void mem_init();
void mem_dump_ram(FILE *output);

//unsigned char *get_address(unsigned long addr);
//unsigned char *get_addressW(unsigned long addr);

/// gameboy memory

extern unsigned char (*gbMemReadB)(unsigned short addr);
extern void (*gbMemWriteB)(unsigned short addr, unsigned char data);
unsigned short gbMemReadW(unsigned short addr);
void gbMemWriteW(unsigned short addr, unsigned short data);

// z80 memory functions

extern unsigned char (*z80MemReadB)(unsigned short addr);
extern unsigned short (*z80MemReadW)(unsigned short addr);
extern void (*z80MemWriteB)(unsigned short addr, unsigned char data);
extern void (*z80MemWriteW)(unsigned short addr, unsigned short data);
extern void (*z80PortWriteB)(unsigned char port, unsigned char data);
extern unsigned char (*z80PortReadB)(unsigned char port);

#if defined(DRZ80) || defined(CZ80)
unsigned char z80ngpMemReadB(unsigned short addr);
unsigned short z80ngpMemReadW(unsigned short addr);
void DrZ80ngpMemWriteB(unsigned char data, unsigned short addr);
void DrZ80ngpMemWriteW(unsigned short data, unsigned short addr);
void DrZ80ngpPortWriteB(unsigned short port, unsigned char data);
unsigned char DrZ80ngpPortReadB(unsigned short port);
#endif

// used internally by z80 emulation for emulation of the memory
// map of the Game Gear
extern unsigned char	*ggVRAM;
extern unsigned char	*ggCRAM;
//
//
unsigned char z80ggMemReadB(unsigned short addr);
unsigned short z80ggMemReadW(unsigned short addr);
void z80ggMemWriteB(unsigned short addr, unsigned char data);
void z80ggMemWriteW(unsigned short addr, unsigned short data);
void z80ggPortWriteB(unsigned char port, unsigned char data);
unsigned char z80ggPortReadB(unsigned char port);

// definitions for wdc6502 emulation
extern unsigned char (*w65MemReadB)(unsigned short addr);
//extern unsigned short (*w65MemReadW)(unsigned short addr);
unsigned short w65MemReadW(unsigned short addr);
extern void (*w65MemWriteB)(unsigned short addr, unsigned char data);
//extern void (*w65MemWriteW)(unsigned short addr, unsigned short data);
void w65MemWriteW(unsigned short addr, unsigned short data);
extern unsigned char *w65GetAddress(unsigned short addr);

//
// Supervision specific
//
extern unsigned char	*svVRAM;

//
// HuC specific defines & externs
//

extern unsigned char	hucMMR[8];

//
// i8086 functions
//

extern unsigned char	*x86Ports;

unsigned char x86MemReadB(unsigned int addr);
void x86MemWriteB(unsigned int addr, unsigned char data);
unsigned short x86MemReadW(unsigned int addr);
void x86MemWriteW(unsigned int addr, unsigned short data);
unsigned char x86PortReadB(unsigned short port);
void x86PortWriteB(unsigned short port, unsigned char data);
unsigned short x86PortReadW(unsigned short port);
void x86PortWriteW(unsigned short port, unsigned short data);

// used for Wonderswan graphics emulation

extern unsigned char	*wsTileMap0;
extern unsigned char	*wsTileMap1;
extern unsigned char	*wsPatterns;
extern unsigned char	*wsSpriteTable;

//
// i8048 functions
//

extern unsigned char	*advBios;
extern unsigned char	*advVidRAM;

unsigned char i8048ROMReadB(unsigned int addr);
unsigned char i8048ExtReadB(unsigned int addr);
void i8048ExtWriteB(unsigned int addr, unsigned char data);
unsigned char i8048PortReadB(unsigned char port);
void i8048PortWriteB(unsigned char port, unsigned char data);
void i8048SetT1();
unsigned char i8048GetT1();

//
// sm85xx memory functions
//

extern unsigned char (*sm85MemReadB)(unsigned short addr);
extern void (*sm85MemWriteB)(unsigned short addr, unsigned char data);

inline unsigned char *get_address(unsigned long addr)
{
	addr&= 0x00FFFFFF;
	if (addr<0x00200000)
	{
		if (addr<0x000008a0)
		{
		    //if(((unsigned long)&cpuram[addr] >> 24) == 0xFF)
            //    dbg_print("1) addr=0x%X returning=0x%X\n", addr, &cpuram[addr]);
			return &cpuram[addr];
		}
		if (addr>0x00003fff && addr<0x00018000)
        {
            //if((unsigned long)&mainram[addr-0x00004000] >> 24 == 0xFF)
            //    dbg_print("2) addr=0x%X returning=0x%X\n", addr, &mainram[addr-0x00004000]);

            switch (addr)  //Thanks Koyote
            {
                case 0x6F80:
                    mainram[addr-0x00004000] = 0xFF;
                    break;
                case 0x6F80+1:
                    mainram[addr-0x00004000] = 0x03;
                    break;
                case 0x6F85:
                    mainram[addr-0x00004000] = 0x00;
                    break;
                case 0x6F82:
                    mainram[addr-0x00004000] = ngpInputState;
                    break;
                case 0x6DA2:
                    mainram[addr-0x00004000] = 0x80;
                    break;
            }
            return &mainram[addr-0x00004000];
        }
	}
	else
	{
		if (addr<0x00400000)
		{
            return &mainrom[(addr-0x00200000)/*&cartAddrMask*/];
		}
        if(addr<0x00800000) //Flavor added
		{
            return 0;
		}
		if (addr<0x00A00000)
		{
            return &mainrom[(addr-(0x00800000-0x00200000))/*&cartAddrMask*/];
		}
		if(addr<0x00FF0000) //Flavor added
		{
            return 0;
		}

        //if((unsigned long)&cpurom[addr-0x00ff0000] >> 24 == 0xFF)
        //   dbg_print("5) addr=0x%X returning=0x%X\n", addr, &cpurom[addr-0x00ff0000]);

        return &cpurom[addr-0x00ff0000];
    }

	//dbg_print("6) addr=0x%X returning=0\n", addr);
	return 0;  //Flavor ERROR
}

/*inline unsigned char *get_addressW(unsigned long addr)
{
	addr&= 0x00FFFFFF;
	if (addr<0x00200000)
	{
		if (addr<0x000008a0)
		{
			return &cpuram[addr];
		}
		if (addr>0x00003fff && addr<0x00018000)
            return &mainram[addr-0x00004000];

        return NULL;
	}
	else
	{
		return NULL;
	}
	return NULL;  //Flavor ERROR
}*/

// read a byte from a memory address (addr)
inline unsigned char tlcsMemReadB(unsigned long addr)
{
	addr&= 0x00FFFFFF;

	if(currentCommand == COMMAND_INFO_READ)
        return flashReadInfo(addr);

	if (addr < 0x00200000) {
		if (addr < 0x000008A0) {
			if(addr == 0xBC)
			{
				ngpSoundExecute();
			}
			return cpuram[addr];
		}
		else if (addr > 0x00003FFF && addr < 0x00018000)
        {

            switch (addr)  //Thanks Koyote
            {
                case 0x6DA2:
                    return 0x80;
                    break;
                case 0x6F80:
                    return 0xFF;
                    break;
                case 0x6F80+1:
                    return 0x03;
                    break;
                case 0x6F85:
                    return 0x00;
                    break;
                case 0x6F82:
                    return ngpInputState;
                    break;

                default:
                    return mainram[addr-0x00004000];
            }
            return mainram[addr-0x00004000];
        }
	}
	else
	{
		if (addr<0x00400000)
            return mainrom[(addr-0x00200000)/*&cartAddrMask*/];
		if (addr<0x00800000)
            return 0xFF;
		if (addr<0x00a00000)
            return mainrom[(addr-(0x00800000-0x00200000))/*&cartAddrMask*/];
		if (addr<0x00ff0000)
            return 0xFF;
		return cpurom[addr-0x00ff0000];
	}
	return 0xFF;
}

// read a word from a memory address (addr)
inline unsigned short tlcsMemReadW(unsigned long addr)
{
#ifdef TARGET_GP2X
	register unsigned short i asm("r0");
	register byte *gA asm("r1");

	gA = get_address(addr);

    if(gA == 0)
        return 0;

    asm volatile(
    "ldrb	%0, [%1]\n\t"
    "ldrb	r2, [%1, #1]\n\t"
    "orr	%0, %0, r2, asl #8"
    : "=r" (i)
    : "r" (gA)
    : "r2");

    return i;
#else
/*	unsigned short i;
	byte *gA = get_address(addr);

    if(gA == 0)
        return 0;

    i = *(gA++);
    i |= (*gA) << 8;

	return i;
	*/
	return tlcsMemReadB(addr) | (tlcsMemReadB(addr+1) << 8);
#endif
}


// read a long word from a memory address (addr)
inline unsigned long tlcsMemReadL(unsigned long addr)
{
#ifdef TARGET_GP2X
	register unsigned long i asm("r0");
	register byte *gA asm("r4");

	gA = get_address(addr);

    if(gA == 0)
        return 0;

    asm volatile(
    "bic	r1,%1,#3	\n"
    "ldmia	r1,{r0,r3}	\n"
    "ands	r1,%1,#3	\n"
    "movne	r2,r1,lsl #3	\n"
    "movne	r0,r0,lsr r2	\n"
    "rsbne	r1,r2,#32	\n"
    "orrne	r0,r0,r3,lsl r1"
    : "=r"(i)
    : "r"(gA)
    : "r1","r2","r3");

    return i;
#else
	unsigned long i;
	byte *gA = get_address(addr);

    if(gA == 0)
        return 0;

    i = *(gA++);
    i |= (*(gA++)) << 8;
    i |= (*(gA++)) << 16;
    i |= (*gA) << 24;

	return i;
	//return tlcsMemReadB(addr) | (tlcsMemReadB(addr +1) << 8) | (tlcsMemReadB(addr +2) << 16) | (tlcsMemReadB(addr +3) << 24);
#endif
}

// write a byte (data) to a memory address (addr)
inline void tlcsMemWriteB(unsigned long addr, unsigned char data)
{
	addr&= 0x00FFFFFF;
    if (addr<0x000008a0)
    {
        switch(addr) {
        //case 0x80:	// CPU speed
        //    break;
        case 0xA0:	// L CH Sound Source Control Register
            if (cpuram[0xB8] == 0x55 && cpuram[0xB9] == 0xAA)
                Write_SoundChipNoise(data);//Flavor SN76496Write(0, data);
            break;
        case 0xA1:	// R CH Sound Source Control Register
            if (cpuram[0xB8] == 0x55 && cpuram[0xB9] == 0xAA)
                Write_SoundChipTone(data); //Flavor SN76496Write(0, data);
            break;
        case 0xA2:	// L CH DAC Control Register
            ngpSoundExecute();
            if (cpuram[0xB8] == 0xAA)
                dac_writeL(data); //Flavor DAC_data_w(0,data);
            break;
        /*case 0xA3:	// R CH DAC Control Register  //Flavor hack for mono only sound
            ngpSoundExecute();
            if (cpuram[0xB8] == 0xAA)
                dac_writeR(data);//Flavor DAC_data_w(1,data);
            break;*/
        case 0xB8:	// Z80 Reset
//				if (data == 0x55)	DAC_data_w(0,0);
        case 0xB9:	// Sourd Source Reset Control Register
            switch(data) {
            case 0x55:
                ngpSoundStart();
                break;
            case 0xAA:
                ngpSoundExecute();
                ngpSoundOff();
                break;
            }
            break;
        case 0xBA:
            ngpSoundExecute();
#if defined(DRZ80) || defined(CZ80)
            Z80_Cause_Interrupt(Z80_NMI_INT);
#else
            z80Interrupt(Z80NMI);
#endif
            break;
        }
        cpuram[addr] = data;
        return;
    }
    else if (addr>0x00003fff && addr<0x00018000)
    {
        if (addr == 0x87E2 && mainram[0x47F0] != 0xAA)
            return;		// disallow writes to GEMODE

        /*if((addr >= 0x8800 && addr <= 0x88FF)
            || (addr >= 0x8C00 && addr <= 0x8FFF)
            || (addr >= 0xA000 && addr <= 0xBFFF)
            || addr == 0x00008020
            || addr == 0x00008021)
        {
            spritesDirty = true;
        }*/

        mainram[addr-0x00004000] = data;
        return;
    }
    else if (addr>=0x00200000 && addr<0x00400000)
        flashChipWrite(addr, data);
    else if (addr>=0x00800000 && addr<0x00A00000)
        flashChipWrite(addr, data);

}





#endif  // _MEMORYH_
