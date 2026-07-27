/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file IS NOT A PART OF Ken Silverman's original release
 */

#ifndef _INCLUDE_DUKE3D_H_
#define _INCLUDE_DUKE3D_H_

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "esp_attr.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "platform.h"
#include "build.h"

// Game specific defines
#define MOVEFIFOSIZ 256
#define MAXUSERQUOTES 4
#define NUMOFFIRSTTIMEACTIVE 192
#define MAX_WEAPONS 12
#define MAX_INVENTORY 10
#define MAXANIMATES 64
#define MAXSOUNDS 256
#define MAXVOLUMES 7
#define MAXLEVELS 32
#define NUM_SOUNDS 1024
#define MAXSCRIPTSIZE 20460
#define MAXANIMWALLS 512
#ifdef CONFIG_IDF_TARGET_ESP32
#define MAXCACHEOBJECTS 4096
#else
#define MAXCACHEOBJECTS 20000
#endif
#define MAX_KNOWN_GRP 10
#define MAXINTERPOLATIONS 2048
#define MAXCYCLERS 256
#define RECSYNCBUFSIZ 2520

typedef struct { uint32_t crc32; const char *name; uint32_t size; } crc32_t;

typedef struct
{
    short i;
    int voice;
} SOUNDOWNER;

typedef struct
{
    uint8_t  *ptr;
    uint8_t  lock;
    int  length, num;
} SAMPLE;

struct animwalltype
{
        short wallnum;
        int32_t tag;
};

struct weaponhit
{
    uint8_t  cgg;
    short picnum,ang,extra,owner,movflag;
    short tempang,actorstayput,dispicnum;
    short timetosleep;
    int32_t floorz,ceilingz,lastvx,lastvy,bposx,bposy,bposz;
    int32_t temp_data[6];
};

struct player_orig
{
    int32_t ox,oy,oz;
    short oa,os;
};

struct player_struct
{
    int32_t zoom,exitx,exity,loogiex[64],loogiey[64],numloogs,loogcnt;
    int32_t posx, posy, posz, horiz, ohoriz, ohorizoff, invdisptime;
    int32_t bobposx,bobposy,oposx,oposy,oposz,pyoff,opyoff;
    int32_t posxv,posyv,poszv,last_p_ang,last_pissed_time,truefz,truecz;
    int32_t player_par,visibility;
    int32_t bobcounter,weapon_sway;
    int32_t pals_time,randomflamex,crack_time;

    int32 aim_mode;

    short ang,oang,angvel,cursectnum,look_ang,last_extra,subweapon;
    short ammo_amount[MAX_WEAPONS],wackedbyactor,frag,fraggedself;

    short curr_weapon, last_weapon, tipincs, horizoff, wantweaponfire;
    short holoduke_amount,newowner,hurt_delay,hbomb_hold_delay;
    short jumping_counter,airleft,knee_incs,access_incs;
    short fta,ftq,access_wallnum,access_spritenum;
    short kickback_pic,got_access,weapon_ang,firstaid_amount;
    short somethingonplayer,on_crane,i,one_parallax_sectnum;
    short over_shoulder_on,random_club_frame,fist_incs;
    short one_eighty_count,cheat_phase;
    short dummyplayersprite,extra_extra8,quick_kick;
    short heat_amount,actorsqu,timebeforeexit,customexitsound;

    short weaprecs[16],weapreccnt;
	uint32_t interface_toggle_flag;

    short rotscrnang,dead_flag,show_empty_weapon;
    short scuba_amount,jetpack_amount,steroids_amount,shield_amount;
    short holoduke_on,pycount,weapon_pos,frag_ps;
    short transporter_hold,last_full_weapon,footprintshade,boot_amount;

    int scream_voice;

    uint8_t  gm,on_warping_sector,footprintcount;
    uint8_t  hbomb_on,jumping_toggle,rapid_fire_hold,on_ground;
    uint8_t  name[32],inven_icon,buttonpalette;

    uint8_t  jetpack_on,spritebridge,lastrandomspot;
    uint8_t  scuba_on,footprintpal,heat_on;

    uint8_t   holster_weapon,falling_counter;
    uint8_t   gotweapon[MAX_WEAPONS],refresh_inventory,*palette;

    uint8_t  toggle_key_flag,knuckle_incs; // ,select_dir;
    uint8_t  walking_snd_toggle, palookup, hard_landing;
    uint8_t  max_secret_rooms,secret_rooms,/*fire_flag,*/pals[3];
    uint8_t  max_actors_killed,actors_killed,return_to_center;

	int32 auto_aim;
	int32 weaponautoswitch;

	uint8_t  fakeplayer;
};

struct user_defs
{
    uint8_t  god,warp_on,cashman,eog,showallmap;
    uint8_t  show_help,scrollmode,clipping;
    char  user_name[MAXPLAYERS][32];
    char  ridecule[10][40];
    char  savegame[10][22];
    char  pwlockout[128],rtsname[128];
    uint8_t  overhead_on,last_overhead;

    short pause_on,from_bonus;
    short camerasprite,last_camsprite;
    short last_level,secretlevel;

    int32_t const_visibility,uw_framerate;
    int32_t camera_time,folfvel,folavel,folx,foly,fola;
    int32_t reccnt;

    int32 entered_name,screen_tilting,shadows,fta_on,executions,auto_run;
    int32 coords,tickrate,m_coop,coop,screen_size,extended_screen_size,lockout,crosshair,showweapons;
    int32 mywchoice[MAX_WEAPONS],wchoice[MAXPLAYERS][MAX_WEAPONS],playerai;

    int32 respawn_monsters,respawn_items,respawn_inventory,recstat,monsters_off,brightness;
    int32 m_respawn_items,m_respawn_monsters,m_respawn_inventory,m_recstat,m_monsters_off,detail;
	int32 m_ffire,ffire,m_player_skill,m_level_number,m_volume_number,multimode,multimode_bot;
    int32 player_skill,level_number,volume_number,m_marker,marker,mouseflip;

	int32 showcinematics, hideweapon;
	int32 auto_aim, gitdat_mdk;
	int32 weaponautoswitch;

	uint8_t  playing_demo_rev;

	uint32_t groupefil_crc32[MAXPLAYERS][4];
	uint16_t conSize[MAXPLAYERS];

	uint8_t  rev[MAXPLAYERS][10];
	uint32_t mapCRC_array[MAXPLAYERS];
	uint32_t exeCRC[MAXPLAYERS];
	uint32_t conCRC[MAXPLAYERS];
};

typedef struct
{
        short frag[MAXPLAYERS], got_access, last_extra, shield_amount, curr_weapon;
        short ammo_amount[MAX_WEAPONS], holoduke_on;
        uint8_t  gotweapon[MAX_WEAPONS], inven_icon, jetpack_on, heat_on;
        short firstaid_amount, steroids_amount, holoduke_amount, jetpack_amount;
        short heat_amount, scuba_amount, boot_amount;
        short last_weapon, weapon_pos, kickback_pic;

} STATUSBARTYPE;

extern EXTERN_ATTR input inputfifo[MOVEFIFOSIZ][MAXPLAYERS], dukeSyncArray[MAXPLAYERS];
extern EXTERN_ATTR input recsync[RECSYNCBUFSIZ];
extern EXTERN_ATTR struct animwalltype animwall[MAXANIMWALLS];
extern EXTERN_ATTR uint8_t  tempbuf[2048];
extern EXTERN_ATTR uint8_t packbuf[576];
extern int32_t gc,max_player_health,max_armour_amount,max_ammo_amount[MAX_WEAPONS];
extern int32_t impact_damage,respawnactortime,respawnitemtime;
extern EXTERN_ATTR short spriteq[1024];
extern short spriteqloc,spriteqamount;
extern int g_iTickRate, g_iTicksPerFrame;
extern int32_t SoundToggle, MusicToggle, FXVolume, MusicVolume, FXDevice;
extern EXTERN_ATTR int32_t soundsiz[NUM_SOUNDS];
extern uint8_t screencapt;
extern int32_t MusicDevice;
extern int32_t NumVoices;
extern int32_t VoiceToggle,AmbienceToggle, OpponentSoundToggle;
extern int32_t mouseSensitivity_X, mouseSensitivity_Y;
extern int32_t ReverseStereo;
extern int32_t totalmemory, show_shareware;
extern short lastsavedpos;
extern uint8_t everyothertime, restorepalette, eightytwofifty;
extern short probey, globalskillsound;
extern int32_t cloudtotalclock, numclouds, clouds[128], cloudx[128], cloudy[128];
extern int32_t syncstat, bufferjitter, mymaxlag, otherminlag;
extern uint8_t movesperpacket;
extern EXTERN_ATTR uint8_t syncval[MAXPLAYERS][MOVEFIFOSIZ];
extern int32_t syncvalhead[MAXPLAYERS], syncvaltail, syncvaltottail, myminlag[MAXPLAYERS];
extern int32_t movefifosendplc, fakemovefifoplc;
extern int32_t myx, omyx, myxvel, myy, omyy, myyvel, myz, omyz, myzvel;
extern short myhoriz, omyhoriz, myhorizoff, omyhorizoff, mycursectnum;
extern short myang, omyang;
extern short myjumpingcounter;
extern uint8_t myjumpingtoggle, myonground, myhardlanding, myreturntocenter;
extern EXTERN_ATTR int32_t myxbak[MOVEFIFOSIZ], myybak[MOVEFIFOSIZ], myzbak[MOVEFIFOSIZ];
extern EXTERN_ATTR int32_t myhorizbak[MOVEFIFOSIZ];
extern EXTERN_ATTR short myangbak[MOVEFIFOSIZ];
extern char typebuf[41], recbuf[80];
extern uint8_t typebuflen;
extern short user_quote_time[4];
extern uint8_t grpVersion, conVersion, nHostForceDisableAutoaim;
extern int32_t myaimmode;
extern uint8_t pub, pus;
extern char boardfilename[128];
extern uint8_t waterpal[768], slimepal[768], titlepal[768], drealms[768], endingpal[768];
extern char level_names[44][33], volume_names[4][33], skill_names[5][33], level_file_names[44][128];
extern int32_t partime[44], designertime[44];
extern short soundps[NUM_SOUNDS], soundpe[NUM_SOUNDS], soundvo[NUM_SOUNDS];
extern uint8_t soundm[NUM_SOUNDS], soundpr[NUM_SOUNDS];
extern int32_t lasermode, camerashitable, numfreezebounces, freezerhurtowner, dukefriction;
extern int32_t rpgblastradius, pipebombblastradius, shrinkerblastradius, tripbombblastradius, morterblastradius, bouncemineblastradius, seenineblastradius;
extern short weaponsandammosprites[15];
extern int32_t fricxv, fricyv;
extern uint8_t earthquaketime;
extern short numcyclers;
extern short cyclers[MAXCYCLERS][6];
extern short numanimwalls;
extern int32_t numinterpolations, startofdynamicinterpolations;
extern EXTERN_ATTR int32_t oldipos[MAXINTERPOLATIONS], *curipos[MAXINTERPOLATIONS], bakipos[MAXINTERPOLATIONS];
extern int8_t multiwho, multiflag, multiwhat, multipos;
extern uint8_t *pic;
extern int32_t totalclocklock;
extern uint8_t lumplockbyte[11];
extern int32_t BYTEVERSION, BYTEVERSION_27, BYTEVERSION_28, BYTEVERSION_29, BYTEVERSION_116, BYTEVERSION_117, BYTEVERSION_118;
extern uint8_t gamequit;
extern int32_t MouseAiming;
extern uint8_t numplayersprites;
extern uint8_t networkmode;
extern char betaname[128];
extern char *mymembuf;
extern char duke_myname[32];
extern crc32_t crc32lookup[];
extern int32_t myxvel, myyvel, myzvel;
extern int32_t cameradist, cameraclock;
extern EXTERN_ATTR struct player_struct ps[MAXPLAYERS];
extern struct player_orig po[MAXPLAYERS];
extern struct user_defs ud;
extern STATUSBARTYPE sbar;
extern short frags[MAXPLAYERS][MAXPLAYERS];
extern short int global_random;
extern int32_t scaredfallz;
extern char  buf[80];
extern EXTERN_ATTR char  fta_quotes[NUMOFFIRSTTIMEACTIVE][64];
extern uint8_t  scantoasc[128],ready2send;
extern uint8_t  scantoascwithshift[128];
extern EXTERN_ATTR SAMPLE Sound[ NUM_SOUNDS ];
extern EXTERN_ATTR SOUNDOWNER SoundOwner[NUM_SOUNDS][4];
extern uint8_t  playerreadyflag[MAXPLAYERS],playerquitflag[MAXPLAYERS];
extern EXTERN_ATTR char  sounds[NUM_SOUNDS][14];
extern EXTERN_ATTR int32_t script[MAXSCRIPTSIZE];
extern int32_t *scriptptr,*insptr,*labelcode,labelcnt;
extern char  *label,*textptr,error,warning;
extern uint8_t killit_flag;
extern EXTERN_ATTR int32_t *actorscrptr[MAXTILES];
extern int32_t *parsing_actor;
extern EXTERN_ATTR uint8_t  actortype[MAXTILES];
extern uint8_t  *music_pointer;
extern uint8_t  ipath[80],opath[80];
extern EXTERN_ATTR char  music_fn[4][11][13];
extern uint8_t music_select;
extern char  env_music_fn[4][13];
extern short camsprite;
extern EXTERN_ATTR struct weaponhit hittype[MAXSPRITES];
extern short numplayers, myconnectindex;
extern short connecthead, connectpoint2[MAXPLAYERS];
extern short screenpeek;
extern int current_menu;
extern int32_t tempwallptr,animatecnt;
extern int32_t lockclock;
extern uint8_t  display_mirror,rtsplaying;
extern int32_t movefifoend[MAXPLAYERS];
extern int32_t ototalclock;
extern EXTERN_ATTR int32_t *animateptr[MAXANIMATES], animategoal[MAXANIMATES];
extern EXTERN_ATTR int32_t animatevel[MAXANIMATES];
extern short neartagsector, neartagwall, neartagsprite;
extern int32_t neartaghitdist;
extern short animatesect[MAXANIMATES];
extern int32_t movefifoplc, vel,svel,angvel,horiz;
extern short mirrorwall[64], mirrorsector[64], mirrorcnt;
extern EXTERN_ATTR int32_t msx[2048],msy[2048];
extern int32_t avgfvel, avgsvel, avgavel, avghorz, avgbits;
extern input loc;

int getGRPcrc32(int grpID);

#define PLUTOPAK 1
#define VOLUMEONE (getGRPcrc32(0)==CRC_BASE_GRP_SHAREWARE_13)
#define VOLUMEALL (getGRPcrc32(0)==CRC_BASE_GRP_FULL_13 || (conVersion == 13 && getGRPcrc32(0)!=CRC_BASE_GRP_SHAREWARE_13 && getGRPcrc32(0)!=CRC_BASE_GRP_PLUTONIUM_14 && getGRPcrc32(0)!=CRC_BASE_GRP_ATOMIC_15))

#include "names.h"
#include "soundefs.h"
#include "function.h"
#include "fixedPoint_math.h"
#include "tiles.h"
#include "gamedefs.h"
#include "keyboard.h"
#include "control.h"
#include "config.h"
#include "engine.h"
#include "funct.h"
#include "rts.h"
#include "_rts.h"
#include "file_lib.h"
#include "util_lib.h"
#include "sounds.h"

#define rnd(X) (((krand())>>8)>=(255-(X)))

char* getGameDir(void);

#define PN  sprite[i].picnum
#define SX  sprite[i].x
#define SY  sprite[i].y
#define SZ  sprite[i].z
#define SS  sprite[i].shade
#define SA  sprite[i].ang
#define SV  sprite[i].xvel
#define ZV  sprite[i].zvel
#define RX  sprite[i].xrepeat
#define RY  sprite[i].yrepeat
#define OW  sprite[i].owner
#define CS  sprite[i].cstat
#define SH  sprite[i].extra
#define CX  sprite[i].xoffset
#define CY  sprite[i].yoffset
#define CD  sprite[i].clipdist
#define PL  sprite[i].pal
#define SP  sprite[i].yvel
#define SLT sprite[i].lotag
#define SHT sprite[i].hitag
#define SECT sprite[i].sectnum

#define T1  hittype[i].temp_data[0]
#define T2  hittype[i].temp_data[1]
#define T3  hittype[i].temp_data[2]
#define T4  hittype[i].temp_data[3]
#define T5  hittype[i].temp_data[4]
#define T6  hittype[i].temp_data[5]
#define SS_STAT  sprite[i].statnum

#define IFHIT j=ifhitbyweapon(i);if(j >= 0)
#define IFMOVING if(ssp(i,CLIPMASK0))
#define IFHITSECT j=ifhitsectors(s->sectnum);if(j >= 0)

#define RANDOMSCRAP spawn(i,SCRAP1+((krand()>>2)%6))

#define IFWITHIN(min,max) if( PN >= min && PN <= max )

#define TICRATE 120
#define TICSPERFRAME (TICRATE/g_iTicksPerFrame)

#define SLEEPTIME 256
#define MAXSLEEPDIST (1024L*10)

#define FOURSLEIGHT (4L<<8)
#define PHEIGHT (38L<<8)

#define TRAND (krand())

#define ALT_IS_PRESSED KB_KeyPressed(sc_LeftAlt)
#define SHIFTS_IS_PRESSED (KB_KeyPressed(sc_LeftShift)|KB_KeyPressed(sc_RightShift))

#define MODE_GAME 4
#define MODE_MENU 1
#define MODE_RESTART 32
#define MODE_EOL 8
#define MODE_DEMO 2
#define MODE_TYPE 16
#define MODE_SENDTOWHOM 64
#define MODE_END 128

#define NUMPAGES 1

#define DUKEITOUTINDC_GRP 1
#define SHAREWARE_GRP13   2
#define ATOMIC_GRP14_15   3
#define REGULAR_GRP13D    4
#define UNKNOWN_GRP       0

#define CHOCOLATE_DUKE_REV_X 1
#define CHOCOLATE_DUKE_REV_DOT_Y 5

#define SCREENSHOTPATH "screenshots"
int screencapture(char *filename, uint8_t inverseit);

#define CRC_BASE_GRP_SHAREWARE_13 0x124011EE
#define CRC_BASE_GRP_FULL_13      0xFD340065
#define CRC_BASE_GRP_PLUTONIUM_14 0x02A945E8
#define CRC_BASE_GRP_ATOMIC_15    0xF51ADCD

#define AFLAMABLE(x) (x==FIRE || x==FIRE2 || x==BURNING || x==BURNING2)
#define KILLIT(i) { deletesprite(i); }

void allocache(uint8_t **ptr, int32_t size, uint8_t *lock);
void clearsoundlocks(void);
void testcallback(uint32_t num);

#define AUTO_AIM_ANGLE 48

// extern uint8_t  gotz;
extern uint8_t  inspace(short sectnum);

#ifdef __cplusplus
}
#endif

#endif  // include-once header.
