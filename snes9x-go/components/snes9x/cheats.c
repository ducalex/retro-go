/* This file is part of Snes9x. See LICENSE file. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "snes9x.h"
#include "cheats.h"
#include "memmap.h"

static bool S9xAllHex(const char* code, int32_t len)
{
   int32_t i;
   for (i = 0; i < len; i++)
      if ((code [i] < '0' || code [i] > '9') &&
          (code [i] < 'a' || code [i] > 'f') &&
          (code [i] < 'A' || code [i] > 'F'))
         return false;

   return true;
}

const char* S9xProActionReplayToRaw(const char* code, uint32_t* address, uint8_t* byte)
{
   uint32_t data = 0;
   if (strlen(code) != 8 || !S9xAllHex(code, 8) || sscanf(code, "%x", &data) != 1)
      return "Invalid Pro Action Replay code - should be 8 hex digits in length.";

   *address = data >> 8;
   *byte = (uint8_t) data;
   return NULL;
}

const char* S9xGoldFingerToRaw(const char* code, uint32_t* address, bool* sram, uint8_t* num_bytes, uint8_t bytes[3])
{
   int32_t i;
   char tmp [15];
   if (strlen(code) != 14)
      return "Invalid Gold Finger code should be 14 hex digits in length.";

   strncpy(tmp, code, 5);
   tmp [5] = 0;
   if (sscanf(tmp, "%x", address) != 1)
      return "Invalid Gold Finger code.";

   for (i = 0; i < 3; i++)
   {
      int32_t byte;
      strncpy(tmp, code + 5 + i * 2, 2);
      tmp [2] = 0;
      if (sscanf(tmp, "%x", &byte) != 1)
         break;
      bytes [i] = (uint8_t) byte;
   }
   *num_bytes = i;
   *sram = code [13] == '1';
   return NULL;
}

const char* S9xGameGenieToRaw(const char* code, uint32_t* address, uint8_t* byte)
{
   char new_code [12];
   static char* real_hex  = "0123456789ABCDEF";
   static char* genie_hex = "DF4709156BC8A23E";
   uint32_t data = 0;
   int32_t i;

   if (strlen(code) != 9 || code[4] != '-' || !S9xAllHex(code, 4) || !S9xAllHex(code + 5, 4))
      return "Invalid Game Genie(tm) code - should be 'xxxx-xxxx'.";

   strcpy(new_code, "0x");
   strncpy(new_code + 2, code, 4);
   strcpy(new_code + 6, code + 5);

   for (i = 2; i < 10; i++)
   {
      int32_t j;
      if (islower(new_code [i]))
         new_code [i] = toupper(new_code [i]);
      for (j = 0; j < 16; j++)
      {
         if (new_code [i] == genie_hex [j])
         {
            new_code [i] = real_hex [j];
            break;
         }
      }
      if (j == 16)
         return "Invalid hex-character in Game Genie(tm) code";
   }
   sscanf(new_code, "%x", &data);
   *byte = (uint8_t)(data >> 24);
   *address = ((data & 0x003c00) << 10) +
              ((data & 0x00003c) << 14) +
              ((data & 0xf00000) >>  8) +
              ((data & 0x000003) << 10) +
              ((data & 0x00c000) >>  6) +
              ((data & 0x0f0000) >> 12) +
              ((data & 0x0003c0) >>  6);

   return NULL;
}
