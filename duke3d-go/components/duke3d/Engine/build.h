/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file has been modified from Ken Silverman's original release
 */

#ifndef _INCLUDE_BUILD_H_
#define _INCLUDE_BUILD_H_
#ifdef _WIN32
   #include "windows/inttypes.h"
#else
   #include <inttypes.h>
#endif

#define MAXSECTORS 1024
#define MAXWALLS 8192
#define MAXSPRITES 4096
#define MAXTILES 9216

#define MAXSTATUS 1024
#define MAXPLAYERS 16
#define MAXXDIM 1600
#define MAXYDIM 1200
#define MAXPALOOKUPS 256
#define MAXPSKYTILES 256
#define MAXSPRITESONSCREEN 1024

#define CLIPMASK0 (((1L)<<16)+1L)
#define CLIPMASK1 (((256L)<<16)+64L)


	/*
     * Make all variables in BUILD.H defined in the ENGINE,
	 *  and externed in GAME
     * (dear lord.  --ryan.)
     */
#ifdef ENGINE
	#define EXTERN
#else
	#define EXTERN extern
#endif

#pragma pack(1)

#include "esp_attr.h"

/*
 * ceilingstat/floorstat:
 *   bit 0: 1 = parallaxing, 0 = not                                 "P"
 *   bit 1: 1 = groudraw, 0 = not
 *   bit 2: 1 = swap x&y, 0 = not                                    "F"
 *   bit 3: 1 = double smooshiness                                   "E"
 *   bit 4: 1 = x-flip                                               "F"
 *   bit 5: 1 = y-flip                                               "F"
 *   bit 6: 1 = Align texture to first wall of sector                "R"
 *   bits 7-8:                                                       "T"
 *          00 = normal floors
 *          01 = masked floors
 *          10 = transluscent masked floors
 *          11 = reverse transluscent masked floors
 *   bits 9-15: reserved
 */

	/* 40 bytes */
typedef struct
{
	short wallptr, wallnum;
	int32_t ceilingz, floorz;
	short ceilingstat, floorstat;
	short ceilingpicnum, ceilingheinum;
	int8_t ceilingshade;
	uint8_t  ceilingpal, ceilingxpanning, ceilingypanning;
	short floorpicnum, floorheinum;
	int8_t floorshade;
	uint8_t  floorpal, floorxpanning, floorypanning;
	uint8_t  visibility, filler;
	short lotag, hitag, extra;
} sectortype;

/*
 * cstat:
 *   bit 0: 1 = Blocking wall (use with clipmove, getzrange)         "B"
 *   bit 1: 1 = bottoms of invisible walls swapped, 0 = not          "2"
 *   bit 2: 1 = align picture on bottom (for doors), 0 = top         "O"
 *   bit 3: 1 = x-flipped, 0 = normal                                "F"
 *   bit 4: 1 = masking wall, 0 = not                                "M"
 *   bit 5: 1 = 1-way wall, 0 = not                                  "1"
 *   bit 6: 1 = Blocking wall (use with hitscan / cliptype 1)        "H"
 *   bit 7: 1 = Transluscence, 0 = not                               "T"
 *   bit 8: 1 = y-flipped, 0 = normal                                "F"
 *   bit 9: 1 = Transluscence reversing, 0 = normal                  "T"
 *   bits 10-15: reserved
 */

	/* 32 bytes */
typedef struct
{
	int32_t x, y;
	short point2, nextwall, nextsector, cstat;
	short picnum, overpicnum;
	int8_t shade;
	uint8_t  pal, xrepeat, yrepeat, xpanning, ypanning;
	short lotag, hitag, extra;
} walltype;


/*
 * cstat:
 *   bit 0: 1 = Blocking sprite (use with clipmove, getzrange)       "B"
 *   bit 1: 1 = transluscence, 0 = normal                            "T"
 *   bit 2: 1 = x-flipped, 0 = normal                                "F"
 *   bit 3: 1 = y-flipped, 0 = normal                                "F"
 *   bits 5-4: 00 = FACE sprite (default)                            "R"
 *             01 = WALL sprite (like masked walls)
 *             10 = FLOOR sprite (parallel to ceilings&floors)
 *   bit 6: 1 = 1-sided sprite, 0 = normal                           "1"
 *   bit 7: 1 = Real centered centering, 0 = foot center             "C"
 *   bit 8: 1 = Blocking sprite (use with hitscan / cliptype 1)      "H"
 *   bit 9: 1 = Transluscence reversing, 0 = normal                  "T"
 *   bits 10-14: reserved
 *   bit 15: 1 = Invisible sprite, 0 = not invisible
 */

	/* 44 bytes */
typedef struct
{
	int32_t x, y, z;
	short cstat, picnum;
	int8_t shade;
	uint8_t  pal, clipdist, filler;
	uint8_t  xrepeat, yrepeat;
	int8_t xoffset, yoffset;
	short sectnum, statnum;
	short ang, owner, xvel, yvel, zvel;
	short lotag, hitag, extra;
} spritetype;

#pragma pack()

EXTERN EXT_RAM_ATTR sectortype sector[MAXSECTORS];
EXTERN EXT_RAM_ATTR walltype wall[MAXWALLS];
EXTERN EXT_RAM_ATTR spritetype sprite[MAXSPRITES];

EXTERN uint16_t mapCRC;

EXTERN int32_t spritesortcnt;
EXTERN EXT_RAM_ATTR spritetype tsprite[MAXSPRITESONSCREEN];

EXTERN uint8_t  vidoption;
EXTERN int32_t xdim, ydim, numpages;

// Fast way to retrive the start of a column in the framebuffer, given a screenspace X coordinate.
EXTERN EXT_RAM_ATTR int32_t ylookup[MAXYDIM+1];

EXTERN int32_t yxaspect, viewingrange;

EXTERN int32_t validmodecnt;
EXTERN short validmode[256];
EXTERN EXT_RAM_ATTR int32_t validmodexdim[256], validmodeydim[256];

EXTERN short numsectors, numwalls;
EXTERN volatile int32_t totalclock;
EXTERN int32_t numframes, randomseed;
EXTERN EXT_RAM_ATTR short sintable[2048];
EXTERN EXT_RAM_ATTR uint8_t  palette[768];
EXTERN short numpalookups;
EXTERN uint8_t  *palookup[MAXPALOOKUPS];
EXTERN uint8_t  parallaxtype, showinvisibility;
EXTERN int32_t parallaxyoffs, parallaxyscale;
EXTERN int32_t visibility, parallaxvisibility;

EXTERN int32_t windowx1, windowy1, windowx2, windowy2;
EXTERN EXT_RAM_ATTR short startumost[MAXXDIM], startdmost[MAXXDIM];

EXTERN short pskyoff[MAXPSKYTILES], pskybits;

EXTERN EXT_RAM_ATTR short headspritesect[MAXSECTORS+1], headspritestat[MAXSTATUS+1];
EXTERN EXT_RAM_ATTR short prevspritesect[MAXSPRITES], prevspritestat[MAXSPRITES];
EXTERN EXT_RAM_ATTR short nextspritesect[MAXSPRITES], nextspritestat[MAXSPRITES];



    /*
	 * These variables are for auto-mapping with the draw2dscreen function.
	 * When you load a new board, these bits are all set to 0 - since
	 * you haven't mapped out anything yet.  Note that these arrays are
	 * bit-mapped.
	 * If you want draw2dscreen() to show sprite #54 then you say:
	 *    spritenum = 54;
	 *    show2dsprite[spritenum>>3] |= (1<<(spritenum&7));
	 * And if you want draw2dscreen() to not show sprite #54 then you say:
	 *    spritenum = 54;
	 *    show2dsprite[spritenum>>3] &= ~(1<<(spritenum&7));
	 * Automapping defaults to 0 (do nothing).  If you set automapping to 1,
	 *    then in 3D mode, the walls and sprites that you see will show up the
	 *    next time you flip to 2D mode.
     */
EXTERN EXT_RAM_ATTR uint8_t  show2dsector[(MAXSECTORS+7)>>3];
EXTERN EXT_RAM_ATTR uint8_t  show2dwall[(MAXWALLS+7)>>3];
EXTERN EXT_RAM_ATTR uint8_t  show2dsprite[(MAXSPRITES+7)>>3];
EXTERN uint8_t  automapping;



//This is the bit vector that marks visited sector during portal flooding. Size is hence (MAXSECTORS / 8)
EXTERN EXT_RAM_ATTR uint8_t  visitedSectors[(MAXSECTORS+7)>>3];

/*************************************************************************
POSITION VARIABLES:

		POSX is your x - position ranging from 0 to 65535
		POSY is your y - position ranging from 0 to 65535
			(the length of a side of the grid in EDITBORD would be 1024)
		POSZ is your z - position (height) ranging from 0 to 65535, 0 highest.
		ANG is your angle ranging from 0 to 2047.  Instead of 360 degrees, or
			 2 * PI radians, I use 2048 different angles, so 90 degrees would
			 be 512 in my system.

SPRITE VARIABLES:

	EXTERN short headspritesect[MAXSECTORS+1], headspritestat[MAXSTATUS+1];
	EXTERN short prevspritesect[MAXSPRITES], prevspritestat[MAXSPRITES];
	EXTERN short nextspritesect[MAXSPRITES], nextspritestat[MAXSPRITES];

	Example: if the linked lists look like the following:
		 ��������������������������������������������������������������Ŀ
		 �      Sector lists:               Status lists:               �
		 ��������������������������������������������������������������Ĵ
		 �  Sector0:  4, 5, 8             Status0:  2, 0, 8             �
		 �  Sector1:  16, 2, 0, 7         Status1:  4, 5, 16, 7, 3, 9   �
		 �  Sector2:  3, 9                                              �
		 ����������������������������������������������������������������
	Notice that each number listed above is shown exactly once on both the
		left and right side.  This is because any sprite that exists must
		be in some sector, and must have some kind of status that you define.


Coding example #1:
	To go through all the sprites in sector 1, the code can look like this:

		sectnum = 1;
		i = headspritesect[sectnum];
		while (i != -1)
		{
			nexti = nextspritesect[i];

			//your code goes here
			//ex: printf("Sprite %d is in sector %d\n",i,sectnum);

			i = nexti;
		}

Coding example #2:
	To go through all sprites with status = 1, the code can look like this:

		statnum = 1;        //status 1
		i = headspritestat[statnum];
		while (i != -1)
		{
			nexti = nextspritestat[i];

			//your code goes here
			//ex: printf("Sprite %d has a status of 1 (active)\n",i,statnum);

			i = nexti;
		}

			 insertsprite(short sectnum, short statnum);
			 deletesprite(short spritenum);
			 changespritesect(short spritenum, short newsectnum);
			 changespritestat(short spritenum, short newstatnum);

TILE VARIABLES:
		NUMTILES - the number of tiles found TILES.DAT.
		TILESIZX[MAXTILES] - simply the x-dimension of the tile number.
		TILESIZY[MAXTILES] - simply the y-dimension of the tile number.
		WALOFF[MAXTILES] - the actual 32-bit offset pointing to the top-left
								 corner of the tile.
		PICANM[MAXTILES] - flags for animating the tile.

TIMING VARIABLES:
		TOTALCLOCK - When the engine is initialized, TOTALCLOCK is set to zero.
			From then on, it is incremented 120 times a second by 1.  That
			means that the number of seconds elapsed is totalclock / 120.
		NUMFRAMES - The number of times the draw3dscreen function was called
			since the engine was initialized.  This helps to determine frame
			rate.  (Frame rate = numframes * 120 / totalclock.)

OTHER VARIABLES:

		STARTUMOST[320] is an array of the highest y-coordinates on each column
				that my engine is allowed to write to.  You need to set it only
				once.
		STARTDMOST[320] is an array of the lowest y-coordinates on each column
				that my engine is allowed to write to.  You need to set it only
				once.
		SINTABLE[2048] is a sin table with 2048 angles rather than the
			normal 360 angles for higher precision.  Also since SINTABLE is in
			all integers, the range is multiplied by 16383, so instead of the
			normal -1<sin(x)<1, the range of sintable is -16383<sintable[]<16383
			If you use this sintable, you can possibly speed up your code as
			well as save space in memory.  If you plan to use sintable, 2
			identities you may want to keep in mind are:
				sintable[ang&2047]       = sin(ang * (3.141592/1024)) * 16383
				sintable[(ang+512)&2047] = cos(ang * (3.141592/1024)) * 16383
		NUMSECTORS - the total number of existing sectors.  Modified every time
			you call the loadboard function.
***************************************************************************/

#define PORTSIG  "Port by Ryan C. Gordon, Andrew Henderson, Dan Olson, Fabien Sanglard and a cast of thousands."

//Global.c
void Error (int errorType, char  *error, ...);
int FindDistance2D(int ix, int iy);

#endif  /* defined _INCLUDE_BUILD_H_ */

/* end of build.h ... */



