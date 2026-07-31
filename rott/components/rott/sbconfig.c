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
/* Copyright 1995 Spacetec IMC Corporation */

#include "rt_def.h"
#if defined(__BORLANDC__)
#  pragma inline
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>

typedef long fixed;


#include "develop.h"
#include "sbconfig.h"
//MED
#include "memcheck.h"

/* ----------------------------------------------------------------------- */

#define DEFAULT_CONFIG_FILENAME  "sbconfig.dat"

#define NUM_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))
#define SIGN(x) ((x<0)?-1:1)

/* ----------------------------------------------------------------------- */

#if defined(__BORLANDC__)

fixed FIXED_MUL(fixed a, fixed b)
{

   fixed ret_code;

   asm {

      mov  eax,a
      mov  edx,b
      imul edx
      shrd eax,edx,16
      mov  ret_code,eax
   }

   return ret_code;
}

#elif defined(_MSC_VER)

/* Microsoft C7.0 can not be done with inline assembler because it
   can not handle 32-bit instructions
*/
fixed FIXED_MUL(fixed a,fixed b)
{
   fixed sgn,ah,al,bh,bl;

   sgn = (SIGN(a) ^ SIGN(b)) ? -1 : 1 ;
   ah = (a >> 16) & 0xffff ;
   al = (a        & 0xffff);
   bh = (b >> 16) & 0xffff ;
   bl = (b        & 0xffff);

   return sgn * ( ((al*bl)>>16) + (ah*bl) + (al*bh) + ((ah*bh)<<16) );
}

#elif defined(__WATCOMC__)

fixed FIXED_MUL(fixed a, fixed b);

#pragma aux FIXED_MUL =     \
   "imul    edx"            \
   "shrd    eax,edx,16"     \
   parm     [eax] [edx]     \
   value    [eax]           \
   modify   exact [eax]     ;

#endif /* definition of inline FIXED_MUL */


static
fixed StrToFx1616(char *string, char **ret_string)
{
	long  whole;
	long  fract;
	fixed result;
	int   sign;

	/* Skip whitespace */
	while((*string==' ')||(*string=='\t')||(*string=='\n')) string++;

	/* Accept numbers in the form: [+-]?[0-9]+(.[0-9]*)
	*/

	sign=1;
	if(*string=='-')
		{
			string++;
			sign=-1;
		}
	else if(*string=='+')
		{
			string++;
			sign=1;
		}

	/* Read in the whole part */
	whole=0;
	while((*string>='0')&&(*string<='9'))
		whole=whole*10+(*string++)-'0';

	/* Read the optional fraction part */
	fract=0;
	if(*string=='.')
		{
			long place=1;
			string++;
			while((*string>='0')&&(*string<='9'))
				{
					fract=fract*10+(*string++)-'0';
					place*=10;
				}
			/* Convert to fixed point */
			fract=(fract<<16)/place;
		}

	if(ret_string) *ret_string=string;

	if(sign==1)
		result= (whole<<16) + fract;
	else
		result=-(whole<<16) + fract;

	return result;
}

/* ----------------------------------------------------------------------- */

static char *SbButtonNames[]={
	"BUTTON_A",
	"BUTTON_B",
	"BUTTON_C",
	"BUTTON_D",
	"BUTTON_E",
	"BUTTON_F"};

static int         cfgFileVersion=0;
static char        cfgButtons[NUM_ELEMENTS(SbButtonNames)][MAX_STRING_LENGTH];
static WarpRecord *pCfgWarps;
static int         nCfgWarps=0;

/*-------------------------------------------------------------------------*/

/* Read a string in the form: "{#, #, #}"
 * and return a pointer to the character AFTER the '}'
 * or NULL if error
 */
static char *GetWarpLevels(char *string, WarpRange *pw)
{
	short value;
   fixed fxvalue;

	if((*string++)!='{')
  		return NULL;

	if(!*string)
  		return NULL;

	/* Expecting the first number - low */
	value=(short)strtol(string, &string, 0);
	if(pw) pw->low=value;

	/* Skip any whitespace */
	while(isspace(*string)) string++;

	if(*string++!=',')
  		return NULL;

	/* Expecting the second number - high */
	value=(short)strtol(string, &string, 0);
	if(pw) pw->high=value;

	/* Skip any whitespace */
	while(isspace(*string)) string++;

	if(*string++!=',')
  		return NULL;

	/* Expecting the third number - multiplier */
	fxvalue=StrToFx1616(string, &string);
	if(pw) pw->mult=fxvalue;

	/* Skip any whitespace */
	while(isspace(*string)) string++;

	if(*string!='}')
		return NULL;

	return string+1;
}



static int GetWarp(char *string, WarpRecord *pRecord)
{
	int        nWarp;
	WarpRange *pWarp;

	/* Only update the field if we successfully read the entire line */
	nWarp=0;
	pWarp=NULL;

	/* Skip whitespace */
	while(isspace(*string)) string++;

	if(*string++!='{') return 0;

	while(string && *string)
		{
			switch(*string)
				{
				case ' ':
				case '\t':
				case '\n':
					string++;
					break;

				case ',':
					string++;
					break;

				case '{':
					if(nWarp++==0)
						pWarp=malloc(sizeof(WarpRange));
					else
						pWarp=realloc(pWarp, nWarp*sizeof(WarpRange));

					string=GetWarpLevels(string, pWarp+nWarp-1);
					break;

				case '}':
					pRecord->nWarp=nWarp;
					pRecord->pWarp=pWarp;
					return 1;
				}
		}

	/* Early EOL (didn't get closing '}') */
	if(nWarp) free(pWarp);
	return 0;
}
	

/*-------------------------------------------------------------------------*/

int SbConfigParse(char *_filename)
{
	int             i;
	FILE           *file;
	char           *pc;
	char            buffer[128];
    char filename[MAX_PATH];

   if(!_filename) _filename=DEFAULT_CONFIG_FILENAME;

    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

	if(!(file=fopen(filename, "r")))
      {
			printf("Config file: %s, not found\n", filename);
			return 0;
      }

	while(fgets(buffer, sizeof(buffer), file))
		{
			int lineParsed=0;

			/* each line in config file starts with either a ;, VERSION or an
	      ** element of GameButtonNames or WarpNames
			*/
	      pc=strtok(buffer, " \t\n,");

	      if(*pc==';')									/* commented out line? */
				continue;

			/* VERSION?  The version will be used in the future to allow fx1616
			**           values in the config file
			*/
			if(!stricmp(pc,"VERSION"))
				{
					pc=strtok(NULL, " \t\n,");
					cfgFileVersion=atoi(pc);
				}

			/* Check if first token is an element of SbButtonNames */
			for(i=0; i<NUM_ELEMENTS(SbButtonNames); i++)
				if(!stricmp(pc, SbButtonNames[i]))
					{
						lineParsed=1;

						/* Save the next token in the appropriate slot */
						pc=strtok(NULL," \t\n,");

						strncpy(cfgButtons[i], pc, MAX_STRING_LENGTH-1);
						cfgButtons[i][MAX_STRING_LENGTH-1]=0;
					}

			/* If the first token is not from GameButtonNames,
			** it must be a WarpName
			*/
			if(!lineParsed)
				{
					char *name;

					name=pc;
					pc=pc+strlen(pc)+1;			/* Skip this token */
					while(isspace(*pc)) pc++;	/* Skip any whitespace */

					if(*pc=='{')
						{
							WarpRecord warpRec;

							strcpy(warpRec.name, name);
							warpRec.pWarp=NULL;
							warpRec.nWarp=0;

							if(GetWarp(pc, &warpRec))
								{
									if(nCfgWarps++==0)
										pCfgWarps=(WarpRecord *)malloc(sizeof(WarpRecord));
									else
										pCfgWarps=(WarpRecord *)realloc(pCfgWarps, nCfgWarps*sizeof(WarpRecord));
									pCfgWarps[nCfgWarps-1]=warpRec;
								}
						}
				}
		} /* end of while getting lines from config file */

	fclose(file);
   return 1;
}

/*-------------------------------------------------------------------------*/

char *SbConfigGetButton(char *btnName)
{
	int i;
	for(i=0; i<NUM_ELEMENTS(SbButtonNames); i++)
		{
			if(!stricmp(btnName, SbButtonNames[i]))
				if(cfgButtons[i][0])
					return cfgButtons[i];
				else
					return NULL;	/* Empty slot */

			if(!stricmp(btnName, cfgButtons[i]))
				return SbButtonNames[i];
		}

	/* Unknown button name */
	return NULL;
}



int SbConfigGetButtonNumber(char *btnName)
{
	int i;
	for(i=0; i<NUM_ELEMENTS(SbButtonNames); i++)
		{
			if(!stricmp(btnName, cfgButtons[i]))
				return i;
		}

	/* Unknown button name */
	return -1;
}

/*-------------------------------------------------------------------------*/

WarpRecord *SbConfigGetWarpRange(char *rngName)
{
	int i;
	for(i=0; i<nCfgWarps; i++)
		if(!stricmp(rngName, pCfgWarps[i].name))
			return &pCfgWarps[i];

	return NULL;
}

/*-------------------------------------------------------------------------*/

fixed SbFxConfigWarp(WarpRecord *warp, short value)
{
	int  i;
	short absValue;
	fixed accum;
	int   sign;

	if(!warp) return INT_TO_FIXED(value);
	
	absValue=value;
	sign=1;
	if(absValue<0)
		{
			absValue= (short)-absValue;
			sign=-1;
		}

	accum=INT_TO_FIXED(0);

	for(i=0; i<warp->nWarp; i++)
		{
      if(absValue<=warp->pWarp[i].low)
		      ;	/* Ignore it if below this range (if required, will have
					**	been caught by the previous warp)
					*/
			else if((absValue>warp->pWarp[i].low) && (absValue<=warp->pWarp[i].high))
				{
					fixed diff;
					fixed partial;
					diff=INT_TO_FIXED((long)absValue-(long)warp->pWarp[i].low);
               partial=FIXED_MUL(diff, warp->pWarp[i].mult);
					accum=FIXED_ADD(accum, partial);
					break;	/* Exit the for loop */
				}
			else	/* Accumulate if greater than this range */
				{
					fixed partial;
					partial=FIXED_MUL(INT_TO_FIXED((long)warp->pWarp[i].high-(long)warp->pWarp[i].low),
										   warp->pWarp[i].mult);
					accum=FIXED_ADD(accum, partial);
				}
		}

	if(sign==1)
		return  accum;
	else
		return -accum;

#if 0	/* Old technique */
		if((absValue>=warp->pWarp[i].low) && (absValue<=warp->pWarp[i].high))
			if(warp->pWarp[i].shift>=0)
				value=accum+((absValue-warp->pWarp[i].low)<<warp->pWarp[i].shift);
			else
				value=accum+((absValue-warp->pWarp[i].low)>>abs(warp->pWarp[i].shift));
		else
			if(warp->pWarp[i].shift>=0)
				accum+=(warp->pWarp[i].high-warp->pWarp[i].low)<<warp->pWarp[i].shift;
			else
				accum+=(warp->pWarp[i].high-warp->pWarp[i].low)>>abs(warp->pWarp[i].shift);

	if(absValue>warp->pWarp[warp->nWarp-1].high)
		value=accum;

	return sign*value;
#endif
}


/*-------------------------------------------------------------------------*/

long SbConfigWarp(WarpRecord *warp, short value)
{
   /* An apparent bug in the msc70 compiler, trashes r
      if it is on the stack.  Leave in unitialized global segment.
   */
   static fixed r;

   r = SbFxConfigWarp(warp,value);

   return r >> 16 ;
}


/*-------------------------------------------------------------------------*/

