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
 // make sure word alignment is OFF!

#include "rt_def.h"
#include <rg_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "rt_sound.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef DOS
#include <io.h>
#include <conio.h>
#include <dos.h>
#else
#include <errno.h>
#endif

#include "states.h"
#include "watcom.h"
#include "rt_ted.h"
#include "_rt_ted.h"
#include "w_wad.h"
#include "z_zone.h"
#include "rt_util.h"
#include "lumpy.h"
#include "rt_vid.h"
#include "rt_actor.h"
#include "rt_stat.h"
#include "rt_menu.h"
#include "rt_draw.h"
#include "rt_com.h"
#include "rt_main.h"
#include "rt_door.h"
#include "rt_playr.h"
#include "rt_view.h"
#include "rt_str.h"
#include "isr.h"
#include "rt_floor.h"
#include "rt_game.h"
#include "rt_rand.h"
#include "rt_cfg.h"
#include "develop.h"
#include "modexlib.h"
#include "engine.h"
#include "rt_debug.h"
#include "rt_scale.h"
#include "rt_net.h"
//MED
#include "memcheck.h"




//========================================
// GLOBAL VARIABLES
//========================================

extern boolean  UseBaseMarker;

teamtype TEAM[MAXPLAYERS];
#ifndef RETRO_GO
int numareatiles[NUMAREAS+1];
#endif
int shapestart,shapestop;
_2dvec SPAWNLOC[MAXSPAWNLOCATIONS],FIRST,SECOND;
int NUMSPAWNLOCATIONS,numteams=0;
wall_t    walls[MAXWALLTILES];
str_clock Clocks[MAXCLOCKS];
int numclocks;
#ifndef RETRO_GO
int LightsInArea[NUMAREAS+1];
#endif

int maxheight;
int nominalheight;
int elevatorstart;
int gunsstart;
int fog;
int lightsource;
int SNAKELEVEL;
int whichpath;

#ifndef RETRO_GO
word *mapplanes[3];
#endif
int  mapwidth;
int  mapheight;
int  lastlevelloaded=-1;

boolean insetupgame;
boolean ISRTL = false;

unsigned MapSpecials = 0;

char LevelName[80];

//========================================
// LOCAL VARIABLES
//========================================

static cachetype * cachelist;
static word cacheindex;
static boolean CachingStarted=false;
static char * ROTTMAPS = STANDARDGAMELEVELS;
static char * BATTMAPS = STANDARDBATTLELEVELS;

static char NormalWeaponTiles[ 10 ] =
   {
   46, 48, 49, 50, 51, 52, 53, 54, 55, 56
   };
static char SharewareWeaponTiles[ 7 ] =
   {
   48, 49, 50, 51, 52, 53, 54
   };

static char CacheStrings[MAXSILLYSTRINGS][80]=
{
   {"Ready yourself\nfor destruction!\0\0"},
   {"Here comes the enemy!\0\0"},
   {"Hope you wrote\nyour will!\0\0"},
   {"Did you bring your\nextra bullets?\0\0"},
   {"Try not to bleed\non the rug.\0\0"},
   {"Let's see...bandages,\ntape, splints,...\0\0"},
   {"Couldn't we just\ntalk this over?\0\0"},
   {"Cache as cache can...\0\0"},
   {"You are smart.\nMake us strong.\0\0"},
	{"Bleh!\0\0"},
   {"I am as far\nabove you...\0\0"},
   {"Just keep thinkin':\nBut it's loadin' COOL\nstuff...\0\0"},
   {"Guess which line\nwill win!\0\0"},
   {"Oh, no. Not again.\0\0"},
   {"Wait! I'm not ready!\nToo late.\0\0"},
	{"Hope this doesn't\ncrash.\0\0"},
	{"Have a sandwich.\0\0"},
   {"Smoke 'em if\nya got 'em...and\nif ya like cancer.\0\0"},
   {"Ummmmm...\0\0"},
   {"Bang! Bang! Bang!\nFreeze!\0\0"},
   {"You have the right\nto...DIE.\0\0"},
   {"Insert funny phrase\nhere.\0\0"},
   {"Blood, bullets,\nnicely decorated\nhallways.\0\0"},
   {"You are to be killed,\nnot hurt.\0\0"},
   {"It's time for you to\ngo down the stairs!\0\0"},
   {"This game, like,\nrules and stuff.\0\0"},
   {"We get money for this!\nHa ha ha ha!\0\0"},
   {"Let's not start any\nreligious wars...\0\0"},
   {"I don't wanna start\nno ting...\0\0"},
   {"Ah, another sacrifice!\0\0"},
   {"If you were dead,\nyou'd be the\nsame thing.\0\0"},
   {"This Game isn't\nhuman; it can't\nbe reasoned with!\0\0"}
};

void SetupGameLevel (void);
void ScanInfoPlane(void);

void DrawPreCache( void );
void InitializePlayerstates(void);
void SetupSnakePath(void);
void SetupRandomActors(void);
void SetupActors(void);
void SetupStatics(void);
void LoftSprites( void );
int GetLumpForTile(int tile);

//========================================

/*
======================
=
= SortPreCache
= Sort the Precache for cachelevel precedence using a HEAPSORT
=
======================
*/
#define SGN(x)          ((x>0) ? (1) : ((x==0) ? (0) : (-1)))

/*--------------------------------------------------------------------------*/
int CompareTags(s1p,s2p) cachetype *s1p,*s2p;
{
// Sort according to lump
   if (DoPanicMapping()==true)
      return SGN(s1p->cachelevel-s2p->cachelevel);
// Sort according to cachelevel
   else
      return SGN(s1p->lump-s2p->lump);
}

void SwitchCacheEntries(s1p,s2p) cachetype *s1p,*s2p;
{
   cachetype temp;

	temp=*s1p;
   *s1p=*s2p;
   *s2p=temp;
}



void SortPreCache( void )
{
   hsort((char *)cachelist,cacheindex,sizeof(cachetype),&CompareTags,&SwitchCacheEntries);
}

//========================================


/*
======================
=
= SetupPreCache
= Setup the cache for pre-cacheing
=
======================
*/
void SetupPreCache( void )
{

   CachingStarted=true;
   cacheindex=0;
   cachelist=(cachetype *)SafeMalloc(MAXPRECACHE*(sizeof(cachetype)));
   DrawPreCache();
}


/*
======================
=
= ShutdownPreCache
= Setup the cache for pre-cacheing
=
======================
*/
void ShutdownPreCache( void )
{

   CachingStarted=false;
   SafeFree((byte *)cachelist);
}


/*
======================
=
= PreCacheLump
= precache the lump and check to see if it is already tagged
=
======================
*/
void PreCacheLump( int lump, int level, int type ) // added type
{
   int i;

   if (CachingStarted==false)
      return;
   if (!W_LumpLength(lump))
      {
#if (PRECACHETEST == 1)
      SoftError("Tried to precache a label, lump = %ld tag=%ld maskednum=%ld\n",lump, level, maskednum);
#endif
      return;
      }
   for (i=1;i<cacheindex;i++)
      if (cachelist[i].lump==lump)
         return;
   cachelist[cacheindex].lump=lump;
   cachelist[cacheindex].cachelevel=level;
   cachelist[cacheindex++].type=type;
   if (cacheindex==MAXPRECACHE)
      Error("MaxPreCache reached\n");
}



/*
======================
=
= PreCacheGroup
= precache the lump and check to see if it is already tagged
=
======================
*/
void PreCacheGroup( int start, int end, int type ) // added type
{
	int i;
   int j;
   int k;
   int found;

   if (CachingStarted==false)
      return;
   k=cacheindex;
   for (j=start;j<=end;j++)
      {
      if (!W_LumpLength(j))
         {
#if (PRECACHETEST == 1)
         SoftError("Tried to precache a label, lump = %ld\n",j);
#endif
         continue;
         }
      found=0;
      for (i=1;i<k;i++)
         if (cachelist[i].lump==j)
            {
            found=1;
            break;
				}
      if (found==0)
         {
         cachelist[cacheindex].lump=j;
         cachelist[cacheindex].cachelevel=PU_CACHEACTORS;
         cachelist[cacheindex++].type=type;

         if (cacheindex==MAXPRECACHE)
            Error("MaxPreCache reached\n");
			}
      }

}


/*
======================
=
= PreCachePlayers
= precache the lump and check to see if it is already tagged
=
======================
*/
void PreCachePlayers(void )
{
	int start;
	int end;
	int i;
	playertype*pstate;

	for(i=0;i<numplayers;i++)
	 {if (i!=consoleplayer) // don't cache consoleplayer
		 {pstate = &PLAYERSTATE[i];
        start=W_GetNumForName("CASSHO11")+(pstate->player*REMOTEOFFSET);
        end  =W_GetNumForName("CASWDEAD")+(pstate->player*REMOTEOFFSET);
		  PreCacheGroup(start,end,cache_patch_t);
		 }
	 }
}




void PreCachePlayerSound(void)
   {
   switch (locplayerstate->player)
      {
      case 0:
         SD_PreCacheSound(SD_PLAYERTCSND);

         break;
      case 1:
         SD_PreCacheSound(SD_PLAYERTBSND);

         break;
      case 2:
         SD_PreCacheSound(SD_PLAYERDWSND);

         break;
      case 3:
         SD_PreCacheSound(SD_PLAYERLNSND);

         break;
      case 4:
         SD_PreCacheSound(SD_PLAYERIPFSND);
         break;
      }
   }


#define IS_ALTERNATE_ACTOR(ob)                                 \
        ((ob->shapeoffset - deathshapeoffset[ob->obclass]) > 0)\

/*
======================
=
= PreCacheActor
= precache the lump and check to see if it is already tagged
=
======================
*/
void PreCacheActor( int actor, int which )
{
	int start;
	int end;

	switch (actor)
		{
		case lowguardobj:
         if (IS_ALTERNATE_ACTOR(new))
				{
				 start = SD_LOWGUARD2SEESND;
				 end = SD_LOWGUARD2SEE3SND;
				 SD_PreCacheSoundGroup(start,end);

             start = SD_LOWGUARD2DIESND;
             end = SD_LOWGUARD2DIESND;
				 SD_PreCacheSoundGroup(start,end);

				 start = SD_LOWGUARDFIRESND;
				 end = SD_SNEAKYSPRINGFSND;
				 SD_PreCacheSoundGroup(start,end);

				 start=W_GetNumForName("MARSHOO1");
				 end  =W_GetNumForName("MNGRISE4");
				 //end  =W_GetNumForName("MARUSE28");
				}

			else
				{start = SD_LOWGUARD1SEESND;
				 end = SD_LOWGUARD1SEE3SND;
				 SD_PreCacheSoundGroup(start,end);

             start = SD_LOWGUARD1DIESND;
             end = SD_LOWGUARD1DIESND;
				 SD_PreCacheSoundGroup(start,end);

				 start = SD_LOWGUARDFIRESND;
				 end = SD_SNEAKYSPRINGFSND;
				 SD_PreCacheSoundGroup(start,end);

				 start=W_GetNumForName("LWGSHOO1");
				 end = W_GetNumForName("SNGRISE4");
				 //end  =W_GetNumForName("LOWUSE28");
				}

			break;
		case highguardobj:

			start = SD_HIGHGUARD1SEESND;
			end = SD_HIGHGUARDDIESND;
			SD_PreCacheSoundGroup(start,end);

         if (IS_ALTERNATE_ACTOR(new))
				{
				start=W_GetNumForName("HIGSHOO1");
				end  =W_GetNumForName("HIGWDEAD");
				//end  =W_GetNumForName("HIHUSE28");
				}
			else
				{
				start=W_GetNumForName("HG2SHOO1");
				end  =W_GetNumForName("HG2WDEAD");
				//end  =W_GetNumForName("H2HUSE28");
				}
			break;

		case overpatrolobj:

			start=W_GetNumForName("OBBOLO1");
         end  =W_GetNumForName("OBBOLO4");
			PreCacheGroup(start,end,cache_patch_t);
			start=W_GetNumForName("NET1");
			end  =W_GetNumForName("NET4");
			PreCacheGroup(start,end,cache_patch_t);

			start = SD_OVERP1SEESND;
			end = SD_OVERPDIESND;
			SD_PreCacheSoundGroup(start,end);
			SD_PreCacheSoundGroup(SD_NETWIGGLESND,SD_NETFALLSND);

         if (IS_ALTERNATE_ACTOR(new))
				{
            start=W_GetNumForName("PATSHOO1");
            end  =W_GetNumForName("PATDEAD");

				//end  =W_GetNumForName("OBPUSE28");
            }

			else
				{
            start=W_GetNumForName("OBPSHOO1");
				end  =W_GetNumForName("OBPDEAD");

				//end  =W_GetNumForName("PATUSE28");
            }

			break;
		case strikeguardobj:


			start = SD_STRIKE1SEESND;
			end = SD_STRIKEDIESND;
			SD_PreCacheSoundGroup(start,end);

         if (IS_ALTERNATE_ACTOR(new))
            {
				start=W_GetNumForName("XYGSHOO1");
				end  =W_GetNumForName("XYLROLL6");
				//end  =W_GetNumForName("XYUSE28");
				}

			else
				{
				start=W_GetNumForName("ANGSHOO1");
				end  =W_GetNumForName("ANLROLL6");
				//end  =W_GetNumForName("ANUSE28");
				}

			break;

		case blitzguardobj:

			start = SD_BLITZ1SEESND;
			end = SD_BLITZDIESND;
			SD_PreCacheSoundGroup(start,end);

         if (IS_ALTERNATE_ACTOR(new))
				{
				start=W_GetNumForName("WIGSHOO1");
				end  =W_GetNumForName("WIHUSE28");
				}

			else
				{
				start=W_GetNumForName("LIGSHOO1");
            end  =W_GetNumForName("LIPEAD11");
				}

			break;

		case triadenforcerobj:

			start = SD_ENFORCERSEESND;
			end = SD_ENFORCERDIESND;
			SD_PreCacheSoundGroup(start,end);

			start=W_GetNumForName("TEGREN1");
			end  =W_GetNumForName("TGRENF6");
			PreCacheGroup(start,end,cache_patch_t);
			start=W_GetNumForName("TRISHOO1");
			end  =W_GetNumForName("TRIWDEAD");
			//end  =W_GetNumForName("TRIUSE28");
			break;
		case deathmonkobj:


			start = SD_MONKSEESND;
			end = SD_MONKDIESND;
			SD_PreCacheSoundGroup(start,end);

			start=W_GetNumForName("MONKDR1");
			end  =W_GetNumForName("MONDEAD");
			//end  =W_GetNumForName("MONUSE28");
			break;


		case dfiremonkobj:

			start = SD_FIREMONKSEESND;
			end = SD_FIREMONKDIESND;
			SD_PreCacheSoundGroup(start,end);

         start = W_GetNumForName("MONFIRE1");
         end = W_GetNumForName("MONFIRE4");
         PreCacheGroup(start,end,cache_patch_t);


         if (IS_ALTERNATE_ACTOR(new))
				{
            start=W_GetNumForName("MRKKSH1");
				end  =W_GetNumForName("MRKDEAD7");
				}

			else
				{
            start=W_GetNumForName("ALLKSH1");
				end  =W_GetNumForName("ALLDEAD7");
				}

			break;

		case roboguardobj:

			start = SD_ROBOTSEESND;
			end = SD_ROBOTDIESND;
			SD_PreCacheSoundGroup(start,end);
			start=W_GetNumForName("ROBOGRD1");
         end  =W_GetNumForName("ROBGRD16");
			break;

		case b_darianobj:

         PreCachePlayerSound();

         start = SD_DARIANSEESND;
			end = SD_DARIANSAY3;
			SD_PreCacheSoundGroup(start,end);


			start=W_GetNumForName("DARSHOO1");
			end  =W_GetNumForName("DARUSE28");
			break;


		case b_heinrichobj:

         PreCachePlayerSound();

			start = SD_KRISTSEESND;
			end = SD_KRISTSAY3;
			SD_PreCacheSoundGroup(start,end);

			start=W_GetNumForName("MINE1");
			end  =W_GetNumForName("MINE4");
			PreCacheGroup(start,end,cache_patch_t);
			start=W_GetNumForName("HSIT1");
			end  =W_GetNumForName("HDOPE8");
			break;

		case b_darkmonkobj:

			start = SD_DARKMONKSEESND;
			end = SD_DARKMONKSAY3;
			SD_PreCacheSoundGroup(start,end);

			start=W_GetNumForName("LIGNING1");
			end  =W_GetNumForName("FSPARK4");
			PreCacheGroup(start,end,cache_patch_t);
			start=W_GetNumForName("TOMS1");
			end  =W_GetNumForName("TOHRH8");
			break;

		case b_darksnakeobj:

         PreCachePlayerSound();

			start = SD_SNAKESEESND;
			end = SD_SNAKESAY3;
			SD_PreCacheSoundGroup(start,end);

			start=W_GetNumForName("TOMRH1");
			end  =W_GetNumForName("TOHRH8");
		case b_robobossobj:

         PreCachePlayerSound();

			start = SD_NMESEESND;
         end = SD_NMESEESND;
			SD_PreCacheSoundGroup(start,end);

			start=W_GetNumForName("RHEAD101");
			end  =W_GetNumForName("NMESAUC4");
			break;
		case patrolgunobj:

			start = SD_EMPLACEMENTSEESND;
			end = SD_BIGEMPLACEFIRESND;
			SD_PreCacheSoundGroup(start,end);


			start=W_GetNumForName("GUNEMP1");
			end  =W_GetNumForName("GUNEMPF8");
			PreCacheGroup(start,end,cache_patch_t);
			start=W_GetNumForName("GRISE11");
			end  =W_GetNumForName("GDEAD2");
			break;

		case wallopobj:
			start=W_GetNumForName("BSTAR1");
			end  =W_GetNumForName("BSTAR4");
			PreCacheGroup(start,end,cache_patch_t);
			start=W_GetNumForName("BCRAFT1");
			end  =W_GetNumForName("BCRAFT16");
			break;

		case wallfireobj:

			SD_PreCacheSound(SD_FIRECHUTESND);
			SD_PreCacheSound(SD_FIREBALLSND);
			SD_PreCacheSound(SD_FIREBALLHITSND);

			start = W_GetNumForName("CRFIRE11");
			end = W_GetNumForName("CREXP5");

		case pillarobj:

			start=W_GetNumForName("PUSHCOL1");
         end  =W_GetNumForName("PSHCOL1A");
         //end  =W_GetNumForName("PUSHCOL3");
			break;

		case firejetobj:

         SD_PreCacheSound(SD_FIREJETSND);

         if (which)
            {
            start=W_GetNumForName("FJUP0");
            end  =W_GetNumForName("FJUP22");
            }
  #if (SHAREWARE == 0)
         else
            {
            start=W_GetNumForName("FJDOWN0");
            end  =W_GetNumForName("FJDOWN22");
            }
  #endif

			break;

		case bladeobj:

			SD_PreCacheSound(SD_BLADESPINSND);


  #if (SHAREWARE == 0)


         if (which&2)
            {
            if (which&1)
               {
               start=W_GetNumForName("SPSTUP1");
               end  =W_GetNumForName("SPSTUP16");
               }
            else
               {
               start=W_GetNumForName("SPSTDN1");
               end  =W_GetNumForName("SPSTDN16");
               }
            }
         else
            {
            if (which&1)
               {
               start=W_GetNumForName("UBLADE1");
               end  =W_GetNumForName("UBLADE9");
               }
            else
               {
               start=W_GetNumForName("DBLADE1");
               end  =W_GetNumForName("DBLADE9");
               }
            }
  #else
         start=W_GetNumForName("UBLADE1");
         end  =W_GetNumForName("UBLADE9");
  #endif

			break;
		case crushcolobj:

			SD_PreCacheSound(SD_CYLINDERMOVESND);
         if (which)
            {
            start=W_GetNumForName("CRDOWN1");
            end  =W_GetNumForName("CRDOWN8");
            }
  #if (SHAREWARE == 0)
         else
            {
            start=W_GetNumForName("CRUP1");
            end  =W_GetNumForName("CRUP8");
            }
  #endif
         break;

		case boulderobj:
			start=W_GetNumForName("BOL11");
			end  =W_GetNumForName("BSINK9");
			SD_PreCacheSound(SD_BOULDERHITSND);
			SD_PreCacheSound(SD_BOULDERROLLSND);
			SD_PreCacheSound(SD_BOULDERFALLSND);

			break;

		case spearobj:
			SD_PreCacheSound(SD_SPEARSTABSND);

         if (which)
            {
            start=W_GetNumForName("SPEARUP1");
            end  =W_GetNumForName("SPERUP16");
            }
#if (SHAREWARE == 0)
         else
            {
            start=W_GetNumForName("SPEARDN1");
            end  =W_GetNumForName("SPERDN16");
            }
#endif

			break;

		case gasgrateobj:

			start = SD_GASSTARTSND;
			end = SD_GASMASKSND;
         SD_PreCacheSoundGroup(start,end);
         if ((locplayerstate->player == 1) || (locplayerstate->player == 3))
			  SD_PreCacheSound(SD_PLAYERCOUGHFSND);
			else
			  SD_PreCacheSound(SD_PLAYERCOUGHMSND);
         start=-1;
         end=-1;
			break;

		case springobj:

			SD_PreCacheSound(SD_SPRINGBOARDSND);

			start=W_GetNumForName("SPRING1");
			end  =W_GetNumForName("SPRING9");
			break;
		default:
			return;
			break;
		}
   if ((start>=0) && (end>=0))
      PreCacheGroup(start,end,cache_patch_t);
}



/*
======================
=
= MiscPreCache
= precache the lump and check to see if it is already tagged
=
======================
*/
void MiscPreCache( void )
{
   int start;
   int end;

	//essential sounds

   SD_PreCacheSoundGroup(SD_HITWALLSND,SD_PLAYERDWHURTSND);
	SD_PreCacheSoundGroup(SD_RICOCHET1SND,SD_RICOCHET3SND);
	SD_PreCacheSound(SD_ATKPISTOLSND);
	SD_PreCacheSoundGroup(SD_PLAYERBURNEDSND,SD_PLAYERLANDSND);
	SD_PreCacheSoundGroup(SD_EXPLODEFLOORSND,SD_EXPLODESND);

   if (lightning==true)
      SD_PreCacheSound(SD_LIGHTNINGSND);

   SD_PreCacheSound(SD_BODYLANDSND);
   SD_PreCacheSound(SD_GIBSPLASHSND);
   SD_PreCacheSound(SD_ACTORLANDSND);
   SD_PreCacheSound(SD_ACTORSQUISHSND);


   // cache in bullet hole graphics
   start=W_GetNumForName("BULLETHO");
   end=W_GetNumForName("ALTBHO");
	PreCacheGroup(start,end,cache_transpatch_t);


   // cache in explosions

   if (DoPanicMapping()==true)
      {
      start=W_GetNumForName("EXPLOS1");
      end  =W_GetNumForName("EXPLOS20");
      PreCacheGroup(start,end,cache_patch_t);
      }
   else
      {
      start=W_GetNumForName("EXPLOS1");
      end  =W_GetNumForName("GREXP25");
      PreCacheGroup(start,end,cache_patch_t);
      }

	// cache in misc player sprites
	start=W_GetNumForName("BLOODS1");
	end  =W_GetNumForName("PLATFRM5");
	PreCacheGroup(start,end,cache_patch_t);

   // cache in missile smoke
	start=W_GetNumForName("MISSMO11");
	end  =W_GetNumForName("MISSMO14");
   PreCacheGroup(start,end,cache_patch_t);

#if (DEVELOPMENT == 1)
	// cache in all weapon sounds
   SD_PreCacheSoundGroup(SD_ATKPISTOLSND,SD_LOSEMODESND);

	// cache in misc player weapons
#if (SHAREWARE == 0)
   start=W_GetNumForName("KNIFE1");
   end  =W_GetNumForName("DOGPAW4");
   PreCacheGroup(start,end,cache_patch_t);
	// cache in kinetic sphere
   start=W_GetNumForName("KSPHERE1");
	end  =W_GetNumForName("KSPHERE4");
   PreCacheGroup(start,end,cache_patch_t);

#else
   start=W_GetNumForName("MPIST11");
   end  =W_GetNumForName("GODHAND8");
   PreCacheGroup(start,end,cache_patch_t);
#endif


   // cache in god mode stuff

	PreCacheGroup(W_GetNumForName("VAPO1"),
	              W_GetNumForName("LITSOUL"),
	              cache_patch_t);

	PreCacheGroup(W_GetNumForName("GODFIRE1"),
					  W_GetNumForName("GODFIRE4"),
					  cache_patch_t);


#endif
   // cache in player's gun

   // cache in rubble
   start=W_GetNumForName("RUBBLE1");
	end  =W_GetNumForName("RUBBLE10");
   PreCacheGroup(start,end,cache_patch_t);

   // cache in guts
   start=W_GetNumForName("GUTS1");
   end  =W_GetNumForName("GUTS12");
   PreCacheGroup(start,end,cache_patch_t);

   // cache in player missile
   start=W_GetNumForName("BJMISS1");
	end  =W_GetNumForName("BJMISS16");
   PreCacheGroup(start,end,cache_patch_t);

   if (gamestate.violence >= vl_high)
      {                                  // cache in all gibs
      if (DoPanicMapping()==true)
         {
         start = W_GetNumForName("ORGAN1");
         end = W_GetNumForName("ORGAN12");
         }
      else
         {
         start = W_GetNumForName("PART1");
         end = W_GetNumForName("GEYE3");
         }
      PreCacheGroup(start,end,cache_patch_t);
      }
}


/*
========================
=
= IsChristmas
=
========================
*/

boolean IsChristmas(void)
   {
   struct dosdate_t date;

   _dos_getdate(&date);

   if (((date.day == 24) || (date.day == 25)) &&      //Christmas
       (date.month == 12)
      )
      return true;

   return false;

   }


/*
========================
=
= CheckHolidays
=
========================
*/

void CheckHolidays(void)
   {
   struct dosdate_t date;

   _dos_getdate(&date);


   if (IsChristmas())
      DrawNormalSprite(0,0,W_GetNumForName("santahat"));

   else if ((date.month == 5) && (date.day == 5))    // Cinco de Mayo
      DrawNormalSprite(0,0,W_GetNumForName("sombrero"));

   else if ((date.month == 7) && (date.day == 4))     // 4th of July
      DrawNormalSprite(0,0,W_GetNumForName("amflag"));

   else if ((date.month == 10) && (date.day == 31))    // Halloween
      DrawNormalSprite(0,0,W_GetNumForName("witchhat"));

   else if ((date.month == 4) && (date.dayofweek == 0))   //Easter
      {
      int i;

      for(i=15;i<=21;i++)
         {
         if (date.day == i)
            DrawNormalSprite(0,0,W_GetNumForName("esterhat"));
         }
      }
   }


/*
======================
=
= DrawPreCache
=
======================
*/
extern boolean dopefish;
void DrawPreCache( void )
{
   if (loadedgame==false)
      {
      char temp[80];
      int width, height, num;
      char buf[30];

      if ( BATTLEMODE )
			{
         VL_DrawPostPic (W_GetNumForName("trilogo"));
         VWB_TBar ( 30, 23, 260, 82 );
         ShowBattleOptions( false, 56, 26 );

         DrawPlayers ();
         }
      else
         {
         pic_t * pic;
         pic=(pic_t *)W_CacheLumpName("mmbk",PU_CACHE, Cvt_pic_t, 1);
         VWB_DrawPic (0, 0, pic);

         CheckHolidays();
         }

      DrawNormalSprite (PRECACHEBARX, PRECACHEBARY, W_GetNumForName ("cachebar"));

      CurrentFont=smallfont;

      PrintY = PRECACHEESTRINGY;
      PrintX = PRECACHEESTRINGX;

      memset (&buf[0], 0, sizeof (buf));

      if ( !BATTLEMODE )
      {
         memcpy (&buf[0], "EPISODE ", 8);
         itoa (gamestate.episode,&buf[8],10);
      }
      else
			memcpy (&buf[0], "COMM-BAT", 8);

      US_MeasureStr (&width, &height, "%s", &buf[0]);
      VWB_TBar (PrintX-2, PrintY-2, width+4, height+4);
      US_BufPrint (&buf[0]);


      PrintY = PRECACHEASTRINGY;

      memset (&buf[0], 0, sizeof (buf));
      memcpy (&buf[0], "AREA ", 5);

      if ( !BATTLEMODE )
         {
         itoa( GetLevel( gamestate.episode, gamestate.mapon ),
            &buf[ 5 ], 10 );
         }
      else
         {
         itoa( gamestate.mapon + 1, &buf[ 5 ], 10 );
         }
      US_MeasureStr (&width, &height, "%s", &buf[0]);
      PrintX = (300-width);
      VWB_TBar (PrintX-2, PrintY-2, width+4, height+4);
      US_BufPrint (&buf[0]);


      PrintY = PRECACHESTRINGY;

      num = (RandomNumber ("PreCacheString", 0)) % MAXSILLYSTRINGS;

      if ((dopefish==true) || (tedlevel == true))
         strcpy (temp, &(CacheStrings[num][0]));
      else
         strcpy (temp, &(LevelName[0]));

      US_MeasureStr (&width, &height, "%s", &temp[0]);

      PrintX = (320-width) >> 1;
      PrintY = PRECACHESTRINGY;
      VWB_TBar (PrintX-2, PrintY-2, width+4, height+4);

      US_BufPrint (&temp[0]);

      VW_UpdateScreen();

      MenuFadeIn ();
      }
}

#define CACHETICDELAY (6)
/*
======================
=
= PreCache
= precache all the lumps for the level
=
======================
*/
void PreCache( void )
{
   int i;
	int total;
   byte * dummy;
   int maxheapsize;
   int newheap;

   int currentmem;
   int currentcache;
   int lastmem=0;
   int lastcache=0;
   int ticdelay;
   byte *tempbuf;

   double Gs;
   Gs = (iGLOBAL_SCREENWIDTH*100/320);
   Gs = Gs / 100;

//SetTextMode (  );

/*
#define  PRECACHEBARX 28
#define  PRECACHEBARY 178

#define  PRECACHELED1X 9
#define  PRECACHELED1Y 8

#define  PRECACHELED2X 9
#define  PRECACHELED2Y 12
*/

   if (CachingStarted==false)
      {
      if (loadedgame==false)
         {
         ClearGraphicsScreen();
         MenuFadeIn ();
         }
      return;
      }

	MiscPreCache();

   SortPreCache();

   if (loadedgame==false)
      {
		maxheapsize=Z_HeapSize();
      total=0;

      tempbuf=bufferofs;
      bufferofs=page1start;
      ticdelay=CACHETICDELAY;
      for (i=1;i<cacheindex;i++)
			{
         // Skip exhaustive pre-caching to save PSRAM. 
         // ROTT will load lumps on demand via W_CacheLumpNum anyway.
         // dummy=W_CacheLumpNum(cachelist[i].lump,cachelist[i].cachelevel, CvtForType(cachelist[i].type), 1);
         total+=W_LumpLength(cachelist[i].lump);
         newheap=Z_UsedHeap();
			currentmem=(newheap*MAXLEDS)/maxheapsize;
         while (lastmem<=currentmem)
            {//SetTextMode (  );
   			if ( iGLOBAL_SCREENWIDTH == 320) {
				DrawNormalSprite (PRECACHEBARX+PRECACHELED1X+(lastmem<<2),
                              PRECACHEBARY+PRECACHELED1Y,
                              W_GetNumForName ("led1"));//led1 progressbar
			}else if ( iGLOBAL_SCREENWIDTH == 640) {
				DrawNormalSprite (72+(Gs*(lastmem<<2)),446,W_GetNumForName ("led1"));//led1 progressbar
				DrawNormalSprite (72+(Gs*(lastmem<<2)),446+3,W_GetNumForName ("led1"));//led1 progressbar
				DrawNormalSprite (72+3+(Gs*(lastmem<<2)),446,W_GetNumForName ("led1"));//led1 progressbar
				DrawNormalSprite (72+3+(Gs*(lastmem<<2)),446+3,W_GetNumForName ("led1"));//led1 progressbar
			}else if ( iGLOBAL_SCREENWIDTH == 800) {
				DrawNormalSprite (91+(Gs*(lastmem<<2)),559,W_GetNumForName ("led1"));//led1 progressbar
				DrawNormalSprite (91+(Gs*(lastmem<<2)),559+3,W_GetNumForName ("led1"));//led1 progressbar
				DrawNormalSprite (91+3+(Gs*(lastmem<<2)),559,W_GetNumForName ("led1"));//led1 progressbar
				DrawNormalSprite (91+3+(Gs*(lastmem<<2)),559+3,W_GetNumForName ("led1"));//led1 progressbar
			}

            lastmem++;
				VW_UpdateScreen (); // was missing, fixed
            }
         // currentcache=(i*MAXLEDS)/(cacheindex+1);
         if ((i & 15) == 0) vTaskDelay(1);
         while (lastcache<=currentcache)

            {

   			if ( iGLOBAL_SCREENWIDTH == 320) {
				DrawNormalSprite (PRECACHEBARX+PRECACHELED2X+(lastcache<<2),
                              PRECACHEBARY+PRECACHELED2Y,
                              W_GetNumForName ("led2"));//led2 progressbar
			}else if ( iGLOBAL_SCREENWIDTH == 640) {
				DrawNormalSprite (72+(Gs*(lastcache<<2)),458,W_GetNumForName ("led2"));//led2 progressbar
				DrawNormalSprite (72+(Gs*(lastcache<<2)),458+3,W_GetNumForName ("led2"));//led2 progressbar
				DrawNormalSprite (72+3+(Gs*(lastcache<<2)),458,W_GetNumForName ("led2"));//led2 progressbar
				DrawNormalSprite (72+3+(Gs*(lastcache<<2)),458+3,W_GetNumForName ("led2"));//led2 progressbar
			
			}else if ( iGLOBAL_SCREENWIDTH == 800) {
				DrawNormalSprite (91+(Gs*(lastcache<<2)),573,W_GetNumForName ("led2"));//led2 progressbar
				DrawNormalSprite (91+(Gs*(lastcache<<2)),573+3,W_GetNumForName ("led2"));//led2 progressbar
				DrawNormalSprite (91+3+(Gs*(lastcache<<2)),573,W_GetNumForName ("led2"));//led2 progressbar
				DrawNormalSprite (91+3+(Gs*(lastcache<<2)),573+3,W_GetNumForName ("led2"));//led2 progressbar
			}
			DisableScreenStretch();//bna++
			VW_UpdateScreen ();//bna++
            lastcache++;
            ticdelay--;
            if (ticdelay==0)
               {
               extern boolean dopefish;

               if ( dopefish==true )
                  {
                  SD_PlayPitchedSound ( SD_DOPEFISHSND, 255, 0 );
                  }
               ticdelay=CACHETICDELAY;
               }
				VW_UpdateScreen (); // was missing, fixed
            }
         }
	  DisableScreenStretch();//bna++
	  VW_UpdateScreen ();//bna++
	  //I_Delay(200);
      bufferofs=tempbuf;
      ShutdownPreCache ();

      if ( BATTLEMODE )
         {
         int width,height;
         char buf[30];//byte * shape;
		double WHratio = 16200/200;
		WHratio = WHratio/100;
///	iGLOBAL_SCREENWIDTH = 640;
//	iGLOBAL_SCREENHEIGHT = 480;
DisableScreenStretch();

	// Cache in fonts
//	shape = W_CacheLumpNum (W_GetNumForName ("newfnt1"), PU_STATIC, Cvt_font_t, 1);
//	bigfont = (font_t *)shape;
         CurrentFont = newfont1;//smallfont;

         strcpy( buf, "Press Any Key" );
         US_MeasureStr (&width, &height, "%s", &buf[ 0 ] );
         PrintX = (iGLOBAL_SCREENWIDTH-(width)) / 2;
         PrintY = WHratio*iGLOBAL_SCREENHEIGHT;//162;
         //VWB_TBar (PrintX-2, PrintY-2, width+4, height+4);
         US_BufPrint (&buf[0]);

         VW_UpdateScreen();

         IN_StartAck();
         while (!IN_CheckAck ())
            ;
         }
 //  EnableScreenStretch();
#if (DEVELOPMENT == 1)
      tempbuf=bufferofs;
      bufferofs=displayofs;
      CurrentFont = smallfont;
      US_CenterWindow(30,6);
      PrintY+=6;
      US_Print("Max  Heap Size:");
      US_PrintUnsigned(maxheapsize);
		US_Print("\n");
      US_Print("Used Heap Size:");
      US_PrintUnsigned(newheap);
      US_Print("\n");
      US_Print("Percentage Used:");
      US_PrintUnsigned(newheap*100/maxheapsize);
		US_Print("\n");
      US_Print("TotalPrecached:");
      US_PrintUnsigned(total);
      bufferofs=tempbuf;
      I_Delay (40);
#endif
#if (PRECACHETEST == 1)
      SoftError("Max  Heap Size: %ld\n",maxheapsize);
      SoftError("Used Heap Size: %ld\n",newheap);
      SoftError("TotalPrecached: %ld\n",total);
#endif
      }
   else
      {
      for (i=1;i<cacheindex;i++)
         {
         if ((i & 15) == 0) vTaskDelay(1);
			dummy=W_CacheLumpNum(cachelist[i].lump,cachelist[i].cachelevel, CvtForType(cachelist[i].type), 1);
         DoLoadGameAction ();
         }
      ShutdownPreCache ();
      }
   if (CheckParm("LEVELSIZE")!=0)
      {
      OpenMapDebug();

      MapDebug("Map Number %d\n",gamestate.mapon);
      MapDebug("sizeoflevel=%d\n",Z_UsedLevelHeap());
      }
#if (PRECACHETEST == 1)
   SoftError("<<<<<<<<<<<<<<<<<<<<<<<Precaching done\n");
#endif
}



/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (word *source, word *dest,long length, unsigned rlewtag)
{
        word  value,count,i;
        word          *end;

        end = dest + length;
        //
        // expand it
        //
        do
        {
					 value = IntelShort(*source++);
                if (value != rlewtag)
                //
					 // uncompressed
					 //
                        *dest++=value;
                else
                {
                //
                // compressed string
                //
                        count = IntelShort(*source++);
                        value = IntelShort(*source++);
                        for (i=1;i<=count;i++)
                                *dest++ = value;
					 }
        } while (dest<end);
}

/*
======================
=
= CheckRTLVersion
=
======================
*/

void CheckRTLVersion
   (
   char *filename
   )

   {
   int  filehandle;
   char RTLSignature[ 4 ];
   unsigned int RTLVersion;

   filehandle = SafeOpenRead( filename );

   //
   // Load RTL signature
   //
   SafeRead( filehandle, RTLSignature, sizeof( RTLSignature ) );

   if ( ( strcmp( RTLSignature, COMMBAT_SIGNATURE ) != 0 ) &&
      ( strcmp( RTLSignature, NORMAL_SIGNATURE ) != 0 ) )
      {
      Error( "The file '%s' is not a valid level file.", filename );
      }

   //
   // Check the version number
   //
   SafeRead( filehandle, &RTLVersion, sizeof( RTLVersion ) );
   SwapIntelLong(&RTLVersion);
   if ( RTLVersion > RTL_VERSION )
      {
      Error(
         "The file '%s' is a version %d.%d %s file.\n"
         "The highest this version of ROTT can load is %d.%d.", filename,
         RTLVersion >> 8, RTLVersion & 0xff, RTLSignature,
         RTL_VERSION >> 8, RTL_VERSION & 0xff );
      }

   close( filehandle );
   }


/*
======================
=
= ReadROTTMap
=
======================
*/

void ReadROTTMap
   (
   char *filename,
   int mapnum
   )

   {
   RTLMAP RTLMap;
   int    filehandle;
   long   pos;
   long   compressed;
   long   expanded;
   int    plane;
   byte  *buffer;

   CheckRTLVersion( filename );
   filehandle = SafeOpenRead( filename );

   //
   // Load map header
   //
   lseek( filehandle, RTL_HEADER_OFFSET + mapnum * sizeof( RTLMap ),
      SEEK_SET );
   SafeRead( filehandle, &RTLMap, sizeof( RTLMap ) );

    SwapIntelLong((int *)&RTLMap.used);
    SwapIntelLong((int *)&RTLMap.CRC);
    SwapIntelLong((int *)&RTLMap.RLEWtag);
    SwapIntelLong((int *)&RTLMap.MapSpecials);
    SwapIntelLongArray((int *)&RTLMap.planestart, NUMPLANES);
    SwapIntelLongArray((int *)&RTLMap.planelength, NUMPLANES);

   if ( !RTLMap.used )
      {
      Error( "ReadROTTMap: Tried to load a non existent map!" );
      }

   #if ( SHAREWARE == 1 )
      if ( RTLMap.RLEWtag == REGISTERED_TAG )
         {
         Error( "Can't use maps from the registered game in shareware version." );
         }

      if ( RTLMap.RLEWtag != SHAREWARE_TAG )
         {
         Error( "Can't use modified maps in shareware version." );
         }
   #endif

   mapwidth  = 128;
   mapheight = 128;

   // Get special map flags
   MapSpecials = RTLMap.MapSpecials;

   //
   // load the planes in
   //
   expanded = 128 * 128 * 2;

	for( plane = 0; plane <= 2; plane++ )
      {
      pos        = RTLMap.planestart[ plane ];
      compressed = RTLMap.planelength[ plane ];
		buffer     = SafeMalloc( compressed );
      lseek( filehandle, pos, SEEK_SET );
      SafeRead( filehandle, buffer, compressed );

      mapplanes[ plane ] = Z_Malloc( expanded, PU_LEVEL, &mapplanes[ plane ] );
      memset(mapplanes[ plane ], 0, expanded);

      // Always expand to 128x128 words (ROTT standard)
      CA_RLEWexpand((word *)buffer, (word *)mapplanes[plane], 128 * 128, RTLMap.RLEWtag);
      
      SafeFree(buffer);
      }
   close(filehandle);

   //
   // get map name
   //
   strcpy( LevelName, RTLMap.Name );
   }



/*
======================
=
= GetNextMap
=
======================
*/
int GetNextMap ( int tilex, int tiley )
   {
   word next;
   word icon;
   boolean done;

   next = MAPSPOT( tilex, tiley, 2 );
   icon = MAPSPOT( tilex, tiley, 1 );
   done=false;
   if ( ( ( icon != EXITTILE ) && ( icon != SECRETEXITTILE ) ) ||
      ( ( ( next&0xff00 ) != 0xe200 ) && ( ( next&0xff00 ) != 0xe400 ) ) )
      {
      int i,j;

	   for ( j = 0; j < mapheight; j++ )
         {
	      for ( i = 0; i < mapwidth; i++ )
            {
            icon = MAPSPOT( i, j, 1 );
            next = MAPSPOT( i, j, 2 );
            if ( ( ( icon == EXITTILE ) || ( icon == SECRETEXITTILE ) ) &&
               ( ( ( next&0xff00 ) == 0xe200 ) ||
               ( ( next&0xff00 ) == 0xe400 ) ) )
               {
               done=true;
               break;
               }
            }

         if ( done == true )
            {
            break;
            }
         }

      if ( !done )
         {
         Error( "GetNextMap : No exit tile on map %d.", gamestate.mapon );
         }
      }
   if ( ( ( next & 0xff00 ) != 0xe200 ) &&
      ( ( next & 0xff00 ) != 0xe400 ) )
      {
      // Should this be DEVELOPMENT only?
      Error( "GetNextMap : Illegal destination map %xh at exit "
         "tile on map %d.", next, gamestate.mapon );
      }

   if ( next == 0xe2ff )
      {
      return -1;
      }

   return ( next & 0xff );
   }

/*
======================
=
= GetMapFileInfo
=
======================
*/
void GetMapFileInfo
   (
   mapfileinfo_t *mapinfo,
   char *filename
   )

   {
   RTLMAP RTLMap[ 100 ];
   int    filehandle;
   int    i;
   int    nummaps;

   CheckRTLVersion( filename );

   filehandle = SafeOpenRead( filename );

   //
   // Load map header
   //
   lseek( filehandle, RTL_HEADER_OFFSET, SEEK_SET );
   SafeRead( filehandle, &RTLMap, sizeof( RTLMap ) );
   close( filehandle );

   nummaps = 0;
   for( i = 0; i < 100; i++ )
      {
      if ( !RTLMap[ i ].used )
         {
         continue;
         }

      mapinfo->maps[ nummaps ].number = i;

      strcpy( mapinfo->maps[ nummaps ].mapname, RTLMap[ i ].Name );

      nummaps++;
      }

   mapinfo->nummaps = nummaps;
   }

/*
======================
=
= GetMapFileName
=
======================
*/
void GetMapFileName ( char * filename )
{
   if ( ( BATTLEMODE ) && (BattleLevels.avail == true) )
      {
      strcpy(filename,BattleLevels.file);
      }
   else if (GameLevels.avail == true)
      {
      strcpy(filename,GameLevels.file);
      }
   else if ( BATTLEMODE )
      {
      strcpy(filename,BATTMAPS);
      }
   else
      {
      strcpy(filename,ROTTMAPS);
      }
}

/*
======================
=
= SetBattleMapFileName
=
======================
*/
void SetBattleMapFileName ( char * filename )
{
   BattleLevels.avail = true;
   memset (&(BattleLevels.file[0]), 0, sizeof (BattleLevels.file));
   strcpy (&(BattleLevels.file[0]), filename);
}

/*
======================
=
= GetMapCRC
=
======================
*/
word GetMapCRC
   (
   int num
   )

   {
   int  filehandle;
   char filename[ 80 ];
   RTLMAP RTLMap;

   GetMapFileName( &filename[ 0 ] );
   CheckRTLVersion( filename );
   filehandle = SafeOpenRead( filename );

   //
   // Load map header
   //
   lseek( filehandle, RTL_HEADER_OFFSET + num * sizeof( RTLMap ), SEEK_SET );
   SafeRead( filehandle, &RTLMap, sizeof( RTLMap ) );

   close( filehandle );

   return( RTLMap.CRC );
   }


/*
======================
=
= GetAlternateMapInfo
=
======================
*/

void GetAlternateMapInfo (mapfileinfo_t * mapinfo, AlternateInformation *info)
{
   if (UL_ChangeDirectory (info->path) == false)
      Error ("ERROR : Can't change to alternate directory %s!\n", info->path);

   GetMapFileInfo (mapinfo, info->file);

   UL_ChangeDirectory (&CWD[0]);
}

/*
======================
=
= GetMapInfo
=
======================
*/
void GetMapInfo
   (
   mapfileinfo_t *mapinfo
   )

   {
   if ( ( BATTLEMODE ) && ( BattleLevels.avail == true ) )
      {
      GetAlternateMapInfo( mapinfo, &BattleLevels );
      }
   else if ( GameLevels.avail == true )
      {
      GetAlternateMapInfo( mapinfo, &GameLevels );
      }
   else if ( BATTLEMODE )
      {
      GetMapFileInfo( mapinfo, BATTMAPS );
      }
   else
      {
      GetMapFileInfo( mapinfo, ROTTMAPS );
      }
   }

/*
======================
=
= LoadTedMap
=
======================
*/
void LoadTedMap
   (
   const char *extension,
   int mapnum
   )

   {
   long    pos;
   long    compressed;
   long    expanded;
   int     plane;
   int     i;
   int     maphandle;
   byte   *buffer;
   maptype mapheader;
   char    name[ 200 ];
   mapfiletype *tinf;

   //
   // load maphead.ext (offsets and tileinfo for map file)
   //
   strcpy( name, "maphead." );
   strcat( name, extension );
   LoadFile( name, ( void * )&tinf );

   // fix structure alignment
   tinf = ( void * )( ( word * )tinf - 1 );

   for( i = 0 ; i < 100 ; i++ )
      {
      tinf->headeroffsets[ i ] = IntelLong( tinf->headeroffsets[ i ] );
      }

   //
   // open the data file
   //
   strcpy( name, "maptemp." );
   strcat( name, extension );
   maphandle = SafeOpenRead( name );

   //
   // load map header
   //
   pos = tinf->headeroffsets[ mapnum ];

   // $FFFFFFFF start is a sparse map
   if ( pos < 0 )
      {
      Error( "LoadTedMap : Tried to load a non existent map!" );
      }

   lseek( maphandle, pos, SEEK_SET );
   SafeRead( maphandle, &mapheader, sizeof( maptype ) );

   for( i = 0 ; i < 3; i++ )
      {
      mapheader.planestart[ i ]  = IntelLong( mapheader.planestart[ i ] );
      mapheader.planelength[ i ] = IntelShort( mapheader.planelength[ i ] );
      }

   mapheader.width  = IntelShort( mapheader.width );
   mapheader.height = IntelShort( mapheader.height );

   mapwidth  = 128;
   mapheight = 128;

   // Set special map flags
   MapSpecials = 0;

   //
   // load the planes in
   //
   expanded = 128 * 128 * 2;

   for( plane = 0; plane <= 2; plane++ )
      {
      pos = mapheader.planestart[ plane ];
      lseek( maphandle, pos, SEEK_SET );

      compressed = mapheader.planelength[ plane ];
      buffer = SafeMalloc( compressed );
      SafeRead( maphandle, buffer, compressed );

      mapplanes[ plane ] = Z_Malloc( expanded, PU_LEVEL, &mapplanes[ plane ] );
      memset(mapplanes[ plane ], 0, expanded);

      // Expand to temp Row-Major buffer
      word *temp = SafeMalloc(128 * 128 * 2);
      memset(temp, 0, 128 * 128 * 2);

      if (mapheader.width == 64) 
      {
          word *rle_temp = SafeMalloc(64 * 64 * 2);
          CA_RLEWexpand((word *)(buffer + 2), rle_temp, 64 * 64, 0xabcd);
          // ROTT maps are Row-Major (X inner). Expand 64x64 to 128x128.
          for (int y = 0; y < 64; y++) 
              memcpy(temp + (y * 128), rle_temp + (y * 64), 64 * 2);
          SafeFree(rle_temp);
      }
      else
      {
          // RLEWexpand directly into Row-Major temp buffer (128x128)
          CA_RLEWexpand( ( word * )( buffer + 2 ), temp, 128 * 128, 0xabcd );
      }

      // Store in mapplanes as Row-Major grid [row][col]
      word *dest = (word *)mapplanes[plane];
      memcpy(dest, temp, 128 * 128 * 2);

      mapwidth = 128; // Force logical stride to match MAPSIZE
      mapheight = 128;

      SafeFree(temp);
      SafeFree( buffer );
      }

   // fix structure alignment
	tinf = ( void * )( ( word * )tinf + 1 );

	SafeFree( tinf );

   if ( close( maphandle ) )
      {
      Error( "Error closing Ted file Error #%d", errno );
      }
   }


/*
======================
=
= LoadAlternateMap
=
======================
*/

void LoadAlternateMap (AlternateInformation *info, int mapnum)
{
   if (UL_ChangeDirectory (info->path) == false)
      Error ("ERROR : Can't change to alternate directory %s!\n",info->path);

   ReadROTTMap (info->file, mapnum);

   UL_ChangeDirectory (&CWD[0]);
}

/*
======================
=
= LoadROTTMap
=
======================
*/
void LoadROTTMap
   (
   int mapnum
   )

   {
   if ( tedlevel == true )
      {
      LoadTedMap( "rot", mapnum );
      }
   else if ( ( BATTLEMODE ) && ( BattleLevels.avail == true ) )
      {
      LoadAlternateMap( &BattleLevels, mapnum );
      }
   else if ( GameLevels.avail == true )
      {
      LoadAlternateMap( &GameLevels, mapnum );
      }
   else if ( BATTLEMODE )
      {
      ReadROTTMap( BATTMAPS, mapnum );
      }
   else
      {
      ReadROTTMap( ROTTMAPS, mapnum );
      }
   }


void CountAreaTiles(void)
{int i,j,areanumber;
 word*map,tile;

 memset(numareatiles,0,(NUMAREAS + 1) * sizeof(int));

 map  = mapplanes[0];

 for(j=0;j<MAPSIZE;j++)
   for(i=0;i<MAPSIZE;i++)
      {tile = *map++;

       areanumber = tile - AREATILE;
       if ((areanumber >= 0) && (areanumber <= NUMAREAS))
         numareatiles[areanumber] ++;
      }

}



#define InitWall(lump,index,newx,newy)      \
   {                                        \
   PreCacheLump(lump,PU_CACHEWALLS,cache_pic_t);  \
   if (W_LumpLength(lump) == 0)                   \
      Error("%s being used in shareware at %d %d",\
      W_GetNameForNum(lump),newx,newy);           \
   actorat[newx][newy]= &walls[index];      \
   tempwall = (wall_t*)actorat[newx][newy]; \
   tempwall->which = WALL;                  \
   tempwall->tile = index;                  \
   }                                        \


/*
==================
=
= SetupWalls
=
==================
*/
void SetupWalls( void )
   {
   int   i,j,lump,index;
	word   *map,tile;
   wall_t * tempwall;


	for (i=0;i<MAXWALLTILES;i++)
      memset(&walls[i],0,sizeof(wall_t));

	map = mapplanes[0];
	for (j=0; j<mapheight; j++)
      {
	   for(i=0; i<mapwidth; i++)
			{
         if ((i & 63) == 0) vTaskDelay(1);
			if ((loadedgame == false) && (MAPSPOT(i,j,2) == 0xeeee))
				{_2Dpoint *tdptr;

				 tdptr = &(MISCVARS->EPOP[MISCVARS->nextpop]);
				 tdptr->x = i;
				 tdptr->y = j;
				 MISCVARS->nextpop ++;
				 MISCVARS->popsleft ++;
			   }
		   tile= *map++;

         if ((tile > 89) ||
             ((tile > 32) && (tile < 36)) ||
             (tile == 44) ||
             (tile == 45) ||
             (tile == 0)
            )
            {
            tilemap[i][j] = 0;
            continue;
            }

         if (tile <= 32)
            {
            index = tile;
            }
         else
            index = tile-3;

         if ((tile > 75) && (tile <= 79))
            {
            lump = tilemap[i][j] = GetLumpForTile(tile);
            PreCacheLump(lump,PU_CACHEWALLS,cache_pic_t);
            PreCacheLump(elevatorstart+5,PU_CACHEWALLS,cache_pic_t);
            PreCacheLump(elevatorstart+6,PU_CACHEWALLS,cache_pic_t);
            PreCacheLump(elevatorstart+7,PU_CACHEWALLS,cache_pic_t);
            tilemap[i][j]|=0x2000;
            if (MAPSPOT(i,j,2)==0)
               MAPSPOT(i,j,2)=21;
            }
         else if ((tile >= 47) && (tile <= 48))
            {
            lump = tilemap[i][j] = GetLumpForTile(tile);
            InitWall(lump,index,i,j);
            tilemap[i][j]|=0x2000;
            if (MAPSPOT(i,j,2)==0)
               MAPSPOT(i,j,2)=21;
            }
         else
            {
            lump = tilemap[i][j] = GetLumpForTile(tile);
            InitWall(lump,index,i,j);
            if (MAPSPOT(i,j,2))
               tilemap[i][j]|=0x2000;
            }
         }
      }
   }


/*
===============
=
= GetNearestAreaNumber
=
===============
*/
word GetNearestAreaNumber ( int tilex, int tiley )
{
	int up,dn,lt,rt;
   int tile;

  	tile=MAPSPOT(tilex,tiley,0)-AREATILE;

  	if ((tile<=NUMAREAS) && (tile>0))
      return (tile+AREATILE);

	up=MAPSPOT(tilex,tiley-1,0)-AREATILE;
	dn=MAPSPOT(tilex,tiley+1,0)-AREATILE;
	lt=MAPSPOT(tilex-1,tiley,0)-AREATILE;
	rt=MAPSPOT(tilex+1,tiley,0)-AREATILE;

	up = ((up>0) && (up<=NUMAREAS));
	dn = ((dn>0) && (dn<=NUMAREAS));
	lt = ((lt>0) && (lt<=NUMAREAS));
	rt = ((rt>0) && (rt<=NUMAREAS));

	if (rt)
		return (MAPSPOT(tilex+1,tiley,0) + AREATILE);
	else if (lt)
		return (MAPSPOT(tilex-1,tiley,0) + AREATILE);
	else if (up)
		return (MAPSPOT(tilex,tiley-1,0) + AREATILE);
	else if (dn)
		return (MAPSPOT(tilex,tiley+1,0) + AREATILE);
//	else
//		Error("GetNearestAreaNumber: Couldn't fix up area at x=%ld y=%ld\n",tilex,tiley);
   return (NUMAREAS+AREATILE-1);
}

/*
===============
=
= SetupWindows
=
===============
*/
void SetupWindows ( void )
{
   int i,j;
   boolean skythere;

   skythere = SkyExists();

	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
         {
         if ((i>=0) && (i<=3) && (j==0))
			   continue;
         if (IsWindow(i,j))
            {
            actorat[i][j]=0;
            if (skythere==true)
               {
               tilemap[i][j]|=0x2000;
               }
            else
               {
               MAPSPOT(i,j,2)=0;
               }
            MAPSPOT(i,j,0)=(word)(GetNearestAreaNumber(i,j));
            }
         }
      }

}


/*
==================
=
= GetWallIndex
=
==================
*/

int GetWallIndex( int texture )
{
	int wallstart;
   int exitstart;

	wallstart=W_GetNumForName("WALLSTRT");
	exitstart=W_GetNumForName("EXITSTRT");
	elevatorstart = W_GetNumForName("ELEVSTRT");

   if (texture&0x1000)
      {
      texture&=~0x1000;
      if (texture==0)
         return 41;
      else if (texture==1)
         return 90;
      else if (texture==2)
         return 91;
      else if (texture==3)
         return 42;
      else if (texture==4)
         return 92;
      else if (texture==5)
         return 93;
      else if (texture==6)
         return 94;
      else if (texture==7)
         return 95;
      else if (texture==8)
         return 96;
      else if (texture==9)
         return 97;
      else if (texture==10)
         return 98;
      else if (texture==11)
         return 99;
      else if (texture==12)
         return 100;
      else if (texture==13)
         return 101;
      else if (texture==14)
         return 102;
      else if (texture==15)
         return 103;
		else if (texture==16)
         return 104;
      }
   else if (texture > elevatorstart)
      return (texture - elevatorstart + 68);
//	else if (texture > specialstart)
//      return (texture - specialstart + 41);
   else if (texture > exitstart)
      return (texture - exitstart + 43);
   else
      {
		if (texture>wallstart+63)
         return (texture - (wallstart + 63) + 76 );
      else if (texture>wallstart+40)
         return (texture - (wallstart + 40) + 45 );
      else
         return (texture - wallstart);
      }
   return 0x8000;
}

/*
==================
=
= SetupAnimatedWalls
=
==================
*/
void SetupAnimatedWalls( void )
{
	int   i,j;
	word   *map,tile;
   wall_t * tempwall;

	InitAnimatedWallList();
	map = mapplanes[0];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
		   {
         if (j==0 && i<=3)
			   {
			   map++;
			   continue;
			   }
			tile= *map++;
			if (tile == 44)
			   {
			   actorat[i][j]= &walls[tile-3];
			   tempwall = (wall_t*)actorat[i][j];
			   tempwall->which = WALL;
			 	tempwall->tile = tile-3;
			   tempwall->flags = FL_W_DAMAGE;
    	      SetupAnimatedWall(0);
				tilemap[i][j]=0;
            tilemap[i][j]|=0x1000;
			   }
			else if (tile == 45)
			   {
				actorat[i][j]= &walls[tile-3];
				tempwall = (wall_t*)actorat[i][j];
				tempwall->which = WALL;
				tempwall->tile = tile-3;
    	      SetupAnimatedWall(3);
				tilemap[i][j]=3;
            tilemap[i][j]|=0x1000;
			   }
			else if ((tile >= 106) && (tile <= 107))
			   {
			   actorat[i][j]= &walls[tile-16];
			   tempwall = (wall_t*)actorat[i][j];
			   tempwall->which = WALL;
			   tempwall->tile = tile-16;
	         SetupAnimatedWall(tile-105);
            tilemap[i][j]=tile-105;
            tilemap[i][j]|=0x1000;
            }
			else if ((tile >= 224) && (tile <= 233))
			   {
			   actorat[i][j]= &walls[tile-224+92];
			   tempwall = (wall_t*)actorat[i][j];
			   tempwall->which = WALL;
			   tempwall->tile = tile-224+94;
            if (tile==233)
               tempwall->flags = FL_W_DAMAGE;
	         SetupAnimatedWall(tile-224+4);
            tilemap[i][j]=tile-224+4;
            tilemap[i][j]|=0x1000;
            }
			else if ((tile >= 242) && (tile <= 244))
				{
			   actorat[i][j]= &walls[tile-242+102];
			   tempwall = (wall_t*)actorat[i][j];
			   tempwall->which = WALL;
			   tempwall->tile = tile-242+102;
	         SetupAnimatedWall(tile-242+14);
            tilemap[i][j]=tile-242+14;
            tilemap[i][j]|=0x1000;
            }
         }
		}
}


/*
==================
=
= SetupSwitches
=
==================
*/

void SetupSwitches( void )
{
	int   i,j;
	word   *map,tile;

	map = mapplanes[0];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
		   {
         if (j==0 && i<=3)
			   {
			   map++;
			   continue;
			   }
		   tile= *map++;
			if ((tile >= 76) && (tile <= 79))
            {
				if (tile == 79)
				   lastswitch->flags |= FL_ON;
				SpawnSwitchThingy(i,j);
				}
			else if ((tile == 157) || (tile == 175)) // hi masked switches
				{
				lastswitch->flags |= FL_W_INVERTED;
				lastswitch->flags |= FL_REVERSIBLE;
				if (tile==175)
					lastswitch->flags |= FL_ON;
				SpawnSwitchThingy(i,j);
				}
			}
		}
}



void RespawnPlayerobj(objtype *ob)
{int rand,numchecked=0;
 int oldsetupgame,nx,ny,ndir;
 playertype *pstate;

 M_LINKSTATE(ob,pstate);

 if (gamestate.battlemode != battle_CaptureTheTriad)
     {
      rand = (GameRandomNumber("playerobj respawn",0) % NUMSPAWNLOCATIONS);

      while(numchecked < NUMSPAWNLOCATIONS)
         {
         if (!actorat[SPAWNLOC[rand].x][SPAWNLOC[rand].y])
            {RevivePlayerobj(SPAWNLOC[rand].x,SPAWNLOC[rand].y,SPAWNLOC[rand].dir,ob);
            return;
            }
         numchecked ++;
         rand = (rand + 1) % NUMSPAWNLOCATIONS;
         }
#if (DEVELOPMENT == 1)
      SoftError("\nno spawn locations available, using FindEmptyTile");
#endif
      nx = SPAWNLOC[rand].x;
      ny = SPAWNLOC[rand].y;
      ndir = SPAWNLOC[rand].dir;
     }
 else
     {nx = TEAM[pstate->team].tilex;
      ny = TEAM[pstate->team].tiley;
      ndir = TEAM[pstate->team].dir;
     }

 oldsetupgame = insetupgame;
 insetupgame = true;
 FindEmptyTile(&nx,&ny);
 insetupgame = oldsetupgame;
 RevivePlayerobj(nx,ny,ndir,ob);


}

#define SetupSpecificFlagTeamAt(whichteam, spawnlocindex) \
{int newx,newy;                                           \
                                                          \
 newx = SPAWNLOC[spawnlocindex].x;                        \
 newy = SPAWNLOC[spawnlocindex].y;                        \
 TEAM[whichteam].tilex = newx;                            \
 TEAM[whichteam].tiley = newy;                            \
 TEAM[whichteam].dir = SPAWNLOC[spawnlocindex].x;         \
 SpawnStatic(newx,newy,stat_collector,9);                  \
 SpawnNewObj(newx,newy,&s_basemarker1,inertobj);          \
 LASTACTOR->z = LASTSTAT->z;                              \
 LASTSTAT->flags |= FL_COLORED;                           \
 LASTSTAT->hitpoints = whichteam;                         \
 locspawned[spawnlocindex]=1;                             \
 for(j=0;j<numplayers;j++)                                \
    {if (PLAYERSTATE[j].uniformcolor !=                   \
         TEAM[whichteam].uniformcolor)                    \
        continue;                                         \
                                                          \
     ntilex = newx;                                       \
     ntiley = newy;                                       \
     FindEmptyTile(&ntilex,&ntiley);                      \
     SpawnPlayerobj(ntilex,ntiley,dir,j);                 \
    }                                                     \
}                                                         \


/*
=============
=
= AssignTeams called from SetGameDescription in rt_net.c
=
=============
*/

void AssignTeams(void)
{int i,color;
 int teamforcolor[MAXPLAYERCOLORS];

 numteams = 0;
 if (!gamestate.teamplay)
    return;

 memset(teamforcolor,-1,sizeof(teamforcolor));
 memset(TEAM,0,sizeof(TEAM));

 for(i=0;i<numplayers;i++)
    {color = PLAYERSTATE[i].uniformcolor;
     if (teamforcolor[color] == -1)
        {TEAM[numteams].uniformcolor = color;

         TEAM[numteams].nummembers ++;
         teamforcolor[color] = numteams;
         numteams++;
         if ((gamestate.battlemode == battle_CaptureTheTriad) &&
             (numteams > 2))
           Error("players selected more colors(%d) than Capture the Triad allows",numteams);
        }
     else
        TEAM[teamforcolor[color]].nummembers ++;

     PLAYERSTATE[i].team = teamforcolor[color];

    }

}



/*
=============
=
= SetupTeams
=
=============
*/


void SetupTeams(void)
{

  int i,j,rand,sx,sy,ntilex,ntiley,dir,
      maxdist,currdist,spawnindex,cnt;
  int locspawned[MAXSPAWNLOCATIONS] = {0};

  if (gamestate.battlemode == battle_CaptureTheTriad)
        {rand = (GameRandomNumber("net player spawn",0) % NUMSPAWNLOCATIONS);

         for(i=0;i<NUMSPAWNLOCATIONS;i++)
           {sx = SPAWNLOC[rand].x;
            sy = SPAWNLOC[rand].y;
            dir = SPAWNLOC[rand].dir;

            if (CheckTile(sx,sy) && (!IsPlatform(sx,sy)) &&
               (Number_of_Empty_Tiles_In_Area_Around(sx,sy) > TEAM[0].nummembers)
               )
               {SetupSpecificFlagTeamAt(0,rand);
               break;
               }

            rand = (rand + 1)%NUMSPAWNLOCATIONS;
            }

         if (i == NUMSPAWNLOCATIONS)
            Error("No spawn location available for team 0, capture the flag");

         maxdist = 0x80000000;
         for(i=0;i<NUMSPAWNLOCATIONS;i++)
            {if (locspawned[i])
               continue;

            sx = SPAWNLOC[i].x;
            sy = SPAWNLOC[i].y;
            dir = SPAWNLOC[i].dir;

            if ((Number_of_Empty_Tiles_In_Area_Around(sx,sy) < TEAM[1].nummembers)
               || (!CheckTile(sx,sy) || IsPlatform(sx,sy))
               )
               continue;

            currdist = FindDistance(sx-TEAM[0].tilex,sy-TEAM[0].tiley);
            if (currdist > maxdist)
               {maxdist = currdist;
               spawnindex = i;
               }
            }

         SetupSpecificFlagTeamAt(1,spawnindex);
        }
  else
     {
       int badcount = 0,teamindex;

       if (numteams > NUMSPAWNLOCATIONS)
          Error("More teams than spawn locations !");
       //cnt =0;
       //for(rand = 0;rand < NUMSPAWNLOCATIONS;rand++)
       for(cnt=0;cnt<numteams;)
          {
          rand = (GameRandomNumber("team spawn",0) % NUMSPAWNLOCATIONS);

          if (locspawned[rand])
            continue;


          sx = SPAWNLOC[rand].x;
          sy = SPAWNLOC[rand].y;
          dir = SPAWNLOC[rand].dir;

          if (Number_of_Empty_Tiles_In_Area_Around(sx,sy) < TEAM[cnt].nummembers)
             {
             badcount ++;
             if (badcount == (NUMSPAWNLOCATIONS - cnt))
                Error("\n%s team cannot spawn in this level",colorname[TEAM[cnt].uniformcolor]);
             continue;
             }

          badcount = 0;
          //Debug("\n\nSpawn Location %d",rand);
          //Debug("\n-----------------");
          TEAM[cnt].tilex = sx;
          TEAM[cnt].tiley = sy;
          TEAM[cnt].dir = dir;
          locspawned[rand]=1;
          cnt++;

          }

       for(j=0;j<numplayers;j++)
          {
          teamindex = PLAYERSTATE[j].team;

          sx = TEAM[teamindex].tilex;
          sy = TEAM[teamindex].tiley;
          dir = TEAM[teamindex].dir;

          FindEmptyTile(&sx,&sy);

         //Debug("\n x: %3d, y: %3d",sx,sy);
          SpawnPlayerobj(sx,sy,dir,j);
          }


     }




//numplayers = 1;
//Error("Okay");
#if ((DEVELOPMENT == 1))
#if (TEAMTEST == 1)

  Debug("Team Spawn Location\n");
  Debug("-------------------\n");
  for(i=0;i<numteams;i++)
    Debug("%d   %3d,%3d\n",i,TEAM[i].tilex,TEAM[i].tiley);


  Debug("Player            Team          Location\n");
  Debug("------            ----          --------\n");
  for(i=0;i<numplayers;i++)
    Debug("  %d             %d            %3d,%3d\n",i,PLAYERSTATE[i].team,PLAYER[i]->tilex,PLAYER[i]->tiley);

//  Error("done");
#endif
#endif

}


/*
==================
=
= SetupPlayers
=
==================
*/

void SetupPlayers( void )
{
	int   i,j;
	word   *map,tile;

	//START in icon plane = 10

	map = mapplanes[1];
	for(j=0;j<mapheight;j++)
		for(i=0;i<mapwidth;i++)
			{
			tile = *map++;
			switch (tile)
				{
				case 19:
				case 20:
				case 21:
				case 22:
					FIRST.x = i;
					FIRST.y = j;
					FIRST.dir = tile-19;
					SPAWNLOC[NUMSPAWNLOCATIONS].x  = i;
					SPAWNLOC[NUMSPAWNLOCATIONS].y  = j;
					SPAWNLOC[NUMSPAWNLOCATIONS].dir  = tile-19;
					if (NUMSPAWNLOCATIONS<MAXSPAWNLOCATIONS)
						NUMSPAWNLOCATIONS ++;
					break;

				case 274:
				case 275:
				case 276:
				case 277:
					SPAWNLOC[NUMSPAWNLOCATIONS].x  = i;
					SPAWNLOC[NUMSPAWNLOCATIONS].y  = j;
					SPAWNLOC[NUMSPAWNLOCATIONS].dir  = tile-274;
					if (NUMSPAWNLOCATIONS<MAXSPAWNLOCATIONS)
						NUMSPAWNLOCATIONS ++;
					break;
				}

			}

   if ( NUMSPAWNLOCATIONS <= 0 )
      {
      Error( "No spawn locations found on map." );
      }

   /*modemgame=true;
   gamestate.teamplay = true;
   PLAYERSTATE[0].uniformcolor = 2;
   PLAYERSTATE[1].uniformcolor = 2;
   numplayers = 2;
   AssignTeams();*/

   if (!BATTLEMODE)
		{
		if (tedlevel)
			{
			if ((tedx==0) || (tedy == 0))
				SpawnPlayerobj (FIRST.x, FIRST.y, FIRST.dir,0);
			else
				SpawnPlayerobj(tedx,tedy,FIRST.dir,0);
			}
		else
			SpawnPlayerobj(FIRST.x,FIRST.y,FIRST.dir,0);
		}

   else if (gamestate.teamplay == true)
	  SetupTeams();

	else
      {
      int rand,cnt,locspawned[MAXSPAWNLOCATIONS]={0};
      int locsleft;

      locsleft=NUMSPAWNLOCATIONS;
      for(cnt=0;cnt<numplayers;)
         {rand = (GameRandomNumber("net player spawn",0) % NUMSPAWNLOCATIONS);
         if (locsleft==0)
            {
            int x,y;

            x=SPAWNLOC[rand].x;
            y=SPAWNLOC[rand].y;
            FindEmptyTile(&x,&y);
            SpawnPlayerobj(x,y,SPAWNLOC[rand].dir,cnt);
            cnt++;
            }
         else if (!locspawned[rand])
            {SpawnPlayerobj(SPAWNLOC[rand].x,SPAWNLOC[rand].y,SPAWNLOC[rand].dir,cnt);
            locspawned[rand]=1;
            locsleft--;
            cnt++;
            }
         }
		}

	if (gamestate.battlemode == battle_Tag)
	  {int i;
		playertype *pstate;

      BATTLE_It = GameRandomNumber("tag choose",0) % numplayers;

		PLAYER[BATTLE_It]->flags |= FL_DESIGNATED;
		for(i=0;i<numplayers;i++)
		  {M_LINKSTATE(PLAYER[i],pstate);

			if (i == BATTLE_It)
			  {pstate->missileweapon = pstate->oldweapon = pstate->new_weapon =
				pstate->oldmissileweapon = pstate->weapon = wp_godhand;
				pstate->bulletweapon = -1;
			  }
			else
			  {pstate->missileweapon = pstate->oldweapon = pstate->new_weapon =
				pstate->oldmissileweapon = pstate->bulletweapon = pstate->weapon = -1;
			  }
		  }
	  }

	PreCachePlayers();
}



/*
==================
=
= SetupMaskedWalls
=
==================
*/

void SetupMaskedWalls( void )
{
	int   i,j;
	word   *map,tile;

	map = mapplanes[0];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
	      {
         tile = *map++;
		   if ((tile >= 158) && (tile <= 160)) // Multi glassed walls
            {
            SpawnMaskedWall(i,j,mw_multi1+(tile-158),MW_MULTI|MW_BLOCKING|MW_BLOCKINGCHANGES|MW_SHOOTABLE);
            }
	      else if ((tile >= 176) && (tile <= 178)) // Multi shot out glassed walls
            {
            SpawnMaskedWall(i,j,mw_multi1+(tile-176),MW_BOTTOMPASSABLE);
            }
		   else if ((tile >= 162) && (tile <= 171))
            {
            switch (tile)
               {
               case 162:
                  SpawnMaskedWall(i,j,mw_normal1,MW_SHOOTABLE|MW_BLOCKING);
                  break;
               case 163:
                  SpawnMaskedWall(i,j,mw_normal1,MW_BLOCKING);
                  break;
               case 164:
                  SpawnMaskedWall(i,j,mw_normal2,MW_SHOOTABLE|MW_BLOCKING);
                  break;
               case 165:
                  SpawnMaskedWall(i,j,mw_normal2,MW_BLOCKING);
                  break;
               case 166:
                  SpawnMaskedWall(i,j,mw_normal3,MW_SHOOTABLE|MW_BLOCKING);
                  break;
               case 167:
                  SpawnMaskedWall(i,j,mw_normal3,MW_BLOCKING);
                  break;
               case 168:
                  SpawnMaskedWall(i,j,mw_singlepane,MW_SHOOTABLE|MW_BLOCKINGCHANGES|MW_BLOCKING);
                  break;
               case 169:
                  SpawnMaskedWall(i,j,mw_singlepane,MW_BOTTOMPASSABLE);
                  break;
               case 170:
                  SpawnMaskedWall(i,j,mw_dogwall,MW_NONDOGBLOCKING|MW_WEAPONBLOCKING);
                  break;
               case 171:
                  SpawnMaskedWall(i,j,mw_peephole,MW_WEAPONBLOCKING|MW_BLOCKING);
                  break;
               }
            }
         else if (tile == 172)
            SpawnMaskedWall(i,j,mw_exitarch,MW_BOTTOMPASSABLE);
         else if (tile == 173)
            SpawnMaskedWall(i,j,mw_secretexitarch,MW_BOTTOMPASSABLE);
         else if (tile == 174) // entry gate
            SpawnMaskedWall(i,j,mw_entrygate,MW_BLOCKING);
		   else if (tile == 157) // hi switch off
            SpawnMaskedWall(i,j,mw_hiswitchoff,MW_BLOCKING);
		   else if (tile == 175) // hi switch on
            SpawnMaskedWall(i,j,mw_hiswitchon,MW_BLOCKING|MW_SWITCHON);
		   else if (tile == 179) // railing;
            SpawnMaskedWall(i,j,mw_railing,MW_ABOVEPASSABLE|MW_MIDDLEPASSABLE);
//         else if (tile == 161) // pillar
//            SpawnMaskedWall(i,j,mw_pillar,MW_BLOCKING);
	      }
      }
	for (j=0;j<mapheight;j++)
	   for(i=0;i<mapwidth;i++)
			{
		   if (IsPlatform(i,j)) // tall platform in icon plane
            {
            if ((MAPSPOT(i,j,0)-AREATILE>=0) || (MAPSPOT(i,j,0)==21))
               // check to see we are not on a wall
               {
               int which;

               which=MAPSPOT(i,j,2)-4;
               switch (which)
                  {
                  case 0:
                     SpawnMaskedWall(i,j,mw_platform1,MW_BOTTOMPASSABLE|MW_MIDDLEPASSABLE);
                     break;
                  case 1:
                     SpawnMaskedWall(i,j,mw_platform2,MW_ABOVEPASSABLE|MW_MIDDLEPASSABLE);
                     break;
                  case 2:
                     SpawnMaskedWall(i,j,mw_platform3,MW_MIDDLEPASSABLE);
                     break;
                  case 3:
                     SpawnMaskedWall(i,j,mw_platform4,MW_BOTTOMPASSABLE);
                     break;
                  case 4:
                     SpawnMaskedWall(i,j,mw_platform5,MW_BOTTOMPASSABLE|MW_ABOVEPASSABLE);
                     break;
                  case 5:
                     SpawnMaskedWall(i,j,mw_platform6,MW_ABOVEPASSABLE);
                     break;
                  case -3:
                     SpawnMaskedWall(i,j,mw_platform7,MW_ABOVEPASSABLE);
                     break;
                  default:
                     Error ("Illegal Maskedwall platform value at x=%d y=%d\n",i,j);
                     break;
						}
#if 0
               if (IsPlatform(i+1,j))
                  {
                  if ( (IsPlatform(i,j+1)) || (IsPlatform(i,j-1)) )
			            SpawnStatic(i,j,83,MAPSPOT(i,j,2));
                  }
               else if (IsPlatform(i-1,j))
                  {
                  if ( (IsPlatform(i,j+1)) || (IsPlatform(i,j-1)) )
			            SpawnStatic(i,j,83,MAPSPOT(i,j,2));
                  }
#endif
               }
            else
               Error("You have what appears to be a platform ontop\n a wall at x=%d y=%d\n",i,j);
            }
         }
}

/*
int GetAreaNumber ( int tilex, int tiley, int dir );
void RemoveDangerWalls
   (
   void
   )

   {
   int   i;
   int   j;
   word *map;
   word  tile;

   map = mapplanes[ 1 ];

   for( j = 0; j < mapheight; j++ )
      {
      for( i = 0; i < mapwidth; i++ )
	      {
         tile = *map++;
         switch( tile )
		      {
            case 256:
		      case 257:
		      case 258:
		      case 259:
               if ( MAPSPOT( i, j, 2 ) == 0 )
                  {
                  MAPSPOT( i, j, 0 ) = ( word )( GetAreaNumber( i, j,
                     ( tile - 256 ) << 1 ) + AREATILE );
                  MAPSPOT( i, j, 1 ) = 0;
                  }
		         break;

            case 300:
		      case 318:
		      case 336:
		      case 354:
               if ( MAPSPOT( i, j, 2 ) == 0 )
                  {
                  MAPSPOT( i, j, 0 ) = ( word )( GetAreaNumber( i, j,
                     ( ( tile - 300 ) / 9 ) + AREATILE ) );
                  MAPSPOT( i, j, 1 ) = 0;
                  }
		         break;
            }
         }
      }
   }
*/


/*
==================
=
= SetupPushWalls
=
==================
*/

void SetupPushWalls( void )
{
	int   i,j;
	word   *map,tile;
	int   temp;

   map = mapplanes[1];
   for(j=0;j<mapheight;j++)
      {
    	for(i=0;i<mapwidth;i++)
	      {
         tile = *map++;
	      switch(tile)
		      {
            case 72:
            case 73:
            case 74:
            case 75:
            case 76:
            case 77:
            case 78:
            case 79:
					if (tilemap[i][j] && ActorIsWall(i,j))
		            {
                  temp=tilemap[i][j]&0x1fff;
			         tilemap[i][j] = pwallnum;
			         if (MAPSPOT(i,j,2))
			            SpawnPushWall(i,j,1,temp,tile-72,0);
			         else
			            SpawnPushWall(i,j,0,temp,tile-72,0);
			         }
		         break;

		      case 80: //OldPushWall
		         if (tilemap[i][j])
		            {
                  temp=tilemap[i][j]&0x1fff;
			         tilemap[i][j] = pwallnum;
			         if (MAPSPOT(i,j,2))
                     Error("You cannot link a pushwall which has no direction associated\n with it at x=%d y=%d\n",i,j);
						else
			            SpawnPushWall(i,j,0,temp,nodir,0);
			         }
			      break;


            case 256:
		      case 257:
		      case 258:
		      case 259:
               if (tilemap[i][j])
			         {
                  temp=tilemap[i][j]&0x1fff;
			         tilemap[i][j] = pwallnum;
			         if (MAPSPOT(i,j,2))
			            SpawnPushWall(i,j,0,temp,(tile-256)<<1,2);
                  else
			            SpawnPushWall(i,j,0,temp,(tile-256)<<1,4);
                  }
               else
                  Error("You have to place a turbomovewall icon on a wall at x=%d y=%d",i,j);
		         break;

            case 300:
		      case 318:
		      case 336:
		      case 354:
		         if (tilemap[i][j])
			         {
                  temp=tilemap[i][j]&0x1fff;
			         tilemap[i][j] = pwallnum;
			         if (MAPSPOT(i,j,2))
							SpawnPushWall(i,j,0,temp,(tile-300)/9,1);
                  else
  	                  SpawnPushWall(i,j,0,temp,(tile-300)/9,3);
                  }
               else
                  Error("You have to place a movewall icon on a wall at x=%d y=%d",i,j);
		         break;
            }
         }
      }
}


/*
==================
=
= GetPushWallNumber
=
==================
*/

int GetPushWallNumber( int tx, int ty )
{
   int i;

   for (i=0;i<pwallnum;i++)
      if ( (pwallobjlist[i]->tilex==tx) && (pwallobjlist[i]->tiley==ty))
         return i;
   Error ("Could not find a push wall at x=%d y=%d\n",tx,ty);
   return -1;
}


/*
==================
=
= SetupPushWallLinks
=
==================
*/
void SetupPushWallLinks( void )
{
	int   i,j;
	word   *map,tile;
   word  touchx,touchy;

   map = mapplanes[1];
   for(j=0;j<mapheight;j++)
      {
    	for(i=0;i<mapwidth;i++)
	      {
         tile = *map++;
	      switch(tile)
		      {
            case 72:
            case 73:
            case 74:
            case 75:
            case 76:
            case 77:
            case 78:
            case 79:
		         if (ActorIsPushWall(i,j))
		            {
			         if (MAPSPOT(i,j,2))
			            {
                     touchx = (word) ((MAPSPOT(i,j,2) >> 8) & 0xff);
							touchy = (word) ((MAPSPOT(i,j,2) >> 0) & 0xff);
				         if (touchindices[touchx][touchy])
                        {
                        if (MAPSPOT(i,j+1,2)!=0)
                           {
#if (DEVELOPMENT == 1)
                           SoftError("MAPWARNING:You left a delay for a linked push wall under the pushwall\n at x=%ld y=%ld\n",i,j);
#endif
                           }
								Link_To_Touchplate(touchx,touchy,ActivatePushWall,NULL,GetPushWallNumber(i,j),0);
                        }
				         else {
                        printf("Plane 0: %d, Plane 1: %d, Plane 2: %d\n", mapplanes[0][touchy*mapwidth+touchx], mapplanes[1][touchy*mapwidth+touchx], mapplanes[2][touchy*mapwidth+touchx]);
				            Error("tried to link a pushwall at x=%d y=%d to a non-existent touchplate at x=%d y=%d\n",i,j,touchx,touchy);
                        }
			            }
			         }
		         break;

            case 80:
		         if (ActorIsPushWall(i,j))
		            {
			         if (MAPSPOT(i,j,2))
			            {
                     Error("You shouldn't be linking a nondirectional-push wall at x=%d y=%d\n",i,j);
                     }
                  }
               break;
            case 256:
		      case 257:
		      case 258:
		      case 259:
		         if (ActorIsPushWall(i,j))
			         {
						if (MAPSPOT(i,j,2))
			            {
                     touchx = (word) ((MAPSPOT(i,j,2) >> 8) & 0xff);
				         touchy = (word) ((MAPSPOT(i,j,2) >> 0) & 0xff);
				         if (touchindices[touchx][touchy])
                        {
                        if (MAPSPOT(i,j+1,2)!=0)
                           {
#if (DEVELOPMENT == 1)
                           SoftError("MAPWARNING:You left a delay for a linked push wall under the pushwall\n at x=%ld y=%ld\n",i,j);
#endif
                           }
								Link_To_Touchplate(touchx,touchy,ActivateMoveWall,NULL,GetPushWallNumber(i,j),0);
                        }
				         else
				            Error("tried to link a turbomovewall at x=%d y=%d to a non-existent touchplate\n",i,j);
                     }
                  }
		         break;

            case 300:
		      case 318:
		      case 336:
		      case 354:
		         if (ActorIsPushWall(i,j))
			         {
			         if (MAPSPOT(i,j,2))
			            {
                     touchx = (word) ((MAPSPOT(i,j,2) >> 8) & 0xff);
				         touchy = (word) ((MAPSPOT(i,j,2) >> 0) & 0xff);
				         if (touchindices[touchx][touchy])
                        {
								if (MAPSPOT(i,j+1,2)!=0)
                           {
#if (DEVELOPMENT == 1)
                           SoftError("MAPWARNING:You left a delay for a linked push wall under the pushwall\n at x=%ld y=%ld\n",i,j);
#endif
                           }
								Link_To_Touchplate(touchx,touchy,ActivateMoveWall,NULL,GetPushWallNumber(i,j),0);
                        }
				         else
				            Error("tried to link a movewall at x=%d y=%d to a non-existent touchplate\n",i,j);
                     }
                  }
		         break;
            }
         }
      }
}

/*
=================
=
= SetupElevators
=
=================
*/

void SetupElevators (void)
{
	int j, i,x,y;
	word *map;
	word tile;
	elevator_t *elev;
	doorobj_t* dptr;

	map = mapplanes[1];
	map += 4 ;

	for (j = 0; j < mapheight; j++)
	{
		for (i = 0; i < mapwidth; i++)
		  {
            if (j == 0 && i < 4) continue;
            tile = *map++;

			if ((tile > 89) && (tile < 98))
				{elev = &ELEVATOR[tile-90];
				 if (!elev->sx)
					{elev->sx = i;
					 elev->sy = j;
					 elev->doortoopen = -1;
					 elev->doorclosing = -1;
					 elev->nextaction = -1;
					 _numelevators ++;
					}
				 else
					{elev->dx = i;
					 elev->dy = j;
					}
				}
		  }
	  }

	if (_numelevators && (!ELEVATOR[0].sx))
	 Error("Elevators must start at 1, dumb ass.");

	for(i=0;i<_numelevators;i++)
	  {elev = &ELEVATOR[i];
		x = elev->sx;
		y = elev->sy;
		for(j=0;j<doornum;j++)
		  {dptr = doorobjlist[j];
			if (((dptr->tilex == (x+1)) && (dptr->tiley == y)) ||
				 ((dptr->tilex == (x-1)) && (dptr->tiley == y)) ||
				 ((dptr->tilex == x) && (dptr->tiley == (y+1))) ||
				 ((dptr->tilex == x) && (dptr->tiley == (y-1))))
			  {elev->door1 = j;
				dptr->eindex = i;
				if ((dptr->tilex == (x+1)) && (dptr->tiley == y))
				  {elev->esx = x-1;
					elev->esy = y;
				  }
				else if ((dptr->tilex == (x-1)) && (dptr->tiley == y))
				  {elev->esx = x+1;
					elev->esy = y;
				  }
				else if ((dptr->tilex == x) && (dptr->tiley == (y+1)))
				  {elev->esx = x;
					elev->esy = y-1;
				  }
				else if ((dptr->tilex == x) && (dptr->tiley == (y-1)))
				  {elev->esx = x;
					elev->esy = y+1;
				  }
				break;
			  }
		  }

		x = elev->dx;
		y = elev->dy;
		for(j=0;j<doornum;j++)
		  {dptr = doorobjlist[j];
			if (((dptr->tilex == (x+1)) && (dptr->tiley == y)) ||
				 ((dptr->tilex == (x-1)) && (dptr->tiley == y)) ||
				 ((dptr->tilex == x) && (dptr->tiley == (y+1))) ||
				 ((dptr->tilex == x) && (dptr->tiley == (y-1))))
			  {elev->door2 = j;
				dptr->eindex = i;
				dptr->flags |= DF_ELEVLOCKED;
				if ((dptr->tilex == (x+1)) && (dptr->tiley == y))
				  {elev->edx = x-1;
					elev->edy = y;
				  }
				else if ((dptr->tilex == (x-1)) && (dptr->tiley == y))
				  {elev->edx = x+1;
					elev->edy = y;
				  }
				else if ((dptr->tilex == x) && (dptr->tiley == (y+1)))
				  {elev->edx = x;
					elev->edy = y-1;
				  }
				else if ((dptr->tilex == x) && (dptr->tiley == (y-1)))
				  {elev->edx = x;
					elev->edy = y+1;
				  }
				break;
			  }

		  }
	  }
  #if ((DEVELOPMENT == 1))
  #if ((ELEVATORTEST == 1))
	 for(i=0;i<_numelevators;i++)
		Debug("\nelevator %d door1 %2d, door2 %2d",i,ELEVATOR[i].door1,ELEVATOR[i].door2);
  #endif
  #endif
}


/*
=================
=
= SetupDoors
=
=================
*/

void SetupDoors (void)
{
   int j, i;
   word *map;
	word tile;
   byte locked;

   map = mapplanes[0];

   for (j = 0; j < mapheight; j++)
      for (i = 0; i < mapwidth; i++)
      {
         tile = *map++;

         if ((tile >= 33) && (tile <= 35))
         {
            tilemap[i][j] = doornum;

            locked=0;
            if (MAPSPOT (i, j, 2))
               locked = 5;

            SpawnDoor (i, j, locked, (tile-33)+15);
         }

         else if ((tile > 89) && (tile < 94))
         {
            tilemap[i][j] = doornum;

            locked = 0;
            if (MAPSPOT (i, j, 2))
               locked = 5;

            SpawnDoor (i, j, locked, tile-90);
         }

         else if ((tile > 93) && (tile < 98))
         {
            Error("locked door %d being used at %d,%d",tile,i,j);
         }


         else if ((tile > 97) && (tile < 105))
         {
            tilemap[i][j] = doornum;

            locked = 0;
            if (MAPSPOT (i, j, 2))
               locked = 5;

            SpawnDoor (i, j, locked, tile-90);
         }
         else if ((tile >= 154) && (tile <= 156))
         {
            tilemap[i][j] = doornum;

            locked=0;
            if (MAPSPOT (i, j, 2))
               locked = 5;

            SpawnDoor (i, j, locked, (tile-154)+18);
         }
      }
}

/*
==================
=
= GetDoorNumber
=
==================
*/

int GetDoorNumber( int tx, int ty )
{
   int i;

   for (i=0;i<doornum;i++)
      if ( (doorobjlist[i]->tilex==tx) && (doorobjlist[i]->tiley==ty))
         return i;
   Error ("Could not find a door at x=%d y=%d\n",tx,ty);
   return -1;
}

/*
=================
=
= SetupDoorLinks
=
=================
*/

void SetupDoorLinks (void)
{
   int  j,
        i,
        k;
   word *map;
   int  clocklinked;
   int  clockx,clocky;
   int  doornumber;
	word touchx,
        tile,
		  touchy;

   map = mapplanes[0];

   for (j = 0; j < mapheight; j++)
      for (i = 0; i < mapwidth; i++)
      {
         tile = *map++;

         if (MAPSPOT (i, j, 2))
         {
            if (IsDoor(i,j)==1)
            {
               clocklinked = 0;

               doornumber=GetDoorNumber(i,j);

               for (k = 0; k < numclocks; k++)
               {
                  clockx = Clocks[k].points_to_tilex;
                  clocky = Clocks[k].points_to_tiley;

                  if ((clockx == i) && (clocky == j))
                  {
                     clocklinked = 1;
                     ClockLink (LinkedOpenDoor, LinkedCloseDoor, doornumber, k);
                     doorobjlist[doornumber]->lock   = 5;
                     doorobjlist[doornumber]->flags |= DF_TIMED;
                  }
               }

               if (!clocklinked)
					{
                  touchx = (word) ((MAPSPOT (i, j, 2) >> 8) & 0xff);
                  touchy = (word) ((MAPSPOT (i, j, 2) >> 0) & 0xff);

                  if (touchindices[touchx][touchy])
                  {
							if (MAPSPOT (i, j, 1) == 192)
								Link_To_Touchplate (touchx, touchy, LinkedCloseDoor,
                                            LinkedCloseDoor, doornumber, 0);
							else
								Link_To_Touchplate (touchx, touchy, LinkedOpenDoor,
                                            LinkedOpenDoor, doornumber, 0);
                  }
                  else
                     Error ("tried to link a door at x=%d y=%d to a non-existent touchplate",i,j);
               }
            }
         }
      }
}


/*
=================
=
= FindTimeTile
=
=================
*/
void FindTimeTile ( int * x, int * y )
{
  int xx,yy;

  xx=*x;
  yy=*y;

  if (!(tilemap[xx+1][yy]) && MAPSPOT(xx+1,yy,2))
     {
     *x=xx+1;
     return;
     }
  if (!(tilemap[xx-1][yy]) && MAPSPOT(xx-1,yy,2))
     {
     *x=xx-1;
     return;
     }
  if (!(tilemap[xx][yy+1]) && MAPSPOT(xx,yy+1,2))
     {
     *y=yy+1;
     return;
     }
  if (!(tilemap[xx][yy-1]) && MAPSPOT(xx,yy-1,2))
     {
     *y=yy-1;
     return;
     }
  Error ("Could not find an end time for a clock linked item\nat x=%d y=%d\n",*x,*y);
}



/*
=================
=
= SetupClocks
=
=================
*/

void SetupClocks (void)
{
   int  i,
        j,
        minutes,
        seconds;
   word *map,
        tile,
        mapx,
        mapy;
   int  endtimex,
        endtimey;


   map  = mapplanes[1];
	map += 4 ;

   for (j = 0; j < mapheight; j++)
   {
      for (i = 0; i < mapwidth; i++)
      {
         if (j == 0 && i < 4) continue;
			tile = *map++;

         if (tile == 121)
         {
            mapx = (word) ((MAPSPOT (i, j, 2) >> 8) & 0xff);
            mapy = (word) ((MAPSPOT (i, j, 2) >> 0) & 0xff);

            minutes = (int) ((MAPSPOT (mapx, mapy, 2) >> 8) & 0xff);
            seconds = (int) ((MAPSPOT (mapx, mapy, 2) >> 0) & 0xff);

            if (seconds > 0x59)
               Error ("seconds of clock time must be below 0x5a (60 secs) ");

            seconds = 10 * (seconds/16) + (seconds % 16);
            minutes = 60 * (10*(minutes/16) + (minutes % 16));

            // total seconds
            Clocks[numclocks].time1 = VBLCOUNTER*(seconds + minutes);

            endtimex=mapx;
            endtimey=mapy;

            FindTimeTile (&endtimex, &endtimey);

            minutes = (int) ((MAPSPOT (endtimex, endtimey, 2) >> 8) & 0xff);
            seconds = (int) ((MAPSPOT (endtimex, endtimey, 2) >> 0) & 0xff);

            if (seconds > 0x59)
                  Error("seconds of clock time must be below 0x5a (60 secs)");

            seconds = 10 * (seconds/16) + (seconds % 16);
            minutes = 60 * (10*(minutes/16) + (minutes % 16));

            // total seconds
            Clocks[numclocks].time2 = VBLCOUNTER * (seconds + minutes);
            Clocks[numclocks].points_to_tilex = mapx;
            Clocks[numclocks].points_to_tiley = mapy;
            Clocks[numclocks].linkindex       = lasttouch;

            numclocks ++;

				// clocks treated as virtual touchplates
				lasttouch ++;
			}
		}
	}
}


/*
=================
=
= LinkElevatorDiskGroups
=
=================
*/

void LinkElevatorDiskGroups(void)
   {
   objtype *diskfinder1,*temp,*master;
   int maxplatformheight[30]={-1};
   int num_distinct_max_heights=0;
   int i;
   boolean newdiskheight;


 #define M_ISELEVDISK(actor) \
    ((actor->obclass == diskobj) && (actor->state == &s_elevdisk))



   for(diskfinder1 = FIRSTACTOR;diskfinder1;diskfinder1 = diskfinder1->next)
      {
      if (!M_ISELEVDISK(diskfinder1))
         continue;

      newdiskheight = true;
      for(i=0;i<num_distinct_max_heights;i++)
         {
         if (maxplatformheight[i] == diskfinder1->temp2)
            {
            newdiskheight = false;
            break;
            }
         }

      if (newdiskheight == true)
         maxplatformheight[num_distinct_max_heights++] = diskfinder1->temp2;

      }


   for(i=0;i<num_distinct_max_heights;i++)
      {

      SpawnDisk(64,64,0,true);
      master = new;
      master->temp2 = maxplatformheight[i];

      for(temp = FIRSTACTOR;temp;temp = temp->next)
         {
         if (temp == master)
            continue;

         if (!M_ISELEVDISK(temp))
            continue;

         if (temp->temp2 != maxplatformheight[i])
            continue;

         temp->target = master;
         SetTilePosition(master,temp->tilex,temp->tiley);
         master->areanumber=AREANUMBER(master->tilex,master->tiley);

         }
      master->flags |= FL_ABP;
      MakeActive(master);
      }


   }


/*
=================
=
= LinkActor
=
=================
*/


void LinkActor (objtype *ob,int tilex,int tiley,
                void (*action)(long),void (*swapaction)(long)
               )
   {
	word  touchx,touchy;
	int   clockx,clocky;
   int   clocklinked,k;
	wall_t * tswitch;


   clocklinked = 0;
   for(k=0;k<numclocks;k++)
      {
      clockx = Clocks[k].points_to_tilex;
      clocky = Clocks[k].points_to_tiley;
      if ((clockx == tilex) && (clocky == tiley))
         {
         clocklinked = 1;
         ClockLink(EnableObject,DisableObject,(long)ob,k);
         }
      }

   if (!clocklinked)
      {
      touchx = (word) ((MAPSPOT(tilex,tiley,2) >> 8) & 0xff);
      touchy = (word) ((MAPSPOT(tilex,tiley,2) >> 0) & 0xff);
      if ((MISCVARS->TOMLOC.x == touchx) && (MISCVARS->TOMLOC.y == touchy))
         {
         objtype *tom = (objtype*)actorat[touchx][touchy];
         tom->whatever = ob;
         }

      else if (touchindices[touchx][touchy])
         {
         tswitch = (wall_t*) actorat[touchx][touchy];

         if (tswitch && (ob->obclass == wallfireobj))
            {
            tswitch->flags |= FL_REVERSIBLE;
            if (tswitch->flags & FL_ON)
               ob->flags |= FL_ACTIVE;
            }


         if (tswitch && (tswitch->flags & FL_ON))
            Link_To_Touchplate(touchx,touchy,swapaction,action,(long)ob,0);
         else
            Link_To_Touchplate(touchx,touchy,action,swapaction,(long)ob,0);
         if (ob->obclass == gasgrateobj)
            {
            ob->temp1 = touchx;
            ob->temp2 = touchy;
            }
         }
      else
         Error("tried to link an object at x=%d y=%d to a non-existent touchplate supposedly at x=%d y=%d",tilex,tiley,touchx,touchy);
      }

   if (tilemap[tilex][tiley])
      (MAPSPOT(tilex,tiley,2))=21;
   }




/*
======================
=
= SetupInanimateActors
=
======================
*/


void SetupInanimateActors (void)
   {
   int   i,j,linked;
	word   *map,tile;
	void (*action)(long),(*swapaction)(long);


   map = mapplanes[1];


   // non-linked, harmless inanimate actors
   for(j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
         {
         tile = *map++;

         switch(tile)
            {

            case 193:
               SpawnSpring(i,j);
               break;

#if 0
            case 460:
//               if ( gamestate.Product != ROTT_SHAREWARE )
                  {
                  SpawnNewObj(i,j,&s_wind,inertobj);
                  }
               break;
#endif

            case 462:
            case 463:
            case 464:
            case 465:
            case 466:

               SpawnDisk(i,j,tile-462,false);
               break;

            case 285:
            case 286:
            case 287:
               SpawnPushColumn(i,j,tile-285,nodir,0);
               break;
            }
         }
      }


   // linked, harmless actors
   map = mapplanes[1];
   for(j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
         {
         tile = *map++;
         action = EnableObject;
         swapaction = DisableObject;
         linked = (MAPSPOT(i,j,2) && (!IsPlatform(i,j)));


         switch(tile)
            {

            case 140:
            case 141:
            case 142:
            case 143:

               if ((!BATTLEMODE) || (gamestate.BattleOptions.SpawnDangers))
                  {
                  PreCacheActor(wallfireobj,0);
                  SpawnWallfire(i,j,tile-140);
                  if (!linked)
                     {
                     new->flags |= FL_ACTIVE;
                     if (tilemap[i][j])
                        MAPSPOT(i,j,2) = 21;
                     }
                  else
                     LinkActor(new,i,j,action,swapaction);
                  }
               else if (tilemap[i][j])
                  MAPSPOT(i,j,2) = 21;

               break;





            case 303:
            case 304:
            case 305:
               SpawnPushColumn(i,j,tile-303,east,linked);
               swapaction = NULL;
               if (linked)
                  LinkActor(new,i,j,action,swapaction);

               break;

            case 321:
            case 322:
            case 323:
               SpawnPushColumn(i,j,tile-321,north,linked);
               swapaction = NULL;
               if (linked)
                  LinkActor(new,i,j,action,swapaction);

               break;

            case 339:
            case 340:
            case 341:
               SpawnPushColumn(i,j,tile-339,west,linked);
               swapaction = NULL;
               if (linked)
                  LinkActor(new,i,j,action,swapaction);

               break;

            case 357:
            case 358:
            case 359:
               SpawnPushColumn(i,j,tile-357,south,linked);
               swapaction = NULL;
               if (linked)
                  LinkActor(new,i,j,action,swapaction);

               break;


            }

         }
      }

   //harmful actors

   if ((!BATTLEMODE) || (gamestate.BattleOptions.SpawnDangers))
      {
      map = mapplanes[1];
      for(j=0;j<mapheight;j++)
         {
         for(i=0;i<mapwidth;i++)
            {
            tile = *map++;
            action = EnableObject;
            swapaction = DisableObject;
            linked = (MAPSPOT(i,j,2) && (!IsPlatform(i,j)));

            switch(tile)
               {
               case 89:
                  SpawnFourWayGun(i,j);
                  break;


               case 156:
               case 157:
                  SpawnBlade(i,j,nodir,0,tile-156);
                  if (linked)
                     LinkActor(new,i,j,action,swapaction);

                  break;

               case 174:
               case 175:
                  SpawnBlade(i,j,nodir,1,tile-174);
                  if (linked)
                     LinkActor(new,i,j,action,swapaction);

                  break;



               case 412:
                  SpawnSpear(i,j,0);
                  break;

               case 430:
                  SpawnSpear(i,j,1);
                  break;

               case 413:
                  SpawnCrushingColumn(i,j,1); //down
                  break;

               case 431:
                  SpawnCrushingColumn(i,j,0); // up
                  break;


               case 192:
                  if (!tilemap[i][j])
                     {
                     SpawnNewObj(i,j,&s_gas1,gasgrateobj);
                     PreCacheActor(gasgrateobj,0);
                     new->flags |= FL_ABP;
                     MakeActive(new);
                     swapaction = NULL;
                     if (linked)
                        LinkActor(new,i,j,action,swapaction);

                     }
                  break;


               case 301:
               case 302:
                  SpawnBlade(i,j,east,tile-301,0);
                  if (!linked)
                     new->flags |= FL_ACTIVE;
                  else
                     LinkActor(new,i,j,action,swapaction);

                  break;

               case 319:
               case 320:
                  SpawnBlade(i,j,north,tile-319,0);
                  if (!linked)
                     new->flags |= FL_ACTIVE;
                  else
                     LinkActor(new,i,j,action,swapaction);

                  break;

               case 337:
               case 338:
                  SpawnBlade(i,j,west,tile-337,0);
                  if (!linked)
                     new->flags |= FL_ACTIVE;
                  else
                     LinkActor(new,i,j,action,swapaction);

                  break;

               case 355:
               case 356:
                  SpawnBlade(i,j,south,tile-355,0);
                  if (!linked)
                     new->flags |= FL_ACTIVE;
                  else
                     LinkActor(new,i,j,action,swapaction);

                  break;

               case 372:
                  SpawnFirejet(i,j,nodir,0);
                  break;

               case 373:
               case 374:
               case 375:
               case 376:
                  SpawnFirejet(i,j,tile-373,0);
                  break;

               case 390:
                  SpawnFirejet(i,j,nodir,1);
                  break;

               case 391:
               case 392:
               case 393:
               case 394:
                  SpawnFirejet(i,j,tile-391,1);
                  break;

               case 278:
               case 279:
               case 280:
               case 281:
                  SpawnBoulder(i,j,tile-278);
                  if (!linked)
                     new->flags |= FL_ACTIVE;
                  else
                     LinkActor(new,i,j,action,swapaction);

                  break;

               }


            }
         }
      }

   LinkElevatorDiskGroups();

   }


/*
===================
=
= FixTiles
=
===================
*/

void FixTiles(void)
   {
   word *map,tile;
   int i,j;

   map = mapplanes[1];
   for(j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
         {
         tile = *map++;
         switch(tile)
            {
            case 140:
            case 141:
            case 142:
            case 143:
            case 192:
            case 278:
            case 279:
            case 280:
            case 281:
               if (tilemap[i][j])
                  (MAPSPOT(i,j,2))=21;
               break;
            }
         }
      }

   }


void Illuminate(void)
{statobj_t*temp;

 if (lightsource==0)
    return;
 for(temp=FIRSTSTAT;temp;temp=temp->statnext)
	if (temp->flags & FL_LIGHTON)
	  TurnOnLight(temp->tilex,temp->tiley);
}


/*
=================
=
= SetupLights
=
=================
*/
void SetupLights(void)
{
	int i,j,touchx,touchy;
	wall_t *tswitch;
	word *map,tile;

// Initialize Lights in Area

	memset(LightsInArea,0,(NUMAREAS + 1) * sizeof(int));

	map = mapplanes[1];
	map+=5;

	for (j=0;j<mapheight;j++)
		{
		int starti;
		if (j==0)
			starti=5;
		else
			starti=0;
		for(i=starti;i<mapwidth;i++)
			{
         if ((i & 63) == 0) vTaskDelay(1);
			tile= *map++;



			switch (tile)
				{

				// Add light sourcing to these objects

				case 23:
				case 24:
				case 25:
				case 26:
				case 27:
				case 42:
				case 63:
				case 64:
					sprites[i][j]->flags |= FL_LIGHTON;
					break;

				case 28:
				case 43:
					if (MAPSPOT(i,j,2))
					  {
						touchx = (word) ((MAPSPOT(i,j,2) >> 8) & 0xff);
						touchy = (word) ((MAPSPOT(i,j,2) >> 0) & 0xff);
						tswitch = (wall_t*) actorat[touchx][touchy];

						if (tswitch && (tswitch->which == WALL))
							{tswitch->flags |= FL_REVERSIBLE;
							 if (!(tswitch->flags & FL_ON))
								{sprites[i][j]->shapenum --;
								 if (touchindices[touchx][touchy])
									 {Link_To_Touchplate(touchx,touchy,ActivateLight,DeactivateLight,(long)(sprites[i][j]),0);
									  sprites[i][j]->linked_to = touchindices[touchx][touchy]-1;
									 }
								 else
									 Error("tried to link a light at x=%d y=%d to a non-existent touchplate",i,j);
								}
							 else
								{if (touchindices[touchx][touchy])
									 {Link_To_Touchplate(touchx,touchy,DeactivateLight,ActivateLight,(long)(sprites[i][j]),0);
									  sprites[i][j]->linked_to = touchindices[touchx][touchy]-1;
									 }
								 else
									 Error("tried to link a light at x=%d y=%d to a non-existent touchplate",i,j);
								 sprites[i][j]->flags |= FL_LIGHTON;
								}
							}
					  else
						 {if (touchindices[touchx][touchy])
							 {Link_To_Touchplate(touchx,touchy,DeactivateLight,ActivateLight,(long)(sprites[i][j]),0);
							  sprites[i][j]->linked_to = touchindices[touchx][touchy]-1;
							 }
						  else
							 Error("tried to link a light at x=%d y=%d to a non-existent touchplate",i,j);
						  sprites[i][j]->flags |= FL_LIGHTON;
						 }

					  }
				  else
					  sprites[i][j]->flags |= FL_LIGHTON;

				  break;
				}
			}
		}
}

/*
==================
=
= PrintMapStats
=
==================
*/
void PrintMapStats (void)
{
   int size;
   int total;

   if (MAPSTATS==false)
      return;

   OpenMapDebug();

   total=0;
   MapDebug("MAP STATS Map Number %d\n",gamestate.mapon);
   MapDebug("=======================\n");
   size=pwallnum*sizeof(pwallobj_t);
   total+=size;
   MapDebug("Number of PushWalls   : %4d size = %6d\n",pwallnum,size);
   size=maskednum*sizeof(maskedwallobj_t);
   total+=size;
   MapDebug("Number of MaskedWalls : %4d size = %6d\n",maskednum,size);
   size=doornum*sizeof(doorobj_t);
   total+=size;
   MapDebug("Number of Doors       : %4d size = %6d\n",doornum,size);
   size=lasttouch*sizeof(touchplatetype);
   total+=size;
   MapDebug("Number of TouchPlates : %4d size = %6d\n",lasttouch,size);
   size=_numelevators*sizeof(elevator_t);
   total+=size;
   MapDebug("Number of Elevators   : %4d size = %6d\n",_numelevators,size);
   size=statcount*sizeof(statobj_t);
   total+=size;
   MapDebug("Number of Sprites     : %4d size = %6d\n",statcount,size);
   size=objcount*sizeof(objtype);
   total+=size;
   MapDebug("Number of Actors      : %4d size = %6d\n",objcount,size);
   MapDebug("Number of Clocks      : %4d\n",numclocks);
   MapDebug("\nTotal size of level : %6d\n",total);
}


boolean IsWeapon(int tile)
{
 if ((tile >= 46) && (tile <= 56))
   return true;

 return  false;

}


char *WeaponName(int tile)
{
 switch(tile)
   {
    case 46:
      return "Bat           ";
      break;

    case 47:
      return "Knife         ";
      break;

    case 48:
      return "Double Pistol ";
      break;

    case 49:
      return "MP40          ";
      break;

    case 50:
      return "Bazooka       ";
      break;

    case 51:
      return "Firebomb      ";
      break;

    case 52:
      return "Heatseeker    ";
      break;

    case 53:
      return "Drunk Missile ";
      break;

    case 54:
      return "Flamewall     ";
      break;

    case 55:
      return "Split Missile ";
      break;

    case 56:
      return "KES           ";
      break;
   }
   return " ";
}



int GetLumpForTile(int tile)
   {
   int wallstart;
   int exitstart;

	wallstart=W_GetNumForName("WALLSTRT");
	exitstart=W_GetNumForName("EXITSTRT");
	elevatorstart = W_GetNumForName("ELEVSTRT");

   if ((tile >= 1) && (tile <= 32))
      {
      return (tile + wallstart);
      }
   else if ((tile >= 36) && (tile <= 45))
      {
      return (tile + wallstart - 3);
      }
   else if (tile == 46)
      {
      return (wallstart + 74);
      }
   else if ((tile >= 47) && (tile <= 48))
      {
      return (tile + exitstart - 46);
      }
   else if ((tile >= 49) && (tile <= 71))
      {
      return (tile + wallstart - 8);
      }
   else if ((tile >= 72) && (tile <= 79))
      {
      return (tile - 72 + elevatorstart + 1);
      }
   else if ((tile >= 80) && (tile <= 89))
      {
      return (tile + wallstart - 16);
      }
   return -1;
   }




#if (DEVELOPMENT == 1)


/*
==================
=
= Insane Dump
=
==================
*/

void InsaneDump(void)
{
   int i,j,level;
   word *map,tile;

   if (TILESTATS==false)
      return;

   int *tally = rg_alloc(1000 * sizeof(int), MEM_SLOW);
   int (*inlevel)[10] = rg_alloc(1000 * 10 * sizeof(int), MEM_SLOW);

   OpenMapDebug();


// WALLS
   memset(tally,0,1000 * sizeof(int));
   memset(inlevel,0,1000 * 10 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= WALLS\n");
   MapDebug("=======================\n");
   mapheight = mapwidth = 128;
   //BATTLEMODE = 1;
   for(level=0;level<8;level ++)
    {
     GetEpisode(level);
     LoadROTTMap(level);
     map = mapplanes[0];
     for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
         {tile = *map++;
         if (IsWall(i,j)==true)
            {tally[tile]++;
             inlevel[tile][level]=1;
            }

         }
      }
    }

   MapDebug("Wall #   Frequency    Levels\n");
   MapDebug("----------------------------\n");
   for (i=0;i<1000;i++)
     if (i < 90)
       {MapDebug("%4d      %4d       %s",i,tally[i],
                 W_GetNameForNum(GetLumpForTile(i)));
        MapDebug("     ");
        for(level=0;level < 10;level ++)
          if (inlevel[i][level])
           MapDebug("%d,",level);
        MapDebug("\n");
       }





// Doors
   memset(tally,0,1000 * sizeof(int));
   memset(inlevel,0,1000 * 10 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= DOORS\n");
   MapDebug("=======================\n");
   for(level=0;level<10;level ++)
    {
     GetEpisode(level);
     LoadROTTMap(level);
     map = mapplanes[0];
     for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
         {tile = *map++;
         if (IsDoor(i,j)==true)
            {tally[tile]++;
             inlevel[tile][level]=1;
            }

         }
      }
    }

   MapDebug("Door #   Frequency    Levels\n");
   MapDebug("----------------------------\n");
   for (i=0;i<1000;i++)
     if (tally[i]!=0)
       {MapDebug("%4d      %4d         ",i,tally[i]);
        for(level=0;level < 10;level ++)
          if (inlevel[i][level])
           MapDebug("%d,",level);
        MapDebug("\n");

       }

// MaskedWalls
   memset(tally,0,1000 * sizeof(int));
   memset(inlevel,0,1000 * 10 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= MASKED WALLS\n");
   MapDebug("=======================\n");
   for(level=0;level<10;level ++)
    {
     GetEpisode(level);
     LoadROTTMap(level);
     map = mapplanes[0];
     for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
         {tile = *map++;
         if ((IsMaskedWall(i,j)) && (!IsPlatform(i,j)))
            {tally[tile]++;
             inlevel[tile][level]=1;
            }

         }
      }
    }

   MapDebug("MWall #   Frequency    Levels\n");
   MapDebug("----------------------------\n");
   for (i=0;i<1000;i++)
     if (tally[i]!=0)
       {MapDebug("%4d      %4d         ",i,tally[i]);
        for(level=0;level < 10;level ++)
          if (inlevel[i][level])
           MapDebug("%d,",level);
        MapDebug("\n");

       }

   SafeFree(tally);
   SafeFree(inlevel);
}

#endif

/*
==================
=
= PrintTileStats
=
==================
*/
void PrintTileStats (void)
{
   int i,j;
   word *map;
   int easytotal;
   int hardtotal;

   if (TILESTATS==false)
      return;

   int *tally = rg_alloc(1000 * sizeof(int), MEM_SLOW);

   OpenMapDebug();

   MapDebug("TILE STATS Map Number %d\n",gamestate.mapon);
   MapDebug("=======================\n\n");


// Weapons
   memset(tally,0,1000 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= WEAPONS\n");
   MapDebug("=======================\n");
   map = mapplanes[1];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
			{
         if (IsWeapon(*map))
           MapDebug("\n %s at %3d,%3d",WeaponName(*map),i,j);
         map++;
         }
      }
   MapDebug("\n\n");
// WALLS
   memset(tally,0,1000 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= WALLS\n");
   MapDebug("=======================\n");
	map = mapplanes[0];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
			{
         if (IsWall(i,j)==true)
            tally[(*map)]++;
         map++;
         }
      }
   MapDebug("Wall #        Frequency\n");
   MapDebug("-----------------------\n");
   for (i=0;i<1000;i++)
     if (tally[i]!=0)
        MapDebug("  %4d    %4d\n",i,tally[i]);

// Doors
   memset(tally,0,1000 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= DOORS\n");
   MapDebug("=======================\n");
	map = mapplanes[0];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
			{
         if (IsDoor(i,j)==true)
            tally[(*map)]++;
         map++;
         }
      }
   MapDebug("Door #        Frequency\n");
   MapDebug("-----------------------\n");
   for (i=0;i<1000;i++)
     if (tally[i]!=0)
        MapDebug("  %4d    %4d\n",i,tally[i]);

// MaskedWalls
   memset(tally,0,1000 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= MASKEDWALLS\n");
   MapDebug("=======================\n");
	map = mapplanes[0];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
			{
         if (IsMaskedWall(i,j)==true)
            if (IsPlatform(i,j)==false)
               tally[(*map)]++;
         map++;
         }
      }
   MapDebug("Mwall #       Frequency\n");
   MapDebug("-----------------------\n");
   for (i=0;i<1000;i++)
     if (tally[i]!=0)
        MapDebug("  %4d    %4d\n",i,tally[i]);
// Platforms
   memset(tally,0,1000 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= PLATFORMS\n");
   MapDebug("=======================\n");
	map = mapplanes[2];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
			{
         if (IsPlatform(i,j)==true)
            tally[(*map)]++;
         map++;
         }
      }
   MapDebug("Pform #        Frequency\n");
   MapDebug("-----------------------\n");
   for (i=0;i<1000;i++)
     if (tally[i]!=0)
        MapDebug("  %4d    %4d\n",i,tally[i]);

// Actors
   memset(tally,0,1000 * sizeof(int));
   MapDebug("=======================\n");
   MapDebug("= ACTORS\n");
   MapDebug("=======================\n");
	map = mapplanes[1];
	for (j=0;j<mapheight;j++)
      {
	   for(i=0;i<mapwidth;i++)
			{
         if ((*map)>0)
            tally[(*map)]++;
         map++;
         }
      }

// Low Guards
   easytotal=0;
   hardtotal=0;
   for (i=108;i<=119;i++)
      easytotal+=tally[i];
   for (i=126;i<=137;i++)
      hardtotal+=tally[i];
   MapDebug("\nLowGuards\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Sneaky Low Guards
   easytotal=0;
   hardtotal=0;
   for (i=120;i<=120;i++)
      easytotal+=tally[i];
   for (i=138;i<=138;i++)
      hardtotal+=tally[i];
   MapDebug("\nSneakyLowGuards\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// High Guards
   easytotal=0;
   hardtotal=0;
   for (i=144;i<=155;i++)
      easytotal+=tally[i];
   for (i=162;i<=173;i++)
      hardtotal+=tally[i];
   MapDebug("\nHighGuards\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// OverPatrol Guards
   easytotal=0;
   hardtotal=0;
   for (i=216;i<=227;i++)
      easytotal+=tally[i];
   for (i=234;i<=245;i++)
      hardtotal+=tally[i];
   MapDebug("\nOverPatrolGuards\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Strike Guards
   easytotal=0;
   hardtotal=0;
   for (i=180;i<=191;i++)
      easytotal+=tally[i];
   for (i=198;i<=204;i++)
      hardtotal+=tally[i];
   MapDebug("\nStrikeGuards\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// TriadEnforcer Guards
   easytotal=0;
   hardtotal=0;
   for (i=288;i<=299;i++)
      easytotal+=tally[i];
   for (i=306;i<=317;i++)
      hardtotal+=tally[i];
   MapDebug("\nTriadEnforcer Guards\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Lightning Guards
   easytotal=0;
   hardtotal=0;
   for (i=324;i<=335;i++)
      easytotal+=tally[i];
   for (i=342;i<=353;i++)
      hardtotal+=tally[i];
   MapDebug("\nLightningGuards\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Random Actors
   easytotal=0;
   hardtotal=0;
   for (i=122;i<=125;i++)
      easytotal+=tally[i];
   MapDebug("\nRandom Actors\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",easytotal);

// Monks
   easytotal=0;
   hardtotal=0;
   for (i=360;i<=371;i++)
      easytotal+=tally[i];
   for (i=378;i<=389;i++)
      hardtotal+=tally[i];
   MapDebug("\nMonks\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Fire Monks
   easytotal=0;
   hardtotal=0;
   for (i=396;i<=407;i++)
      easytotal+=tally[i];
   for (i=414;i<=425;i++)
      hardtotal+=tally[i];
   MapDebug("\nFire Monks\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Robo Guards
   easytotal=0;
   hardtotal=0;
   for (i=158;i<=161;i++)
      easytotal+=tally[i];
   for (i=176;i<=179;i++)
      hardtotal+=tally[i];
   MapDebug("\nRoboGuards\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Ballistikrafts
   easytotal=0;
   hardtotal=0;
   for (i=408;i<=411;i++)
      easytotal+=tally[i];
   for (i=426;i<=429;i++)
      hardtotal+=tally[i];
   MapDebug("\nBallistikrafts\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Boulders
   easytotal=0;
   hardtotal=0;
   for (i=278;i<=281;i++)
      easytotal+=tally[i];
   for (i=395;i<=395;i++)
      hardtotal+=tally[i];
   MapDebug("\nBoulders\n");
   MapDebug("-----------------------\n");
   MapDebug("Boulders=%4d\n",easytotal);
   MapDebug("BoulderHoles=%4d\n",hardtotal);

// PushColumns
   easytotal=0;
   hardtotal=0;
   for (i=285;i<=287;i++)
      easytotal+=tally[i];
   for (i=303;i<=305;i++)
      easytotal+=tally[i];
   for (i=321;i<=323;i++)
      easytotal+=tally[i];
   for (i=339;i<=341;i++)
      easytotal+=tally[i];
   for (i=357;i<=359;i++)
      easytotal+=tally[i];
   MapDebug("\nPushColumns\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",easytotal);

// Gun Emplacements
   easytotal=0;
   hardtotal=0;
   for (i=194;i<=197;i++)
      easytotal+=tally[i];
   for (i=212;i<=215;i++)
      hardtotal+=tally[i];
   MapDebug("\nGun Emplacements\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// 4-way guns
   easytotal=0;
   hardtotal=0;
   for (i=89;i<=89;i++)
      easytotal+=tally[i];
   for (i=211;i<=211;i++)
      hardtotal+=tally[i];
   MapDebug("\n4-way guns\n");
   MapDebug("-----------------------\n");
   MapDebug("EasyTotal=%4d\n",easytotal);
   MapDebug("HardTotal=%4d\n",hardtotal);
   MapDebug("    Total=%4d\n",easytotal+hardtotal);

// Stabbers from above
   MapDebug("\nStabbers from above\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[412]);

// Stabbers from below
   MapDebug("\nStabbers from below\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[430]);

// Crushing pillar from above
   MapDebug("\nCrushing pillar from above\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[413]);

// Crushing pillar from below
   MapDebug("\nCrushing pillar from below\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[431]);

// Above Spinner
   MapDebug("\nAbove Spinner\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[156]);

// Ground Spinner
   MapDebug("\nGround Spinner\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[174]);

// Spinner from above
   MapDebug("\nSpinner from above\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[157]);

// Spinner from below
   MapDebug("\nSpinner from below\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[175]);

// Bosses
   easytotal=0;
   for (i=99;i<=103;i++)
      easytotal+=tally[i];
   MapDebug("\nBosses\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",easytotal);

// Spring Boards
   MapDebug("\nSpring Boards\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",tally[193]);

// Above FlameJets
   easytotal=0;
   hardtotal=0;
   for (i=372;i<=376;i++)
      easytotal+=tally[i];
   for (i=390;i<=394;i++)
      hardtotal+=tally[i];
   MapDebug("\nFlameJets\n");
   MapDebug("-----------------------\n");
   MapDebug("    Above=%4d\n",easytotal);
   MapDebug("   Ground=%4d\n",hardtotal);

// Fire Chutes
   easytotal=0;
   for (i=140;i<=143;i++)
      easytotal+=tally[i];
   MapDebug("\nFireChutes\n");
   MapDebug("-----------------------\n");
   MapDebug("    Total=%4d\n",easytotal);

// Sprites
   MapDebug("=======================\n");
   MapDebug("= SPRITES\n");
   MapDebug("=======================\n");
   MapDebug("Sprite #       Frequency\n");
   MapDebug("-----------------------\n");
   for (i=1;i<=72;i++)
      if (tally[i]!=0)
         MapDebug("  %4d    %4d\n",i,tally[i]);
   for (i=210;i<=210;i++)
      if (tally[i]!=0)
         MapDebug("  %4d    %4d\n",i,tally[i]);
   for (i=228;i<=233;i++)
      if (tally[i]!=0)
         MapDebug("  %4d    %4d\n",i,tally[i]);
   for (i=246;i<=255;i++)
      if (tally[i]!=0)
         MapDebug("  %4d    %4d\n",i,tally[i]);
   for (i=260;i<=273;i++)
      if (tally[i]!=0)
         MapDebug("  %4d    %4d\n",i,tally[i]);
   for (i=282;i<=284;i++)
      if (tally[i]!=0)
         MapDebug("  %4d    %4d\n",i,tally[i]);

   SafeFree(tally);
}

//***************************************************************************
//
// GetSongForLevel - returns song to play for current level
//
//***************************************************************************
int GetSongForLevel ( void )
{
   int i;
   int num;

   for (i=0;i<mapwidth;i++)
      {
      num = MAPSPOT(i,0,2);
      if ( (num>>8) == 0xba )
         return (num&0xff);
      }
//   Error("GetSongForLevel: could not find song in level %ld\n",gamestate.mapon);
//   return -1;
     return 0;
}

/*
==================
=
= DoSharewareConversionBackgroundPlane
=
==================
*/
void DoSharewareConversionBackgroundPlane (void)
{
   int i,j;
   word * map;


	for (j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
			{
         map=&(mapplanes[0][MAPSIZE*(j)+(i)]);
         switch (*map)
            {

            // Tom Face
            case 45:
               *map=44;
               break;
            // Doors
            case 90:
            case 92:
            case 93:
            case 98:
            case 99:
            case 100:
               *map=91;
               break;
            case 103:
            case 104:
               *map=101;
               break;
            case 154:
               *map=33;
               break;
            case 155:
               *map=34;
               break;
            case 156:
               *map=35;
               break;

            //locked doors
            case 94:
               *map = 101;
               *(&(mapplanes[1][MAPSIZE*(j)+(i)]))=29;
               break;
            case 95:
               *map = 101;
               *(&(mapplanes[1][MAPSIZE*(j)+(i)]))=30;
               break;
            case 96:
               *map = 101;
               *(&(mapplanes[1][MAPSIZE*(j)+(i)]))=31;
               break;
            case 97:
               *map = 101;
               *(&(mapplanes[1][MAPSIZE*(j)+(i)]))=32;
               break;
            // Tall pillar
            case 161:
               *map=0;
               break;
            // Masked Walls
            case 162:
            case 166:
               *map=164;
               break;
            case 163:
            case 167:
            case 170:
            case 171:
               *map=165;
               break;

            // Floors and Ceilings
            case 180:
            case 183:
            case 184:
               *map=181;
               break;
            case 198:
            case 201:
            case 202:
               *map=199;
               break;
            case 188:
               *map=187;
               break;
            case 206:
               *map=205;
               break;
            case 190:
               *map=191;
               break;
            case 208:
               *map=209;
               break;
            case 192:
            case 193:
            case 194:
            case 195:
               *map=189;
               break;
            case 210:
            case 211:
            case 212:
            case 213:
               *map=207;
               break;
            // Skys
            case 237:
            case 238:
            case 239:
               *map=234;
               break;
            // Animating walls
            case 107:
               *map=106;
               break;
            case 228:
            case 229:
            case 230:
            case 242:
               *map=21;
               break;
            case 233:
               *map=44;
               break;
            case 232:
               *map=231;
               break;
            }
         }
      }
}


/*
========================================
=
= DoLowMemoryConversionBackgroundPlane
=
========================================
*/
void DoLowMemoryConversionBackgroundPlane (void)
   {
   int i,j;
   word * map;


	for (j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
			{
         map=&(mapplanes[0][MAPSIZE*(j)+(i)]);
         switch (*map)
            {
            //Walls

            case 2:
            case 3:
            case 4:
               *map = 1;
               break;

            case 6:
            case 7:
            case 8:
               *map = 5;
               break;

            case 14:
            case 15:
            case 16:
               *map = 13;
               break;

            case 18:
            case 19:
            case 20:
               *map = 17;
               break;

            case 26:
            case 27:
            case 28:
               *map = 25;
               break;

            case 30:
            case 31:
            case 32:
               *map = 29;
               break;

#if 0
            case 37:
            case 38:
            case 39:
               *map = 36;
               break;

            case 41:
            case 42:
            case 43:
               *map = 40;
               break;
#endif

            case 50:
            case 51:
            case 52:
               *map = 49;
               break;

#if 0
            case 55:
            case 56:
            case 57:
               *map = 54;
               break;

            case 59:
            case 60:
            case 61:
               *map = 58;
               break;
#endif

            case 66:
            case 67:
            case 68:
               *map = 65;
               break;

            case 70:
            case 71:
               *map = 69;
               break;

            case 81:
            case 82:
            case 84:
               *map = 83;
               break;

            // Masked Walls
            case 158:
            case 159:
            case 160:
            case 168:
            case 169:
            case 176:
            case 178:
               *map=177;
               break;
            case 162:
            case 163:
            case 164:
            case 166:
            case 167:
               *map=165;
               break;

            //Doors
            case 90:
            case 91:
            case 92:
            case 93:
            case 98:
            case 99:
            case 100:
            case 101:
            case 103:
            case 104:
            case 33:
            case 34:
            case 35:
            case 154:
            case 155:
            case 156:
               *map = 101;
               break;

            //Animating Walls
            case 22:
            case 23:
            case 24:
            case 228:
            case 229:
            case 230:
            case 231:
            case 232:
            case 242:
            case 243:
            case 244:
               *map = 21;
               break;
            case 233:
               *map = 44;
               break;

#if 0
            //Skys
            case 234:
            case 235:
            case 236:
            case 237:
            case 238:
            case 239:
               *map=(*(&(mapplanes[0][MAPSIZE*(0)+(0)]))) + 18;
               break;
#endif
            }
         }
      }
   }


/*
========================================
=
= DoLowMemoryConversionIconPlane
=
========================================
*/
void DoLowMemoryConversionIconPlane (void)
   {
#if 0
   int i,j;
   word * map;


	for (j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
			{
         map=&(mapplanes[2][MAPSIZE*(j)+(i)]);
         switch (*map)
            {
            case 13:
               *(&(mapplanes[0][MAPSIZE*(j)+(i)]))=21;
               *map=0;
               break;
            }
         }
      }
#endif
   }



/*
========================================
=
= DoLowMemoryConversionForegroundPlane
=
========================================
*/

void DoLowMemoryConversionForegroundPlane (void)
   {
   int i,j;
   word * map;


	for (j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
			{
         map=&MAPSPOT(i,j,1);
         switch (*map)
            {
            // light sourcing
            case 139:
               *map=0;
               break;

            //sprites
            case 42:
            case 43:
            case 63:
            case 64:
               *map = 43;
               break;

            case 246:
            case 247:
            case 248:
            case 264:
            case 265:
            case 267:
            case 283:
               *map = 266;
               break;

            //lightning
            case 377:
               *map = 0;
               break;

            // actor guards

            // normal low guards
            case 108:
            case 109:
            case 110:
            case 111:
            case 112:
            case 113:
            case 114:
            case 115:
            case 116:
            case 117:
            case 118:
            case 119:

            case 126:
            case 127:
            case 128:
            case 129:
            case 130:
            case 131:
            case 132:
            case 133:
            case 134:
            case 135:
            case 136:
            case 137:
               (*map)+=216;
               break;

            // sneaky low guards
            case 120:
            case 138:
               *map = 0;
               break;

            // normal over patrol
            case 216:
            case 217:
            case 218:
            case 219:
            case 220:
            case 221:
            case 222:
            case 223:
            case 224:
            case 225:
            case 226:
            case 227:


            case 234:
            case 235:
            case 236:
            case 237:
            case 238:
            case 239:
            case 240:
            case 241:
            case 242:
            case 243:
            case 244:
            case 245:
               (*map)-=36;
               break;

            //environment dangers

#if (SHAREWARE==0)
            case 412:      //spears to firejets
               *map = 372;
               break;

            case 430:
               *map = 390;
               break;

            case 413:       //cylinders down to firejets
               *map = 372;
               break;
#endif

            case 156:
            case 157:
               *map = 372;    //spinblade stabbers to firejets
               break;

            case 174:
            case 175:
               *map = 390;
               break;

            case 301:          // directional spin blades
               *map = 373;
               break;

            case 319:          // directional spin blades
               *map = 374;
               break;

            case 337:          // directional spin blades
               *map = 375;
               break;

            case 355:          // directional spin blades
               *map = 376;
               break;

            case 302:          // directional spin blades
               *map = 391;
               break;

            case 320:          // directional spin blades
               *map = 392;
               break;

            case 338:          // directional spin blades
               *map = 393;
               break;

            case 356:          // directional spin blades
               *map = 394;
               break;

            case 194:      // directional emplacements to four-way
            case 195:      // easy
            case 196:
            case 197:
               *map = 89;
               break;

            case 212:      // hard
            case 213:
            case 214:
            case 215:
               *map = 211;
               break;

            }
         }
      }
   }


/*
==================
=
= DoSharewareConversionForegroundPlane
=
==================
*/
void DoSharewareConversionForegroundPlane (void)
{
   int i,j;
   word * map;


	for (j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
			{
         map=&(mapplanes[1][MAPSIZE*(j)+(i)]);
         switch (*map)
            {
            case 32:  // Crystal Key
            case 47:  // Knife
            case 65:  // DIP BALL D
            case 66:  // DIP BALL I
            case 67:  // DIP BALL P
            case 99:  // Boss
            case 100:  // Boss
            case 101:  // Boss
            case 102:  // Boss
            case 103:  // Boss
            case 210:  // Scott's head
            case 278:  // Boulder
            case 279:  // Boulder
            case 280:  // Boulder
            case 281:  // Boulder
            case 395:  // Boulder
               *map=0;
               break;
            case 41:  // 3-UP to 1-UP
               *map=40;
               break;
            case 46:  // Bat
               *map=50;
               break;
            case 55:  // Split Missile
               *map=52;
               break;
            case 56:  // KES
               *map=53;
               break;
            case 253:  // Dog Mode
               *map=254;
               break;
            case 262:  // Tom Larva
               *map=263;
               break;
            }
         }
      }
}

/*
========================================
=
= DoRegisterConversionBackgroundPlane
=
========================================
*/
void DoRegisterConversionBackgroundPlane (void)
   {
   int i,j;
   word * map;


	for (j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
			{
         map=&(mapplanes[0][MAPSIZE*(j)+(i)]);
         switch (*map)
            {
            //locked doors

            case 94:
               *map = 101;
               *(&(mapplanes[1][MAPSIZE*(j)+(i)]))=29;
               break;
            case 95:
               *map = 101;
               *(&(mapplanes[1][MAPSIZE*(j)+(i)]))=30;
               break;
            case 96:
               *map = 101;
               *(&(mapplanes[1][MAPSIZE*(j)+(i)]))=31;
               break;
            case 97:
               *map = 101;
               *(&(mapplanes[1][MAPSIZE*(j)+(i)]))=32;
               break;

            case 232:
               *map = 231; //map chains to machinery
               break;

            case 228:
               *map = 230; //map gray water to blue water
               break;

            }
         }
      }
   }



/*
========================================
=
= DoRegisterConversionForegroundPlane
=
========================================
*/
void DoRegisterConversionForegroundPlane (void)
   {
//   int i,j;
//   word * map;


#if 0
	for (j=0;j<mapheight;j++)
      {
      for(i=0;i<mapwidth;i++)
			{
         map=&MAPSPOT(i,j,1);
         switch (*map)
            {
            //sprites
            case 42:
            case 43:
            case 63:
            case 64:
               *map = 43;
               break;

            }
         }
      }
#endif
   }

/*
==================
=
= DoSharewareConversion
=
==================
*/
void DoSharewareConversion (void)
{
   DoSharewareConversionBackgroundPlane ();
   DoSharewareConversionForegroundPlane ();
}


/*
==================
=
= DoRegisterConversion
=
==================
*/
void DoRegisterConversion (void)
{
   DoRegisterConversionBackgroundPlane ();
   DoRegisterConversionForegroundPlane ();
}

/*
=======================
=
= DoPanicMapping
=
=======================
*/
boolean DoPanicMapping (void)
   {
   if ((lowmemory==true) && (modemgame==false) && (demorecord==false) && (demoplayback==false))
      return true;
   else
      return false;
   }

/*
=======================
=
= DoLowMemoryConversion
=
=======================
*/
void DoLowMemoryConversion (void)
   {
   DoLowMemoryConversionBackgroundPlane ();
   if ((modemgame==false) && (demorecord==false) && (demoplayback==false))
      DoLowMemoryConversionForegroundPlane ();
   DoLowMemoryConversionIconPlane ();
   }



/*
==================
=
= SetupGameLevel
=
==================
*/

void ScanInfoPlane(void)
{
}

void SetupGameLevel (void)
{
	int crud;
	int i;

#if 0
   mapwidth = mapheight = 128;

   InsaneDump();
   /*
   for(i=0;i<11;i++)
     {GetEpisode(i);
      LoadROTTMap(i);
      MapDebug("\n//================================//");
      MapDebug("\n//   SHAREWARE LEVEL %d            //",i);
      MapDebug("\n//================================//\n\n");

      PrintTileStats();
     }
   */
   Error("okay");
#endif

	insetupgame=true;

   InitializeRNG ();

	if ((demoplayback==true) || (demorecord==true))
      SetRNGindex ( 0 );

   if (gamestate.randomseed!=-1)
      SetRNGindex ( gamestate.randomseed );

	if (tedlevel)
		{
		GetEpisode (tedlevelnum);
		LoadROTTMap(tedlevelnum);
		gamestate.mapon=tedlevelnum;
		}
	else
		{
		GetEpisode (gamestate.mapon);
		LoadROTTMap(gamestate.mapon);
		}
   if (DoPanicMapping())
      {
      DoLowMemoryConversion();
      }
   if ( gamestate.Product == ROTT_SHAREWARE )
      {
      DoSharewareConversion ();
      }
   else
      {
      DoRegisterConversion ();
      }
   if ( (NewGame) || (lastlevelloaded!=gamestate.mapon) || (loadedgame==false) )
              {
              SetupPreCache();
              lastlevelloaded=gamestate.mapon;
              MU_StartSong(song_level);
              }

   // Reset timing to prevent engine from catching up on missed frames during load
   oldtime = oldpolltime = GetTicCount();

   shapestart = W_GetNumForName("SHAPSTRT");
   shapestop = W_GetNumForName("SHAPSTOP");
	gunsstart=W_GetNumForName("GUNSTART");

	playstate = ex_stillplaying;
	SNAKELEVEL = 0;
	whichpath = 0;

	// som of the code / calls below need bufferofs & friends to point
	// to to the real screen, not the stretch buffer
	DisableScreenStretch();//bna++ shut off streech mode

	InitializePlayerstates();

	ResetCheatCodes();

	gamestate.killtotal     = gamestate.killcount     = 0;
	gamestate.secrettotal   = gamestate.secretcount   = 0;
	gamestate.treasuretotal = gamestate.treasurecount = 0;
	gamestate.supertotal    = gamestate.supercount    = 0;
	gamestate.healthtotal   = gamestate.healthcount   = 0;
	gamestate.missiletotal  = gamestate.missilecount  = 0;
	gamestate.democratictotal = gamestate.democraticcount = 0;
	gamestate.planttotal    = gamestate.plantcount    = 0;
	gamestate.DODEMOCRATICBONUS1 = true;
	gamestate.DOGROUNDZEROBONUS  = false;

	if (gamestate.mapon == 30)
	 SNAKELEVEL = 1;
	else if (gamestate.mapon == 32)
	 SNAKELEVEL = 2;
	else if (gamestate.mapon == 33)
	 SNAKELEVEL = 3;

	InitAreas();
	InitDoorList();
	InitElevators();

	InitStaticList ();
	InitActorList();

	memset (tilemap,0,MAPSIZE * MAPSIZE * sizeof(word));

	memset (actorat,0,MAPSIZE * MAPSIZE * sizeof(void *));
	memset (sprites,0,MAPSIZE * MAPSIZE * sizeof(statobj_t *));
	memset (mapseen,0,MAPSIZE * MAPSIZE * sizeof(byte));
	memset (LightsInArea,0,(NUMAREAS + 1) * sizeof(int));

	PrintTileStats();

	SetupLightLevels();

   crud=(word)MAPSPOT(0,0,1);
	if ((crud>=90) && (crud<=97))
		{
		levelheight=crud-89;
		maxheight = (levelheight << 6)-32;
		nominalheight = maxheight-32;
		}
	else if ((crud>=450) && (crud<=457))
		{
		levelheight=crud-450+9;
		maxheight = (levelheight << 6)-32;
		nominalheight = maxheight-32;
		}
	else
		Error("You must specify a valid height sprite icon at (2,0) on map %d\n",gamestate.mapon);

/*
   if ( ( BATTLEMODE ) && ( !gamestate.BattleOptions.SpawnDangers ) )
      {
      RemoveDangerWalls();
      }
*/
// pheight=maxheight-32;
   CountAreaTiles();
	SetupWalls();

	SetupClocks();
	SetupAnimatedWalls();

	if (loadedgame==false)
		{
		SetupSwitches();
		SetupStatics ();
		SetupMaskedWalls();
		SetupDoors();
		SetupPushWalls();
		SetupPlayers();
      if (!BATTLEMODE)
         {
	     	SetupActors();
	      SetupRandomActors();
         }
		SetupElevators();
		SetupDoorLinks();
		SetupPushWallLinks();
		FixDoorAreaNumbers();
		FixMaskedWallAreaNumbers();
		SetupWindows();
		SetupLights();
      SetupInanimateActors();
      }
   else {
      FixTiles();
   }
   
	if (gamestate.SpawnEluder || gamestate.SpawnDeluder)
      {
//MED
      for (i=0;i<25;i++)
         RespawnEluder();
      }


   if ( ( BATTLEMODE ) && ( MapSpecials & MAP_SPECIAL_TOGGLE_PUSHWALLS ) )
      {
      ActivateAllPushWalls();
      }
   Illuminate();

	if (SNAKELEVEL == 1)
		SetupSnakePath();

	LoftSprites();

	SetPlaneViewSize();
	if (loadedgame==false)
		{
		ConnectAreas();
#if (DEVELOPMENT == 1)
#if (PRECACHETEST == 1)
		SoftError("Start PreCaching\n");
#endif
#endif
#if (DEVELOPMENT == 1)
      PrintMapStats();
#endif
		PreCache();
#if (DEVELOPMENT == 1)
#if (PRECACHETEST == 1)
		SoftError("Done PreCaching\n");
#endif
#endif
		SetupPlayScreen();
		SetupScreen(false);
	}

        if (BATTLEMODE) {
           SetModemLightLevel ( gamestate.BattleOptions.LightLevel );
        }

        if (player != NULL) {
            for (i=0;i<100;i++) {
		UpdateLightLevel(player->areanumber);
            }
        }
        
     // Reset timing to prevent engine from catching up on missed frames during load
     oldtime = oldpolltime = GetTicCount();
     
	insetupgame=false;

	tedlevel = false;   // turn it off once we have done any ted stuff
	
	EnableScreenStretch();
}


void InitializePlayerstates(void)
{int i;
 playertype * pstate;

 if (NewGame || (gamestate.mapon == 0) || tedlevel)
		{for(i=0;i<numplayers;i++)
			InitializeWeapons(&PLAYERSTATE[i]);
		}

 for(i=0;i<numplayers;i++)
	 {
    pstate=&PLAYERSTATE[i];
    if (
         (pstate->missileweapon == wp_godhand)
#if (SHAREWARE == 0)
         ||
         (pstate->missileweapon == wp_dog)
#endif
       )
		 {
       pstate->weapon=pstate->new_weapon=pstate->oldweapon;
		 pstate->missileweapon = pstate->oldmissileweapon;

		 }

	 ResetPlayerstate(pstate);
	 }

 NewGame = false;

}


void SetupSnakePath(void)
{
#if (SHAREWARE == 0)
 int i,j;
 word *map,tile;

 map = mapplanes[1];

 for(j=0;j<mapheight;j++)
  for(i=0;i<mapwidth;i++)
	{tile = *map++;
	 if ((tile >= 72) && (tile <= 79) && (!tilemap[i][j]))
	  {SNAKEPATH[whichpath].x = i;
		SNAKEPATH[whichpath].y = j;
		whichpath ++;
	  }

	}
#endif
}


void SetupRandomActors(void)
{int i,j;
 word *map,tile;
 int startj,totalrandom=0,count=0,ambush,locindex,orig;
 byte actorpresent[10]={0},index=0,randomtype,used[100]={0};
 _2Dpoint randloc[100];


 map = mapplanes[1];
 map+=5;
 for(i=0;i<10;i++)
  {if (RANDOMACTORTYPE[i])
	  actorpresent[index++]=i;
  }


 if (!index)
  return;

 for (j=0;j<mapheight;j++)
	{
	int starti;
	if (j==0)
		starti=5;
	 else
		starti=0;
	 for(i=starti;i<mapwidth;i++)
		{tile= *map++;

		 if ((tile >= 122) && (tile <= 125))
			 {randloc[totalrandom].x = i;
			  randloc[totalrandom].y = j;
			  totalrandom++;
			  if (totalrandom >= 100)
				Error("Max random actors (100) exceeded");
			 }

		}
	}

 orig = totalrandom;
 switch(gamestate.difficulty)
    {
    case gd_baby:
       totalrandom = 7*totalrandom/10;
       break;

    case gd_easy:
       totalrandom = 8*totalrandom/10;
       break;

    case gd_medium:
       totalrandom = 9*totalrandom/10;
       break;

    }


 while(count<totalrandom)
	{ locindex = (GameRandomNumber("rand loc index",0) % orig);

	  if (!used[locindex])
       {randomtype = actorpresent[GameRandomNumber("SetupRandomActors",0) % index];
		  ambush = (GameRandomNumber("rand actor",0) < 128);
		  i = randloc[locindex].x;
		  j = randloc[locindex].y;
		  tile = mapplanes[1][i*128 + j];
		  SpawnStand(randomtype,i,j,tile-122,ambush);
		  used[locindex] = 1;
        PreCacheActor(randomtype,0);
		  count++;
		 }
	}

}

void SetupActors(void)
{
  int i,j;
  word *map,tile;


  //GetRainActors();

  map = mapplanes[1];
  map+=5;

  for (j=0;j<mapheight;j++)
		{
		int starti;
		if (j==0)
			starti=5;
		else
			starti=0;
		for(i=starti;i<mapwidth;i++)
			{
			tile= *map++;

			switch(tile)
			  {

				case 126:
				case 127:
				case 128:
				case 129:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 108:
				case 109:
				case 110:
				case 111:
					SpawnStand(lowguardobj,i,j,tile-108,0);
					break;




				case 130:
				case 131:
				case 132:
				case 133:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 112:
				case 113:
				case 114:
				case 115:
					SpawnPatrol(lowguardobj,i,j,tile-112);
					break;

				case 134:
				case 135:
				case 136:
				case 137:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 116:
				case 117:
				case 118:
				case 119:
					SpawnStand(lowguardobj,i,j,tile-116,1);
					break;

				case 138:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 120:
					SpawnSneaky(i,j);
					break;

				case 162:
				case 163:
				case 164:
				case 165:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 144:
				case 145:
				case 146:
				case 147:
					SpawnStand(highguardobj,i,j,tile-144,0);
					break;

				case 170:
				case 171:
				case 172:
				case 173:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 152:
				case 153:
				case 154:
				case 155:
					SpawnStand(highguardobj,i,j,tile-152,1);
					break;



				case 166:
				case 167:
				case 168:
				case 169:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 148:
				case 149:
				case 150:
				case 151:
					SpawnPatrol(highguardobj,i,j,tile-148);
					break;

				case 176:
				case 177:
				case 178:
				case 179:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 158:
				case 159:
				case 160:
				case 161:
					SpawnPatrol(roboguardobj,i,j,tile-158);
					break;

				case 212:
				case 213:
				case 214:
				case 215:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 194:
				case 195:
				case 196:
				case 197:
					SpawnGunThingy(patrolgunobj,i,j,tile-194);
					break;

				case 198:
				case 199:
				case 200:
				case 201:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 180:
				case 181:
				case 182:
				case 183:
					SpawnStand(strikeguardobj,i,j,tile-180,0);
					break;

				case 206:
				case 207:
				case 208:
				case 209:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 188:
				case 189:
				case 190:
				case 191:
					SpawnStand(strikeguardobj,i,j,tile-188,1);
					break;

				case 202:
				case 203:
				case 204:
				case 205:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 184:
				case 185:
				case 186:
				case 187:
					SpawnPatrol(strikeguardobj,i,j,tile-184);
					break;

				case 234:
				case 235:
				case 236:
				case 237:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 216:
				case 217:
				case 218:
				case 219:
					SpawnStand(overpatrolobj,i,j,tile-216,0);
					break;

				case 242:
				case 243:
				case 244:
				case 245:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 224:
				case 225:
				case 226:
				case 227:
					SpawnStand(overpatrolobj,i,j,tile-224,1);
					break;

				case 238:
				case 239:
				case 240:
				case 241:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 220:
				case 221:
				case 222:
				case 223:
					SpawnPatrol(overpatrolobj,i,j,tile-220);
					break;
				case 306:
				case 307:
				case 308:
				case 309:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 288:
				case 289:
				case 290:
				case 291:
					SpawnStand(triadenforcerobj,i,j,tile-288,0);
					break;

				case 314:
				case 315:
				case 316:
				case 317:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 296:
				case 297:
				case 298:
				case 299:
					SpawnStand(triadenforcerobj,i,j,tile-296,1);
					break;

				case 310:
				case 311:
				case 312:
				case 313:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 292:
				case 293:
				case 294:
				case 295:
					SpawnPatrol(triadenforcerobj,i,j,tile-292);
					break;

				case 342:
				case 343:
				case 344:
				case 345:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 324:
				case 325:
				case 326:
				case 327:
					SpawnStand(blitzguardobj,i,j,tile-324,0);
					break;

				case 350:
				case 351:
				case 352:
				case 353:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 332:
				case 333:
				case 334:
				case 335:
					SpawnStand(blitzguardobj,i,j,tile-332,1);
					break;

				case 346:
				case 347:
				case 348:
				case 349:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;
				case 328:
				case 329:
				case 330:
				case 331:
					SpawnPatrol(blitzguardobj,i,j,tile-328);
					break;

				case 378:
				case 379:
				case 380:
				case 381:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 360:
				case 361:
				case 362:
				case 363:
					SpawnStand(deathmonkobj,i,j,tile-360,0);
					break;

				case 386:
				case 387:
				case 388:
				case 389:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 368:
				case 369:
				case 370:
				case 371:
					SpawnStand(deathmonkobj,i,j,tile-368,1);
					break;

				case 382:
				case 383:
				case 384:
				case 385:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 364:
				case 365:
				case 366:
				case 367:
					SpawnPatrol(deathmonkobj,i,j,tile-364);
					break;

				case 414:
				case 415:
				case 416:
				case 417:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 396:
				case 397:
				case 398:
				case 399:
					SpawnStand(dfiremonkobj,i,j,tile-396,0);
					break;



				case 422:
				case 423:
				case 424:
				case 425:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;


				case 404:
				case 405:
				case 406:
				case 407:
					SpawnStand(dfiremonkobj,i,j,tile-404,1);
					break;

				case 418:
				case 419:
				case 420:
				case 421:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 400:
				case 401:
				case 402:
				case 403:
					SpawnPatrol(dfiremonkobj,i,j,tile-400);
					break;

				case 99:
					SpawnStand(b_darianobj,i,j,tile-99,0);
					break;
				case 100:
					SpawnStand(b_heinrichobj,i,j,tile-100,0);
					break;
				case 101:
					SpawnStand(b_darkmonkobj,i,j,tile-101,0);
					MISCVARS->TOMLOC.x = i;
					MISCVARS->TOMLOC.y = j;
					break;
				case 102:
					SpawnMultiSpriteActor(b_robobossobj,i,j,tile-102);
					break;

				case 103:
					SpawnSnake(i,j);
					break;

				case 426:
				case 427:
				case 428:
				case 429:
					if (gamestate.difficulty < gd_hard)
						break;
					tile -= 18;

				case 408:
				case 409:
				case 410:
				case 411:
					SpawnPatrol(wallopobj,i,j,tile-408);
					break;


			  }
			}
		 }
}

void SetupStatics(void)
{
	int i,j,spawnz;
	word *map,tile;

    printf("SetupStatics started. Map: %dx%d\n", mapwidth, mapheight);

	map = mapplanes[1];
	map+=5;

   BATTLE_NumCollectorItems = 0;
	for (j=0;j<mapheight;j++)
		{
		int starti;
		if (j==0)
			starti=5;
		else
			starti=0;
		for(i=starti;i<mapwidth;i++)
			{
         if ((i & 63) == 0) vTaskDelay(1);
			tile= *map++;
			spawnz = (MAPSPOT(i,j,2))?(MAPSPOT(i,j,2)):(-1);

         if ( gamestate.BattleOptions.RandomWeapons )
            {
            int num;

            switch( tile )
               {
               case 46:
               case 48:
               case 49:
               case 50:
               case 51:
               case 52:
               case 53:
               case 54:
               case 55:
               case 56:
                  if ( gamestate.Product == ROTT_SHAREWARE )
                     {
                     num = ( GameRandomNumber( "Random Weapon", 0 ) % 7 );
                     tile = SharewareWeaponTiles[ num ];
                     }
                  else
                     {
                     num = ( GameRandomNumber( "Random Weapon", 1 ) % 10 );
                     tile = NormalWeaponTiles[ num ];
                     }
                  break;
               }
            }

			switch (tile)
				{

				// Add light sourcing to these objects

				case 23:
				case 24:
				case 25:
				case 26:
				case 27:
				case 28:
				case 42:
				case 43:
				case 63:
				case 64:
						SpawnStatic(i,j,tile-23,spawnz);
					break;

				case 44:
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
					{
						gamestate.healthtotal ++;
						gamestate.democratictotal ++;
					}
					break;


				case 36:
				case 37:
				case 38:
				case 39:
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.healthtotal ++;
					break;

				case 29:
				case 30:
				case 31:
				case 32:
               if (IsDoor (i, j) == 0)
                  {
                  if ( BATTLEMODE )
                     {
                     // Spawn empty table
                     SpawnStatic( i, j, 247 - 246 + 57, spawnz );
                     }
                  else
                     {
                     // Spawn key table
                     SpawnStatic( i, j, tile - 23, spawnz );
                     }
                  }
					break;

				case 33:
				case 34:
				case 35:
				case 40:
				case 41:
				case 45:
						SpawnStatic(i,j,tile-23,spawnz);
					break;

				case 46:
               #if (SHAREWARE == 1)
                 Error("\n tried to spawn excalibat at %d,%d in shareware !",i,j);
               #endif


               SD_PreCacheSoundGroup(SD_EXCALIBOUNCESND,SD_EXCALIBLASTSND);


					PreCacheGroup(W_GetNumForName("EXBAT1"),
									  W_GetNumForName("EXBAT7"),
									  cache_patch_t);



						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.missiletotal ++;
					break;
				case 47:
					PreCacheGroup(W_GetNumForName("KNIFE1"),
									  W_GetNumForName("KNIFE10"),
									  cache_patch_t);
					PreCacheGroup(W_GetNumForName("ESTATUE1"),
									  W_GetNumForName("ESTATUE8"),
									  cache_patch_t);

						SpawnStatic(i,j,tile-23,spawnz);
					break;

				case 48:
					SD_PreCacheSound(SD_ATKTWOPISTOLSND);

               if ((locplayerstate->player == 1) || (locplayerstate->player == 3))
					  PreCacheGroup(W_GetNumForName("RFPIST1"),
										 W_GetNumForName("LFPIST3"),
										 cache_patch_t);

               else if (locplayerstate->player == 2)
					  PreCacheGroup(W_GetNumForName("RBMPIST1"),
										 W_GetNumForName("LBMPIST3"),
										 cache_patch_t);

					else
					  PreCacheGroup(W_GetNumForName("RMPIST1"),
										 W_GetNumForName("LMPIST3"),
										 cache_patch_t);

               SpawnStatic(i,j,tile-23,spawnz);

               break;
				case 49:

					SD_PreCacheSound(SD_ATKMP40SND);
					PreCacheGroup(W_GetNumForName("MP401"),
									  W_GetNumForName("MP403"),
									  cache_patch_t);
						SpawnStatic(i,j,tile-23,spawnz);
					break;

				case 50:
					SD_PreCacheSound(SD_MISSILEHITSND);
					SD_PreCacheSound(SD_MISSILEFLYSND);
					SD_PreCacheSound(SD_BAZOOKAFIRESND);
					PreCacheGroup(W_GetNumForName("BAZOOKA1"),
									  W_GetNumForName("BAZOOKA4"),
									  cache_patch_t);
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.missiletotal ++;
					break;
				case 51:


               SD_PreCacheSound(SD_MISSILEHITSND);
					SD_PreCacheSound(SD_MISSILEFLYSND);
					SD_PreCacheSound(SD_FIREBOMBFIRESND);
					PreCacheGroup(W_GetNumForName("FBOMB1"),
									  W_GetNumForName("FBOMB4"),
									  cache_patch_t);
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.missiletotal ++;
					break;
				case 52:
					SD_PreCacheSound(SD_MISSILEHITSND);
					SD_PreCacheSound(SD_MISSILEFLYSND);
					SD_PreCacheSound(SD_HEATSEEKFIRESND);
					PreCacheGroup(W_GetNumForName("HSEEK1"),
									  W_GetNumForName("HSEEK4"),
									  cache_patch_t);
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.missiletotal ++;
					break;
				case 53:
               SD_PreCacheSound(SD_MISSILEHITSND);
					SD_PreCacheSound(SD_MISSILEFLYSND);
					SD_PreCacheSound(SD_DRUNKFIRESND);
					PreCacheGroup(W_GetNumForName("DRUNK1"),
									  W_GetNumForName("DRUNK4"),
									  cache_patch_t);
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.missiletotal ++;
					break;
				case 54:
					SD_PreCacheSound(SD_MISSILEHITSND);
					SD_PreCacheSound(SD_MISSILEFLYSND);
					SD_PreCacheSound(SD_FLAMEWALLFIRESND);
					SD_PreCacheSound(SD_FLAMEWALLSND);
					PreCacheGroup(W_GetNumForName("FIREW1"),
									  W_GetNumForName("FIREW3"),
									  cache_patch_t);
					PreCacheGroup(W_GetNumForName("FWALL1"),
									  W_GetNumForName("FWALL15"),
									  cache_patch_t);
					PreCacheGroup(W_GetNumForName("SKEL1"),
									  W_GetNumForName("SKEL48"),
									  cache_patch_t);
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.missiletotal ++;
					break;
				case 55:
               #if (SHAREWARE == 1)
                 Error("\n tried to spawn split missile at %d,%d in shareware !",i,j);
               #endif
               SD_PreCacheSound(SD_MISSILEHITSND);
					SD_PreCacheSound(SD_MISSILEFLYSND);
					SD_PreCacheSound(SD_SPLITFIRESND);
					SD_PreCacheSound(SD_SPLITSND);
					PreCacheGroup(W_GetNumForName("SPLIT1"),
									  W_GetNumForName("SPLIT4"),
									  cache_patch_t);
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.missiletotal ++;
					break;
				case 56:
               #if (SHAREWARE == 1)
                 Error("\n tried to spawn kes at %d,%d in shareware !",i,j);
               #endif



               SD_PreCacheSound(SD_GRAVSND);
					SD_PreCacheSound(SD_GRAVHITSND);
					SD_PreCacheSound(SD_GRAVFIRESND);
					SD_PreCacheSound(SD_GRAVBUILDSND);

					PreCacheGroup(W_GetNumForName("KES1"),
									  W_GetNumForName("KES6"),
									  cache_patch_t);
					PreCacheGroup(W_GetNumForName("KSPHERE1"),
									  W_GetNumForName("KSPHERE4"),
									  cache_patch_t);
						SpawnStatic(i,j,tile-23,spawnz);
					if (loadedgame == false)
						gamestate.missiletotal ++;
					break;

				case 57:
				case 58:
				case 59:
				case 60:
				case 61:
				case 62:
				case 65:
				case 66:
				case 67:
						SpawnStatic(i,j,tile-23,spawnz);
					break;

				case 68:
				case 69:
				case 70:
				case 71:
						SpawnStatic(i,j,tile-23,spawnz);
					break;

				case  98:
						SpawnStatic(i,j,tile-98+49,spawnz);
					break;

				case 210:
						SpawnStatic(i,j,stat_scotthead,spawnz);
					break;

				case 228:
				case 229:
				case 230:
				case 231:
				case 232:
				case 233:
						SpawnStatic(i,j,tile-228+51,spawnz);
					break;
				case 246:
				case 247:
				case 248:
				case 249:
				case 250:
				case 251:
						SpawnStatic(i,j,tile-246+57,spawnz);
					break;
				case 264:
				case 265:
						SpawnStatic(i,j,tile-264+63,spawnz);
					gamestate.planttotal ++;
					break;

				case 266:
						SpawnStatic(i,j,stat_urn,spawnz);
					break;

            case 268:
               SpawnStatic(i,j,tile-265+63,spawnz);
               break;

            case 269:
						SpawnStatic(i,j,tile-265+63,spawnz);
					break;

				case 267:
						SpawnStatic(i,j,stat_emptystatue,spawnz);
					break;

				case 282:
					SpawnStatic(i,j,stat_heatgrate,spawnz);
					break;

				case 283:
					SpawnStatic(i,j,stat_standardpole,spawnz);
					break;

            case 284:
               if ( !BATTLEMODE )
                  {
                  SpawnStatic(i,j,stat_pit,spawnz);
                  }
               break;




				case 252:
					SD_PreCacheSound(SD_GRAVSND);
					SD_PreCacheSound(SD_GRAVHITSND);
               SD_PreCacheSoundGroup(SD_GODMODEFIRESND,SD_LOSEMODESND);
               if ((locplayerstate->player == 1) || (locplayerstate->player ==3))
					  SD_PreCacheSound(SD_GODWOMANSND);
					else
					  SD_PreCacheSound(SD_GODMANSND);


					PreCacheGroup(W_GetNumForName("GODHAND1"),
									  W_GetNumForName("GODHAND8"),
									  cache_patch_t);

					PreCacheGroup(W_GetNumForName("VAPO1"),
									  W_GetNumForName("LITSOUL"),
									  cache_patch_t);

					PreCacheGroup(W_GetNumForName("GODFIRE1"),
									  W_GetNumForName("GODFIRE4"),
									  cache_patch_t);

						SpawnStatic(i,j,stat_godmode,spawnz);
					if (loadedgame == false)
						gamestate.supertotal ++;
					break;

				case 253:

               #if (SHAREWARE == 1)
                  Error("DogMode Power up in shareware at x=%d y=%d\n",i,j);
               #endif

               SD_PreCacheSoundGroup(SD_DOGMODEPANTSND,SD_DOGMODELICKSND);
               if ((locplayerstate->player == 1) || (locplayerstate->player ==3))
					  SD_PreCacheSound(SD_DOGWOMANSND);
					else
					  SD_PreCacheSound(SD_DOGMANSND);



					PreCacheGroup(W_GetNumForName("DOGNOSE1"),
									  W_GetNumForName("DOGPAW4"),
									  cache_patch_t);
						SpawnStatic(i,j,stat_dogmode,spawnz);
					if (loadedgame == false)
						gamestate.supertotal ++;
					break;

				case 254:
						SpawnStatic(i,j,stat_fleetfeet,spawnz);
					if (loadedgame == false)
						gamestate.supertotal ++;
					break;

				case 255:
						SpawnStatic(i,j,stat_random,spawnz);
					if (loadedgame == false)
						gamestate.supertotal ++;
					break;

				case 260:
						SpawnStatic(i,j,stat_elastic,spawnz);
					if (loadedgame == false)
						gamestate.supertotal ++;
					break;

				case 261:
						SpawnStatic(i,j,stat_mushroom,spawnz);
					if (loadedgame == false)
						{
						gamestate.supertotal ++;
						gamestate.democratictotal ++;
						}
					break;


				case 262:
						SpawnStatic(i,j,stat_tomlarva,spawnz);
					break;

				case 263:
						if (gamestate.SpawnCollectItems)
                     {
							SpawnStatic(i,j,stat_collector,spawnz);
                     LASTSTAT->flags |= FL_COLORED;
                     LASTSTAT->hitpoints =
                        ( GameRandomNumber("colors",0) % MAXPLAYERCOLORS );
                     BATTLE_NumCollectorItems++;
                     }
					break;

				case 270:
						SpawnStatic(i,j,stat_bulletproof,spawnz);
					if (loadedgame == false)
						gamestate.supertotal ++;
					break;
				case 271:
						SpawnStatic(i,j,stat_asbesto,spawnz);
					if (loadedgame == false)
						gamestate.supertotal ++;
					break;
				case 272:
						SpawnStatic(i,j,stat_gasmask,spawnz);
					if (loadedgame == false)
						gamestate.supertotal ++;
					break;
				case 461:
						SpawnStatic(i,j,stat_disk,spawnz);
					break;
				 }
			}
		}
   }



void RaiseSprites( int x, int y, int count, int dir )
{
	int a,c;
	int dx,dy;
	int h;
   int i;
   int xx;
   int hc;
   int d;

   dx=0;
   dy=0;
   if (dir==1)
      dx=1;
   else
      dy=1;


   if (((statobj_t *)sprites[x][y])->z==-65)
      {
      c=(maxheight+20)<<8;
      hc=(count+1)<<7;
      a=(c<<8)/(hc*hc);
      for (i=0;i<count;i++)
         {
         xx=-hc+((i+1)<<8);
         h=(c-FixedMulShift(a,(xx*xx),8) )>>8;
         ((statobj_t *)sprites[x+(dx*i)][y+(dy*i)])->z=maxheight-h;
         }
      }
   else
      {
      if (ActorIsSpring(x-(dx),y-(dy)))
         d=1;
      else if (ActorIsSpring(x+(dx*count),y+(dy*count)))
         d=0;
      else
         Error("Cannot find a spring board around a ramp ascension near x=%d y=%d\n",x,y);

      hc=((maxheight+20)<<16)/(count+1);
      h=hc<<1;
      for (i=0;i<count;i++)
         {
         if (d==1)
            ((statobj_t *)sprites[x+(dx*i)][y+(dy*i)])->z=maxheight-(h>>16);
         else
            ((statobj_t *)sprites[x+(dx*(count-i-1))][y+(dy*(count-i-1))])->z=maxheight-(h>>16);
         h+=hc;
         }
      }
}

void LoftSprites( void )
{
   int x,y;
   int count;

   for(y=1;y<mapheight-1;y++)
      {
    	for(x=1;x<mapwidth-1;x++)
         {
         if (StaticUndefined(x,y))
            {
            if (StaticUndefined(x+1,y))
               {
               count=1;
               while (StaticUndefined(x+count,y))
                  count++;
               if (count<3)
                  Error ("Are You kidding me? You are trying to loft <3 sprites in an arc??? \n x=%d y=%d\n",x,y);
               RaiseSprites(x,y,count,1);
               }
            else if (StaticUndefined(x,y+1))
               {
               count=1;
               while (StaticUndefined(x,y+count))
                  count++;
               if (count<3)
                  Error ("Are You kidding me? You are trying to loft <3 sprites??? \n x=%d y=%d\n",x,y);
               RaiseSprites(x,y,count,0);
               }
            else
               Error ("Sprite Lofter is confused around x=%d y=%d\n",x,y);
            }
         }
      }
}

