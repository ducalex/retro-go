/*
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU Library General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#if defined(__linux__)
#include <limits.h>
#else
#define PATH_MAX 255
#define MAX_INPUT 255
#endif

#include <sys/time.h>
#include <stdlib.h>

#include "utils.h"
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>


/******************************************************************************
 * Log stuff
 *****************************************************************************/

void
Log(const char *format, ...)
{
	va_list ap;
	va_start (ap, format);
	vprintf (format, ap);
	va_end (ap);
}


static double osd_getTime(void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);
	// printf("current microsec = %f\n",tp.tv_sec + 1e-6 * tp.tv_usec);
	return tp.tv_sec + 1e-6 * tp.tv_usec;
}


static void osd_sleep(double s)
{
	return;

	// emu_too_fast = 0;
	printf("Sleeping %d micro seconds\n", s);
	// Log("Sleeping %f seconds\n", s);
	return;
	if (s > 0)
	{
		struct timeval tp;

		tp.tv_sec = 0;
		tp.tv_usec = 1e6 * s;
		select(1, NULL, NULL, NULL, &tp);
		usleep(s * 1e6);
		// emu_too_fast = 1;
	}
}


void
wait_next_vsync()
{
	return;

	static double lasttime = 0, lastcurtime = 0, frametime = 0.1;
	double curtime;
	const double deltatime = (1.0 / 60.0);

	curtime = osd_getTime();

	osd_sleep(lasttime + deltatime - curtime);
	curtime = osd_getTime();

	lastcurtime = curtime;

	lasttime += deltatime;

	if ((lasttime + deltatime) < curtime)
		lasttime = curtime;
}


/*!
 * file_exists : Check whether a file exists. If doesn't involve much checking (like read/write access,
 * whether it is a symbolic link or a directory, ...)
 * @param name_to_check Name of the file to check for existence
 * @return 0 if the file doesn't exist, else non null value
 */
int
file_exists(char* name_to_check)
{
	struct stat stat_buffer;
	return !stat(name_to_check, &stat_buffer);
}


/*!
 * file_size : Open a file and determine it's size
 * @param file_name Name of file to check size of
 * @return 0 if file doesn't exist, else size
 */
long
file_size(char* file_name)
{
	FILE* f = fopen(file_name, "rb");
	long position;
	if (f == NULL)
		return 0;
	fseek(f, 0, SEEK_END);
	position = ftell(f);
	fclose(f);
	return position;
}
