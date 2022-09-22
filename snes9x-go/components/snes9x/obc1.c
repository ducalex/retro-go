/* This file is part of Snes9x. See LICENSE file. */

#include <string.h>
#include "memmap.h"
#include "obc1.h"

static uint8_t* OBC1_RAM = NULL;

int32_t OBC1_Address;
int32_t OBC1_BasePtr;
int32_t OBC1_Shift;

uint8_t GetOBC1(uint16_t Address)
{
   switch (Address)
   {
   case 0x7ff0:
      return OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2)];
   case 0x7ff1:
      return OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 1];
   case 0x7ff2:
      return OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 2];
   case 0x7ff3:
      return OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 3];
   case 0x7ff4:
      return OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200];
   }

   return OBC1_RAM[Address & 0x1fff];
}

void SetOBC1(uint8_t Byte, uint16_t Address)
{
   switch (Address)
   {
   case 0x7ff0:
      OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2)] = Byte;
      break;
   case 0x7ff1:
      OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 1] = Byte;
      break;
   case 0x7ff2:
      OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 2] = Byte;
      break;
   case 0x7ff3:
      OBC1_RAM[OBC1_BasePtr + (OBC1_Address << 2) + 3] = Byte;
      break;
   case 0x7ff4:
   {
      uint8_t Temp;
      Temp = OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200];
      Temp = (Temp & ~(3 << OBC1_Shift)) | ((Byte & 3) << OBC1_Shift);
      OBC1_RAM[OBC1_BasePtr + (OBC1_Address >> 2) + 0x200] = Temp;
      break;
   }
   case 0x7ff5:
   {
      if (Byte & 1)
         OBC1_BasePtr = 0x1800;
      else
         OBC1_BasePtr = 0x1c00;

      OBC1_RAM[0x1ff5] = Byte;
      break;
   }
   case 0x7ff6:
   {
      OBC1_Address = Byte & 0x7f;
      OBC1_Shift = (Byte & 3) << 1;
      break;
   }
   default:
      OBC1_RAM[Address & 0x1fff] = Byte;
      break;
   }
}

uint8_t* GetMemPointerOBC1(uint32_t Address)
{
   return (Memory.FillRAM + (Address & 0xffff));
}

void ResetOBC1()
{
   OBC1_Address = 0;
   OBC1_BasePtr = 0x1c00;
   OBC1_Shift = 0;
   OBC1_RAM = &Memory.FillRAM[0x6000];
   memset(OBC1_RAM, 0x00, 0x2000);
}
