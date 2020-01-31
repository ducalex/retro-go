#include <stdlib.h>
#include <stdio.h>

#include "gnuboy.h"
#include "rc.h"

extern rcvar_t rcfile_exports[], emu_exports[], loader_exports[],
	lcd_exports[], rtc_exports[], debug_exports[], sound_exports[],
	vid_exports[], joy_exports[], pcm_exports[];


rcvar_t *sources[] =
{
	rcfile_exports,
	emu_exports,
	loader_exports,
	lcd_exports,
	rtc_exports,
	debug_exports,
	sound_exports,
	vid_exports,
	joy_exports,
	pcm_exports,
	NULL
};


void init_exports()
{
	rcvar_t **s = sources;
	
	while (*s)
		rc_exportvars(*(s++));
}


void show_exports()
{
	int i, j;
	for (i = 0; sources[i]; i++)
		for (j = 0; sources[i][j].name; j++)
			printf("%s\n", sources[i][j].name);
}
