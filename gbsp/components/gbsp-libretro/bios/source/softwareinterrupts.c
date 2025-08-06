/*
  Custom GBA BIOS replacement
  Copyright (c) 2002-2006 VBA Development Team
  Copyright (c) 2006-2013 VBA-M Development Team
  Copyright (c) 2013 Normmatt
	
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <tonc.h>

#define HALTCNT     (*(vu8*) 0x04000301)

const s16 sineTable[256] = {
  (s16)0x0000, (s16)0x0192, (s16)0x0323, (s16)0x04B5, (s16)0x0645, (s16)0x07D5, (s16)0x0964, (s16)0x0AF1,
  (s16)0x0C7C, (s16)0x0E05, (s16)0x0F8C, (s16)0x1111, (s16)0x1294, (s16)0x1413, (s16)0x158F, (s16)0x1708,
  (s16)0x187D, (s16)0x19EF, (s16)0x1B5D, (s16)0x1CC6, (s16)0x1E2B, (s16)0x1F8B, (s16)0x20E7, (s16)0x223D,
  (s16)0x238E, (s16)0x24DA, (s16)0x261F, (s16)0x275F, (s16)0x2899, (s16)0x29CD, (s16)0x2AFA, (s16)0x2C21,
  (s16)0x2D41, (s16)0x2E5A, (s16)0x2F6B, (s16)0x3076, (s16)0x3179, (s16)0x3274, (s16)0x3367, (s16)0x3453,
  (s16)0x3536, (s16)0x3612, (s16)0x36E5, (s16)0x37AF, (s16)0x3871, (s16)0x392A, (s16)0x39DA, (s16)0x3A82,
  (s16)0x3B20, (s16)0x3BB6, (s16)0x3C42, (s16)0x3CC5, (s16)0x3D3E, (s16)0x3DAE, (s16)0x3E14, (s16)0x3E71,
  (s16)0x3EC5, (s16)0x3F0E, (s16)0x3F4E, (s16)0x3F84, (s16)0x3FB1, (s16)0x3FD3, (s16)0x3FEC, (s16)0x3FFB,
  (s16)0x4000, (s16)0x3FFB, (s16)0x3FEC, (s16)0x3FD3, (s16)0x3FB1, (s16)0x3F84, (s16)0x3F4E, (s16)0x3F0E,
  (s16)0x3EC5, (s16)0x3E71, (s16)0x3E14, (s16)0x3DAE, (s16)0x3D3E, (s16)0x3CC5, (s16)0x3C42, (s16)0x3BB6,
  (s16)0x3B20, (s16)0x3A82, (s16)0x39DA, (s16)0x392A, (s16)0x3871, (s16)0x37AF, (s16)0x36E5, (s16)0x3612,
  (s16)0x3536, (s16)0x3453, (s16)0x3367, (s16)0x3274, (s16)0x3179, (s16)0x3076, (s16)0x2F6B, (s16)0x2E5A,
  (s16)0x2D41, (s16)0x2C21, (s16)0x2AFA, (s16)0x29CD, (s16)0x2899, (s16)0x275F, (s16)0x261F, (s16)0x24DA,
  (s16)0x238E, (s16)0x223D, (s16)0x20E7, (s16)0x1F8B, (s16)0x1E2B, (s16)0x1CC6, (s16)0x1B5D, (s16)0x19EF,
  (s16)0x187D, (s16)0x1708, (s16)0x158F, (s16)0x1413, (s16)0x1294, (s16)0x1111, (s16)0x0F8C, (s16)0x0E05,
  (s16)0x0C7C, (s16)0x0AF1, (s16)0x0964, (s16)0x07D5, (s16)0x0645, (s16)0x04B5, (s16)0x0323, (s16)0x0192,
  (s16)0x0000, (s16)0xFE6E, (s16)0xFCDD, (s16)0xFB4B, (s16)0xF9BB, (s16)0xF82B, (s16)0xF69C, (s16)0xF50F,
  (s16)0xF384, (s16)0xF1FB, (s16)0xF074, (s16)0xEEEF, (s16)0xED6C, (s16)0xEBED, (s16)0xEA71, (s16)0xE8F8,
  (s16)0xE783, (s16)0xE611, (s16)0xE4A3, (s16)0xE33A, (s16)0xE1D5, (s16)0xE075, (s16)0xDF19, (s16)0xDDC3,
  (s16)0xDC72, (s16)0xDB26, (s16)0xD9E1, (s16)0xD8A1, (s16)0xD767, (s16)0xD633, (s16)0xD506, (s16)0xD3DF,
  (s16)0xD2BF, (s16)0xD1A6, (s16)0xD095, (s16)0xCF8A, (s16)0xCE87, (s16)0xCD8C, (s16)0xCC99, (s16)0xCBAD,
  (s16)0xCACA, (s16)0xC9EE, (s16)0xC91B, (s16)0xC851, (s16)0xC78F, (s16)0xC6D6, (s16)0xC626, (s16)0xC57E,
  (s16)0xC4E0, (s16)0xC44A, (s16)0xC3BE, (s16)0xC33B, (s16)0xC2C2, (s16)0xC252, (s16)0xC1EC, (s16)0xC18F,
  (s16)0xC13B, (s16)0xC0F2, (s16)0xC0B2, (s16)0xC07C, (s16)0xC04F, (s16)0xC02D, (s16)0xC014, (s16)0xC005,
  (s16)0xC000, (s16)0xC005, (s16)0xC014, (s16)0xC02D, (s16)0xC04F, (s16)0xC07C, (s16)0xC0B2, (s16)0xC0F2,
  (s16)0xC13B, (s16)0xC18F, (s16)0xC1EC, (s16)0xC252, (s16)0xC2C2, (s16)0xC33B, (s16)0xC3BE, (s16)0xC44A,
  (s16)0xC4E0, (s16)0xC57E, (s16)0xC626, (s16)0xC6D6, (s16)0xC78F, (s16)0xC851, (s16)0xC91B, (s16)0xC9EE,
  (s16)0xCACA, (s16)0xCBAD, (s16)0xCC99, (s16)0xCD8C, (s16)0xCE87, (s16)0xCF8A, (s16)0xD095, (s16)0xD1A6,
  (s16)0xD2BF, (s16)0xD3DF, (s16)0xD506, (s16)0xD633, (s16)0xD767, (s16)0xD8A1, (s16)0xD9E1, (s16)0xDB26,
  (s16)0xDC72, (s16)0xDDC3, (s16)0xDF19, (s16)0xE075, (s16)0xE1D5, (s16)0xE33A, (s16)0xE4A3, (s16)0xE611,
  (s16)0xE783, (s16)0xE8F8, (s16)0xEA71, (s16)0xEBED, (s16)0xED6C, (s16)0xEEEF, (s16)0xF074, (s16)0xF1FB,
  (s16)0xF384, (s16)0xF50F, (s16)0xF69C, (s16)0xF82B, (s16)0xF9BB, (s16)0xFB4B, (s16)0xFCDD, (s16)0xFE6E
};

const u8 ScaleTable[180] = {
	(u8)0xE0, (u8)0xE1, (u8)0xE2, (u8)0xE3, (u8)0xE4, (u8)0xE5, (u8)0xE6, (u8)0xE7, (u8)0xE8,
	(u8)0xE9, (u8)0xEA, (u8)0xEB, (u8)0xD0, (u8)0xD1, (u8)0xD2, (u8)0xD3, (u8)0xD4, (u8)0xD5,
	(u8)0xD6, (u8)0xD7, (u8)0xD8, (u8)0xD9, (u8)0xDA, (u8)0xDB, (u8)0xC0, (u8)0xC1, (u8)0xC2,
	(u8)0xC3, (u8)0xC4, (u8)0xC5, (u8)0xC6, (u8)0xC7, (u8)0xC8, (u8)0xC9, (u8)0xCA, (u8)0xCB,
	(u8)0xB0, (u8)0xB1, (u8)0xB2, (u8)0xB3, (u8)0xB4, (u8)0xB5, (u8)0xB6, (u8)0xB7, (u8)0xB8,
	(u8)0xB9, (u8)0xBA, (u8)0xBB, (u8)0xA0, (u8)0xA1, (u8)0xA2, (u8)0xA3, (u8)0xA4, (u8)0xA5,
	(u8)0xA6, (u8)0xA7, (u8)0xA8, (u8)0xA9, (u8)0xAA, (u8)0xAB, (u8)0x90, (u8)0x91, (u8)0x92,
	(u8)0x93, (u8)0x94, (u8)0x95, (u8)0x96, (u8)0x97, (u8)0x98, (u8)0x99, (u8)0x9A, (u8)0x9B,
	(u8)0x80, (u8)0x81, (u8)0x82, (u8)0x83, (u8)0x84, (u8)0x85, (u8)0x86, (u8)0x87, (u8)0x88,
	(u8)0x89, (u8)0x8A, (u8)0x8B, (u8)0x70, (u8)0x71, (u8)0x72, (u8)0x73, (u8)0x74, (u8)0x75,
	(u8)0x76, (u8)0x77, (u8)0x78, (u8)0x79, (u8)0x7A, (u8)0x7B, (u8)0x60, (u8)0x61, (u8)0x62,
	(u8)0x63, (u8)0x64, (u8)0x65, (u8)0x66, (u8)0x67, (u8)0x68, (u8)0x69, (u8)0x6A, (u8)0x6B,
	(u8)0x50, (u8)0x51, (u8)0x52, (u8)0x53, (u8)0x54, (u8)0x55, (u8)0x56, (u8)0x57, (u8)0x58,
	(u8)0x59, (u8)0x5A, (u8)0x5B, (u8)0x40, (u8)0x41, (u8)0x42, (u8)0x43, (u8)0x44, (u8)0x45,
	(u8)0x46, (u8)0x47, (u8)0x48, (u8)0x49, (u8)0x4A, (u8)0x4B, (u8)0x30, (u8)0x31, (u8)0x32,
	(u8)0x33, (u8)0x34, (u8)0x35, (u8)0x36, (u8)0x37, (u8)0x38, (u8)0x39, (u8)0x3A, (u8)0x3B,
	(u8)0x20, (u8)0x21, (u8)0x22, (u8)0x23, (u8)0x24, (u8)0x25, (u8)0x26, (u8)0x27, (u8)0x28,
	(u8)0x29, (u8)0x2A, (u8)0x2B, (u8)0x10, (u8)0x11, (u8)0x12, (u8)0x13, (u8)0x14, (u8)0x15,
	(u8)0x16, (u8)0x17, (u8)0x18, (u8)0x19, (u8)0x1A, (u8)0x1B, (u8)0x00, (u8)0x01, (u8)0x02,
	(u8)0x03, (u8)0x04, (u8)0x05, (u8)0x06, (u8)0x07, (u8)0x08, (u8)0x09, (u8)0xA, (u8)0xB
};

const u32 FreqTable[12] = {
	(u32)0x80000000,
	(u32)0x879C7C97,
	(u32)0x8FACD61E,
	(u32)0x9837F052,
	(u32)0xA14517CC,
	(u32)0xAADC0848,
	(u32)0xB504F334,
	(u32)0xBFC886BB,
	(u32)0xCB2FF52A,
	(u32)0xD744FCCB,
	(u32)0xE411F03A,
	(u32)0xF1A1BF39
};

/*-----------------------------------------------------------------
  Absolute Value from http://stackoverflow.com/questions/16499475/arm-assembly-absolute-value-function-are-two-or-three-lines-faster
-----------------------------------------------------------------*/
s32 abs(s32 x){
    s32 signext = (x >= 0) ? 0 : -1; //This can be done with an ASR instruction
    return (x + signext) ^ signext;
}

/*-----------------------------------------------------------------
  Square Root Method from http://www.finesse.demon.co.uk/steven/sqrt.html
-----------------------------------------------------------------*/
#define iter1(N) \
    try = root + (1 << (N)); \
    if (n >= try << (N))   \
    {   n -= try << (N);   \
        root |= 2 << (N); \
    }

u32 sqrt_ (u32 n)
{
    u32 root = 0, try;
    iter1 (15);    iter1 (14);    iter1 (13);    iter1 (12);
    iter1 (11);    iter1 (10);    iter1 ( 9);    iter1 ( 8);
    iter1 ( 7);    iter1 ( 6);    iter1 ( 5);    iter1 ( 4);
    iter1 ( 3);    iter1 ( 2);    iter1 ( 1);    iter1 ( 0);
    return root >> 1;
}

/*-----------------------------------------------------------------
  Wrapper methods for VBA-M Methods
-----------------------------------------------------------------*/
static inline u16 CPUReadByte(u32 adr)
{
	return *(u8*)adr;
}
static inline u16 CPUReadHalfWord(u32 adr)
{
	return *(u16*)adr;
}
static inline u32 CPUReadMemory(u32 adr)
{
	return *(u32*)adr;
}
static inline void CPUWriteMemory(u32 adr, u32 val)
{
	*(u32*)adr = val;
}
static inline void CPUWriteHalfWord(u32 adr, u16 val)
{
	*(u16*)adr = val;
}
static inline void CPUWriteByte(u32 adr, u8 val)
{
	*(u8*)adr = val;
}
static inline void CPUUpdateRegister(u32 adr, u16 val)
{
	*(u16*)(0x04000000+adr) = val;
}

u32 umul3232H32(u32 val, u32 val2);

/*-----------------------------------------------------------------
  Assembly method declarations
-----------------------------------------------------------------*/
extern int swi_Div(u32 a, u32 b); //Returns result

/*-----------------------------------------------------------------
0x27 - CustomHalt
  Writes the 8bit parameter value to HALTCNT, below values are equivalent to Halt
  and Stop/Sleep functions, other values reserved, purpose unknown.
  8bit parameter (00h=Halt, 80h=Stop)
-----------------------------------------------------------------*/
void swi_CustomHalt(u8 val) {
	HALTCNT = val;
}

/*-----------------------------------------------------------------
 0x02 - Halt
  Halts the CPU until an interrupt request occurs. The CPU is switched into low-power mode,
  all other circuits (video, sound, timers, serial, keypad, system clock) are kept operating.
  Halt mode is terminated when any enabled interrupts are requested, that is when (IE AND IF)
  is not zero, the GBA locks up if that condition doesn't get true.
  However, the state of CPUs IRQ disable bit in CPSR register, and the IME register are
  don't care, Halt passes through even if either one has disabled interrupts.
-----------------------------------------------------------------*/
void swi_Halt() {
	swi_CustomHalt(0);
}

/*-----------------------------------------------------------------
0x03 - Stop
  Switches the GBA into very low power mode (to be used similar as a screen-saver).
  The CPU, System Clock, Sound, Video, SIO-Shift Clock, DMAs, and Timers are stopped.
  Stop state can be terminated by the following interrupts only
  (as far as enabled in IE register): Joypad, Game Pak, or General-Purpose-SIO.

  "The system clock is stopped so the IF flag is not set."
  Preparation for Stop:
  Disable Video before implementing Stop (otherwise Video just freezes, but still keeps consuming battery power).
  Possibly required to disable Sound also? Obviously, it'd be also recommended to disable any external
  hardware (such like Rumble or Infra-Red) as far as possible.
-----------------------------------------------------------------*/
void swi_Stop() {
	swi_CustomHalt(0x80);
}

/*-----------------------------------------------------------------
  Used by the IntrWait functions
-----------------------------------------------------------------*/
bool CheckInterrupts(u32 waitFlags)
{
	REG_IME = 0; //Disable interrupts
	u16 intFlags = *(vu16*)(0x04000000-8); //Get current flags
	u16 flags = intFlags & waitFlags;
	if(flags)
	{
		intFlags = (flags) ^ intFlags;
		*(vu16*)(0x04000000-8) = intFlags;
	}
	REG_IME = 1; //Enable interrupts
	
	return flags;
}

/*-----------------------------------------------------------------
0x04 - IntrWait
  Continues to wait in Halt state until one (or more) of the specified interrupt(s) do occur.
  The function forcefully sets IME=1. When using multiple interrupts at the same time,
  this function is having less overhead than repeatedly calling the Halt function
-----------------------------------------------------------------*/
void swi_IntrWait(bool discard, u32 waitFlags)
{
	if(discard)
	{
		CheckInterrupts(waitFlags);
	}
	
	u32 val = 0;
	do
	{
		HALTCNT = 0;
		val = CheckInterrupts(waitFlags);
	}
	while(!val);
}

/*-----------------------------------------------------------------
0x05 - VBlankIntrWait
  Continues to wait in Halt state until one (or more) of the specified interrupt(s) do occur.
  The function forcefully sets IME=1. When using multiple interrupts at the same time,
  this function is having less overhead than repeatedly calling the Halt function
-----------------------------------------------------------------*/
void swi_VBlankIntrWait()
{
	swi_IntrWait(true,1);
}

/*-----------------------------------------------------------------
0x06 - Div
  Signed Division, r0/r1.

    r0  signed 32bit Number
    r1  signed 32bit Denom

  Return:

    r0  Number DIV Denom ;signed
    r1  Number MOD Denom ;signed
    r3  ABS (Number DIV Denom) ;unsigned

  For example, incoming -1234, 10 should return -123, -4, +123.
  The function usually gets caught in an endless loop upon division by zero.
-----------------------------------------------------------------*/
/*u32 swi_Div(s32 num, s32 denom)
{
	//Implemented in assembly
}*/

/*-----------------------------------------------------------------
0x07 - DivArm
  Same as above (SWI 06h Div), but incoming parameters are exchanged, r1/r0 (r0=Denom, r1=number).
  For compatibility with ARM's library. Slightly slower (3 clock cycles) than SWI 06h.
-----------------------------------------------------------------*/
/*u32 swi_DivArm(s32 num, s32 denom)
{
	//Implemented in assembly
}*/

/*-----------------------------------------------------------------
0x08 - Sqrt
  Calculate square root.
-----------------------------------------------------------------*/
u32 swi_Sqrt(u32 input)
{
	return sqrt_(input);
}

/*-----------------------------------------------------------------
0x09 - ArcTan
  Calculates the arc tangent.
-----------------------------------------------------------------*/
u32 swi_ArcTan(u32 input)
{
  s32 a =  -(((s32)(input*input)) >> 14);
  s32 b = ((0xA9 * a) >> 14) + 0x390;
  b = ((b * a) >> 14) + 0x91C;
  b = ((b * a) >> 14) + 0xFB6;
  b = ((b * a) >> 14) + 0x16AA;
  b = ((b * a) >> 14) + 0x2081;
  b = ((b * a) >> 14) + 0x3651;
  b = ((b * a) >> 14) + 0xA2F9;
  a = ((s32)input * b) >> 16;
  return (u32)a;
}

/*-----------------------------------------------------------------
0x0A - ArcTan2
  Calculates the arc tangent after correction processing.
-----------------------------------------------------------------*/
u32 swi_ArcTan2(s32 x, s32 y)
{
  u32 res = 0;
  if (y == 0) {
    res = ((x>>16) & 0x8000);
  } else {
    if (x == 0) {
      res = ((y>>16) & 0x8000) + 0x4000;
    } else {
	  if ((abs(x) > abs(y)) || ((abs(x) == abs(y)) && (!((x<0) && (y<0))))) {
        u32 div = swi_Div(y << 14, (u32)x);
        div = swi_ArcTan(div);
        if (x < 0)
          res = 0x8000 + div;
        else
          res = (((y>>16) & 0x8000)<<1) + div;
      } else {
        u32 div = swi_Div(x << 14, (u32)y);
        div = swi_ArcTan(div);
        res = (0x4000 + ((y>>16) & 0x8000)) - div;
      }
    }
  }
  return res;
}

/*-----------------------------------------------------------------
0x0B - CpuSet
  Memory copy/fill in units of 4 bytes or 2 bytes.
  Memcopy is implemented as repeated LDMIA/STMIA [Rb]!,r3 or LDRH/STRH r3,[r0,r5] instructions.
  Memfill as single LDMIA or LDRH followed by repeated STMIA [Rb]!,r3 or STRH r3,[r0,r5].
  The length must be a multiple of 4 bytes (32bit mode) or 2 bytes (16bit mode).
  The (half)wordcount in r2 must be length/4 (32bit mode) or length/2 (16bit mode),
  ie. length in word/halfword units rather than byte units. 
-----------------------------------------------------------------*/
void swi_CpuSet(u32 source, u32 dest, u32 cnt)
{
  if(((source & 0xe000000) == 0) ||
     ((source + (((cnt << 11)>>9) & 0x1fffff)) & 0xe000000) == 0)
    return;

  int count = cnt & 0x1FFFFF;

  // 32-bit ?
  if((cnt >> 26) & 1) {
    // needed for 32-bit mode!
    source &= 0xFFFFFFFC;
    dest &= 0xFFFFFFFC;
    // fill ?
    if((cnt >> 24) & 1) {
	  u32 value = (source>0x0EFFFFFF ? 0x1CAD1CAD : CPUReadMemory(source));
      while(count) {
		CPUWriteMemory(dest, value);
		dest += 4;
        count--;
      }
    } else {
      // copy
      while(count) {
		CPUWriteMemory(dest, (source>0x0EFFFFFF ? 0x1CAD1CAD : CPUReadMemory(source)));
		dest += 4;
		source += 4;
        count--;
      }
    }
  } else {
    // 16-bit fill?
    if((cnt >> 24) & 1) {
	  u16 value = (source>0x0EFFFFFF ? 0x1CAD : CPUReadHalfWord(source));
      while(count) {
        CPUWriteHalfWord(dest, value);
		dest += 2;
        count--;
      }
    } else {
      // copy
      while(count) {
        CPUWriteHalfWord(dest, (source>0x0EFFFFFF ? 0x1CAD : CPUReadHalfWord(source)));
		dest += 2;
		source += 2;
        count--;
      }
    }
  }
}

/*-----------------------------------------------------------------
0x0C - CpuFastSet
  Memory copy/fill in units of 32 bytes. Memcopy is implemented as repeated LDMIA/STMIA [Rb]!,r2-r9 instructions.
  Memfill as single LDR followed by repeated STMIA [Rb]!,r2-r9.
  After processing all 32-byte-blocks, the NDS additonally processes the remaining words as 4-byte blocks.

  The length is specifed as wordcount, ie. the number of bytes divided by 4.
  On the GBA, the length must be a multiple of 8 words (32 bytes). 
-----------------------------------------------------------------*/
void swi_CpuFastSet(u32 source, u32 dest, u32 cnt)
{
  if((((u32)source & 0xe000000) == 0) ||
     (((u32)source + (((cnt << 11)>>9) & 0x1fffff)) & 0xe000000) == 0)
    return;

  // needed for 32-bit mode!
  source &= 0xFFFFFFFC;
  dest &= 0xFFFFFFFC;

  int count = cnt & 0x1FFFFF;

  // fill?
  if((cnt >> 24) & 1) {
    u32 value = (source>0x0EFFFFFF ? 0xBAFFFFFB : CPUReadMemory(source));
    while(count > 0) {
      // BIOS always transfers 32 bytes at a time
      for(int i = 0; i < 8; i++) {
        CPUWriteMemory(dest, value);
        dest += 4;
      }
      count -= 8;
    }
  } else {
    // copy
    while(count > 0) {
      // BIOS always transfers 32 bytes at a time
      for(int i = 0; i < 8; i++) {
        CPUWriteMemory(dest, (source>0x0EFFFFFF ? 0xBAFFFFFB :CPUReadMemory(source)));
        source += 4;
        dest += 4;
      }
      count -= 8;
    }
  }
}

/*-----------------------------------------------------------------
0x0D - GetBiosChecksum
  Calculates the checksum of the BIOS ROM (by reading in 32bit units, and adding up these values).
  IRQ and FIQ are disabled during execution.
  The checksum is BAAE187Fh (GBA and GBA SP), or BAAE1880h (DS in GBA mode, whereas the only difference
  is that the byte at [3F0Ch] is changed from 00h to 01h, otherwise the BIOS is 1:1 same as GBA BIOS,
  it does even include multiboot code).
-----------------------------------------------------------------*/
int swi_GetBiosChecksum()
{
	//TODO: Actually checksum this bios
	//TODO: Make this bios checksum the same as official bios
	return 0xBAAE187F;
}

/*-----------------------------------------------------------------
0x0E - BgAffineSet
  Used to calculate BG Rotation/Scaling parameters.
-----------------------------------------------------------------*/
void swi_BgAffineSet(u32 src, u32 dest, u32 num)
{
  while(num--) {
    s32 cx = CPUReadMemory(src);
    src+=4;
    s32 cy = CPUReadMemory(src);
    src+=4;
    s16 dispx = CPUReadHalfWord(src);
    src+=2;
    s16 dispy = CPUReadHalfWord(src);
    src+=2;
    s16 rx = CPUReadHalfWord(src);
    src+=2;
    s16 ry = CPUReadHalfWord(src);
    src+=2;
    u16 theta = CPUReadHalfWord(src)>>8;
    src+=4; // keep structure alignment
    s32 a = sineTable[(theta+0x40)&255];
    s32 b = sineTable[theta];

    s16 dx =  (rx * a)>>14;
    s16 dmx = (rx * b)>>14;
    s16 dy =  (ry * b)>>14;
    s16 dmy = (ry * a)>>14;

    CPUWriteHalfWord(dest, dx);
    dest += 2;
    CPUWriteHalfWord(dest, -dmx);
    dest += 2;
    CPUWriteHalfWord(dest, dy);
    dest += 2;
    CPUWriteHalfWord(dest, dmy);
    dest += 2;

    s32 startx = cx - dx * dispx + dmx * dispy;
    s32 starty = cy - dy * dispx - dmy * dispy;

    CPUWriteMemory(dest, startx);
    dest += 4;
    CPUWriteMemory(dest, starty);
    dest += 4;
  }
}

/*-----------------------------------------------------------------
0x0F - ObjAffineSet
  Calculates and sets the OBJ's affine parameters from the scaling ratio and angle of rotation.
  The affine parameters are calculated from the parameters set in Srcp.
  The four affine parameters are set every Offset bytes, starting from the Destp address.
  If the Offset value is 2, the parameters are stored contiguously. If the value is 8, they match the structure of OAM.
  When Srcp is arrayed, the calculation can be performed continuously by specifying Num.
-----------------------------------------------------------------*/
void swi_ObjAffineSet(u32 src, u32 dest, int num, int offset)
{
  while(num--) {
    s16 rx = CPUReadHalfWord(src);
    src+=2;
    s16 ry = CPUReadHalfWord(src);
    src+=2;
    u16 theta = CPUReadHalfWord(src)>>8;
    src+=4; // keep structure alignment

    s32 a = (s32)sineTable[(theta+0x40)&255];
    s32 b = (s32)sineTable[theta];

    s16 dx =  ((s32)rx * a)>>14;
    s16 dmx = ((s32)rx * b)>>14;
    s16 dy =  ((s32)ry * b)>>14;
    s16 dmy = ((s32)ry * a)>>14;

    CPUWriteHalfWord(dest, dx);
    dest += offset;
    CPUWriteHalfWord(dest, -dmx);
    dest += offset;
    CPUWriteHalfWord(dest, dy);
    dest += offset;
    CPUWriteHalfWord(dest, dmy);
    dest += offset;
  }
}

/*-----------------------------------------------------------------
0x10 - BitUnPack
  Used to increase the color depth of bitmaps or tile data. For example, to convert a 1bit
  monochrome font into 4bit or 8bit GBA tiles.
  The Unpack Info is specified separately, allowing to convert the same source data into different formats.
-----------------------------------------------------------------*/
void swi_BitUnPack(u32 source, u32 dest, u32 header)
{
  int len = CPUReadHalfWord(header);
    // check address
  if(((source & 0xe000000) == 0) ||
     ((source + len) & 0xe000000) == 0)
    return;

  int bits = CPUReadByte(header+2);
  int revbits = 8 - bits;
  // u32 value = 0;
  u32 base = CPUReadMemory(header+4);
  bool addBase = (base & 0x80000000) ? true : false;
  base &= 0x7fffffff;
  int dataSize = CPUReadByte(header+3);

  int data = 0;
  int bitwritecount = 0;
  while(1) {
    len -= 1;
    if(len < 0)
      break;
    int mask = 0xff >> revbits;
    u8 b = CPUReadByte(source);
    source++;
    int bitcount = 0;
    while(1) {
      if(bitcount >= 8)
        break;
      u32 d = b & mask;
      u32 temp = d >> bitcount;
      if(d || addBase) {
        temp += base;
      }
      data |= temp << bitwritecount;
      bitwritecount += dataSize;
      if(bitwritecount >= 32) {
        CPUWriteMemory(dest, data);
        dest += 4;
        data = 0;
        bitwritecount = 0;
      }
      mask <<= bits;
      bitcount += bits;
    }
  }
}

/*-----------------------------------------------------------------
0x11 - LZ77UnCompWram
  Expands LZ77-compressed data. The Wram function is faster, and writes in units of 8bits.
  For the Vram function the destination must be halfword aligned, data is written in units of 16bits.
  If the size of the compressed data is not a multiple of 4, please adjust it as much as possible
  by padding with 0.
  
  Align the source address to a 4-Byte boundary.
-----------------------------------------------------------------*/
void swi_LZ77UnCompWram(u32 source, u32 dest)
{
  u32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;

  int len = header >> 8;

  while(len > 0) {
    u8 d = CPUReadByte(source++);

    if(d) {
      for(int i = 0; i < 8; i++) {
        if(d & 0x80) {
          u16 data = CPUReadByte(source++) << 8;
          data |= CPUReadByte(source++);
          int length = (data >> 12) + 3;
          int offset = (data & 0x0FFF);
          u32 windowOffset = dest - offset - 1;
          for(int i2 = 0; i2 < length; i2++) {
            CPUWriteByte(dest++, CPUReadByte(windowOffset++));
            len--;
            if(len == 0)
              return;
          }
        } else {
          CPUWriteByte(dest++, CPUReadByte(source++));
          len--;
          if(len == 0)
            return;
        }
        d <<= 1;
      }
    } else {
      for(int i = 0; i < 8; i++) {
        CPUWriteByte(dest++, CPUReadByte(source++));
        len--;
        if(len == 0)
          return;
      }
    }
  }
}

/*-----------------------------------------------------------------
0x12 - LZ77UnCompVram
  Expands LZ77-compressed data. The Wram function is faster, and writes in units of 8bits.
  For the Vram function the destination must be halfword aligned, data is written in units of 16bits.
  If the size of the compressed data is not a multiple of 4, please adjust it as much as possible
  by padding with 0.
  
  Align the source address to a 4-Byte boundary.
-----------------------------------------------------------------*/
void swi_LZ77UnCompVram_(u32 source, u32 dest, int checkBios)
{
  u32 header = CPUReadMemory(source);
  source += 4;

  if(checkBios && (((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0))
    return;

  int byteCount = 0;
  int byteShift = 0;
  u32 writeValue = 0;

  int len = header >> 8;

  while(len > 0) {
    u8 d = CPUReadByte(source++);

    if(d) {
      for(int i = 0; i < 8; i++) {
        if(d & 0x80) {
          u16 data = CPUReadByte(source++) << 8;
          data |= CPUReadByte(source++);
          int length = (data >> 12) + 3;
          int offset = (data & 0x0FFF);
          u32 windowOffset = dest + byteCount - offset - 1;
          for(int i2 = 0; i2 < length; i2++) {
            writeValue |= (CPUReadByte(windowOffset++) << byteShift);
            byteShift += 8;
            byteCount++;

            if(byteCount == 2) {
              CPUWriteHalfWord(dest, writeValue);
              dest += 2;
              byteCount = 0;
              byteShift = 0;
              writeValue = 0;
            }
            len--;
            if(len == 0)
              return;
          }
        } else {
          writeValue |= (CPUReadByte(source++) << byteShift);
          byteShift += 8;
          byteCount++;
          if(byteCount == 2) {
            CPUWriteHalfWord(dest, writeValue);
            dest += 2;
            byteCount = 0;
            byteShift = 0;
            writeValue = 0;
          }
          len--;
          if(len == 0)
            return;
        }
        d <<= 1;
      }
    } else {
      for(int i = 0; i < 8; i++) {
        writeValue |= (CPUReadByte(source++) << byteShift);
        byteShift += 8;
        byteCount++;
        if(byteCount == 2) {
          CPUWriteHalfWord(dest, writeValue);
          dest += 2;
          byteShift = 0;
          byteCount = 0;
          writeValue = 0;
        }
        len--;
        if(len == 0)
          return;
      }
    }
  }
}

void swi_LZ77UnCompVram(u32 source, u32 dest)
{
  swi_LZ77UnCompVram_(source,dest,1);
}

/*-----------------------------------------------------------------
0x13 - HuffUnComp
  The decoder starts in root node, the separate bits in the bitstream specify if the next node is node0 or node1, 
  if that node is a data node, then the data is stored in memory, and the decoder is reset to the root node.
  The most often used data should be as close to the root node as possible. For example, the 4-byte string "Huff"
  could be compressed to 6 bits: 10-11-0-0, with root.0 pointing directly to data "f", and root.1 pointing to a
  child node, whose nodes point to data "H" and data "u".

  Data is written in units of 32bits, if the size of the compressed data is not a multiple of 4,
  please adjust it as much as possible by padding with 0.
  Align the source address to a 4Byte boundary.
-----------------------------------------------------------------*/
void swi_HuffUnComp(u32 source, u32 dest)
{
  u32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;

  u8 treeSize = CPUReadByte(source++);

  u32 treeStart = source;

  source += ((treeSize+1)<<1)-1; // minus because we already skipped one byte

  int len = header >> 8;

  u32 mask = 0x80000000;
  u32 data = CPUReadMemory(source);
  source += 4;

  int pos = 0;
  u8 rootNode = CPUReadByte(treeStart);
  u8 currentNode = rootNode;
  bool writeData = false;
  int byteShift = 0;
  int byteCount = 0;
  u32 writeValue = 0;

  if((header & 0x0F) == 8) {
    while(len > 0) {
      // take left
      if(pos == 0)
        pos++;
      else
        pos += (((currentNode & 0x3F)+1)<<1);

      if(data & mask) {
        // right
        if(currentNode & 0x40)
          writeData = true;
        currentNode = CPUReadByte(treeStart+pos+1);
      } else {
        // left
        if(currentNode & 0x80)
          writeData = true;
        currentNode = CPUReadByte(treeStart+pos);
      }

      if(writeData) {
        writeValue |= (currentNode << byteShift);
        byteCount++;
        byteShift += 8;

        pos = 0;
        currentNode = rootNode;
        writeData = false;

        if(byteCount == 4) {
          byteCount = 0;
          byteShift = 0;
          CPUWriteMemory(dest, writeValue);
          writeValue = 0;
          dest += 4;
          len -= 4;
        }
      }
      mask >>= 1;
      if(mask == 0) {
        mask = 0x80000000;
        data = CPUReadMemory(source);
        source += 4;
      }
    }
  } else {
    int halfLen = 0;
    int value = 0;
    while(len > 0) {
      // take left
      if(pos == 0)
        pos++;
      else
        pos += (((currentNode & 0x3F)+1)<<1);

      if((data & mask)) {
        // right
        if(currentNode & 0x40)
          writeData = true;
        currentNode = CPUReadByte(treeStart+pos+1);
      } else {
        // left
        if(currentNode & 0x80)
          writeData = true;
        currentNode = CPUReadByte(treeStart+pos);
      }

      if(writeData) {
        if(halfLen == 0)
          value |= currentNode;
        else
          value |= (currentNode<<4);

        halfLen += 4;
        if(halfLen == 8) {
          writeValue |= (value << byteShift);
          byteCount++;
          byteShift += 8;

          halfLen = 0;
          value = 0;

          if(byteCount == 4) {
            byteCount = 0;
            byteShift = 0;
            CPUWriteMemory(dest, writeValue);
            dest += 4;
            writeValue = 0;
            len -= 4;
          }
        }
        pos = 0;
        currentNode = rootNode;
        writeData = false;
      }
      mask >>= 1;
      if(mask == 0) {
        mask = 0x80000000;
        data = CPUReadMemory(source);
        source += 4;
      }
    }
  }
}

/*-----------------------------------------------------------------
0x14 - RLUnCompWram
  Expands run-length compressed data. The Wram function is faster, and writes in units of 8bits. 
  For the Vram function the destination must be halfword aligned, data is written in units of 16bits.
  If the size of the compressed data is not a multiple of 4, please adjust it as much as possible by padding with 0.
  Align the source address to a 4Byte boundary.
-----------------------------------------------------------------*/
void swi_RLUnCompWram(u32 source, u32 dest)
{
  u32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;

  int len = header >> 8;

  while(len > 0) {
    u8 d = CPUReadByte(source++);
    int l = d & 0x7F;
    if(d & 0x80) {
      u8 data = CPUReadByte(source++);
      l += 3;
      for(int i = 0;i < l; i++) {
        CPUWriteByte(dest++, data);
        len--;
        if(len == 0)
          return;
      }
    } else {
      l++;
      for(int i = 0; i < l; i++) {
        CPUWriteByte(dest++,  CPUReadByte(source++));
        len--;
        if(len == 0)
          return;
      }
    }
  }
}

/*-----------------------------------------------------------------
0x15 - RLUnCompVram
  Expands run-length compressed data. The Wram function is faster, and writes in units of 8bits. 
  For the Vram function the destination must be halfword aligned, data is written in units of 16bits.
  If the size of the compressed data is not a multiple of 4, please adjust it as much as possible by padding with 0.
  Align the source address to a 4Byte boundary.
-----------------------------------------------------------------*/
void swi_RLUnCompVram(u32 source, u32 dest)
{
  u32 header = CPUReadMemory(source & 0xFFFFFFFC);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;

  int len = header >> 8;
  int byteCount = 0;
  int byteShift = 0;
  u32 writeValue = 0;

  while(len > 0) {
    u8 d = CPUReadByte(source++);
    int l = d & 0x7F;
    if(d & 0x80) {
      u8 data = CPUReadByte(source++);
      l += 3;
      for(int i = 0;i < l; i++) {
        writeValue |= (data << byteShift);
        byteShift += 8;
        byteCount++;

        if(byteCount == 2) {
          CPUWriteHalfWord(dest, writeValue);
          dest += 2;
          byteCount = 0;
          byteShift = 0;
          writeValue = 0;
        }
        len--;
        if(len == 0)
          return;
      }
    } else {
      l++;
      for(int i = 0; i < l; i++) {
        writeValue |= (CPUReadByte(source++) << byteShift);
        byteShift += 8;
        byteCount++;
        if(byteCount == 2) {
          CPUWriteHalfWord(dest, writeValue);
          dest += 2;
          byteCount = 0;
          byteShift = 0;
          writeValue = 0;
        }
        len--;
        if(len == 0)
          return;
      }
    }
  }
}

/*-----------------------------------------------------------------
0x16 - Diff8bitUnFilterWram
  These aren't actually real decompression functions, destination data will have exactly the same size as source data.
  However, assume a bitmap or wave form to contain a stream of increasing numbers such like 10..19, the filtered/unfiltered data would be:

  unfiltered:   10  11  12  13  14  15  16  17  18  19
  filtered:     10  +1  +1  +1  +1  +1  +1  +1  +1  +1

  In this case using filtered data (combined with actual compression algorithms) will obviously produce better compression results.
  Data units may be either 8bit or 16bit used with Diff8bit or Diff16bit functions respectively. 
  The 8bitVram function allows to write to VRAM directly (which uses 16bit data bus) by writing two 8bit values at once, 
  the downside is that it is eventually slower as the 8bitWram function.
-----------------------------------------------------------------*/
void swi_Diff8bitUnFilterWram(u32 source, u32 dest)
{
  u32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     (((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0))
    return;

  int len = header >> 8;

  u8 data = CPUReadByte(source++);
  CPUWriteByte(dest++, data);
  len--;

  while(len > 0) {
    u8 diff = CPUReadByte(source++);
    data += diff;
    CPUWriteByte(dest++, data);
    len--;
  }
}

/*-----------------------------------------------------------------
0x17 - Diff8bitUnFilterVram
  These aren't actually real decompression functions, destination data will have exactly the same size as source data.
  However, assume a bitmap or wave form to contain a stream of increasing numbers such like 10..19, the filtered/unfiltered data would be:

  unfiltered:   10  11  12  13  14  15  16  17  18  19
  filtered:     10  +1  +1  +1  +1  +1  +1  +1  +1  +1

  In this case using filtered data (combined with actual compression algorithms) will obviously produce better compression results.
  Data units may be either 8bit or 16bit used with Diff8bit or Diff16bit functions respectively. 
  The 8bitVram function allows to write to VRAM directly (which uses 16bit data bus) by writing two 8bit values at once, 
  the downside is that it is eventually slower as the 8bitWram function.
-----------------------------------------------------------------*/
void swi_Diff8bitUnFilterVram(u32 source, u32 dest)
{
  u32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;

  int len = header >> 8;

  u8 data = CPUReadByte(source++);
  u16 writeData = data;
  int shift = 8;
  int bytes = 1;

  while(len >= 2) {
    u8 diff = CPUReadByte(source++);
    data += diff;
    writeData |= (data << shift);
    bytes++;
    shift += 8;
    if(bytes == 2) {
      CPUWriteHalfWord(dest, writeData);
      dest += 2;
      len -= 2;
      bytes = 0;
      writeData = 0;
      shift = 0;
    }
  }
}

/*-----------------------------------------------------------------
0x18 - Diff16bitUnFilter
  These aren't actually real decompression functions, destination data will have exactly the same size as source data.
  However, assume a bitmap or wave form to contain a stream of increasing numbers such like 10..19, the filtered/unfiltered data would be:

  unfiltered:   10  11  12  13  14  15  16  17  18  19
  filtered:     10  +1  +1  +1  +1  +1  +1  +1  +1  +1

  In this case using filtered data (combined with actual compression algorithms) will obviously produce better compression results.
  Data units may be either 8bit or 16bit used with Diff8bit or Diff16bit functions respectively. 
  The 8bitVram function allows to write to VRAM directly (which uses 16bit data bus) by writing two 8bit values at once, 
  the downside is that it is eventually slower as the 8bitWram function.
-----------------------------------------------------------------*/
void swi_Diff16bitUnFilter(u32 source, u32 dest)
{
  u32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;

  int len = header >> 8;

  u16 data = CPUReadHalfWord(source);
  source += 2;
  CPUWriteHalfWord(dest, data);
  dest += 2;
  len -= 2;

  while(len >= 2) {
    u16 diff = CPUReadHalfWord(source);
    source += 2;
    data += diff;
    CPUWriteHalfWord(dest, data);
    dest += 2;
    len -= 2;
  }
}

/*-----------------------------------------------------------------
0x18 - Diff16bitUnFilter
  Calculates the value of the assignment to ((SoundArea)sa).vchn[x].fr when playing the wave data, wa,
  with the interval (MIDI KEY) mk and the fine adjustment value (halftones=256) fp.
-----------------------------------------------------------------*/

typedef struct {
	u16	Type;				//Always 0
	u16	Stat;				//Loop Mode
	u32	Freq;				//Frequency Q10
	u32	Loop;				//Loop Start
	u32	Size;				//Loop End/Length
	s8	Data[1];			//PCM Data
} WaveData;

u32 swi_MidiKey2Freq(WaveData* wa, u32 mk, u32 fp)
{
  if(mk > 178)
  {
	  fp = 0xFF000000;
	  mk = 178;
  }

  u8 scale = ScaleTable[mk];
  u32 freq = FreqTable[(scale & 0x0F)];
  u32 hn = scale / 16;
  u32 temp2 = freq >> hn;

  u32 scale2 = ScaleTable[mk+1];
  u32 freq2 = FreqTable[(scale2 & 0x0F)];
  u32 hn2 = scale2 / 16;
  u32 temp4 = freq2 >> hn2;

  u32 diff = temp4 - temp2;
  u32 temp6 = umul3232H32(diff,fp);

  u32 wave_freq = wa->Freq;

  u32 result = umul3232H32((temp6+temp2),wave_freq);

  return result;
}

/*-----------------------------------------------------------------
0x1C - SoundDriverMain
  Main of the sound driver.
  Call every 1/60 of a second. The flow of the process is to call SoundDriverVSync, which is explained later, immediately after the V-Blank interrupt.
  After that, this routine is called after BG and OBJ processing is executed.
-----------------------------------------------------------------*/
void swi_SoundDriverMain()
{
	u32 m4data = CPUReadMemory(0x03007FF0);
	u32 m4dataOriginal = m4data;
	u32 flag = CPUReadMemory(m4data);
	
	if(flag == 0x68736D53) //Special engine flag
	{
		CPUWriteMemory(m4data,flag+1);
		
		void (*functionPtr)(u32);
		functionPtr = (void *)CPUReadMemory(m4data+0x20);
		
		//If MPlayMain exists call it?
		if(functionPtr)
		{
			u32 param = CPUReadMemory(m4data+0x24);
			
			functionPtr(param);
		}
		
		functionPtr = (void *)CPUReadMemory(m4data+0x28);
		//If CgbSound exists call it?
		if(functionPtr)
		{
			u32 param = CPUReadMemory(m4data+0x24);
			
			functionPtr(param);
		}
		
		u32 adr = m4data + 0x350;
		u32 freq = CPUReadMemory(m4data+0x10);
		u8 unknown = CPUReadByte(m4data+0x04);
		
		if(unknown > 0)
		{
			u8 unknown2 = CPUReadByte(m4data+0x0B);
			unknown2 -= unknown-1;
			adr += freq * unknown2;
		}
		
		u32 unknownConst = 0x630;
		u8 unknown1 = CPUReadByte(m4data+0x05);
		if(unknown1)
		{
			u32 temp = 0;
			//misc crap from 1E20 to 1E70
			if(unknown == 2)
				temp = m4data+0x350;
			else
				temp = adr+freq;
				
			int count = freq;
				
			do
			{
				s32 unknown2;
				s32 unknown3;
				
				unknown2 = *(s8*)(adr+unknownConst);
				unknown3 = *(s8*)(adr);
				unknown2 += unknown3;
				
				unknown3 = *(s8*)(temp+unknownConst);
				unknown2 += unknown3;
				
				unknown3 = *(s8*)(temp);
				temp += 1;
				unknown2 += unknown3;
				
				unknown2 *= unknown1;
				
				unknown2 >>= 9;
				
				//If its negative
				if(unknown2 & 0x80)
				{
					unknown2+=1;
				}
				
				//Store new note?
				*(s8*)(adr+unknownConst) = unknown2;
				*(s8*)(adr) = unknown2;
				
			} while (count--);
		}
		
		//1E74
		u32 adr2 = unknownConst + adr;
		
		u32 count = freq>>3;
		
		if(count <= 0)
		{
			CPUWriteMemory(adr,0);
			adr += 4;
			CPUWriteMemory(adr2,0);
			adr2 += 4;
		}
		
		count >>= 1;
		
		if(count <= 0)
		{
			CPUWriteMemory(adr,0);
			adr += 4;
			CPUWriteMemory(adr2,0);
			adr2 += 4;
			
			CPUWriteMemory(adr,0);
			adr += 4;
			CPUWriteMemory(adr2,0);
			adr2 += 4;
		}
		
		do
		{
			CPUWriteMemory(adr,0);
			adr += 4;
			CPUWriteMemory(adr2,0);
			adr2 += 4;
			
			CPUWriteMemory(adr,0);
			adr += 4;
			CPUWriteMemory(adr2,0);
			adr2 += 4;
			
			CPUWriteMemory(adr,0);
			adr += 4;
			CPUWriteMemory(adr2,0);
			adr2 += 4;
			
			CPUWriteMemory(adr,0);
			adr += 4;
			CPUWriteMemory(adr2,0);
			adr2 += 4;
		} while(count-- > 0);
		
		u32 unknown4 = CPUReadMemory(m4data+0x14);
		u32 unknown5 = CPUReadMemory(m4data+0x18);
		u8 loopcount = CPUReadByte(m4data+0x06);
		
		//Why jump 0x50 forward?
		m4data += 0x50;

		do
		{
			//00001EAE
			u32 unknown7 = CPUReadMemory(m4data+0x24);
			u8 unknown8 = CPUReadByte(m4data);
			
			//00001EBE
			if(unknown8 & 0xC7)
			{
				if(!(unknown8 & 0x80))
				{
					if(unknown8 & 0x40)
					{
						u8 unknown11 = 0x03;
						CPUWriteByte(m4data,unknown11);
						CPUWriteMemory(m4data+0x28,unknown7+0x10);
						CPUWriteMemory(m4data+0x18,CPUReadMemory(unknown7+0x0C));
						CPUWriteByte(m4data+0x09,0x00);
						CPUWriteMemory(m4data+0x1C,0x00);
						
						//00001EDE
						u8 unknown9 = CPUReadByte(unknown7+0x03);
						
						if(unknown8 & 0xC0)
						{
							//00001EE6
							unknown11 = 0x13;
							CPUWriteByte(m4data,unknown11);
						}
						//jump to 1F46
						
						//00001F46
						u8 unknown10 = CPUReadByte(m4data+0x04);
						
						//WTF a byte is ALWAYS less than 0xFF
						//0xFF might be an error code?
						if(unknown10 >= 0xFF)
						{
							unknown10 = 0xFF;
							unknown11--;
							CPUWriteByte(m4data,unknown11);
						}
						
						//00001F54
						CPUWriteByte(m4data+0x09,unknown10);
						u32 unknown12 = (CPUReadByte(m4dataOriginal+0x07) + 1) * unknown11;
						u32 unknown13 = unknown12 << 4;
						unknown12 = unknown13 * CPUReadByte(m4data+0x02);
						unknown12 >>= 8;
						CPUWriteByte(m4data+0x0A,(u8)unknown12);
						
						//00001F68
						u32 unknown14 = (CPUReadByte(m4data+0x03) * unknown13) >> 8;
						CPUWriteByte(m4data+0x0B,(u8)unknown12);
					}
				}
			}
			m4data += 0x40; //000020EA
		} while (loopcount--); //000020E8
		
		//000020F0
		//Reset flag?
		CPUWriteMemory(m4data,0x68736D53);
	}
}

/*-----------------------------------------------------------------
0x23 - MusicPlayerContinue
-----------------------------------------------------------------*/
void swi_MusicPlayerContinue(u32 dst)
{
}

/*-----------------------------------------------------------------
0x24 - MusicPlayerFadeOut
-----------------------------------------------------------------*/
void swi_MusicPlayerFadeOut(u32 dst)
{
}

/*-----------------------------------------------------------------
0x2A - SoundGetJumpList
  Receives pointers to 36 additional sound-related BIOS functions.
-----------------------------------------------------------------*/
extern void swi_Invalid();
void swi_SoundGetJumpList(u32 dst)
{
  //Dummy out the jump list by forcing all of them to return immediately
  for(int i = 0; i < 0x24; i++) {
    CPUWriteMemory(dst, (u32)&swi_Invalid);
    dst += 4;
  }
}

/*-----------------------------------------------------------------
0x00 - SoftReset
  Clears 200h bytes of RAM (containing stacks, and BIOS IRQ vector/flags), initializes system, supervisor, and irq stack pointers,
  sets R0-R12, LR_svc, SPSR_svc, LR_irq, and SPSR_irq to zero, and enters system mode.
  
  Host  sp_svc    sp_irq    sp_sys    zerofilled area       return address
  GBA   3007FE0h  3007FA0h  3007F00h  [3007E00h..3007FFFh]  Flag[3007FFAh]

  The GBA return address 8bit flag is interpreted as 00h=8000000h (ROM), or 01h-FFh=2000000h (RAM), entered in ARM state.
  Note: The reset is applied only to the CPU that has executed the SWI (ie. on the NDS, the other CPU will remain unaffected).
  Return: Does not return to calling procedure, instead, loads the above return address into R14, and then jumps to that address by a "BX R14" opcode.
-----------------------------------------------------------------*/
/*void swi_SoftReset()
{
	//Implemented in assembly
}*/

/*-----------------------------------------------------------------
0x01 - RegisterRamReset
  Resets the I/O registers and RAM specified in ResetFlags. However, it does not clear the CPU internal RAM area from 3007E00h-3007FFFh.

    r0  ResetFlags
       Bit   Expl.
       0     Clear 256K on-board WRAM  ;-don't use when returning to WRAM
       1     Clear 32K in-chip WRAM    ;-excluding last 200h bytes
       2     Clear Palette
       3     Clear VRAM
       4     Clear OAM              ;-zerofilled! does NOT disable OBJs!
       5     Reset SIO registers    ;-switches to general purpose mode!
       6     Reset Sound registers
       7     Reset all other registers (except SIO, Sound)

  Return: No return value.
  Bug: LSBs of SIODATA32 are always destroyed, even if Bit5 of R0 was cleared.
  The function always switches the screen into forced blank by setting DISPCNT=0080h (regardless of incoming R0, screen becomes white).
-----------------------------------------------------------------*/
#define COPY_MODE_FILL		1<<24
void swi_RegisterRamReset(u32 flags)
{
  // no need to trace here. this is only called directly from GBA.cpp
  // to emulate bios initialization

  u32 zero = 0;
  CPUUpdateRegister(0x0, 0x80);

  if(flags) {
    if(flags & 0x01) {
      // clear work RAM
	  swi_CpuFastSet((u32)&zero, 0x02000000, 0x40000/4 | COPY_MODE_FILL);
    }
    if(flags & 0x02) {
      // clear internal RAM
	  swi_CpuFastSet((u32)&zero, 0x03000000, 0x7e00/4 | COPY_MODE_FILL); // don't clear 0x7e00-0x7fff
    }
    if(flags & 0x04) {
      // clear palette RAM
	  swi_CpuFastSet((u32)&zero, 0x05000000, 0x400/4 | COPY_MODE_FILL);
    }
    if(flags & 0x08) {
      // clear VRAM
	  swi_CpuFastSet((u32)&zero, 0x06000000, 0x18000/4 | COPY_MODE_FILL);
    }
    if(flags & 0x10) {
      // clean OAM
	  swi_CpuFastSet((u32)&zero, 0x07000000, 0x400/4 | COPY_MODE_FILL);
    }

    if(flags & 0x80) {
      int i;
      for(i = 0; i < 0x10; i++)
        CPUUpdateRegister(0x200+i*2, 0);

      for(i = 0; i < 0xF; i++)
        CPUUpdateRegister(0x4+i*2, 0);

      for(i = 0; i < 0x20; i++)
        CPUUpdateRegister(0x20+i*2, 0);

      for(i = 0; i < 0x18; i++)
        CPUUpdateRegister(0xb0+i*2, 0);

      CPUUpdateRegister(0x130, 0);
      CPUUpdateRegister(0x20, 0x100);
      CPUUpdateRegister(0x30, 0x100);
      CPUUpdateRegister(0x26, 0x100);
      CPUUpdateRegister(0x36, 0x100);
    }

    if(flags & 0x20) {
      int i;
      for(i = 0; i < 8; i++)
        CPUUpdateRegister(0x110+i*2, 0);
      CPUUpdateRegister(0x134, 0x8000);
      for(i = 0; i < 7; i++)
        CPUUpdateRegister(0x140+i*2, 0);
    }

    if(flags & 0x40) {
      int i;
      CPUWriteByte(0x4000084, 0);
      CPUWriteByte(0x4000084, 0x80);
      CPUWriteMemory(0x4000080, 0x880e0000);
      CPUUpdateRegister(0x88, CPUReadHalfWord(0x4000088)&0x3ff);
      CPUWriteByte(0x4000070, 0x70);
      for(i = 0; i < 8; i++)
        CPUUpdateRegister(0x90+i*2, 0);
      CPUWriteByte(0x4000070, 0);
      for(i = 0; i < 8; i++)
        CPUUpdateRegister(0x90+i*2, 0);
      CPUWriteByte(0x4000084, 0);
    }
  }
}
