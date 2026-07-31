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
#ifndef _rt_def_public
#define _rt_def_public

#include <assert.h>
#ifndef assert
#define assert(x) ((void)0)
#endif
void __assert_func(const char *file, int line, const char *func, const char *failedexpr);

#ifdef RETRO_GO
#define boolean esp_boolean
#include <rg_system.h>
#undef boolean
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use rott_boolean internally to avoid clashes with SDK or audiolib */
typedef unsigned char rott_boolean;
#define boolean rott_boolean
#define __boolean_defined

// RT_DEF.H Zee big one
#include "develop.h"
#define SAVE_SCREEN  1

#if PLATFORM_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <dirent.h>
#include <ctype.h>
#endif

#if (defined _MSC_VER)
/* __int64 is built in. */
#include <malloc.h>
#include <fcntl.h>
//#include <io.h>
//#define alloca(x) _alloca(x)
#define access(x, y) _access(x, y)
#define F_OK  0
#elif (defined __GNUC__)
#define __int64 long long
#else
#error please define your platform.
#endif

#if PLATFORM_DOS || PLATFORM_WIN32
#define PATH_SEP_CHAR '\\'
#define PATH_SEP_STR  "\\"
#elif PLATFORM_UNIX
#define PATH_SEP_CHAR '/'
#define PATH_SEP_STR  "/"
#define ROOTDIR       "/"
#define CURDIR        "./"
#elif PLATFORM_MACCLASSIC
#define PATH_SEP_CHAR ':'
#define PATH_SEP_STR  ":"
#else
#error please define your platform.
#endif

#if (!defined MAX_PATH)
  #if (defined MAXPATHLEN)
    #define MAX_PATH MAXPATHLEN
  #elif (defined PATH_MAX)
    #define MAX_PATH PATH_MAX
  #else
    #define MAX_PATH 256
  #endif
#endif

//
//
//
//***************************************************************************
//
//    Global Constants
//
//***************************************************************************

#ifndef DATADIR
#ifdef RETRO_GO
#define DATADIR RG_BASE_PATH_ROMS "/rott/"
#else
#define DATADIR ""
#endif
#endif


#undef PI
#undef M_PI

#ifndef NULL
#define NULL       0
#endif

#define PI      3.141592657
#define LONG(a) ((int)a)
#define M_PI            3.14159

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef O_TEXT
#define O_TEXT 0
#endif

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#if !PLATFORM_DOS
  #if PLATFORM_WIN32
    #define strcmpi(x, y) stricmp(x, y)
    #define _fstricmp(x, y) stricmp(x, y)
  #elif PLATFORM_UNIX
    #ifndef strcmpi
      #define strcmpi(x, y) strcasecmp(x, y)
    #endif

    #ifndef stricmp
      #define stricmp(x, y) strcasecmp(x, y)
    #endif

    #ifndef _fstricmp
      #define _fstricmp(x, y) strcasecmp(x, y)
    #endif

    char *strupr(char *);
    char *itoa(int, char *, int);
    char *ltoa(long, char *, int);
    char *ultoa(unsigned long, char *, int);
    char getch(void);
    long filelength(int handle);
  #else
    #error please define for your platform.
  #endif

#if !defined(ANSIESC)
#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())
#else
#define STUB_FUNCTION
#endif

#define far
#define cdecl
#endif

//***************************************************************************
//
//    Screen Constants
//
//***************************************************************************
#define VIEWGLOBAL              0x10000         // globals visable flush to wall
/*
#define VIEWWIDTH               MAXSCREENWIDTH//320*2             // size of view window
#define VIEWHEIGHT              MAXSCREENHEIGHT//200*2
#define MAXSCANLINES            MAXSCREENHEIGHT//200*2             // size of ylookup table
*/
#define CHARWIDTH               2
#define TILEWIDTH               4
#define STATUSLINES             16



//***************************************************************************
//
//    Engine Constants
//
//***************************************************************************

#define ANGLES                            2048 // must be divisible by 4
#define ANGLEQUAD                         (ANGLES/4)
#define FINEANGLES                        2048
#define FINEANGLEQUAD                     (FINEANGLES/4)
#define ANG90                             (FINEANGLES/4)
#define ANG180                            (ANG90*2)
#define ANG270                            (ANG90*3)
#define ANG360                            (ANG90*4)
#define VANG90                            (ANGLES/4)
#define VANG180                           (VANG90*2)
#define VANG270                           (VANG90*3)
#define VANG360                           (VANG90*4)
#define ANGLESDIV8                        (ANGLES/8)
#define MINDIST                           (0x5800l)
#define PIXRADIUS                         512
#define FOCALLENGTH                       (0x5700l)             // in global coordinates
#define MAXTICS                           10

//***************************************************************************
//
//    Map Constants/ macros
//
//***************************************************************************


#define GLOBAL1                           (1l<<16)
#define TILEGLOBAL                        GLOBAL1
#define PIXGLOBAL                         (GLOBAL1/64)
// central resolution globals
extern int iGLOBAL_SCREENWIDTH;
extern int iGLOBAL_SCREENHEIGHT;
extern int iGLOBAL_SCREENBWIDE;
extern int iG_SCREENWIDTH;
extern int iG_SCREENHEIGHT;

extern int iGLOBAL_HEALTH_X;
extern int iGLOBAL_HEALTH_Y;
extern int iGLOBAL_AMMO_X;
extern int iGLOBAL_AMMO_Y;

extern int iGLOBAL_FOCALWIDTH;
extern double dGLOBAL_FPFOCALWIDTH;

#define TILESHIFT                         16l
#define UNSIGNEDSHIFT                     8
#define PLAYERSIZE                        0x5700l                       // player radius
#define MAPSIZE                           128                         // maps are 64*64 max
#define NUMAREAS                          47
#define MAPSPOT(x,y,plane)                (mapplanes[plane][128*(y)+(x)])

#define AREATILE                          107
#define ICONARROWS                        72
#define PUSHABLETILE                      80
#define ELEVATORTILE                      72
#define ALTELEVATORTILE                   106
#define LASTLEVELVALUE                    459

#define AREANUMBER(x,y)                   (MAPSPOT((x),(y),0)-AREATILE)

//***************************************************************************
//
//    Scale Constants
//
//***************************************************************************

#define SFRACBITS 16
#define SFRACUNIT (0x10000)
#define FRACUNIT (0x100)

//***************************************************************************
//
//    Actor Constants
//
//***************************************************************************

#define RUNSPEED                6000
#define MINACTORDIST            0x9000l
#define MAXSTATS                400                           // max number of lamps, bonus, etc

#define MAXWALLTILES            105          // max number of wall tiles
#define MAXACTORS               600
#define NORTH                   0
#define EAST                    1
#define SOUTH                   2
#define WEST                    3

//***************************************************************************
//
//    Input Defines - joystick, keyboard and mouse
//
//***************************************************************************

#define MaxJoys            2
#define MaxKbds            2
#define MaxJoys            2
#define NumCodes           128

// Key definitions

#define  key_None    0
#define  key_Return     0x0d
#define  key_Enter      key_Return
#define  key_Escape     0x1b
#define  key_Space      0x20
#define  key_BackSpace  0x08
#define  key_Tab        0x09
#define  key_Delete     0x7f

#define  ANGLEBITS      16
#define  ANGLEFRACMAX   (FINEANGLES<<ANGLEBITS)


//***************************************************************************
//
//    SWIFT Constants
//
//***************************************************************************

//
// device type codes, returned in deviceType field (SWIFT_StaticData)
//
#define SWIFT_DEV_NONE		0
#define SWIFT_DEV_CYBERMAN	1

//
// Dynamic device data
//
#define SDD_EXTERNAL_POWER_CONNECTED	1
#define SDD_EXTERNAL_POWER_TOO_HIGH	   2

#define AX(r) ((r).x.eax)
#define BX(r) ((r).x.ebx)
#define CX(r) ((r).x.ecx)
#define DX(r) ((r).x.edx)
#define SI(r) ((r).x.esi)
#define DI(r) ((r).x.edi)

//***************************************************************************
//
//    Global Types
//
//***************************************************************************

///////////////////      GLOBAL DATA TYPES ///////////////////////////////


typedef unsigned char           byte;
typedef unsigned short          word;
typedef unsigned long           longword;

void  VL_SetLineWidth (unsigned width);
void  VL_SetVGAPlaneMode ( void );
void  VL_ClearVideo (byte color);
void  VL_SetPalette (byte *palette);
void  VL_GetPalette (byte *palette);
void  VL_ClearBuffer (byte *buffer, byte color);
void  VL_CopyDisplayToHidden (void);
void  VL_CopyPlanarPageToMemory (byte *dest, byte *src);
void  VL_CopyPlanarPage (byte *dest, byte *src);
void  XFlipPage (void);
void  WaitVBL (void);
void  VH_UpdateScreen (void);
void  SetTextMode ( void );
void  TurnOffTextCursor ( void );

typedef int fixed;


//////////////////////////////////////////////////////////////////////////


//////////////////      GLOBAL ENUMERATED TYPES    ///////////////////////

#ifndef __cplusplus
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
#endif




typedef enum {
		  east,
		  northeast,
		  north,
		  northwest,
		  west,
		  southwest,
		  south,
		  southeast,
		  nodir
} dirtype;

typedef enum
 {SPRITE,
  WALL,
  ACTOR,
  DOOR,
  PWALL,
  MWALL
 }
 thingtype;

#define NUMTXBUTTONS    16
enum    {
		  bt_nobutton=-1,
		  bt_attack=0,
		  bt_strafe=1,
		  bt_run=2,
		  bt_use=3,
		  bt_lookup=4,
		  bt_lookdown=5,
		  bt_swapweapon=6,
		  bt_dropweapon=7,
		  bt_horizonup=8,
        bt_horizondown=9,
        bt_pistol=10,
        bt_dualpistol=11,
        bt_mp40=12,
        bt_missileweapon=13,
        bt_autorun=14,
		  bt_recordsound=15,
		  bt_strafeleft=16,
		  bt_straferight=17,
        bt_turnaround=18,
        bt_aimbutton=19,
        di_north=20,
        di_east=21,
        di_south=22,
        di_west=23,
        bt_map=24,
        bt_message=25,
        bt_directmsg=26,
        NUMBUTTONS
};


#if (SHAREWARE == 0)
 #define MAXWEAPONS  13
#else
 #define MAXWEAPONS  9
#endif


typedef enum    { wp_pistol,
						wp_twopistol,
						wp_mp40,
						wp_bazooka,
						wp_heatseeker,
                  wp_drunk,
                  wp_firebomb,
                  wp_firewall,
						wp_godhand,

#if (SHAREWARE == 0)
                  wp_split,
						wp_kes,
						wp_bat,
                  wp_dog
#endif

                } weapontype;



enum    {
						gd_baby,
						gd_easy,
						gd_medium,
						gd_hard
};


typedef enum    {
		  ex_stillplaying,
		  ex_completed,
		  ex_died,
		  ex_warped,
		  ex_resetgame,
		  ex_loadedgame,
		  ex_victorious,
		  ex_abort,
		  ex_demodone,
		  ex_skiplevel,
		  ex_secretlevel,
        ex_secretdone,
        ex_titles,
		  ex_demorecord,
		  ex_demoplayback,

		  ex_bossdied,
		  ex_gameover,
		  ex_battledone
} exit_t;

// Types for cache lumps (for endian converters)
enum    {
	cache_other,
	cache_pic_t,
	cache_lpic_t,
	cache_font_t,
	cache_lbm_t,
	cache_patch_t,
	cache_transpatch_t,
	cache_cfont_t
};

////////////////////////////////////////////////////////////////////////////

/////////////////      GLOBAL STRUCTURE TYPES     //////////////////////////


//=================== SHARED FLAGS =====================================//

#define FL_SHOOTABLE            0x01
#define FL_ACTIVE               0x02
#define FL_VISIBLE              0x08
#define FL_NOFRICTION           0x100 //
#define FL_DYING                0x1000
#define FL_ALTERNATE            0x20000
#define FL_GODSTRUCK            0x80000
#define FL_ABP                  0x400000
#define FL_BLOCK                0x800000
#define FL_HBM                  0x1000000
#define FL_RIDING               0x2000000
#define FL_SOLIDCOLOR           0x4000000
#define FL_SEEN                 0x8000000
#define FL_COLORED              0x10000000
#define FL_FULLLIGHT       0x80000000

//=================== ACTOR FLAGS =====================================//

#define FL_NEVERMARK            4
#define FL_ATTACKMODE           0x10
#define FL_FIRSTATTACK          0x20
#define FL_ISFIRE               0x20
#define FL_AMBUSH               0x40
#define FL_NONMARK              0x80
#define FL_DONE                 0x200 // used by push column
#define FL_DODGE                0x400 // "
#define FL_STUCK                0x800 //
#define FL_HASAUTO              0x2000
#define FL_FALLINGOBJECT        0x4000
#define FL_KEYACTOR             0x8000
#define FL_HEAD                 0x40000
#define FL_TARGET               0x200000
#define FL_FLIPPED              0x40000000
#define FL_CRAZY                0x40000000


//================== PLAYER FLAGS ======================================//

#define FL_BPV                  0x10
#define FL_SHROOMS              0x20
#define FL_AV                   0x40
#define FL_ELASTO               0x200
#define FL_FLEET                0x400
#define FL_PUSHED               0x2000
#define FL_PAIN                 0x4000
#define FL_GODMODE              0x8000
#define FL_DOGMODE              0x10000
#define FL_GASMASK              0x100000
#define FL_DESIGNATED           0x20000000
#define FL_DIDTAG               0x40000000

//================== SPRITE FLAGS ======================================//

#define NOTHING                 -1
#define FL_BONUS                0x04
#define FL_LIGHT                0x20
#define FL_CHANGES              0x40
#define FL_BACKWARDS            0x100
#define FL_BANDF                0x200
#define FL_HEAT                 0x400
#define FL_LIGHTOFF             0x800
#define FL_ACTOR                0x1000
#define FL_ROTATING             0x2000
#define FL_RESPAWN              0x4000
#define FL_WOODEN               0x8000
#define FL_METALLIC             0x10000
#define FL_EARTHEN              0x20000
#define FL_LIGHTON              0x40000
#define FL_FADING               0x80000
#define FL_DEADBODY             0x100000
#define FL_WEAPON               0x200000
#define FL_TRANSLUCENT          0x2000000
#define FL_HEIGHTFLIPPABLE      0x40000000

#endif
