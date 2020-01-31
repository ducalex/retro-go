#ifndef __RTC_H__
#define __RTC_H__

#include <stdio.h>

#include "defs.h"

struct rtc
{
	int batt;
	int sel;
	int latch;
	int d, h, m, s, t;
	int stop, carry;
	byte regs[8];
};

extern struct rtc rtc;

void rtc_latch(byte b);
void rtc_write(byte b);
void rtc_save_internal(FILE *f);
void rtc_load_internal(FILE *f);
void rtc_tick();

#endif



