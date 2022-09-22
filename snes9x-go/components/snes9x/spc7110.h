/* This file is part of Snes9x. See LICENSE file. */

#ifndef _spc7110_h
#define _spc7110_h
#include "port.h"

#define DECOMP_BUFFER_SIZE 0x10000

void Del7110Gfx(void);
uint8_t S9xGetSPC7110(uint16_t Address);
uint8_t S9xGetSPC7110Byte(uint32_t Address);
uint8_t* Get7110BasePtr(uint32_t);
void S9xSetSPC7110(uint8_t data, uint16_t Address);
void S9xSpc7110Init(void);
uint8_t* Get7110BasePtr(uint32_t);
void S9xSpc7110Reset(void);
void S9xUpdateRTC(void);
void Do7110Logging(void);
int32_t S9xRTCDaysInMonth(int32_t month, int32_t year);

typedef struct
{
   uint8_t reg[16];
   int16_t index;
   uint8_t control;
   bool    init;
   time_t  last_used;
} S7RTC;

typedef struct
{
   uint8_t  reg4800;
   uint8_t  reg4801;
   uint8_t  reg4802;
   uint8_t  reg4803;
   uint8_t  reg4804;
   uint8_t  reg4805;
   uint8_t  reg4806;
   uint8_t  reg4807;
   uint8_t  reg4808;
   uint8_t  reg4809;
   uint8_t  reg480A;
   uint8_t  reg480B;
   uint8_t  reg480C;
   uint8_t  reg4811;
   uint8_t  reg4812;
   uint8_t  reg4813;
   uint8_t  reg4814;
   uint8_t  reg4815;
   uint8_t  reg4816;
   uint8_t  reg4817;
   uint8_t  reg4818;
   uint8_t  reg4820;
   uint8_t  reg4821;
   uint8_t  reg4822;
   uint8_t  reg4823;
   uint8_t  reg4824;
   uint8_t  reg4825;
   uint8_t  reg4826;
   uint8_t  reg4827;
   uint8_t  reg4828;
   uint8_t  reg4829;
   uint8_t  reg482A;
   uint8_t  reg482B;
   uint8_t  reg482C;
   uint8_t  reg482D;
   uint8_t  reg482E;
   uint8_t  reg482F;
   uint8_t  reg4830;
   uint8_t  reg4831;
   uint8_t  reg4832;
   uint8_t  reg4833;
   uint8_t  reg4834;
   uint8_t  reg4840;
   uint8_t  reg4841;
   uint8_t  reg4842;
   uint8_t  AlignBy;
   uint8_t  written;
   uint8_t  offset_add;
   uint32_t DataRomOffset;
   uint32_t DataRomSize;
   uint32_t bank50Internal;
   uint8_t  bank50[DECOMP_BUFFER_SIZE];
} SPC7110Regs;

extern SPC7110Regs s7r;
extern S7RTC rtc_f9;
#endif
