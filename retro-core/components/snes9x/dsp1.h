/* This file is part of Snes9x. See LICENSE file. */

#ifndef _DSP1_H_
#define _DSP1_H_

extern void (*SetDSP)(uint8_t, uint16_t);
extern uint8_t(*GetDSP)(uint16_t);

void DSP1SetByte(uint8_t byte, uint16_t address);
uint8_t DSP1GetByte(uint16_t address);

void DSP2SetByte(uint8_t byte, uint16_t address);
uint8_t DSP2GetByte(uint16_t address);

void DSP3SetByte(uint8_t byte, uint16_t address);
uint8_t DSP3GetByte(uint16_t address);

typedef struct
{
   bool     waiting4command;
   bool     first_parameter;
   uint8_t  command;
   uint32_t in_count;
   uint32_t in_index;
   uint32_t out_count;
   uint32_t out_index;
   uint8_t  *parameters; // [512];
   uint8_t  *output;     // [512];
} SDSP1;

void S9xResetDSP1(void);
uint8_t S9xGetDSP(uint16_t Address);
void S9xSetDSP(uint8_t Byte, uint16_t Address);
extern SDSP1 DSP1;
#endif
