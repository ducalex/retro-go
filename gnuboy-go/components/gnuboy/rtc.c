


#include <stdio.h>
#include <time.h>

#include "defs.h"
#include "mem.h"
#include "rtc.h"
#include "rc.h"

struct rtc rtc;

static int syncrtc = 1;

rcvar_t rtc_exports[] =
{
	RCV_BOOL("syncrtc", &syncrtc),
	RCV_END
};


void rtc_latch(byte b)
{
	if ((rtc.latch ^ b) & b & 1)
	{
		rtc.regs[0] = rtc.s;
		rtc.regs[1] = rtc.m;
		rtc.regs[2] = rtc.h;
		rtc.regs[3] = rtc.d;
		rtc.regs[4] = (rtc.d>>9) | (rtc.stop<<6) | (rtc.carry<<7);
		rtc.regs[5] = 0xff;
		rtc.regs[6] = 0xff;
		rtc.regs[7] = 0xff;
	}
	rtc.latch = b;
}

void rtc_write(byte b)
{
	/* printf("write %02X: %02X (%d)\n", rtc.sel, b, b); */
	if (!(rtc.sel & 8)) return;
	switch (rtc.sel & 7)
	{
	case 0:
		rtc.s = rtc.regs[0] = b;
		while (rtc.s >= 60) rtc.s -= 60;
		break;
	case 1:
		rtc.m = rtc.regs[1] = b;
		while (rtc.m >= 60) rtc.m -= 60;
		break;
	case 2:
		rtc.h = rtc.regs[2] = b;
		while (rtc.h >= 24) rtc.h -= 24;
		break;
	case 3:
		rtc.regs[3] = b;
		rtc.d = (rtc.d & 0x100) | b;
		break;
	case 4:
		rtc.regs[4] = b;
		rtc.d = (rtc.d & 0xff) | ((b&1)<<9);
		rtc.stop = (b>>6)&1;
		rtc.carry = (b>>7)&1;
		break;
	}
}

void rtc_tick()
{
	if (rtc.stop) return;
	if (++rtc.t == 60)
	{
		if (++rtc.s == 60)
		{
			if (++rtc.m == 60)
			{
				if (++rtc.h == 24)
				{
					if (++rtc.d == 365)
					{
						rtc.d = 0;
						rtc.carry = 1;
					}
					rtc.h = 0;
				}
				rtc.m = 0;
			}
			rtc.s = 0;
		}
		rtc.t = 0;
	}
}

void rtc_save_internal(FILE *f)
{
	time_t rt;
    
	rt = time(0);
#ifdef GNUBOY_USE_BINARY_RTC_FILES
	/* WARNING using binary real time clock files is not portable! */
	fwrite((const void*)&rtc.carry, sizeof(rtc.carry), 1, f);
	fwrite((const void*)&rtc.stop, sizeof(rtc.stop), 1, f);
	fwrite((const void*)&rtc.d, sizeof(rtc.d), 1, f);
	fwrite((const void*)&rtc.h, sizeof(rtc.h), 1, f);
	fwrite((const void*)&rtc.m, sizeof(rtc.m), 1, f);
	fwrite((const void*)&rtc.s, sizeof(rtc.s), 1, f);
	fwrite((const void*)&rtc.t, sizeof(rtc.t), 1, f);
	fwrite((const void*)&rt, sizeof(rt), 1, f);
#else /* GNUBOY_USE_BINARY_RTC_FILES */
	fprintf(f, "%d %d %d %02d %02d %02d %02d\n%d\n",
		rtc.carry, rtc.stop, rtc.d, rtc.h, rtc.m, rtc.s, rtc.t, (int)rt);
#endif /* GNUBOY_USE_BINARY_RTC_FILES */
}

void rtc_load_internal(FILE *f)
{
	time_t rt = 0;
#ifdef GNUBOY_USE_BINARY_RTC_FILES
	/* WARNING using binary real time clock files is not portable! */
	fread(&rtc.carry, sizeof(rtc.carry), 1, f);
	fread(&rtc.stop, sizeof(rtc.stop), 1, f);
	fread(&rtc.d, sizeof(rtc.d), 1, f);
	fread(&rtc.h, sizeof(rtc.h), 1, f);
	fread(&rtc.m, sizeof(rtc.m), 1, f);
	fread(&rtc.s, sizeof(rtc.s), 1, f);
	fread(&rtc.t, sizeof(rtc.t), 1, f);
	fread(&rt, sizeof(rt), 1, f);
#else /* GNUBOY_USE_BINARY_RTC_FILES */
	fscanf(
		f, "%d %d %d %02d %02d %02d %02d\n%d\n",
		&rtc.carry, &rtc.stop, &rtc.d,
		&rtc.h, &rtc.m, &rtc.s, &rtc.t, (int *)&rt);
#endif /* GNUBOY_USE_BINARY_RTC_FILES */
	while (rtc.t >= 60) rtc.t -= 60;
	while (rtc.s >= 60) rtc.s -= 60;
	while (rtc.m >= 60) rtc.m -= 60;
	while (rtc.h >= 24) rtc.h -= 24;
	while (rtc.d >= 365) rtc.d -= 365;
	rtc.stop &= 1;
	rtc.carry &= 1;
	if (rt) rt = (time(0) - rt) * 60;
	if (syncrtc) while (rt-- > 0) rtc_tick();
}

