/* This file is part of Snes9x. See LICENSE file. */

#include "seta.h"

void (*SetSETA)(uint32_t, uint8_t) = &S9xSetST010;
uint8_t(*GetSETA)(uint32_t) = &S9xGetST010;

uint8_t S9xGetSetaDSP(uint32_t Address)
{
   return GetSETA(Address);
}

void S9xSetSetaDSP(uint8_t Byte, uint32_t Address)
{
   SetSETA(Address, Byte);
}
