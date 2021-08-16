#ifndef __RTC_H__
#define __RTC_H__

#include <stdio.h>

#include "gnuboy.h"

typedef struct
{
	n32 sel, latch;
	// Ticks (60 = +1s)
	n32 ticks;
	// Current time
	n32 d, h, m, s, flags;
	// Latched time
	n32 regs[8];
} gb_rtc_t;

extern gb_rtc_t rtc;

void rtc_latch(byte b);
void rtc_write(byte b);
void rtc_save(FILE *f);
void rtc_load(FILE *f);
void rtc_tick();
void rtc_reset(bool hard);
void rtc_sync();

#endif



