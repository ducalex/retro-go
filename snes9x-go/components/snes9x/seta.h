/* This file is part of Snes9x. See LICENSE file. */

#ifndef _seta_h
#define _seta_h

#include "port.h"

#define ST_010 0x01
#define ST_011 0x02
#define ST_018 0x03

uint8_t S9xGetSetaDSP(uint32_t Address);
void S9xSetSetaDSP(uint8_t byte, uint32_t Address);
uint8_t S9xGetST018(uint32_t Address);
void S9xSetST018(uint8_t Byte, uint32_t Address);

uint8_t S9xGetST010(uint32_t Address);
void S9xSetST010(uint32_t Address, uint8_t Byte);
uint8_t S9xGetST011(uint32_t Address);
void S9xSetST011(uint32_t Address, uint8_t Byte);

extern void (*SetSETA)(uint32_t, uint8_t);
extern uint8_t(*GetSETA)(uint32_t);

typedef struct
{
   uint8_t input_params [16];
   uint8_t output_params[16];
   uint8_t op_reg;
   uint8_t execute;
   bool    control_enable;
} ST010_Regs;

typedef struct
{
   bool     waiting4command;
   uint8_t  status;
   uint8_t  command;
   uint32_t in_count;
   uint32_t in_index;
   uint32_t out_count;
   uint32_t out_index;
   uint8_t  parameters [512];
   uint8_t  output     [512];
} ST011_Regs;

typedef struct
{
   bool     waiting4command;
   uint8_t  status;
   uint8_t  part_command;
   uint8_t  pass;
   uint32_t command;
   uint32_t in_count;
   uint32_t in_index;
   uint32_t out_count;
   uint32_t out_index;
   uint8_t  parameters [512];
   uint8_t  output     [512];
} ST018_Regs;
#endif
