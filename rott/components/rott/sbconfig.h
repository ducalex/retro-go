/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef SBCONFIG_H
#define SBCONFIG_H
/* ----------------------------------------------------------------------- */
/* See the bottom of this file for a syntax diagram for the config file */

#define MAX_STRING_LENGTH 40

/* ----------------------------------------------------------------------- */

//typedef long fixed;	/* 16.16 fixed pt number */

#define INT_TO_FIXED(n)   ((fixed)((long)(n)<<16))
#define FIXED_TO_INT(n)   ((long)((n)>>16))
#define FLOAT_TO_FIXED(n) ((fixed)((n)*65536.0))

#define FIXED_ADD(a, b)		((a)+(b))
#define FIXED_SUB(a, b)		((a)-(b))

/* ----------------------------------------------------------------------- */

typedef struct {
	short low; 			/* range of input values this warp covers */
	short high;
	fixed mult;		/* multiplier to be applied to this range */
} WarpRange;



typedef struct {
	char       name[MAX_STRING_LENGTH];
	WarpRange *pWarp;
	int        nWarp;
} WarpRecord;

/* ----------------------------------------------------------------------- */

/* Parse the config file */
int   SbConfigParse(char *filename);

/* Get the button config name for the button named <btnName> or return NULL
 * if none exists.  <btnName> can be either the left or the right side name.
 * So, for the config line:
 *
 * BUTTON_A MY_BUTTON
 *
 * ConfigGetButton("BUTTON_A") will return "MY_BUTTON"
 * ConfigGetButton("MY_BUTTON") will return "BUTTON_A"
 *
 * Note that this makes it illegal to have game button names have the names
 * "BUTTON_A", "BUTTON_B", "BUTTON_C", etc...
 *
 * Also, the comparison is CASE INSENSITIVE.
 */
char *SbConfigGetButton(char *btnName);
int   SbConfigGetButtonNumber(char *btnName);

/*
 * Get the warp ranges for the config range named <rngName> or return NULL
 * if none exists.
 */
WarpRecord *SbConfigGetWarpRange(char *rngName);

/*
 * Warp a value based on the given warp range
 */
fixed SbFxConfigWarp(WarpRecord *warp, short value);  /* returns fixed pt */
long    SbConfigWarp(WarpRecord *warp, short value);  /* returns integer */

/* ----------------------------------------------------------------------- */
/* Lexical Definitions:
**
** comment:	;[^\n]*\n
** integer: [0-9]+
** identifier: [A-Za-z_][A-Za-z_0-9]*
**
*/
/* Syntax Diagram:	(Line by Line parsing)
**
**
** ConfigFile:
**           |	ConfigLine ConfigFile
**	          ;
**
** ConfigLine:
**           | comment
**           | VersionLine comment
**           | ButtonLine comment
**           | RangeLine comment
**           ;
**
** VersionLine:	'VERSION' integer
**            ;
**
** ButtonLine:	'BUTTON_A' identifier
**           |	'BUTTON_B' identifier
**           |	'BUTTON_C' identifier
**           |	'BUTTON_D' identifier
**           |	'BUTTON_E' identifier
**           |	'BUTTON_F' identifier
**           ;
**
** RangeLine: identifier '{' RangeList '}'
**          ;
**
** RangeList:
**          | '{' integer ',' integer ',' integer '}'
**          | '{' integer ',' integer ',' integer '}' ',' RangeList
**          ;
**
*/
/* ----------------------------------------------------------------------- */
#endif

