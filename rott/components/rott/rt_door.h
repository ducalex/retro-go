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
#ifndef _rt_door_public
#define _rt_door_public

//***************************************************************************
//
//   RT_DOOR.C - doors
//
//***************************************************************************

#define MAXTOUCHPLATES 64
#define MAXMASKED      300  // max masked walls
#define MAXDOORS       150  // max number of sliding doors
#define MAXPWALLS      150  // max number of pushwalls
#define DF_TIMED       0x01
#define DF_ELEVLOCKED  0x02
#define DF_MULTI       0x04
#define MAXELEVATORS   16

#define PW_DAMAGE      0x01

#define NUMELEVATORACTIONS 5

typedef enum
{ev_ras, //ready at source
 ev_rad, //ready at destination
 ev_mts, //moving to source
 ev_mtd,  //moving to destination
			 //door at elevator location open
 ev_doorclosing //door at elevator location closed
}estate;

typedef enum
{
 mw_peephole,
 mw_dogwall,
 mw_multi1,
 mw_multi2,
 mw_multi3,
 mw_singlepane,
 mw_normal1,
 mw_normal2,
 mw_normal3,
 mw_exitarch,
 mw_secretexitarch,
 mw_railing,
 mw_hiswitchon,
 mw_hiswitchoff,
 mw_platform1,
 mw_platform2,
 mw_platform3,
 mw_platform4,
 mw_platform5,
 mw_platform6,
 mw_platform7,
 mw_entrygate
}masked_walls;

typedef struct elevator
{ short sx,sy;
  short dx,dy;
  short esx,esy,edx,edy;
  short door1,door2;
  signed char state;
  short doortoopen;
  short doorclosing;
  short ticcount;
  short nextaction;
}elevator_t;


typedef struct doorstruct
{
		  thingtype   which;
		  byte        tilex,tiley;
		  word        texture;
		  word        alttexture;
		  word        sidepic;
		  word        basetexture;
		  byte        lock;
		  byte        flags;
		  short int   ticcount;
		  signed char eindex;
		  boolean     vertical;
		  int         soundhandle;
		  int         position;
		  enum    {dr_open,dr_closed,dr_opening,dr_closing}       action;
} doorobj_t;

typedef struct pwallstruct
{
		  thingtype      which;
		  int       x,y;
		  int       momentumx,momentumy;
		  byte      areanumber;
		  byte      lock;
		  byte      dir;
		  byte      tilex,tiley;
		  byte      num;
		  byte      speed;
		  word      texture;
		  int       soundhandle;
		  enum      {pw_npushed,pw_pushing,pw_pushed,pw_moving}       action;
		  int       state;
        byte      flags;
} pwallobj_t;

typedef struct tplate
{ void (*action)(long);
  void (*swapaction)(long);
  struct tplate * nextaction;
  struct tplate * prevaction;
  long whichobj;
  byte tictime;
  byte ticcount;
  byte triggered;
  byte done;
  byte complete;
  byte clocktype;
} touchplatetype;

#define MW_SHOOTABLE       0x01
#define MW_BLOCKING        0x02
#define MW_MULTI           0x04
#define MW_BLOCKINGCHANGES 0x08
#define MW_ABOVEPASSABLE   0x10
#define MW_NONDOGBLOCKING  0x20
#define MW_WEAPONBLOCKING  0x40
#define MW_BOTTOMPASSABLE  0x80
#define MW_MIDDLEPASSABLE  0x100
#define MW_ABP             0x200
#define MW_SWITCHON        0x400
#define MW_BOTTOMFLIPPING  0x800
#define MW_TOPFLIPPING     0x1000
#define M_ISDOOR(x,y) ((tilemap[x][y] & 0x8000) && (!(tilemap[x][y] & 0x4000)))
#define M_ISMWALL(x,y) ((tilemap[x][y] & 0x8000) && (tilemap[x][y] & 0x4000))


typedef struct mwall
{
  thingtype      which;
  byte      tilex,tiley;
  signed char areanumber;
  signed short toptexture;
  signed short midtexture;
  signed short bottomtexture;
  word      flags;
  boolean   vertical;
  int       sidepic;

  struct mwall *next;
  struct mwall *prev;

} maskedwallobj_t;

typedef struct animmwall
{
  word     num;
  byte     count;
  signed char ticcount;
  struct animmwall *next;
  struct animmwall *prev;

} animmaskedwallobj_t;

extern elevator_t          *ELEVATOR;
extern int                 _numelevators;
extern animmaskedwallobj_t *FIRSTANIMMASKEDWALL,*LASTANIMMASKEDWALL;
extern maskedwallobj_t     *FIRSTMASKEDWALL,*LASTMASKEDWALL;
extern word                (*touchindices)[MAPSIZE];
extern int                 lasttouch;
extern touchplatetype      **touchplate,**lastaction;
extern byte                *TRIGGER;
extern byte                *numactions;

extern doorobj_t           **doorobjlist;
extern int                 doornum;
extern maskedwallobj_t     **maskobjlist;
extern int                 maskednum;
extern pwallobj_t          **pwallobjlist;
extern int                 pwallnum;
																																			 // 0xffff = fully open
extern byte                (*areaconnect)[NUMAREAS];
extern boolean             *areabyplayer;


void ActivateAllPushWalls(void);
boolean CheckTile(int,int);
void FindEmptyTile(int*,int*);
int  Number_of_Empty_Tiles_In_Area_Around(int,int);
void AddTouchplateAction(touchplatetype*,int);
void RemoveTouchplateAction(touchplatetype*,int);

void InitElevators(void);
void ProcessElevators(void);
void OperateElevatorDoor(int);


int  PlatformHeight(int,int);
void Link_To_Touchplate(word, word, void (*)(long), void (*)(long), long, int);
void TriggerStuff(void);
void ClockLink(void (*)(long),void(*)(long),long,int);
void RecursiveConnect(int);
void ConnectAreas(void);
void InitAreas(void);
void InitDoorList(void);
void SpawnDoor(int,int,int,int);
void SpawnMaskedWall (int tilex, int tiley, int which, int flags);
void OpenDoor(int);
void CloseDoor(int);
void OperateDoor (int keys, int door, boolean localplayer );
void DoorOpen(int);
void DoorOpening(int);
void DoorClosing(int door);
void MoveDoors(void);
void SpawnPushWall (int tilex, int tiley, int lock, int texture, int dir, int type);
void MovePWalls(void);
void WallPushing (int pwall);
void PushWall (int pwall, int dir);
void OperatePushWall (int pwall, int dir, boolean localplayer );
void ActivatePushWall (long pwall);
void ActivateMoveWall (long pwall);
int  UpdateMaskedWall (int num);

void FixDoorAreaNumbers ( void );
void FixMaskedWallAreaNumbers ( void );
void SaveMaskedWalls(byte ** buf, int * size);
void LoadMaskedWalls(byte * buf, int size);
void SaveDoors(byte ** buf, int * size);
void SaveTouchPlates(byte ** buf, int * size);
void LoadDoors(byte * buf, int size);
void LoadTouchPlates(byte * buf, int size);
void SavePushWalls(byte ** buf, int * sz);
void LoadPushWalls(byte * bufptr, int sz);

void DeactivateAnimMaskedWall(animmaskedwallobj_t* amwall);
void ActivateAnimMaskedWall(animmaskedwallobj_t* amwall);

void SpawnAnimatedMaskedWall ( int num );
void KillAnimatedMaskedWall ( animmaskedwallobj_t * temp );

void DoAnimatedMaskedWalls ( void );

void SaveElevators(byte ** buffer,int *size);

void LoadElevators(byte * buffer,int size);

void MakeWideDoorVisible ( int doornum );
void LinkedCloseDoor (long door);
void LinkedOpenDoor (long door);
int IsWall (int tilex, int tiley);
int IsDoor (int tilex, int tiley);
int IsMaskedWall (int tilex, int tiley);
#endif
