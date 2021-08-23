#ifndef __RTC_H__
#define __RTC_H__

#include <stdio.h>

#include "gnuboy.h"

typedef struct
{
	n32 sel, flags, latch, dirty;
	n32 ticks; // Ticks (60 = +1s)
	n32 d, h, m, s; // Current time
	n32 regs[5]; // Latched time
} gb_rtc_t;

extern gb_rtc_t rtc;

void rtc_latch(byte b);
void rtc_write(byte b);
void rtc_save(FILE *f);
void rtc_load(FILE *f);
void rtc_tick();
void rtc_reset(bool hard);

#endif



