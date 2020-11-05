#include <stdio.h>
#include <time.h>

#include "defs.h"
#include "mem.h"
#include "rtc.h"

struct rtc rtc;

// Set in the far future for VBA-M support
#define RT_BASE 1893456000


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

	fwrite(&rtc.s, sizeof(rtc.s), 1, f);
	fwrite(&rtc.m, sizeof(rtc.m), 1, f);
	fwrite(&rtc.h, sizeof(rtc.h), 1, f);
	fwrite(&rtc.d, sizeof(rtc.d), 1, f);
	fwrite(&rtc.flags, sizeof(rtc.flags), 1, f);
	for (int i = 0; i < 5; i++) {
		int tmp = rtc.regs[i];
		fwrite(&tmp, sizeof(tmp), 1, f);
	}
	fwrite(&rt, sizeof(rt), 1, f);
}

void rtc_load(FILE *f)
{
	int64_t rt = 0;
	int tmp;

	// Try to read old format first
	tmp = fscanf(f, "%d %*d %d %02d %02d %02d %02d\n%*d\n",
		&rtc.flags, &rtc.d, &rtc.h, &rtc.m, &rtc.s, &rtc.ticks);

	if (tmp >= 5)
		return;

	fread(&rtc.s, sizeof(rtc.s), 1, f);
	fread(&rtc.m, sizeof(rtc.m), 1, f);
	fread(&rtc.h, sizeof(rtc.h), 1, f);
	fread(&rtc.d, sizeof(rtc.d), 1, f);
	fread(&rtc.flags, sizeof(rtc.flags), 1, f);
	for (int i = 0; i < 5; i++) {
		fread(&tmp, sizeof(tmp), 1, f);
		rtc.regs[i] = tmp;
	}
	fread(&rt, sizeof(rt), 1, f);
}
