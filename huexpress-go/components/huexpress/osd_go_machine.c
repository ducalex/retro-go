#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "osd.h"


void osd_log(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}


void osd_wait_next_vsync(void)
{
	const double deltatime = (1000000.0 / 60.0);
	static double lasttime, curtime, prevtime, sleep;
	struct timeval tp;

	gettimeofday(&tp, NULL);
	curtime = tp.tv_sec * 1000000.0 + tp.tv_usec;

	sleep = lasttime + deltatime - curtime;

	if (sleep > 0)
	{
		tp.tv_sec = 0;
		tp.tv_usec = sleep;
		select(1, NULL, NULL, NULL, &tp);
		usleep(sleep);
	}
	else if (sleep < -8333.0)
	{
		osd_skipFrames++;
	}

    odroid_system_tick(!osd_blitFrames, true, (uint)(curtime - prevtime));
	osd_blitFrames = 0;

	gettimeofday(&tp, NULL);
	curtime = prevtime = tp.tv_sec * 1000000.0 + tp.tv_usec;
	lasttime += deltatime;

	if ((lasttime + deltatime) < curtime)
		lasttime = curtime;
}
