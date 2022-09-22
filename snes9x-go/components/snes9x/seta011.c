/* This file is part of Snes9x. See LICENSE file. */

#include "seta.h"
#include "memmap.h"

ST011_Regs ST011;
uint8_t board[9][9]; /* shougi playboard */

uint8_t S9xGetST011(uint32_t Address)
{
   uint8_t t;
   uint16_t address = (uint16_t) Address & 0xFFFF;

   if (address == 0x01) /* status check */
      t = 0xFF;
   else /* read directly from s-ram */
      t = Memory.SRAM[address];

   return t;
}

void S9xSetST011(uint32_t Address, uint8_t Byte)
{
   uint16_t address = (uint16_t) Address & 0xFFFF;
   static bool reset = false;

   if (!reset) /* bootup values */
   {
      ST011.waiting4command = true;
      reset = true;
   }

   Memory.SRAM[address] = Byte;

   if (address == 0x00) /* op commands/data goes through this address */
   {
      if (ST011.waiting4command) /* check for new commands */
      {
         ST011.waiting4command = false;
         ST011.command = Byte;
         ST011.in_index = 0;
         ST011.out_index = 0;
         switch (ST011.command)
         {
         case 0x01:
            ST011.in_count = 12 * 10 + 8;
            break;
         case 0x02:
            ST011.in_count = 4;
            break;
         case 0x04:
         case 0x05:
         case 0x06:
         case 0x07:
         case 0x0E:
            ST011.in_count = 0;
            break;
         default:
            ST011.waiting4command = true;
            break;
         }
      }
      else
      {
         ST011.parameters [ST011.in_index] = Byte;
         ST011.in_index++;
      }
   }

   if (ST011.in_count == ST011.in_index) /* Actually execute the command */
   {
      ST011.waiting4command = true;
      ST011.out_index = 0;
      switch (ST011.command)
      {
      case 0x01: /* unknown: download playboard */
      {
         /* 9x9 board data: top to bottom, left to right */
         /* Values represent piece types and ownership */
         int32_t lcv;
         for (lcv = 0; lcv < 9; lcv++)
            memcpy(board[lcv], ST011.parameters + lcv * 10, 9 * 1);
         break;
      }
      case 0x04: /* unknown */
      case 0x05:
      {
         /* outputs */
         Memory.SRAM[0x12C] = 0x00;
         Memory.SRAM[0x12E] = 0x00;
         break;
      }
      case 0x0E: /* unknown */
      {
         /* outputs */
         Memory.SRAM[0x12C] = 0x00;
         Memory.SRAM[0x12D] = 0x00;
         break;
      }
      }
   }
}
