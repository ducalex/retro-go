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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>


/******************************************************************************
 * Log stuff
 *****************************************************************************/

char *log_filename;//[PATH_MAX];
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


#ifndef MY_VSYNC_DISABLE
static double osd_getTime(void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);
	// printf("current microsec = %f\n",tp.tv_sec + 1e-6 * tp.tv_usec);
	return tp.tv_sec + 1e-6 * tp.tv_usec;
}
#endif /* !MY_VSYNC_DISABLE */


#ifndef MY_VSYNC_DISABLE
static void osd_sleep(double s)
{
	// emu_too_fast = 0;
	// printf("Sleeping %d micro seconds\n", s);
	// Log("Sleeping %f seconds\n", s);
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
#endif /* !MY_VSYNC_DISABLE */


void
wait_next_vsync()
{
#ifndef MY_VSYNC_DISABLE
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
#endif
}


unsigned long TAB_CONST[256] = {
	0X0,
	0X77073096,
	0XEE0E612C,
	0X990951BA,
	0X76DC419,
	0X706AF48F,
	0XE963A535,
	0X9E6495A3,
	0XEDB8832,
	0X79DCB8A4,
	0XE0D5E91E,
	0X97D2D988,
	0X9B64C2B,
	0X7EB17CBD,
	0XE7B82D07,
	0X90BF1D91,
	0X1DB71064,
	0X6AB020F2,
	0XF3B97148,
	0X84BE41DE,
	0X1ADAD47D,
	0X6DDDE4EB,
	0XF4D4B551,
	0X83D385C7,
	0X136C9856,
	0X646BA8C0,
	0XFD62F97A,
	0X8A65C9EC,
	0X14015C4F,
	0X63066CD9,
	0XFA0F3D63,
	0X8D080DF5,
	0X3B6E20C8,
	0X4C69105E,
	0XD56041E4,
	0XA2677172,
	0X3C03E4D1,
	0X4B04D447,
	0XD20D85FD,
	0XA50AB56B,
	0X35B5A8FA,
	0X42B2986C,
	0XDBBBC9D6,
	0XACBCF940,
	0X32D86CE3,
	0X45DF5C75,
	0XDCD60DCF,
	0XABD13D59,
	0X26D930AC,
	0X51DE003A,
	0XC8D75180,
	0XBFD06116,
	0X21B4F4B5,
	0X56B3C423,
	0XCFBA9599,
	0XB8BDA50F,
	0X2802B89E,
	0X5F058808,
	0XC60CD9B2,
	0XB10BE924,
	0X2F6F7C87,
	0X58684C11,
	0XC1611DAB,
	0XB6662D3D,
	0X76DC4190,
	0X1DB7106,
	0X98D220BC,
	0XEFD5102A,
	0X71B18589,
	0X6B6B51F,
	0X9FBFE4A5,
	0XE8B8D433,
	0X7807C9A2,
	0XF00F934,
	0X9609A88E,
	0XE10E9818,
	0X7F6A0DBB,
	0X86D3D2D,
	0X91646C97,
	0XE6635C01,
	0X6B6B51F4,
	0X1C6C6162,
	0X856530D8,
	0XF262004E,
	0X6C0695ED,
	0X1B01A57B,
	0X8208F4C1,
	0XF50FC457,
	0X65B0D9C6,
	0X12B7E950,
	0X8BBEB8EA,
	0XFCB9887C,
	0X62DD1DDF,
	0X15DA2D49,
	0X8CD37CF3,
	0XFBD44C65,
	0X4DB26158,
	0X3AB551CE,
	0XA3BC0074,
	0XD4BB30E2,
	0X4ADFA541,
	0X3DD895D7,
	0XA4D1C46D,
	0XD3D6F4FB,
	0X4369E96A,
	0X346ED9FC,
	0XAD678846,
	0XDA60B8D0,
	0X44042D73,
	0X33031DE5,
	0XAA0A4C5F,
	0XDD0D7CC9,
	0X5005713C,
	0X270241AA,
	0XBE0B1010,
	0XC90C2086,
	0X5768B525,
	0X206F85B3,
	0XB966D409,
	0XCE61E49F,
	0X5EDEF90E,
	0X29D9C998,
	0XB0D09822,
	0XC7D7A8B4,
	0X59B33D17,
	0X2EB40D81,
	0XB7BD5C3B,
	0XC0BA6CAD,
	0XEDB88320,
	0X9ABFB3B6,
	0X3B6E20C,
	0X74B1D29A,
	0XEAD54739,
	0X9DD277AF,
	0X4DB2615,
	0X73DC1683,
	0XE3630B12,
	0X94643B84,
	0XD6D6A3E,
	0X7A6A5AA8,
	0XE40ECF0B,
	0X9309FF9D,
	0XA00AE27,
	0X7D079EB1,
	0XF00F9344,
	0X8708A3D2,
	0X1E01F268,
	0X6906C2FE,
	0XF762575D,
	0X806567CB,
	0X196C3671,
	0X6E6B06E7,
	0XFED41B76,
	0X89D32BE0,
	0X10DA7A5A,
	0X67DD4ACC,
	0XF9B9DF6F,
	0X8EBEEFF9,
	0X17B7BE43,
	0X60B08ED5,
	0XD6D6A3E8,
	0XA1D1937E,
	0X38D8C2C4,
	0X4FDFF252,
	0XD1BB67F1,
	0XA6BC5767,
	0X3FB506DD,
	0X48B2364B,
	0XD80D2BDA,
	0XAF0A1B4C,
	0X36034AF6,
	0X41047A60,
	0XDF60EFC3,
	0XA867DF55,
	0X316E8EEF,
	0X4669BE79,
	0XCB61B38C,
	0XBC66831A,
	0X256FD2A0,
	0X5268E236,
	0XCC0C7795,
	0XBB0B4703,
	0X220216B9,
	0X5505262F,
	0XC5BA3BBE,
	0XB2BD0B28,
	0X2BB45A92,
	0X5CB36A04,
	0XC2D7FFA7,
	0XB5D0CF31,
	0X2CD99E8B,
	0X5BDEAE1D,
	0X9B64C2B0,
	0XEC63F226,
	0X756AA39C,
	0X26D930A,
	0X9C0906A9,
	0XEB0E363F,
	0X72076785,
	0X5005713,
	0X95BF4A82,
	0XE2B87A14,
	0X7BB12BAE,
	0XCB61B38,
	0X92D28E9B,
	0XE5D5BE0D,
	0X7CDCEFB7,
	0XBDBDF21,
	0X86D3D2D4,
	0XF1D4E242,
	0X68DDB3F8,
	0X1FDA836E,
	0X81BE16CD,
	0XF6B9265B,
	0X6FB077E1,
	0X18B74777,
	0X88085AE6,
	0XFF0F6A70,
	0X66063BCA,
	0X11010B5C,
	0X8F659EFF,
	0XF862AE69,
	0X616BFFD3,
	0X166CCF45,
	0XA00AE278,
	0XD70DD2EE,
	0X4E048354,
	0X3903B3C2,
	0XA7672661,
	0XD06016F7,
	0X4969474D,
	0X3E6E77DB,
	0XAED16A4A,
	0XD9D65ADC,
	0X40DF0B66,
	0X37D83BF0,
	0XA9BCAE53,
	0XDEBB9EC5,
	0X47B2CF7F,
	0X30B5FFE9,
	0XBDBDF21C,
	0XCABAC28A,
	0X53B39330,
	0X24B4A3A6,
	0XBAD03605,
	0XCDD70693,
	0X54DE5729,
	0X23D967BF,
	0XB3667A2E,
	0XC4614AB8,
	0X5D681B02,
	0X2A6F2B94,
	0XB40BBE37,
	0XC30C8EA1,
	0X5A05DF1B,
	0X2D02EF8D
};

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
