#ifndef __RTC_H__
#define __RTC_H__

#include <stdio.h>

#include "emu.h"

typedef struct
{
	int sel, latch;
	// Ticks (60 = +1s)
	int ticks;
	// Current time
	int d, h, m, s, flags;
	// Latched time
	byte regs[8];
} rtc_t;

extern rtc_t rtc;

void rtc_latch(byte b);
void rtc_write(byte b);
void rtc_save(FILE *f);
void rtc_load(FILE *f);
void rtc_tick();

#endif



