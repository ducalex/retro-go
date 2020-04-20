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
#define PATH_MAX 192
#define MAX_INPUT 255
#endif

#include <sys/time.h>
#include <stdlib.h>

#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>


/******************************************************************************
 * Log stuff
 *****************************************************************************/

char log_filename[PATH_MAX];
// real filename of the logging file
// it thus also includes full path on advanced system

void
Log(const char *format, ...)
{
	//FILE *log_file;
	char buf[256];

	va_list ap;
	va_start (ap, format);
	vsprintf (buf, format, ap);
	va_end (ap);

    printf(buf);
	//if (!(log_file = fopen (log_filename, "at")))
	//	return;
    //
	//fprintf (log_file, buf);
	//fflush (log_file);
	//fclose (log_file);
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

void
patch_rom(char* filename, int offset, uchar value)
{
	FILE* f;

	f = fopen(filename, "rb+");

	if (f == NULL) {
		Log("Error patching %s (%d,%d)\n", filename, offset, value);
		return;
	}

	fseek(f, offset, SEEK_SET);

	fputc(value, f);

	fclose(f);
}


char *
strupr(char *s)
{
	char *t = s;
	while (*s)
		{
			*s = toupper(*s);
			s++;
		}
	return t;
}


char *
strcasestr (const char *s1, const char *s2)
{
	char *tmps1 = (char *) strupr (strdup (s1));
	char *tmps2 = (char *) strupr (strdup (s2));

	char *result = strstr (tmps1, tmps2);

	free (tmps1);
	free (tmps2);

	return result;
}


int
stricmp (char *s1, char *s2)
{
	char *tmps1 = (char *)strupr(strdup(s1));
	char *tmps2 = (char *)strupr(strdup(s2));

	int result = strcmp(tmps1, tmps2);

	free (tmps1);
	free (tmps2);

	return result;
}


void
get_directory_from_filename(char* filename)
{
	struct stat file_stat;
	stat(filename, &file_stat);

	if (S_ISDIR(file_stat.st_mode)) {
		if ('/' != filename[strlen(filename) - 1]) {
			// It is a directory but not finished by a slash
			strcat(filename, "/");
		}
		return;
	}

	// It is a file, we'll remove the basename part from it
	// There's a slash, we put the end of string marker just after the last one
	if (strrchr(filename, '/') != NULL)
	{
		strrchr(filename, '/')[1] = 0;
	}

}


/*!
 * wipe_directory : suppress a directory, eventually containing files (but not other directories)
 * @param directory_name Name of the directory to suppress
 */
void
wipe_directory(char* directory_name)
{
	DIR* directory;
	struct dirent* directory_entry;

	directory = opendir(directory_name);

	if (NULL == directory) {
		Log("wipe_directory failed : %s\n", strerror(errno));
		return;
	}

	while (directory_entry = readdir(directory))
		unlink(directory_entry->d_name);

	closedir(directory);

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
