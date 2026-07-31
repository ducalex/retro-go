#include "rt_def.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>
#if PLATFORM_UNIX
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#endif

#include <fcntl.h>


#if defined(USE_SDL)
#include "SDL.h"
#endif
              
/* 
  Copied over from Wolf3D Linux: http://www.icculus.org/wolf3d/
  Modified for ROTT.
 */
 
int _argc;
char **_argv;

#if PLATFORM_UNIX
long filelength(int handle)
{
	struct stat buf;
	
	if (fstat(handle, &buf) == -1) {
		perror("filelength");
		exit(EXIT_FAILURE);
	}
	
	return buf.st_size;
}

char *strlwr(char *s)
{
	char *p = s;
	
	while (*p) {
		*p = tolower(*p);
		p++;
	}
	
	return s;
}

char *strupr(char *s)
{
	char *p = s;
	
	while (*p) {
		*p = toupper(*p);
		p++;
	}
	
	return s;
}
	
char *itoa(int value, char *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%d", value);
			break;
		case 16:
			sprintf(string, "%x", value);
			break;
		default:
			STUB_FUNCTION;
			break;
	}
	
	return string;
}

char *ltoa(long value, char *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%ld", value);
			break;
		case 16:
			sprintf(string, "%lx", value);
			break;
		default:
			STUB_FUNCTION;
			break;
	}
	
	return string;
}

char *ultoa(unsigned long value, char *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%lu", value);
			break;
		case 16:
			sprintf(string, "%lux", value);
			break;
		default:
			STUB_FUNCTION;
			break;
	}
	
	return string;
}
#endif

char getch(void)
{
     return 0;
}
extern char ApogeePath[256];

int setup_homedir (void)
{
#if PLATFORM_UNIX && !defined(__MINGW32__)
	int err;

	/* try to create the root directory */
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/.rott/", getenv ("HOME"));
	err = mkdir (ApogeePath, S_IRWXU);
	
/* keep the shareware and registered game data separated */
#if (SHAREWARE == 1)
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/.rott/", getenv ("HOME"));
#else
	snprintf (ApogeePath, sizeof (ApogeePath), "%s/.rott/darkwar/", getenv ("HOME"));
#endif

	err = mkdir (ApogeePath, S_IRWXU);
	if (err == -1 && errno != EEXIST)
	{
		fprintf (stderr, "Couldn't create preferences directory: %s\n", 
				strerror (errno));
		return -1;
	}
#else
    sprintf(ApogeePath, ".%s", PATH_SEP_STR);
#endif

	return 0;
}

/* from Dan Olson */
void put_dos2ansi(byte attrib)
{
	int lookup[] = {30,34,32,36,31,35,33,37};
	byte fore,back,blink=0,intens=0;
	
	fore = attrib&15;	/* bits 0-3 */
	back = attrib&112; /* bits 4-6 */
       	blink = attrib&128; /* bit 7 */
	
	/* Fix background, blink is either on or off. */
	back = back>>4;
 
	/* Fix foreground */
	if (fore > 7) {
		intens = 1;
		fore-=8;
	}

	/* Convert fore/back */
	fore = lookup[fore];
	back = lookup[back]+10;

	// 'Render"
	if (blink)
		printf ("\033[%d;5;%dm\033[%dm", intens, fore, back);
	else
		printf ("\033[%d;25;%dm\033[%dm", intens, fore, back);
}

void DisplayTextSplash(byte *text, int l)
{
	int i;
	int bound = 80*l*2;

	for (i=0;i<bound;i+=2)
	{
		put_dos2ansi(text[i+1]);
		putchar (text[i]);
	}

	printf ("\033[m");
}

#if !defined(__CYGWIN__) && !defined(__MINGW32__)
#ifndef ESP_PLATFORM
#include <execinfo.h>
#endif

void print_stack (int level)
{
#ifndef ESP_PLATFORM
	void *array[64];
	char **syms;
	int size, i;

	printf ("Stack dump:\n");
	printf ("{\n");
	size = backtrace (array, (sizeof (array))/(sizeof (array[0])));
	syms = backtrace_symbols (array, size);
	for (i=level+1; i<size;++i) {
		printf ("\t%s\n",syms[i]);
	}
	free (syms);
	/*
	for (i = 2; i <size; ++i) {
		printf ("\t%p\n", array[i]);
	}
	*/
	printf ("}\n");
#else
    printf ("Stack dump not available on this platform\n");
#endif
}
#else

void print_stack (int level)
{
        printf("Stack dump not implemented.\n");
}

#endif

void crash_print (int sig)
{
	printf ("OH NO OH NO ROTT CRASHED!\n");
	printf ("Here is where:\n");
	print_stack (1);
#if defined(USE_SDL)
	SDL_Quit ();
#endif
	exit (1);
}

#if 0
/* ** */

uint16_t SwapInt16L(uint16_t i)
{
#if BYTE_ORDER == BIG_ENDIAN
	return ((uint16_t)i >> 8) | ((uint16_t)i << 8);
#else
	return i;
#endif
}

uint32_t SwapInt32L(uint32_t i)
{
#if BYTE_ORDER == BIG_ENDIAN
	return	((uint32_t)(i & 0xFF000000) >> 24) | 
		((uint32_t)(i & 0x00FF0000) >>  8) |
		((uint32_t)(i & 0x0000FF00) <<  8) | 
		((uint32_t)(i & 0x000000FF) << 24);
#else
	return i;
#endif
}

/* ** */

int OpenWrite(char *_fn)
{
	int fp;
    char fn[MAX_PATH];
    strncpy(fn, _fn, sizeof (fn));
    fn[sizeof (fn) - 1] = '\0';
    FixFilePath(fn);

	fp = open(fn, O_CREAT|O_WRONLY|O_TRUNC|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	return fp;
}

int OpenWriteAppend(char *_fn)
{
	int fp;
    char fn[MAX_PATH];
    strncpy(fn, _fn, sizeof (fn));
    fn[sizeof (fn) - 1] = '\0';
    FixFilePath(fn);

	fp = open(fn, O_CREAT|O_WRONLY|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	return fp;
}

void CloseWrite(int fp)
{
	close(fp);
}

int WriteSeek(int fp, int offset, int whence)
{
	return lseek(fp, offset, whence);
}

int WritePos(int fp)
{
	return lseek(fp, 0, SEEK_CUR);
}

int WriteInt8(int fp, int8_t d)
{
	return write(fp, &d, 1);
}

int WriteInt16(int fp, int16_t d)
{
	int16_t b = SwapInt16L(d);
	
	return write(fp, &b, 2) / 2;
}

int WriteInt32(int fp, int32_t d)
{
	int32_t b = SwapInt32L(d);
	
	return write(fp, &b, 4) / 4;
}

int WriteBytes(int fp, byte *d, int len)
{
	return write(fp, d, len);
}


int OpenRead(char *_fn)
{
	int fp;
    char fn[MAX_PATH];
    strncpy(fn, _fn, sizeof (fn));
    fn[sizeof (fn) - 1] = '\0';
    FixFilePath(fn);
	
	fp = open(fn, O_RDONLY | O_BINARY);
	
	return fp;
}

void CloseRead(int fp)
{
	close(fp);
}

int ReadSeek(int fp, int offset, int whence)
{
	return lseek(fp, offset, whence);
}

int ReadLength(int fp)
{
	return filelength(fp);
}

int8_t ReadInt8(int fp)
{
	byte d[1];
	
	read(fp, d, 1);
	
	return d[0];
}

int16_t ReadInt16(int fp)
{
	byte d[2];
	
	read(fp, d, 2);
	
	return (d[0]) | (d[1] << 8);
}

int32_t ReadInt32(int fp)
{
	byte d[4];
	
	read(fp, d, 4);
	
	return (d[0]) | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
}

int ReadBytes(int fp, byte *d, int len)
{
	return read(fp, d, len);
}

#endif
