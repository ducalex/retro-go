#include <sys/time.h>
#include <stdio.h>
#include <time.h>

#include "emu.h"
#include "mem.h"
#include "rtc.h"

rtc_t rtc;

// Set in the far future for VBA-M support
#define RT_BASE 1893456000


void rtc_reset()
{
	memset(&rtc, 0, sizeof(rtc));
	rtc_sync();
}

void rtc_sync()
{
	time_t timer = time(NULL);
	struct tm *info = localtime(&timer);

	rtc.d = info->tm_yday;
	rtc.h = info->tm_hour;
	rtc.m = info->tm_min;
	rtc.s = info->tm_sec;

	printf("%s: Clock set to day %03d at %02d:%02d:%02d\n",
		__func__, rtc.d, rtc.h, rtc.m, rtc.s);
}

void rtc_latch(byte b)
{
	if ((rtc.latch ^ b) & b & 1)
	{
		rtc.regs[0] = rtc.s;
		rtc.regs[1] = rtc.m;
		rtc.regs[2] = rtc.h;
		rtc.regs[3] = rtc.d;
		rtc.regs[4] = rtc.flags;
		rtc.regs[5] = 0xff;
		rtc.regs[6] = 0xff;
		rtc.regs[7] = 0xff;
	}
	rtc.latch = b;
}

void rtc_write(byte b)
{
	/* printf("write %02X: %02X (%d)\n", rtc.sel, b, b); */
	switch (rtc.sel & 0xf)
	{
	case 0x8: // Seconds
		rtc.regs[0] = b;
		rtc.s = b % 60;
		break;
	case 0x9: // Minutes
		rtc.regs[1] = b;
		rtc.m = b % 60;
		break;
	case 0xA: // Hours
		rtc.regs[2] = b;
		rtc.h = b % 24;
		break;
	case 0xB: // Days (lower 8 bits)
		rtc.regs[3] = b;
		rtc.d = ((rtc.d & 0x100) | b) % 365;
		break;
	case 0xC: // Flags (days upper 1 bit, carry, stop)
		rtc.regs[4] = b;
		rtc.flags = b;
		rtc.d = ((rtc.d & 0xff) | ((b&1)<<9)) % 365;
		break;
	}
}

void rtc_tick()
{
	if ((rtc.flags & 0x40))
		return; // rtc stop

	if (++rtc.ticks >= 60)
	{
		if (++rtc.s >= 60)
		{
			if (++rtc.m >= 60)
			{
				if (++rtc.h >= 24)
				{
					if (++rtc.d >= 365)
					{
						rtc.d = 0;
						rtc.flags |= 0x80;
					}
					rtc.h = 0;
				}
				rtc.m = 0;
			}
			rtc.s = 0;
		}
		rtc.ticks = 0;
	}
}

void rtc_save(FILE *f)
{
	int64_t rt = RT_BASE + (rtc.s + (rtc.m * 60) + (rtc.h * 3600) + (rtc.d * 86400));

	fwrite(&rtc.s, 4, 1, f);
	fwrite(&rtc.m, 4, 1, f);
	fwrite(&rtc.h, 4, 1, f);
	fwrite(&rtc.d, 4, 1, f);
	fwrite(&rtc.flags, 4, 1, f);
	for (int i = 0; i < 5; i++) {
		fwrite(&rtc.regs[i], 4, 1, f);
	}
	fwrite(&rt, 8, 1, f);
}

void rtc_load(FILE *f)
{
	int64_t rt = 0;

	// Try to read old format first
	int tmp = fscanf(f, "%d %*d %d %02d %02d %02d %02d\n%*d\n",
		&rtc.flags, &rtc.d, &rtc.h, &rtc.m, &rtc.s, &rtc.ticks);

	if (tmp >= 5)
		return;

	fread(&rtc.s, 4, 1, f);
	fread(&rtc.m, 4, 1, f);
	fread(&rtc.h, 4, 1, f);
	fread(&rtc.d, 4, 1, f);
	fread(&rtc.flags, 4, 1, f);
	for (int i = 0; i < 5; i++) {
		fread(&rtc.regs[i], 4, 1, f);
	}
	fread(&rt, 8, 1, f);
}
