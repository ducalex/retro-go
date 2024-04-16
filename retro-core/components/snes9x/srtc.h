/* This file is part of Snes9x. See LICENSE file. */

#ifndef _srtc_h_
#define _srtc_h_

#include <time.h>

#define MAX_RTC_INDEX       0xC

#define MODE_READ           0
#define MODE_LOAD_RTC       1
#define MODE_COMMAND        2
#define MODE_COMMAND_DONE   3

#define COMMAND_LOAD_RTC    0
#define COMMAND_CLEAR_RTC   4

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

typedef struct
{
   bool     needs_init;
   bool     count_enable; /* Does RTC mark time or is it frozen */
   uint8_t  data [MAX_RTC_INDEX + 1];
   int8_t   index;
   uint8_t  mode;
   time_t   system_timestamp; /* Of latest RTC load time */
} SRTC_DATA;

extern SRTC_DATA rtc;

void    S9xUpdateSrtcTime(void);
void    S9xSetSRTC(uint8_t data, uint16_t Address);
uint8_t S9xGetSRTC(uint16_t Address);
void    S9xSRTCPreSaveState(void);
void    S9xSRTCPostLoadState(void);
void    S9xResetSRTC(void);
void    S9xHardResetSRTC(void);
#endif /* _srtc_h */
