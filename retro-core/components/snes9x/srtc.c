/* This file is part of Snes9x. See LICENSE file. */

#include <string.h>
#include "snes9x.h"
#include "srtc.h"
#include "memmap.h"

/***   The format of the rtc_data structure is:

Index Description     Range (nibble)
----- --------------  ---------------------------------------

  0   Seconds low     0-9
  1   Seconds high    0-5

  2   Minutes low     0-9
  3   Minutes high    0-5

  4   Hour low        0-9
  5   Hour high       0-2

  6   Day low         0-9
  7   Day high        0-3

  8   Month           1-C (0xC is December, 12th month)

  9   Year ones       0-9
  A   Year tens       0-9
  B   Year High       9-B  (9=19xx, A=20xx, B=21xx)

  C   Day of week     0-6  (0=Sunday, 1=Monday,...,6=Saturday)

***/

SRTC_DATA           rtc;

static int32_t month_keys[12] = { 1, 4, 4, 0, 2, 5, 0, 3, 6, 1, 4, 6 };


/*********************************************************************************************
 * Note, if you are doing a save state for this game:
 *
 * On save:
 *
 * Call S9xUpdateSrtcTime and save the rtc data structure.
 *
 * On load:
 *
 * restore the rtc data structure
 *      rtc.system_timestamp = time (NULL);
 *********************************************************************************************/


void S9xResetSRTC(void)
{
   rtc.index = -1;
   rtc.mode = MODE_READ;
}

void S9xHardResetSRTC(void)
{
   memset(&rtc, 0, sizeof(rtc));
   rtc.index = -1;
   rtc.mode = MODE_READ;
   rtc.count_enable = false;
   rtc.needs_init = true;
   rtc.system_timestamp = time(NULL); /* Get system timestamp */
}

/**********************************************************************************************/
/* S9xSRTCComputeDayOfWeek(void)                                                              */
/* Return 0-6 for Sunday-Saturday                                                             */
/**********************************************************************************************/
uint32_t S9xSRTCComputeDayOfWeek(void)
{
   uint32_t year = rtc.data[10] * 10 + rtc.data[9];
   uint32_t month = rtc.data[8];
   uint32_t day = rtc.data[7] * 10 + rtc.data[6];
   uint32_t day_of_week;
   year += (rtc.data[11] - 9) * 100;

   /* Range check the month for valid array indices */
   if (month > 12)
      month = 1;

   day_of_week = year + (year / 4) + month_keys[month - 1] + day - 1;

   if ((year % 4 == 0) && (month <= 2))
      day_of_week--;

   day_of_week %= 7;

   return day_of_week;
}


/**********************************************************************************************/
/* S9xSRTCDaysInMonth()                                                                       */
/* Return the number of days in a specific month for a certain year                           */
/**********************************************************************************************/
int32_t S9xSRTCDaysInMmonth(int32_t month, int32_t year)
{
   switch(month)
   {
      case 2:
         if((year % 4 == 0)) /* DKJM2 only uses 199x - 22xx */
            return 29;
         return 28;
      case 4:
      case 6:
      case 9:
      case 11:
         return 30;
      default:
         return 31;
   }
}

#define MINUTETICKS  60
#define HOURTICKS   (60 * MINUTETICKS)
#define DAYTICKS    (24 * HOURTICKS)


/**********************************************************************************************/
/* S9xUpdateSrtcTime()                                                                        */
/* Advance the  S-RTC time if counting is enabled                                             */
/**********************************************************************************************/
void S9xUpdateSrtcTime(void)
{
   time_t  cur_systime;
   int32_t time_diff;

   /* Keep track of game time by computing the number of seconds that pass on the system */
   /* clock and adding the same number of seconds to the S-RTC clock structure. */
   /* Note: Dai Kaijyu Monogatari II only allows dates in the range 1996-21xx. */

   if (rtc.count_enable && !rtc.needs_init)
   {
      cur_systime = time(NULL);
      time_diff = (int32_t)(cur_systime - rtc.system_timestamp);
      rtc.system_timestamp = cur_systime;

      if (time_diff > 0)
      {
         int32_t seconds;
         int32_t minutes;
         int32_t hours;
         int32_t days;
         int32_t month;
         int32_t year;
         int32_t temp_days;
         int32_t year_hundreds;
         int32_t year_tens;
         int32_t year_ones;

         if (time_diff > DAYTICKS)
         {
            days = time_diff / DAYTICKS;
            time_diff = time_diff - days * DAYTICKS;
         }
         else
            days = 0;

         if (time_diff > HOURTICKS)
         {
            hours = time_diff / HOURTICKS;
            time_diff = time_diff - hours * HOURTICKS;
         }
         else
            hours = 0;

         if (time_diff > MINUTETICKS)
         {
            minutes = time_diff / MINUTETICKS;
            time_diff = time_diff - minutes * MINUTETICKS;
         }
         else
            minutes = 0;

         if (time_diff > 0)
            seconds = time_diff;
         else
            seconds = 0;


         seconds += (rtc.data[1] * 10 + rtc.data[0]);
         if (seconds >= 60)
         {
            seconds -= 60;
            minutes += 1;
         }

         minutes += (rtc.data[3] * 10 + rtc.data[2]);
         if (minutes >= 60)
         {
            minutes -= 60;
            hours += 1;
         }

         hours += (rtc.data[5] * 10 + rtc.data[4]);
         if (hours >= 24)
         {
            hours -= 24;
            days += 1;
         }

         if (days > 0)
         {
            year =  rtc.data[10] * 10 + rtc.data[9];
            year += (1000 + rtc.data[11] * 100);
            month = rtc.data[8];
            days += (rtc.data[7] * 10 + rtc.data[6]);
            while (days > (temp_days = S9xSRTCDaysInMmonth(month, year)))
            {
               days -= temp_days;
               month += 1;
               if (month > 12)
               {
                  year += 1;
                  month = 1;
               }
            }

            year_tens = year % 100;
            year_ones = year_tens % 10;
            year_tens /= 10;
            year_hundreds = (year - 1000) / 100;
            rtc.data[6] = days % 10;
            rtc.data[7] = days / 10;
            rtc.data[8] = month;
            rtc.data[9] = year_ones;
            rtc.data[10] = year_tens;
            rtc.data[11] = year_hundreds;
            rtc.data[12] = S9xSRTCComputeDayOfWeek();
         }

         rtc.data[0] = seconds % 10;
         rtc.data[1] = seconds / 10;
         rtc.data[2] = minutes % 10;
         rtc.data[3] = minutes / 10;
         rtc.data[4] = hours % 10;
         rtc.data[5] = hours / 10;
         return;
      }
   }
}

/**********************************************************************************************/
/* S9xSetSRTC(void)                                                                           */
/* This function sends data to the S-RTC used in Dai Kaijyu Monogatari II                     */
/**********************************************************************************************/
void S9xSetSRTC(uint8_t data, uint16_t Address)
{
   (void) Address;
   data &= 0x0F; /* Data is only 4-bits, mask out unused bits. */

   if (data >= 0xD) /* It's an RTC command */
   {
      switch (data)
      {
      case 0xD:
         rtc.mode = MODE_READ;
         rtc.index = -1;
         break;
      case 0xE:
         rtc.mode = MODE_COMMAND;
         break;
      default:
         break;
      }

      return;
   }

   if (rtc.mode == MODE_LOAD_RTC)
   {
      if ((rtc.index >= 0) || (rtc.index < MAX_RTC_INDEX))
      {
         rtc.data[rtc.index++] = data;

         if (rtc.index == MAX_RTC_INDEX) /* We have all the data for the RTC load */
         {
            rtc.system_timestamp = time(NULL); /* Get local system time */
            rtc.data[rtc.index++] = S9xSRTCComputeDayOfWeek(); /* Get the day of the week */

            /* Start RTC counting again */
            rtc.count_enable = true;
            rtc.needs_init = false;
         }

         return;
      }
   }
   else if (rtc.mode == MODE_COMMAND)
   {
      switch (data)
      {
      case COMMAND_CLEAR_RTC:
         rtc.count_enable = false; /* Disable RTC counter */
         memset(rtc.data, 0, MAX_RTC_INDEX + 1);
         rtc.index = -1;
         rtc.mode = MODE_COMMAND_DONE;
         break;
      case COMMAND_LOAD_RTC:
         rtc.count_enable = false; /* Disable RTC counter */
         rtc.index = 0;  /* Setup for writing */
         rtc.mode = MODE_LOAD_RTC;
         break;
      default:
         rtc.mode = MODE_COMMAND_DONE; /* unrecognized command - need to implement. */
      }

      return;
   }
}

/**********************************************************************************************/
/* S9xGetSRTC()                                                                               */
/* This function retrieves data from the S-RTC                                                */
/**********************************************************************************************/
uint8_t S9xGetSRTC(uint16_t Address)
{
   (void) Address;
   if (rtc.mode == MODE_READ)
   {
      if (rtc.index < 0)
      {
         S9xUpdateSrtcTime(); /* Only update it if the game reads it */
         rtc.index++;
         return 0x0f; /* Send start marker. */
      }
      else if (rtc.index > MAX_RTC_INDEX)
      {
         rtc.index = -1; /* Setup for next set of reads */
         return 0x0f; /* Data done marker. */
      }
      else
         return rtc.data[rtc.index++]; /* Feed out the data */
   }
   else
      return 0x0;
}

void S9xSRTCPreSaveState()
{
   if (Settings.SRTC)
   {
      int32_t s;

      S9xUpdateSrtcTime();
      s = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;

      if (s > 0x20000)
         s = 0x20000;

      Memory.SRAM [s + 0] = rtc.needs_init;
      Memory.SRAM [s + 1] = rtc.count_enable;
      /* memmove converted: Different mallocs [Neb] */
      memcpy(&Memory.SRAM [s + 2], rtc.data, MAX_RTC_INDEX + 1);
      Memory.SRAM [s + 3 + MAX_RTC_INDEX] = rtc.index;
      Memory.SRAM [s + 4 + MAX_RTC_INDEX] = rtc.mode;

#ifdef MSB_FIRST
      Memory.SRAM [s + 5  + MAX_RTC_INDEX] = (uint8_t)(rtc.system_timestamp >>  0);
      Memory.SRAM [s + 6  + MAX_RTC_INDEX] = (uint8_t)(rtc.system_timestamp >>  8);
      Memory.SRAM [s + 7  + MAX_RTC_INDEX] = (uint8_t)(rtc.system_timestamp >> 16);
      Memory.SRAM [s + 8  + MAX_RTC_INDEX] = (uint8_t)(rtc.system_timestamp >> 24);
      Memory.SRAM [s + 9  + MAX_RTC_INDEX] = (uint8_t)(rtc.system_timestamp >> 32);
      Memory.SRAM [s + 10 + MAX_RTC_INDEX] = (uint8_t)(rtc.system_timestamp >> 40);
      Memory.SRAM [s + 11 + MAX_RTC_INDEX] = (uint8_t)(rtc.system_timestamp >> 48);
      Memory.SRAM [s + 12 + MAX_RTC_INDEX] = (uint8_t)(rtc.system_timestamp >> 56);
#else
      uint64_t *dst = (uint64_t *)&Memory.SRAM[s + 5 + MAX_RTC_INDEX];
      *dst = rtc.system_timestamp;
#endif
   }
}

void S9xSRTCPostLoadState()
{
   if (Settings.SRTC)
   {
      int32_t s = Memory.SRAMSize ? (1 << (Memory.SRAMSize + 3)) * 128 : 0;
      if (s > 0x20000)
         s = 0x20000;

      rtc.needs_init = Memory.SRAM [s + 0];
      rtc.count_enable = Memory.SRAM [s + 1];
      /* memmove converted: Different mallocs [Neb] */
      memcpy(rtc.data, &Memory.SRAM [s + 2], MAX_RTC_INDEX + 1);
      rtc.index = Memory.SRAM [s + 3 + MAX_RTC_INDEX];
      rtc.mode = Memory.SRAM [s + 4 + MAX_RTC_INDEX];

#ifdef MSB_FIRST
      rtc.system_timestamp |= (Memory.SRAM [s +  5 + MAX_RTC_INDEX] <<  0);
      rtc.system_timestamp |= (Memory.SRAM [s +  6 + MAX_RTC_INDEX] <<  8);
      rtc.system_timestamp |= (Memory.SRAM [s +  7 + MAX_RTC_INDEX] << 16);
      rtc.system_timestamp |= (Memory.SRAM [s +  8 + MAX_RTC_INDEX] << 24);
      rtc.system_timestamp |= (Memory.SRAM [s +  9 + MAX_RTC_INDEX] << 32);
      rtc.system_timestamp |= (Memory.SRAM [s + 10 + MAX_RTC_INDEX] << 40);
      rtc.system_timestamp |= (Memory.SRAM [s + 11 + MAX_RTC_INDEX] << 48);
      rtc.system_timestamp |= (Memory.SRAM [s + 12 + MAX_RTC_INDEX] << 56);
#else
      /* memmove converted: Different mallocs [Neb] */
      uint64_t *src = (uint64_t *)&Memory.SRAM[s + 5 + MAX_RTC_INDEX];
      rtc.system_timestamp = *src;
#endif
      S9xUpdateSrtcTime();
   }
}
