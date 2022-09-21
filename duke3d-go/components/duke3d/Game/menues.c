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

#include "duke3d.h"
#include "mouse.h"
#include "animlib.h"
#include "control.h"

#include "filesystem.h"
#include "SDL.h"
#include "premap.h"
#include "display.h"

static const char* TAG = "menues";

extern SDL_Surface *surface;
extern short inputloc;
extern int recfilep;
extern uint8_t  vgacompatible;
short probey=0,lastprobey=0,last_menu,globalskillsound=-1;
short sh,onbar,buttonstat,deletespot;
short last_zero,last_fifty,last_threehundred = 0;
static uint8_t  fileselect = 1, menunamecnt;
EXT_RAM_ATTR static char menuname[256][17];

// File tree info
//
//uint8_t  szCurrentDirectory[1024] = {'\0'};

#define FILETYPE_DIRECTORY 0
#define FILETYPE_FILE      1

typedef struct fileentry
{
	uint8_t  filename[16];
	int type;
}FILEENTRY;

typedef struct filelist
{
	FILEENTRY *files;
}FILELIST;

FILELIST m_Files;// = NULL;

//
//

#define MENU_SELECT_EPISODE	100
#define	MENU_USER_MAP		101


// CTW - REMOVED
/* Error codes */
/*
#define eTenBnNotInWindows 3801
#define eTenBnBadGameIni 3802
#define eTenBnBadTenIni 3803
#define eTenBnBrowseCancel 3804
#define eTenBnBadTenInst 3805

int  tenBnStart(void);
void tenBnSetBrowseRtn(uint8_t  *(*rtn)(uint8_t  *str, int len));
void tenBnSetExitRtn(void (*rtn)(void));
void tenBnSetEndRtn(void (*rtn)(void));*/
// CTW END - REMOVED

void dummyfunc(void)
{
}

void dummymess(int i,uint8_t  *c)
{
}

// CTW - REMOVED
/*
void TENtext(void)
{
    int32_t dacount,dalastcount;

    puts("\nDuke Nukem 3D has been licensed exclusively to TEN (Total");
    puts("Entertainment Network) for wide-area networked (WAN) multiplayer");
    puts("games.\n");

    puts("The multiplayer code within Duke Nukem 3D has been highly");
    puts("customized to run best on TEN, where you'll experience fast and");
    puts("stable performance, plus other special benefits.\n");

    puts("We do not authorize or recommend the use of Duke Nukem 3D with");
    puts("gaming services other than TEN.\n");

    puts("Duke Nukem 3D is protected by United States copyright law and");
    puts("international treaty.\n");

    puts("For the best online multiplayer gaming experience, please call TEN");
    puts("at 800-8040-TEN, or visit TEN's Web Site at www.ten.net.\n");

    puts("Press any key to continue.\n");

    _bios_timeofday(0,&dacount);

    while( _bios_keybrd(1) == 0 )
    {
        _bios_timeofday(0,&dalastcount);
        if( (dacount+240) < dalastcount ) break;
    }
}
*/
// CTW END - REMOVED

void cmenu(short cm)
{
    current_menu = cm;

    if( (cm >= 1000 && cm <= 1010) )
        return;

    if( cm == 0 )
        probey = last_zero;
    else if(cm == 50)
        probey = last_fifty;
    else if(cm >= 300 && cm < 400)
        probey = last_threehundred;
    else if(cm == 110)
        probey = 1;
    else probey = 0;
    lastprobey = -1;
}


void savetemp(char  *fn,uint8_t* daptr,int32_t dasiz)
{
    int fp;

    fp = open(fn,O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);

    write(fp,(uint8_t  *)daptr,dasiz);

    close(fp);
}

void getangplayers(short snum)
{
    short i,a;

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        if(i != snum)
        {
            a = ps[snum].ang+getangle(ps[i].posx-ps[snum].posx,ps[i].posy-ps[snum].posy);
            a = a-1024;
            rotatesprite(
                (320<<15) + (((sintable[(a+512)&2047])>>7)<<15),
                (320<<15) - (((sintable[a&2047])>>8)<<15),
                klabs(sintable[((a>>1)+768)&2047]<<2),0,APLAYER,0,ps[i].palookup,0,0,0,xdim-1,ydim-1);
        }
    }
}

int loadpheader(uint8_t  spot,int32 *vn,int32 *ln,int32 *psk,int32 *nump)
{

    char  fn[] = "game0.sav";
    int32_t fil;
    int32_t bv;

         fn[4] = spot+'0';

     if ((fil = TCkopen4load(fn,0)) == -1) return(-1);

     tiles[MAXTILES-3].lock = 255;

     kdfread(&bv,4,1,fil);
     if(bv != BYTEVERSION)
     {
        FTA(114,&ps[myconnectindex],1);
        kclose(fil);
        return 1;
     }

     kdfread(nump,sizeof(int32),1,fil);

     kdfread(tempbuf,19,1,fil);
         kdfread(vn,sizeof(int32),1,fil);
         kdfread(ln,sizeof(int32),1,fil);
     kdfread(psk,sizeof(int32),1,fil);

     if (tiles[MAXTILES-3].data == NULL)
         allocache(&tiles[MAXTILES-3].data,160*100,&tiles[MAXTILES-3].lock);
    tiles[MAXTILES-3].dim.width = 100;
    tiles[MAXTILES-3].dim.height = 160;
    kdfread(tiles[MAXTILES-3].data,160,100,fil);
    kclose(fil);
    return(0);
}


int loadplayer(int8_t spot)
{
     short k,music_changed;
     char  fn[] = "game0.sav";
     char  mpfn[] = "gameA_00.sav";
     char  *fnptr, scriptptrs[MAXSCRIPTSIZE];
     int32_t fil, bv, i, j, x;
     int32 nump;

     if(spot < 0)
     {
        multiflag = 1;
        multiwhat = 0;
        multipos = -spot-1;
        return -1;
     }

     if( multiflag == 2 && multiwho != myconnectindex )
     {
         fnptr = mpfn;
         mpfn[4] = spot + 'A';

         if(ud.multimode > 9)
         {
             mpfn[6] = (multiwho/10) + '0';
             mpfn[7] = (multiwho%10) + '0';
         }
         else mpfn[7] = multiwho + '0';
     }
     else
     {
        fnptr = fn;
        fn[4] = spot + '0';
     }

     if ((fil = TCkopen4load(fnptr,0)) == -1) return(-1);

	 if(ud.recstat != 2)
		ready2send = 0;

     kdfread(&bv,4,1,fil);
     if(bv != BYTEVERSION)
     {
        FTA(114,&ps[myconnectindex],1);
        kclose(fil);
		if(ud.recstat != 2)
		{	
			// FIX_00084: Various bugs in the load game (single player) option if ESC is hit or if wrong version 
			ototalclock = totalclock;
			ready2send = 1;
		}
        return 1;
     }

     kdfread(&nump,sizeof(nump),1,fil);
     if(nump != numplayers)
     {
        kclose(fil);
		if(ud.recstat != 2)
		{
			// FIX_00084: Various bugs in the load game (single player) option if ESC is hit or if wrong version 
	        ototalclock = totalclock;
	        ready2send = 1;
		}
        FTA(124,&ps[myconnectindex],1);
        return 1;
     }

     if(numplayers > 1)
     {
         pub = NUMPAGES;
         pus = NUMPAGES;
         vscrn();
         drawbackground();
         menutext(160,100,0,0,"LOADING...");
         nextpage();
    }

     waitforeverybody();

         FX_StopAllSounds();
     clearsoundlocks();
         MUSIC_StopSong();

     if(numplayers > 1)
         kdfread(&buf,19,1,fil);
     else
         kdfread(&ud.savegame[spot][0],19,1,fil);

     music_changed = (music_select != (ud.volume_number*11) + ud.level_number);

         kdfread(&ud.volume_number,sizeof(ud.volume_number),1,fil);
         kdfread(&ud.level_number,sizeof(ud.level_number),1,fil);
         kdfread(&ud.player_skill,sizeof(ud.player_skill),1,fil);

         ud.m_level_number = ud.level_number;
         ud.m_volume_number = ud.volume_number;
         ud.m_player_skill = ud.player_skill;

                 //Fake read because lseek won't work with compression
     tiles[MAXTILES-3].lock = 1;
    
     if (tiles[MAXTILES-3].data == NULL)
         allocache(&tiles[MAXTILES-3].data,160*100,&tiles[MAXTILES-3].lock);
    
     tiles[MAXTILES-3].dim.width = 100;
    tiles[MAXTILES-3].dim.height = 160;
    
     kdfread(tiles[MAXTILES-3].data,160,100,fil);

         kdfread(&numwalls,2,1,fil);
     kdfread(&wall[0],sizeof(walltype),MAXWALLS,fil);
         kdfread(&numsectors,2,1,fil);
     kdfread(&sector[0],sizeof(sectortype),MAXSECTORS,fil);
         kdfread(&sprite[0],sizeof(spritetype),MAXSPRITES,fil);
         kdfread(&headspritesect[0],2,MAXSECTORS+1,fil);
         kdfread(&prevspritesect[0],2,MAXSPRITES,fil);
         kdfread(&nextspritesect[0],2,MAXSPRITES,fil);
         kdfread(&headspritestat[0],2,MAXSTATUS+1,fil);
         kdfread(&prevspritestat[0],2,MAXSPRITES,fil);
         kdfread(&nextspritestat[0],2,MAXSPRITES,fil);
         kdfread(&numcyclers,sizeof(numcyclers),1,fil);
         kdfread(&cyclers[0][0],12,MAXCYCLERS,fil);
     kdfread(ps,sizeof(ps),1,fil);
     kdfread(po,sizeof(po),1,fil);
         kdfread(&numanimwalls,sizeof(numanimwalls),1,fil);
         kdfread(&animwall,sizeof(animwall),1,fil);
         kdfread(&msx[0],sizeof(int32_t),sizeof(msx)/sizeof(int32_t),fil);
         kdfread(&msy[0],sizeof(int32_t),sizeof(msy)/sizeof(int32_t),fil);
     kdfread((short *)&spriteqloc,sizeof(short),1,fil);
     kdfread((short *)&spriteqamount,sizeof(short),1,fil);
     kdfread((short *)&spriteq[0],sizeof(short),spriteqamount,fil);
         kdfread(&mirrorcnt,sizeof(short),1,fil);
         kdfread(&mirrorwall[0],sizeof(short),64,fil);
     kdfread(&mirrorsector[0],sizeof(short),64,fil);
     kdfread(&show2dsector[0],sizeof(uint8_t ),MAXSECTORS>>3,fil);
     kdfread(&actortype[0],sizeof(uint8_t ),MAXTILES,fil);
     kdfread(&boardfilename[0],sizeof(boardfilename),1,fil);

     kdfread(&numclouds,sizeof(numclouds),1,fil);
     kdfread(&clouds[0],sizeof(short)<<7,1,fil);
     kdfread(&cloudx[0],sizeof(short)<<7,1,fil);
     kdfread(&cloudy[0],sizeof(short)<<7,1,fil);

     kdfread(&scriptptrs[0],1,MAXSCRIPTSIZE,fil);
     kdfread(&script[0],4,MAXSCRIPTSIZE,fil);
     for(i=0;i<MAXSCRIPTSIZE;i++)
        if( scriptptrs[i] )
     {
         j = (int32_t)script[i]+(int32_t)&script[0];
         script[i] = j;
     }

     kdfread(&actorscrptr[0],4,MAXTILES,fil);
     for(i=0;i<MAXTILES;i++)
         if(actorscrptr[i])
     {
        j = (int32_t)actorscrptr[i]+(int32_t)&script[0];
        actorscrptr[i] = (int32_t *)j;
     }

     kdfread(&scriptptrs[0],1,MAXSPRITES,fil);
     kdfread(&hittype[0],sizeof(struct weaponhit),MAXSPRITES,fil);

     for(i=0;i<MAXSPRITES;i++)
     {
        j = (int32_t)(&script[0]);
        if( scriptptrs[i]&1 ) T2 += j;
        if( scriptptrs[i]&2 ) T5 += j;
        if( scriptptrs[i]&4 ) T6 += j;
     }

         kdfread(&lockclock,sizeof(lockclock),1,fil);
     kdfread(&pskybits,sizeof(pskybits),1,fil);
     kdfread(&pskyoff[0],sizeof(pskyoff[0]),MAXPSKYTILES,fil);

         kdfread(&animatecnt,sizeof(animatecnt),1,fil);
         kdfread(&animatesect[0],2,MAXANIMATES,fil);
         kdfread(&animateptr[0],4,MAXANIMATES,fil);
     for(i = animatecnt-1;i>=0;i--) animateptr[i] = (int32_t *)((int32_t)animateptr[i]+(int32_t)(&sector[0]));
         kdfread(&animategoal[0],4,MAXANIMATES,fil);
         kdfread(&animatevel[0],4,MAXANIMATES,fil);

         kdfread(&earthquaketime,sizeof(earthquaketime),1,fil);
     kdfread(&ud.from_bonus,sizeof(ud.from_bonus),1,fil);
     kdfread(&ud.secretlevel,sizeof(ud.secretlevel),1,fil);
     kdfread(&ud.respawn_monsters,sizeof(ud.respawn_monsters),1,fil);
     ud.m_respawn_monsters = ud.respawn_monsters;
     kdfread(&ud.respawn_items,sizeof(ud.respawn_items),1,fil);
     ud.m_respawn_items = ud.respawn_items;
     kdfread(&ud.respawn_inventory,sizeof(ud.respawn_inventory),1,fil);
     ud.m_respawn_inventory = ud.respawn_inventory;

     kdfread(&ud.god,sizeof(ud.god),1,fil);
     kdfread(&ud.auto_run,sizeof(ud.auto_run),1,fil);
     kdfread(&ud.crosshair,sizeof(ud.crosshair),1,fil);
     kdfread(&ud.monsters_off,sizeof(ud.monsters_off),1,fil);
     ud.m_monsters_off = ud.monsters_off;
     kdfread(&ud.last_level,sizeof(ud.last_level),1,fil);
     kdfread(&ud.eog,sizeof(ud.eog),1,fil);

     kdfread(&ud.coop,sizeof(ud.coop),1,fil);
     ud.m_coop = ud.coop;
     kdfread(&ud.marker,sizeof(ud.marker),1,fil);
     ud.m_marker = ud.marker;
     kdfread(&ud.ffire,sizeof(ud.ffire),1,fil);
     ud.m_ffire = ud.ffire;

     kdfread(&camsprite,sizeof(camsprite),1,fil);
     kdfread(&connecthead,sizeof(connecthead),1,fil);
     kdfread(connectpoint2,sizeof(connectpoint2),1,fil);
     kdfread(&numplayersprites,sizeof(numplayersprites),1,fil);
     kdfread((short *)&frags[0][0],sizeof(frags),1,fil);

     kdfread(&randomseed,sizeof(randomseed),1,fil);
     kdfread(&global_random,sizeof(global_random),1,fil);
     kdfread(&parallaxyscale,sizeof(parallaxyscale),1,fil);

     kclose(fil);

     if(ps[myconnectindex].over_shoulder_on != 0)
     {
         cameradist = 0;
         cameraclock = 0;
         ps[myconnectindex].over_shoulder_on = 1;
     }

     screenpeek = myconnectindex;

     clearbufbyte(gotpic,sizeof(gotpic),0L);
     clearsoundlocks();
         cacheit();
     docacheit();

     if(music_changed == 0)
        music_select = (ud.volume_number*11) + ud.level_number;
     playmusic(&music_fn[0][music_select][0]);

     ps[myconnectindex].gm = MODE_GAME;
         ud.recstat = 0;

     if(ps[myconnectindex].jetpack_on)
         spritesound(DUKE_JETPACK_IDLE,ps[myconnectindex].i);

     restorepalette = 1;
     setpal(&ps[myconnectindex]);
     vscrn();

     FX_SetReverb(0);

     if(ud.lockout == 0)
     {
         for(x=0;x<numanimwalls;x++)
             if( wall[animwall[x].wallnum].extra >= 0 )
                 wall[animwall[x].wallnum].picnum = wall[animwall[x].wallnum].extra;
     }
     else
     {
         for(x=0;x<numanimwalls;x++)
             switch(wall[animwall[x].wallnum].picnum)
         {
             case FEMPIC1:
                 wall[animwall[x].wallnum].picnum = BLANKSCREEN;
                 break;
             case FEMPIC2:
             case FEMPIC3:
                 wall[animwall[x].wallnum].picnum = SCREENBREAK6;
                 break;
         }
     }

     numinterpolations = 0;
     startofdynamicinterpolations = 0;

     k = headspritestat[3];
     while(k >= 0)
     {
        switch(sprite[k].lotag)
        {
            case 31:
                setinterpolation(&sector[sprite[k].sectnum].floorz);
                break;
            case 32:
                setinterpolation(&sector[sprite[k].sectnum].ceilingz);
                break;
            case 25:
                setinterpolation(&sector[sprite[k].sectnum].floorz);
                setinterpolation(&sector[sprite[k].sectnum].ceilingz);
                break;
            case 17:
                setinterpolation(&sector[sprite[k].sectnum].floorz);
                setinterpolation(&sector[sprite[k].sectnum].ceilingz);
                break;
            case 0:
            case 5:
            case 6:
            case 11:
            case 14:
            case 15:
            case 16:
            case 26:
            case 30:
                setsectinterpolate(k);
                break;
        }

        k = nextspritestat[k];
     }

     for(i=numinterpolations-1;i>=0;i--) bakipos[i] = *curipos[i];
     for(i = animatecnt-1;i>=0;i--)
         setinterpolation(animateptr[i]);

     show_shareware = 0;
     everyothertime = 0;

     clearbufbyte(playerquitflag,MAXPLAYERS,0x01010101);

     resetmys();

     ready2send = 1;

     flushpackets();
     clearfifo();
     waitforeverybody();

     resettimevars();

     return(0);
}

int saveplayer(int8_t spot)
{
     int32_t i, j;
     char  fn[] = "game0.sav";
     char  mpfn[] = "gameA_00.sav";
     char  *fnptr,scriptptrs[MAXSCRIPTSIZE];
         FILE *fil;
     int32_t bv = BYTEVERSION;
	 char  fullpathsavefilename[16];

     if(spot < 0)
     {
        multiflag = 1;
        multiwhat = 1;
        multipos = -spot-1;
        return -1;
     }

     waitforeverybody();

     if( multiflag == 2 && multiwho != myconnectindex )
     {
         fnptr = mpfn;
         mpfn[4] = spot + 'A';

         if(ud.multimode > 9)
         {
             mpfn[6] = (multiwho/10) + '0';
             mpfn[7] = multiwho + '0';
         }
         else mpfn[7] = multiwho + '0';
     }
     else
     {
        fnptr = fn;
        fn[4] = spot + '0';
     }


	// Are we loading a TC?
	if(getGameDir()[0] != '\0')
	{
		// Yes
		sprintf(fullpathsavefilename, "%s\\%s", getGameDir(), fnptr);
	}
	else
	{
		// No 
		sprintf(fullpathsavefilename, "%s", fnptr);
	}

     if ((fil = fopen(fullpathsavefilename,"wb")) == 0) return(-1);

     ready2send = 0;

     dfwrite(&bv,4,1,fil);
     dfwrite(&ud.multimode,sizeof(ud.multimode),1,fil);

         dfwrite(&ud.savegame[spot][0],19,1,fil);
         dfwrite(&ud.volume_number,sizeof(ud.volume_number),1,fil);
     dfwrite(&ud.level_number,sizeof(ud.level_number),1,fil);
         dfwrite(&ud.player_skill,sizeof(ud.player_skill),1,fil);
     dfwrite(tiles[MAXTILES-1].data,160,100,fil);

         dfwrite(&numwalls,2,1,fil);
     dfwrite(&wall[0],sizeof(walltype),MAXWALLS,fil);
         dfwrite(&numsectors,2,1,fil);
     dfwrite(&sector[0],sizeof(sectortype),MAXSECTORS,fil);
         dfwrite(&sprite[0],sizeof(spritetype),MAXSPRITES,fil);
         dfwrite(&headspritesect[0],2,MAXSECTORS+1,fil);
         dfwrite(&prevspritesect[0],2,MAXSPRITES,fil);
         dfwrite(&nextspritesect[0],2,MAXSPRITES,fil);
         dfwrite(&headspritestat[0],2,MAXSTATUS+1,fil);
         dfwrite(&prevspritestat[0],2,MAXSPRITES,fil);
         dfwrite(&nextspritestat[0],2,MAXSPRITES,fil);
         dfwrite(&numcyclers,sizeof(numcyclers),1,fil);
         dfwrite(&cyclers[0][0],12,MAXCYCLERS,fil);
     dfwrite(ps,sizeof(ps),1,fil);
     dfwrite(po,sizeof(po),1,fil);
         dfwrite(&numanimwalls,sizeof(numanimwalls),1,fil);
         dfwrite(&animwall,sizeof(animwall),1,fil);
         dfwrite(&msx[0],sizeof(int32_t),sizeof(msx)/sizeof(int32_t),fil);
         dfwrite(&msy[0],sizeof(int32_t),sizeof(msy)/sizeof(int32_t),fil);
     dfwrite(&spriteqloc,sizeof(short),1,fil);
     dfwrite(&spriteqamount,sizeof(short),1,fil);
     dfwrite(&spriteq[0],sizeof(short),spriteqamount,fil);
         dfwrite(&mirrorcnt,sizeof(short),1,fil);
         dfwrite(&mirrorwall[0],sizeof(short),64,fil);
         dfwrite(&mirrorsector[0],sizeof(short),64,fil);
     dfwrite(&show2dsector[0],sizeof(uint8_t ),MAXSECTORS>>3,fil);
     dfwrite(&actortype[0],sizeof(uint8_t ),MAXTILES,fil);
     dfwrite(&boardfilename[0],sizeof(boardfilename),1,fil);

     dfwrite(&numclouds,sizeof(numclouds),1,fil);
     dfwrite(&clouds[0],sizeof(short)<<7,1,fil);
     dfwrite(&cloudx[0],sizeof(short)<<7,1,fil);
     dfwrite(&cloudy[0],sizeof(short)<<7,1,fil);

     for(i=0;i<MAXSCRIPTSIZE;i++)
     {
          if( (int32_t)script[i] >= (int32_t)(&script[0]) && (int32_t)script[i] < (int32_t)(&script[MAXSCRIPTSIZE]) )
          {
                scriptptrs[i] = 1;
                j = (int32_t)script[i] - (int32_t)&script[0];
                script[i] = j;
          }
          else scriptptrs[i] = 0;
     }

     dfwrite(&scriptptrs[0],1,MAXSCRIPTSIZE,fil);
     dfwrite(&script[0],4,MAXSCRIPTSIZE,fil);

     for(i=0;i<MAXSCRIPTSIZE;i++)
        if( scriptptrs[i] )
     {
        j = script[i]+(int32_t)&script[0];
        script[i] = j;
     }

     for(i=0;i<MAXTILES;i++)
         if(actorscrptr[i])
     {
        j = (int32_t)actorscrptr[i]-(int32_t)&script[0];
        actorscrptr[i] = (int32_t *)j;
     }
     dfwrite(&actorscrptr[0],4,MAXTILES,fil);
     for(i=0;i<MAXTILES;i++)
         if(actorscrptr[i])
     {
         j = (int32_t)actorscrptr[i]+(int32_t)&script[0];
         actorscrptr[i] = (int32_t *)j;
     }

     for(i=0;i<MAXSPRITES;i++)
     {
        scriptptrs[i] = 0;

        if(actorscrptr[PN] == 0) continue;

        j = (int32_t)&script[0];

        if(T2 >= j && T2 < (int32_t)(&script[MAXSCRIPTSIZE]) )
        {
            scriptptrs[i] |= 1;
            T2 -= j;
        }
        if(T5 >= j && T5 < (int32_t)(&script[MAXSCRIPTSIZE]) )
        {
            scriptptrs[i] |= 2;
            T5 -= j;
        }
        if(T6 >= j && T6 < (int32_t)(&script[MAXSCRIPTSIZE]) )
        {
            scriptptrs[i] |= 4;
            T6 -= j;
        }
    }

    dfwrite(&scriptptrs[0],1,MAXSPRITES,fil);
    dfwrite(&hittype[0],sizeof(struct weaponhit),MAXSPRITES,fil);

    for(i=0;i<MAXSPRITES;i++)
    {
        if(actorscrptr[PN] == 0) continue;
        j = (int32_t)&script[0];

        if(scriptptrs[i]&1)
            T2 += j;
        if(scriptptrs[i]&2)
            T5 += j;
        if(scriptptrs[i]&4)
            T6 += j;
    }

         dfwrite(&lockclock,sizeof(lockclock),1,fil);
     dfwrite(&pskybits,sizeof(pskybits),1,fil);
     dfwrite(&pskyoff[0],sizeof(pskyoff[0]),MAXPSKYTILES,fil);
         dfwrite(&animatecnt,sizeof(animatecnt),1,fil);
         dfwrite(&animatesect[0],2,MAXANIMATES,fil);
         for(i = animatecnt-1;i>=0;i--) animateptr[i] = (int32_t *)((int32_t)animateptr[i]-(int32_t)(&sector[0]));
         dfwrite(&animateptr[0],4,MAXANIMATES,fil);
         for(i = animatecnt-1;i>=0;i--) animateptr[i] = (int32_t *)((int32_t)animateptr[i]+(int32_t)(&sector[0]));
         dfwrite(&animategoal[0],4,MAXANIMATES,fil);
         dfwrite(&animatevel[0],4,MAXANIMATES,fil);

         dfwrite(&earthquaketime,sizeof(earthquaketime),1,fil);
         dfwrite(&ud.from_bonus,sizeof(ud.from_bonus),1,fil);
     dfwrite(&ud.secretlevel,sizeof(ud.secretlevel),1,fil);
     dfwrite(&ud.respawn_monsters,sizeof(ud.respawn_monsters),1,fil);
     dfwrite(&ud.respawn_items,sizeof(ud.respawn_items),1,fil);
     dfwrite(&ud.respawn_inventory,sizeof(ud.respawn_inventory),1,fil);
     dfwrite(&ud.god,sizeof(ud.god),1,fil);
     dfwrite(&ud.auto_run,sizeof(ud.auto_run),1,fil);
     dfwrite(&ud.crosshair,sizeof(ud.crosshair),1,fil);
     dfwrite(&ud.monsters_off,sizeof(ud.monsters_off),1,fil);
     dfwrite(&ud.last_level,sizeof(ud.last_level),1,fil);
     dfwrite(&ud.eog,sizeof(ud.eog),1,fil);
     dfwrite(&ud.coop,sizeof(ud.coop),1,fil);
     dfwrite(&ud.marker,sizeof(ud.marker),1,fil);
     dfwrite(&ud.ffire,sizeof(ud.ffire),1,fil);
     dfwrite(&camsprite,sizeof(camsprite),1,fil);
     dfwrite(&connecthead,sizeof(connecthead),1,fil);
     dfwrite(connectpoint2,sizeof(connectpoint2),1,fil);
     dfwrite(&numplayersprites,sizeof(numplayersprites),1,fil);
     dfwrite((short *)&frags[0][0],sizeof(frags),1,fil);

     dfwrite(&randomseed,sizeof(randomseed),1,fil);
     dfwrite(&global_random,sizeof(global_random),1,fil);
     dfwrite(&parallaxyscale,sizeof(parallaxyscale),1,fil);

         fclose(fil);

     if(ud.multimode < 2)
     {
         strcpy(fta_quotes[122],"GAME SAVED");
         FTA(122,&ps[myconnectindex],1);
     }

     ready2send = 1;

     waitforeverybody();

     ototalclock = totalclock;

     return(0);
}

#define LMB (buttonstat&1)
#define RMB (buttonstat&2)

// FIX_00036: Mouse wheel can now be used in menu
#define WHEELUP (buttonstat&8)
#define WHEELDOWN (buttonstat&16)

ControlInfo minfo;

int probe(int x,int y,int i,int n)
{
	return( probeXduke(x, y, i, n, 65536L) );
}

int probeXduke(int x,int y,int i,int n, int32_t spriteSize)
{
    short centre;
	int32 mouseSens;

	static int32_t delay_counter_up = 0, delay_counter_down = 0, delay_up = 50, delay_down = 50;
	static int32_t mi = 0;

	// FIX_00075: Bad Sensitivity aint32_t Y axis when using mouse in menu (Thanks to Turrican)
	mouseSens = CONTROL_GetMouseSensitivity_Y();
	mouseSens = mouseSens ? mouseSens : 1;

    if( ((ControllerType == controltype_keyboardandmouse)||
		(ControllerType == controltype_joystickandmouse)) )
		//&& CONTROL_MousePresent )
    {
		memset(&minfo, 0, sizeof(ControlInfo));

        CONTROL_GetInput( &minfo );
		//mouseY = CONTROL_GetMouseY();
		//mi = mouseY;
        mi += minfo.dz / mouseSens;
		mi += minfo.dpitch / mouseSens;
    }

    else minfo.dz = minfo.dyaw = 0;

    if( x == (320>>1) )
        centre = 320>>2;
    else centre = 0;

        if( KB_KeyPressed( sc_UpArrow ) || KB_KeyPressed( sc_PgUp ) || KB_KeyPressed( sc_kpad_8 ) ||
            (mi < -1024) || WHEELUP)
        {
			// FIX_00060: Repeat key function was not working in the menu
			if(delay_counter_up==0 || (totalclock-delay_counter_up)>delay_up || (mi < -1024) || WHEELUP)
			{
				mi = 0;
				sound(KICK_HIT);

				probey--;
				if(probey < 0) probey = n-1;
				minfo.dz = 0;
				minfo.dpitch = 0;
				if (delay_counter_up && (totalclock-delay_counter_up)>delay_up)
					delay_up = 10;
				delay_counter_up = totalclock;
			}
        }
		else
		{
            KB_ClearKeyDown( sc_UpArrow );
            KB_ClearKeyDown( sc_kpad_8 );
            KB_ClearKeyDown( sc_PgUp );
			delay_counter_up = 0;
			delay_up = 50;
		}

        if( KB_KeyPressed( sc_DownArrow ) || KB_KeyPressed( sc_PgDn ) || KB_KeyPressed( sc_kpad_2 )
            || (mi > 1024) || WHEELDOWN )
        {
			if(delay_counter_down==0 || (totalclock-delay_counter_down)>delay_down || (mi > 1024) || WHEELDOWN)
			{
				mi = 0;
				sound(KICK_HIT);
				probey++;
				minfo.dz = 0;
				minfo.dpitch = 0;
				if (delay_counter_down && (totalclock-delay_counter_down)>delay_down)
					delay_down = 10;
				delay_counter_down = totalclock;
			}
        }
		else
		{
			KB_ClearKeyDown( sc_DownArrow );
			KB_ClearKeyDown( sc_kpad_2 );
			KB_ClearKeyDown( sc_PgDn );
			delay_counter_down = 0;
			delay_down = 50;
		}

    if(probey >= n)
        probey = 0;

    if(centre)
    {
//        rotatesprite(((320>>1)+(centre)+54)<<16,(y+(probey*i)-4)<<16,65536L,0,SPINNINGNUKEICON+6-((6+(totalclock>>3))%7),sh,0,10,0,0,xdim-1,ydim-1);
//        rotatesprite(((320>>1)-(centre)-54)<<16,(y+(probey*i)-4)<<16,65536L,0,SPINNINGNUKEICON+((totalclock>>3)%7),sh,0,10,0,0,xdim-1,ydim-1);

        rotatesprite(((320>>1)+(centre>>1)+70)<<16,(y+(probey*i)-4)<<16,spriteSize,0,SPINNINGNUKEICON+6-((6+(totalclock>>3))%7),sh,0,10,0,0,xdim-1,ydim-1);
        rotatesprite(((320>>1)-(centre>>1)-70)<<16,(y+(probey*i)-4)<<16,spriteSize,0,SPINNINGNUKEICON+((totalclock>>3)%7),sh,0,10,0,0,xdim-1,ydim-1);
    }
    else
        rotatesprite((x-tiles[BIGFNTCURSOR].dim.width-4)<<16,(y+(probey*i)-4)<<16,spriteSize,0,SPINNINGNUKEICON+(((totalclock>>3))%7),sh,0,10,0,0,xdim-1,ydim-1);

    if( KB_KeyPressed(sc_Space) || KB_KeyPressed( sc_kpad_Enter ) || KB_KeyPressed( sc_Enter ) || (LMB))// && !onbar) )
    {
        if(current_menu != 110)
            sound(PISTOL_BODYHIT);
        KB_ClearKeyDown( sc_Enter );
        KB_ClearKeyDown( sc_Space );
        KB_ClearKeyDown( sc_kpad_Enter );
        return(probey);
    }
    else if( KB_KeyPressed( sc_Escape ) || (RMB) )
    {
        onbar = 0;
        KB_ClearKeyDown( sc_Escape );
        sound(EXITMENUSOUND);
        return(-1);
    }
    else
    {
        if(onbar == 0) return(-probey-2);
        if ( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -128 ) )
            return(probey);
        else if ( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 128 ) )
            return(probey);
        else return(-probey-2);
    }
}

int menutext(int x,int y,short s,short p,char  *t)
{
    short i, ac, centre;

    y -= 12;

    i = centre = 0;

    if( x == (320>>1) )
    {
        while( *(t+i) )
        {
            if(*(t+i) == ' ')
            {
                centre += 5;
                i++;
                continue;
            }
            ac = 0;
            if(*(t+i) >= '0' && *(t+i) <= '9')
                ac = *(t+i) - '0' + BIGALPHANUM-10;
            else if(*(t+i) >= 'a' && *(t+i) <= 'z')
                ac = toupper(*(t+i)) - 'A' + BIGALPHANUM;
            else if(*(t+i) >= 'A' && *(t+i) <= 'Z')
                ac = *(t+i) - 'A' + BIGALPHANUM;
            else switch(*(t+i))
            {
                case '-':
                    ac = BIGALPHANUM-11;
                    break;
                case '.':
                    ac = BIGPERIOD;
                    break;
                case '\'':
                    ac = BIGAPPOS;
                    break;
                case ',':
                    ac = BIGCOMMA;
                    break;
                case '!':
                    ac = BIGX;
                    break;
                case '?':
                    ac = BIGQ;
                    break;
                case ';':
                    ac = BIGSEMI;
                    break;
                case ':':
                    ac = BIGSEMI;
                    break;
                default:
                    centre += 5;
                    i++;
                    continue;
            }

            centre += tiles[ac].dim.width-1;
            i++;
        }
    }

    if(centre)
        x = (320-centre-10)>>1;

    while(*t)
    {
        if(*t == ' ') {x+=5;t++;continue;}
        ac = 0;
        if(*t >= '0' && *t <= '9')
            ac = *t - '0' + BIGALPHANUM-10;
        else if(*t >= 'a' && *t <= 'z')
            ac = toupper(*t) - 'A' + BIGALPHANUM;
        else if(*t >= 'A' && *t <= 'Z')
            ac = *t - 'A' + BIGALPHANUM;
        else switch(*t)
        {
            case '-':
                ac = BIGALPHANUM-11;
                break;
            case '.':
                ac = BIGPERIOD;
                break;
            case ',':
                ac = BIGCOMMA;
                break;
            case '!':
                ac = BIGX;
                break;
            case '\'':
                ac = BIGAPPOS;
                break;
            case '?':
                ac = BIGQ;
                break;
            case ';':
                ac = BIGSEMI;
                break;
            case ':':
                ac = BIGCOLIN;
                break;
            default:
                x += 5;
                t++;
                continue;
        }

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,10+16,0,0,xdim-1,ydim-1);

        x += tiles[ac].dim.width;
        t++;
    }
    return (x);
}

int menutextc(int x,int y,short s,short p,char  *t)
{
    short i, ac, centre;

    s += 8;
    y -= 12;

    i = centre = 0;

//    if( x == (320>>1) )
    {
        while( *(t+i) )
        {
            if(*(t+i) == ' ')
            {
                centre += 5;
                i++;
                continue;
            }
            ac = 0;
            if(*(t+i) >= '0' && *(t+i) <= '9')
                ac = *(t+i) - '0' + BIGALPHANUM+26+26;
            if(*(t+i) >= 'a' && *(t+i) <= 'z')
                ac = *(t+i) - 'a' + BIGALPHANUM+26;
            if(*(t+i) >= 'A' && *(t+i) <= 'Z')
                ac = *(t+i) - 'A' + BIGALPHANUM;

            else switch(*t)
            {
                case '-':
                    ac = BIGALPHANUM-11;
                    break;
                case '.':
                    ac = BIGPERIOD;
                    break;
                case ',':
                    ac = BIGCOMMA;
                    break;
                case '!':
                    ac = BIGX;
                    break;
                case '?':
                    ac = BIGQ;
                    break;
                case ';':
                    ac = BIGSEMI;
                    break;
                case ':':
                    ac = BIGCOLIN;
                    break;
            }

            centre += tiles[ac].dim.width-1;
            i++;
        }
    }

    x -= centre>>1;

    while(*t)
    {
        if(*t == ' ') {x+=5;t++;continue;}
        ac = 0;
        if(*t >= '0' && *t <= '9')
            ac = *t - '0' + BIGALPHANUM+26+26;
        if(*t >= 'a' && *t <= 'z')
            ac = *t - 'a' + BIGALPHANUM+26;
        if(*t >= 'A' && *t <= 'Z')
            ac = *t - 'A' + BIGALPHANUM;
        switch(*t)
        {
            case '-':
                ac = BIGALPHANUM-11;
                break;
            case '.':
                ac = BIGPERIOD;
                break;
            case ',':
                ac = BIGCOMMA;
                break;
            case '!':
                ac = BIGX;
                break;
            case '?':
                ac = BIGQ;
                break;
            case ';':
                ac = BIGSEMI;
                break;
            case ':':
                ac = BIGCOLIN;
                break;
        }

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,10+16,0,0,xdim-1,ydim-1);

        x += tiles[ac].dim.width;
        t++;
    }
    return (x);
}


void bar(int x,int y,short *p,short dainc,uint8_t  damodify,short s, short pa)
{
    short xloc;
    uint8_t  rev;

    if(dainc < 0) { dainc = -dainc; rev = 1; }
    else rev = 0;
    y-=2;

    if(damodify)
    {
        if(rev == 0)
        {
            if( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -256 ) ) // && onbar) )
            {
                KB_ClearKeyDown( sc_LeftArrow );
                KB_ClearKeyDown( sc_kpad_4 );

                *p -= dainc;
                if(*p < 0)
                    *p = 0;
                sound(KICK_HIT);
            }
            if( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 256 ) )//&& onbar) )
            {
                KB_ClearKeyDown( sc_RightArrow );
                KB_ClearKeyDown( sc_kpad_6 );

                *p += dainc;
                if(*p > 63)
                    *p = 63;
                sound(KICK_HIT);
            }
        }
        else
        {
            if( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 256 ))//&& onbar ))
            {
                KB_ClearKeyDown( sc_RightArrow );
                KB_ClearKeyDown( sc_kpad_6 );

                *p -= dainc;
                if(*p < 0)
                    *p = 0;
                sound(KICK_HIT);
            }
            if( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -256 ))// && onbar) )
            {
                KB_ClearKeyDown( sc_LeftArrow );
                KB_ClearKeyDown( sc_kpad_4 );

                *p += dainc;
                if(*p > 64)
                    *p = 64;
                sound(KICK_HIT);
            }
        }
    }

    xloc = *p;

    rotatesprite( (x+22)<<16,(y-3)<<16,65536L,0,SLIDEBAR,s,pa,10,0,0,xdim-1,ydim-1);
    if(rev == 0)
        rotatesprite( (x+xloc+1)<<16,(y+1)<<16,65536L,0,SLIDEBAR+1,s,pa,10,0,0,xdim-1,ydim-1);
    else
        rotatesprite( (x+(65-xloc) )<<16,(y+1)<<16,65536L,0,SLIDEBAR+1,s,pa,10,0,0,xdim-1,ydim-1);
}

#define SHX(X) 0
// ((x==X)*(-sh))
#define PHX(X) 0
// ((x==X)?1:2)
#define MWIN(X) rotatesprite( 320<<15,200<<15,X,0,MENUSCREEN,-16,0,10+64,0,0,xdim-1,ydim-1)
#define MWINXY(X,OX,OY) rotatesprite( ( 320+(OX) )<<15, ( 200+(OY) )<<15,X,0,MENUSCREEN,-16,0,10+64,0,0,xdim-1,ydim-1)


int32 volnum,levnum,plrskl,numplr;
short lastsavedpos = -1;

void dispnames(void)
{
    short x, c = 160;

    c += 64;
    for(x = 0;x <= 108;x += 12)
    rotatesprite((c+91-64)<<16,(x+56)<<16,65536L,0,TEXTBOX,24,0,10,0,0,xdim-1,ydim-1);

    rotatesprite(22<<16,97<<16,65536L,0,WINDOWBORDER2,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(180<<16,97<<16,65536L,1024,WINDOWBORDER2,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(99<<16,50<<16,65536L,512,WINDOWBORDER1,24,0,10,0,0,xdim-1,ydim-1);
    rotatesprite(103<<16,144<<16,65536L,1024+512,WINDOWBORDER1,24,0,10,0,0,xdim-1,ydim-1);

    minitext(c,48,ud.savegame[0],2,10+16);
    minitext(c,48+12,ud.savegame[1],2,10+16);
    minitext(c,48+12+12,ud.savegame[2],2,10+16);
    minitext(c,48+12+12+12,ud.savegame[3],2,10+16);
    minitext(c,48+12+12+12+12,ud.savegame[4],2,10+16);
    minitext(c,48+12+12+12+12+12,ud.savegame[5],2,10+16);
    minitext(c,48+12+12+12+12+12+12,ud.savegame[6],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12,ud.savegame[7],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12+12,ud.savegame[8],2,10+16);
    minitext(c,48+12+12+12+12+12+12+12+12+12,ud.savegame[9],2,10+16);

}


// This is the same thing as was in build.c ...
//  We DO have a _dos_findfirst implementation now...maybe use that instead?
//     --ryan.
#if ORIGINAL_DUKE3D_GETFILENAMES
int getfilenames(uint8_t  kind[6])
{
        short type;
        struct find_t fileinfo;

        if (strcmp(kind,"SUBD") == 0)
        {
                strcpy(kind,"*.*");
                if (_dos_findfirst(kind,_A_SUBDIR,&fileinfo) != 0)
                        return(-1);
                type = 1;
        }
        else
        {
                if (_dos_findfirst(kind,_A_NORMAL,&fileinfo) != 0)
                        return(-1);
                type = 0;
        }
        do
        {
                if ((type == 0) || ((fileinfo.attrib&16) > 0))
                        if ((fileinfo.name[0] != '.') || (fileinfo.name[1] != 0))
                        {
                                strcpy(menuname[menunamecnt],fileinfo.name);
                                menuname[menunamecnt][16] = type;
                                menunamecnt++;
                        }
        }
        while (_dos_findnext(&fileinfo) == 0);

        return(0);
}

#else

int getfilenames(char  kind[6])
{
/* !!! FIXME: Visual C? */
#if (defined __WATCOMC__)
	short type;
	struct find_t fileinfo;

	if (strcmp(kind,"SUBD") == 0)
	{
		strcpy(kind,"*.*");
		if (_dos_findfirst(kind,_A_SUBDIR,&fileinfo) != 0)
			return(-1);
		type = 1;
	}
	else
	{
		if (_dos_findfirst(kind,_A_NORMAL,&fileinfo) != 0)
			return(-1);
		type = 0;
	}
	do
	{
		if ((type == 0) || ((fileinfo.attrib&16) > 0))
			if ((fileinfo.name[0] != '.') || (fileinfo.name[1] != 0))
			{
				strcpy(menuname[menunamecnt],fileinfo.name);
				menuname[menunamecnt][16] = type;
				menunamecnt++;
			}
	}
	while (_dos_findnext(&fileinfo) == 0);

#elif (defined PLATFORM_UNIX)

    DIR *dir;
    struct dirent *dent;
    struct stat statbuf;
    int add_this;
    uint8_t  *ptr = NULL;
    int len = 0;
    int subdirs = 0;

	if (strcmp(kind,"SUBD") == 0)
        subdirs = 1;

    dir = opendir(".");
    if (dir == NULL)
        return(-1);

    do
    {
        add_this = 0;
        dent = readdir(dir);
        if (dent != NULL)
        {
            if (stat(dent->d_name, &statbuf) == 0)
            {
                if (subdirs)
                {
                    if (S_ISDIR(statbuf.st_mode))
                    add_this = 1;
                } /* if */
                else
                {
                    /* need to expand support if this assertion ever fails. */
                    assert(stricmp(kind, "*.MAP") == 0);
                    len = strlen(dent->d_name);
                    if (len >= 5)
                    {
                        ptr = ((uint8_t  *) dent->d_name) + len;
                        ptr += strlen(ptr) - 4;
                        if (stricmp(ptr, ".MAP") == 0)
                    add_this = 1;
                    } /* if */
                } /* else */

                if (add_this)
    			{
	    			strcpy(menuname[menunamecnt],dent->d_name);
		    		menuname[menunamecnt][16] = subdirs;
			    	menunamecnt++;
                } /* if */
            } /* if */
        } /* if */
    } while (dent != NULL);

    closedir(dir);

#endif
	return(0);
}

#endif


void sortfilenames()
{
        uint8_t  sortbuffer[17];
        int32_t i, j, k;

        for(i=1;i<menunamecnt;i++)
                for(j=0;j<i;j++)
                {
                         k = 0;
                         while ((menuname[i][k] == menuname[j][k]) && (menuname[i][k] != 0) && (menuname[j][k] != 0))
                                 k++;
                        if (menuname[i][k] < menuname[j][k])
                        {
                                memcpy(&sortbuffer[0],&menuname[i][0],sizeof(menuname[0]));
                                memcpy(&menuname[i][0],&menuname[j][0],sizeof(menuname[0]));
                                memcpy(&menuname[j][0],&sortbuffer[0],sizeof(menuname[0]));
                        }
                }
}

int32_t quittimer = 0;

void gameexitanycase(void)
{
    KB_FlushKeyboardQueue();

    if( gamequit == 0 && ( numplayers > 1 ) )
    {
        if(ps[myconnectindex].gm&MODE_GAME)
        {
            gamequit = 1;
            quittimer = totalclock+120;
			sound(SHORT_CIRCUIT);
        }
        else
        {
            sendlogoff();
            gameexit(" ");
			sound(NITEVISION_ONOFF);
        }
    }
    else if( numplayers < 2 )
        gameexit(" ");

    if( ( totalclock > quittimer ) && ( gamequit == 1) )
	{
        gameexit("Timed out.");
	}
}


void menus(void)
{
    short c,x;
    volatile int32_t l;
	int i,j;

	static int lastkeysetup = 0;
	static int waiting4key = false;
	static int current_resolution = 0;
    char text[512];
    
    // JK - with limited buttons, change key bindings in the menu
    if( (ps[myconnectindex].gm&MODE_MENU) == 0 )
    {
        keyMode = 0;
    } else {
        keyMode = 1;
    }

    getpackets();

    if(((ControllerType == controltype_keyboardandmouse)||
		(ControllerType == controltype_joystickandmouse)) 
		//&& CONTROL_MousePresent
		)
    {
		
        if(buttonstat != 0 && !onbar) // anti-repeat
        {
            x = MOUSE_GetButtons()<<3;
            if( x ) 
			{
				buttonstat = x<<3;
			}
            else 
			{
				buttonstat = 0;
			}
        }
        else
			
            buttonstat = MOUSE_GetButtons();
    }
    else buttonstat = 0;

    if( (ps[myconnectindex].gm&MODE_MENU) == 0 )
    {
        tiles[MAXTILES-3].lock = 1;
        return;
    }

    ps[myconnectindex].gm &= (0xff-MODE_TYPE);
    ps[myconnectindex].fta = 0;

    x = 0;

    sh = 4-(sintable[(totalclock<<4)&2047]>>11);

    if(!(current_menu >= 1000 && current_menu <= 2999 && current_menu >= 300 && current_menu <= 369))
        vscrn();

	// printf("Current menu=%d, game mode=%d, last menu =%d\n", current_menu, ps[myconnectindex].gm, last_menu);

    switch(current_menu)
    {
        case 25000:
            gametext(160,90,"SELECT A SAVE SPOT BEFORE",0,2+8+16);
            gametext(160,90+9,"YOU QUICK RESTORE.",0,2+8+16);

            x = probe(186,124,0,0);
            if(x >= -1)
            {
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
                ps[myconnectindex].gm &= ~MODE_MENU;
            }
            break;

        case 20000:
            x = probe(326,190,0,0);
            gametext(160,50-8,"YOU ARE PLAYING THE SHAREWARE",0,2+8+16);
            gametext(160,59-8,"VERSION OF DUKE NUKEM 3D.  WHILE",0,2+8+16);
            gametext(160,68-8,"THIS VERSION IS REALLY COOL, YOU",0,2+8+16);
            gametext(160,77-8,"ARE MISSING OVER 75% OF THE TOTAL",0,2+8+16);
            gametext(160,86-8,"GAME, ALONG WITH OTHER GREAT EXTRAS",0,2+8+16);
            gametext(160,95-8,"AND GAMES, WHICH YOU'LL GET WHEN",0,2+8+16);
            gametext(160,104-8,"YOU ORDER THE COMPLETE VERSION AND",0,2+8+16);
            gametext(160,113-8,"GET THE FINAL TWO EPISODES.",0,2+8+16);

            gametext(160,113+8,"PLEASE READ THE 'HOW TO ORDER' ITEM",0,2+8+16);
            gametext(160,122+8,"ON THE MAIN MENU IF YOU WISH TO",0,2+8+16);
            gametext(160,131+8,"UPGRADE TO THE FULL REGISTERED",0,2+8+16);
            gametext(160,140+8,"VERSION OF DUKE NUKEM 3D.",0,2+8+16);
            gametext(160,149+16,"PRESS ANY KEY...",0,2+8+16);

            if( x >= -1 ) cmenu(100);
            break;


        case 15001:
        case 15000:

            gametext(160,90,"LOAD last game:",0,2+8+16);

            sprintf(text,"\"%s\"",ud.savegame[lastsavedpos]);
            gametext(160,99,text,0,2+8+16);

            gametext(160,99+9,"(Y/N)",0,2+8+16);

			_handle_events();
            if( KB_KeyPressed(sc_Escape) || KB_KeyPressed(sc_N) || RMB)
            {
                if(sprite[ps[myconnectindex].i].extra <= 0)
                {
                    enterlevel(MODE_GAME);
                    return;
                }

                KB_ClearKeyDown(sc_N);
                KB_ClearKeyDown(sc_Escape);

                ps[myconnectindex].gm &= ~MODE_MENU;
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
            }

            if(  KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();
                FX_StopAllSounds();

                if(ud.multimode > 1)
                {
                    loadplayer(-1-lastsavedpos);
                    ps[myconnectindex].gm = MODE_GAME;
                }
                else
                {
                    c = loadplayer(lastsavedpos);
                    if(c == 0)
                        ps[myconnectindex].gm = MODE_GAME;
                }
            }

            probe(186,124+9,0,0);

            break;

        case 10000:  // parental
        case 10001:

            c = (320>>1)-120;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"ADULT MODE");

            x = probe(c+6,43,16,2);

            if(x == -1) 
			{ 
				cmenu(702); 
				probey = 6;			
				break; 
			}

            menutext(c,43,SHX(-2),PHX(-2),"ADULT MODE");
			menutext(c+160+40,43,0,0,ud.lockout?"OFF":"ON");
			
			menutext(c,43+16,SHX(-3),PHX(-3),"ENTER PASSWORD");

            if(current_menu == 10001)
            {
                gametext(160,43+16+16+16-12,"ENTER PASSWORD",0,2+8+16);
                x = strget((320>>1),43+16+16+16,buf,19, 998);

                if(x == 1) // user hit enter key
                {
                    if(ud.pwlockout[0] == 0 || ud.lockout == 0 ) // if already unlocked then we set pwd or reset pwd is there is already one
                        strcpy(&ud.pwlockout[0],buf);
                    else if( strcmp(buf,&ud.pwlockout[0]) == 0 ) // if (pwd was up AND lockout is true (locked) AND pwd is good)
                    {
                        ud.lockout = 0;
                        buf[0] = 0;

                        for(x=0;x<numanimwalls;x++)
                            if( wall[animwall[x].wallnum].picnum != W_SCREENBREAK &&
                                wall[animwall[x].wallnum].picnum != W_SCREENBREAK+1 &&
                                wall[animwall[x].wallnum].picnum != W_SCREENBREAK+2 )
                                    if( wall[animwall[x].wallnum].extra >= 0 )
                                        wall[animwall[x].wallnum].picnum = wall[animwall[x].wallnum].extra;

                    }
                    current_menu = 10000;
                    KB_ClearKeyDown(sc_Enter);
                    KB_ClearKeyDown(sc_kpad_Enter);
                    KB_FlushKeyboardQueue();
                }
				else if(x==-1) // pressed esc while typing. We discard the text.
				{
					*buf = 0; 
					current_menu = 10000;
					KB_ClearKeyDown(sc_Escape);
				}
			}
            else
            {
                if(x == 0)
                {
                    if( ud.lockout == 1 )
                    {
                        if(ud.pwlockout[0] == 0)
                        {
                            ud.lockout = 0;
                            for(x=0;x<numanimwalls;x++)
                            if( wall[animwall[x].wallnum].picnum != W_SCREENBREAK &&
                                wall[animwall[x].wallnum].picnum != W_SCREENBREAK+1 &&
                                wall[animwall[x].wallnum].picnum != W_SCREENBREAK+2 )
                                    if( wall[animwall[x].wallnum].extra >= 0 )
                                        wall[animwall[x].wallnum].picnum = wall[animwall[x].wallnum].extra;
                        }
                        else
                        {
                            buf[0] = 0;
                            current_menu = 10001;
                            inputloc = 0;
                            KB_FlushKeyboardQueue();
                        }
                    }
                    else
                    {
                        ud.lockout = 1;

                        for(x=0;x<numanimwalls;x++)
                            switch(wall[animwall[x].wallnum].picnum)
                            {
                                case FEMPIC1:
                                    wall[animwall[x].wallnum].picnum = BLANKSCREEN;
                                    break;
                                case FEMPIC2:
                                case FEMPIC3:
                                    wall[animwall[x].wallnum].picnum = SCREENBREAK6;
                                    break;
                            }
                    }
                }

                else if(x == 1)
                {
                    current_menu = 10001; // ask for password change
                    inputloc = 0;
					*buf = 0;
                    KB_FlushKeyboardQueue();
                }
            }

            break;

        case 1000:
        case 1001:
        case 1002:
        case 1003:
        case 1004:
        case 1005:
        case 1006:
        case 1007:
        case 1008:
        case 1009:

            rotatesprite(160<<16,200<<15,65536L,0,MENUSCREEN,16,0,10+64,0,0,xdim-1,ydim-1);
            rotatesprite(160<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,24,0,0,"LOAD GAME");
            rotatesprite(101<<16,97<<16,65536,512,MAXTILES-3,-32,0,4+10+64,0,0,xdim-1,ydim-1);

            dispnames();

            sprintf((char*)tempbuf,"PLAYERS: %-2d                      ",numplr);
            gametext(160,158,(char*)tempbuf,0,2+8+16);

            sprintf((char*)tempbuf,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+volnum,1+levnum,plrskl);
            gametext(160,170,(char*)tempbuf,0,2+8+16);

            gametext(160,90,"LOAD game:",0,2+8+16);
            sprintf((char*)tempbuf,"\"%s\"",ud.savegame[current_menu-1000]);
            gametext(160,99,(char*)tempbuf,0,2+8+16);
            gametext(160,99+9,"(Y/N)",0,2+8+16);

			_handle_events();
            if( KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                lastsavedpos = current_menu-1000;

                KB_FlushKeyboardQueue();
                //if(ud.multimode < 2 && ud.recstat != 2)
                //{
                //    ready2send = 1;
                //    totalclock = ototalclock;
                //}

                if(ud.multimode > 1)
                {
                    if( ps[myconnectindex].gm&MODE_GAME )
                    {
                        loadplayer(-1-lastsavedpos);
                        ps[myconnectindex].gm = MODE_GAME;
                    }
                    else
                    {
                        tempbuf[0] = 126;
                        tempbuf[1] = lastsavedpos;
                        for(x=connecthead;x>=0;x=connectpoint2[x])
                            if(x != myconnectindex)
                                sendpacket(x,(uint8_t*)tempbuf,2);

                        getpackets();

                        loadplayer(lastsavedpos);

                        multiflag = 0;
                    }
                }
                else
                {
					// FIX_00084: Various bugs in the load game (single player) option if ESC is hit or if wrong version 

                    c = loadplayer(lastsavedpos);
                    if(c == 0)
					{
                        ps[myconnectindex].gm = MODE_GAME;
						if (ud.recstat != 2) // if not playing a demo then ..
							totalclock = ototalclock;
					}
					else
						cmenu(1010); // failed loading game
                }

                break;
            }

            if( KB_KeyPressed(sc_N) || KB_KeyPressed(sc_Escape) || RMB)
            {
                KB_ClearKeyDown(sc_N);
                KB_ClearKeyDown(sc_Escape);
                sound(EXITMENUSOUND);
				cmenu(300);
				// FIX_00084: Various bugs in the load game (single player) option if ESC is hit or if wrong version 
				// simply get back w/o changing anything.

            }

            break;

		case 1010: //loading a saved game failed

			gametext(160,99,"YOU CAN'T LOAD THIS GAME",0,2+8+16);
			gametext(160,99+9,"EITHER A WONG VERSION",0,2+8+16);
			gametext(160,99+9+9,"OR BAD # OF PLAYERS OR...",0,2+8+16);

			_handle_events();
			if( KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Escape) || KB_KeyPressed(sc_Enter)
				|| RMB)            {
                KB_ClearKeyDown(sc_Space);
                KB_ClearKeyDown(sc_Escape);
				KB_ClearKeyDown(sc_Enter);
                sound(EXITMENUSOUND);
				cmenu(300);
			}

			break;

        case 1500:
			_handle_events();
            if( KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();
                cmenu(100);
            }
            if( KB_KeyPressed(sc_N) || KB_KeyPressed(sc_Escape) || RMB)
            {
                KB_ClearKeyDown(sc_N);
                KB_ClearKeyDown(sc_Escape);
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
                ps[myconnectindex].gm &= ~MODE_MENU;
                sound(EXITMENUSOUND);
                break;
            }
            probe(186,124,0,0);
            gametext(160,90,"ABORT this game?",0,2+8+16);
            gametext(160,90+9,"(Y/N)",0,2+8+16);

            break;

        case 2000:
        case 2001:
        case 2002:
        case 2003:
        case 2004:
        case 2005:
        case 2006:
        case 2007:
        case 2008:
        case 2009:

            rotatesprite(160<<16,200<<15,65536L,0,MENUSCREEN,16,0,10+64,0,0,xdim-1,ydim-1);
            rotatesprite(160<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,24,0,0,"SAVE GAME");

            rotatesprite(101<<16,97<<16,65536L,512,MAXTILES-3,-32,0,4+10+64,0,0,xdim-1,ydim-1);
            sprintf(text,"PLAYERS: %-2d                      ",ud.multimode);
            gametext(160,158,text,0,2+8+16);

            sprintf(text,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+ud.volume_number,1+ud.level_number,ud.player_skill);
            gametext(160,170,text,0,2+8+16);

            dispnames();

            gametext(160,90,"OVERWRITE previous SAVED game?",0,2+8+16);
            gametext(160,90+9,"(Y/N)",0,2+8+16);

			_handle_events();
            if( KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();
                inputloc = strlen(&ud.savegame[current_menu-2000][0]);

                cmenu(current_menu-2000+360);

                KB_FlushKeyboardQueue();
                break;
            }
            if( KB_KeyPressed(sc_N) || KB_KeyPressed(sc_Escape) || RMB)
            {
                KB_ClearKeyDown(sc_N);
                KB_ClearKeyDown(sc_Escape);
                cmenu(351);
                sound(EXITMENUSOUND);
            }

            probe(186,124,0,0);

            break;

        case 990:  // credits
        case 991:
        case 992:
        case 993:
        case 994:
        case 995:
        case 996:
        case 997:
			c = 160;
			if (!PLUTOPAK) {
            	//rotatesprite(c<<16,200<<15,65536L,0,MENUSCREEN,16,0,10+64,0,0,xdim-1,ydim-1);
            	rotatesprite(c<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            	menutext(c,24,0,0,"CREDITS");

				l = 7;
			} else {
				l = 2;
			}

            if(KB_KeyPressed(sc_Escape)) { cmenu(0); break; }

            if( KB_KeyPressed( sc_LeftArrow ) ||
                KB_KeyPressed( sc_kpad_4 ) ||
                KB_KeyPressed( sc_UpArrow ) ||
                KB_KeyPressed( sc_PgUp ) ||
                KB_KeyPressed( sc_kpad_8 ) )
            {
                KB_ClearKeyDown(sc_LeftArrow);
                KB_ClearKeyDown(sc_kpad_4);
                KB_ClearKeyDown(sc_UpArrow);
                KB_ClearKeyDown(sc_PgUp);
                KB_ClearKeyDown(sc_kpad_8);

                sound(KICK_HIT);
                current_menu--;
                if(current_menu < 990) current_menu = 990+l;
            }
            else if(
                KB_KeyPressed( sc_PgDn ) ||
                KB_KeyPressed( sc_Enter ) ||
                KB_KeyPressed( sc_Space ) ||
                KB_KeyPressed( sc_kpad_Enter ) ||
                KB_KeyPressed( sc_RightArrow ) ||
                KB_KeyPressed( sc_DownArrow ) ||
                KB_KeyPressed( sc_kpad_2 ) ||
                KB_KeyPressed( sc_kpad_9 ) ||
                KB_KeyPressed( sc_kpad_6 ) )
            {
                KB_ClearKeyDown(sc_PgDn);
                KB_ClearKeyDown(sc_Enter);
                KB_ClearKeyDown(sc_RightArrow);
                KB_ClearKeyDown(sc_kpad_Enter);
                KB_ClearKeyDown(sc_kpad_6);
                KB_ClearKeyDown(sc_kpad_9);
                KB_ClearKeyDown(sc_kpad_2);
                KB_ClearKeyDown(sc_DownArrow);
                KB_ClearKeyDown(sc_Space);
                sound(KICK_HIT);
                current_menu++;
                if(current_menu > 990+l) current_menu = 990;
            }

			if (!PLUTOPAK) { // This is Jonathon Fowler code. Text respects the true 1.3/1.3d --mk
			switch (current_menu) {
				case 990:
					gametext(c,40,                      "ORIGINAL CONCEPT",0,2+8+16);
					gametext(c,40+9,                    "TODD REPLOGLE",0,2+8+16);
					gametext(c,40+9+9,                  "ALLEN H. BLUM III",0,2+8+16);
						
					gametext(c,40+9+9+9+9,              "PRODUCED & DIRECTED BY",0,2+8+16);
					gametext(c,40+9+9+9+9+9,            "GREG MALONE",0,2+8+16);

					gametext(c,40+9+9+9+9+9+9+9,        "EXECUTIVE PRODUCER",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9+9+9,      "GEORGE BROUSSARD",0,2+8+16);

					gametext(c,40+9+9+9+9+9+9+9+9+9+9,  "BUILD ENGINE",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9+9+9+9+9+9,"KEN SILVERMAN",0,2+8+16);
					break;
				case 991:
					gametext(c,40,                      "GAME PROGRAMMING",0,2+8+16);
					gametext(c,40+9,                    "TODD REPLOGLE",0,2+8+16);
						
					gametext(c,40+9+9+9,                "3D ENGINE/TOOLS/NET",0,2+8+16);
					gametext(c,40+9+9+9+9,              "KEN SILVERMAN",0,2+8+16);

					gametext(c,40+9+9+9+9+9+9,          "NETWORK LAYER/SETUP PROGRAM",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9+9,        "MARK DOCHTERMANN",0,2+8+16);
					break;
				case 992:
					gametext(c,40,                      "MAP DESIGN",0,2+8+16);
					gametext(c,40+9,                    "ALLEN H BLUM III",0,2+8+16);
					gametext(c,40+9+9,                  "RICHARD GRAY",0,2+8+16);
						
					gametext(c,40+9+9+9+9,              "3D MODELING",0,2+8+16);
					gametext(c,40+9+9+9+9+9,            "CHUCK JONES",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9,          "SAPPHIRE CORPORATION",0,2+8+16);

					gametext(c,40+9+9+9+9+9+9+9+9,      "ARTWORK",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9+9+9+9,    "DIRK JONES, STEPHEN HORNBACK",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9+9+9+9+9,  "JAMES STOREY, DAVID DEMARET",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9+9+9+9+9+9,"DOUGLAS R WOOD",0,2+8+16);
					break;
				case 993:
					gametext(c,40,                      "SOUND ENGINE",0,2+8+16);
					gametext(c,40+9,                    "JIM DOSE",0,2+8+16);
						
					gametext(c,40+9+9+9,                "SOUND & MUSIC DEVELOPMENT",0,2+8+16);
					gametext(c,40+9+9+9+9,              "ROBERT PRINCE",0,2+8+16);
					gametext(c,40+9+9+9+9+9,            "LEE JACKSON",0,2+8+16);

					gametext(c,40+9+9+9+9+9+9+9,        "VOICE TALENT",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9+9+9,      "LANI MINELLA - VOICE PRODUCER",0,2+8+16);
					gametext(c,40+9+9+9+9+9+9+9+9+9,    "JON ST. JOHN AS \"DUKE NUKEM\"",0,2+8+16);
					break;
				case 994:
					gametext(c,60,                      "GRAPHIC DESIGN",0,2+8+16);
					gametext(c,60+9,                    "PACKAGING, MANUAL, ADS",0,2+8+16);
					gametext(c,60+9+9,                  "ROBERT M. ATKINS",0,2+8+16);
					gametext(c,60+9+9+9,                "MICHAEL HADWIN",0,2+8+16);
					
					gametext(c,60+9+9+9+9+9,            "SPECIAL THANKS TO",0,2+8+16);
					gametext(c,60+9+9+9+9+9+9,          "STEVEN BLACKBURN, TOM HALL",0,2+8+16);
					gametext(c,60+9+9+9+9+9+9+9,        "SCOTT MILLER, JOE SIEGLER",0,2+8+16);
					gametext(c,60+9+9+9+9+9+9+9+9,      "TERRY NAGY, COLLEEN COMPTON",0,2+8+16);
					gametext(c,60+9+9+9+9+9+9+9+9+9,    "HASH INC., FORMGEN, INC.",0,2+8+16);
					break;
				case 995:
					gametext(c,49,                      "THE 3D REALMS BETA TESTERS",0,2+8+16);

					gametext(c,49+9+9,                  "NATHAN ANDERSON, WAYNE BENNER",0,2+8+16);
					gametext(c,49+9+9+9,                "GLENN BRENSINGER, ROB BROWN",0,2+8+16);
					gametext(c,49+9+9+9+9,              "ERIK HARRIS, KEN HECKBERT",0,2+8+16);
					gametext(c,49+9+9+9+9+9,            "TERRY HERRIN, GREG HIVELY",0,2+8+16);
					gametext(c,49+9+9+9+9+9+9,          "HANK LEUKART, ERIC BAKER",0,2+8+16);
					gametext(c,49+9+9+9+9+9+9+9,        "JEFF RAUSCH, KELLY ROGERS",0,2+8+16);
					gametext(c,49+9+9+9+9+9+9+9+9,      "MIKE DUNCAN, DOUG HOWELL",0,2+8+16);
					gametext(c,49+9+9+9+9+9+9+9+9+9,    "BILL BLAIR",0,2+8+16);
					break;
				case 996:
					gametext(c,32,                      "COMPANY PRODUCT SUPPORT",0,2+8+16);

					gametext(c,32+9+9,                  "THE FOLLOWING COMPANIES WERE COOL",0,2+8+16);
					gametext(c,32+9+9+9,                "ENOUGH TO GIVE US LOTS OF STUFF",0,2+8+16);
					gametext(c,32+9+9+9+9,              "DURING THE MAKING OF DUKE NUKEM 3D.",0,2+8+16);

					gametext(c,32+9+9+9+9+9+9,          "ALTEC LANSING MULTIMEDIA",0,2+8+16);
					gametext(c,32+9+9+9+9+9+9+9,        "FOR TONS OF SPEAKERS AND THE",0,2+8+16);
					gametext(c,32+9+9+9+9+9+9+9+9,      "THX-LICENSED SOUND SYSTEM",0,2+8+16);
					gametext(c,32+9+9+9+9+9+9+9+9+9,    "FOR INFO CALL 1-800-548-0620",0,2+8+16);
					
					gametext(c,32+9+9+9+9+9+9+9+9+9+9+9,"CREATIVE LABS, INC.",0,2+8+16);

					gametext(c,32+9+9+9+9+9+9+9+9+9+9+9+9+9,"THANKS FOR THE HARDWARE, GUYS.",0,2+8+16);
					break;
				case 997:
					gametext(c,50,                      "DUKE NUKEM IS A TRADEMARK OF",0,2+8+16);
					gametext(c,50+9,                    "3D REALMS ENTERTAINMENT",0,2+8+16);
						
					gametext(c,50+9+9+9,                "DUKE NUKEM",0,2+8+16);
					gametext(c,50+9+9+9+9,              "(C) 1996 3D REALMS ENTERTAINMENT",0,2+8+16);

					if (VOLUMEONE) {
					gametext(c,106,                     "PLEASE READ LICENSE.DOC FOR SHAREWARE",0,2+8+16);
					gametext(c,106+9,                   "DISTRIBUTION GRANTS AND RESTRICTIONS",0,2+8+16);
					}

					gametext(c,VOLUMEONE?134:115,       "MADE IN DALLAS, TEXAS USA",0,2+8+16);
					break;
			}
			}

            switch(current_menu)
            {
                case 990:
                case 991:
                case 992:
                   rotatesprite(160<<16,200<<15,65536L,0,2504+current_menu-990,0,0,10+64,0,0,xdim-1,ydim-1);

                   break;

            }
            break;

        case 0: // main menu
            c = (320>>1);
            rotatesprite(c<<16,28<<16,65536L,0,INGAMEDUKETHREEDEE,0,0,10,0,0,xdim-1,ydim-1);
            rotatesprite((c+100)<<16,36<<16,65536L,0,PLUTOPAKSPRITE+2,(sintable[(totalclock<<4)&2047]>>11),0,2+8,0,0,xdim-1,ydim-1);
            
			x = probe(c,67,16,6);

            if(x >= 0)
            {
				if( ud.multimode > 1 && x == 0 && ud.recstat != 2)
                {
                    if( movesperpacket == 4 && myconnectindex != connecthead )
                        break;

                    last_zero = 0;
                    cmenu( 600 );
                }
                else
                {
                    last_zero = x;
                    switch(x)
                    {
                        case 0:
                            cmenu(100);
                            break;
                        case 1: cmenu(200);break;
                        case 2:
                            if(movesperpacket == 4 && connecthead != myconnectindex)
                                break;
                            cmenu(300);
                            break;
                        case 3: KB_FlushKeyboardQueue();cmenu(400);break;  // help
                        case 4: cmenu(990);break;  // credit
                        case 5: cmenu(501);break;  // quit

                    }
                }
            }

            if(KB_KeyPressed(sc_Q)) cmenu(501);

            if(x == -1)
            {
				// FIX_00069: Hitting Esc at the menu screen shows an empty green screen
				if(ud.recstat == 2) // playing demo
					ps[myconnectindex].gm &= ~MODE_MENU;
            }

            if(movesperpacket == 4)
            {
                if( myconnectindex == connecthead )
                    menutext(c,67,SHX(-2),PHX(-2),"NEW GAME");
                else
                    menutext(c,67,SHX(-2),1,"NEW GAME");
            }
            else
                menutext(c,67,SHX(-2),PHX(-2),"NEW GAME");

            menutext(c,67+16,SHX(-3),PHX(-3),"OPTIONS");

            if(movesperpacket == 4 && connecthead != myconnectindex)
                menutext(c,67+16+16,SHX(-4),1,"LOAD GAME");
            else menutext(c,67+16+16,SHX(-4),PHX(-4),"LOAD GAME");

if(VOLUMEONE)
            menutext(c,67+16+16+16,SHX(-5),PHX(-5),"HOW TO ORDER");
else
            menutext(c,67+16+16+16,SHX(-5),PHX(-5),"HELP");

            menutext(c,67+16+16+16+16,SHX(-6),PHX(-6),"CREDITS");

            menutext(c,67+16+16+16+16+16,SHX(-7),PHX(-7),"QUIT");

            break;
// CTW END - MODIFICATION

        case 50: // general menu as cmenu(0) but for multiplayer games
            c = (320>>1);
            rotatesprite(c<<16,32<<16,65536L,0,INGAMEDUKETHREEDEE,0,0,10,0,0,xdim-1,ydim-1);
            rotatesprite((c+100)<<16,36<<16,65536L,0,PLUTOPAKSPRITE+2,(sintable[(totalclock<<4)&2047]>>11),0,2+8,0,0,xdim-1,ydim-1);
            x = probe(c,67,16,7);
            switch(x)
            {
                case 0:
                    if(movesperpacket == 4 && myconnectindex != connecthead)
                        break;
                    if(ud.multimode < 2 || ud.recstat == 2)
                        cmenu(1500);
                    else
                    {
                        cmenu(600);
                        last_fifty = 0;
                    }
                    break;
                case 1:
                    if(movesperpacket == 4 && connecthead != myconnectindex)
                        break;
                    if(ud.recstat != 2)
                    {
                        last_fifty = 1;
                        cmenu(350);
                        setview(0,0,xdim-1,ydim-1);
                    }
                    break;
                case 2:
                    if(movesperpacket == 4 && connecthead != myconnectindex)
                        break;
                    last_fifty = 2;
                    cmenu(300);
                    break;
                case 3:
                    last_fifty = 3;
                    cmenu(200);
                    break;
                case 4:
                    last_fifty = 4;
                    KB_FlushKeyboardQueue();
                    cmenu(400);
                    break;
                case 5:
                    if(numplayers < 2)
                    {
                        last_fifty = 5;
                        cmenu(503);
                    }
                    break;
                case 6:
                    last_fifty = 6;
                    cmenu(502);
                    break;
                case -1:
                    ps[myconnectindex].gm &= ~MODE_MENU;
                    if(ud.multimode < 2 && ud.recstat != 2)
                    {
                        ready2send = 1;
                        totalclock = ototalclock;
                    }
                    break;
            }

            if( KB_KeyPressed(sc_Q) )
                cmenu(502);

            if(movesperpacket == 4 && connecthead != myconnectindex)
            {
                menutext(c,67+16*0 ,SHX(-2),1,"NEW GAME");
                menutext(c,67+16*1 ,SHX(-3),1,"SAVE GAME");
                menutext(c,67+16*2 ,SHX(-4),1,"LOAD GAME");
            }
            else
            {
                menutext(c,67+16*0 ,SHX(-2),PHX(-2),"NEW GAME");
                menutext(c,67+16*1 ,SHX(-3),PHX(-3),"SAVE GAME");
                menutext(c,67+16*2 ,SHX(-4),PHX(-4),"LOAD GAME");
            }

            menutext(c,67+16*3 ,SHX(-5),PHX(-5),"OPTIONS");
if(VOLUMEONE)
            menutext(c,67+16*4 ,SHX(-6),PHX(-6),"HOW TO ORDER");
else
            menutext(c,67+16*4 ,SHX(-6),PHX(-6)," HELP");

            if(numplayers > 1)
                menutext(c,67+16*5 ,SHX(-7),1,"QUIT TO TITLE");
            else menutext(c,67+16*5 ,SHX(-7),PHX(-7),"QUIT TO TITLE");
            menutext(c,67+16*6,SHX(-8),PHX(-8),"QUIT GAME");

            break;

        case 100: // Title menu
            rotatesprite(160<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,24,0,0,"SELECT AN EPISODE");
if(PLUTOPAK)
{            //////if(boardfilename[0])
            //
			// uncomment this for user map
			//x = probe(160,60,20,5);
			x = probe(160,60,20,4);

            //////else x = probe(160,60,20,4);
}
else
{
            if(boardfilename[0])
                x = probe(160,60,20,4);
            else x = probe(160,60,20,3);
}
            if(x >= 0)
            {

				if (VOLUMEONE)
				{
					if(x > 0)
						cmenu(20000);
					else
					{
						ud.m_volume_number = x;
						ud.m_level_number = 0;
						cmenu(110);
				    }
				}

				else
				{

					if((x == 3 && boardfilename[0])&&!PLUTOPAK)
					{
					    ud.m_volume_number = 0;
					    ud.m_level_number = 7;
					}
					else

					/*
					if(x == 4 && boardfilename[0])
					{
						ud.m_volume_number = 0;
						ud.m_level_number = 7;
					}
					*/

					// USER MAP
					if((x == 4)&&PLUTOPAK)
					{
						//CONSOLE_Printf("MENU_USER_MAP");
						//
                    //[Todo: generate file list starting from .\\maps]")

						cmenu(MENU_USER_MAP); // cmenu(101)
						break;
					}
					else
					{
						ud.m_volume_number = x;
						ud.m_level_number = 0;
					}
					cmenu(110);
				}
            }
            else if(x == -1)
            {
                if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);
                else cmenu(0);
            }

            menutext(160,60,SHX(-2),PHX(-2),volume_names[0]);

            c = 80;
			if (VOLUMEONE)
			{
				menutext(160,60+20,SHX(-3),1,volume_names[1]);
	            menutext(160,60+20+20,SHX(-4),1,volume_names[2]);
				if(PLUTOPAK)
					menutext(160,60+20+20,SHX(-5),1,volume_names[3]);
			}
			else
			{
				menutext(160,60+20,SHX(-3),PHX(-3),volume_names[1]);
				menutext(160,60+20+20,SHX(-4),PHX(-4),volume_names[2]);
				if(PLUTOPAK)
				{            
					menutext(160,60+20+20+20,SHX(-5),PHX(-5),volume_names[3]);
					//if(boardfilename[0])
					//{

					// uncomment this for usermap
					//menutext(160,60+20+20+20+20,SHX(-6),PHX(-6),"USER MAP");
		           
					//gametextpal(160,60+20+20+20+20+3,boardfilename,16+(sintable[(totalclock<<4)&2047]>>11),2);
					//}
				}
				else
				{	/*
		            if(boardfilename[0])
				    {
						menutext(160,60+20+20+20,SHX(-6),PHX(-6),"USER MAP");
						gametext(160,60+20+20+20+6,boardfilename,2,2+8+16);
					}
					*/
				}
			}
            break;

		case 101: // MENU_USER_MAP
	            c = (320>>1);
				rotatesprite(c<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
				menutext(c,24,0,0,"USER MAP");

				// Draw USER MAP background
				{
					int y, x1;
					int32_t xPos, xPos2;
					int32_t yPos; //yPos2;

					xPos =  ( xdim *32) / 320;
					yPos =  ( ydim *30) / 200;

					xPos2 =  ( xdim *282) / 320;
					//yPos2 =  ( ydim *130) / 200;

					for(y=yPos; y < (ydim - (yPos*2)); y+=128)
					{
						for(x1=xPos; x1 < xPos2; x1+=128)
						{
							rotatesprite(x1<<16,y<<16,65536L,0,BIGHOLE,8,0,1+8+16+64+128,0,0, xdim - xPos, ydim - (yPos*2));
						}
					}
				}	

				c = (320>>1)-120;
				x = probe(c,70,19,4);
				
				if(x == -1)
				{
					cmenu(MENU_SELECT_EPISODE);
				}
			break;

        case 110:
            c = (320>>1);
            rotatesprite(c<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(c,24,0,0,"SELECT SKILL");
            x = probe(c,70,19,4);
            if(x >= 0)
            {
                switch(x)
                {
                    case 0: globalskillsound = JIBBED_ACTOR6;break;
                    case 1: globalskillsound = BONUS_SPEECH1;break;
                    case 2: globalskillsound = DUKE_GETWEAPON2;break;
                    case 3: globalskillsound = JIBBED_ACTOR5;break;
                }

                sound(globalskillsound);

                ud.m_player_skill = x+1;
                if(x == 3) ud.m_respawn_monsters = 1;
                else ud.m_respawn_monsters = 0;

                ud.m_monsters_off = ud.monsters_off = 0;

                ud.m_respawn_items = 0;
                ud.m_respawn_inventory = 0;

                ud.multimode = 1;

				// if (ud.showcinematics) 
                //if(ud.m_volume_number == 3) // not needed to play cinematics. Black screen not nice
                //{
                //    flushperms();
                //    setview(0,0,xdim-1,ydim-1);
                //    clearview(0L);
                //    nextpage();
                //}
				
                newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill);
                enterlevel(MODE_GAME);
            }
            else if(x == -1)
            {
                cmenu(100);
                KB_FlushKeyboardQueue();
            }

            menutext(c,70,SHX(-2),PHX(-2),skill_names[0]);
            menutext(c,70+19,SHX(-3),PHX(-3),skill_names[1]);
            menutext(c,70+19+19,SHX(-4),PHX(-4),skill_names[2]);
            menutext(c,70+19+19+19,SHX(-5),PHX(-5),skill_names[3]);
            break;

        case 200:

            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"OPTIONS");

            c = (320>>1)-120;

            x = probe(c+6,43,16,6);

            if(x == -1)
                { if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);else cmenu(0); 
                  CONFIG_WriteSetup();}

            switch(x)
            {
				case 0:
					cmenu(702); // game options
					break;

                case 1:
                    cmenu(703); // keybaord setup
					probey = 7;
                    break;

                case 2:
                    cmenu(701); // mouse setup
                    break;

				case 3:
                    cmenu(700);  // sound setup
                    break;

				case 4:  
					cmenu(706); // Video setup
					lastkeysetup = 0;
					current_resolution = 0; // in case we don't find it
					for(i=0; i<validmodecnt; i++)
					{
						if(validmodexdim[i] == xdim && validmodeydim[i] == ydim)
							current_resolution = i;
					}
					break;

                case 5: // record on/off
                    if( (ps[myconnectindex].gm&MODE_GAME) )
                    {
                        closedemowrite();
                        break;
                    }
                    ud.m_recstat = !ud.m_recstat;
                    break;

				//case -7:
				//	gametext(320>>1,43+16*6,"*** DISABLED. WILL BE FIXED SOON***",0,2+8+16); // center-i
				//	break;

			}

			menutext(c,43,SHX(-6),PHX(-6),"GAME OPTIONS");

			menutext(c,43+16,SHX(-6),PHX(-6),"SETUP KEYBOARD");

			menutext(c,43+16+16,SHX(-6),PHX(-6),"SETUP MOUSE");

            menutext(c,43+16+16+16,SHX(-8),PHX(-8),"SETUP SOUND");

            menutext(c,43+16+16+16+16,SHX(-8),PHX(-8),"SETUP VIDEO");

            if( (ps[myconnectindex].gm&MODE_GAME) && ud.m_recstat != 1 )
            {
                menutext(c,43+16+16+16+16+16,SHX(-10),1,"RECORD");
                menutext(c+160+40,43+16+16+16+16+16,SHX(-10),1,"OFF");
            }
            else
            {
                menutext(c,43+16+16+16+16+16,SHX(-10),PHX(-10),"RECORD");

                if(ud.m_recstat == 1)
                    menutext(c+160+40,43+16+16+16+16+16,SHX(-10),PHX(-10),"ON");
                else menutext(c+160+40,43+16+16+16+16+16,SHX(-10),PHX(-10),"OFF");
            }

            break;

        case 700:

            c = (320>>1)-120;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"SETUP SOUNDS");
            onbar = ((probey == 2)&&SoundToggle) || ((probey == 3)&&MusicToggle) ;

            x = probe(c+6,43,16,8);

            switch(x)
            {
                case -1:
					cmenu(200);
					probey = 3;
                    break;

                case 0:
                    if (FXDevice != NumSoundCards)
                    {
                        SoundToggle = 1-SoundToggle;
                        if( SoundToggle == 0 )
                        {
                            FX_StopAllSounds();
                            clearsoundlocks();
                        }
                    }
                    break;

				case 1:

                    if(eightytwofifty == 0 || numplayers < 2)
                        if(MusicDevice != NumSoundCards)
                    {
                        MusicToggle = 1-MusicToggle;
                        if( MusicToggle == 0 ) 
							MUSIC_StopSong();
                        else
                        {
                            if(ud.recstat != 2 && ps[myconnectindex].gm&MODE_GAME)
                                playmusic(&music_fn[0][music_select][0]);
                            else playmusic(&env_music_fn[0][0]);

                            MUSIC_Continue();
                        }
                    }
                    break;

                case 4:
                    if(SoundToggle && (FXDevice != NumSoundCards)) VoiceToggle = 1-VoiceToggle;
                    break;

				case 5:
                    if(SoundToggle && (FXDevice != NumSoundCards)) AmbienceToggle = 1-AmbienceToggle;
                    break;

				case 6:
                    if(SoundToggle && (FXDevice != NumSoundCards))
                    {
                        ReverseStereo = 1-ReverseStereo;
                        FX_SetReverseStereo(ReverseStereo);
					}
                    break;

				case 7: // xduke: 1.3d sound style - hear opponent
					if(SoundToggle && (FXDevice != NumSoundCards))
                    {
                        OpponentSoundToggle = 1-OpponentSoundToggle;
                    }
					break;

                default:
                    break;
            }

            if(SoundToggle && FXDevice != NumSoundCards) menutext(c+160+40,43,0,(FXDevice == NumSoundCards),"ON");
            else menutext(c+160+40,43,0,(FXDevice == NumSoundCards),"OFF");

            if(MusicToggle && (MusicDevice != NumSoundCards) && (!eightytwofifty||numplayers<2))
                menutext(c+160+40,43+16,0,(MusicDevice == NumSoundCards),"ON");
            else menutext(c+160+40,43+16,0,(MusicDevice == NumSoundCards),"OFF");

            menutext(c,43,SHX(-2),(FXDevice == NumSoundCards),"SOUND");
            menutext(c,43+16+16,SHX(-4),(FXDevice==NumSoundCards)||SoundToggle==0,"SOUND VOLUME");
            {
                l = FXVolume;
                FXVolume >>= 2;
                bar(c+167+40,43+16+16,(short *)&FXVolume,4,(FXDevice!=NumSoundCards)&&x==2,SHX(-4),SoundToggle==0||(FXDevice==NumSoundCards));
                if(l != FXVolume)
                    FXVolume <<= 2;
                if(l != FXVolume)
                    FX_SetVolume( (short) FXVolume );
            }
            menutext(c,43+16,SHX(-3),(MusicDevice==NumSoundCards),"MUSIC");
            menutext(c,43+16+16+16,SHX(-5),(MusicDevice==NumSoundCards)||(numplayers > 1 && eightytwofifty)||MusicToggle==0,"MUSIC VOLUME");
            {
                l = MusicVolume;
                MusicVolume >>= 2;
                bar(c+167+40,43+16+16+16,
                    (short *)&MusicVolume,4,
                    (eightytwofifty==0||numplayers < 2) && (MusicDevice!=NumSoundCards) && x==3,SHX(-5),
                    (numplayers > 1 && eightytwofifty)||MusicToggle==0||(MusicDevice==NumSoundCards));
                MusicVolume <<= 2;
                if(l != MusicVolume)
                {
                    STUBBED("Check this");
                    // !!! FIXME: Used to be Music_ not MUSIC_.  --ryan.
                    MUSIC_SetVolume( (short) MusicVolume );
                }
            }
            menutext(c,43+16+16+16+16,SHX(-6),(FXDevice==NumSoundCards)||SoundToggle==0,"DUKE TALK");
            menutext(c,43+16+16+16+16+16,SHX(-7),(FXDevice==NumSoundCards)||SoundToggle==0,"AMBIENCE");

            menutext(c,43+16+16+16+16+16+16,SHX(-8),(FXDevice==NumSoundCards)||SoundToggle==0,"FLIP STEREO");

            if(VoiceToggle) menutext(c+160+40,43+16+16+16+16,0,(FXDevice==NumSoundCards)||SoundToggle==0,"ON");
            else menutext(c+160+40,43+16+16+16+16,0,(FXDevice==NumSoundCards)||SoundToggle==0,"OFF");

            if(AmbienceToggle) menutext(c+160+40,43+16+16+16+16+16,0,(FXDevice==NumSoundCards)||SoundToggle==0,"ON");
            else menutext(c+160+40,43+16+16+16+16+16,0,(FXDevice==NumSoundCards)||SoundToggle==0,"OFF");

            if(ReverseStereo) menutext(c+160+40,43+16+16+16+16+16+16,0,(FXDevice==NumSoundCards)||SoundToggle==0,"ON");
            else menutext(c+160+40,43+16+16+16+16+16+16,0,(FXDevice==NumSoundCards)||SoundToggle==0,"OFF");

            menutext(c,43+16+16+16+16+16+16+16,SHX(-9),(FXDevice==NumSoundCards)||SoundToggle==0,"OPPONENT SOUND");
			if(OpponentSoundToggle) menutext(c+160+40,43+16+16+16+16+16+16+16,0,(FXDevice==NumSoundCards)||SoundToggle==0,"ON");
			else menutext(c+160+40,43+16+16+16+16+16+16+16,0,(FXDevice==NumSoundCards)||SoundToggle==0,"OFF");

            break;

        case 701:
            c = (320>>1)-120;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"SETUP MOUSE");
            onbar = ( probey == 0 || probey == 1);

            x = probe(c+6,43,16,8);
            switch(x)
            {
				case -7:
					gametext(320>>1,43+16*7+3,"*** SHORTCUT: ALT-M ***",0,2+8+16); // center-i
					break;

                case -1:
					cmenu(200);
					probey = 2;
                    break;

                case 0:
				case 1:
					break;
				case 2:

 					if ( ((ControllerType == controltype_keyboardandmouse)||
						(ControllerType == controltype_joystickandmouse)) )
					{
						MouseAiming = !MouseAiming;
						if(MouseAiming)
							myaimmode = 0;

					}
                    break;

				case 3:

 					if ( ((ControllerType == controltype_keyboardandmouse)||
						(ControllerType == controltype_joystickandmouse)) )
					{
						if(!MouseAiming) // means we are in toggle mode
							myaimmode = !myaimmode;
					}
                    break;

                case 4:

 					if ( ((ControllerType == controltype_keyboardandmouse)||
						(ControllerType == controltype_joystickandmouse)) )
					{
						ud.mouseflip = 1-ud.mouseflip;
					}
                    break;

				case 5:

					if (SDL_WM_GrabInput(SDL_GRAB_QUERY)==SDL_GRAB_ON) 
					{
						SDL_WM_GrabInput(SDL_GRAB_OFF);
						SDL_ShowCursor(1);
					}
					else
					{
						SDL_WM_GrabInput(SDL_GRAB_ON);
						SDL_ShowCursor(0);
					}
					break;

				case 6:
					cmenu(704); // Button setup
                    break;

				case 7:
					cmenu(705); // Digital axes setup
                    break;

                default:
                    break;
            }

			{            
			short sense;

			sense = CONTROL_GetMouseSensitivity_X();
			menutext(c,43+16*0,SHX(-7),PHX(-7),"X SENSITIVITY");
			bar(c+167+40,43+16*0,&sense,1,x==0,SHX(-7),PHX(-7));
			CONTROL_SetMouseSensitivity_X( sense );

			sense = CONTROL_GetMouseSensitivity_Y();
			menutext(c,43+16*1,SHX(-7),PHX(-7),"Y SENSITIVITY");
			bar(c+167+40,43+16*1,&sense,1,x==1,SHX(-7),PHX(-7));
			CONTROL_SetMouseSensitivity_Y( sense );

			menutext(c,43+16*2,SHX(-7),PHX(-7),"MOUSE AIM TYPE");
			if(MouseAiming) menutext(c+160+40,43+16*2,SHX(-7),PHX(-7),"HELD");
			else menutext(c+160+40,43+16*2,SHX(-7),PHX(-7),"TOGGLE");

			menutext(c,43+16*3,SHX(-7),MouseAiming,"MOUSE AIMING");
			if(myaimmode) menutext(c+160+40,43+16*3,SHX(-7),MouseAiming,"ON");
			else menutext(c+160+40,43+16*3,SHX(-7),MouseAiming,"OFF");

			menutext(c,43+16*4,SHX(-7),PHX(-7),"MOUSE AIMING FLIP");
			if(ud.mouseflip) menutext(c+160+40,43+16*4,SHX(-7),PHX(-7),"ON");
			else menutext(c+160+40,43+16*4,SHX(-7),PHX(-7),"OFF");


			menutext(c,43+16*5,SHX(-7),PHX(-7),"MOUSE CURSOR");
			if(SDL_WM_GrabInput(SDL_GRAB_QUERY)==SDL_GRAB_ON)
				menutext(c+160+40,43+16*5,SHX(-7),PHX(-7),"TAKEN");
			else
				menutext(c+160+40,43+16*5,SHX(-7),PHX(-7),"FREE'D");

			menutext(c,43+16*6,SHX(-7),PHX(-7),"BUTTON SETUP...");
		
			menutext(c,43+16*7,SHX(-7),PHX(-7),"DIGITAL AXES SETUP...");	
			}

            break;

        case 702:
            c = (320>>1)-120;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"GAME OPTIONS");

            onbar = 0;

			x = probe(c+6,43,16,7);

            switch(x)
            {

                case -1:
					cmenu(200); 
                    break;

                case 0:
                    ud.shadows = 1-ud.shadows;
                    break;
                case 1:
                    ud.screen_tilting = 1-ud.screen_tilting;
                    break;
				case 2:
					ud.showcinematics = !ud.showcinematics;
					break;
				case 3:
					ud.hideweapon = !ud.hideweapon;
					vscrn(); // FIX_00056: Refresh issue w/FPS, small Weapon and custom FTA, when screen resized down
					break;
				case 4:
					ud.weaponautoswitch = !ud.weaponautoswitch;
					break;
				case 5:
					// FIX_00045: Autoaim mode can now be toggled on/off from menu
					if( nHostForceDisableAutoaim == 0)
					{
						ud.auto_aim++;
						ud.auto_aim = ((ud.auto_aim-1)%2)+1; // 2 = normal = full; 1 = bullet only
					}					
					break;
                case 6: // parental
#ifndef AUSTRALIA
                    cmenu(10000); 
#endif
                    break;

			}


			menutext(c,43+16*0,SHX(-3),PHX(-3),"SHADOWS");
            if(ud.shadows) menutext(c+160+40,43+16*0,0,0,"ON");
            else menutext(c+160+40,43+16*0,0,0,"OFF");

			menutext(c,43+16*1,SHX(-4),PHX(-4),"SCREEN TILTING");
            switch(ud.screen_tilting)
            {
                case 0: menutext(c+160+40,43+16*1,0,0,"OFF");break;
                case 1: menutext(c+160+40,43+16*1,0,0,"ON");break;
                case 2: menutext(c+160+40,43+16*1,0,0,"FULL");break;
            }

			menutext(c,43+16*2,SHX(-3),PHX(-3),"CINEMATICS");
            if(ud.showcinematics) menutext(c+160+40,43+16*2,0,0,"ON");
            else menutext(c+160+40,43+16*2,0,0,"OFF");

			menutext(c,43+16*3,SHX(-3),PHX(-3),"WEAPON MODEL");
			if(ud.hideweapon) menutext(c+160+40,43+16*3,0,0,"OFF");
            else menutext(c+160+40,43+16*3,0,0,"ON");

			menutext(c,43+16*4,SHX(-3),PHX(-3),"WEAPON SWITCH");
			if(ud.weaponautoswitch) menutext(c+160+40,43+16*4,0,0,"OFF");
            else menutext(c+160+40,43+16*4,0,0,"ON");

			switch(ud.auto_aim)
            {
                case 0: menutext(c,43+16*5,0,nHostForceDisableAutoaim,"AUTOAIM DISABLED BY HOST"); 
						break;
				case 1: menutext(c,43+16*5,0,0,"AUTOAIM BULLET ONLY");
						break;
                case 2: menutext(c,43+16*5,0,0,"AUTOAIM REGULAR FULL");
						break;
            }

#ifndef AUSTRALIA
            menutext(c,43+16*6,SHX(-9),PHX(-9),"PARENTAL LOCK");
#else
            menutext(c,43+16*6,SHX(-9),1,"PARENTAL LOCK");
#endif


			break;

        case 703:

			// black translucent background underneath lists
			rotatesprite(0<<16, 0<<16, 65536l<<5, 0, BLANK, 8, 0, 1+2+8+16+64,
			    scale(0,xdim,320),scale(0,ydim,200),
			    scale(320-0,xdim,320)-1,scale(200-34,ydim,200)-1);

			c = (320>>1)-120-25;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"SETUP KEYBOARD");

			onbar = 0;
			x = probeXduke(c+210+lastkeysetup*62,46+16+16+16,0,NUMGAMEFUNCTIONS,20000);
			if(waiting4key)
			{
				probey = waiting4key-1; // force it to stay at the same location
				x=-(waiting4key-1)-2;
			}

			if (( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_LeftArrow ) ||
				 KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_kpad_6 )) &&
				!waiting4key) // set left or right column flag
			{
				lastkeysetup = !lastkeysetup;
				KB_ClearKeyDown( sc_RightArrow ); KB_ClearKeyDown( sc_LeftArrow );
				KB_ClearKeyDown( sc_kpad_4 ); KB_ClearKeyDown( sc_kpad_6 );
				sound(KICK_HIT);
			}

			if (KB_KeyPressed(sc_Delete) && -2>=x && x>=(-NUMGAMEFUNCTIONS-1) && !waiting4key) // clear a key
			{
				if(lastkeysetup)
					CONTROL_MapKey(-x-2, KeyMapping[-x-2].key1, 0);
				else
					CONTROL_MapKey(-x-2, 0, KeyMapping[-x-2].key2);

				KB_ClearKeyDown( sc_Delete ); // Avoid repeating delete
				sound(EXITMENUSOUND);
			}

			if ( (0<=x && x<NUMGAMEFUNCTIONS) || waiting4key) // set a key
			{
				if(!waiting4key)
				{ 
					waiting4key = x+1; // so it's always true
					KB_ClearLastScanCode(); // clear the enter hit that was just hit
				}

				if(KB_GetLastScanCode())
				{
					if(KB_GetLastScanCode() != sc_Escape)  // ESC is reserved for menu. Using it for controls could discard it completely, eg: AutoRun = "Escape"
					{
						if(lastkeysetup)		
							CONTROL_MapKey(waiting4key-1, KeyMapping[waiting4key-1].key1, KB_GetLastScanCode());
						else
							CONTROL_MapKey(waiting4key-1, KB_GetLastScanCode(), KeyMapping[waiting4key-1].key2);

						sound(KICK_HIT);
					}

					KB_ClearLastScanCode();
					KB_FlushKeyboardQueue();
					KB_ClearKeysDown();
					waiting4key = false;
				}					
			}

			if(!waiting4key) 
				switch(x)
				{
					case -1:
						cmenu(200);
						probey = 1; // back to setup keyboard
						break;
				}

			// display and scroll the whole keyboard list
			j = 7; // -j .. 0 .. j lines => 2*j+1 lines
			if(-2>=x && x>=(-NUMGAMEFUNCTIONS-1)) // -2 to -54 (53 values -> 0->52)
			{
				for(i=-j; i<=+j; i++)
					if(NUMGAMEFUNCTIONS > (-x-2+i) && (-x-2+i) >= 0)
					{
						gametext(c-10,47-16+8*(i+j),gamefunctions[-x-2+i],i?0:0,2+8+16); // disp name
						if (i ||  lastkeysetup || !waiting4key || (totalclock%128 < 64)) // blink 1st key
							gametext(c+185,47-16+8*(i+j),KB_ScanCodeToString( KeyMapping[-x-2+i].key1 )?KB_ScanCodeToString( KeyMapping[-x-2+i].key1 ):"...",i?0:0,2+8+16); // center-i
						if (i || !lastkeysetup || !waiting4key || (totalclock%128 < 64)) // blink 2nd key
							gametext(c+247,47-16+8*(i+j),KB_ScanCodeToString( KeyMapping[-x-2+i].key2 )?KB_ScanCodeToString( KeyMapping[-x-2+i].key2 ):"...",i?0:0,2+8+16); // center-i
					}
			}

			if(waiting4key)
				gametext(320>>1,47-16+8*(2*j+2)-4,"*** HIT A NEW KEY ***",0,2+8+16); // center-i
			else
				gametext(320>>1,47-16+8*(2*j+2)-4,"*** HIT ENTER OR DEL KEY ***",0,2+8+16); // center-i

			break;

        case 704: // mouse button setup, frm menu 701
            c = (320>>1)-120-25;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"SETUP MOUSE");

			// black translucent background underneath lists
			rotatesprite(0<<16, 0<<16, 65536l<<5, 0, BLANK, 8, 0, 1+2+8+16+64,
			    scale(0,xdim,320),scale(40,ydim,200),
			    scale(320-0,xdim,320)-1,scale(200-75,ydim,200)-1);

			onbar = 0;
			x = probeXduke(c+146,46+8,8,MAXMOUSEBUTTONS,20000);
			lastkeysetup = 0;
			
			if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed (sc_BackSpace) )
			{
				lastkeysetup = 1; // reversed;
				KB_ClearKeyDown( sc_kpad_4 );
				KB_ClearKeyDown( sc_LeftArrow );
				KB_ClearKeyDown(sc_BackSpace);
				sound(KICK_HIT);
				x = -x-2;
			}
			else if ( KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ) )
			{
				KB_ClearKeyDown( sc_kpad_6 );
				KB_ClearKeyDown( sc_RightArrow );
				sound(KICK_HIT);
				x = -x-2;
			}

			if (KB_KeyPressed(sc_Delete) && -2>=x && x>=(-MAXMOUSEBUTTONS-1)) // clear a key
			{
				MouseMapping[-x-2] = -1;
				KB_ClearKeyDown( sc_Delete ); // Avoid repeating delete
				sound(EXITMENUSOUND);
			}

			if (0<=x && x<MAXMOUSEBUTTONS) // set a function
			{	
				if (lastkeysetup) // going up
				{
					MouseMapping[x]--;
					if(MouseMapping[x]==-2)
						MouseMapping[x] = NUMGAMEFUNCTIONS-1;

				}
				else
				{
					MouseMapping[x]++;
					if(MouseMapping[x]==NUMGAMEFUNCTIONS)
						MouseMapping[x] = -1; // Discard
				}
			}

			switch(x)
			{
				case -1:
					cmenu(701);
					probey = 6; // back to the general mouse setup menu
					break;
			}

			// display the button list
			for(i=0; i<MAXMOUSEBUTTONS; i++)
			{
				sprintf(text, "#%d",i);
				switch(i)
				{
					case 0:
						strcat(text, " Left");
						break;
					case 1:
						strcat(text, " Right");
						break;
					case 2:
						strcat(text, " Middle");
						break;
					case 3:
						strcat(text, " Wheel up");
						break;
					case 4:
						strcat(text, " Wheel down");
						break;
					default:
						strcat(text, " (Extra)");
						break;
				}
				
				gametext(c,47+i*8,text,0,2+8+16);
				gametext(c+121,47+i*8,(MouseMapping[i]!=-1)?CONFIG_FunctionNumToName(MouseMapping[i]):"...",0,2+8+16);
			}

			gametext(320>>1,47+(MAXMOUSEBUTTONS+1)*8,"*** USE ARROWS OR DEL KEY ***",0,2+8+16);

			break;

        case 705: // digital axes setup, from menu 701
            c = (320>>1)-120-25;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"DIGITAL AXES");

			// black translucent background underneath lists
			rotatesprite(0<<16, 0<<16, 65536l<<5, 0, BLANK, 8, 0, 1+2+8+16+64,
			    scale(0,xdim,320),scale(40,ydim,200),
			    scale(320-0,xdim,320)-1,scale(200-100,ydim,200)-1);

			onbar = 0;
			x = probeXduke(c+146,46+8,8,MAXMOUSEAXES*2,20000);
			lastkeysetup = 0;
			
			if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed (sc_BackSpace) )
			{
				lastkeysetup = 1; // reversed;
				KB_ClearKeyDown( sc_kpad_4 );
				KB_ClearKeyDown( sc_LeftArrow );
				KB_ClearKeyDown(sc_BackSpace);
				sound(KICK_HIT);
				x = -x-2;
			}
			else if ( KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ) )
			{
				KB_ClearKeyDown( sc_kpad_6 );
				KB_ClearKeyDown( sc_RightArrow );
				sound(KICK_HIT);
				x = -x-2;
			}

			if (KB_KeyPressed(sc_Delete) && -2>=x && x>=(-(MAXMOUSEAXES*2)-1)) // clear a key
			{
				MouseDigitalAxeMapping[(-x-2)>>1][(-x-2)&1] = -1;
				KB_ClearKeyDown( sc_Delete ); // Avoid repeating delete
				sound(EXITMENUSOUND);
			}

			if (0<=x && x<(MAXMOUSEAXES*2)) // set a function
			{	
				if (lastkeysetup) // going up
				{
					MouseDigitalAxeMapping[x>>1][x&1]--;
					if(MouseDigitalAxeMapping[x>>1][x&1]==-2)
						MouseDigitalAxeMapping[x>>1][x&1] = NUMGAMEFUNCTIONS-1;
				}
				else
				{
					MouseDigitalAxeMapping[x>>1][x&1]++;
					if(MouseDigitalAxeMapping[x>>1][x&1]==NUMGAMEFUNCTIONS)
						MouseDigitalAxeMapping[x>>1][x&1] = -1; // Discard
				}
			}

			switch(x)
			{
				case -1:
					cmenu(701);
					probey = 7; // back to the general mouse setup menu
					break;
			}

			// display the button list
			for(i=0; i<(MAXMOUSEAXES*2); i++)
			{
				*tempbuf = 0;
				switch(i)
				{
					case 0:
						strcat(text, "X left");
						break;
					case 1:
						strcat(text, "X right");
						break;
					case 2:
						strcat(text, "Y up");
						break;
					case 3:
						strcat(text, "Y down");
						break;
					default:
						break;
				}
				
				gametext(c,47+i*8,text,0,2+8+16);
				gametext(c+121,47+i*8,(MouseDigitalAxeMapping[i>>1][i&1]!=-1)?CONFIG_FunctionNumToName(MouseDigitalAxeMapping[i>>1][i&1]):"...",0,2+8+16);
			}

			gametext(320>>1,47+(4+1)*8,"*** USE ARROWS OR DEL KEY ***",0,2+8+16);

			break;

        case 706: // Video setup
			// FIX_00042: Build in Video setup.
			c = (320>>1)-120;
            rotatesprite(320<<15,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"VIDEO SETUP");

			onbar = (probey == 3 || probey == 4);
			x = probe(c+6,43,16,6);
			
			switch(x)
			{
				case -7: // cursor idle on the FPS option (5)
					gametext(320>>1,43+16*7,"*** SHORTCUT: TYPE DNRATE ***",0,2+8+16); // center-i
					
					break;

				case -3: // cursor idle on the togglefullscreen option (1)
					gametext(320>>1,43+16*7,"*** SHORTCUT: ALT-ENTER ***",0,2+8+16); // center-i

					break;

				case -2: // cursor idle on the resolution option (0)
					if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) )
					{
						current_resolution--;
						if(current_resolution == -1) 
							current_resolution = 0;
						lastkeysetup = 1; // indicates we changed
						KB_ClearKeyDown( sc_kpad_4 );
						KB_ClearKeyDown( sc_LeftArrow );
						sound(PISTOL_BODYHIT);
					}
					else if ( KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ) )
					{
						current_resolution++; // reversed;
						if(current_resolution == validmodecnt) 
							current_resolution = validmodecnt-1;
						lastkeysetup = 1; // indicates we changed
						KB_ClearKeyDown( sc_kpad_6 );
						KB_ClearKeyDown( sc_RightArrow );
						sound(PISTOL_BODYHIT);
					}

					if(lastkeysetup)
						gametext(320>>1,43+16*7,"*** HIT ENTER TO VALIDATE ***",0,2+8+16); // center-i
					else
						gametext(320>>1,43+16*7,"*** LEFT/RIGHT ARROW TO SELECT ***",0,2+8+16); // center-i

					break;

				case -1:
					cmenu(200);
					probey = 4; // back to the general option menu
					break;

				case 0:
					if(lastkeysetup)
						setgamemode(ScreenMode,validmodexdim[current_resolution],validmodeydim[current_resolution]);
					lastkeysetup = 0; // indicating changes are done
					break;

				case 1:
					BFullScreen = !BFullScreen;
					SDL_QuitSubSystem(SDL_INIT_VIDEO);
					_platform_init(0, NULL, "Duke Nukem 3D", "Duke3D");
					_setgamemode(ScreenMode,validmodexdim[current_resolution],validmodeydim[current_resolution]);
					break;

                case 2:
                    ud.detail = 1-ud.detail;
                    break;

				case 5:
					ud.tickrate ^= 1;
					vscrn(); // FIX_00056: Refresh issue w/FPS, small Weapon and custom FTA, when screen resized down
					break;
			}
				
			menutext(c,43,0,0,"RESOLUTION");
			sprintf(text, "%d x %d", validmodexdim[current_resolution],validmodeydim[current_resolution]);
			if (lastkeysetup == 0 || (totalclock%64 < 32)) // blink color after change
				menutext(c+150,43,0,0,text);
			else
				menutext(c+150,43,0,1,text);

			menutext(c,43+16*1,SHX(-3),PHX(-3),"FULLSCREEN");
			menutext(c+160+40,43+16*1,0,0,BFullScreen?"ON":"OFF");
			
			menutext(c,43+16*2,SHX(-2),PHX(-2),"DETAIL");
			menutext(c+160+40,43+16*2,0,0,ud.detail?"HIGH":"LOW");
			{
				int32 screen_size = ud.screen_size;

				// FIX_00027: Added an extra small statusbar (HUD)
	            menutext(c,43+16*3,SHX(-5),PHX(-5),"SCREEN SIZE");
	            bar(c+167+40,43+16*3,(short *)&screen_size,-4,x==3,SHX(-5),PHX(-5));
				if(ud.screen_size==4)
				{
					if(screen_size==0)
					{
						ud.extended_screen_size++;
						if(ud.extended_screen_size==2)
						{
							ud.extended_screen_size = 1;
							ud.screen_size -= 4;
						}
					}
					else if(screen_size==8)
					{
						ud.extended_screen_size--;
						if(ud.extended_screen_size<0)
						{
							ud.extended_screen_size=0;
							ud.screen_size += 4;
						}
					}
				}
				else
					ud.screen_size = screen_size;
			}

            menutext(c,43+16*4,SHX(-6),PHX(-6),"BRIGHTNESS");
            bar(c+167+40,43+16*4,(short *)&ud.brightness,8,x==4,SHX(-6),PHX(-6));
            if(x==4) setbrightness(ud.brightness>>2,&ps[myconnectindex].palette[0]);

			menutext(c,43+16*5,SHX(-2),PHX(-2),"SHOW FPS");
			menutext(c+160+40,43+16*5,0,0,(ud.tickrate&1)?"ON":"OFF");

			break;

        case 350:
            cmenu(351);
            screencapt = 1;
            displayrooms(myconnectindex,65536);
            savetemp("duke3d.tmp",tiles[MAXTILES-1].data,160*100);
            screencapt = 0;
            break;

        case 360:
        case 361:
        case 362:
        case 363:
        case 364:
        case 365:
        case 366:
        case 367:
        case 368:
        case 369:
        case 351:
        case 300:

            c = 320>>1;
            rotatesprite(c<<16,200<<15,65536L,0,MENUSCREEN,16,0,10+64,0,0,xdim-1,ydim-1);
            rotatesprite(c<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);

            if(current_menu == 300) menutext(c,24,0,0,"LOAD GAME");
            else menutext(c,24,0,0,"SAVE GAME");

            if(current_menu >= 360 && current_menu <= 369 )
            {
                sprintf(text,"PLAYERS: %-2d                      ",ud.multimode);
                gametext(160,158,text,0,2+8+16);
                sprintf(text,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+ud.volume_number,1+ud.level_number,ud.player_skill);
                gametext(160,170,text,0,2+8+16);

                x = strget((320>>1),184,&ud.savegame[current_menu-360][0],19, 999 );

                if(x == -1)
                {
            //        readsavenames();
                    ps[myconnectindex].gm = MODE_GAME;
                    if(ud.multimode < 2  && ud.recstat != 2)
                    {
                        ready2send = 1;
                        totalclock = ototalclock;
                    }
                    goto DISPLAYNAMES;
                }

                if( x == 1 )
                {
                    if( ud.savegame[current_menu-360][0] == 0 )
                    {
                        KB_FlushKeyboardQueue();
                        cmenu(351);
                    }
                    else
                    {
                        if(ud.multimode > 1)
                            saveplayer(-1-(current_menu-360));
                        else saveplayer(current_menu-360);
                        lastsavedpos = current_menu-360;
                        ps[myconnectindex].gm = MODE_GAME;

                        if(ud.multimode < 2  && ud.recstat != 2)
                        {
                            ready2send = 1;
                            totalclock = ototalclock;
                        }
                        KB_ClearKeyDown(sc_Escape);
                        sound(EXITMENUSOUND);
                    }
                }

                rotatesprite(101<<16,97<<16,65536,512,MAXTILES-1,-32,0,2+4+8+64,0,0,xdim-1,ydim-1);
                dispnames();
                rotatesprite((c+67+strlen(&ud.savegame[current_menu-360][0])*4)<<16,(50+12*probey)<<16,32768L-10240,0,SPINNINGNUKEICON+(((totalclock)>>3)%7),0,0,10,0,0,xdim-1,ydim-1);
                break;
            }

           last_threehundred = probey;

            x = probe(c+68,54,12,10);

          if(current_menu == 300)
          {
              if( ud.savegame[probey][0] )
              {
                  if( lastprobey != probey )
                  {
                     loadpheader(probey,&volnum,&levnum,&plrskl,&numplr);
                     lastprobey = probey;
                  }

                  rotatesprite(101<<16,97<<16,65536L,512,MAXTILES-3,-32,0,4+10+64,0,0,xdim-1,ydim-1);
                  sprintf(text,"PLAYERS: %-2d                      ",numplr);
                  gametext(160,158,text,0,2+8+16);
                  sprintf(text,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+volnum,1+levnum,plrskl);
                  gametext(160,170,text,0,2+8+16);
              }
              else menutext(69,70,0,0,"EMPTY");
          }
          else
          {
              if( ud.savegame[probey][0] )
              {
                  if(lastprobey != probey)
                      loadpheader(probey,&volnum,&levnum,&plrskl,&numplr);
                  lastprobey = probey;
                  rotatesprite(101<<16,97<<16,65536L,512,MAXTILES-3,-32,0,4+10+64,0,0,xdim-1,ydim-1);
              }
              else menutext(69,70,0,0,"EMPTY");
              sprintf(text,"PLAYERS: %-2d                      ",ud.multimode);
              gametext(160,158,text,0,2+8+16);
              sprintf(text,"EPISODE: %-2d / LEVEL: %-2d / SKILL: %-2d",1+ud.volume_number,1+ud.level_number,ud.player_skill);
              gametext(160,170,text,0,2+8+16);
          }

            switch( x )
            {
                case -1:
                    if(current_menu == 300) //load game
                    {
                        if( (ps[myconnectindex].gm&MODE_GAME) != MODE_GAME)
                        {
                            cmenu(0);
                            break;
                        }
                        else
                            ps[myconnectindex].gm &= ~MODE_MENU;
                    }
                    else // save game
                        ps[myconnectindex].gm = MODE_GAME;

                    if(ud.multimode < 2 && ud.recstat != 2)
                    {
                        ready2send = 1;
                        totalclock = ototalclock;
                    }

                    break;
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    if( current_menu == 300)
                    {
                        if( ud.savegame[x][0] )
                            current_menu = (1000+x);
                    }
                    else
                    {
                        if( ud.savegame[x][0] != 0)
                            current_menu = 2000+x;
                        else
                        {
                            KB_FlushKeyboardQueue();
                            current_menu = (360+x);
                            ud.savegame[x][0] = 0;
                            inputloc = 0;
                        }
                    }
                    break;
            }

            DISPLAYNAMES:
            dispnames();
            break;

        case 400: // help
        case 401:
		if(!VOLUMEONE)
		{
			c = 320>>1;

            if( KB_KeyPressed( sc_LeftArrow ) ||
                KB_KeyPressed( sc_kpad_4 ) ||
                KB_KeyPressed( sc_UpArrow ) ||
                KB_KeyPressed( sc_PgUp ) ||
                KB_KeyPressed( sc_kpad_8 ) )
            {
                KB_ClearKeyDown(sc_LeftArrow);
                KB_ClearKeyDown(sc_kpad_4);
                KB_ClearKeyDown(sc_UpArrow);
                KB_ClearKeyDown(sc_PgUp);
                KB_ClearKeyDown(sc_kpad_8);

                sound(KICK_HIT);
                current_menu--;
                if(current_menu < 400) current_menu = 401;
            }
            else if(
                KB_KeyPressed( sc_PgDn ) ||
                KB_KeyPressed( sc_Enter ) ||
                KB_KeyPressed( sc_kpad_Enter ) ||
                KB_KeyPressed( sc_RightArrow ) ||
                KB_KeyPressed( sc_DownArrow ) ||
                KB_KeyPressed( sc_kpad_2 ) ||
                KB_KeyPressed( sc_kpad_9 ) ||
                KB_KeyPressed( sc_Space ) ||
                KB_KeyPressed( sc_kpad_6 ) )
            {
                KB_ClearKeyDown(sc_PgDn);
                KB_ClearKeyDown(sc_Enter);
                KB_ClearKeyDown(sc_RightArrow);
                KB_ClearKeyDown(sc_kpad_Enter);
                KB_ClearKeyDown(sc_kpad_6);
                KB_ClearKeyDown(sc_kpad_9);
                KB_ClearKeyDown(sc_kpad_2);
                KB_ClearKeyDown(sc_DownArrow);
                KB_ClearKeyDown(sc_Space);
                sound(KICK_HIT);
                current_menu++;
                if(current_menu > 401) current_menu = 400;
            }

            if( KB_KeyPressed(sc_Escape) )
            {
				KB_ClearKeyDown(sc_Escape); // or else ESC will be activated in cmenu(0)
                if(ps[myconnectindex].gm&MODE_GAME)
                    cmenu(50);
                else cmenu(0);
                return;
            }

            flushperms();
            switch(current_menu)
            {
                case 400:
                    rotatesprite(0,0,65536L,0,TEXTSTORY,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                    break;
                case 401:
                    rotatesprite(0,0,65536L,0,F1HELP,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                    break;
            }
		break;
		}
		
		case 402:
        case 403:
		if(VOLUMEONE)
		{
            c = 320>>1;

            if( KB_KeyPressed( sc_LeftArrow ) ||
                KB_KeyPressed( sc_kpad_4 ) ||
                KB_KeyPressed( sc_UpArrow ) ||
                KB_KeyPressed( sc_PgUp ) ||
                KB_KeyPressed( sc_kpad_8 ) )
            {
                KB_ClearKeyDown(sc_LeftArrow);
                KB_ClearKeyDown(sc_kpad_4);
                KB_ClearKeyDown(sc_UpArrow);
                KB_ClearKeyDown(sc_PgUp);
                KB_ClearKeyDown(sc_kpad_8);

                sound(KICK_HIT);
                current_menu--;
                if(current_menu < 400) current_menu = 403;
            }
            else if(
                KB_KeyPressed( sc_PgDn ) ||
                KB_KeyPressed( sc_Enter ) ||
                KB_KeyPressed( sc_kpad_Enter ) ||
                KB_KeyPressed( sc_RightArrow ) ||
                KB_KeyPressed( sc_DownArrow ) ||
                KB_KeyPressed( sc_kpad_2 ) ||
                KB_KeyPressed( sc_kpad_9 ) ||
                KB_KeyPressed( sc_Space ) ||
                KB_KeyPressed( sc_kpad_6 ) )
            {
                KB_ClearKeyDown(sc_PgDn);
                KB_ClearKeyDown(sc_Enter);
                KB_ClearKeyDown(sc_RightArrow);
                KB_ClearKeyDown(sc_kpad_Enter);
                KB_ClearKeyDown(sc_kpad_6);
                KB_ClearKeyDown(sc_kpad_9);
                KB_ClearKeyDown(sc_kpad_2);
                KB_ClearKeyDown(sc_DownArrow);
                KB_ClearKeyDown(sc_Space);
                sound(KICK_HIT);
                current_menu++;
                if(current_menu > 403) current_menu = 400;
            }

            if( KB_KeyPressed(sc_Escape) )
            {
                if(ps[myconnectindex].gm&MODE_GAME)
                    cmenu(50);
                else cmenu(0);
                return;
            }

            flushperms();
            rotatesprite(0,0,65536L,0,ORDERING+current_menu-400,0,0,10+16+64,0,0,xdim-1,ydim-1);
		}
		break;

        case 500: // from f10
		case 501: // from menu 0
		case 502: // from menu 50

            c = 320>>1;

            gametext(c,90,"Are you sure you want to quit?",0,2+8+16);
            gametext(c,99,"(Y/N)",0,2+8+16);

			_handle_events();
            if( KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
				gameexitanycase();
            }

            x = probe(186,124,0,0);
            if(x == -1 || KB_KeyPressed(sc_N) || RMB)
            {
                KB_ClearKeyDown(sc_N);
                quittimer = 0;
				// FIX_00073: menu off messed up. While in game hit Esc -> select quit -> press esc => stuck in menu
				if (current_menu==500)
					ps[myconnectindex].gm &= ~MODE_MENU;
				else if(current_menu==501)
					cmenu(0);
				else
					cmenu(50);
            }
            break;

		case 503:
            c = 320>>1;
            gametext(c,90,"Quit to Title?",0,2+8+16);
            gametext(c,99,"(Y/N)",0,2+8+16);

			_handle_events();
            if( KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) || KB_KeyPressed(sc_Y) || LMB )
            {
                KB_FlushKeyboardQueue();
                ps[myconnectindex].gm = MODE_DEMO;
                if(ud.recstat == 1)
                    closedemowrite();
				if( ud.m_recstat != 2 && ud.last_level >= 0 && ud.multimode > 1 && ud.coop != 1)
					dobonus(1);
				ud.last_level = -1;
                cmenu(0);
            }

            x = probe(186,124,0,0);

            if(x == -1 || KB_KeyPressed(sc_N) || RMB)
            {
                cmenu(50);
                if(ud.multimode < 2  && ud.recstat != 2)
                {
                    ready2send = 1;
                    totalclock = ototalclock;
                }
            }

            break;

        case 601:
            displayfragbar();
            rotatesprite(160<<16,29<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,34,0,0,&ud.user_name[myconnectindex][0]);

            sprintf((char*)tempbuf,"Waiting for master");
            gametext(160,50,(char*)tempbuf,0,2+8+16);
            gametext(160,59,"to select level",0,2+8+16);

            if( KB_KeyPressed(sc_Escape) )
            {
                KB_ClearKeyDown(sc_Escape);
                sound(EXITMENUSOUND);
                cmenu(0);
            }
            break;

        case 602:
            if(menunamecnt == 0)
            {
        //        getfilenames("SUBD");
                getfilenames("*.MAP");
                sortfilenames();
                if (menunamecnt == 0)
                    cmenu(600);
            }
        case 603:
            c = (320>>1) - 120;
            displayfragbar();
            rotatesprite(320>>1<<16,19<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(320>>1,24,0,0,"USER MAPS");
            for(x=0;x<menunamecnt;x++)
            {
                if(x == fileselect)
                    minitext(15 + (x/15)*54,32 + (x%15)*8,menuname[x],0,26);
                else minitext(15 + (x/15)*54,32 + (x%15)*8,menuname[x],16,26);
            }

            fileselect = probey;
            if( KB_KeyPressed( sc_LeftArrow ) || KB_KeyPressed( sc_kpad_4 ) || ((buttonstat&1) && minfo.dyaw < -256 ) )
            {
                KB_ClearKeyDown( sc_LeftArrow );
                KB_ClearKeyDown( sc_kpad_4 );
                probey -= 15;
                if(probey < 0) probey += 15;
                else sound(KICK_HIT);
            }
            if( KB_KeyPressed( sc_RightArrow ) || KB_KeyPressed( sc_kpad_6 ) || ((buttonstat&1) && minfo.dyaw > 256 ) )
            {
                KB_ClearKeyDown( sc_RightArrow );
                KB_ClearKeyDown( sc_kpad_6 );
                probey += 15;
                if(probey >= menunamecnt)
                    probey -= 15;
                else sound(KICK_HIT);
            }

            onbar = 0;
            x = probe(0,0,0,menunamecnt);

            if(x == -1) cmenu(600);
            else if(x >= 0)
            {
                tempbuf[0] = 8;
                tempbuf[1] = ud.m_level_number = 6;
                tempbuf[2] = ud.m_volume_number = 0;
                tempbuf[3] = ud.m_player_skill+1;

                if(ud.player_skill == 3)
                    ud.m_respawn_monsters = 1;
                else ud.m_respawn_monsters = 0;

                if(ud.m_coop == 0) ud.m_respawn_items = 1;
                else ud.m_respawn_items = 0;

                ud.m_respawn_inventory = 1;

                tempbuf[4] = ud.m_monsters_off;
                tempbuf[5] = ud.m_respawn_monsters;
                tempbuf[6] = ud.m_respawn_items;
                tempbuf[7] = ud.m_respawn_inventory;
                tempbuf[8] = ud.m_coop;
                tempbuf[9] = ud.m_marker;

                x = strlen(menuname[probey]);

                copybufbyte(menuname[probey],tempbuf+10,x);
                copybufbyte(menuname[probey],boardfilename,x+1);

                for(c=connecthead;c>=0;c=connectpoint2[c])
                    if(c != myconnectindex)
                        sendpacket(c,(uint8_t*)tempbuf,x+10);

                newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill+1);
                enterlevel(MODE_GAME);
            }
            break;

        case 600: // multiplayer and bot menu
            c = (320>>1) - 120;
            if((ps[myconnectindex].gm&MODE_GAME) != MODE_GAME)
                displayfragbar();
            rotatesprite(160<<16,26<<16,65536L,0,MENUBAR,16,0,10,0,0,xdim-1,ydim-1);
            menutext(160,31,0,0,&ud.user_name[myconnectindex][0]);

            x = probe(c,57-8,16,8);

            switch(x)
            {
				// FIX_00068: menu "New Game" in multiplayer mode now allowing left/right arrow for selection
				case -7: // idle on case 5
					if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) ||
						 KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ))
					{
						if(ud.m_coop == 1)
							ud.m_ffire = !ud.m_ffire;

						KB_ClearKeyDown( sc_kpad_4 );
						KB_ClearKeyDown( sc_LeftArrow );
						KB_ClearKeyDown( sc_kpad_6 );
						KB_ClearKeyDown( sc_RightArrow );
						sound(PISTOL_BODYHIT);
					} 
                    break;

				case -6: // idle on case 4
					if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) ||
						 KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ))
					{
						if(ud.m_coop == 0)
							ud.m_marker = !ud.m_marker;

						KB_ClearKeyDown( sc_kpad_4 );
						KB_ClearKeyDown( sc_LeftArrow );
						KB_ClearKeyDown( sc_kpad_6 );
						KB_ClearKeyDown( sc_RightArrow );
						sound(PISTOL_BODYHIT);
					} 
                    break;

				case -5: // idle on case 3
					if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) )
					{
						if(ud.m_monsters_off == 1 && ud.m_player_skill > 0)
							ud.m_monsters_off = 0;

						if(ud.m_monsters_off == 1)
						{
							ud.m_monsters_off = 0;
							ud.m_player_skill = 3;
						}
						else
						{
							ud.m_player_skill--;
							if(ud.m_player_skill < 0)
							{
								ud.m_player_skill = 0;
								ud.m_monsters_off = 1;
							}
						}

						KB_ClearKeyDown( sc_kpad_4 );
						KB_ClearKeyDown( sc_LeftArrow );
						sound(PISTOL_BODYHIT);
					} 
					else if( KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ) )
					{	
						if(ud.m_monsters_off == 1 && ud.m_player_skill > 0)
							ud.m_monsters_off = 0;

						if(ud.m_monsters_off == 0)
						{
							ud.m_player_skill++;
							if(ud.m_player_skill > 3)
							{
								ud.m_player_skill = 0;
								ud.m_monsters_off = 1;
							}
						}
						else
						{
							ud.m_monsters_off = 0;
							ud.m_player_skill = 0;
						}

						KB_ClearKeyDown( sc_kpad_6 );
						KB_ClearKeyDown( sc_RightArrow );
						sound(PISTOL_BODYHIT);
					}
                    break;

#ifndef ONELEVELDEMO

				case -4: // idle on case 2
					if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) )
					{
						ud.m_level_number--;
if (!VOLUMEONE)
{
						if(ud.m_volume_number == 0 && ud.m_level_number < 0)
							ud.m_level_number = 6;
}
else
{
						if(ud.m_volume_number == 0 && ud.m_level_number < 0)
							ud.m_level_number = 5;
}
						if(ud.m_level_number < 0) ud.m_level_number = 10;

						KB_ClearKeyDown( sc_kpad_4 );
						KB_ClearKeyDown( sc_LeftArrow );
						sound(PISTOL_BODYHIT);
					}
					else if ( KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ) )
					{
						ud.m_level_number++;
if (!VOLUMEONE)
{
						if(ud.m_volume_number == 0 && ud.m_level_number > 6)
							ud.m_level_number = 0;
}
else
{
						if(ud.m_volume_number == 0 && ud.m_level_number > 5)
							ud.m_level_number = 0;
}
						if(ud.m_level_number > 10) ud.m_level_number = 0;

						KB_ClearKeyDown( sc_kpad_6 );
						KB_ClearKeyDown( sc_RightArrow );
						sound(PISTOL_BODYHIT);
					}
					break;
#endif

				case -3: // Idle on case 1
if (!VOLUMEONE)
{
					if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) )
					{
						ud.m_volume_number--;
if(PLUTOPAK)
						{	if(ud.m_volume_number < 0 ) ud.m_volume_number = 3; }
else
						{	if(ud.m_volume_number < 0) ud.m_volume_number = 2; }

						if(ud.m_volume_number == 0 && ud.m_level_number > 6)
							ud.m_level_number = 0;
						if(ud.m_level_number > 10) ud.m_level_number = 0;

						KB_ClearKeyDown( sc_kpad_4 );
						KB_ClearKeyDown( sc_LeftArrow );
						sound(PISTOL_BODYHIT);
					}
					else if( KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ) )
					{
						ud.m_volume_number++;
if(PLUTOPAK)
						{	if(ud.m_volume_number > 3) ud.m_volume_number = 0; }
else
						{	if(ud.m_volume_number > 2) ud.m_volume_number = 0; }

						if(ud.m_volume_number == 0 && ud.m_level_number > 6)
							ud.m_level_number = 0;
						if(ud.m_level_number > 10) ud.m_level_number = 0;

						KB_ClearKeyDown( sc_kpad_6 );
						KB_ClearKeyDown( sc_RightArrow );
						sound(PISTOL_BODYHIT);
					}
}
					break;

				case -2: // Idle on case 0
					if ( KB_KeyPressed( sc_kpad_4 ) || KB_KeyPressed( sc_LeftArrow ) )
					{
						ud.m_coop--;
						if(ud.m_coop == -1) ud.m_coop = 2;

						KB_ClearKeyDown( sc_kpad_4 );
						KB_ClearKeyDown( sc_LeftArrow );
						sound(PISTOL_BODYHIT);
					} 
					else if( KB_KeyPressed( sc_kpad_6 ) || KB_KeyPressed( sc_RightArrow ) )
					{	
						ud.m_coop++;
						if(ud.m_coop == 3) ud.m_coop = 0;

						KB_ClearKeyDown( sc_kpad_6 );
						KB_ClearKeyDown( sc_RightArrow );
						sound(PISTOL_BODYHIT);
					}
                    break;

                case -1:
                    ud.m_recstat = 0;
                    if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);
                    else cmenu(0);
                    break;
                case 0:
                    ud.m_coop++;
                    if(ud.m_coop == 3) ud.m_coop = 0;
                    break;
                case 1:
if (!VOLUMEONE)
{                    ud.m_volume_number++;
if(PLUTOPAK)
					{	if(ud.m_volume_number > 3) ud.m_volume_number = 0; }
else
					{	if(ud.m_volume_number > 2) ud.m_volume_number = 0; }

                    if(ud.m_volume_number == 0 && ud.m_level_number > 6)
                        ud.m_level_number = 0;
                    if(ud.m_level_number > 10) ud.m_level_number = 0;
}
                    break;
                case 2:
#ifndef ONELEVELDEMO
                    ud.m_level_number++;
if (!VOLUMEONE)
{
                    if(ud.m_volume_number == 0 && ud.m_level_number > 6)
                        ud.m_level_number = 0;
}
else
{
                    if(ud.m_volume_number == 0 && ud.m_level_number > 5)
                        ud.m_level_number = 0;
}
                    if(ud.m_level_number > 10) ud.m_level_number = 0;
#endif
                    break;
                case 3:
                    if(ud.m_monsters_off == 1 && ud.m_player_skill > 0)
                        ud.m_monsters_off = 0;

                    if(ud.m_monsters_off == 0)
                    {
                        ud.m_player_skill++;
                        if(ud.m_player_skill > 3)
                        {
                            ud.m_player_skill = 0;
                            ud.m_monsters_off = 1;
                        }
                    }
                    else 
					{
						ud.m_monsters_off = 0;
						ud.m_player_skill = 0;
					}

                    break;

                case 4:
                    if(ud.m_coop == 0)
                        ud.m_marker = !ud.m_marker;
                    break;

                case 5:
                    if(ud.m_coop == 1)
                        ud.m_ffire = !ud.m_ffire;
                    break;

                case 6: // select user map
if(!VOLUMEONE)
{                    if(boardfilename[0] == 0) break;

                    tempbuf[0] = 5;
                    tempbuf[1] = ud.m_level_number = 7;
                    tempbuf[2] = ud.m_volume_number = 0;
                    tempbuf[3] = ud.m_player_skill+1;

                    ud.level_number = ud.m_level_number;
                    ud.volume_number = ud.m_volume_number;

                    if( ud.m_player_skill == 3 ) ud.m_respawn_monsters = 1;
                    else ud.m_respawn_monsters = 0;

                    if(ud.m_coop == 0) ud.m_respawn_items = 1;
                    else ud.m_respawn_items = 0;

                    ud.m_respawn_inventory = 1;

                    tempbuf[4] = ud.m_monsters_off;
                    tempbuf[5] = ud.m_respawn_monsters;
                    tempbuf[6] = ud.m_respawn_items;
                    tempbuf[7] = ud.m_respawn_inventory;
                    tempbuf[8] = ud.m_coop;
                    tempbuf[9] = ud.m_marker;
                    tempbuf[10] = ud.m_ffire;

                    for(c=connecthead;c>=0;c=connectpoint2[c])
                    {
                        resetweapons(c);
                        resetinventory(c);

                        if(c != myconnectindex)
                            sendpacket(c,(uint8_t*)tempbuf,11);
                    }

                    newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill+1);
                    enterlevel(MODE_GAME);

                    return;
}
                case 7: // start game

                    tempbuf[0] = 5;
                    tempbuf[1] = ud.m_level_number;
                    tempbuf[2] = ud.m_volume_number;
                    tempbuf[3] = ud.m_player_skill+1;

                    if( ud.m_player_skill == 3 ) ud.m_respawn_monsters = 1;
                    else ud.m_respawn_monsters = 0;

                    if(ud.m_coop == 0) ud.m_respawn_items = 1;
                    else ud.m_respawn_items = 0;

                    ud.m_respawn_inventory = 1;

                    tempbuf[4] = ud.m_monsters_off;
                    tempbuf[5] = ud.m_respawn_monsters;
                    tempbuf[6] = ud.m_respawn_items;
                    tempbuf[7] = ud.m_respawn_inventory;
                    tempbuf[8] = ud.m_coop;
                    tempbuf[9] = ud.m_marker;
                    tempbuf[10] = ud.m_ffire;

                    for(c=connecthead;c>=0;c=connectpoint2[c])
                    {
                        resetweapons(c);
                        resetinventory(c);

                        if(c != myconnectindex)
                            sendpacket(c,(uint8_t*)tempbuf,11);
                    }

                    newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill+1);
                    enterlevel(MODE_GAME);

                    return;

            }

            c += 40;

            if(ud.m_coop==1) gametext(c+70,57-7-9,"COOPERATIVE PLAY",0,2+8+16);
            else if(ud.m_coop==2) gametext(c+70,57-7-9,"DUKEMATCH (NO SPAWN)",0,2+8+16);
            else gametext(c+70,57-7-9,"DUKEMATCH (SPAWN)",0,2+8+16);

 if(VOLUMEONE)
            gametext(c+70,57+16-7-9,volume_names[ud.m_volume_number],0,2+8+16);
 else
            gametext(c+70,57+16-7-9,volume_names[ud.m_volume_number],0,2+8+16);
 
            gametext(c+70,57+16+16-7-9,&level_names[11*ud.m_volume_number+ud.m_level_number][0],0,2+8+16);

            if(ud.m_monsters_off == 0 || ud.m_player_skill > 0)
                gametext(c+70,57+16+16+16-7-9,skill_names[ud.m_player_skill],0,2+8+16);
            else gametext(c+70,57+16+16+16-7-9,"NONE",0,2+8+16);

            if(ud.m_coop == 0)
            {
                if(ud.m_marker)
                    gametext(c+70,57+16+16+16+16-7-9,"ON",0,2+8+16);
                else gametext(c+70,57+16+16+16+16-7-9,"OFF",0,2+8+16);
            }

            if(ud.m_coop == 1)
            {
                if(ud.m_ffire)
                    gametext(c+70,57+16+16+16+16+16-7-9,"ON",0,2+8+16);
                else gametext(c+70,57+16+16+16+16+16-7-9,"OFF",0,2+8+16);
            }

            c -= 44;

            menutext(c,57-9,SHX(-2),PHX(-2),"GAME TYPE");

if (VOLUMEONE)
{            sprintf(text,"EPISODE %d",ud.m_volume_number+1);
            menutext(c,57+16-9,SHX(-3),1,text);
} 
else 
{
            sprintf(text,"EPISODE %d",ud.m_volume_number+1);
            menutext(c,57+16-9,SHX(-3),PHX(-3),text);
}

#ifndef ONELEVELDEMO
            sprintf(text,"LEVEL %d",ud.m_level_number+1);
            menutext(c,57+16+16-9,SHX(-4),PHX(-4),text);
#else
            sprintf(text,"LEVEL %d",ud.m_level_number+1);
            menutext(c,57+16+16-9,SHX(-4),1,text);
#endif
            menutext(c,57+16+16+16-9,SHX(-5),PHX(-5),"MONSTERS");

            if(ud.m_coop == 0)
                menutext(c,57+16+16+16+16-9,SHX(-6),PHX(-6),"MARKERS");
            else
                menutext(c,57+16+16+16+16-9,SHX(-6),1,"MARKERS");

            if(ud.m_coop == 1)
                menutext(c,57+16+16+16+16+16-9,SHX(-6),PHX(-6),"FR. FIRE");
            else menutext(c,57+16+16+16+16+16-9,SHX(-6),1,"FR. FIRE");

if(!VOLUMEONE)
{            menutext(c,57+16+16+16+16+16+16-9,SHX(-7),boardfilename[0] == 0,"USER MAP");
            if( boardfilename[0] != 0 )
                gametext(c+70+44,57+16+16+16+16+16,boardfilename,0,2+8+16);
}
else
{
			menutext(c,57+16+16+16+16+16+16-9,SHX(-7),1,"USER MAP");
}

            menutext(c,57+16+16+16+16+16+16+16-9,SHX(-8),PHX(-8),"START GAME");

    break;
    }

    if( (ps[myconnectindex].gm&MODE_MENU) != MODE_MENU)
    {
        vscrn();
        cameraclock = totalclock;
        cameradist = 65536L;
    }
}

void palto(uint8_t  r,uint8_t  g,uint8_t  b,int32_t e)
{
    int i;
    uint8_t  temparray[768];

    for(i=0;i<(256*3);i+=3)
    {
        temparray[i  ] =ps[myconnectindex].palette[i+0]+((((int32_t)r-(int32_t)ps[myconnectindex].palette[i+0])*(int32_t)(e&127))>>6);
        temparray[i+1] =ps[myconnectindex].palette[i+1]+((((int32_t)g-(int32_t)ps[myconnectindex].palette[i+1])*(int32_t)(e&127))>>6);
        temparray[i+2] =ps[myconnectindex].palette[i+2]+((((int32_t)b-(int32_t)ps[myconnectindex].palette[i+2])*(int32_t)(e&127))>>6);
    }

    setbrightness(ud.brightness>>2,temparray);
}


void drawoverheadmap(int32_t cposx, int32_t cposy, int32_t czoom, short cang)
{
        int32_t i, j, k, l, x1, y1, x2, y2, x3, y3, x4, y4, ox, oy, xoff, yoff;
        int32_t dax, day, cosang, sinang, xspan, yspan, sprx, spry;
        int32_t xrepeat, yrepeat, z1, z2, startwall, endwall, tilenum, daang;
        int32_t xvect, yvect, xvect2, yvect2;
        short p;
        uint8_t  col;
        walltype *wal, *wal2;
        spritetype *spr;

        xvect = sintable[(-cang)&2047] * czoom;
        yvect = sintable[(1536-cang)&2047] * czoom;
        xvect2 = mulscale16(xvect,yxaspect);
        yvect2 = mulscale16(yvect,yxaspect);

                //Draw red lines
        for(i=0;i<numsectors;i++)
        {
                if (!(show2dsector[i>>3]&(1<<(i&7)))) continue;

                startwall = sector[i].wallptr;
                endwall = sector[i].wallptr + sector[i].wallnum;

                z1 = sector[i].ceilingz; z2 = sector[i].floorz;

                for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
                {
                        k = wal->nextwall; if (k < 0) continue;

                        //if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;
                        //if ((k > j) && ((show2dwall[k>>3]&(1<<(k&7))) > 0)) continue;

                        if (sector[wal->nextsector].ceilingz == z1)
                                if (sector[wal->nextsector].floorz == z2)
                                        if (((wal->cstat|wall[wal->nextwall].cstat)&(16+32)) == 0) continue;

                        col = 139; //red
                        if ((wal->cstat|wall[wal->nextwall].cstat)&1) col = 234; //magenta

                        if (!(show2dsector[wal->nextsector>>3]&(1<<(wal->nextsector&7))))
                                col = 24;
            else continue;

                        ox = wal->x-cposx; oy = wal->y-cposy;
                        x1 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                        y1 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

                        wal2 = &wall[wal->point2];
                        ox = wal2->x-cposx; oy = wal2->y-cposy;
                        x2 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                        y2 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

                        drawline256(x1,y1,x2,y2,col);
                }
        }

                //Draw sprites
        k = ps[screenpeek].i;
        for(i=0;i<numsectors;i++)
        {
                if (!(show2dsector[i>>3]&(1<<(i&7)))) continue;
                for(j=headspritesect[i];j>=0;j=nextspritesect[j])
                        //if ((show2dsprite[j>>3]&(1<<(j&7))) > 0)
                        {
                spr = &sprite[j];

                if (j == k || (spr->cstat&0x8000) || spr->cstat == 257 || spr->xrepeat == 0) continue;

                                col = 71; //cyan
                                if (spr->cstat&1) col = 234; //magenta

                                sprx = spr->x;
                                spry = spr->y;

                if( (spr->cstat&257) != 0) switch (spr->cstat&48)
                                {
                    case 0: break;
                                                ox = sprx-cposx; oy = spry-cposy;
                                                x1 = dmulscale16(ox,xvect,-oy,yvect);
                                                y1 = dmulscale16(oy,xvect2,ox,yvect2);

                                                ox = (sintable[(spr->ang+512)&2047]>>7);
                                                oy = (sintable[(spr->ang)&2047]>>7);
                                                x2 = dmulscale16(ox,xvect,-oy,yvect);
                                                y2 = dmulscale16(oy,xvect,ox,yvect);

                                                x3 = mulscale16(x2,yxaspect);
                                                y3 = mulscale16(y2,yxaspect);

                                                drawline256(x1-x2+(xdim<<11),y1-y3+(ydim<<11),
                                                                                x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                                                drawline256(x1-y2+(xdim<<11),y1+x3+(ydim<<11),
                                                                                x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                                                drawline256(x1+y2+(xdim<<11),y1-x3+(ydim<<11),
                                                                                x1+x2+(xdim<<11),y1+y3+(ydim<<11),col);
                        break;

                                        case 16:
                        if( spr->picnum == LASERLINE )
                        {
                            x1 = sprx; y1 = spry;
                            tilenum = spr->picnum;
                            xoff = (int32_t)((int8_t  )((tiles[tilenum].animFlags>>8)&255))+((int32_t)spr->xoffset);
                            if ((spr->cstat&4) > 0)
                                xoff = -xoff;
                            k = spr->ang; l = spr->xrepeat;
                            dax = sintable[k&2047]*l; day = sintable[(k+1536)&2047]*l;
                            l = tiles[tilenum].dim.width;
                            k = (l>>1)+xoff;
                            x1 -= mulscale16(dax,k);
                            x2 = x1+mulscale16(dax,l);
                            y1 -= mulscale16(day,k);
                            y2 = y1+mulscale16(day,l);

                            ox = x1-cposx; oy = y1-cposy;
                            x1 = dmulscale16(ox,xvect,-oy,yvect);
                            y1 = dmulscale16(oy,xvect2,ox,yvect2);

                            ox = x2-cposx; oy = y2-cposy;
                            x2 = dmulscale16(ox,xvect,-oy,yvect);
                            y2 = dmulscale16(oy,xvect2,ox,yvect2);

                            drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                                                                x2+(xdim<<11),y2+(ydim<<11),col);
                        }

                        break;

                    case 32:

                                                tilenum = spr->picnum;
                                                xoff = (int32_t)((int8_t  )((tiles[tilenum].animFlags>>8)&255))+((int32_t)spr->xoffset);
                                                yoff = (int32_t)((int8_t  )((tiles[tilenum].animFlags>>16)&255))+((int32_t)spr->yoffset);
                                                if ((spr->cstat&4) > 0) xoff = -xoff;
                                                if ((spr->cstat&8) > 0) yoff = -yoff;

                                                k = spr->ang;
                                                cosang = sintable[(k+512)&2047];
                                        sinang = sintable[k];
                                                xspan = tiles[tilenum].dim.width;
                                        xrepeat = spr->xrepeat;
                                                yspan = tiles[tilenum].dim.height;
                                        yrepeat = spr->yrepeat;

                                                dax = ((xspan>>1)+xoff)*xrepeat; day = ((yspan>>1)+yoff)*yrepeat;
                                                x1 = sprx + dmulscale16(sinang,dax,cosang,day);
                                                y1 = spry + dmulscale16(sinang,day,-cosang,dax);
                                                l = xspan*xrepeat;
                                                x2 = x1 - mulscale16(sinang,l);
                                                y2 = y1 + mulscale16(cosang,l);
                                                l = yspan*yrepeat;
                                                k = -mulscale16(cosang,l); x3 = x2+k; x4 = x1+k;
                                                k = -mulscale16(sinang,l); y3 = y2+k; y4 = y1+k;

                                                ox = x1-cposx; oy = y1-cposy;
                                                x1 = dmulscale16(ox,xvect,-oy,yvect);
                                                y1 = dmulscale16(oy,xvect2,ox,yvect2);

                                                ox = x2-cposx; oy = y2-cposy;
                                                x2 = dmulscale16(ox,xvect,-oy,yvect);
                                                y2 = dmulscale16(oy,xvect2,ox,yvect2);

                                                ox = x3-cposx; oy = y3-cposy;
                                                x3 = dmulscale16(ox,xvect,-oy,yvect);
                                                y3 = dmulscale16(oy,xvect2,ox,yvect2);

                                                ox = x4-cposx; oy = y4-cposy;
                                                x4 = dmulscale16(ox,xvect,-oy,yvect);
                                                y4 = dmulscale16(oy,xvect2,ox,yvect2);

                                                drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                                                                x2+(xdim<<11),y2+(ydim<<11),col);

                                                drawline256(x2+(xdim<<11),y2+(ydim<<11),
                                                                                x3+(xdim<<11),y3+(ydim<<11),col);

                                                drawline256(x3+(xdim<<11),y3+(ydim<<11),
                                                                                x4+(xdim<<11),y4+(ydim<<11),col);

                                                drawline256(x4+(xdim<<11),y4+(ydim<<11),
                                                                                x1+(xdim<<11),y1+(ydim<<11),col);

                                                break;
                                }
                        }
        }

                //Draw white lines
        for(i=0;i<numsectors;i++)
        {
                if (!(show2dsector[i>>3]&(1<<(i&7)))) continue;

                startwall = sector[i].wallptr;
                endwall = sector[i].wallptr + sector[i].wallnum;

                k = -1;
                for(j=startwall,wal=&wall[startwall];j<endwall;j++,wal++)
                {
                        if (wal->nextwall >= 0) continue;

                        //if ((show2dwall[j>>3]&(1<<(j&7))) == 0) continue;

                        if (tiles[wal->picnum].dim.width == 0)
                            continue;
                        if (tiles[wal->picnum].dim.height== 0)
                            continue;

                        if (j == k)
                                { x1 = x2; y1 = y2; }
                        else
                        {
                                ox = wal->x-cposx; oy = wal->y-cposy;
                                x1 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                                y1 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);
                        }

                        k = wal->point2; wal2 = &wall[k];
                        ox = wal2->x-cposx; oy = wal2->y-cposy;
                        x2 = dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                        y2 = dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);

                        drawline256(x1,y1,x2,y2,24);
                }
        }

         for(p=connecthead;p >= 0;p=connectpoint2[p])
         {
          if(ud.scrollmode && p == screenpeek) continue;

          ox = sprite[ps[p].i].x-cposx; oy = sprite[ps[p].i].y-cposy;
                  daang = (sprite[ps[p].i].ang-cang)&2047;
                  if (p == screenpeek) { ox = 0; oy = 0; daang = 0; }
                  x1 = mulscale(ox,xvect,16) - mulscale(oy,yvect,16);
                  y1 = mulscale(oy,xvect2,16) + mulscale(ox,yvect2,16);

          if(p == screenpeek || ud.coop == 1 )
          {
                if(sprite[ps[p].i].xvel > 16 && ps[p].on_ground)
                    i = APLAYERTOP+((totalclock>>4)&3);
                else
                    i = APLAYERTOP;

                j = klabs(ps[p].truefz-ps[p].posz)>>8;
                j = mulscale(czoom*(sprite[ps[p].i].yrepeat+j),yxaspect,16);

                if(j < 22000) j = 22000;
                else if(j > (65536<<1)) j = (65536<<1);

                rotatesprite((x1<<4)+(xdim<<15),(y1<<4)+(ydim<<15),j,
                    daang,i,sprite[ps[p].i].shade,sprite[ps[p].i].pal,
                    (sprite[ps[p].i].cstat&2)>>1,windowx1,windowy1,windowx2,windowy2);
          }
         }	        
}



void endanimsounds(int32_t fr)
{
    switch(ud.volume_number)
    {
        case 0:break;
        case 1:
            switch(fr)
            {
                case 1:
                    sound(WIND_AMBIENCE);
                    break;
                case 26:
                    sound(ENDSEQVOL2SND1);
                    break;
                case 36:
                    sound(ENDSEQVOL2SND2);
                    break;
                case 54:
                    sound(THUD);
                    break;
                case 62:
                    sound(ENDSEQVOL2SND3);
                    break;
                case 75:
                    sound(ENDSEQVOL2SND4);
                    break;
                case 81:
                    sound(ENDSEQVOL2SND5);
                    break;
                case 115:
                    sound(ENDSEQVOL2SND6);
                    break;
                case 124:
                    sound(ENDSEQVOL2SND7);
                    break;
            }
            break;
        case 2:
            switch(fr)
            {
                case 1:
                    sound(WIND_REPEAT);
                    break;
                case 98:
                    sound(DUKE_GRUNT);
                    break;
                case 82+20:
                    sound(THUD);
                    sound(SQUISHED);
                    break;
                case 104+20:
                    sound(ENDSEQVOL3SND3);
                    break;
                case 114+20:
                    sound(ENDSEQVOL3SND2);
                    break;
                case 158:
                    sound(PIPEBOMB_EXPLODE);
                    break;
            }
            break;
    }
}

void logoanimsounds(int32_t fr)
{
    switch(fr)
    {
        case 1:
            sound(FLY_BY);
            break;
        case 19:
            sound(PIPEBOMB_EXPLODE);
            break;
    }
}

void intro4animsounds(int32_t fr)
{
    switch(fr)
    {
        case 1:
            sound(INTRO4_B);
            break;
        case 12:
        case 34:
            sound(SHORT_CIRCUIT);
            break;
        case 18:
            sound(INTRO4_5);
            break;
    }
}

void first4animsounds(int32_t fr)
{
    switch(fr)
    {
        case 1:
            sound(INTRO4_1);
            break;
        case 12:
            sound(INTRO4_2);
            break;
        case 7:
            sound(INTRO4_3);
            break;
        case 26:
            sound(INTRO4_4);
            break;
    }
}

void intro42animsounds(int32_t fr)
{
    switch(fr)
    {
        case 10:
            sound(INTRO4_6);
            break;
    }
}




void endanimvol41(int32_t fr)
{
    switch(fr)
    {
        case 3:
            sound(DUKE_UNDERWATER);
            break;
        case 35:
            sound(VOL4ENDSND1);
            break;
    }
}

void endanimvol42(int32_t fr)
{
    switch(fr)
    {
        case 11:
            sound(DUKE_UNDERWATER);
            break;
        case 20:
            sound(VOL4ENDSND1);
            break;
        case 39:
            sound(VOL4ENDSND2);
            break;
        case 50:
            FX_StopAllSounds();
            break;
    }
}

void endanimvol43(int32_t fr)
{
    switch(fr)
    {
        case 1:
            sound(BOSS4_DEADSPEECH);
            break;
        case 40:
            sound(VOL4ENDSND1);
            sound(DUKE_UNDERWATER);
            break;
        case 50:
            sound(BIGBANG);
            break;
    }
}


int32_t lastanimhack=0;
void playanm(char  *fn,uint8_t  t)
{
    uint8_t  *animbuf, *palptr;
    int32_t i, j, k, length=0, numframes=0;
    int32 handle=-1;

ESP_LOGV(TAG, "KB_FlushKeyboardQueue");
    if(t != 7 && t != 9 && t != 10 && t != 11)
        KB_FlushKeyboardQueue();

ESP_LOGV(TAG, "KB_KeyWaiting");
    if( KB_KeyWaiting() )
    {
        FX_StopAllSounds();
        goto ENDOFANIMLOOP;
    }
ESP_LOGV(TAG, "TCkopen4load");
    handle = TCkopen4load(fn,0);

    if(handle == -1) 
		return;

ESP_LOGV(TAG, "kfilelength");
    length = kfilelength(handle);

    tiles[MAXTILES-3-t].lock = 219+t;

ESP_LOGV(TAG, "allocache");
    if(anim == 0 || lastanimhack != (MAXTILES-3-t))
        allocache((uint8_t**)&anim,length+sizeof(anim_t),&tiles[MAXTILES-3-t].lock);

    animbuf = (uint8_t  *)(anim)+sizeof(anim_t);

    lastanimhack = (MAXTILES-3-t);

    tiles[MAXTILES-3-t].dim.width = 200;
    tiles[MAXTILES-3-t].dim.height = 320;

ESP_LOGV(TAG, "kread");
    kread(handle,animbuf,length);
ESP_LOGV(TAG, "kclose");
    kclose(handle);

ESP_LOGV(TAG, "ANIM_LoadAnim");
    ANIM_LoadAnim (animbuf);
    numframes = ANIM_NumFrames();

ESP_LOGV(TAG, "ANIM_GetPalette");
    palptr = ANIM_GetPalette();
    for(i=0;i<256;i++)
    {
            j = (i<<2); k = j-i;
            tempbuf[j+0] = (palptr[k+2]>>2);
            tempbuf[j+1] = (palptr[k+1]>>2);
            tempbuf[j+2] = (palptr[k+0]>>2);
            tempbuf[j+3] = 0;
    }

ESP_LOGV(TAG, "VBE_setPalette");
    VBE_setPalette(tempbuf);

    ototalclock = totalclock + 10;

    for(i=1;i<numframes;i++)
    {
       while(totalclock < ototalclock)
       {
ESP_LOGV(TAG, "KB_KeyWaiting");
          if( KB_KeyWaiting() )
              goto ENDOFANIMLOOP;
          getpackets();
       }

       if(t == 10) ototalclock += 14;
       else if(t == 9) ototalclock += 10;
       else if(t == 7) ototalclock += 18;
       else if(t == 6) ototalclock += 14;
       else if(t == 5) ototalclock += 9;
       else if(ud.volume_number == 3) ototalclock += 10;
       else if(ud.volume_number == 2) ototalclock += 10;
       else if(ud.volume_number == 1) ototalclock += 18;
       else                           ototalclock += 10;

ESP_LOGV(TAG, "ANIM_DrawFrame");
       tiles[MAXTILES-3-t].data = ANIM_DrawFrame(i);
ESP_LOGV(TAG, "rotatesprite");
       rotatesprite(0<<16,0<<16,65536L,512,MAXTILES-3-t,0,0,2+4+8+16+64, 0,0,xdim-1,ydim-1);
ESP_LOGV(TAG, "nextpage");
       nextpage();

       if(t == 8) endanimvol41(i);
       else if(t == 10) endanimvol42(i);
       else if(t == 11) endanimvol43(i);
       else if(t == 9) intro42animsounds(i);
       else if(t == 7) intro4animsounds(i);
       else if(t == 6) first4animsounds(i);
       else if(t == 5) logoanimsounds(i);
       else if(t < 4) endanimsounds(i);
        }

    ENDOFANIMLOOP:

ESP_LOGV(TAG, "ANIM_FreeAnim");
    ANIM_FreeAnim ();
    tiles[MAXTILES-3-t].lock = 1;
}

