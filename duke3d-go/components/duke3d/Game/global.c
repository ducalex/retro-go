//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aint32_t with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "global.h"
#include "duke3d.h"

#include "esp_attr.h"

#include "SDL.h"

char  *mymembuf;
EXT_RAM_ATTR uint8_t  MusicPtr[1];//72000


crc32_t crc32lookup[] = {
		// good:
		{ CRC_BASE_GRP_SHAREWARE_13,	"SHAREWARE 1.3D", 11035779 },
		{ CRC_BASE_GRP_FULL_13,			"FULL 1.3D     ", 26524524 },
		{ CRC_BASE_GRP_PLUTONIUM_14,	"PLUTONIUM 1.4 ", 44348015 },
		{ CRC_BASE_GRP_ATOMIC_15,		"ATOMIC 1.5    ", 44356548 },
		// unknown:
		{ 0,							"HACK/UNKNOWN  ", 0}
	};

uint8_t  conVersion = 13;
uint8_t  grpVersion = 0;

// FIX_00015: Backward compliance with older demos (down to demos v27, 28, 116 and 117 only)

// For BYTEVERSION diff, 27/116 vs 28/117 see extras\duke3d.h vs source\duke3d.h
// from the official source code release. 

int BYTEVERSION_27 = 27; // 1.3 under 1.4 Plutonium. Not supported anymore
int BYTEVERSION_116 = 116; // 1.4 Plutonium. Not supported anymore

int BYTEVERSION_28 = 28; // 1.3 under 1.5 engine
int BYTEVERSION_117 = 117; // 1.5 Atomic

int BYTEVERSION_29 = 29; // 1.3 under xDuke v19.6.
int BYTEVERSION_118 = 118; // 1.5 Atomic under xDuke v19.6.

int BYTEVERSION_1_3 = 1; // for 1.3 demos (Not compatible)

int BYTEVERSION			= 119; // xDuke v19.7

short global_random;
short neartagsector, neartagwall, neartagsprite;

int32_t gc,neartaghitdist,lockclock,max_player_health,max_armour_amount,max_ammo_amount[MAX_WEAPONS];

// int32_t temp_data[MAXSPRITES][6];
EXT_RAM_ATTR struct weaponhit hittype[MAXSPRITES];
EXT_RAM_ATTR short spriteq[1024];
short spriteqloc,spriteqamount=64;

// ported build engine has this, too.  --ryan.
#if PLATFORM_DOS
short moustat = 0;
#endif

EXT_RAM_ATTR struct animwalltype animwall[MAXANIMWALLS];
short numanimwalls;
EXT_RAM_ATTR int32_t *animateptr[MAXANIMATES], animategoal[MAXANIMATES], animatevel[MAXANIMATES], animatecnt;
// int32_t oanimateval[MAXANIMATES];
short animatesect[MAXANIMATES];
EXT_RAM_ATTR int32_t msx[2048],msy[2048];
short cyclers[MAXCYCLERS][6],numcyclers;

EXT_RAM_ATTR char  fta_quotes[NUMOFFIRSTTIMEACTIVE][64];

EXT_RAM_ATTR uint8_t  tempbuf[2048];
EXT_RAM_ATTR uint8_t packbuf[576];

char  buf[80];

short camsprite;
short mirrorwall[64], mirrorsector[64], mirrorcnt;

int current_menu;

uint8_t  betaname[80];

EXT_RAM_ATTR char level_names[44][33];
EXT_RAM_ATTR char level_file_names[44][128];
int32_t partime[44],designertime[44];
char  volume_names[4][33] = { "L.A. MELTDOWN", "LUNAR APOCALYPSE", "SHRAPNEL CITY", "" }; // Names are not in 1.3 con files. MUST be in code.
char  skill_names[5][33] = { "PIECE OF CAKE", "LET'S ROCK", "COME GET SOME", "DAMN I'M GOOD", "" };

volatile int32_t checksume;
EXT_RAM_ATTR int32_t soundsiz[NUM_SOUNDS];

EXT_RAM_ATTR short soundps[NUM_SOUNDS],soundpe[NUM_SOUNDS],soundvo[NUM_SOUNDS];
EXT_RAM_ATTR uint8_t  soundm[NUM_SOUNDS],soundpr[NUM_SOUNDS];
EXT_RAM_ATTR char  sounds[NUM_SOUNDS][14];

short title_zoom;

fx_device device;

EXT_RAM_ATTR SAMPLE Sound[ NUM_SOUNDS ];
EXT_RAM_ATTR SOUNDOWNER SoundOwner[NUM_SOUNDS][4];

uint8_t  numplayersprites,earthquaketime;

int32_t fricxv,fricyv;
struct player_orig po[MAXPLAYERS];
EXT_RAM_ATTR struct player_struct ps[MAXPLAYERS];
struct user_defs ud;

uint8_t  pus, pub;
EXT_RAM_ATTR uint8_t  syncstat, syncval[MAXPLAYERS][MOVEFIFOSIZ];
int32_t syncvalhead[MAXPLAYERS], syncvaltail, syncvaltottail;

input sync[MAXPLAYERS], loc;
EXT_RAM_ATTR input recsync[RECSYNCBUFSIZ];
int32_t avgfvel, avgsvel, avgavel, avghorz, avgbits;


EXT_RAM_ATTR input inputfifo[MOVEFIFOSIZ][MAXPLAYERS];
EXT_RAM_ATTR input recsync[RECSYNCBUFSIZ];

int32_t movefifosendplc;

  //Multiplayer syncing variables
short screenpeek;
int32_t movefifoend[MAXPLAYERS];


    //Game recording variables

uint8_t  playerreadyflag[MAXPLAYERS],ready2send;
uint8_t  playerquitflag[MAXPLAYERS];
int32_t vel, svel, angvel, horiz, ototalclock, respawnactortime=768, respawnitemtime=768, groupfile;

EXT_RAM_ATTR int32_t script[MAXSCRIPTSIZE];
int32_t *scriptptr,*insptr,*labelcode,labelcnt;
EXT_RAM_ATTR int32_t *actorscrptr[MAXTILES];
int32_t *parsing_actor;
char  *label,*textptr,error,warning ;
uint8_t killit_flag;
uint8_t  *music_pointer;
EXT_RAM_ATTR uint8_t  actortype[MAXTILES];


uint8_t  display_mirror,typebuflen;
char typebuf[41];

EXT_RAM_ATTR char  music_fn[4][11][13];
uint8_t music_select;
char  env_music_fn[4][13];
uint8_t  rtsplaying;


short weaponsandammosprites[15] = {
        RPGSPRITE,
        CHAINGUNSPRITE,
        DEVISTATORAMMO,
        RPGAMMO,
        RPGAMMO,
        JETPACK,
        SHIELD,
        FIRSTAID,
        STEROIDS,
        RPGAMMO,
        RPGAMMO,
        RPGSPRITE,
        RPGAMMO,
        FREEZESPRITE,
        FREEZEAMMO
    };

int32_t impact_damage;

        //GLOBAL.C - replace the end "my's" with this
int32_t myx, omyx, myxvel, myy, omyy, myyvel, myz, omyz, myzvel;
short myhoriz, omyhoriz, myhorizoff, omyhorizoff;
short myang, omyang, mycursectnum, myjumpingcounter,frags[MAXPLAYERS][MAXPLAYERS];

uint8_t  myjumpingtoggle, myonground, myhardlanding, myreturntocenter;
int8_t multiwho, multipos, multiwhat, multiflag;

int32_t fakemovefifoplc,movefifoplc;
EXT_RAM_ATTR int32_t myxbak[MOVEFIFOSIZ], myybak[MOVEFIFOSIZ], myzbak[MOVEFIFOSIZ];
int32_t myhorizbak[MOVEFIFOSIZ],dukefriction = 0xcc00, show_shareware;

short myangbak[MOVEFIFOSIZ];
char  myname[2048] = "XDUKE";
uint8_t  camerashitable,freezerhurtowner=0,lasermode;
// CTW - MODIFICATION
// uint8_t  networkmode = 255, movesperpacket = 1,gamequit = 0,playonten = 0,everyothertime;
uint8_t  networkmode = 255, movesperpacket = 1,gamequit = 0,everyothertime;
// CTW END - MODIFICATION
int32_t numfreezebounces=3,rpgblastradius,pipebombblastradius,tripbombblastradius,shrinkerblastradius,morterblastradius,bouncemineblastradius,seenineblastradius;
STATUSBARTYPE sbar;

int32_t myminlag[MAXPLAYERS], mymaxlag, otherminlag, bufferjitter = 1;
short numclouds,clouds[128],cloudx[128],cloudy[128];
int32_t cloudtotalclock = 0,totalmemory = 0;
int32_t numinterpolations = 0, startofdynamicinterpolations = 0;
EXT_RAM_ATTR int32_t oldipos[MAXINTERPOLATIONS];
EXT_RAM_ATTR int32_t bakipos[MAXINTERPOLATIONS];
EXT_RAM_ATTR int32_t *curipos[MAXINTERPOLATIONS];


// portability stuff.  --ryan.
// A good portion of this was ripped from GPL'd Rise of the Triad.  --ryan.

void FixFilePath(char  *filename)
{
#if PLATFORM_UNIX
    uint8_t  *ptr;
    uint8_t  *lastsep = filename;

    if ((!filename) || (*filename == '\0'))
        return;

    if (access(filename, F_OK) == 0)  /* File exists; we're good to go. */
        return;

    for (ptr = filename; 1; ptr++)
    {
        if (*ptr == '\\')
            *ptr = PATH_SEP_CHAR;

        if ((*ptr == PATH_SEP_CHAR) || (*ptr == '\0'))
        {
            uint8_t  pch = *ptr;
            struct dirent *dent = NULL;
            DIR *dir;

            if ((pch == PATH_SEP_CHAR) && (*(ptr + 1) == '\0'))
                return; /* eos is pathsep; we're done. */

            if (lastsep == ptr)
                continue;  /* absolute path; skip to next one. */

            *ptr = '\0';
            if (lastsep == filename) {
                dir = opendir((*lastsep == PATH_SEP_CHAR) ? ROOTDIR : CURDIR);
                
                if (*lastsep == PATH_SEP_CHAR) {
                    lastsep++;
                }
            } 
            else
            {
                *lastsep = '\0';
                dir = opendir(filename);
                *lastsep = PATH_SEP_CHAR;
                lastsep++;
            }

            if (dir == NULL)
            {
                *ptr = PATH_SEP_CHAR;
                return;  /* maybe dir doesn't exist? give up. */
            }

            while ((dent = readdir(dir)) != NULL)
            {
                if (strcasecmp(dent->d_name, lastsep) == 0)
                {
                    /* found match; replace it. */
                    strcpy(lastsep, dent->d_name);
                    break;
                }
            }

            closedir(dir);
            *ptr = pch;
            lastsep = ptr;

            if (dent == NULL)
                return;  /* no match. oh well. */

            if (pch == '\0')  /* eos? */
                return;
        }
    }
#endif
}


#if PLATFORM_DOS
 /* no-op. */

#elif PLATFORM_WIN32
int _dos_findfirst(uint8_t  *filename, int x, struct find_t *f)
{
    int32_t rc = _findfirst(filename, &f->data);
    f->handle = rc;
    if (rc != -1)
    {
        strncpy(f->name, f->data.name, sizeof (f->name) - 1);
        f->name[sizeof (f->name) - 1] = '\0';
        return(0);
    }
    return(1);
}

int _dos_findnext(struct find_t *f)
{
    int rc = 0;
    if (f->handle == -1)
        return(1);   /* invalid handle. */

    rc = _findnext(f->handle, &f->data);
    if (rc == -1)
    {
        _findclose(f->handle);
        f->handle = -1;
        return(1);
    }

    strncpy(f->name, f->data.name, sizeof (f->name) - 1);
    f->name[sizeof (f->name) - 1] = '\0';
    return(0);
}

#elif defined(PLATFORM_UNIX) || defined(PLATFORM_MACOSX) || defined(PLATFORM_ESP32)
int _dos_findfirst(char  *filename, int x, struct find_t *f)
{
    char  *ptr;

    if (strlen(filename) >= sizeof (f->pattern))
        return(1);

    strcpy(f->pattern, filename);
    FixFilePath(f->pattern);
    ptr = strrchr(f->pattern, PATH_SEP_CHAR);

    if (ptr == NULL)
    {
        ptr = filename;
        f->dir = opendir(CURDIR);
    }
    else
    {
        *ptr = '\0';
        SDL_LockDisplay();
        f->dir = opendir(f->pattern);
        SDL_UnlockDisplay();
        memmove(f->pattern, ptr + 1, strlen(ptr + 1) + 1);
    }

    return(_dos_findnext(f));
}


static int check_pattern_nocase(const char  *x, const char  *y)
{
    if ((x == NULL) || (y == NULL))
        return(0);  /* not a match. */

    while ((*x) && (*y))
    {
        if (*x == '*')
        {
            x++;
            while (*y != '\0')
            {
                if (toupper((int) *x) == toupper((int) *y))
                    break;
                y++;
            }
        }

        else if (*x == '?')
        {
            if (*y == '\0')
                return(0);  /* anything but EOS is okay. */
        }

        else
        {
            if (toupper((int) *x) != toupper((int) *y))
                return(0);  /* not a match. */
        }

        x++;
        y++;
    }

    return(*x == *y);  /* it's a match (both should be EOS). */
}

int _dos_findnext(struct find_t *f)
{
    struct dirent *dent;

    if (f->dir == NULL)
        return(1);  /* no such dir or we're just done searching. */
    SDL_LockDisplay();
    while ((dent = readdir(f->dir)) != NULL)
    {
        if (check_pattern_nocase(f->pattern, dent->d_name))
        {
            if (strlen(dent->d_name) < sizeof (f->name))
            {
                strcpy(f->name, dent->d_name);
                return(0);  /* match. */
            }
        }
    }

    closedir(f->dir);
    SDL_UnlockDisplay();
    f->dir = NULL;
    return(1);  /* no match in whole directory. */
}
#else
#error please define for your platform.
#endif


#if !PLATFORM_DOS
void _dos_getdate(struct dosdate_t *date)
{
	time_t curtime = time(NULL);
	struct tm *tm;
	
	if (date == NULL) {
		return;
	}
	
	memset(date, 0, sizeof(struct dosdate_t));
	
	if ((tm = localtime(&curtime)) != NULL) {
		date->day = tm->tm_mday;
		date->month = tm->tm_mon + 1;
		date->year = tm->tm_year + 1900;
		date->dayofweek = tm->tm_wday + 1;
	}
}
#endif


int FindDistance2D(int ix, int iy)
{
  int   t;

  ix= abs(ix);        /* absolute values */
  iy= abs(iy);

  if (ix<iy)
  {
     int tmp = ix;
     ix = iy;
     iy = tmp;
  }

  t = iy + (iy>>1);

  return (ix - (ix>>5) - (ix>>7)  + (t>>2) + (t>>6));
}

int FindDistance3D(int ix, int iy, int iz)
{
   int   t;

   ix= abs(ix);           /* absolute values */
   iy= abs(iy);
   iz= abs(iz);

   if (ix<iy)
   {
     int tmp = ix;
     ix = iy;
     iy = tmp;
   }

   if (ix<iz)
   {
     int tmp = ix;
     ix = iz;
     iz = tmp;
   }

   t = iy + iz;

   return (ix - (ix>>4) + (t>>2) + (t>>3));
}
#include "SDL.h"
void Error (int errorType, char  *error, ...)
{
   va_list argptr;

   SDL_Quit();

   //FCS: http://duke3d.m-klein.com is obscolete :/ !
   /*
   if(errorType==EXIT_FAILURE)
	   printf("ERROR: Please copy that screen and visit http://duke3d.m-klein.com for report:\n");
	else
		printf("http://duke3d.m-klein.com\n");
	*/


   va_start (argptr, error);
   vprintf(error, argptr);
   va_end (argptr);

   //printf("Press any key to continue...\n");

	// FIX_00043: Nicer exit on error. Ask the user to hit a key on exits and error exits.
   //getch();

   exit (errorType);
}

void write2disk(int line, char * cfilename, char  *filename2write, char  *message)
{
	// usage: write2disk(__LINE__, __FILE__, "c:\temp\my_dbug_file.txt", uint8_t * msg);

	int i, k=0;
	char  filename[2048];
	FILE *pFile;

	for(i=0; cfilename[i]; i++)
	{
		if(cfilename[i]=='\\')
		{
			i++;
			k = 0;
		}
		filename[k++]=(cfilename[i]=='.')?0:cfilename[i];
	}
    SDL_LockDisplay();
	pFile = fopen(filename2write,"a");
	fprintf(pFile,"%-4d %-5s %s", line, filename, message);
	fclose(pFile);
    SDL_UnlockDisplay();
}

int32 SafeOpenAppend (const char  *_filename, int32 filetype)
{
	int	handle;
    char  filename[MAX_PATH];
    
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);
    SDL_LockDisplay();
#if (defined PLATFORM_WIN32)
    handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_APPEND );
#else
	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_APPEND , S_IREAD | S_IWRITE);
#endif
    SDL_UnlockDisplay();
	if (handle == -1)
		Error (EXIT_FAILURE, "Error opening for append %s: %s",filename,strerror(errno));

	return handle;
}

boolean SafeFileExists ( const char  * _filename )
{
    char  filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);
    SDL_LockDisplay();
    int ret;
#if( defined PLATFORM_WIN32)
    ret = access(filename, 6)      
#else
    ret = access(filename, F_OK);
#endif
    SDL_UnlockDisplay();
    return( ret == 0);
}


int32 SafeOpenWrite (const char  *_filename, int32 filetype)
{
	int	handle;
    char  filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);
    SDL_LockDisplay();
#if (defined PLATFORM_WIN32)
    handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_TRUNC );
#else
	handle = open(filename,O_RDWR | O_BINARY | O_CREAT | O_TRUNC
	, S_IREAD | S_IWRITE);
#endif
    SDL_UnlockDisplay();
	if (handle == -1)
		Error (EXIT_FAILURE, "Error opening %s: %s",filename,strerror(errno));

	return handle;
}




int32 SafeOpenRead (const char  *_filename, int32 filetype)
{
	int	handle;
    char  filename[MAX_PATH];
    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);
    SDL_LockDisplay();
	handle = open(filename,O_RDONLY | O_BINARY);
    SDL_UnlockDisplay();
	if (handle == -1)
		Error (EXIT_FAILURE, "Error opening %s: %s",filename,strerror(errno));

	return handle;
}


void SafeRead (int32 handle, void *buffer, int32 count)
{
	unsigned	iocount;
    SDL_LockDisplay();
	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (read (handle,buffer,iocount) != (int)iocount)
			Error (EXIT_FAILURE, "File read failure reading %ld bytes",count);
		buffer = (void *)( (byte *)buffer + iocount );
		count -= iocount;
	}
    SDL_UnlockDisplay();
}


void SafeWrite (int32 handle, void *buffer, int32 count)
{
	unsigned	iocount;
    SDL_LockDisplay();
	while (count)
	{
		iocount = count > 0x8000 ? 0x8000 : count;
		if (write (handle,buffer,iocount) != (int)iocount)
			Error (EXIT_FAILURE, "File write failure writing %ld bytes",count);
		buffer = (void *)( (byte *)buffer + iocount );
		count -= iocount;
	}
    SDL_UnlockDisplay();
}

void SafeWriteString (int handle, char  * buffer)
{
	unsigned	iocount;

    iocount=strlen(buffer);
    SDL_LockDisplay();
	if (write (handle,buffer,iocount) != (int)iocount)
			Error (EXIT_FAILURE, "File write string failure writing %s\n",buffer);
    SDL_UnlockDisplay();
}

void *SafeMalloc (int32_t size)
{
	void *ptr;

    ptr = malloc(size);

	if (!ptr)
      Error (EXIT_FAILURE, "SafeMalloc failure for %lu bytes",size);

	return ptr;
}

void SafeRealloc (void **x, int32 size)
{
	void *ptr;

    ptr = realloc(*x, size);

	if (!ptr)
      Error (EXIT_FAILURE, "SafeRealloc failure for %lu bytes",size);

    *x = ptr;
}

void *SafeLevelMalloc (int32_t size)
{
	void *ptr;

    ptr = malloc(size);

	if (!ptr)
      Error (EXIT_FAILURE, "SafeLevelMalloc failure for %lu bytes",size);

	return ptr;
}

void SafeFree (void * ptr)
{
   if ( ptr == NULL )
      Error (EXIT_FAILURE, "SafeFree : Tried to free a freed pointer\n");

    free(ptr);

}

short	SwapShort (short l)
{
	byte	b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}

short	KeepShort (short l)
{
	return l;
}


int32_t	Swapint32_t (int32_t l)
{
	byte	b1,b2,b3,b4;

	b1 = l&255;
	b2 = (l>>8)&255;
	b3 = (l>>16)&255;
	b4 = (l>>24)&255;

	return ((int32_t)b1<<24) + ((int32_t)b2<<16) + ((int32_t)b3<<8) + b4;
}

int32_t	Keepint32_t (int32_t l)
{
	return l;
}


#undef KeepShort
#undef KeepLong
#undef SwapShort
#undef SwapLong

void SwapIntelLong(int32_t *l)
{
    *l = IntelLong(*l);
}

void SwapIntelShort(short *s)
{
    *s = IntelShort(*s);
}

void SwapIntelLongArray(int32_t *l, int num)
{
    while (num--) {
        SwapIntelLong(l);
        l++;
    }
}

void SwapIntelShortArray(short *s, int num)
{
    while (num--) {
        SwapIntelShort(s);
        s++;
    }
}


/* 
  Copied over from Wolf3D Linux: http://www.icculus.org/wolf3d/
  Modified for ROTT.
  Stolen for Duke3D, too.
 */
 
#if PLATFORM_UNIX
uint8_t  *strlwr(uint8_t  *s)
{
	uint8_t  *p = s;
	
	while (*p) {
		*p = tolower(*p);
		p++;
	}
	
	return s;
}

uint8_t  *strupr(uint8_t  *s)
{
	uint8_t  *p = s;
	
	while (*p) {
		*p = toupper(*p);
		p++;
	}
	
	return s;
}
	
uint8_t  *itoa(int value, uint8_t  *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%d", value);
			break;
		case 16:
			sprintf(string, "%x", value);
			break;
		default:
			STUBBED("unknown radix");
			break;
	}
	
	return string;
}

uint8_t  *ltoa(int32_t value, uint8_t  *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%d", value);
			break;
		case 16:
			sprintf(string, "%x", value);
			break;
		default:
			STUBBED("unknown radix");
			break;
	}
	
	return string;
}

uint8_t  *ultoa(uint32_t value, uint8_t  *string, int radix)
{
	switch (radix) {
		case 10:
			sprintf(string, "%u", value);
			break;
		case 16:
			sprintf(string, "%ux", value);
			break;
		default:
			STUBBED("unknown radix");
			break;
	}
	
	return string;
}
#endif



int setup_homedir (void)
{
    char  ApogeePath[256];
#if PLATFORM_UNIX
	int err;

	snprintf (ApogeePath, sizeof (ApogeePath), "%s/.duke3d/", getenv ("HOME"));

	//err = mkdir (ApogeePath, S_IRWXU);
	err = mkdir (ApogeePath);
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


uint8_t    CheckParm (char  *check)
{
    int i;
    for (i = 1; i < _argc; i++)
    {
        if ((*(_argv[i]) == '-') && (strcmpi(_argv[i] + 1, check) == 0))
            return(i);
    }

    return(0);
}


static void (*shutdown_func)(void) = NULL;

void RegisterShutdownFunction( void (* shutdown) (void) )
{
    shutdown_func = shutdown;
}

void Shutdown(void)
{
    if (shutdown_func != NULL)
    {
        shutdown_func();
        shutdown_func = NULL;
    }
}


