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
//***************************************************************************
//
//    RT_TED.C - Ted stuff for maps and such
//
//***************************************************************************

#include "rottnet.h"


#ifndef _rt_ted_public
#define _rt_ted_public

#define MAXCLOCKS 10
#define MAXSPAWNLOCATIONS 50
#define POWERUPTICS  ((VBLCOUNTER*60)+6)
#define IsPlatform(x,y)  ( (MAPSPOT((x),(y),2)==1) || ((MAPSPOT((x),(y),2)>=4) && (MAPSPOT((x),(y),2)<=9)))

#define EXITTILE             (107)
#define SECRETEXITTILE       (106)

#define FL_SWITCH      0x01
#define FL_ON          0x02
#define FL_REVERSIBLE  0x04
#define FL_W_DAMAGE    0x08
#define FL_W_INVERTED  0x10
#define FL_S_FLIPPED   0x20



#define MAXTEAMS 11

typedef struct
{
 int nummembers;
 int uniformcolor;
 int tilex,tiley;
 byte dir;

} teamtype;

extern teamtype TEAM[MAXPLAYERS];

typedef struct
{ thingtype   which;
  byte        flags;
  byte        hitpoints;
  word        tile;
  byte        tilex,tiley;


} wall_t;

typedef struct
{
  int  number;
  char mapname[23];
} mapinfo_t;

typedef struct
{
  int  nummaps;
  mapinfo_t maps[100];
} mapfileinfo_t;

#define MAXLEVELNAMELENGTH 23
#define ALLOCATEDLEVELNAMELENGTH 24
#define NUMPLANES        3
#define NUMHEADEROFFSETS 100

#define MAP_SPECIAL_TOGGLE_PUSHWALLS 0x0001

typedef struct
   {
   unsigned used;
   unsigned CRC;
   unsigned RLEWtag;
   unsigned MapSpecials;
   unsigned planestart[ NUMPLANES ];
   unsigned planelength[ NUMPLANES ];
   char     Name[ ALLOCATEDLEVELNAMELENGTH ];
   } RTLMAP;


typedef struct
 {int x,y,dir;
 }_2dvec;

extern _2dvec SPAWNLOC[MAXSPAWNLOCATIONS],FIRST,SECOND;

typedef struct
 {int time1;
  int time2;
  byte points_to_tilex;
  byte points_to_tiley;
  int linkindex;
 }str_clock;


extern int  *numareatiles;
extern int  shapestart,shapestop;
extern int  NUMSPAWNLOCATIONS;
extern int  mapwidth;
extern int  mapheight;

extern  wall_t walls[MAXWALLTILES];
extern str_clock Clocks[MAXCLOCKS];
extern int *LightsInArea;
extern int numclocks;
extern word ELEVATORLOCATION;

extern unsigned short int *mapplanes[3];
extern int gunsstart;
extern int elevatorstart;
extern int spritestop;
extern int fog;
extern int lightsource;
extern int SNAKELEVEL;
extern boolean insetupgame;
extern char LevelName[80];
extern boolean ISRTL;

void PreCacheGroup(int,int,int); // added type
void AssignTeams(void);
void LoadTedMap( const char *extension, int mapnum );
void SetupGameLevel(void);
void ScanInfoPlane(void);
void PreCacheLump( int lump, int level, int type ); // added type
void SetupGameLevelAgain (void);
void ScanInfoPlaneAgain (void);
void PreCacheActor( int actor, int which );
void PreCache( void );

void SetupWalls( void );
void SetupAnimatedWalls( void );
void SetupSwitches( void );
void SetupPlayers( void );
void SetupMaskedWalls( void );
void SetupPushWalls( void );
void SetupPushWallLinks( void );
void SetupDoors (void);
void SetupDoorLinks (void);
void SetupClocks (void);
void SetupLinkedActors (void);
void SetupLights(void);
void SetupWindows ( void );

int GetWallIndex( int texture );
void PrintMapStats (void);
void PrintTileStats (void);

void GetMapInfo (mapfileinfo_t * mapinfo);
void GetMapFileName ( char * filename );
void SetBattleMapFileName ( char * filename );
word GetMapCRC ( int num );

int GetNextMap ( int tilex, int tiley );
void Illuminate();

int GetSongForLevel ( void );
void CheckHolidays(void);
boolean IsChristmas(void);

boolean DoPanicMapping (void);

#endif
