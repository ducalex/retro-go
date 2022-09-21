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

#ifdef _WIN32
  #include <windows.h>
#elif defined(__APPLE__)
  #include "SDL.h"
#endif

#include "types.h"

#include "develop.h"
#include "scriplib.h"
#include "file_lib.h"
#include "gamedefs.h"
#include "keyboard.h"
#include "util_lib.h"
#include "function.h"
#include "control.h"
#include "sounds.h"
#include "config.h"
#include "sndcards.h"

#include "duke3d.h"

#include "console.h"
#include "cvars.h"
#include "cvar_defs.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "global.h"

#include "SDL.h"
#include "esp_attr.h"

#define MINITEXT_BLUE	0
#define MINITEXT_RED	2
#define MINITEXT_YELLOW	23
#define MINITEXT_GRAY	17

#define COLOR_ON  MINITEXT_YELLOW
#define COLOR_OFF MINITEXT_BLUE

#define IDFSIZE 479985668
// #define IDFSIZE 9961476
// #define IDFSIZE 16384
#define IDFILENAME "DUKE3D.IDF"

#define TIMERUPDATESIZ 32

int32_t cameradist = 0, cameraclock = 0;
uint8_t  eightytwofifty = 0;
uint8_t  playerswhenstarted;
uint8_t  qe,cp;

uint8_t  nHostForceDisableAutoaim = 0;

// Game play speed
int g_iTickRate = 120;
int g_iTicksPerFrame = 26;

int32 CommandSoundToggleOff = 0;
int32 CommandMusicToggleOff = 0;

// For addfaz's stun server. use /stun to activate
uint16_t g_bStun = 0;

char confilename[128] = {"GAME.CON"};
char boardfilename[128] = {0};
uint8_t  waterpal[768], slimepal[768], titlepal[768], drealms[768], endingpal[768];
char  firstdemofile[80] = { '\0' };

#define patchstatusbar(x1,y1,x2,y2)                                        \
{                                                                          \
    rotatesprite(0,(200-34)<<16,65536L,0,BOTTOMSTATUSBAR,4,0,10+16+64+128, \
        scale(x1,xdim,320),scale(y1,ydim,200),                             \
        scale(x2,xdim,320)-1,scale(y2,ydim,200)-1);                        \
}

void newint24( int errval, int ax, int bp, int si );

int recfilep,totalreccnt;
uint8_t  debug_on = 0,actor_tog = 0,memorycheckoveride=0;
uint8_t *rtsptr;


extern uint8_t  syncstate;
extern int32 numlumps;

FILE *frecfilep = (FILE *)NULL;
void pitch_test( void );

uint8_t  restorepalette,screencapt,nomorelogohack;
int sendmessagecommand = -1;

#if PLATFORM_DOS
task *TimerPtr=NULL;
#endif

extern int32_t lastvisinc;

// Build Engine port implements this.  --ryan.
#if PLATFORM_DOS
static void timerhandler(task *unused)
{
    totalclock++;
}

void inittimer()
{
    TimerPtr = TS_ScheduleTask( timerhandler,TICRATE, 1, NULL );
    TS_Dispatch();
}

void uninittimer(void)
{
   if (TimerPtr)
      TS_Terminate( TimerPtr );
   TimerPtr = NULL;
   TS_Shutdown();
}
#else
void timerhandler(void)
{
    totalclock++;
}
#endif

int gametext(int x,int y,char  *t,uint8_t  s,short dabits)
{
    short ac,newx;
    char* oldt;
    uint8_t  centre;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tiles[ac].dim.width;
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,dabits,0,0,xdim-1,ydim-1);

        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tiles[ac].dim.width;

        t++;
    }

    return (x);
}

int gametextpal(int x,int y,char  *t,uint8_t  s,uint8_t  p)
{
    short ac,newx;
    uint8_t  centre;
    char* oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;

    if(centre)
    {
        while(*t)
        {
            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            if(*t >= '0' && *t <= '9')
                newx += 8;
            else newx += tiles[ac].dim.width;
            t++;
        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM )
            break;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,2+8+16,0,0,xdim-1,ydim-1);
        if(*t >= '0' && *t <= '9')
            x += 8;
        else x += tiles[ac].dim.width;

        t++;
    }

    return (x);
}

int gametextpart(int x,int y,char  *t,uint8_t  s,short p)
{
    short ac,newx, cnt;
    uint8_t  centre;
    char * oldt;

    centre = ( x == (320>>1) );
    newx = 0;
    oldt = t;
    cnt = 0;

    if(centre)
    {
        while(*t)
        {
            if(cnt == p) break;

            if(*t == 32) {newx+=5;t++;continue;}
            else ac = *t - '!' + STARTALPHANUM;

            if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

            newx += tiles[ac].dim.width;
            t++;
            cnt++;

        }

        t = oldt;
        x = (320>>1)-(newx>>1);
    }

    cnt = 0;
    while(*t)
    {
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + STARTALPHANUM;

        if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

        if(cnt == p)
        {
            rotatesprite(x<<16,y<<16,65536L,0,ac,s,1,2+8+16,0,0,xdim-1,ydim-1);
            break;
        }
        else
            rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,2+8+16,0,0,xdim-1,ydim-1);

        x += tiles[ac].dim.width;

        t++;
        cnt++;
    }

    return (x);
}

int minitext(int x,int y,char  *str,uint8_t  p,uint8_t  sb)
{
    short ac;
    char  buf[128];
    char  *t;

    strncpy (buf, str, 128);
    buf[127] = 0;
    t = buf;

    while(*t)
    {
        *t = toupper(*t);
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + MINIFONT;

        rotatesprite(x<<16,y<<16,65536L,0,ac,0,p,sb,0,0,xdim-1,ydim-1);
        x += 4; // tilesizx[ac]+1;

        t++;
    }
    return (x);
}

int minitextshade(int x,int y,char  *str,uint8_t  s,uint8_t  p,uint8_t  sb)
{
    short ac;
    char  buf[128];
    char  *t;

    strncpy (buf, str, 128);
    buf[127] = 0;
    t = buf;

    while(*t)
    {
        *t = toupper(*t);
        if(*t == 32) {x+=5;t++;continue;}
        else ac = *t - '!' + MINIFONT;

        rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,sb,0,0,xdim-1,ydim-1);
        x += 4; // tilesizx[ac]+1;

        t++;
    }
    return (x);
}

void gamenumber(int32_t x,int32_t y,int32_t n,uint8_t  s)
{
    char  b[10];


    //
    // uint8_t  * ltoa(int32_t l, uint8_t  * buffer, int radix);
    // is NON-STANDARD and equivalent to STANDARD
    // (void) sprintf(buffer, "%ld", l);
    //ltoa(n,b,10);
    sprintf(b,"%d",n);
    gametext(x,y,b,s,2+8+16);
}


char  recbuf[80];
void allowtimetocorrecterrorswhenquitting(void)
{
     int32_t i, j, oldtotalclock;

     ready2send = 0;

     for(j=0;j<8;j++)
     {
          oldtotalclock = totalclock;

          while (totalclock < oldtotalclock+TICSPERFRAME)
              getpackets();

          if(KB_KeyPressed(sc_Escape)) return;

          packbuf[0] = 127;
          for(i=connecthead;i>=0;i=connectpoint2[i])
                if (i != myconnectindex)
                     sendpacket(i,packbuf,1);
     }
}

#define MAXUSERQUOTES 4
int32_t quotebot, quotebotgoal;
short user_quote_time[MAXUSERQUOTES];
char  user_quote[MAXUSERQUOTES][128];
// uint8_t  typebuflen,typebuf[41];

static void adduserquote(char  *daquote)
{
    int32_t i;

    for(i=MAXUSERQUOTES-1;i>0;i--)
    {
        strcpy(user_quote[i],user_quote[i-1]);
        user_quote_time[i] = user_quote_time[i-1];
    }
    strcpy(user_quote[0],daquote);
    user_quote_time[0] = 180;
    pub = NUMPAGES;
}

char  *grpVersion2char_from_crc(unsigned int crc32_grp_to_identify)
{
	char  *id;
	int i=0;

	id = crc32lookup[MAX_KNOWN_GRP].name; // unknown version

	for(i=0; i<MAX_KNOWN_GRP; i++)
	{
		if(crc32lookup[i].crc32==crc32_grp_to_identify)
			id = crc32lookup[i].name;
	}

	return(id);
}

char  *grpVersion2char(uint8_t  grp_to_identify)
{
	char  *id;

	switch(grp_to_identify)
	{
		case DUKEITOUTINDC_GRP:
			id = "v1.5 DC PACK";
			break;
		case SHAREWARE_GRP13:
			id = "v1.3 SHAREW.";
			break;
		case ATOMIC_GRP14_15:
			id = "v1.5 ATOMIC";
			break;
		case REGULAR_GRP13D:
			id = "v1.3D FULL";
			break;
		case UNKNOWN_GRP:
			id = "vX.X UNKNOWN";
			break;
		default:
			Error(EXIT_FAILURE,"Failed the GRP Identification\n");
			break;
	}

	return(id);
}

//This is a function from the Engine module, used in getpackets.
void sampletimer(void);

void getpackets(void)
{
    int32_t i, j, k, l;
    short other, packbufleng;
    input *osyn, *nsyn;

	sampletimer();
    if(qe == 0 && KB_KeyPressed(sc_LeftControl) && KB_KeyPressed(sc_LeftAlt) && KB_KeyPressed(sc_Delete))
    {
        qe = 1;
        gameexit("Quick Exit.");
    }

	// not a net game
    if (numplayers < 2)
	{
		//printf("getpackets() numplayers < 2");
		return;
	}

    while ((packbufleng = getpacket(&other,packbuf)) > 0)
    {
#ifdef _DEBUG_NETWORKING_
		printf("RECEIVED PACKET: type: %d : len %d\n", packbuf[0], packbufleng);
#endif

        switch(packbuf[0])
        {
			case 253:
				// This should have already been handled by mmulti.cpp so ignore it
				printf("Invalid Packet: %d", packbuf[0]);
			break;

            case 125:
                cp = 0;
                break;

            case 126:
                multiflag = 2;
                multiwhat = 0;
                multiwho = other;
                multipos = packbuf[1];
                loadplayer( multipos );
                multiflag = 0;
                break;
            case 0:  //[0] (receive master sync buffer)
                j = 1;

                if ((movefifoend[other]&(TIMERUPDATESIZ-1)) == 0)
                    for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                    {
                        if (playerquitflag[i] == 0) continue;
                        if (i == myconnectindex)
                            otherminlag = (int32_t)((int8_t  )packbuf[j]);
                        j++;
                    }

                osyn = (input *)&inputfifo[(movefifoend[connecthead]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[connecthead])&(MOVEFIFOSIZ-1)][0];

                k = j;
                for(i=connecthead;i>=0;i=connectpoint2[i])
                    j += playerquitflag[i];
                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    if (playerquitflag[i] == 0) continue;

                    l = packbuf[k++];
                    if (i == myconnectindex)
                        { j += ((l&1)<<1)+(l&2)+((l&4)>>2)+((l&8)>>3)+((l&16)>>4)+((l&32)>>5)+((l&64)>>6)+((l&128)>>7); continue; }

                    copybufbyte(&osyn[i],&nsyn[i],sizeof(input));
                    if (l&1)   nsyn[i].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                    if (l&2)   nsyn[i].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                    if (l&4)   nsyn[i].avel = (int8_t  )packbuf[j++];
                    if (l&8)   nsyn[i].bits = ((nsyn[i].bits&0xffffff00)|((int32_t)packbuf[j++]));
                    if (l&16)  nsyn[i].bits = ((nsyn[i].bits&0xffff00ff)|((int32_t)packbuf[j++])<<8);
                    if (l&32)  nsyn[i].bits = ((nsyn[i].bits&0xff00ffff)|((int32_t)packbuf[j++])<<16);
                    if (l&64)  nsyn[i].bits = ((nsyn[i].bits&0x00ffffff)|((int32_t)packbuf[j++])<<24);
                    if (l&128) nsyn[i].horz = (int8_t  )packbuf[j++];

                    if (nsyn[i].bits&(1<<26)) playerquitflag[i] = 0;
                    movefifoend[i]++;
                }

                while (j != packbufleng)
                {
                    for(i=connecthead;i>=0;i=connectpoint2[i])
                        if(i != myconnectindex)
                    {
                        syncval[i][syncvalhead[i]&(MOVEFIFOSIZ-1)] = packbuf[j];
                        syncvalhead[i]++;
                    }
                    j++;
                }

                for(i=connecthead;i>=0;i=connectpoint2[i])
                    if (i != myconnectindex)
                        for(j=1;j<movesperpacket;j++)
                        {
                            copybufbyte(&nsyn[i],&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i],sizeof(input));
                            movefifoend[i]++;
                        }

                 movefifosendplc += movesperpacket;

                break;
            case 1:  //[1] (receive slave sync buffer)
                j = 2; k = packbuf[1];

                osyn = (input *)&inputfifo[(movefifoend[other]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[other])&(MOVEFIFOSIZ-1)][0];

                copybufbyte(&osyn[other],&nsyn[other],sizeof(input));
                if (k&1)   nsyn[other].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&2)   nsyn[other].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&4)   nsyn[other].avel = (int8_t  )packbuf[j++];
                if (k&8)   nsyn[other].bits = ((nsyn[other].bits&0xffffff00)|((int32_t)packbuf[j++]));
                if (k&16)  nsyn[other].bits = ((nsyn[other].bits&0xffff00ff)|((int32_t)packbuf[j++])<<8);
                if (k&32)  nsyn[other].bits = ((nsyn[other].bits&0xff00ffff)|((int32_t)packbuf[j++])<<16);
                if (k&64)  nsyn[other].bits = ((nsyn[other].bits&0x00ffffff)|((int32_t)packbuf[j++])<<24);
                if (k&128) nsyn[other].horz = (int8_t  )packbuf[j++];
                movefifoend[other]++;

                while (j != packbufleng)
                {
                    syncval[other][syncvalhead[other]&(MOVEFIFOSIZ-1)] = packbuf[j++];
                    syncvalhead[other]++;
                }

                for(i=1;i<movesperpacket;i++)
                {
                    copybufbyte(&nsyn[other],&inputfifo[movefifoend[other]&(MOVEFIFOSIZ-1)][other],sizeof(input));
                    movefifoend[other]++;
                }

                break;

            case 4: // message talk T
                strcpy(recbuf,(char*)packbuf+1);
                recbuf[packbufleng-1] = 0;

                adduserquote(recbuf);
                sound(EXITMENUSOUND);

                pus = NUMPAGES;
                pub = NUMPAGES;

                break;

            case 5:
                ud.m_level_number = ud.level_number = packbuf[1];
                ud.m_volume_number = ud.volume_number = packbuf[2];
                ud.m_player_skill = ud.player_skill = packbuf[3];
                ud.m_monsters_off = ud.monsters_off = packbuf[4];
                ud.m_respawn_monsters = ud.respawn_monsters = packbuf[5];
                ud.m_respawn_items = ud.respawn_items = packbuf[6];
                ud.m_respawn_inventory = ud.respawn_inventory = packbuf[7];
                ud.m_coop = packbuf[8];
                ud.m_marker = ud.marker = packbuf[9];
                ud.m_ffire = ud.ffire = packbuf[10];

                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    resetweapons(i);
                    resetinventory(i);
                }

                newgame(ud.volume_number,ud.level_number,ud.player_skill);
                ud.coop = ud.m_coop;

                enterlevel(MODE_GAME);

                break;

            case 6: // get names
                for (i=2;packbuf[i] && i<=11;i++) // limit size of name
                    ud.user_name[other][i-2] = packbuf[i];
                ud.user_name[other][i-2] = 0;

				// we allow the old rancidmeat 19.1 to connect, using the old grpVersion system w/ BYTEVERSION
				if(packbuf[1] == BYTEVERSION_27 || packbuf[1] == BYTEVERSION_117)
				{
					// Old rancid was using either BYTEVERSION_27 or BYTEVERSION_117
					Error(EXIT_SUCCESS,	"STOP: Your opponent is using an obsolete version\n"
										"Please ask him to update to xDuke v%d.%d!\n", CHOCOLATE_DUKE_REV_X, CHOCOLATE_DUKE_REV_DOT_Y);
				}
				break;

            case 9:
                for (i=1;i<packbufleng;i++)
                    ud.wchoice[other][i-1] = packbuf[i];
                break;

            case 7:

                if(numlumps == 0) break;

                if (SoundToggle == 0 || ud.lockout == 1 || FXDevice == NumSoundCards)
                    break;
                rtsptr = RTS_GetSound(packbuf[1]-1);
                if (*rtsptr == 'C')
                    FX_PlayVOC3D(rtsptr,0,0,0,255,-packbuf[1]);
                else
                    FX_PlayWAV3D(rtsptr,0,0,0,255,-packbuf[1]);
                rtsplaying = 7;
                break;
            case 8:
                ud.m_level_number = ud.level_number = packbuf[1];
                ud.m_volume_number = ud.volume_number = packbuf[2];
                ud.m_player_skill = ud.player_skill = packbuf[3];
                ud.m_monsters_off = ud.monsters_off = packbuf[4];
                ud.m_respawn_monsters = ud.respawn_monsters = packbuf[5];
                ud.m_respawn_items = ud.respawn_items = packbuf[6];
                ud.m_respawn_inventory = ud.respawn_inventory = packbuf[7];
                ud.m_coop = ud.coop = packbuf[8];
                ud.m_marker = ud.marker = packbuf[9];
                ud.m_ffire = ud.ffire = packbuf[10];

                copybufbyte(packbuf+10,boardfilename,packbufleng-11);
                boardfilename[packbufleng-11] = 0;

                for(i=connecthead;i>=0;i=connectpoint2[i])
                {
                    resetweapons(i);
                    resetinventory(i);
                }

                newgame(ud.volume_number,ud.level_number,ud.player_skill);
                enterlevel(MODE_GAME);
                break;

            case 16:
                movefifoend[other] = movefifoplc = movefifosendplc = fakemovefifoplc = 0;
                syncvalhead[other] = syncvaltottail = 0L;

            case 17:
                j = 1;

                if ((movefifoend[other]&(TIMERUPDATESIZ-1)) == 0)
                    if (other == connecthead)
                        for(i=connectpoint2[connecthead]; i>=0; i=connectpoint2[i])
                        {
                            if (i == myconnectindex)
							{
								otherminlag = (int32_t)((int8_t  )packbuf[j]);
							}

                            j++;
                        }

                osyn = (input *)&inputfifo[(movefifoend[other]-1)&(MOVEFIFOSIZ-1)][0];
                nsyn = (input *)&inputfifo[(movefifoend[other])&(MOVEFIFOSIZ-1)][0];

                copybufbyte(&osyn[other],&nsyn[other],sizeof(input));
                k = packbuf[j++];
                if (k&1)   nsyn[other].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&2)   nsyn[other].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
                if (k&4)   nsyn[other].avel = (int8_t  )packbuf[j++];
                if (k&8)   nsyn[other].bits = ((nsyn[other].bits&0xffffff00)|((int32_t)packbuf[j++]));
                if (k&16)  nsyn[other].bits = ((nsyn[other].bits&0xffff00ff)|((int32_t)packbuf[j++])<<8);
                if (k&32)  nsyn[other].bits = ((nsyn[other].bits&0xff00ffff)|((int32_t)packbuf[j++])<<16);
                if (k&64)  nsyn[other].bits = ((nsyn[other].bits&0x00ffffff)|((int32_t)packbuf[j++])<<24);
                if (k&128) nsyn[other].horz = (int8_t  )packbuf[j++];
                movefifoend[other]++;

                for(i=1;i<movesperpacket;i++)
                {
                    copybufbyte(&nsyn[other],&inputfifo[movefifoend[other]&(MOVEFIFOSIZ-1)][other],sizeof(input));
                    movefifoend[other]++;
                }

                if (j > packbufleng)
				{
					printf("INVALID GAME PACKET!!! (%d too many bytes) (j= %d, packbuflen= %d, type: %d)\n",j-packbufleng, j, packbufleng, packbuf[0]);
				}

                while (j != packbufleng)
                {
                    syncval[other][syncvalhead[other]&(MOVEFIFOSIZ-1)] = packbuf[j++];
                    syncvalhead[other]++;
                }

                break;
            case 127:
                break;

#ifdef CHECK_XDUKE_REV
			case 131: // xDuke Rev ID
				memcpy(ud.rev[other], packbuf, 10);
				break;
#endif

			case 132: // get map CRC of opponents (to debug out of synch)
				ud.mapCRC[other] = (uint16_t)packbuf[1] + (uint16_t)(packbuf[2]<<8);
				break;

			case 133: // client refused to disable the autoaim by host

				Error(EXIT_SUCCESS,	"One or more players refused to play with AutoAim OFF because this breaks\n"
									"the official Duke's gameplay. Please restart without this option...\n");

				break;

			case 134: // Get GRP CRC32 + Con size + exeCRC + conCRC
				memcpy(ud.groupefil_crc32[other], packbuf+1, sizeof(groupefil_crc32));
				memcpy(ud.conSize+other, packbuf+1+sizeof(groupefil_crc32), sizeof(ud.conSize[0]));
				memcpy(ud.conCRC+other, packbuf+1+sizeof(groupefil_crc32)+sizeof(ud.conSize[0]), sizeof(ud.conCRC[0]));
				memcpy(ud.exeCRC+other, packbuf+1+sizeof(groupefil_crc32)+sizeof(ud.conSize[0])+sizeof(ud.conCRC[0]), sizeof(ud.exeCRC[0]));
				break;

            case 250:
                {
					playerreadyflag[other]++;
					printf("Player %d '%s' is ready...\n", other, ud.user_name[other]);
				}
                break;
            case 255:
                gameexitanycase();
                break;
        }
    }
}

//From player.c
void computergetinput(int32_t snum, input *syn);
IRAM_ATTR void faketimerhandler()
{
    int32_t i, j, k;
    input *osyn, *nsyn;

    //Check if we should quit the game.
    if ((qe == 0 && KB_KeyPressed(sc_LeftControl) && KB_KeyPressed(sc_LeftAlt) && KB_KeyPressed(sc_Delete)) ||
        (qe == 0 && KB_KeyPressed(sc_LeftAlt) && KB_KeyPressed(sc_F4)))
    {
        qe = 1;
        gameexit("Quick Exit.");
    }

    //Has it been 120ticks ?
    if ((totalclock < ototalclock+TICSPERFRAME) || (ready2send == 0))
		return; // Returns here when playing a demo.

    //YES : Add 120tick
    ototalclock += TICSPERFRAME;

    //Check network stuff.
    getpackets();
    if (getoutputcirclesize() >= 16)
        return;


    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (i != myconnectindex)
            if (movefifoend[i] < movefifoend[myconnectindex]-200)
                return;

     if( !CONSOLE_IsActive())
     {
        getinput(myconnectindex);
     }

     avgfvel += loc.fvel; // x
     avgsvel += loc.svel; // y
     avgavel += loc.avel;
     avghorz += loc.horz;
     avgbits |= loc.bits;
     if (movefifoend[myconnectindex]&(movesperpacket-1))
     {
          copybufbyte(&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex],
                          &inputfifo[movefifoend[myconnectindex]&(MOVEFIFOSIZ-1)][myconnectindex],sizeof(input));
          movefifoend[myconnectindex]++;
          return;
     }

     nsyn = &inputfifo[movefifoend[myconnectindex]&(MOVEFIFOSIZ-1)][myconnectindex];
     nsyn[0].fvel = avgfvel/movesperpacket;
     nsyn[0].svel = avgsvel/movesperpacket;
     nsyn[0].avel = avgavel/movesperpacket;
     nsyn[0].horz = avghorz/movesperpacket;
     nsyn[0].bits = avgbits;
     avgfvel = avgsvel = avgavel = avghorz = avgbits = 0;
     movefifoend[myconnectindex]++;

     if (numplayers < 2)
     {
          if (ud.multimode > 1) for(i=connecthead;i>=0;i=connectpoint2[i])
              if(i != myconnectindex)
              {
                  //clearbufbyte(&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i],sizeof(input),0L);
                  if(ud.playerai)
                      computergetinput(i,&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i]);
                  movefifoend[i]++;
              }
          return;
     }

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (i != myconnectindex)
        {
            k = (movefifoend[myconnectindex]-1)-movefifoend[i];
            myminlag[i] = min(myminlag[i],k);
            mymaxlag = max(mymaxlag,k);
        }

    if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
    {
        i = mymaxlag-bufferjitter; mymaxlag = 0;
        if (i > 0) bufferjitter += ((3+i)>>2);
        else if (i < 0) bufferjitter -= ((1-i)>>2);
    }

    if (networkmode == 1)
    {
        packbuf[0] = 17;

		if ((movefifoend[myconnectindex]-1) == 0)
		{
			packbuf[0] = 16;
		}

        j = 1;

            //Fix timers and buffer/jitter value
        if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
        {
            if (myconnectindex != connecthead)
            {
                i = myminlag[connecthead]-otherminlag;
                if (klabs(i) > 8)
				{
					i >>= 1;
				}
                else
				if (klabs(i) > 2)
				{
					i = ksgn(i);
				}
                else
				{
					i = 0;
				}

                totalclock -= TICSPERFRAME*i;
                myminlag[connecthead] -= i; otherminlag += i;
            }

            if (myconnectindex == connecthead)
                for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
                    packbuf[j++] = min(max(myminlag[i],-128),127);

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        osyn = (input *)&inputfifo[(movefifoend[myconnectindex]-2)&(MOVEFIFOSIZ-1)][myconnectindex];
        nsyn = (input *)&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex];

        k = j;
        packbuf[j++] = 0;

        if (nsyn[0].fvel != osyn[0].fvel)
        {
            packbuf[j++] = (uint8_t )nsyn[0].fvel;
            packbuf[j++] = (uint8_t )(nsyn[0].fvel>>8);
            packbuf[k] |= 1;
        }
        if (nsyn[0].svel != osyn[0].svel)
        {
            packbuf[j++] = (uint8_t )nsyn[0].svel;
            packbuf[j++] = (uint8_t )(nsyn[0].svel>>8);
            packbuf[k] |= 2;
        }
        if (nsyn[0].avel != osyn[0].avel)
        {
            packbuf[j++] = (int8_t  )nsyn[0].avel;
            packbuf[k] |= 4;
        }
        if ((nsyn[0].bits^osyn[0].bits)&0x000000ff) packbuf[j++] = (nsyn[0].bits&255), packbuf[k] |= 8;
        if ((nsyn[0].bits^osyn[0].bits)&0x0000ff00) packbuf[j++] = ((nsyn[0].bits>>8)&255), packbuf[k] |= 16;
        if ((nsyn[0].bits^osyn[0].bits)&0x00ff0000) packbuf[j++] = ((nsyn[0].bits>>16)&255), packbuf[k] |= 32;
        if ((nsyn[0].bits^osyn[0].bits)&0xff000000) packbuf[j++] = ((nsyn[0].bits>>24)&255), packbuf[k] |= 64;
        if (nsyn[0].horz != osyn[0].horz)
        {
            packbuf[j++] = (uint8_t )nsyn[0].horz;
            packbuf[k] |= 128;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (i != myconnectindex)
                sendpacket(i,packbuf,j);

        return;
    }
    if (myconnectindex != connecthead)   //Slave
    {
            //Fix timers and buffer/jitter value
        if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
        {
            i = myminlag[connecthead]-otherminlag;
            if (klabs(i) > 8) i >>= 1;
            else if (klabs(i) > 2) i = ksgn(i);
            else i = 0;

            totalclock -= TICSPERFRAME*i;
            myminlag[connecthead] -= i; otherminlag += i;

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        packbuf[0] = 1; packbuf[1] = 0; j = 2;

        osyn = (input *)&inputfifo[(movefifoend[myconnectindex]-2)&(MOVEFIFOSIZ-1)][myconnectindex];
        nsyn = (input *)&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex];

        if (nsyn[0].fvel != osyn[0].fvel)
        {
            packbuf[j++] = (uint8_t )nsyn[0].fvel;
            packbuf[j++] = (uint8_t )(nsyn[0].fvel>>8);
            packbuf[1] |= 1;
        }
        if (nsyn[0].svel != osyn[0].svel)
        {
            packbuf[j++] = (uint8_t )nsyn[0].svel;
            packbuf[j++] = (uint8_t )(nsyn[0].svel>>8);
            packbuf[1] |= 2;
        }
        if (nsyn[0].avel != osyn[0].avel)
        {
            packbuf[j++] = (int8_t  )nsyn[0].avel;
            packbuf[1] |= 4;
        }
        if ((nsyn[0].bits^osyn[0].bits)&0x000000ff) packbuf[j++] = (nsyn[0].bits&255), packbuf[1] |= 8;
        if ((nsyn[0].bits^osyn[0].bits)&0x0000ff00) packbuf[j++] = ((nsyn[0].bits>>8)&255), packbuf[1] |= 16;
        if ((nsyn[0].bits^osyn[0].bits)&0x00ff0000) packbuf[j++] = ((nsyn[0].bits>>16)&255), packbuf[1] |= 32;
        if ((nsyn[0].bits^osyn[0].bits)&0xff000000) packbuf[j++] = ((nsyn[0].bits>>24)&255), packbuf[1] |= 64;
        if (nsyn[0].horz != osyn[0].horz)
        {
            packbuf[j++] = (uint8_t )nsyn[0].horz;
            packbuf[1] |= 128;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        sendpacket(connecthead,packbuf,j);
        return;
    }

        //This allows allow packet-resends
    for(i=connecthead;i>=0;i=connectpoint2[i])
        if (movefifoend[i] <= movefifosendplc)
        {
            packbuf[0] = 127;
            for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
               sendpacket(i,packbuf,1);
            return;
        }

    while (1)  //Master
    {
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (playerquitflag[i] && (movefifoend[i] <= movefifosendplc)) return;

        osyn = (input *)&inputfifo[(movefifosendplc-1)&(MOVEFIFOSIZ-1)][0];
        nsyn = (input *)&inputfifo[(movefifosendplc  )&(MOVEFIFOSIZ-1)][0];

            //MASTER -> SLAVE packet
        packbuf[0] = 0; j = 1;

            //Fix timers and buffer/jitter value
        if ((movefifosendplc&(TIMERUPDATESIZ-1)) == 0)
        {
            for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
               if (playerquitflag[i])
                packbuf[j++] = min(max(myminlag[i],-128),127);

            for(i=connecthead;i>=0;i=connectpoint2[i])
                myminlag[i] = 0x7fffffff;
        }

        k = j;
        for(i=connecthead;i>=0;i=connectpoint2[i])
           j += playerquitflag[i];
        for(i=connecthead;i>=0;i=connectpoint2[i])
        {
            if (playerquitflag[i] == 0) continue;

            packbuf[k] = 0;
            if (nsyn[i].fvel != osyn[i].fvel)
            {
                packbuf[j++] = (uint8_t )nsyn[i].fvel;
                packbuf[j++] = (uint8_t )(nsyn[i].fvel>>8);
                packbuf[k] |= 1;
            }
            if (nsyn[i].svel != osyn[i].svel)
            {
                packbuf[j++] = (uint8_t )nsyn[i].svel;
                packbuf[j++] = (uint8_t )(nsyn[i].svel>>8);
                packbuf[k] |= 2;
            }
            if (nsyn[i].avel != osyn[i].avel)
            {
                packbuf[j++] = (int8_t  )nsyn[i].avel;
                packbuf[k] |= 4;
            }
            if ((nsyn[i].bits^osyn[i].bits)&0x000000ff) packbuf[j++] = (nsyn[i].bits&255), packbuf[k] |= 8;
            if ((nsyn[i].bits^osyn[i].bits)&0x0000ff00) packbuf[j++] = ((nsyn[i].bits>>8)&255), packbuf[k] |= 16;
            if ((nsyn[i].bits^osyn[i].bits)&0x00ff0000) packbuf[j++] = ((nsyn[i].bits>>16)&255), packbuf[k] |= 32;
            if ((nsyn[i].bits^osyn[i].bits)&0xff000000) packbuf[j++] = ((nsyn[i].bits>>24)&255), packbuf[k] |= 64;
            if (nsyn[i].horz != osyn[i].horz)
            {
                packbuf[j++] = (uint8_t )nsyn[i].horz;
                packbuf[k] |= 128;
            }
            k++;
        }

        while (syncvalhead[myconnectindex] != syncvaltail)
        {
            packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
            syncvaltail++;
        }

        for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
            if (playerquitflag[i])
            {
                 sendpacket(i,packbuf,j);
                 if (nsyn[i].bits&(1<<26))
                    playerquitflag[i] = 0;
            }

        movefifosendplc += movesperpacket;
    }

}

extern int32_t cacnum;

typedef struct {
    uint8_t *hand;
    int32_t leng;
    uint8_t  *lock; }
cactype;
extern cactype cac[];

void caches(void)
{
     short i,k;
    char text[512];

     k = 0;
     for(i=0;i<cacnum;i++)
          if ((*cac[i].lock) >= 200)
          {
                sprintf(text,"Locked- %d: Leng:%d, Lock:%d",i,cac[i].leng,*cac[i].lock);
                printext256(0L,k,31,-1,text,1); k += 6;
          }

     k += 6;

     for(i=1;i<11;i++)
          if (lumplockbyte[i] >= 200)
          {
                sprintf(text,"RTS Locked %hd:",i);
                printext256(0L,k,31,-1,text,1); k += 6;
          }


}

// FIX_00024: A key can be assigned to the new SHOW_INFO function. Display map CRC when
//             in deathmatch. Usefull to identify who loaded a wrong map in multiplayer.
void dispVersion(void)
{
	int i;
	int offx, offy, stepx, stepy;
    char text[512];

	offx = 21; offy = 30;
	stepx = 73; stepy = 20;

	// black translucent background underneath lists
	rotatesprite(0<<16, 0<<16, 65536l<<5, 0, BLANK, 8, 0, 1+2+8+16+64,
		scale(0,xdim,320),scale(26,ydim,200),
		scale(320-0,xdim,320)-1,scale(200-((ud.multimode>4)?(161-1*7)-stepy:(161-1*7)),ydim,200)-1);

	// FIX_00009: Show map CRC and GRP file version of each player in case of Out Of Synch
	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		// Disp name
		sprintf(text,"%s", ud.user_name[i]);
		minitext(offx+(stepx*(i&3)),offy+0+((i&4)>>2)*stepy, text, sprite[ps[i].i].pal, 2+8+16);

		// Disp MAP CRC
		if(ps[i].fakeplayer)
			sprintf(text,"MAP CRC: (bot)");
		else
			sprintf(text,"MAP CRC: %X", ud.mapCRC[i]);
		minitext(offx+(stepx*(i&3)),offy+7+((i&4)>>2)*stepy, text, COLOR_ON,2+8+16);

	}
}

void checksync(void)
{
      int32_t i;

      for(i=connecthead;i>=0;i=connectpoint2[i])
            if (syncvalhead[i] == syncvaltottail) break;
      if (i < 0)
      {
             syncstat = 0;
             do
             {
                     for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
					 {
						 if (syncval[i][syncvaltottail&(MOVEFIFOSIZ-1)] != syncval[connecthead][syncvaltottail&(MOVEFIFOSIZ-1)])
						 {
                                 syncstat = 1;
						}
					 }

                     syncvaltottail++;
                     for(i=connecthead;i>=0;i=connectpoint2[i])
                     {
						 if (syncvalhead[i] == syncvaltottail)
						 {
							 break;
						 }
					 }
             } while (i < 0);
      }

      if (connectpoint2[connecthead] < 0)
	  {
		  syncstat = 0;
	  }

      if (syncstat)
      {
			minitext(21,30+35+30, "Out Of Sync - Please restart game", COLOR_ON,2+8+16);
			// FIX_00090: Removed info key. FPS were shown after CRC msg. CRC not always removed. (Turrican)
			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				if (ud.mapCRC[connecthead]!=ud.mapCRC[i])
				{
					minitext(21,30+42+30, "Map CRC mismatching. Please use exactly the same map.", COLOR_ON,2+8+16);
					dispVersion();
				}
				else
				minitext(21,30+42+30, "Verify the con files. Close your P2P if any", COLOR_ON,2+8+16);

			}
	  }

      if (syncstate)
      {
          //printext256(4L,160L,31,0,"Missed Network packet!",0);
          //printext256(4L,138L,31,0,"RUN DN3DHELP.EXE for information.",0);
          minitext(21,30+35+30, "Missed Network packet!", COLOR_ON,2+8+16);
      }

}


void check_fta_sounds(short i)
{
    if(sprite[i].extra > 0) switch(PN)
    {
        case LIZTROOPONTOILET:
        case LIZTROOPJUSTSIT:
        case LIZTROOPSHOOT:
        case LIZTROOPJETPACK:
        case LIZTROOPDUCKING:
        case LIZTROOPRUNNING:
        case LIZTROOP:
            spritesound(PRED_RECOG,i);
            break;
        case LIZMAN:
        case LIZMANSPITTING:
        case LIZMANFEEDING:
        case LIZMANJUMP:
            spritesound(CAPT_RECOG,i);
            break;
        case PIGCOP:
        case PIGCOPDIVE:
            spritesound(PIG_RECOG,i);
            break;
        case RECON:
            spritesound(RECO_RECOG,i);
            break;
        case DRONE:
            spritesound(DRON_RECOG,i);
            break;
        case COMMANDER:
        case COMMANDERSTAYPUT:
            spritesound(COMM_RECOG,i);
            break;
        case ORGANTIC:
            spritesound(TURR_RECOG,i);
            break;
        case OCTABRAIN:
        case OCTABRAINSTAYPUT:
            spritesound(OCTA_RECOG,i);
            break;
        case BOSS1:
            sound(BOS1_RECOG);
            break;
        case BOSS2:
            if(sprite[i].pal == 1)
                sound(BOS2_RECOG);
            else sound(WHIPYOURASS);
            break;
        case BOSS3:
            if(sprite[i].pal == 1)
                sound(BOS3_RECOG);
            else sound(RIPHEADNECK);
            break;
        case BOSS4:
        case BOSS4STAYPUT:
            if(sprite[i].pal == 1)
                sound(BOS4_RECOG);
            sound(BOSS4_FIRSTSEE);
            break;
        case GREENSLIME:
            spritesound(SLIM_RECOG,i);
            break;
    }
}

short inventory(spritetype *s)
{
    switch(s->picnum)
    {
        case FIRSTAID:
        case STEROIDS:
        case HEATSENSOR:
        case BOOTS:
        case JETPACK:
        case HOLODUKE:
        case AIRTANK:
            return 1;
    }
    return 0;
}


short badguy(spritetype *s)
{

    switch(s->picnum)
    {
            case SHARK:
            case RECON:
            case DRONE:
            case LIZTROOPONTOILET:
            case LIZTROOPJUSTSIT:
            case LIZTROOPSTAYPUT:
            case LIZTROOPSHOOT:
            case LIZTROOPJETPACK:
            case LIZTROOPDUCKING:
            case LIZTROOPRUNNING:
            case LIZTROOP:
            case OCTABRAIN:
            case COMMANDER:
            case COMMANDERSTAYPUT:
            case PIGCOP:
            case EGG:
            case PIGCOPSTAYPUT:
            case PIGCOPDIVE:
            case LIZMAN:
            case LIZMANSPITTING:
            case LIZMANFEEDING:
            case LIZMANJUMP:
            case ORGANTIC:
            case BOSS1:
            case BOSS2:
            case BOSS3:
            case BOSS4:
            case GREENSLIME:
            case GREENSLIME+1:
            case GREENSLIME+2:
            case GREENSLIME+3:
            case GREENSLIME+4:
            case GREENSLIME+5:
            case GREENSLIME+6:
            case GREENSLIME+7:
            case RAT:
            case ROTATEGUN:
                return 1;
    }
    if( actortype[s->picnum] ) return 1;

    return 0;
}


short badguypic(short pn)
{

    switch(pn)
    {
            case SHARK:
            case RECON:
            case DRONE:
            case LIZTROOPONTOILET:
            case LIZTROOPJUSTSIT:
            case LIZTROOPSTAYPUT:
            case LIZTROOPSHOOT:
            case LIZTROOPJETPACK:
            case LIZTROOPDUCKING:
            case LIZTROOPRUNNING:
            case LIZTROOP:
            case OCTABRAIN:
            case COMMANDER:
            case COMMANDERSTAYPUT:
            case PIGCOP:
            case EGG:
            case PIGCOPSTAYPUT:
            case PIGCOPDIVE:
            case LIZMAN:
            case LIZMANSPITTING:
            case LIZMANFEEDING:
            case LIZMANJUMP:
            case ORGANTIC:
            case BOSS1:
            case BOSS2:
            case BOSS3:
            case BOSS4:
            case GREENSLIME:
            case GREENSLIME+1:
            case GREENSLIME+2:
            case GREENSLIME+3:
            case GREENSLIME+4:
            case GREENSLIME+5:
            case GREENSLIME+6:
            case GREENSLIME+7:
            case RAT:
            case ROTATEGUN:
                return 1;
    }

    if( actortype[pn] ) return 1;

    return 0;
}



void myos(int32_t x, int32_t y, short tilenum, int8_t shade, uint8_t  orientation)
{
    uint8_t  p;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

    p = sector[ps[screenpeek].cursectnum].floorpal;
    rotatesprite(x<<16,y<<16,65536L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);
}

void myospal(int32_t x, int32_t y, short tilenum, int8_t shade, uint8_t  orientation, uint8_t  p)
{
//    uint8_t  fp;
    short a;

    if(orientation&4)
        a = 1024;
    else a = 0;

//    fp = sector[ps[screenpeek].cursectnum].floorpal;

    rotatesprite(x<<16,y<<16,65536L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);

}

void invennum(int32_t x,int32_t y,uint8_t  num1,uint8_t  ha,uint8_t  sbits)
{
    char  dabuf[80] = {0};
    sprintf(dabuf,"%d",num1);
    if(num1 > 99)
    {
        rotatesprite((x-4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
    }
    else if(num1 > 9)
    {
        rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
    }
    else
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
}

void orderweaponnum(short ind,int32_t x,int32_t y,int32_t num1, int32_t num2,uint8_t  ha)
{
    rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x-3)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);

    minitextshade(x+1,y-4,"ORDER",26,6,2+8+16+128);
}

void weaponnum(short ind,int32_t x,int32_t y,int32_t num1, int32_t num2,uint8_t  ha)
{
    char  dabuf[80] = {0};

    rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x-3)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x+9)<<16,y<<16,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

    if(num1 > 99) num1 = 99;
    if(num2 > 99) num2 = 99;

    sprintf(dabuf,"%d",num1);
    if(num1 > 9)
    {
        rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%d",num2);
    if(num2 > 9)
    {
        rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}

void weaponnum999(uint8_t  ind,int32_t x,int32_t y,int32_t num1, int32_t num2,uint8_t  ha)
{
    char  dabuf[80] = {0};

    rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x-4)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
    rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%d",num1);
    if(num1 > 99)
    {
        rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else if(num1 > 9)
    {
        rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

    sprintf(dabuf,"%d",num2);
    if(num2 > 99)
    {
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+21)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+25)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else if(num2 > 9)
    {
        rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
        rotatesprite((x+21)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
    }
    else rotatesprite((x+25)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}


    //REPLACE FULLY
void weapon_amounts(struct player_struct *p,int32_t x,int32_t y,int32_t u)
{
     int cw;

     cw = p->curr_weapon;

     if (u&4)
     {
         if (u != 0xffffffff) patchstatusbar(96,178,96+12,178+6);
         weaponnum999(PISTOL_WEAPON,x,y,
                     p->ammo_amount[PISTOL_WEAPON],max_ammo_amount[PISTOL_WEAPON],
                     12-20*(cw == PISTOL_WEAPON) );
     }
     if (u&8)
     {
         if (u != 0xffffffff) patchstatusbar(96,184,96+12,184+6);
         weaponnum999(SHOTGUN_WEAPON,x,y+6,
                     p->ammo_amount[SHOTGUN_WEAPON],max_ammo_amount[SHOTGUN_WEAPON],
                     (!p->gotweapon[SHOTGUN_WEAPON]*9)+12-18*
                     (cw == SHOTGUN_WEAPON) );
     }
     if (u&16)
     {
         if (u != 0xffffffff) patchstatusbar(96,190,96+12,190+6);
         weaponnum999(CHAINGUN_WEAPON,x,y+12,
                      p->ammo_amount[CHAINGUN_WEAPON],max_ammo_amount[CHAINGUN_WEAPON],
                      (!p->gotweapon[CHAINGUN_WEAPON]*9)+12-18*
                      (cw == CHAINGUN_WEAPON) );
     }
     if (u&32)
     {
         if (u != 0xffffffff) patchstatusbar(135,178,135+8,178+6);
         weaponnum(RPG_WEAPON,x+39,y,
                  p->ammo_amount[RPG_WEAPON],max_ammo_amount[RPG_WEAPON],
                  (!p->gotweapon[RPG_WEAPON]*9)+12-19*
                  (cw == RPG_WEAPON) );
     }
     if (u&64)
     {
         if (u != 0xffffffff) patchstatusbar(135,184,135+8,184+6);
         weaponnum(HANDBOMB_WEAPON,x+39,y+6,
                     p->ammo_amount[HANDBOMB_WEAPON],max_ammo_amount[HANDBOMB_WEAPON],
                     (((!p->ammo_amount[HANDBOMB_WEAPON])|(!p->gotweapon[HANDBOMB_WEAPON]))*9)+12-19*
                     ((cw == HANDBOMB_WEAPON) || (cw == HANDREMOTE_WEAPON)));
     }
     if (u&128)
     {
         if (u != 0xffffffff) patchstatusbar(135,190,135+8,190+6);

		if(VOLUMEONE)
		{
			orderweaponnum(SHRINKER_WEAPON,x+39,y+12,
	                     p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
	                     (!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
	                     (cw == SHRINKER_WEAPON) );
		}
		else
		{
			if(p->subweapon&(1<<GROW_WEAPON))
	             weaponnum(SHRINKER_WEAPON,x+39,y+12,
	                 p->ammo_amount[GROW_WEAPON],max_ammo_amount[GROW_WEAPON],
	                 (!p->gotweapon[GROW_WEAPON]*9)+12-18*
	                 (cw == GROW_WEAPON) );
	         else
	             weaponnum(SHRINKER_WEAPON,x+39,y+12,
	                 p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
	                 (!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
	                 (cw == SHRINKER_WEAPON) );
		}
     }
     if (u&256)
     {
		if (u != 0xffffffff) patchstatusbar(166,178,166+8,178+6);

		if(VOLUMEONE)
		{
	        orderweaponnum(DEVISTATOR_WEAPON,x+70,y,
                     p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
                     (!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
                     (cw == DEVISTATOR_WEAPON) );
		}
		else
		{
	         weaponnum(DEVISTATOR_WEAPON,x+70,y,
	                     p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
	                     (!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
	                     (cw == DEVISTATOR_WEAPON) );
		}
     }
     if (u&512)
     {
		if (u != 0xffffffff) patchstatusbar(166,184,166+8,184+6);
		if(VOLUMEONE)
		{
	         orderweaponnum(TRIPBOMB_WEAPON,x+70,y+6,
	                 p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
	                 (!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
	                 (cw == TRIPBOMB_WEAPON) );
		}
		else
		{
			weaponnum(TRIPBOMB_WEAPON,x+70,y+6,
		                     p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
		                     (!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
		                     (cw == TRIPBOMB_WEAPON) );
		}
     }

     if (u&65536L)
     {
         if (u != 0xffffffff) patchstatusbar(166,190,166+8,190+6);
		if(VOLUMEONE)
		{
			orderweaponnum(-1,x+70,y+12,
	                     p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
	                     (!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
	                     (cw == FREEZE_WEAPON) );
		}
		else
		{
	         weaponnum(-1,x+70,y+12,
	                     p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
	                     (!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
	                     (cw == FREEZE_WEAPON) );
		}
     }
}

void digitalnumber(int32_t x,int32_t y,int32_t n,uint8_t  s,uint8_t  cs)
{
    short i, j, k, p, c;
    char  b[10];

    //
    // uint8_t  * ltoa(int32_t l, uint8_t  * buffer, int radix);
    // is NON-STANDARD and equivalent to STANDARD
    // (void) sprintf(buffer, "%ld", l);
    //ltoa(n,b,10);
    sprintf(b,"%d",n);

    i = strlen(b);
    j = 0;

    for(k=0;k<i;k++)
    {
        p = DIGITALNUM+*(b+k)-'0';
        j += tiles[p].dim.width+1;
    }
    c = x-(j>>1);

    j = 0;
    for(k=0;k<i;k++)
    {
        p = DIGITALNUM+*(b+k)-'0';
        rotatesprite((c+j)<<16,y<<16,65536L,0,p,s,0,cs,0,0,xdim-1,ydim-1);
        j += tiles[p].dim.width+1;
    }
}

/*

void scratchmarks(int32_t x,int32_t y,int32_t n,uint8_t  s,uint8_t  p)
{
    int32_t i, ni;

    ni = n/5;
    for(i=ni;i >= 0;i--)
    {
        overwritesprite(x-2,y,SCRATCH+4,s,0,0);
        x += tilesizx[SCRATCH+4]-1;
    }

    ni = n%5;
    if(ni) overwritesprite(x,y,SCRATCH+ni-1,s,p,0);
}
  */
void displayinventory(struct player_struct *p)
{
    short n, j, xoff, y;

    j = xoff = 0;

    n = (p->jetpack_amount > 0)<<3; if(n&8) j++;
    n |= ( p->scuba_amount > 0 )<<5; if(n&32) j++;
    n |= (p->steroids_amount > 0)<<1; if(n&2) j++;
    n |= ( p->holoduke_amount > 0)<<2; if(n&4) j++;
    n |= (p->firstaid_amount > 0); if(n&1) j++;
    n |= (p->heat_amount > 0)<<4; if(n&16) j++;
    n |= (p->boot_amount > 0)<<6; if(n&64) j++;

    xoff = 160-(j*11);

    j = 0;

    if(ud.screen_size > 4)
        y = 154;
    else y = 172;

    if(ud.screen_size == 4)
    {
        if(ud.multimode > 1)
            xoff += 56;
        else xoff += 65;
    }

    while( j <= 9 )
    {
        if( n&(1<<j) )
        {
            switch( n&(1<<j) )
            {
                case   1:
                rotatesprite(xoff<<16,y<<16,65536L,0,FIRSTAID_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   2:
                rotatesprite((xoff+1)<<16,y<<16,65536L,0,STEROIDS_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   4:
                rotatesprite((xoff+2)<<16,y<<16,65536L,0,HOLODUKE_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case   8:
                rotatesprite(xoff<<16,y<<16,65536L,0,JETPACK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case  16:
                rotatesprite(xoff<<16,y<<16,65536L,0,HEAT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case  32:
                rotatesprite(xoff<<16,y<<16,65536L,0,AIRTANK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
                case 64:
                rotatesprite(xoff<<16,(y-1)<<16,65536L,0,BOOT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
            }

            xoff += 22;

            if(p->inven_icon == j+1)
                rotatesprite((xoff-2)<<16,(y+19)<<16,65536L,1024,ARROW,-32,0,2+16,windowx1,windowy1,windowx2,windowy2);
        }

        j++;
    }
}



void displayfragbar(void)
{
    short i, j;
    char text[512];

    j = 0;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        if(i > j) j = i;

    rotatesprite(0,0,65600L,0,FRAGBAR,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 4) rotatesprite(319,(8)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 8) rotatesprite(319,(16)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
    if(j >= 12) rotatesprite(319,(24)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        minitext(21+(73*(i&3)),2+((i&28)<<1),&ud.user_name[i][0],sprite[ps[i].i].pal,2+8+16+128);
        sprintf(text,"%d",ps[i].frag-ps[i].fraggedself);
        minitext(17+50+(73*(i&3)),2+((i&28)<<1),text,sprite[ps[i].i].pal,2+8+16+128);
    }
}

void display_boardfilename_FPS_weapon(short *offx, short *offy, short *stepx, short *stepy)
{

	short i;

	// FIX_00025: Can toggle FPS and map name during a game (use dnrate OR toggle
	//            from menu when in deathmatch).

	// Display boardfilename and FPS
	if(ud.tickrate&1)
	{
		tics(*offx, *offy, COLOR_ON);
		*offy += *stepy;
	}
	if(ud.tickrate&2)
		dispVersion();

	// We display the weapons here instead of changing the function
	// displayweapon() because the display will be much faster
	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		if (ud.hideweapon && i==screenpeek)
			drawsmallweapon(ps[i].curr_weapon, 1, 130, (ud.screen_size<=4)?170:140);
	}
}


// FIX_00026: Weapon can now be hidden (on your screen only).
void drawsmallweapon(short weapon, float scale, short x, short y)
{
	float t = 60000;
	int s;
	float offsetx, offsety;


	switch(weapon)
	{
		case  KNEE_WEAPON			: s=0;					break;
		case  PISTOL_WEAPON			: s=FIRSTGUNSPRITE;
										offsetx = 8;
										offsety = 7;
										break;
		case  SHOTGUN_WEAPON		: s=SHOTGUNSPRITE;
										t = 45000;
										offsetx = -1;
										offsety = 9;
										break;
		case  CHAINGUN_WEAPON		: s=CHAINGUNSPRITE;
										t = 45000;
										offsetx = -1;
										offsety = 9;
										break;
		case  RPG_WEAPON			: s=RPGSPRITE;
										t = 45000;
										offsetx = 4;
										offsety = 9;
										break;
		case  HANDBOMB_WEAPON		: s=HEAVYHBOMB;
										t=20000;
										offsetx = 16;
										offsety = 13;
										break;
		case  SHRINKER_WEAPON		: s=SHRINKERSPRITE;
										t = 30000;
										offsetx = 6;
										offsety = 14;
										break;
		case  DEVISTATOR_WEAPON		: s=DEVISTATORSPRITE;
										t = 45000;
										offsetx = 3;
										offsety = 9;
										break;
		case  TRIPBOMB_WEAPON		: s=TRIPBOMBSPRITE;
										t = 75000;
										offsetx = 10;
										offsety = 12;
										break;
		case  FREEZE_WEAPON			:	s=FREEZESPRITE;
										t = 45000;
										offsetx = 1;
										offsety = 6;
										break;
		case  HANDREMOTE_WEAPON		: s=0;
										break;
		case  GROW_WEAPON			: s=GROWSPRITEICON;
										t = 30000;
										offsetx = 6;
										offsety = 4;
										break;
		default						: s=0;
	}

	if(s)
		rotatesprite((x+(short)(offsetx*scale))<<16,(y+(short)(offsety*scale))<<16,(int)(t*scale),0,s,0,0,2+8+16,0,0,xdim-1,ydim-1);

	return;
}

void coolgaugetext(short snum)
{
    struct player_struct *p;
    int32_t i, j, o, ss, u;
    uint8_t  permbit;
	short offx = 3, offy = 3, stepx=60, stepy=6;
    char text[512];

    p = &ps[snum];

    if (p->invdisptime > 0)
    {
        displayinventory(p);
    }


    if(ps[snum].gm&MODE_MENU)
        if( (current_menu >= 400  && current_menu <= 405) )
            return;

	offy += countfragbars(); //add fragbars
	display_boardfilename_FPS_weapon(&offx, &offy, &stepx, &stepy);


    ss = ud.screen_size; if (ss < 4) return;

    // Draw the multi player frag status bar
    if ( ud.multimode > 1 && ud.coop != 1 )
    {
        if (pus)
            {
                displayfragbar();
            }
        else
        {
            for(i=connecthead;i>=0;i=connectpoint2[i])
            {
                if (ps[i].frag != sbar.frag[i])
                {
                    displayfragbar();
                    break;
                }
            }
        }
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (i != myconnectindex)
                sbar.frag[i] = ps[i].frag;
    }

    if (ss == 4)   //DRAW MINI STATUS BAR:
    {
    	// FIX_00027: Added an extra small statusbar (HUD)
		if(ud.extended_screen_size>0)
		{
			offx = 5; offy = 160;

			sprintf(text,"%d", ps[screenpeek].ammo_amount[ps[screenpeek].curr_weapon]);
			minitext(offx+26,offy+21,text,COLOR_ON,2+8+16); //minitext: 2 red light, 23 yellow
			sprintf(text,"%d", ps[screenpeek].last_extra);
			gametext(offx,offy+20,text,ps[screenpeek].last_extra<=50?15:0,2+8+16); //minitext: 2 red light, 23 yellow
			rotatesprite((offx+0*10)<<16,(offy+28)<<16,20000,0,SHIELD,ps[screenpeek].shield_amount?25:100,0,2+8+16,0,0,xdim-1,ydim-1);
			rotatesprite((offx+0*10)<<16,(offy+28)<<16,ksqrt(ps[screenpeek].shield_amount)*20000/10,0,SHIELD,0,0,2+8+16,0,0,xdim-1,ydim-1);
			rotatesprite((offx+1*10)<<16,(offy+28)<<16,35000,0,JETPACK_ICON,ps[screenpeek].jetpack_amount?25:100,0,2+8+16,0,0,xdim-1,ydim-1);
			rotatesprite((offx+1*10)<<16,(offy+28)<<16,ksqrt(ps[screenpeek].jetpack_amount)*35000/40,0,JETPACK_ICON,0,0,2+8+16,0,0,xdim-1,ydim-1);
			rotatesprite((offx+2*10-1)<<16,(offy+28)<<16,35000,0,STEROIDS_ICON,ps[screenpeek].steroids_amount?25:100,0,2+8+16,0,0,xdim-1,ydim-1);
			rotatesprite((offx+2*10-1)<<16,(offy+28)<<16,ksqrt(ps[screenpeek].steroids_amount)*35000/20,0,STEROIDS_ICON,5,0,2+8+16,0,0,xdim-1,ydim-1);
			rotatesprite((offx+3*10-3)<<16,(offy+28)<<16,40000,0,FIRSTAID_ICON,ps[screenpeek].firstaid_amount?25:100,0,2+8+16,0,0,xdim-1,ydim-1);
			rotatesprite((offx+3*10-3)<<16,(offy+28)<<16,ksqrt(ps[screenpeek].firstaid_amount)*40000/10,0,FIRSTAID_ICON,0,0,2+8+16,0,0,xdim-1,ydim-1);
		}
		else
		{
			if (p->inven_icon)
				rotatesprite(69<<16,(200-30)<<16,65536L,0,INVENTORYBOX,0,21,10+16,0,0,xdim-1,ydim-1);
			rotatesprite(5<<16,(200-28)<<16,65536L,0,HEALTHBOX,0,21,10+16,0,0,xdim-1,ydim-1);

			if(sprite[p->i].pal == 1 && p->last_extra < 2)
				digitalnumber(20,200-17,1,-16,10+16);
			else digitalnumber(20,200-17,p->last_extra,-16,10+16);

			rotatesprite(37<<16,(200-28)<<16,65536L,0,AMMOBOX,0,21,10+16,0,0,xdim-1,ydim-1);

			if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
			digitalnumber(53,200-17,p->ammo_amount[i],-16,10+16);

			o = 158; permbit = 0;
			if (p->inven_icon)
			{
				switch(p->inven_icon)
				{
					case 1: i = FIRSTAID_ICON; break;
					case 2: i = STEROIDS_ICON; break;
					case 3: i = HOLODUKE_ICON; break;
					case 4: i = JETPACK_ICON; break;
					case 5: i = HEAT_ICON; break;
					case 6: i = AIRTANK_ICON; break;
					case 7: i = BOOT_ICON; break;
					default: i = -1;
				}
				if (i >= 0) rotatesprite((231-o)<<16,(200-21)<<16,65536L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);

				minitext(292-30-o,190,"%",6,10+16+permbit);

				j = 0x80000000;
				switch(p->inven_icon)
				{
					case 1: i = p->firstaid_amount; break;
					case 2: i = ((p->steroids_amount+3)>>2); break;
					case 3: i = ((p->holoduke_amount+15)/24); j = p->holoduke_on; break;
					case 4: i = ((p->jetpack_amount+15)>>4); j = p->jetpack_on; break;
					case 5: i = p->heat_amount/12; j = p->heat_on; break;
					case 6: i = ((p->scuba_amount+63)>>6); break;
					case 7: i = (p->boot_amount>>1); break;
				}
				invennum(284-30-o,200-6,(uint8_t )i,0,10+permbit);
				if (j > 0) minitext(288-30-o,180,"ON",0,10+16+permbit);
				else if (j != 0x80000000) minitext(284-30-o,180,"OFF",2,10+16+permbit);
				if (p->inven_icon >= 6) minitext(284-35-o,180,"AUTO",2,10+16+permbit);
			}
		}
        return;
    }

        //DRAW/UPDATE FULL STATUS BAR:

    if (pus) { pus = 0; u = 0xffffffff; } else u = 0;

    if (sbar.frag[myconnectindex] != p->frag) { sbar.frag[myconnectindex] = p->frag; u |= 32768; }
    if (sbar.got_access != p->got_access) { sbar.got_access = p->got_access; u |= 16384; }
    if (sbar.last_extra != p->last_extra) { sbar.last_extra = p->last_extra; u |= 1; }
    if (sbar.shield_amount != p->shield_amount) { sbar.shield_amount = p->shield_amount; u |= 2; }
    if (sbar.curr_weapon != p->curr_weapon) { sbar.curr_weapon = p->curr_weapon; u |= (4+8+16+32+64+128+256+512+1024+65536L); }
    for(i=1;i < 10;i++)
    {
        if (sbar.ammo_amount[i] != p->ammo_amount[i]) {
        sbar.ammo_amount[i] = p->ammo_amount[i]; if(i < 9) u |= ((2<<i)+1024); else u |= 65536L+1024; }
        if (sbar.gotweapon[i] != p->gotweapon[i]) { sbar.gotweapon[i] =
        p->gotweapon[i]; if(i < 9 ) u |= ((2<<i)+1024); else u |= 65536L+1024; }
    }
    if (sbar.inven_icon != p->inven_icon) { sbar.inven_icon = p->inven_icon; u |= (2048+4096+8192); }
    if (sbar.holoduke_on != p->holoduke_on) { sbar.holoduke_on = p->holoduke_on; u |= (4096+8192); }
    if (sbar.jetpack_on != p->jetpack_on) { sbar.jetpack_on = p->jetpack_on; u |= (4096+8192); }
    if (sbar.heat_on != p->heat_on) { sbar.heat_on = p->heat_on; u |= (4096+8192); }
    if (sbar.firstaid_amount != p->firstaid_amount) { sbar.firstaid_amount = p->firstaid_amount; u |= 8192; }
    if (sbar.steroids_amount != p->steroids_amount) { sbar.steroids_amount = p->steroids_amount; u |= 8192; }
    if (sbar.holoduke_amount != p->holoduke_amount) { sbar.holoduke_amount = p->holoduke_amount; u |= 8192; }
    if (sbar.jetpack_amount != p->jetpack_amount) { sbar.jetpack_amount = p->jetpack_amount; u |= 8192; }
    if (sbar.heat_amount != p->heat_amount) { sbar.heat_amount = p->heat_amount; u |= 8192; }
    if (sbar.scuba_amount != p->scuba_amount) { sbar.scuba_amount = p->scuba_amount; u |= 8192; }
    if (sbar.boot_amount != p->boot_amount) { sbar.boot_amount = p->boot_amount; u |= 8192; }
    if (u == 0) return;

    //0 - update health
    //1 - update armor
    //2 - update PISTOL_WEAPON ammo
    //3 - update SHOTGUN_WEAPON ammo
    //4 - update CHAINGUN_WEAPON ammo
    //5 - update RPG_WEAPON ammo
    //6 - update HANDBOMB_WEAPON ammo
    //7 - update SHRINKER_WEAPON ammo
    //8 - update DEVISTATOR_WEAPON ammo
    //9 - update TRIPBOMB_WEAPON ammo
    //10 - update ammo display
    //11 - update inventory icon
    //12 - update inventory on/off
    //13 - update inventory %
    //14 - update keys
    //15 - update kills
    //16 - update FREEZE_WEAPON ammo

    if (u == 0xffffffff)
    {
        patchstatusbar(0,0,320,200);
        if (ud.multimode > 1 && ud.coop != 1)
            rotatesprite(277<<16,(200-27)<<16,65536L,0,KILLSICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
    }
    if (ud.multimode > 1 && ud.coop != 1)
    {
        if (u&32768)
        {
            if (u != 0xffffffff) patchstatusbar(276,183,299,193);
            digitalnumber(287,200-17,max(p->frag-p->fraggedself,0),-16,10+16+128);
        }
    }
    else
    {
        if (u&16384)
        {
            if (u != 0xffffffff) patchstatusbar(275,182,299,194);
            if (p->got_access&4) rotatesprite(275<<16,182<<16,65536L,0,ACCESS_ICON,0,23,10+16+128,0,0,xdim-1,ydim-1);
            if (p->got_access&2) rotatesprite(288<<16,182<<16,65536L,0,ACCESS_ICON,0,21,10+16+128,0,0,xdim-1,ydim-1);
            if (p->got_access&1) rotatesprite(281<<16,189<<16,65536L,0,ACCESS_ICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
        }
    }
    if (u&(4+8+16+32+64+128+256+512+65536L)) weapon_amounts(p,96,182,u);

    if (u&1)
    {
        if (u != 0xffffffff) patchstatusbar(20,183,43,193);
        if(sprite[p->i].pal == 1 && p->last_extra < 2)
            digitalnumber(32,200-17,1,-16,10+16+128);
        else digitalnumber(32,200-17,p->last_extra,-16,10+16+128);
    }
    if (u&2)
    {
        if (u != 0xffffffff) patchstatusbar(52,183,75,193);
        digitalnumber(64,200-17,p->shield_amount,-16,10+16+128);
    }

    if (u&1024)
    {
        if (u != 0xffffffff) patchstatusbar(196,183,219,193);
        if (p->curr_weapon != KNEE_WEAPON)
        {
            if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
            digitalnumber(230-22,200-17,p->ammo_amount[i],-16,10+16+128);
        }
    }

    if (u&(2048+4096+8192))
    {
        if (u != 0xffffffff)
        {
            if (u&(2048+4096)) { patchstatusbar(231,179,265,197); }
                              else { patchstatusbar(250,190,261,195); }
        }
        if (p->inven_icon)
        {
            o = 0; permbit = 128;

            if (u&(2048+4096))
            {
                switch(p->inven_icon)
                {
                    case 1: i = FIRSTAID_ICON; break;
                    case 2: i = STEROIDS_ICON; break;
                    case 3: i = HOLODUKE_ICON; break;
                    case 4: i = JETPACK_ICON; break;
                    case 5: i = HEAT_ICON; break;
                    case 6: i = AIRTANK_ICON; break;
                    case 7: i = BOOT_ICON; break;
                }
                rotatesprite((231-o)<<16,(200-21)<<16,65536L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);
                minitext(292-30-o,190,"%",6,10+16+permbit);
                if (p->inven_icon >= 6) minitext(284-35-o,180,"AUTO",2,10+16+permbit);
            }
            if (u&(2048+4096))
            {
                switch(p->inven_icon)
                {
                    case 3: j = p->holoduke_on; break;
                    case 4: j = p->jetpack_on; break;
                    case 5: j = p->heat_on; break;
                    default: j = 0x80000000;
                }
                if (j > 0) minitext(288-30-o,180,"ON",0,10+16+permbit);
                else if (j != 0x80000000) minitext(284-30-o,180,"OFF",2,10+16+permbit);
            }
            if (u&8192)
            {
                switch(p->inven_icon)
                {
                    case 1: i = p->firstaid_amount; break;
                    case 2: i = ((p->steroids_amount+3)>>2); break;
                    case 3: i = ((p->holoduke_amount+15)/24); break;
                    case 4: i = ((p->jetpack_amount+15)>>4); break;
                    case 5: i = p->heat_amount/12; break;
                    case 6: i = ((p->scuba_amount+63)>>6); break;
                    case 7: i = (p->boot_amount>>1); break;
                }
                invennum(284-30-o,200-6,(uint8_t )i,0,10+permbit);
            }
        }
    }
}


#define AVERAGEFRAMES 16
static int32_t frameval[AVERAGEFRAMES], framecnt = 0;

void tics(short offx, short offy, short color)
{
    int32_t i;
	char  fps[512], mapname[512];
	int32_t currentFps;
	static int32_t fpsAvg = 0, savedFps = 0;
	static boolean toggle = true;
    char text[512];

	strcpy(mapname,boardfilename);
	for(i=0;i<512;i++)
		if(mapname[i]=='.')
			mapname[i]=0;

	if( mapname[0] != 0 && ud.m_level_number == 7 && ud.m_volume_number == 0 )
		sprintf(text, "%s", mapname);
    else
		//sprintf(tempbuf, "%s", level_names[ud.volume_number*11 + ud.level_number]);
		sprintf(text, "e%dl%d", ud.volume_number+1, ud.level_number+1);


    i = totalclock;

    if (i != frameval[framecnt])
    {
		currentFps = (TICRATE*AVERAGEFRAMES)/(i-frameval[framecnt]);
		fpsAvg = ((fpsAvg<<3)+(fpsAvg<<2) + (currentFps<<2))>>4;

		frameval[framecnt] = i;
    }

    framecnt = ((framecnt+1)&(AVERAGEFRAMES-1));

	// refresh screen and update visible FPS. This is to allow a refresh
	// of the screen when the screensize > 4 w/o compromising the FPS.
	if(ud.screen_size>8)
		if ((totalclock%64) < 32)
		{
			if(toggle)
			{
				vscrn();
				savedFps = fpsAvg;
			}
			toggle = false;
		}
		else
		{
			toggle = true;
		}
	else
		savedFps = fpsAvg;

	sprintf(fps," %d", savedFps);
	strcat(text, fps);

	minitext(offx,offy,text,color,2+8+16+128);
}

void coords(short snum)
{
    short x = 200, y = 0;
    char text[512];
    // x = 250 is too much on the right and
    // will make the text going out of the screen
    // if screen <= (320x200)
    // This will also *write beyond the video
    // buffer limit* and will crash the game.

    if(ud.coop != 1)
    {
        if(ud.multimode > 1 && ud.multimode < 5)
            y = 8;
        else if(ud.multimode > 4)
            y = 16;
    }

    sprintf(text,"X= %d",ps[snum].posx);
    printext256(x,y,31,-1,text,1);
    sprintf(text,"Y= %d",ps[snum].posy);
    printext256(x,y+7L,31,-1,text,1);
    sprintf(text,"Z= %d",ps[snum].posz);
    printext256(x,y+14L,31,-1,text,1);
    sprintf(text,"A= %d",ps[snum].ang);
    printext256(x,y+21L,31,-1,text,1);
    sprintf(text,"ZV= %d",ps[snum].poszv);
    printext256(x,y+28L,31,-1,text,1);
    sprintf(text,"OG= %d",ps[snum].on_ground);
    printext256(x,y+35L,31,-1,text,1);
    sprintf(text,"AM= %d",ps[snum].ammo_amount[GROW_WEAPON]);
    printext256(x,y+43L,31,-1,text,1);
    sprintf(text,"LFW= %d",ps[snum].last_full_weapon);
    printext256(x,y+50L,31,-1,text,1);
    sprintf(text,"SECTL= %d",sector[ps[snum].cursectnum].lotag);
    printext256(x,y+57L,31,-1,text,1);
    sprintf(text,"SEED= %d",randomseed);
    printext256(x,y+64L,31,-1,text,1);
    sprintf(text,"THOLD= %d",ps[snum].transporter_hold);
    printext256(x,y+64L+7,31,-1,text,1);
}

void operatefta(void)
{
     int32_t i, j, k;

     if(ud.screen_size > 0) j = 200-45; else j = 200-8;
     quotebot = min(quotebot,j);
     quotebotgoal = min(quotebotgoal,j);
     if(ps[myconnectindex].gm&MODE_TYPE) j -= 8;
     quotebotgoal = j; j = quotebot;
     for(i=0;i<MAXUSERQUOTES;i++)
     {
         k = user_quote_time[i]; if (k <= 0) break;

         if (k > 4)
              gametext(320>>1,j,user_quote[i],0,2+8+16);
         else if (k > 2) gametext(320>>1,j,user_quote[i],0,2+8+16+1);
             else gametext(320>>1,j,user_quote[i],0,2+8+16+1+32);
         j -= 8;
     }

     if (ps[screenpeek].fta <= 1) return;

     if (ud.coop != 1 && ud.screen_size > 0 && ud.multimode > 1)
     {
         j = 0; k = 8;
         for(i=connecthead;i>=0;i=connectpoint2[i])
             if (i > j) j = i;

         if (j >= 4 && j <= 8) k += 8;
         else if (j > 8 && j <= 12) k += 16;
         else if (j > 12) k += 24;
     }
     else k = 0;

     if (ps[screenpeek].ftq == 115 || ps[screenpeek].ftq == 116)
     {
         k = quotebot;
         for(i=0;i<MAXUSERQUOTES;i++)
         {
             if (user_quote_time[i] <= 0) break;
             k -= 8;
         }
         k -= 4;
     }

     j = ps[screenpeek].fta;
     if (j > 4)
          gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16);
     else
         if (j > 2) gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16+1);
     else
         gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16+1+32);
}

void FTA(short q,struct player_struct *p, int mode)
{
    if( ud.fta_on == 1 || mode)
    {
        if( p->fta > 0 && q != 115 && q != 116 )
            if( p->ftq == 115 || p->ftq == 116 ) return;

        p->fta = 100;

        if( p->ftq != q || q == 26 )
        // || q == 26 || q == 115 || q ==116 || q == 117 || q == 122 )
        {
            p->ftq = q;
            pub = NUMPAGES;
            pus = NUMPAGES;
        }
    }
}

void showtwoscreens(void)
{
    short i;

	if(VOLUMEONE)
	{
	    setview(0,0,xdim-1,ydim-1);
	    flushperms();
	    ps[myconnectindex].palette = palette;
	    for(i=0;i<64;i+=7) palto(0,0,0,i);
	    KB_FlushKeyboardQueue();

	    rotatesprite(0,0,65536L,0,3291,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);

	    nextpage();
		for(i=63;i>0;i-=7) palto(0,0,0,i);

	    while( !KB_KeyWaiting() ); // getpackets(); // Net already off. Trying to get packets here makes sporadic crash..

	    for(i=0;i<64;i+=7)
			palto(0,0,0,i);

	    KB_FlushKeyboardQueue();

	    rotatesprite(0,0,65536L,0,3290,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
	    nextpage();

		for(i=63;i>0;i-=7)
			palto(0,0,0,i);

	    while( !KB_KeyWaiting() ); // getpackets();
	}
	else if(PLUTOPAK)
	{
		setview(0,0,xdim-1,ydim-1);
	    flushperms();
	    ps[myconnectindex].palette = palette;

	    for(i=0;i<64;i+=7)
			palto(0,0,0,i);

	    KB_FlushKeyboardQueue();

	    clearview(0L);
	    rotatesprite(0,0,65536L,0,TENSCREEN,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
	    nextpage();

		for(i=63;i>0;i-=7)
			palto(0,0,0,i);

	    totalclock = 0;

	    while( !KB_KeyWaiting() );

	}
}


void gameexit(char  *msg)
{
    char  t[256];

    strncpy(t,msg,256); t[255] = 0;

    if(*t != 0) ps[myconnectindex].palette = (uint8_t  *) &palette[0];

    if(numplayers > 1)
        allowtimetocorrecterrorswhenquitting();

    if(ud.recstat == 1)
        closedemowrite();

    if(frecfilep != NULL)
    {
        fclose(frecfilep);
        frecfilep = NULL;
    }

    if(qe || cp)
        goto GOTOHERE;

	// FIX_00089: scoreboard not shown for last player who quits a DM. Only 19.7 affected. (Sarah)
    if( ud.m_recstat != 2 && ud.last_level >= 0 && playerswhenstarted > 1 && ud.coop != 1 && *t == ' ')
    {
        dobonus(1);
// CTW - MODIFICATION
//      setgamemode();
// FIX_00028: No need to call the videodriver on gameexit()
//		setgamemode(ScreenMode,ScreenWidth,ScreenHeight);
// CTW END - MODIFICATION
    }

    if(playerswhenstarted > 1)
        uninitmultiplayers();  /* deinits network transport. */

// CTW - MODIFICATION
/*  if( *t != 0 && *(t+1) != 'V' && *(t+1) != 'Y' && playonten == 0 )
        showtwoscreens();*/
    if( *t != 0 && *(t+1) != 'V' && *(t+1) != 'Y' && true)
		if(ud.showcinematics) // FIX_00029: toggle cinematics on / off
			showtwoscreens();
// CTW END - MODIFICATION

    GOTOHERE:

    Shutdown();

    if(*t != 0)
    {
        setvmode(0x3);

// CTW - MODIFICATION
/*      if(playonten == 0)
        {
            if(*t == ' ' && *(t+1) == 0) *t = 0;
            printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
            printf("%s%s","\n",t);
        }*/
        if(true)
        {
            if(*t == ' ' && *(t+1) == 0) *t = 0;
            #if PLATFORM_DOS   // Is there a good reason for this? --ryan.
            printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
            #else
            printf("\n%s\n",t);
            #endif
        }
// CTW END - MODIFICATION
    }

    uninitgroupfile();

    unlink("duke3d.tmp");

    Error(EXIT_SUCCESS, "");

}




short inputloc = 0;
short strget(short x,short y,char  *t,short dalen,short c)
{
    short ch,sc;

    while(KB_KeyWaiting())
    {
        sc = 0;
        ch = KB_Getch();

        if (ch == 0)
        {

            sc = KB_Getch();
            if( sc == 104) return(1);

            continue;
        }
        else
        {
            if(ch == 8) // asc_BackSpace
            {
                if( inputloc > 0 )
                {
                    inputloc--;
                    *(t+inputloc) = 0;
                }
            }
            else
            {
                if(ch == asc_Enter || sc == 104)
                {
                    KB_ClearKeyDown(sc_Enter);
                    KB_ClearKeyDown(sc_kpad_Enter);
                    return (1);
                }
                else if(ch == asc_Escape)
                {
                    KB_ClearKeyDown(sc_Escape);
                    return (-1);
                }
                else if ( ch >= 32 && inputloc < dalen && ch < 127)
                {
                    ch = toupper(ch);
                    *(t+inputloc) = ch;
                    *(t+inputloc+1) = 0;
                    inputloc++;
                }
            }
        }
    }

    if( c == 999 ) return(0);
    if( c == 998 )
    {
        char  b[41],ii;
        for(ii=0;ii<inputloc;ii++)
            b[ii] = '*';
        b[ii] = 0;
        x = gametext(x,y,b,c,2+8+16);
    }
    else x = gametext(x,y,t,c,2+8+16);
    c = 4-(sintable[(totalclock<<4)&2047]>>11);
    rotatesprite((x+8)<<16,(y+4)<<16,32768L,0,SPINNINGNUKEICON+((totalclock>>3)%7),c,0,2+8,0,0,xdim-1,ydim-1);

    return (0);
}

void typemode(void)
{
    short ch, hitstate, i, j;
    char text[512];

     if( ps[myconnectindex].gm&MODE_SENDTOWHOM )
     {
          if(sendmessagecommand != -1 || ud.multimode < 3 || movesperpacket == 4)
          {
                text[0] = 4; // message command
                text[1] = 0;
                recbuf[0]  = 0;

                if(ud.multimode < 3)
                     sendmessagecommand = 2;

                strcat(recbuf,ud.user_name[myconnectindex]);
                strcat(recbuf,": ");
                strcat(recbuf,typebuf);
                j = strlen(recbuf);
                recbuf[j] = 0;
                strcat(text+1,recbuf);

                if(sendmessagecommand >= ud.multimode || movesperpacket == 4)
                {
                     for(ch=connecthead;ch >= 0;ch=connectpoint2[ch])
                          if (ch != myconnectindex)
                                sendpacket(ch,(uint8_t*)tempbuf,j+1);

                     adduserquote(recbuf);
                     quotebot += 8;
                     quotebotgoal = quotebot;
                }
                else if(sendmessagecommand >= 0)
                     sendpacket(sendmessagecommand,(uint8_t*)tempbuf,j+1);

                sendmessagecommand = -1;
                ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
          }
          else if(sendmessagecommand == -1)
          {
                j = 50;
                gametext(320>>1,j,"SEND MESSAGE TO...",0,2+8+16); j += 8;
                for(i=connecthead;i>=0;i=connectpoint2[i])
//                for(i=0;i<ud.multimode;i++)
                {
                     if (i == myconnectindex)
                     {
                         minitextshade((320>>1)-40+1,j+1,"A/ENTER - ALL",26,0,2+8+16);
                         minitext((320>>1)-40,j,"A/ENTER - ALL",0,2+8+16); j += 7;
                     }
                     else
                     {
                         sprintf(buf,"      %d - %s",i+1,ud.user_name[i]);
                         minitextshade((320>>1)-40-6+1,j+1,buf,26,0,2+8+16);
                         minitext((320>>1)-40-6,j,buf,0,2+8+16); j += 7;
                     }
                }
                minitextshade((320>>1)-40-4+1,j+1,"    ESC - Abort",26,0,2+8+16);
                minitext((320>>1)-40-4,j,"    ESC - Abort",0,2+8+16); j += 7;

                //sprintf(buf,"PRESS 1-%ld FOR INDIVIDUAL PLAYER.",ud.multimode);
                //gametext(320>>1,j,buf,0,2+8+16); j += 8;
                //gametext(320>>1,j,"'A' OR 'ENTER' FOR ALL PLAYERS",0,2+8+16); j += 8;
                //gametext(320>>1,j,"ESC ABORTS",0,2+8+16); j += 8;

                if (ud.screen_size > 0) j = 200-45; else j = 200-8;
                gametext(320>>1,j,typebuf,0,2+8+16);

                if( KB_KeyWaiting() )
                {
                     i = KB_Getch();

                     if(i == 'A' || i == 'a' || i == 13)
                          sendmessagecommand = ud.multimode;
                     else if(i >= '1' || i <= (ud.multimode + '1') )
                          sendmessagecommand = i - '1';
                     else
                     {
                        sendmessagecommand = ud.multimode;
                          if(i == 27)
                          {
                              ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
                              sendmessagecommand = -1;
                          }
                          else
                          typebuf[0] = 0;
                     }

                     KB_ClearKeyDown(sc_1);
                     KB_ClearKeyDown(sc_2);
                     KB_ClearKeyDown(sc_3);
                     KB_ClearKeyDown(sc_4);
                     KB_ClearKeyDown(sc_5);
                     KB_ClearKeyDown(sc_6);
                     KB_ClearKeyDown(sc_7);
                     KB_ClearKeyDown(sc_8);
                     KB_ClearKeyDown(sc_A);
                     KB_ClearKeyDown(sc_Escape);
                     KB_ClearKeyDown(sc_Enter);
                }
          }
     }
     else
     {
          if(ud.screen_size > 0) j = 200-45; else j = 200-8;
          hitstate = strget(320>>1,j,typebuf,30,1);

          if(hitstate == 1)
          {
                KB_ClearKeyDown(sc_Enter);
                ps[myconnectindex].gm |= MODE_SENDTOWHOM;
          }
          else if(hitstate == -1)
                ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
          else pub = NUMPAGES;
     }
}

void moveclouds(void)
{
    if( totalclock > cloudtotalclock || totalclock < (cloudtotalclock-7))
    {
        short i;

        cloudtotalclock = totalclock+6;

        for(i=0;i<numclouds;i++)
        {
            cloudx[i] += (sintable[(ps[screenpeek].ang+512)&2047]>>9);
            cloudy[i] += (sintable[ps[screenpeek].ang&2047]>>9);

            sector[clouds[i]].ceilingxpanning = cloudx[i]>>6;
            sector[clouds[i]].ceilingypanning = cloudy[i]>>6;
        }
    }
}


void displayrest(int32_t smoothratio)
{
    int32_t a, i, j;

    struct player_struct *pp;
    walltype *wal;
    int32_t cposx,cposy,cang;

    pp = &ps[screenpeek];


    if(ud.show_help)
    {
        switch(ud.show_help)
        {
            case 1:
                rotatesprite(0,0,65536L,0,TEXTSTORY,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                break;
            case 2:
                rotatesprite(0,0,65536L,0,F1HELP,0,0,10+16+64, 0,0,xdim-1,ydim-1);
                break;
        }

        if ( KB_KeyPressed(sc_Escape ) )
        {
            KB_ClearKeyDown(sc_Escape);
            ud.show_help = 0;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 1;
                totalclock = ototalclock;
            }
            vscrn();
        }
        return;
    }

    i = pp->cursectnum;

    show2dsector[i>>3] |= (1<<(i&7));
    wal = &wall[sector[i].wallptr];
    for(j=sector[i].wallnum;j>0;j--,wal++)
    {
        i = wal->nextsector;
        if (i < 0) continue;
        if (wal->cstat&0x0071) continue;
        if (wall[wal->nextwall].cstat&0x0071) continue;
        if (sector[i].lotag == 32767) continue;
        if (sector[i].ceilingz >= sector[i].floorz) continue;
        show2dsector[i>>3] |= (1<<(i&7));
    }

    if(ud.camerasprite == -1)
    {
        if( ud.overhead_on != 2 )
        {
            if(pp->newowner >= 0)
                cameratext(pp->newowner);
            else
            {
                displayweapon(screenpeek);
                if(pp->over_shoulder_on == 0 )
                    displaymasks(screenpeek);
            }
            moveclouds();
        }

        if( ud.overhead_on > 0 )
        {
                smoothratio = min(max(smoothratio,0),65536);
                dointerpolations(smoothratio);
                if( ud.scrollmode == 0 )
                {
                     if(pp->newowner == -1)
                     {
                         if (screenpeek == myconnectindex && numplayers > 1)
                         {
                             cposx = omyx+mulscale16((int32_t)(myx-omyx),smoothratio);
                             cposy = omyy+mulscale16((int32_t)(myy-omyy),smoothratio);
                             cang = omyang+mulscale16((int32_t)(((myang+1024-omyang)&2047)-1024),smoothratio);
                         }
                         else
                         {
                              cposx = pp->oposx+mulscale16((int32_t)(pp->posx-pp->oposx),smoothratio);
                              cposy = pp->oposy+mulscale16((int32_t)(pp->posy-pp->oposy),smoothratio);
                              cang = pp->oang+mulscale16((int32_t)(((pp->ang+1024-pp->oang)&2047)-1024),smoothratio);
                         }
                    }
                    else
                    {
                        cposx = pp->oposx;
                        cposy = pp->oposy;
                        cang = pp->oang;
                    }
                }
                else
                {

                     ud.fola += ud.folavel>>3;
                     ud.folx += (ud.folfvel*sintable[(512+2048-ud.fola)&2047])>>14;
                     ud.foly += (ud.folfvel*sintable[(512+1024-512-ud.fola)&2047])>>14;

                     cposx = ud.folx;
                     cposy = ud.foly;
                     cang = ud.fola;
                }

                if(ud.overhead_on == 2)
                {
                    clearview(0L);
                    drawmapview(cposx,cposy,pp->zoom,cang);
                }
                drawoverheadmap( cposx,cposy,pp->zoom,cang);

                restoreinterpolations();

                if(ud.overhead_on == 2)
                {
                    if(ud.screen_size > 0) a = 147;
                    else a = 182;

                    minitext(1,a+6,volume_names[ud.volume_number],0,2+8+16);
                    minitext(1,a+12,level_names[ud.volume_number*11 + ud.level_number],0,2+8+16);
                }
        }
    }

    coolgaugetext(screenpeek);
    operatefta();

    if( KB_KeyPressed(sc_Escape) && ud.overhead_on == 0
        && ud.show_help == 0
        && ps[myconnectindex].newowner == -1)
    {
			if( (ps[myconnectindex].gm&MODE_MENU) != MODE_MENU &&
            ps[myconnectindex].newowner == -1 &&
            (ps[myconnectindex].gm&MODE_TYPE) != MODE_TYPE)
        {
            KB_ClearKeyDown(sc_Escape);
            FX_StopAllSounds();
            clearsoundlocks();

            intomenusounds();

            ps[myconnectindex].gm |= MODE_MENU;

            if(ud.multimode < 2 && ud.recstat != 2) ready2send = 0;

            if(ps[myconnectindex].gm&MODE_GAME) cmenu(50);
            else cmenu(0);
            screenpeek = myconnectindex;
        }
    }

    if(ps[myconnectindex].newowner == -1 && ud.overhead_on == 0 && ud.crosshair && ud.camerasprite == -1)
        rotatesprite((160L-(ps[myconnectindex].look_ang>>1))<<16,100L<<16,65536L,0,CROSSHAIR,0,0,2+1,windowx1,windowy1,windowx2,windowy2);

    if(ps[myconnectindex].gm&MODE_TYPE)
        typemode();
    else
    {
        CONSOLE_HandleInput();
        if( !CONSOLE_IsActive())
        {
            menus();
        }
        CONSOLE_Render();
    }

    if( ud.pause_on==1 && (ps[myconnectindex].gm&MODE_MENU) == 0 )
	{
		if (!CONSOLE_IsActive()) //Addfaz Console Pause Game line addition
		{
			menutext(160,100,0,0,"GAME PAUSED");
		}
		else
		{
            menutext(160,120,0,0,"GAME PAUSED");
		}
	}

    if(ud.coords)
        coords(screenpeek);

	// FIX_00085: Optimized Video driver. FPS increases by +20%.
    if( pp->pals_time > 0 && pp->loogcnt == 0)
    {
        palto( pp->pals[0],
               pp->pals[1],
               pp->pals[2],
               pp->pals_time|128);

        restorepalette = 1;
    }
    else if( restorepalette )
    {
        setbrightness(ud.brightness>>2,&pp->palette[0]);
        restorepalette = 0;
    }
    else if(pp->loogcnt > 0) palto(0,64,0,(pp->loogcnt>>1)+128);

}


void updatesectorz(int32_t x, int32_t y, int32_t z, short *sectnum)
{
    walltype *wal;
    int32_t i, j, cz, fz;

    getzsofslope(*sectnum,x,y,&cz,&fz);
    if ((z >= cz) && (z <= fz))
        if (inside(x,y,*sectnum) != 0) return;

    if ((*sectnum >= 0) && (*sectnum < numsectors))
    {
        wal = &wall[sector[*sectnum].wallptr];
        j = sector[*sectnum].wallnum;
        do
        {
            i = wal->nextsector;
            if (i >= 0)
            {
                getzsofslope(i,x,y,&cz,&fz);
                if ((z >= cz) && (z <= fz))
                    if (inside(x,y,(short)i) == 1)
                        { *sectnum = i; return; }
            }
            wal++; j--;
        } while (j != 0);
    }

    for(i=numsectors-1;i>=0;i--)
    {
        getzsofslope(i,x,y,&cz,&fz);
        if ((z >= cz) && (z <= fz))
            if (inside(x,y,(short)i) == 1)
                { *sectnum = i; return; }
    }

    *sectnum = -1;
}

IRAM_ATTR void view(struct player_struct *pp, int32_t *vx, int32_t *vy,int32_t *vz,short *vsectnum, short ang, short horiz)
{
     spritetype *sp;
     int32_t i, nx, ny, nz, hx, hy, hitx, hity, hitz;
     short bakcstat, hitsect, hitwall, hitsprite, daang;

     nx = (sintable[(ang+1536)&2047]>>4);
     ny = (sintable[(ang+1024)&2047]>>4);
     nz = (horiz-100)*128;

     sp = &sprite[pp->i];

     bakcstat = sp->cstat;
     sp->cstat &= (short)~0x101;

     updatesectorz(*vx,*vy,*vz,vsectnum);
     hitscan(*vx,*vy,*vz,*vsectnum,nx,ny,nz,&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

     if(*vsectnum < 0)
     {
        sp->cstat = bakcstat;
        return;
     }

     hx = hitx-(*vx); hy = hity-(*vy);
     if (klabs(nx)+klabs(ny) > klabs(hx)+klabs(hy))
     {
         *vsectnum = hitsect;
         if (hitwall >= 0)
         {
             daang = getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,
                                    wall[wall[hitwall].point2].y-wall[hitwall].y);

             i = nx*sintable[daang]+ny*sintable[(daang+1536)&2047];
             if (klabs(nx) > klabs(ny)) hx -= mulscale28(nx,i);
                                          else hy -= mulscale28(ny,i);
         }
         else if (hitsprite < 0)
         {
             if (klabs(nx) > klabs(ny)) hx -= (nx>>5);
                                          else hy -= (ny>>5);
         }
         if (klabs(nx) > klabs(ny)) i = divscale16(hx,nx);
                                      else i = divscale16(hy,ny);
         if (i < cameradist) cameradist = i;
     }
     *vx = (*vx)+mulscale16(nx,cameradist);
     *vy = (*vy)+mulscale16(ny,cameradist);
     *vz = (*vz)+mulscale16(nz,cameradist);

     cameradist = min(cameradist+((totalclock-cameraclock)<<10),65536);
     cameraclock = totalclock;

     updatesectorz(*vx,*vy,*vz,vsectnum);

     sp->cstat = bakcstat;
}

    //REPLACE FULLY
void drawbackground(void)
{
     short dapicnum;
     int32_t x,y,x1,y1,x2,y2;

     flushperms();

     switch(ud.m_volume_number)
     {
          default:dapicnum = BIGHOLE;break;
          case 1:dapicnum = BIGHOLE;break;
          case 2:dapicnum = BIGHOLE;break;
     }

     y1 = 0; y2 = ydim;
     if( ready2send || ud.recstat == 2 )
     {
        if(ud.coop != 1)
        {
            if (ud.multimode > 1) y1 += scale(ydim,8,200);
            if (ud.multimode > 4) y1 += scale(ydim,8,200);
        }
        if (ud.screen_size >= 8) y2 = scale(ydim,200-34,200);
     }

     for(y=y1;y<y2;y+=128)
          for(x=0;x<xdim;x+=128)
                rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,0,y1,xdim-1,y2-1);

	 // FIX_00081: Screen border in menu
     if(ud.screen_size > 8 && (ps[myconnectindex].gm & MODE_GAME || ud.recstat == 2 )) // ud.recstat == 2 => playing demo
     {
          y = 0;
          if(ud.coop != 1)
          {
             if (ud.multimode > 1) y += 8;
             if (ud.multimode > 4) y += 8;
          }

          x1 = max(windowx1-4,0);
          y1 = max(windowy1-4,y);
          x2 = min(windowx2+4,xdim-1);
          y2 = min(windowy2+4,scale(ydim,200-34,200)-1);

          for(y=y1+4;y<y2-4;y+=64)
          {
                rotatesprite(x1<<16,y<<16,65536L,0,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
                rotatesprite((x2+1)<<16,(y+64)<<16,65536L,1024,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
          }

          for(x=x1+4;x<x2-4;x+=64)
          {
                rotatesprite((x+64)<<16,y1<<16,65536L,512,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
                rotatesprite(x<<16,(y2+1)<<16,65536L,1536,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
          }

          rotatesprite(x1<<16,y1<<16,65536L,0,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite((x2+1)<<16,y1<<16,65536L,512,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite((x2+1)<<16,(y2+1)<<16,65536L,1024,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
          rotatesprite(x1<<16,(y2+1)<<16,65536L,1536,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
     }
}


// Floor Over Floor

// If standing in sector with SE42
// then draw viewing to SE41 and raise all =hi SE43 cielings.

// If standing in sector with SE43
// then draw viewing to SE40 and lower all =hi SE42 floors.

// If standing in sector with SE44
// then draw viewing to SE40.

// If standing in sector with SE45
// then draw viewing to SE41.

#define FOFTILE 13
#define FOFTILEX 32
#define FOFTILEY 32
EXT_RAM_ATTR int32_t tempsectorz[MAXSECTORS];
EXT_RAM_ATTR int32_t tempsectorpicnum[MAXSECTORS];
//short tempcursectnum;

static void SE40_Draw(int spnum,int32_t x,int32_t y,int32_t z,short a,short h,int32_t smoothratio)
{
 int i=0,j=0,k=0;
 int floor1=0,floor2=0,ok=0,fofmode=0;
 int32_t offx,offy;

 if(sprite[spnum].ang!=512) return;

 i = FOFTILE;    //Effect TILE
 if (!(gotpic[i>>3]&(1<<(i&7)))) return;
 gotpic[i>>3] &= ~(1<<(i&7));

 floor1=spnum;

 if(sprite[spnum].lotag==42) fofmode=40;
 if(sprite[spnum].lotag==43) fofmode=41;
 if(sprite[spnum].lotag==44) fofmode=40;
 if(sprite[spnum].lotag==45) fofmode=41;

// fofmode=sprite[spnum].lotag-2;

// sectnum=sprite[j].sectnum;
// sectnum=cursectnum;
 ok++;

/*  recursive?
 for(j=0;j<MAXSPRITES;j++)
 {
  if(
     sprite[j].sectnum==sectnum &&
     sprite[j].picnum==1 &&
     sprite[j].lotag==110
    ) { DrawFloorOverFloor(j); break;}
 }
*/

// if(ok==0) { Message("no fof",RED); return; }

 for(j=0;j<MAXSPRITES;j++)
 {
  if(
     sprite[j].picnum==1 &&
     sprite[j].lotag==fofmode &&
     sprite[j].hitag==sprite[floor1].hitag
    ) { floor1=j; fofmode=sprite[j].lotag; ok++; break;}
 }
// if(ok==1) { Message("no floor1",RED); return; }

 if(fofmode==40) k=41; else k=40;

 for(j=0;j<MAXSPRITES;j++)
 {
	  if(
		 sprite[j].picnum==1 &&
		 sprite[j].lotag==k &&
		 sprite[j].hitag==sprite[floor1].hitag
		)
	  {
		  floor2=j;
		  ok++;
		  break;
	  }
 }

// if(ok==2) { Message("no floor2",RED); return; }

 for(j=0;j<MAXSPRITES;j++)  // raise ceiling or floor
 {
  if(sprite[j].picnum==1 &&
     sprite[j].lotag==k+2 &&
     sprite[j].hitag==sprite[floor1].hitag
    )
    {
     if(k==40)
     {tempsectorz[sprite[j].sectnum]=sector[sprite[j].sectnum].floorz;
      sector[sprite[j].sectnum].floorz+=(((z-sector[sprite[j].sectnum].floorz)/32768)+1)*32768;
      tempsectorpicnum[sprite[j].sectnum]=sector[sprite[j].sectnum].floorpicnum;
      sector[sprite[j].sectnum].floorpicnum=13;
     }
     if(k==41)
     {tempsectorz[sprite[j].sectnum]=sector[sprite[j].sectnum].ceilingz;
      sector[sprite[j].sectnum].ceilingz+=(((z-sector[sprite[j].sectnum].ceilingz)/32768)-1)*32768;
      tempsectorpicnum[sprite[j].sectnum]=sector[sprite[j].sectnum].ceilingpicnum;
      sector[sprite[j].sectnum].ceilingpicnum=13;
     }
    }
 }

 i=floor1;
 offx=x-sprite[i].x;
 offy=y-sprite[i].y;
 i=floor2;
 drawrooms(offx+sprite[i].x,offy+sprite[i].y,z,a,h,sprite[i].sectnum);
 animatesprites(x,y,a,smoothratio);
 drawmasks();

 for(j=0;j<MAXSPRITES;j++)  // restore ceiling or floor
 {
  if(sprite[j].picnum==1 &&
     sprite[j].lotag==k+2 &&
     sprite[j].hitag==sprite[floor1].hitag
    )
    {
     if(k==40)
     {sector[sprite[j].sectnum].floorz=tempsectorz[sprite[j].sectnum];
      sector[sprite[j].sectnum].floorpicnum=tempsectorpicnum[sprite[j].sectnum];
     }
     if(k==41)
     {sector[sprite[j].sectnum].ceilingz=tempsectorz[sprite[j].sectnum];
      sector[sprite[j].sectnum].ceilingpicnum=tempsectorpicnum[sprite[j].sectnum];
     }
    }// end if
 }// end for

} // end SE40




static void se40code(int32_t x,int32_t y,int32_t z,int32_t a,int32_t h, int32_t smoothratio)
{
    int i;

    i = headspritestat[15];
    while(i >= 0)
    {
        switch(sprite[i].lotag)
        {
//            case 40:
//            case 41:
//                SE40_Draw(i,x,y,a,smoothratio);
//                break;
            case 42:
            case 43:
            case 44:
            case 45:
                if(ps[screenpeek].cursectnum == sprite[i].sectnum)
                    SE40_Draw(i,x,y,z,a,h,smoothratio);
                break;
        }
        i = nextspritestat[i];
    }
}

static int32_t oyrepeat=-1;

IRAM_ATTR void displayrooms(short snum,int32_t smoothratio)
{
    int32_t cposx,cposy,cposz,dst,j,fz,cz;
    short sect, cang, k, choriz;
    struct player_struct *p;
    int32_t tposx,tposy,i;
    short tang;

    p = &ps[snum];

    if(pub > 0)
    {
        if(ud.screen_size > 8) drawbackground();
        pub = 0;
    }

    if( ud.overhead_on == 2 || ud.show_help || p->cursectnum == -1)
        return;

    smoothratio = min(max(smoothratio,0),65536);

    visibility = p->visibility;

    if(ud.pause_on || ps[snum].on_crane > -1) smoothratio = 65536;

    sect = p->cursectnum;
    if(sect < 0 || sect >= MAXSECTORS) return;

    dointerpolations(smoothratio);

    animatecamsprite();

    if(ud.camerasprite >= 0)
    {
        spritetype *s;

        s = &sprite[ud.camerasprite];

        if(s->yvel < 0) s->yvel = -100;
        else if(s->yvel > 199) s->yvel = 300;

        cang = hittype[ud.camerasprite].tempang+mulscale16((int32_t)(((s->ang+1024-hittype[ud.camerasprite].tempang)&2047)-1024),smoothratio);

        se40code(s->x,s->y,s->z,cang,s->yvel,smoothratio);

        drawrooms(s->x,s->y,s->z-(4<<8),cang,s->yvel,s->sectnum);
        animatesprites(s->x,s->y,cang,smoothratio);
        drawmasks();
    }
    else
    {
        i = divscale22(1,sprite[p->i].yrepeat+28);
        if (i != oyrepeat)
        {
            oyrepeat = i;
			//printf("1: %d %d\n", oyrepeat,yxaspect);
            setaspect(oyrepeat,yxaspect);
			//printf("2: %d %d\n", oyrepeat,yxaspect);
        }

        if(screencapt)
        {
            tiles[MAXTILES-1].lock = 254;
            if (tiles[MAXTILES-1].data == NULL)
                allocache(&tiles[MAXTILES-1].data,100*160,&tiles[MAXTILES-1].lock);

            setviewtotile(MAXTILES-1,100L,160L);
        }
        else if( ( ud.screen_tilting && p->rotscrnang ) || ud.detail==0 )
        {
                if (ud.screen_tilting) tang = p->rotscrnang; else tang = 0;

                tiles[MAXTILES-2].lock = 255;
                if (tiles[MAXTILES-2].data == NULL)
                    allocache(&tiles[MAXTILES-2].data,320L*320L,&tiles[MAXTILES-2].lock);
                if ((tang&1023) == 0)
                    setviewtotile(MAXTILES-2,200L>>(1-ud.detail),320L>>(1-ud.detail));
                else
                    setviewtotile(MAXTILES-2,320L>>(1-ud.detail),320L>>(1-ud.detail));
                if ((tang&1023) == 512)
                {     //Block off unscreen section of 90ø tilted screen
                    j = ((320-60)>>(1-ud.detail));
                    for(i=(60>>(1-ud.detail))-1;i>=0;i--)
                    {
                        startumost[i] = 1; startumost[i+j] = 1;
                        startdmost[i] = 0; startdmost[i+j] = 0;
                    }
                }

                i = (tang&511); if (i > 256) i = 512-i;
                i = sintable[i+512]*8 + sintable[i]*5L;
                setaspect(i>>1,yxaspect);
          }

          if ( (snum == myconnectindex) && (numplayers > 1) )
                  {
                                cposx = omyx+mulscale16((int32_t)(myx-omyx),smoothratio);
                                cposy = omyy+mulscale16((int32_t)(myy-omyy),smoothratio);
                                cposz = omyz+mulscale16((int32_t)(myz-omyz),smoothratio);
                                cang = omyang+mulscale16((int32_t)(((myang+1024-omyang)&2047)-1024),smoothratio);
                                choriz = omyhoriz+omyhorizoff+mulscale16((int32_t)(myhoriz+myhorizoff-omyhoriz-omyhorizoff),smoothratio);
                                sect = mycursectnum;
                  }
                  else
                  {
                                cposx = p->oposx+mulscale16((int32_t)(p->posx-p->oposx),smoothratio);
                                cposy = p->oposy+mulscale16((int32_t)(p->posy-p->oposy),smoothratio);
                                cposz = p->oposz+mulscale16((int32_t)(p->posz-p->oposz),smoothratio);
                                cang = p->oang+mulscale16((int32_t)(((p->ang+1024-p->oang)&2047)-1024),smoothratio);
                                choriz = p->ohoriz+p->ohorizoff+mulscale16((int32_t)(p->horiz+p->horizoff-p->ohoriz-p->ohorizoff),smoothratio);
                  }
                  cang += p->look_ang;

                  if (p->newowner >= 0)
                  {
                                cang = p->ang+p->look_ang;
                                choriz = p->horiz+p->horizoff;
                                cposx = p->posx;
                                cposy = p->posy;
                                cposz = p->posz;
                                sect = sprite[p->newowner].sectnum;
                                smoothratio = 65536L;
                  }

                  else if( p->over_shoulder_on == 0 )
                                cposz += p->opyoff+mulscale16((int32_t)(p->pyoff-p->opyoff),smoothratio);
                  else view(p,&cposx,&cposy,&cposz,&sect,cang,choriz);

        cz = hittype[p->i].ceilingz;
        fz = hittype[p->i].floorz;

        if(earthquaketime > 0 && p->on_ground == 1)
        {
            cposz += 256-(((earthquaketime)&1)<<9);
            cang += (2-((earthquaketime)&2))<<2;
        }

        if(sprite[p->i].pal == 1) cposz -= (18<<8);

        if(p->newowner >= 0)
            choriz = 100+sprite[p->newowner].shade;
        else if(p->spritebridge == 0)
        {
            if( cposz < ( p->truecz + (4<<8) ) ) cposz = cz + (4<<8);
            else if( cposz > ( p->truefz - (4<<8) ) ) cposz = fz - (4<<8);
        }

        if (sect >= 0)
        {
            getzsofslope(sect,cposx,cposy,&cz,&fz);
            if (cposz < cz+(4<<8)) cposz = cz+(4<<8);
            if (cposz > fz-(4<<8)) cposz = fz-(4<<8);
        }

        if(choriz > 299) choriz = 299;
        else if(choriz < -99) choriz = -99;

        se40code(cposx,cposy,cposz,cang,choriz,smoothratio);

        if ((gotpic[MIRROR>>3]&(1<<(MIRROR&7))) > 0)
        {
            dst = 0x7fffffff; i = 0;
            for(k=0;k<mirrorcnt;k++)
            {
                j = klabs(wall[mirrorwall[k]].x-cposx);
                j += klabs(wall[mirrorwall[k]].y-cposy);
                if (j < dst) dst = j, i = k;
            }

            if( wall[mirrorwall[i]].overpicnum == MIRROR )
            {
                preparemirror(cposx,cposy,cposz,cang,choriz,mirrorwall[i],mirrorsector[i],&tposx,&tposy,&tang);

                j = visibility;
                visibility = (j>>1) + (j>>2);

                drawrooms(tposx,tposy,cposz,tang,choriz,mirrorsector[i]+MAXSECTORS);

                display_mirror = 1;
                animatesprites(tposx,tposy,tang,smoothratio);
                display_mirror = 0;

                drawmasks();
                completemirror();   //Reverse screen x-wise in this function
                visibility = j;
            }
            gotpic[MIRROR>>3] &= ~(1<<(MIRROR&7));
        }

        drawrooms(cposx,cposy,cposz,cang,choriz,sect);
        animatesprites(cposx,cposy,cang,smoothratio);
        drawmasks();

        if(screencapt == 1)
        {
            setviewback();
            tiles[MAXTILES-1].lock = 1;
            screencapt = 0;
        }
        else if( ( ud.screen_tilting && p->rotscrnang) || ud.detail==0 )
        {
            if (ud.screen_tilting) tang = p->rotscrnang; else tang = 0;
            setviewback();
            tiles[MAXTILES-2].animFlags &= 0xff0000ff;
            i = (tang&511); if (i > 256) i = 512-i;
            i = sintable[i+512]*8 + sintable[i]*5L;
            if ((1-ud.detail) == 0) i >>= 1;
            rotatesprite(160<<16,100<<16,i,tang+512,MAXTILES-2,0,0,4+2+64,windowx1,windowy1,windowx2,windowy2);
            tiles[MAXTILES-2].lock = 199;
        }
    }

    restoreinterpolations();

    if (totalclock < lastvisinc)
    {
        if (klabs(p->visibility-ud.const_visibility) > 8)
            p->visibility += (ud.const_visibility-p->visibility)>>2;
    }
    else p->visibility = ud.const_visibility;
}





short LocateTheLocator(short n,short sn)
{
    short i;

    i = headspritestat[7];
    while(i >= 0)
    {
        if( (sn == -1 || sn == SECT) && n == SLT )
            return i;
        i = nextspritestat[i];
    }
    return -1;
}

short EGS(short whatsect,int32_t s_x,int32_t s_y,int32_t s_z,short s_pn,int8_t s_s,int8_t s_xr,int8_t s_yr,short s_a,short s_ve,int32_t s_zv,short s_ow,int8_t s_ss)
{
    short i;
    spritetype *s;

    i = insertsprite(whatsect,s_ss);

    if( i < 0 )
        gameexit(" Too many sprites spawned. This may happen (for any duke port) if you have hacked the steroids trail in the *.con files. If so, delete your *.con files to use the internal ones and try again.");

    hittype[i].bposx = s_x;
    hittype[i].bposy = s_y;
    hittype[i].bposz = s_z;

    s = &sprite[i];

    s->x = s_x;
    s->y = s_y;
    s->z = s_z;
    s->cstat = 0;
    s->picnum = s_pn;
    s->shade = s_s;
    s->xrepeat = s_xr;
    s->yrepeat = s_yr;
    s->pal = 0;

    s->ang = s_a;
    s->xvel = s_ve;
    s->zvel = s_zv;
    s->owner = s_ow;
    s->xoffset = 0;
    s->yoffset = 0;
    s->yvel = 0;
    s->clipdist = 0;
    s->pal = 0;
    s->lotag = 0;

    hittype[i].picnum = sprite[s_ow].picnum;

    hittype[i].lastvx = 0;
    hittype[i].lastvy = 0;

    hittype[i].timetosleep = 0;
    hittype[i].actorstayput = -1;
    hittype[i].extra = -1;
    hittype[i].owner = s_ow;
    hittype[i].cgg = 0;
    hittype[i].movflag = 0;
    hittype[i].tempang = 0;
    hittype[i].dispicnum = 0;
    hittype[i].floorz = hittype[s_ow].floorz;
    hittype[i].ceilingz = hittype[s_ow].ceilingz;

    T1=T3=T4=T6=0;
    if( actorscrptr[s_pn] )
    {
        s->extra = *actorscrptr[s_pn];
        T5 = *(actorscrptr[s_pn]+1);
        T2 = *(actorscrptr[s_pn]+2);
        s->hitag = *(actorscrptr[s_pn]+3);
    }
    else
    {
        T2=T5=0;
        s->extra = 0;
        s->hitag = 0;
    }

    if (show2dsector[SECT>>3]&(1<<(SECT&7))) show2dsprite[i>>3] |= (1<<(i&7));
    else show2dsprite[i>>3] &= ~(1<<(i&7));
/*
    if(s->sectnum < 0)
    {
        s->xrepeat = s->yrepeat = 0;
        changespritestat(i,5);
    }
*/
    return(i);
}

uint8_t  wallswitchcheck(short i)
{
    switch(PN)
    {
        case HANDPRINTSWITCH:
        case HANDPRINTSWITCH+1:
        case ALIENSWITCH:
        case ALIENSWITCH+1:
        case MULTISWITCH:
        case MULTISWITCH+1:
        case MULTISWITCH+2:
        case MULTISWITCH+3:
        case ACCESSSWITCH:
        case ACCESSSWITCH2:
        case PULLSWITCH:
        case PULLSWITCH+1:
        case HANDSWITCH:
        case HANDSWITCH+1:
        case SLOTDOOR:
        case SLOTDOOR+1:
        case LIGHTSWITCH:
        case LIGHTSWITCH+1:
        case SPACELIGHTSWITCH:
        case SPACELIGHTSWITCH+1:
        case SPACEDOORSWITCH:
        case SPACEDOORSWITCH+1:
        case FRANKENSTINESWITCH:
        case FRANKENSTINESWITCH+1:
        case LIGHTSWITCH2:
        case LIGHTSWITCH2+1:
        case POWERSWITCH1:
        case POWERSWITCH1+1:
        case LOCKSWITCH1:
        case LOCKSWITCH1+1:
        case POWERSWITCH2:
        case POWERSWITCH2+1:
        case DIPSWITCH:
        case DIPSWITCH+1:
        case DIPSWITCH2:
        case DIPSWITCH2+1:
        case TECHSWITCH:
        case TECHSWITCH+1:
        case DIPSWITCH3:
        case DIPSWITCH3+1:
            return 1;
    }
    return 0;
}


int32_t tempwallptr;
short spawn( short j, short pn )
{
    short i, s, startwall, endwall, sect, clostest;
    int32_t x, y, d;
    spritetype *sp;
    char text[512];

    if(j >= 0)
    {
        i = EGS(sprite[j].sectnum,sprite[j].x,sprite[j].y,sprite[j].z
            ,pn,0,0,0,0,0,0,j,0);
        hittype[i].picnum = sprite[j].picnum;
    }
    else
    {
        i = pn;

        hittype[i].picnum = PN;
        hittype[i].timetosleep = 0;
        hittype[i].extra = -1;

        hittype[i].bposx = SX;
        hittype[i].bposy = SY;
        hittype[i].bposz = SZ;

        OW = hittype[i].owner = i;
        hittype[i].cgg = 0;
        hittype[i].movflag = 0;
        hittype[i].tempang = 0;
        hittype[i].dispicnum = 0;
        hittype[i].floorz = sector[SECT].floorz;
        hittype[i].ceilingz = sector[SECT].ceilingz;

        hittype[i].lastvx = 0;
        hittype[i].lastvy = 0;
        hittype[i].actorstayput = -1;

        T1 = T2 = T3 = T4 = T5 = T6 = 0;

        if( PN != SPEAKER && PN != LETTER && PN != DUCK && PN != TARGET && PN != TRIPBOMB && PN != VIEWSCREEN && PN != VIEWSCREEN2 && (CS&48) )
            if( !(PN >= CRACK1 && PN <= CRACK4) )
        {
            if(SS == 127) return i;
            if( wallswitchcheck(i) == 1 && (CS&16) )
            {
                if( PN != ACCESSSWITCH && PN != ACCESSSWITCH2 && sprite[i].pal)
                {
                    if( (ud.multimode < 2) || (ud.multimode > 1 && ud.coop==1) )
                    {
                        sprite[i].xrepeat = sprite[i].yrepeat = 0;
                        sprite[i].cstat = SLT = SHT = 0;
                        return i;
                    }
                }
                CS |= 257;
                if( sprite[i].pal && PN != ACCESSSWITCH && PN != ACCESSSWITCH2)
                    sprite[i].pal = 0;
                return i;
            }

            if( SHT )
            {
                changespritestat(i,12);
                CS |=  257;
                SH = impact_damage;
                return i;
            }
        }

        s = PN;

        if( CS&1 ) CS |= 256;

        if( actorscrptr[s] )
        {
            SH = *(actorscrptr[s]);
            T5 = *(actorscrptr[s]+1);
            T2 = *(actorscrptr[s]+2);
            if( *(actorscrptr[s]+3) && SHT == 0 )
                SHT = *(actorscrptr[s]+3);
        }
        else T2 = T5 = 0;
    }

    sp = &sprite[i];
    sect = sp->sectnum;

    switch(sp->picnum)
    {
            default:

                if( actorscrptr[sp->picnum] )
                {
                    if( j == -1 && sp->lotag > ud.player_skill )
                    {
                        sp->xrepeat=sp->yrepeat=0;
                        changespritestat(i,5);
                        break;
                    }

                        //  Init the size
                    if(sp->xrepeat == 0 || sp->yrepeat == 0)
                        sp->xrepeat = sp->yrepeat = 1;

                    if( actortype[sp->picnum] & 3)
                    {
                        if( ud.monsters_off == 1 )
                        {
                            sp->xrepeat=sp->yrepeat=0;
                            changespritestat(i,5);
                            break;
                        }

                        makeitfall(i);

                        if( actortype[sp->picnum] & 2)
                            hittype[i].actorstayput = sp->sectnum;

                        ps[myconnectindex].max_actors_killed++;
                        sp->clipdist = 80;
                        if(j >= 0)
                        {
                            if(sprite[j].picnum == RESPAWN)
                                hittype[i].tempang = sprite[i].pal = sprite[j].pal;
                            changespritestat(i,1);
                        }
                        else changespritestat(i,2);
                    }
                    else
                    {
                        sp->clipdist = 40;
                        sp->owner = i;
                        changespritestat(i,1);
                    }

                    hittype[i].timetosleep = 0;

                    if(j >= 0)
                        sp->ang = sprite[j].ang;
                }
                break;
            case FOF:
                sp->xrepeat = sp->yrepeat = 0;
                changespritestat(i,5);
                break;
            case WATERSPLASH2:
                if(j >= 0)
                {
                    setsprite(i,sprite[j].x,sprite[j].y,sprite[j].z);
                    sp->xrepeat = sp->yrepeat = 8+(TRAND&7);
                }
                else sp->xrepeat = sp->yrepeat = 16+(TRAND&15);

                sp->shade = -16;
                sp->cstat |= 128;
                if(j >= 0)
                {
                    if(sector[sprite[j].sectnum].lotag == 2)
                    {
                        sp->z = getceilzofslope(SECT,SX,SY)+(16<<8);
                        sp->cstat |= 8;
                    }
                    else if( sector[sprite[j].sectnum].lotag == 1)
                        sp->z = getflorzofslope(SECT,SX,SY);
                }

                if(sector[sect].floorpicnum == FLOORSLIME ||
                    sector[sect].ceilingpicnum == FLOORSLIME)
                        sp->pal = 7;
            case NEON1:
            case NEON2:
            case NEON3:
            case NEON4:
            case NEON5:
            case NEON6:
            case DOMELITE:
                if(sp->picnum != WATERSPLASH2)
                    sp->cstat |= 257;
            case NUKEBUTTON:
                if(sp->picnum == DOMELITE)
                    sp->cstat |= 257;
            case JIBS1:
            case JIBS2:
            case JIBS3:
            case JIBS4:
            case JIBS5:
            case JIBS6:
            case HEADJIB1:
            case ARMJIB1:
            case LEGJIB1:
            case LIZMANHEAD1:
            case LIZMANARM1:
            case LIZMANLEG1:
            case DUKETORSO:
            case DUKEGUN:
            case DUKELEG:
                changespritestat(i,5);
                break;
            case TONGUE:
                if(j >= 0)
                    sp->ang = sprite[j].ang;
                sp->z -= 38<<8;
                sp->zvel = 256-(TRAND&511);
                sp->xvel = 64-(TRAND&127);
                changespritestat(i,4);
                break;
            case NATURALLIGHTNING:
                sp->cstat &= ~257;
                sp->cstat |= 32768;
                break;
            case TRANSPORTERSTAR:
            case TRANSPORTERBEAM:
                if(j == -1) break;
                if(sp->picnum == TRANSPORTERBEAM)
                {
                    sp->xrepeat = 31;
                    sp->yrepeat = 1;
                    sp->z = sector[sprite[j].sectnum].floorz-(40<<8);
                }
                else
                {
                    if(sprite[j].statnum == 4)
                    {
                        sp->xrepeat = 8;
                        sp->yrepeat = 8;
                    }
                    else
                    {
                        sp->xrepeat = 48;
                        sp->yrepeat = 64;
                        if(sprite[j].statnum == 10 || badguy(&sprite[j]) )
                            sp->z -= (32<<8);
                    }
                }

                sp->shade = -127;
                sp->cstat = 128|2;
                sp->ang = sprite[j].ang;

                sp->xvel = 128;
                changespritestat(i,5);
                ssp(i,CLIPMASK0);
                setsprite(i,sp->x,sp->y,sp->z);
                break;

            case FRAMEEFFECT1:
			case FRAMEEFFECT1_13CON:
				if(j >= 0)
                {
                    sp->xrepeat = sprite[j].xrepeat;
                    sp->yrepeat = sprite[j].yrepeat;
                    T2 = sprite[j].picnum;
                }
                else sp->xrepeat = sp->yrepeat = 0;

                changespritestat(i,5);

                break;

            case LASERLINE:
                sp->yrepeat = 6;
                sp->xrepeat = 32;

                if(lasermode == 1)
                    sp->cstat = 16 + 2;
                else if(lasermode == 0 || lasermode == 2)
                    sp->cstat = 16;
                else
                {
                    sp->xrepeat = 0;
                    sp->yrepeat = 0;
                }

                if(j >= 0) sp->ang = hittype[j].temp_data[5]+512;
                changespritestat(i,5);
                break;

            case FORCESPHERE:
                if(j == -1 )
                {
                    sp->cstat = (short) 32768;
                    changespritestat(i,2);
                }
                else
                {
                    sp->xrepeat = sp->yrepeat = 1;
                    changespritestat(i,5);
                }
                break;

            case BLOOD:
               sp->xrepeat = sp->yrepeat = 16;
               sp->z -= (26<<8);
               if( j >= 0 && sprite[j].pal == 6 )
                   sp->pal = 6;
               changespritestat(i,5);
               break;
            case BLOODPOOL:
            case PUKE:
                {
                    short s1;
                    s1 = sp->sectnum;

                    updatesector(sp->x+108,sp->y+108,&s1);
                    if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                    {
                        updatesector(sp->x-108,sp->y-108,&s1);
                        if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                        {
                            updatesector(sp->x+108,sp->y-108,&s1);
                            if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                            {
                                updatesector(sp->x-108,sp->y+108,&s1);
                                if(s1 >= 0 && sector[s1].floorz != sector[sp->sectnum].floorz)
                                { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                            }
                            else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                        }
                        else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                    }
                    else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                }

                if( sector[SECT].lotag == 1 )
                {
                    changespritestat(i,5);
                    break;
                }

                if(j >= 0 && sp->picnum != PUKE)
                {
                    if( sprite[j].pal == 1)
                        sp->pal = 1;
                    else if( sprite[j].pal != 6 && sprite[j].picnum != NUKEBARREL && sprite[j].picnum != TIRE )
                    {
                        if(sprite[j].picnum == FECES)
                            sp->pal = 7; // Brown
                        else sp->pal = 2; // Red
                    }
                    else sp->pal = 0;  // green

                    if(sprite[j].picnum == TIRE)
                        sp->shade = 127;
                }
                sp->cstat |= 32;
            case FECES:
                if( j >= 0)
                    sp->xrepeat = sp->yrepeat = 1;
                changespritestat(i,5);
                break;

            case BLOODSPLAT1:
            case BLOODSPLAT2:
            case BLOODSPLAT3:
            case BLOODSPLAT4:
                sp->cstat |= 16;
                sp->xrepeat = 7+(TRAND&7);
                sp->yrepeat = 7+(TRAND&7);
                sp->z -= (16<<8);
                if(j >= 0 && sprite[j].pal == 6)
                    sp->pal = 6;
                insertspriteq(i);
                changespritestat(i,5);
                break;

            case TRIPBOMB:
                if( sp->lotag > ud.player_skill )
                {
                    sp->xrepeat=sp->yrepeat=0;
                    changespritestat(i,5);
                    break;
                }

                sp->xrepeat=4;
                sp->yrepeat=5;

                sp->owner = i;
                sp->hitag = i;

                sp->xvel = 16;
                ssp(i,CLIPMASK0);
                hittype[i].temp_data[0] = 17;
                hittype[i].temp_data[2] = 0;
                hittype[i].temp_data[5] = sp->ang;

            case SPACEMARINE:
                if(sp->picnum == SPACEMARINE)
                {
                    sp->extra = 20;
                    sp->cstat |= 257;
                }
                changespritestat(i,2);
                break;

            case HYDRENT:
            case PANNEL1:
            case PANNEL2:
            case SATELITE:
            case FUELPOD:
            case SOLARPANNEL:
            case ANTENNA:
            case GRATE1:
            case CHAIR1:
            case CHAIR2:
            case CHAIR3:
            case BOTTLE1:
            case BOTTLE2:
            case BOTTLE3:
            case BOTTLE4:
            case BOTTLE5:
            case BOTTLE6:
            case BOTTLE7:
            case BOTTLE8:
            case BOTTLE10:
            case BOTTLE11:
            case BOTTLE12:
            case BOTTLE13:
            case BOTTLE14:
            case BOTTLE15:
            case BOTTLE16:
            case BOTTLE17:
            case BOTTLE18:
            case BOTTLE19:
            case OCEANSPRITE1:
            case OCEANSPRITE2:
            case OCEANSPRITE3:
            case OCEANSPRITE5:
            case MONK:
            case INDY:
            case LUKE:
            case JURYGUY:
            case SCALE:
            case VACUUM:
            case FANSPRITE:
            case CACTUS:
            case CACTUSBROKE:
            case HANGLIGHT:
            case FETUS:
            case FETUSBROKE:
            case CAMERALIGHT:
            case MOVIECAMERA:
            case IVUNIT:
            case POT1:
            case POT2:
            case POT3:
            case TRIPODCAMERA:
            case SUSHIPLATE1:
            case SUSHIPLATE2:
            case SUSHIPLATE3:
            case SUSHIPLATE4:
            case SUSHIPLATE5:
            case WAITTOBESEATED:
            case VASE:
            case PIPE1:
            case PIPE2:
            case PIPE3:
            case PIPE4:
            case PIPE5:
            case PIPE6:
                sp->clipdist = 32;
                sp->cstat |= 257;
            case OCEANSPRITE4:
                changespritestat(i,0);
                break;
            case FEMMAG1:
            case FEMMAG2:
                sp->cstat &= ~257;
                changespritestat(i,0);
                break;
            case DUKETAG:
            case SIGN1:
            case SIGN2:
                if(ud.multimode < 2 && sp->pal)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                }
                else sp->pal = 0;
                break;
            case MASKWALL1:
            case MASKWALL2:
            case MASKWALL3:
            case MASKWALL4:
            case MASKWALL5:
            case MASKWALL6:
            case MASKWALL7:
            case MASKWALL8:
            case MASKWALL9:
            case MASKWALL10:
            case MASKWALL11:
            case MASKWALL12:
            case MASKWALL13:
            case MASKWALL14:
            case MASKWALL15:
                j = sp->cstat&60;
                sp->cstat = j|1;
                changespritestat(i,0);
                break;
            case FOOTPRINTS:
            case FOOTPRINTS2:
            case FOOTPRINTS3:
            case FOOTPRINTS4:
                if(j >= 0)
                {
                    short s1;
                    s1 = sp->sectnum;

                    updatesector(sp->x+84,sp->y+84,&s1);
                    if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                    {
                        updatesector(sp->x-84,sp->y-84,&s1);
                        if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                        {
                            updatesector(sp->x+84,sp->y-84,&s1);
                            if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
                            {
                                updatesector(sp->x-84,sp->y+84,&s1);
                                if(s1 >= 0 && sector[s1].floorz != sector[sp->sectnum].floorz)
                                { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
                            }
                            else { sp->xrepeat = sp->yrepeat = 0;break;}
                        }
                        else { sp->xrepeat = sp->yrepeat = 0;break;}
                    }
                    else { sp->xrepeat = sp->yrepeat = 0;break;}

                    sp->cstat = 32+((ps[sprite[j].yvel].footprintcount&1)<<2);
                    sp->ang = sprite[j].ang;
                }

                sp->z = sector[sect].floorz;
                if(sector[sect].lotag != 1 && sector[sect].lotag != 2)
                    sp->xrepeat = sp->yrepeat = 32;

                insertspriteq(i);
                changespritestat(i,5);
                break;

            case FEM1:
            case FEM2:
            case FEM3:
            case FEM4:
            case FEM5:
            case FEM6:
            case FEM7:
            case FEM8:
            case FEM9:
            case FEM10:
            case PODFEM1:
            case NAKED1:
            case STATUE:
            case TOUGHGAL:
                sp->yvel = sp->hitag;
                sp->hitag = -1;
                if(sp->picnum == PODFEM1) sp->extra <<= 1;
            case BLOODYPOLE:

            case QUEBALL:
            case STRIPEBALL:

                if(sp->picnum == QUEBALL || sp->picnum == STRIPEBALL)
                {
                    sp->cstat = 256;
                    sp->clipdist = 8;
                }
                else
                {
                    sp->cstat |= 257;
                    sp->clipdist = 32;
                }

                changespritestat(i,2);
                break;

            case DUKELYINGDEAD:
                if(j >= 0 && sprite[j].picnum == APLAYER)
                {
                    sp->xrepeat = sprite[j].xrepeat;
                    sp->yrepeat = sprite[j].yrepeat;
                    sp->shade = sprite[j].shade;
                    sp->pal = ps[sprite[j].yvel].palookup;
                }
            case DUKECAR:
            case HELECOPT:
//                if(sp->picnum == HELECOPT || sp->picnum == DUKECAR) sp->xvel = 1024;
                sp->cstat = 0;
                sp->extra = 1;
                sp->xvel = 292;
                sp->zvel = 360;
            case RESPAWNMARKERRED:
            case BLIMP:

                if(sp->picnum == RESPAWNMARKERRED)
                {
                    sp->xrepeat = sp->yrepeat = 24;
                    if(j >= 0) sp->z = hittype[j].floorz; // -(1<<4);
                }
                else
                {
                    sp->cstat |= 257;
                    sp->clipdist = 128;
                }
            case MIKE:
                if(sp->picnum == MIKE)
                    sp->yvel = sp->hitag;
            case WEATHERWARN:
                changespritestat(i,1);
                break;

            case SPOTLITE:
                T1 = sp->x;
                T2 = sp->y;
                break;
            case BULLETHOLE:
                sp->xrepeat = sp->yrepeat = 3;
                sp->cstat = 16+(TRAND&12);
                insertspriteq(i);
            case MONEY:
            case MAIL:
            case PAPER:
                if( sp->picnum == MONEY || sp->picnum == MAIL || sp->picnum == PAPER )
                {
                    hittype[i].temp_data[0] = TRAND&2047;
                    sp->cstat = TRAND&12;
                    sp->xrepeat = sp->yrepeat = 8;
                    sp->ang = TRAND&2047;
                }
                changespritestat(i,5);
                break;

            case VIEWSCREEN:
            case VIEWSCREEN2:
                sp->owner = i;
                sp->lotag = 1;
                sp->extra = 1;
                changespritestat(i,6);
                break;

            case SHELL: //From the player
            case SHOTGUNSHELL:
                if( j >= 0 )
                {
                    short snum,a;

                    if(sprite[j].picnum == APLAYER)
                    {
                        snum = sprite[j].yvel;
                        a = ps[snum].ang-(TRAND&63)+8;  //Fine tune

                        T1 = TRAND&1;
                        if(sp->picnum == SHOTGUNSHELL)
                            sp->z = (6<<8)+ps[snum].pyoff+ps[snum].posz-((ps[snum].horizoff+ps[snum].horiz-100)<<4);
                        else sp->z = (3<<8)+ps[snum].pyoff+ps[snum].posz-((ps[snum].horizoff+ps[snum].horiz-100)<<4);
                        sp->zvel = -(TRAND&255);
                    }
                    else
                    {
                        a = sp->ang;
                        sp->z = sprite[j].z-PHEIGHT+(3<<8);
                    }

                    sp->x = sprite[j].x+(sintable[(a+512)&2047]>>7);
                    sp->y = sprite[j].y+(sintable[a&2047]>>7);

                    sp->shade = -8;

                    sp->ang = a-512;
                    sp->xvel = 20;

					//  do not try to make it 0 when ud.hideweapon Will make OOS when shooting in water
					sp->xrepeat=sp->yrepeat=4;

                    changespritestat(i,5);
                }
                break;

            case RESPAWN:
                sp->extra = 66-13;
            case MUSICANDSFX:
                if( ud.multimode < 2 && sp->pal == 1)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }
                sp->cstat = (short)32768;
                changespritestat(i,11);
                break;

            case EXPLOSION2:
            case EXPLOSION2BOT:
            case BURNING:
            case BURNING2:
            case SMALLSMOKE:
            case SHRINKEREXPLOSION:
            case COOLEXPLOSION1:

                if(j >= 0)
                {
                    sp->ang = sprite[j].ang;
                    sp->shade = -64;
                    sp->cstat = 128|(TRAND&4);
                }

                if(sp->picnum == EXPLOSION2 || sp->picnum == EXPLOSION2BOT)
                {
                    sp->xrepeat = 48;
                    sp->yrepeat = 48;
                    sp->shade = -127;
                    sp->cstat |= 128;
                }
                else if(sp->picnum == SHRINKEREXPLOSION )
                {
                    sp->xrepeat = 32;
                    sp->yrepeat = 32;
                }
                else if( sp->picnum == SMALLSMOKE )
                {
                    // 64 "money"
                    sp->xrepeat = 24;
                    sp->yrepeat = 24;
                }
                else if(sp->picnum == BURNING || sp->picnum == BURNING2)
                {
                    sp->xrepeat = 4;
                    sp->yrepeat = 4;
                }

                if(j >= 0)
                {
                    x = getflorzofslope(sp->sectnum,sp->x,sp->y);
                    if(sp->z > x-(12<<8) )
                        sp->z = x-(12<<8);
                }

                changespritestat(i,5);

                break;

            case PLAYERONWATER:
                if(j >= 0)
                {
                    sp->xrepeat = sprite[j].xrepeat;
                    sp->yrepeat = sprite[j].yrepeat;
                    sp->zvel = 128;
                    if(sector[sp->sectnum].lotag != 2)
                        sp->cstat |= 32768;
                }
                changespritestat(i,13);
                break;

            case APLAYER:
                sp->xrepeat = sp->yrepeat = 0;
                j = ud.coop;
                if(j == 2) j = 0;

                if( ud.multimode < 2 || (ud.multimode > 1 && j != sp->lotag) )
                    changespritestat(i,5);
                else
                    changespritestat(i,10);
                break;
            case WATERBUBBLE:
                if(j >= 0 && sprite[j].picnum == APLAYER)
                    sp->z -= (16<<8);
                if( sp->picnum == WATERBUBBLE)
                {
                    if( j >= 0 )
                        sp->ang = sprite[j].ang;
                    sp->xrepeat = sp->yrepeat = 4;
                }
                else sp->xrepeat = sp->yrepeat = 32;

                changespritestat(i,5);
                break;

            case CRANE:

                sp->cstat |= 64|257;

                sp->picnum += 2;
                sp->z = sector[sect].ceilingz+(48<<8);
                T5 = tempwallptr;

                msx[tempwallptr] = sp->x;
                msy[tempwallptr] = sp->y;
                msx[tempwallptr+2] = sp->z;

                s = headspritestat[0];
                while(s >= 0)
                {
                    if( sprite[s].picnum == CRANEPOLE && SHT == (sprite[s].hitag) )
                    {
                        msy[tempwallptr+2] = s;

                        T2 = sprite[s].sectnum;

                        sprite[s].xrepeat = 48;
                        sprite[s].yrepeat = 128;

                        msx[tempwallptr+1] = sprite[s].x;
                        msy[tempwallptr+1] = sprite[s].y;

                        sprite[s].x = sp->x;
                        sprite[s].y = sp->y;
                        sprite[s].z = sp->z;
                        sprite[s].shade = sp->shade;

                        setsprite(s,sprite[s].x,sprite[s].y,sprite[s].z);
                        break;
                    }
                    s = nextspritestat[s];
                }

                tempwallptr += 3;
                sp->owner = -1;
                sp->extra = 8;
                changespritestat(i,6);
                break;

            case WATERDRIP:
                if((j >= 0 && sprite[j].statnum == 10) || sprite[j].statnum == 1)
                {
                    sp->shade = 32;
                    if(sprite[j].pal != 1)
                    {
                        sp->pal = 2;
                        sp->z -= (18<<8);
                    }
                    else sp->z -= (13<<8);
                    sp->ang = getangle(ps[connecthead].posx-sp->x,ps[connecthead].posy-sp->y);
                    sp->xvel = 48-(TRAND&31);
                    ssp(i,CLIPMASK0);
                }
                else if(j == -1)
                {
                    sp->z += (4<<8);
                    T1 = sp->z;
                    T2 = TRAND&127;
                }
            case TRASH:

                if(sp->picnum != WATERDRIP)
                    sp->ang = TRAND&2047;

            case WATERDRIPSPLASH:

                sp->xrepeat = 24;
                sp->yrepeat = 24;


                changespritestat(i,6);
                break;

            case PLUG:
                sp->lotag = 9999;
                changespritestat(i,6);
                break;
            case TOUCHPLATE:
                T3 = sector[sect].floorz;
                if(sector[sect].lotag != 1 && sector[sect].lotag != 2)
                    sector[sect].floorz = sp->z;
                if(sp->pal && ud.multimode > 1)
                {
                    sp->xrepeat=sp->yrepeat=0;
                    changespritestat(i,5);
                    break;
                }
            case WATERBUBBLEMAKER:
                sp->cstat |= 32768;
                changespritestat(i,6);
                break;
            case BOLT1:
            case BOLT1+1:
            case BOLT1+2:
            case BOLT1+3:
            case SIDEBOLT1:
            case SIDEBOLT1+1:
            case SIDEBOLT1+2:
            case SIDEBOLT1+3:
                T1 = sp->xrepeat;
                T2 = sp->yrepeat;
            case MASTERSWITCH:
                if(sp->picnum == MASTERSWITCH)
                    sp->cstat |= 32768;
                sp->yvel = 0;
                changespritestat(i,6);
                break;
            case TARGET:
            case DUCK:
            case LETTER:
                sp->extra = 1;
                sp->cstat |= 257;
                changespritestat(i,1);
                break;
            case OCTABRAINSTAYPUT:
            case LIZTROOPSTAYPUT:
            case PIGCOPSTAYPUT:
            case LIZMANSTAYPUT:
            case BOSS1STAYPUT:
            case PIGCOPDIVE:
            case COMMANDERSTAYPUT:
            case BOSS4STAYPUT:
                hittype[i].actorstayput = sp->sectnum;
            case BOSS1:
            case BOSS2:
            case BOSS3:
            case BOSS4:
            case ROTATEGUN:
            case GREENSLIME:
                if(sp->picnum == GREENSLIME)
                    sp->extra = 1;
            case DRONE:
            case LIZTROOPONTOILET:
            case LIZTROOPJUSTSIT:
            case LIZTROOPSHOOT:
            case LIZTROOPJETPACK:
            case LIZTROOPDUCKING:
            case LIZTROOPRUNNING:
            case LIZTROOP:
            case OCTABRAIN:
            case COMMANDER:
            case PIGCOP:
            case LIZMAN:
            case LIZMANSPITTING:
            case LIZMANFEEDING:
            case LIZMANJUMP:
            case ORGANTIC:
            case RAT:
            case SHARK:

                if(sp->pal == 0)
                {
                    switch(sp->picnum)
                    {
                        case LIZTROOPONTOILET:
                        case LIZTROOPSHOOT:
                        case LIZTROOPJETPACK:
                        case LIZTROOPDUCKING:
                        case LIZTROOPRUNNING:
                        case LIZTROOPSTAYPUT:
                        case LIZTROOPJUSTSIT:
                        case LIZTROOP:
                            sp->pal = 22;
                            break;
                    }
                }

                if( sp->picnum == BOSS4STAYPUT || sp->picnum == BOSS1 || sp->picnum == BOSS2 || sp->picnum == BOSS1STAYPUT || sp->picnum == BOSS3 || sp->picnum == BOSS4 )
                {
                    if(j >= 0 && sprite[j].picnum == RESPAWN)
                        sp->pal = sprite[j].pal;
                    if(sp->pal)
                    {
                        sp->clipdist = 80;
                        sp->xrepeat = 40;
                        sp->yrepeat = 40;
                    }
                    else
                    {
                        sp->xrepeat = 80;
                        sp->yrepeat = 80;
                        sp->clipdist = 164;
                    }
                }
                else
                {
                    if(sp->picnum != SHARK)
                    {
                        sp->xrepeat = 40;
                        sp->yrepeat = 40;
                        sp->clipdist = 80;
                    }
                    else
                    {
                        sp->xrepeat = 60;
                        sp->yrepeat = 60;
                        sp->clipdist = 40;
                    }
                }

                if(j >= 0) sp->lotag = 0;

                if( ( sp->lotag > ud.player_skill ) || ud.monsters_off == 1 )
                {
                    sp->xrepeat=sp->yrepeat=0;
                    changespritestat(i,5);
                    break;
                }
                else
                {
                    makeitfall(i);

                    if(sp->picnum == RAT)
                    {
                        sp->ang = TRAND&2047;
                        sp->xrepeat = sp->yrepeat = 48;
                        sp->cstat = 0;
                    }
                    else
                    {
                        sp->cstat |= 257;

                        if(sp->picnum != SHARK)
                            ps[myconnectindex].max_actors_killed++;
                    }

                    if(sp->picnum == ORGANTIC) sp->cstat |= 128;

                    if(j >= 0)
                    {
                        hittype[i].timetosleep = 0;
                        check_fta_sounds(i);
                        changespritestat(i,1);
                    }
                    else changespritestat(i,2);
                }

                if(sp->picnum == ROTATEGUN)
                    sp->zvel = 0;

                break;

            case LOCATORS:
                sp->cstat |= 32768;
                changespritestat(i,7);
                break;

            case ACTIVATORLOCKED:
            case ACTIVATOR:
                sp->cstat = (short) 32768;
                if(sp->picnum == ACTIVATORLOCKED)
                    sector[sp->sectnum].lotag |= 16384;
                changespritestat(i,8);
                break;

            case DOORSHOCK:
                sp->cstat |= 1+256;
                sp->shade = -12;
                changespritestat(i,6);
                break;

            case OOZ:
            case OOZ2:
                sp->shade = -12;

                if(j >= 0)
                {
                    if( sprite[j].picnum == NUKEBARREL )
                        sp->pal = 8;
                    insertspriteq(i);
                }

                changespritestat(i,1);

                getglobalz(i);

                j = (hittype[i].floorz-hittype[i].ceilingz)>>9;

                sp->yrepeat = j;
                sp->xrepeat = 25-(j>>1);
                sp->cstat |= (TRAND&4);

                break;

            case HEAVYHBOMB:
                if(j >= 0)
                    sp->owner = j;
                else sp->owner = i;
                sp->xrepeat = sp->yrepeat = 9;
                sp->yvel = 4;
            case REACTOR2:
            case REACTOR:
            case RECON:

                if(sp->picnum == RECON)
                {
                    if( sp->lotag > ud.player_skill )
                    {
                        sp->xrepeat = sp->yrepeat = 0;
                        changespritestat(i,5);
                        return i;
                    }
                    ps[myconnectindex].max_actors_killed++;
                    hittype[i].temp_data[5] = 0;
                    if(ud.monsters_off == 1)
                    {
                        sp->xrepeat = sp->yrepeat = 0;
                        changespritestat(i,5);
                        break;
                    }
                    sp->extra = 130;
                }

                if(sp->picnum == REACTOR || sp->picnum == REACTOR2)
                    sp->extra = impact_damage;

                CS |= 257; // Make it hitable

                if( ud.multimode < 2 && sp->pal != 0)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }
                sp->pal = 0;
                SS = -17;

                changespritestat(i,2);
                break;

            case ATOMICHEALTH:
            case STEROIDS:
            case HEATSENSOR:
            case SHIELD:
            case AIRTANK:
            case TRIPBOMBSPRITE:
            case JETPACK:
            case HOLODUKE:

            case FIRSTGUNSPRITE:
            case CHAINGUNSPRITE:
            case SHOTGUNSPRITE:
            case RPGSPRITE:
            case SHRINKERSPRITE:
            case FREEZESPRITE:
            case DEVISTATORSPRITE:

            case SHOTGUNAMMO:
            case FREEZEAMMO:
            case HBOMBAMMO:
            case CRYSTALAMMO:
            case GROWAMMO:
            case BATTERYAMMO:
            case DEVISTATORAMMO:
            case RPGAMMO:
            case BOOTS:
            case AMMO:
            case AMMOLOTS:
            case COLA:
            case FIRSTAID:
            case SIXPAK:
                if(j >= 0)
                {
                    sp->lotag = 0;
                    sp->z -= (32<<8);
                    sp->zvel = -1024;
                    ssp(i,CLIPMASK0);
                    sp->cstat = TRAND&4;
                }
                else
                {
                    sp->owner = i;
                    sp->cstat = 0;
                }

                if( ( ud.multimode < 2 && sp->pal != 0) || (sp->lotag > ud.player_skill) )
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }

                sp->pal = 0;

            case ACCESSCARD:

                if(sp->picnum == ATOMICHEALTH)
                    sp->cstat |= 128;

                if(ud.multimode > 1 && ud.coop != 1 && sp->picnum == ACCESSCARD)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }
                else
                {
                    if(sp->picnum == AMMO)
                        sp->xrepeat = sp->yrepeat = 16;
                    else sp->xrepeat = sp->yrepeat = 32;
                }

                sp->shade = -17;

                if(j >= 0) changespritestat(i,1);
                else
                {
                    changespritestat(i,2);
                    makeitfall(i);
                }
                break;

            case WATERFOUNTAIN:
                SLT = 1;

            case TREE1:
            case TREE2:
            case TIRE:
            case CONE:
            case BOX:
                CS = 257; // Make it hitable
                sprite[i].extra = 1;
                changespritestat(i,6);
                break;

            case FLOORFLAME:
                sp->shade = -127;
                changespritestat(i,6);
                break;

            case BOUNCEMINE:
                sp->owner = i;
                sp->cstat |= 1+256; //Make it hitable
                sp->xrepeat = sp->yrepeat = 24;
                sp->shade = -127;
                sp->extra = impact_damage<<2;
                changespritestat(i,2);
                break;

            case CAMERA1:
            case CAMERA1+1:
            case CAMERA1+2:
            case CAMERA1+3:
            case CAMERA1+4:
            case CAMERAPOLE:
                sp->extra = 1;

                if(camerashitable) sp->cstat = 257;
                else sp->cstat = 0;

            case GENERICPOLE:

                if( ud.multimode < 2 && sp->pal != 0 )
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }
                else sp->pal = 0;
                if(sp->picnum == CAMERAPOLE || sp->picnum == GENERICPOLE) break;
                sp->picnum = CAMERA1;
                changespritestat(i,1);
                break;
            case STEAM:
                if(j >= 0)
                {
                    sp->ang = sprite[j].ang;
                    sp->cstat = 16+128+2;
                    sp->xrepeat=sp->yrepeat=1;
                    sp->xvel = -8;
                    ssp(i,CLIPMASK0);
                }
            case CEILINGSTEAM:
                changespritestat(i,6);
                break;

            case SECTOREFFECTOR:
                sp->yvel = sector[sect].extra;
                sp->cstat |= 32768;
                sp->xrepeat = sp->yrepeat = 0;

                switch(sp->lotag)
                {
                    case 28:
                        T6 = 65;// Delay for lightning
                        break;
                    case 7: // Transporters!!!!
                    case 23:// XPTR END
                        if(sp->lotag != 23)
                        {
                            for(j=0;j<MAXSPRITES;j++)
                                if(sprite[j].statnum < MAXSTATUS && sprite[j].picnum == SECTOREFFECTOR && ( sprite[j].lotag == 7 || sprite[j].lotag == 23 ) && i != j && sprite[j].hitag == SHT)
                                {
                                    OW = j;
                                    break;
                                }
                        }
                        else OW = i;

                        T5 = sector[sect].floorz == SZ;
                        sp->cstat = 0;
                        changespritestat(i,9);
                        return i;
                    case 1:
                        sp->owner = -1;
                        T1 = 1;
                        break;
                    case 18:

                        if(sp->ang == 512)
                        {
                            T2 = sector[sect].ceilingz;
                            if(sp->pal)
                                sector[sect].ceilingz = sp->z;
                        }
                        else
                        {
                            T2 = sector[sect].floorz;
                            if(sp->pal)
                                sector[sect].floorz = sp->z;
                        }

                        sp->hitag <<= 2;
                        break;

                    case 19:
                        sp->owner = -1;
                        break;
                    case 25: // Pistons
                        T4 = sector[sect].ceilingz;
                        T5 = 1;
                        sector[sect].ceilingz = sp->z;
                        setinterpolation(&sector[sect].ceilingz);
                        break;
                    case 35:
                        sector[sect].ceilingz = sp->z;
                        break;
                    case 27:
                        if(ud.recstat == 1)
                        {
                            sp->xrepeat=sp->yrepeat=64;
                            sp->cstat &= 32767;
                        }
                        break;
                    case 12:

                        T2 = sector[sect].floorshade;
                        T3 = sector[sect].ceilingshade;
                        break;

                    case 13:

                        T1 = sector[sect].ceilingz;
                        T2 = sector[sect].floorz;

                        if( klabs(T1-sp->z) < klabs(T2-sp->z) )
                            sp->owner = 1;
                        else sp->owner = 0;

                        if(sp->ang == 512)
                        {
                            if(sp->owner)
                                sector[sect].ceilingz = sp->z;
                            else
                                sector[sect].floorz = sp->z;
                        }
                        else
                            sector[sect].ceilingz = sector[sect].floorz = sp->z;

                        if( sector[sect].ceilingstat&1 )
                        {
                            sector[sect].ceilingstat ^= 1;
                            T4 = 1;

                            if(!sp->owner && sp->ang==512)
                            {
                                sector[sect].ceilingstat ^= 1;
                                T4 = 0;
                            }

                            sector[sect].ceilingshade =
                                sector[sect].floorshade;

                            if(sp->ang==512)
                            {
                                startwall = sector[sect].wallptr;
                                endwall = startwall+sector[sect].wallnum;
                                for(j=startwall;j<endwall;j++)
                                {
                                    x = wall[j].nextsector;
                                    if(x >= 0)
                                        if( !(sector[x].ceilingstat&1) )
                                    {
                                        sector[sect].ceilingpicnum =
                                            sector[x].ceilingpicnum;
                                        sector[sect].ceilingshade =
                                            sector[x].ceilingshade;
                                        break; //Leave earily
                                    }
                                }
                            }
                        }

                        break;

                    case 17:

                        T3 = sector[sect].floorz; //Stopping loc

                        j = nextsectorneighborz(sect,sector[sect].floorz,-1,-1);
                        T4 = sector[j].ceilingz;

                        j = nextsectorneighborz(sect,sector[sect].ceilingz,1,1);
                        T5 = sector[j].floorz;

                        if(numplayers < 2)
                        {
                            setinterpolation(&sector[sect].floorz);
                            setinterpolation(&sector[sect].ceilingz);
                        }

                        break;

                    case 24:
                        sp->yvel <<= 1;
                    case 36:
                        break;

                    case 20:
                    {
                        int32_t q;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        //find the two most clostest wall x's and y's
                        q = 0x7fffffff;

                        for(s=startwall;s<endwall;s++)
                        {
                            x = wall[s].x;
                            y = wall[s].y;

                            d = FindDistance2D(sp->x-x,sp->y-y);
                            if( d < q )
                            {
                                q = d;
                                clostest = s;
                            }
                        }

                        T2 = clostest;

                        q = 0x7fffffff;

                        for(s=startwall;s<endwall;s++)
                        {
                            x = wall[s].x;
                            y = wall[s].y;

                            d = FindDistance2D(sp->x-x,sp->y-y);
                            if(d < q && s != T2)
                            {
                                q = d;
                                clostest = s;
                            }
                        }

                        T3 = clostest;
                    }

                    break;

                    case 3:

                        T4=sector[sect].floorshade;

                        sector[sect].floorshade = sp->shade;
                        sector[sect].ceilingshade = sp->shade;

                        sp->owner = sector[sect].ceilingpal<<8;
                        sp->owner |= sector[sect].floorpal;

                        //fix all the walls;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                        {
                            if(!(wall[s].hitag&1))
                                wall[s].shade=sp->shade;
                            if( (wall[s].cstat&2) && wall[s].nextwall >= 0)
                                wall[wall[s].nextwall].shade = sp->shade;
                        }
                        break;

                    case 31:
                        T2 = sector[sect].floorz;
                    //    T3 = sp->hitag;
                        if(sp->ang != 1536) sector[sect].floorz = sp->z;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].hitag == 0) wall[s].hitag = 9999;

                        setinterpolation(&sector[sect].floorz);

                        break;
                    case 32:
                        T2 = sector[sect].ceilingz;
                        T3 = sp->hitag;
                        if(sp->ang != 1536) sector[sect].ceilingz = sp->z;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].hitag == 0) wall[s].hitag = 9999;

                        setinterpolation(&sector[sect].ceilingz);

                        break;

                    case 4: //Flashing lights

                        T3 = sector[sect].floorshade;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        sp->owner = sector[sect].ceilingpal<<8;
                        sp->owner |= sector[sect].floorpal;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].shade > T4)
                                T4 = wall[s].shade;

                        break;

                    case 9:
                        if( sector[sect].lotag &&
                            labs(sector[sect].ceilingz-sp->z) > 1024)
                                sector[sect].lotag |= 32768; //If its open
                    case 8:
                        //First, get the ceiling-floor shade

                        T1 = sector[sect].floorshade;
                        T2 = sector[sect].ceilingshade;

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        for(s=startwall;s<endwall;s++)
                            if(wall[s].shade > T3)
                                T3 = wall[s].shade;

                        T4 = 1; //Take Out;

                        break;

                    case 11://Pivitor rotater
                        if(sp->ang>1024) T4 = 2;
                        else T4 = -2;
                    case 0:
                    case 2://Earthquakemakers
                    case 5://Boss Creature
                    case 6://Subway
                    case 14://Caboos
                    case 15://Subwaytype sliding door
                    case 16://That rotating blocker reactor thing
                    case 26://ESCELATOR
                    case 30://No rotational subways

                        if(sp->lotag == 0)
                        {
                            if( sector[sect].lotag == 30 )
                            {
                                if(sp->pal) sprite[i].clipdist = 1;
                                else sprite[i].clipdist = 0;
                                T4 = sector[sect].floorz;
                                sector[sect].hitag = i;
                            }

                            for(j = 0;j < MAXSPRITES;j++)
                            {
                                if( sprite[j].statnum < MAXSTATUS )
                                if( sprite[j].picnum == SECTOREFFECTOR &&
                                    sprite[j].lotag == 1 &&
                                    sprite[j].hitag == sp->hitag)
                                {
                                    if( sp->ang == 512 )
                                    {
                                        sp->x = sprite[j].x;
                                        sp->y = sprite[j].y;
                                    }
                                    break;
                                }
                            }
                            if(j == MAXSPRITES)
                            {
                                sprintf(text,"Found lonely Sector Effector (lotag 0) at (%d,%d)\n",sp->x,sp->y);
                                gameexit(text);
                            }
                            sp->owner = j;
                        }

                        startwall = sector[sect].wallptr;
                        endwall = startwall+sector[sect].wallnum;

                        T2 = tempwallptr;
                        for(s=startwall;s<endwall;s++)
                        {
                            msx[tempwallptr] = wall[s].x-sp->x;
                            msy[tempwallptr] = wall[s].y-sp->y;
                            tempwallptr++;
                            if(tempwallptr > 2047)
                            {
                                sprintf(text,"Too many moving sectors at (%d,%d).\n",wall[s].x,wall[s].y);
                                gameexit(text);
                            }
                        }
                        if( sp->lotag == 30 || sp->lotag == 6 || sp->lotag == 14 || sp->lotag == 5 )
                        {

                            startwall = sector[sect].wallptr;
                            endwall = startwall+sector[sect].wallnum;

                            if(sector[sect].hitag == -1)
                                sp->extra = 0;
                            else sp->extra = 1;

                            sector[sect].hitag = i;

                            j = 0;

                            for(s=startwall;s<endwall;s++)
                            {
                                if( wall[ s ].nextsector >= 0 &&
                                    sector[ wall[ s ].nextsector].hitag == 0 &&
                                        sector[ wall[ s ].nextsector].lotag < 3 )
                                    {
                                        s = wall[s].nextsector;
                                        j = 1;
                                        break;
                                    }
                            }

                            if(j == 0)
                            {
                                sprintf(text,"Subway found no zero'd sectors with locators\nat (%d,%d).\n",sp->x,sp->y);
                                gameexit(text);
                            }

                            sp->owner = -1;
                            T1 = s;

                            if(sp->lotag != 30)
                                T4 = sp->hitag;
                        }

                        else if(sp->lotag == 16)
                            T4 = sector[sect].ceilingz;

                        else if( sp->lotag == 26 )
                        {
                            T4 = sp->x;
                            T5 = sp->y;
                            if(sp->shade==sector[sect].floorshade) //UP
                                sp->zvel = -256;
                            else
                                sp->zvel = 256;

                            sp->shade = 0;
                        }
                        else if( sp->lotag == 2)
                        {
                            T6 = sector[sp->sectnum].floorheinum;
                            sector[sp->sectnum].floorheinum = 0;
                        }
                }

                switch(sp->lotag)
                {
                    case 6:
                    case 14:
                        j = callsound(sect,i);
                        if(j == -1) j = SUBWAY;
                        hittype[i].lastvx = j;
                    case 30:
                        if(numplayers > 1) break;
                    case 0:
                    case 1:
                    case 5:
                    case 11:
                    case 15:
                    case 16:
                    case 26:
                        setsectinterpolate(i);
                        break;
                }

                switch(sprite[i].lotag)
                {
                    case 40:
                    case 41:
                    case 43:
                    case 44:
                    case 45:
                        changespritestat(i,15);
                        break;
                    default:
                        changespritestat(i,3);
                        break;
                }

                break;


            case SEENINE:
            case OOZFILTER:

                sp->shade = -16;
                if(sp->xrepeat <= 8)
                {
                    sp->cstat = (short)32768;
                    sp->xrepeat=sp->yrepeat=0;
                }
                else sp->cstat = 1+256;
                sp->extra = impact_damage<<2;
                sp->owner = i;

                changespritestat(i,6);
                break;

            case CRACK1:
            case CRACK2:
            case CRACK3:
            case CRACK4:
            case FIREEXT:
                if(sp->picnum == FIREEXT)
                {
                    sp->cstat = 257;
                    sp->extra = impact_damage<<2;
                }
                else
                {
                    sp->cstat |= 17;
                    sp->extra = 1;
                }

                if( ud.multimode < 2 && sp->pal != 0)
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                    break;
                }

                sp->pal = 0;
                sp->owner = i;
                changespritestat(i,6);
                sp->xvel = 8;
                ssp(i,CLIPMASK0);
                break;

            case TOILET:
            case STALL:
                sp->lotag = 1;
                sp->cstat |= 257;
                sp->clipdist = 8;
                sp->owner = i;
                break;
            case CANWITHSOMETHING:
            case CANWITHSOMETHING2:
            case CANWITHSOMETHING3:
            case CANWITHSOMETHING4:
            case RUBBERCAN:
                sp->extra = 0;
            case EXPLODINGBARREL:
            case HORSEONSIDE:
            case FIREBARREL:
            case NUKEBARREL:
            case FIREVASE:
            case NUKEBARRELDENTED:
            case NUKEBARRELLEAKED:
            case WOODENHORSE:

                if(j >= 0)
                    sp->xrepeat = sp->yrepeat = 32;
                sp->clipdist = 72;
                makeitfall(i);
                if(j >= 0)
                    sp->owner = j;
                else sp->owner = i;
            case EGG:
                if( ud.monsters_off == 1 && sp->picnum == EGG )
                {
                    sp->xrepeat = sp->yrepeat = 0;
                    changespritestat(i,5);
                }
                else
                {
                    if(sp->picnum == EGG)
                        sp->clipdist = 24;
                    sp->cstat = 257|(TRAND&4);
                    changespritestat(i,2);
                }
                break;
            case TOILETWATER:
                sp->shade = -16;
                changespritestat(i,6);
                break;
    }
    return i;
}


void animatesprites(int32_t x,int32_t y,short a,int32_t smoothratio)
{
    short i, j, k, p, sect;
    int32_t l, t1,t3,t4;
    spritetype *s,*t;

    for(j=0;j < spritesortcnt; j++)
    {
        t = &tsprite[j];
        i = t->owner;
        s = &sprite[t->owner];

        switch(t->picnum)
        {
            case BLOODPOOL:
            case PUKE:
            case FOOTPRINTS:
            case FOOTPRINTS2:
            case FOOTPRINTS3:
            case FOOTPRINTS4:
                if(t->shade == 127) continue;
                break;
            case RESPAWNMARKERRED:
            case RESPAWNMARKERYELLOW:
            case RESPAWNMARKERGREEN:
                if(ud.marker == 0)
                    t->xrepeat = t->yrepeat = 0;
                continue;
            case CHAIR3:

                k = (((t->ang+3072+128-a)&2047)>>8)&7;
                if(k>4)
                {
                    k = 8-k;
                    t->cstat |= 4;
                }
                else t->cstat &= ~4;
                t->picnum = s->picnum+k;
                break;
            case BLOODSPLAT1:
            case BLOODSPLAT2:
            case BLOODSPLAT3:
            case BLOODSPLAT4:
                if(ud.lockout) t->xrepeat = t->yrepeat = 0;
                else if(t->pal == 6)
                {
                    t->shade = -127;
                    continue;
                }
            case BULLETHOLE:
            case CRACK1:
            case CRACK2:
            case CRACK3:
            case CRACK4:
                t->shade = 16;
                continue;
            case NEON1:
            case NEON2:
            case NEON3:
            case NEON4:
            case NEON5:
            case NEON6:
                continue;
            case GREENSLIME:
            case GREENSLIME+1:
            case GREENSLIME+2:
            case GREENSLIME+3:
            case GREENSLIME+4:
            case GREENSLIME+5:
            case GREENSLIME+6:
            case GREENSLIME+7:
                break;
            default:
                if( ( (t->cstat&16) ) || ( badguy(t) && t->extra > 0) || t->statnum == 10)
                    continue;
        }

        if (sector[t->sectnum].ceilingstat&1)
            l = sector[t->sectnum].ceilingshade;
        else
            l = sector[t->sectnum].floorshade;

        if(l < -127) l = -127;
        if(l > 128) l =  127;
        t->shade = l;
    }


    for(j=0;j < spritesortcnt; j++ )  //Between drawrooms() and drawmasks()
    {                             //is the perfect time to animate sprites
        t = &tsprite[j];
        i = t->owner;
        s = &sprite[i];

        switch(s->picnum)
        {
            case SECTOREFFECTOR:
                if(t->lotag == 27 && ud.recstat == 1)
                {
                    t->picnum = 11+((totalclock>>3)&1);
                    t->cstat |= 128;
                }
                else
                    t->xrepeat = t->yrepeat = 0;
                break;
            case NATURALLIGHTNING:
               t->shade = -127;
               break;
            case FEM1:
            case FEM2:
            case FEM3:
            case FEM4:
            case FEM5:
            case FEM6:
            case FEM7:
            case FEM8:
            case FEM9:
            case FEM10:
            case MAN:
            case MAN2:
            case WOMAN:
            case NAKED1:
            case PODFEM1:
            case FEMMAG1:
            case FEMMAG2:
            case FEMPIC1:
            case FEMPIC2:
            case FEMPIC3:
            case FEMPIC4:
            case FEMPIC5:
            case FEMPIC6:
            case FEMPIC7:
            case BLOODYPOLE:
            case FEM6PAD:
            case STATUE:
            case STATUEFLASH:
            case OOZ:
            case OOZ2:
            case WALLBLOOD1:
            case WALLBLOOD2:
            case WALLBLOOD3:
            case WALLBLOOD4:
            case WALLBLOOD5:
            case WALLBLOOD7:
            case WALLBLOOD8:
            case SUSHIPLATE1:
            case SUSHIPLATE2:
            case SUSHIPLATE3:
            case SUSHIPLATE4:
            case FETUS:
            case FETUSJIB:
            case FETUSBROKE:
            case HOTMEAT:
            case FOODOBJECT16:
            case DOLPHIN1:
            case DOLPHIN2:
            case TOUGHGAL:
            case TAMPON:
            case XXXSTACY:
            case 4946:
            case 4947:
            case 693:
            case 2254:
            case 4560:
            case 4561:
            case 4562:
            case 4498:
            case 4957:
                if(ud.lockout)
                {
                    t->xrepeat = t->yrepeat = 0;
                    continue;
                }
        }

        if( t->statnum == 99 ) continue;
        if( s->statnum != 1 && s->picnum == APLAYER && ps[s->yvel].newowner == -1 && s->owner >= 0 )
        {
            t->x -= mulscale16(65536-smoothratio,ps[s->yvel].posx-ps[s->yvel].oposx);
            t->y -= mulscale16(65536-smoothratio,ps[s->yvel].posy-ps[s->yvel].oposy);
            t->z = ps[s->yvel].oposz + mulscale16(smoothratio,ps[s->yvel].posz-ps[s->yvel].oposz);
            t->z += (40<<8);
        }
        else if( ( s->statnum == 0 && s->picnum != CRANEPOLE) || s->statnum == 10 || s->statnum == 6 || s->statnum == 4 || s->statnum == 5 || s->statnum == 1 )
        {
            t->x -= mulscale16(65536-smoothratio,s->x-hittype[i].bposx);
            t->y -= mulscale16(65536-smoothratio,s->y-hittype[i].bposy);
            t->z -= mulscale16(65536-smoothratio,s->z-hittype[i].bposz);
        }

        sect = s->sectnum;
        t1 = T2;t3 = T4;t4 = T5;

        switch(s->picnum)
        {
            case DUKELYINGDEAD:
                t->z += (24<<8);
                break;
            case BLOODPOOL:
            case FOOTPRINTS:
            case FOOTPRINTS2:
            case FOOTPRINTS3:
            case FOOTPRINTS4:
                if(t->pal == 6)
                    t->shade = -127;
            case PUKE:
            case MONEY:
            case MONEY+1:
            case MAIL:
            case MAIL+1:
            case PAPER:
            case PAPER+1:
                if(ud.lockout && s->pal == 2)
                {
                    t->xrepeat = t->yrepeat = 0;
                    continue;
                }
                break;
            case TRIPBOMB:
                continue;
            case FORCESPHERE:
                if(t->statnum == 5)
                {
                    short sqa,sqb;

                    sqa =
                        getangle(
                            sprite[s->owner].x-ps[screenpeek].posx,
                            sprite[s->owner].y-ps[screenpeek].posy);
                    sqb =
                        getangle(
                            sprite[s->owner].x-t->x,
                            sprite[s->owner].y-t->y);

                    if( klabs(getincangle(sqa,sqb)) > 512 )
                        if( ldist(&sprite[s->owner],t) < ldist(&sprite[ps[screenpeek].i],&sprite[s->owner]) )
                            t->xrepeat = t->yrepeat = 0;
                }
                continue;
            case BURNING:
            case BURNING2:
                if( sprite[s->owner].statnum == 10 )
                {
                    if( display_mirror == 0 && sprite[s->owner].yvel == screenpeek && ps[sprite[s->owner].yvel].over_shoulder_on == 0 )
                        t->xrepeat = 0;
                    else
                    {
                        t->ang = getangle(x-t->x,y-t->y);
                        t->x = sprite[s->owner].x;
                        t->y = sprite[s->owner].y;
                        t->x += sintable[(t->ang+512)&2047]>>10;
                        t->y += sintable[t->ang&2047]>>10;
                    }
                }
                break;

            case ATOMICHEALTH:
                t->z -= (4<<8);
                break;
            case CRYSTALAMMO:
                t->shade = (sintable[(totalclock<<4)&2047]>>10);
                continue;
            case VIEWSCREEN:
            case VIEWSCREEN2:
                if(camsprite >= 0 && hittype[OW].temp_data[0] == 1)
                {
                    t->picnum = STATIC;
                    t->cstat |= (rand()&12);
                    t->xrepeat += 8;
                    t->yrepeat += 8;
                }
                break;

            case SHRINKSPARK:
                t->picnum = SHRINKSPARK+( (totalclock>>4)&3 );
                break;
            case GROWSPARK:
                t->picnum = GROWSPARK+( (totalclock>>4)&3 );
                break;
            case RPG:
                 k = getangle(s->x-x,s->y-y);
                 k = (((s->ang+3072+128-k)&2047)/170);
                 if(k > 6)
                 {
                    k = 12-k;
                    t->cstat |= 4;
                 }
                 else t->cstat &= ~4;
                 t->picnum = RPG+k;
                 break;

            case RECON:

                k = getangle(s->x-x,s->y-y);
                if( T1 < 4 )
                    k = (((s->ang+3072+128-k)&2047)/170);
                else k = (((s->ang+3072+128-k)&2047)/170);

                if(k>6)
                {
                    k = 12-k;
                    t->cstat |= 4;
                }
                else t->cstat &= ~4;

                if( klabs(t3) > 64 ) k += 7;
                t->picnum = RECON+k;

                break;

            case APLAYER:

                p = s->yvel;

                if(t->pal == 1) t->z -= (18<<8);

                if(ps[p].over_shoulder_on > 0 && ps[p].newowner < 0 )
                {
                    t->cstat |= 2;
                    if ( screenpeek == myconnectindex && numplayers >= 2 )
                    {
                        t->x = omyx+mulscale16((int32_t)(myx-omyx),smoothratio);
                        t->y = omyy+mulscale16((int32_t)(myy-omyy),smoothratio);
                        t->z = omyz+mulscale16((int32_t)(myz-omyz),smoothratio)+(40<<8);
                        t->ang = omyang+mulscale16((int32_t)(((myang+1024-omyang)&2047)-1024),smoothratio);
                        t->sectnum = mycursectnum;
                    }
                }

                if( ( display_mirror == 1 || screenpeek != p || s->owner == -1 ) && ud.multimode > 1 && ud.showweapons && sprite[ps[p].i].extra > 0 && ps[p].curr_weapon > 0 )
                {
                    memcpy((spritetype *)&tsprite[spritesortcnt],(spritetype *)t,sizeof(spritetype));

                    tsprite[spritesortcnt].statnum = 99;

                    tsprite[spritesortcnt].yrepeat = ( t->yrepeat>>3 );
                    if(t->yrepeat < 4) t->yrepeat = 4;

                    tsprite[spritesortcnt].shade = t->shade;
                    tsprite[spritesortcnt].cstat = 0;

                    switch(ps[p].curr_weapon)
                    {
                        case PISTOL_WEAPON:      tsprite[spritesortcnt].picnum = FIRSTGUNSPRITE;       break;
                        case SHOTGUN_WEAPON:     tsprite[spritesortcnt].picnum = SHOTGUNSPRITE;        break;
                        case CHAINGUN_WEAPON:    tsprite[spritesortcnt].picnum = CHAINGUNSPRITE;       break;
                        case RPG_WEAPON:         tsprite[spritesortcnt].picnum = RPGSPRITE;            break;
                        case HANDREMOTE_WEAPON:
                        case HANDBOMB_WEAPON:    tsprite[spritesortcnt].picnum = HEAVYHBOMB;           break;
                        case TRIPBOMB_WEAPON:    tsprite[spritesortcnt].picnum = TRIPBOMBSPRITE;       break;
                        case GROW_WEAPON:        tsprite[spritesortcnt].picnum = GROWSPRITEICON;       break;
                        case SHRINKER_WEAPON:    tsprite[spritesortcnt].picnum = SHRINKERSPRITE;       break;
                        case FREEZE_WEAPON:      tsprite[spritesortcnt].picnum = FREEZESPRITE;         break;
                        case DEVISTATOR_WEAPON:  tsprite[spritesortcnt].picnum = DEVISTATORSPRITE;     break;
                    }

                    if(s->owner >= 0)
                        tsprite[spritesortcnt].z = ps[p].posz-(12<<8);
                    else tsprite[spritesortcnt].z = s->z-(51<<8);
                    if(ps[p].curr_weapon == HANDBOMB_WEAPON)
                    {
                        tsprite[spritesortcnt].xrepeat = 10;
                        tsprite[spritesortcnt].yrepeat = 10;
                    }
                    else
                    {
                        tsprite[spritesortcnt].xrepeat = 16;
                        tsprite[spritesortcnt].yrepeat = 16;
                    }
                    tsprite[spritesortcnt].pal = 0;
                    spritesortcnt++;
                }

                if(s->owner == -1)
                {
                    k = (((s->ang+3072+128-a)&2047)>>8)&7;
                    if(k>4)
                    {
                        k = 8-k;
                        t->cstat |= 4;
                    }
                    else t->cstat &= ~4;

                    if(sector[t->sectnum].lotag == 2) k += 1795-1405;
                    else if( (hittype[i].floorz-s->z) > (64<<8) ) k += 60;

                    t->picnum += k;
                    t->pal = ps[p].palookup;

                    goto PALONLY;
                }

                if( ps[p].on_crane == -1 && (sector[s->sectnum].lotag&0x7ff) != 1 )
                {
                    l = s->z-hittype[ps[p].i].floorz+(3<<8);
                    if( l > 1024 && s->yrepeat > 32 && s->extra > 0 )
                        s->yoffset = (int8_t  )(l/(s->yrepeat<<2));
                    else s->yoffset=0;
                }

                if(ps[p].newowner > -1)
                {
                    t4 = *(actorscrptr[APLAYER]+1);
                    t3 = 0;
                    t1 = *(actorscrptr[APLAYER]+2);
                }

                if(ud.camerasprite == -1 && ps[p].newowner == -1)
                    if(s->owner >= 0 && display_mirror == 0 && ps[p].over_shoulder_on == 0 )
                        if( ud.multimode < 2 || ( ud.multimode > 1 && p == screenpeek ) )
                {
                    t->owner = -1;
                    t->xrepeat = t->yrepeat = 0;
                    continue;
                }

                PALONLY:

                if( sector[sect].floorpal )
                    t->pal = sector[sect].floorpal;

                if(s->owner == -1) continue;

                if( t->z > hittype[i].floorz && t->xrepeat < 32 )
                    t->z = hittype[i].floorz;

                break;

            case JIBS1:
            case JIBS2:
            case JIBS3:
            case JIBS4:
            case JIBS5:
            case JIBS6:
            case HEADJIB1:
            case LEGJIB1:
            case ARMJIB1:
            case LIZMANHEAD1:
            case LIZMANARM1:
            case LIZMANLEG1:
            case DUKELEG:
            case DUKEGUN:
            case DUKETORSO:
                if(ud.lockout)
                {
                    t->xrepeat = t->yrepeat = 0;
                    continue;
                }
                if(t->pal == 6) t->shade = -120;

            case SCRAP1:
            case SCRAP2:
            case SCRAP3:
            case SCRAP4:
            case SCRAP5:
            case SCRAP6:
            case SCRAP6+1:
            case SCRAP6+2:
            case SCRAP6+3:
            case SCRAP6+4:
            case SCRAP6+5:
            case SCRAP6+6:
            case SCRAP6+7:

                if(hittype[i].picnum == BLIMP && t->picnum == SCRAP1 && s->yvel >= 0)
                    t->picnum = s->yvel;
                else t->picnum += T1;
                t->shade -= 6;

                if( sector[sect].floorpal )
                    t->pal = sector[sect].floorpal;
                break;

            case WATERBUBBLE:
                if(sector[t->sectnum].floorpicnum == FLOORSLIME)
                {
                    t->pal = 7;
                    break;
                }
            default:

                if( sector[sect].floorpal )
                    t->pal = sector[sect].floorpal;
                break;
        }

        if( actorscrptr[s->picnum] )
        {
            if(t4>10000)
				// FIX_00093: fixed crashbugs in multiplayer (mine/blimp)
				// This is the mine issue (confusion bug in hittype[i].temp_data[4] usage)
				// close to blimp bug (search for BLIMP)
				// -> t4 aka macro T5 is incremented at DETONATEB: in actor.c
				// for a time counter. Instead we want an address.
				// Issue happens in confessn.map (do a dnclip + dnkroz + dncoords,
				// start with duke3d_w32 /m /q2 -map confessn.map)
				// go through the Guilty logo till x = -2932, y = 42174, z = 18416.
				// blow up the bomb. Wait in the water. Look at the respawn sign
				// at the bottom of the chain. Crashes when it's about to respawn.
				// Lame fix. ok for w32. Doesn't work for other plateform.
				// How to make a differene between a timer and an address??
            {
                l = *(int32_t *)(t4+8);

                switch( l )
                {
                    case 2:
                        k = (((s->ang+3072+128-a)&2047)>>8)&1;
                        break;

                    case 3:
                    case 4:
                        k = (((s->ang+3072+128-a)&2047)>>7)&7;
                        if(k > 3)
                        {
                            t->cstat |= 4;
                            k = 7-k;
                        }
                        else t->cstat &= ~4;
                        break;

                    case 5:
                        k = getangle(s->x-x,s->y-y);
                        k = (((s->ang+3072+128-k)&2047)>>8)&7;
                        if(k>4)
                        {
                            k = 8-k;
                            t->cstat |= 4;
                        }
                        else t->cstat &= ~4;
                        break;
                    case 7:
                        k = getangle(s->x-x,s->y-y);
                        k = (((s->ang+3072+128-k)&2047)/170);
                        if(k>6)
                        {
                            k = 12-k;
                            t->cstat |= 4;
                        }
                        else t->cstat &= ~4;
                        break;
                    case 8:
                        k = (((s->ang+3072+128-a)&2047)>>8)&7;
                        t->cstat &= ~4;
                        break;
                    default:
                        k = 0;
                        break;
                }

                t->picnum += k + ( *(int32_t *)t4 ) + l * t3;

                if(l > 0)
                    while(tiles[t->picnum].dim.width == 0 && t->picnum > 0 )
                    t->picnum -= l;       //Hack, for actors

                if( hittype[i].dispicnum >= 0)
                    hittype[i].dispicnum = t->picnum;
            }
            else if(display_mirror == 1)
                t->cstat |= 4;
        }

        if( s->statnum == 13 || badguy(s) || (s->picnum == APLAYER && s->owner >= 0) )
            if(t->statnum != 99 && s->picnum != EXPLOSION2 && s->picnum != HANGLIGHT && s->picnum != DOMELITE)
                if(s->picnum != HOTMEAT)
        {
            if( hittype[i].dispicnum < 0 )
            {
                hittype[i].dispicnum++;
                continue;
            }
            else if( ud.shadows && spritesortcnt < (MAXSPRITESONSCREEN-2))
            {
                int32_t daz,xrep,yrep;

                if( (sector[sect].lotag&0xff) > 2 || s->statnum == 4 || s->statnum == 5 || s->picnum == DRONE || s->picnum == COMMANDER )
                    daz = sector[sect].floorz;
                else
                    daz = hittype[i].floorz;

                if( (s->z-daz) < (8<<8) )
                    if( ps[screenpeek].posz < daz )
                {
                    memcpy((spritetype *)&tsprite[spritesortcnt],(spritetype *)t,sizeof(spritetype));

                    tsprite[spritesortcnt].statnum = 99;

                    tsprite[spritesortcnt].yrepeat = ( t->yrepeat>>3 );
                    if(t->yrepeat < 4) t->yrepeat = 4;

                    tsprite[spritesortcnt].shade = 127;
                    tsprite[spritesortcnt].cstat |= 2;

                    tsprite[spritesortcnt].z = daz;
                    xrep = tsprite[spritesortcnt].xrepeat;// - (klabs(daz-t->z)>>11);
                    tsprite[spritesortcnt].xrepeat = xrep;
                    tsprite[spritesortcnt].pal = 4;

                    yrep = tsprite[spritesortcnt].yrepeat;// - (klabs(daz-t->z)>>11);
                    tsprite[spritesortcnt].yrepeat = yrep;
                    spritesortcnt++;
                }
            }

            if( ps[screenpeek].heat_amount > 0 && ps[screenpeek].heat_on )
            {
                t->pal = 6;
                t->shade = 0;
            }
        }


        switch(s->picnum)
        {
            case LASERLINE:
                if(sector[t->sectnum].lotag == 2) t->pal = 8;
                t->z = sprite[s->owner].z-(3<<8);
                if(lasermode == 2 && ps[screenpeek].heat_on == 0 )
                    t->yrepeat = 0;
            case EXPLOSION2:
            case EXPLOSION2BOT:
            case FREEZEBLAST:
            case ATOMICHEALTH:
            case FIRELASER:
            case SHRINKSPARK:
            case GROWSPARK:
            case CHAINGUN:
            case SHRINKEREXPLOSION:
            case RPG:
            case FLOORFLAME:
                if(t->picnum == EXPLOSION2)
                {
                    ps[screenpeek].visibility = -127;
                    lastvisinc = totalclock+32;
                    restorepalette = 1;
                }
                t->shade = -127;
                break;
            case FIRE:
            case FIRE2:
            case BURNING:
            case BURNING2:
                if( sprite[s->owner].picnum != TREE1 && sprite[s->owner].picnum != TREE2 )
                    t->z = sector[t->sectnum].floorz;
                t->shade = -127;
                break;
            case COOLEXPLOSION1:
                t->shade = -127;
                t->picnum += (s->shade>>1);
                break;
            case PLAYERONWATER:

                k = (((t->ang+3072+128-a)&2047)>>8)&7;
                if(k>4)
                {
                    k = 8-k;
                    t->cstat |= 4;
                }
                else t->cstat &= ~4;

                t->picnum = s->picnum+k+((T1<4)*5);
                t->shade = sprite[s->owner].shade;

                break;

            case WATERSPLASH2:
                t->picnum = WATERSPLASH2+t1;
                break;
            case REACTOR2:
                t->picnum = s->picnum + T3;
                break;
            case SHELL:
                t->picnum = s->picnum+(T1&1);
            case SHOTGUNSHELL:
                t->cstat |= 12;
                if(T1 > 1) t->cstat &= ~4;
                if(T1 > 2) t->cstat &= ~12;
                break;
            case FRAMEEFFECT1:
			case FRAMEEFFECT1_13CON:
			if(s->owner >= 0 && sprite[s->owner].statnum < MAXSTATUS)
                {
                    if(sprite[s->owner].picnum == APLAYER)
                        if(ud.camerasprite == -1)
                            if(screenpeek == sprite[s->owner].yvel && display_mirror == 0)
                    {
                        t->owner = -1;
                        break;
                    }
                    if( (sprite[s->owner].cstat&32768) == 0 )
                    {
                        t->picnum = hittype[s->owner].dispicnum;
                        t->pal = sprite[s->owner].pal;
                        t->shade = sprite[s->owner].shade;
                        t->ang = sprite[s->owner].ang;
                        t->cstat = 2|sprite[s->owner].cstat;
                    }
                }
                break;

            case CAMERA1:
            case RAT:
                k = (((t->ang+3072+128-a)&2047)>>8)&7;
                if(k>4)
                {
                    k = 8-k;
                    t->cstat |= 4;
                }
                else t->cstat &= ~4;
                t->picnum = s->picnum+k;
                break;
        }

        hittype[i].dispicnum = t->picnum;
        if(sector[t->sectnum].floorpicnum == MIRROR)
            t->xrepeat = t->yrepeat = 0;
    }
}



#define NUMCHEATCODES 26
const uint8_t  cheatquotes[NUMCHEATCODES][14] = {
    {"cornholio"},	// 0
    {"stuff"},		// 1
    {"scotty###"},	// 2
    {"coords"},		// 3
    {"view"},		// 4
    {"time"},		// 5
    {"unlock"},		// 6
    {"cashman"},	// 7
    {"items"},		// 8
    {"rate"},		// 9
    {"skill#"},		// 10
    {"beta"},		// 11
    {"hyper"},		// 12
    {"monsters"},	// 13
    {"<RESERVED>"},	// 14
    {"<RESERVED>"},	// 15
    {"todd"},		// 16
    {"showmap"},	// 17
    {"kroz"},		// 18
    {"allen"},		// 19
    {"clip"},		// 20
    {"weapons"},	// 21
    {"inventory"},	// 22
    {"keys"},		// 23
    {"debug"}		// 24
//    {"ending"}

};


uint8_t  cheatbuf[10],cheatbuflen;
void cheats(void)
{
    short ch, i, j, k, weapon;

    if( (ps[myconnectindex].gm&MODE_TYPE) || (ps[myconnectindex].gm&MODE_MENU))
        return;

    if ( ps[myconnectindex].cheat_phase == 1)
    {
       while (KB_KeyWaiting())
       {
          ch = KB_Getch();
          ch = tolower(ch);

          if( !( (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ) )
          {
             ps[myconnectindex].cheat_phase = 0;
//             FTA(46,&ps[myconnectindex]);
             return;
          }

          cheatbuf[cheatbuflen++] = ch;
          cheatbuf[cheatbuflen] = 0;

          if(cheatbuflen > 11)
          {
              ps[myconnectindex].cheat_phase = 0;
              return;
          }

          for(k = 0;k < NUMCHEATCODES;k++)
          {
              for(j = 0;j<cheatbuflen;j++)
              {
                  if( cheatbuf[j] == cheatquotes[k][j] || (cheatquotes[k][j] == '#' && ch >= '0' && ch <= '9') )
                  {
                      if( cheatquotes[k][j+1] == 0 ) goto FOUNDCHEAT;
                      if(j == cheatbuflen-1) return;
                  }
                  else break;
              }
          }

          ps[myconnectindex].cheat_phase = 0;
          return;

          FOUNDCHEAT:
          {
                switch(k)
                {
                    case 0: // cornholio
                    case 18: // kroz

                        ud.god = 1-ud.god;

                        if(ud.god)
                        { // set on
                            pus = 1;
                            pub = 1;
                            sprite[ps[myconnectindex].i].cstat = 257;

                            hittype[ps[myconnectindex].i].temp_data[0] = 0;
                            hittype[ps[myconnectindex].i].temp_data[1] = 0;
                            hittype[ps[myconnectindex].i].temp_data[2] = 0;
                            hittype[ps[myconnectindex].i].temp_data[3] = 0;
                            hittype[ps[myconnectindex].i].temp_data[4] = 0;
                            hittype[ps[myconnectindex].i].temp_data[5] = 0;

                            sprite[ps[myconnectindex].i].hitag = 0;
                            sprite[ps[myconnectindex].i].lotag = 0;
                            sprite[ps[myconnectindex].i].pal =
                                ps[myconnectindex].palookup;

                            FTA(17,&ps[myconnectindex],1);
                        }
                        else // set off
                        {
                            ud.god = 0;
                            sprite[ps[myconnectindex].i].extra = max_player_health;
                            hittype[ps[myconnectindex].i].extra = -1;
                            ps[myconnectindex].last_extra = max_player_health;
                            FTA(18,&ps[myconnectindex],1);
                        }

                        sprite[ps[myconnectindex].i].extra = max_player_health;
                        hittype[ps[myconnectindex].i].extra = 0;
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();

                        return;

                    case 1: // stuff

						if(VOLUMEONE)
                        	j = 6;
						else
                        	j = 0;

                        for ( weapon = PISTOL_WEAPON;weapon < MAX_WEAPONS-j;weapon++ )
                           ps[myconnectindex].gotweapon[weapon]  = 1;

                        for ( weapon = PISTOL_WEAPON;
                              weapon < (MAX_WEAPONS-j);
                              weapon++ )
                            addammo( weapon, &ps[myconnectindex], max_ammo_amount[weapon] );

                        ps[myconnectindex].ammo_amount[GROW_WEAPON] = 50;

                        ps[myconnectindex].steroids_amount =         400;
                        ps[myconnectindex].heat_amount     =        1200;
                        ps[myconnectindex].boot_amount          =    200;
                        ps[myconnectindex].shield_amount =           100;
                        ps[myconnectindex].scuba_amount =            6400;
                        ps[myconnectindex].holoduke_amount =         2400;
                        ps[myconnectindex].jetpack_amount =          1600;
                        ps[myconnectindex].firstaid_amount =         max_player_health;

                        ps[myconnectindex].got_access =              7;
                        FTA(5,&ps[myconnectindex],1);
                        ps[myconnectindex].cheat_phase = 0;

                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].inven_icon = 1;
                        return;

                    case 2:  // dnscotty###
                    case 10: // skill#

                        if(k == 2)
                        {
                            short volnume,levnume;
                            volnume = cheatbuf[6] - '0';
                            levnume = (cheatbuf[7] - '0')*10+(cheatbuf[8]-'0');

                            volnume--;
                            levnume--;
							if (VOLUMEONE)
							{
								if( volnume > 0 )
	                            {
	                                ps[myconnectindex].cheat_phase = 0;
	                                KB_FlushKeyboardQueue();
	                                return;
	                            }
							}

                            if((volnume > 4)&&PLUTOPAK)
                            {
                                ps[myconnectindex].cheat_phase = 0;
                                KB_FlushKeyboardQueue();
                                return;
                            }
                            else

							if((volnume > 3)&&!PLUTOPAK)
                            {
                                ps[myconnectindex].cheat_phase = 0;
                                KB_FlushKeyboardQueue();
                                return;
                            }
                            else

                            if(volnume == 0)
                            {
                                if(levnume > 5)
                                {
                                    ps[myconnectindex].cheat_phase = 0;
                                    KB_FlushKeyboardQueue();
                                    return;
                                }
                            }
                            else
                            {
                                if(levnume >= 11)
                                {
                                    ps[myconnectindex].cheat_phase = 0;
                                    KB_FlushKeyboardQueue();
                                    return;
                                }
                            }

                            ud.m_volume_number = ud.volume_number = volnume;
                            ud.m_level_number = ud.level_number = levnume;

                        }
                        else ud.m_player_skill = ud.player_skill =
                            cheatbuf[5] - '1';

                        if(numplayers > 1 && myconnectindex == connecthead)
                        {
                            tempbuf[0] = 5;
                            tempbuf[1] = ud.m_level_number;
                            tempbuf[2] = ud.m_volume_number;
                            tempbuf[3] = ud.m_player_skill;
                            tempbuf[4] = ud.m_monsters_off;
                            tempbuf[5] = ud.m_respawn_monsters;
                            tempbuf[6] = ud.m_respawn_items;
                            tempbuf[7] = ud.m_respawn_inventory;
                            tempbuf[8] = ud.m_coop;
                            tempbuf[9] = ud.m_marker;
                            tempbuf[10] = ud.m_ffire;

                            for(i=connecthead;i>=0;i=connectpoint2[i])
                                sendpacket(i,(uint8_t*)tempbuf,11);
                        }
                        else ps[myconnectindex].gm |= MODE_RESTART;

                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 3: // coords
                        ps[myconnectindex].cheat_phase = 0;
                        ud.coords = 1-ud.coords;
                        KB_FlushKeyboardQueue();
                        return;

                    case 4: // view
                        if( ps[myconnectindex].over_shoulder_on )
                            ps[myconnectindex].over_shoulder_on = 0;
                        else
                        {
                            ps[myconnectindex].over_shoulder_on = 1;
                            cameradist = 0;
                            cameraclock = totalclock;
                        }
                        // FTA(22,&ps[myconnectindex],1);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 5: // time
                        // FTA(21,&ps[myconnectindex]);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

					case 6: // unlock
                        for(i=numsectors-1;i>=0;i--) //Unlock
                        {
                            j = sector[i].lotag;
                            if(j == -1 || j == 32767) continue;
                            if( (j & 0x7fff) > 2 )
                            {
                                if( j&(0xffff-16384) )
                                    sector[i].lotag &= (0xffff-16384);
                                operatesectors(i,ps[myconnectindex].i);
                            }
                        }
                        operateforcefields(ps[myconnectindex].i,-1);

                        FTA(100,&ps[myconnectindex],1);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 7: // cashman
                        ud.cashman = 1-ud.cashman;
                        KB_ClearKeyDown(sc_N);
                        ps[myconnectindex].cheat_phase = 0;
                        return;

                    case 8: // items
                        ps[myconnectindex].steroids_amount =         400;
                        ps[myconnectindex].heat_amount     =        1200;
                        ps[myconnectindex].boot_amount          =    200;
                        ps[myconnectindex].shield_amount =           100;
                        ps[myconnectindex].scuba_amount =            6400;
                        ps[myconnectindex].holoduke_amount =         2400;
                        ps[myconnectindex].jetpack_amount =          1600;

                        ps[myconnectindex].firstaid_amount =         max_player_health;
                        ps[myconnectindex].got_access =              7;
                        FTA(5,&ps[myconnectindex],1);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 9: // rate
                        ud.tickrate ^= 1;
						vscrn(); // FIX_00056: Refresh issue w/FPS, small Weapon and custom FTA, when screen resized down
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 11: // beta
                        FTA(105,&ps[myconnectindex],1);
                        KB_ClearKeyDown(sc_H);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 12: // hyper
                        ps[myconnectindex].steroids_amount = 399;
                        ps[myconnectindex].heat_amount = 1200;
                        ps[myconnectindex].cheat_phase = 0;
                        FTA(37,&ps[myconnectindex],1);
                        KB_FlushKeyboardQueue();
                        return;

                    case 13: // monsters
                        if(actor_tog == 3) actor_tog = 0;
                        actor_tog++;
                        ps[screenpeek].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 14: // <RESERVED>
                    case 25: // ??
                        ud.eog = 1;
                        ps[myconnectindex].gm |= MODE_EOL;
                        KB_FlushKeyboardQueue();
                        return;

                    case 15: // <RESERVED>
                        ps[myconnectindex].gm = MODE_EOL;
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 16: // todd
                        FTA(99,&ps[myconnectindex],1);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                   case 17: // showmap
                        ud.showallmap = 1-ud.showallmap;
                        if(ud.showallmap)
                        {
                            for(i=0;i<(MAXSECTORS>>3);i++)
                                show2dsector[i] = 255;
                            for(i=0;i<(MAXWALLS>>3);i++)
                                show2dwall[i] = 255;
                            FTA(111,&ps[myconnectindex],1);
                        }
                        else
                        {
                            for(i=0;i<(MAXSECTORS>>3);i++)
                                show2dsector[i] = 0;
                            for(i=0;i<(MAXWALLS>>3);i++)
                                show2dwall[i] = 0;
                            FTA(1,&ps[myconnectindex],1);
                        }
                        ps[myconnectindex].cheat_phase = 0;
                        KB_FlushKeyboardQueue();
                        return;

                    case 19: // allen
                        FTA(79,&ps[myconnectindex],1);
                        ps[myconnectindex].cheat_phase = 0;
                        KB_ClearKeyDown(sc_N);
                        return;

					case 20: // clip
                        ud.clipping = 1-ud.clipping;
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        FTA(112+ud.clipping,&ps[myconnectindex],1);
                        return;

					case 21: // weapons
						if(VOLUMEONE)
                        	j = 6;
						else
                        	j = 0;

                        for ( weapon = PISTOL_WEAPON;weapon < MAX_WEAPONS-j;weapon++ )
                        {
                            addammo( weapon, &ps[myconnectindex], max_ammo_amount[weapon] );
                            ps[myconnectindex].gotweapon[weapon]  = 1;
                        }

                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        FTA(119,&ps[myconnectindex],1);
                        return;

                    case 22: // inventory
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        ps[myconnectindex].steroids_amount =         400;
                        ps[myconnectindex].heat_amount     =        1200;
                        ps[myconnectindex].boot_amount          =    200;
                        ps[myconnectindex].shield_amount =           100;
                        ps[myconnectindex].scuba_amount =            6400;
                        ps[myconnectindex].holoduke_amount =         2400;
                        ps[myconnectindex].jetpack_amount =          1600;
                        ps[myconnectindex].firstaid_amount =         max_player_health;
                        FTA(120,&ps[myconnectindex],1);
                        ps[myconnectindex].cheat_phase = 0;
                        return;

                    case 23: // keys
                        ps[myconnectindex].got_access =              7;
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        FTA(121,&ps[myconnectindex],1);
                        return;

                    case 24: // debug
                        debug_on = 1-debug_on;
                        KB_FlushKeyboardQueue();
                        ps[myconnectindex].cheat_phase = 0;
                        break;
                }
             }
          }
       }

    else
    {
        if( KB_KeyPressed(sc_D) )
        {
            if( ps[myconnectindex].cheat_phase >= 0 && numplayers < 2 && ud.recstat == 0)
                ps[myconnectindex].cheat_phase = -1;
        }

        if( KB_KeyPressed(sc_N) )
        {
            if( ps[myconnectindex].cheat_phase == -1 )
            {
                if(ud.player_skill == 4)
                {
                    FTA(22,&ps[myconnectindex],1);
                    ps[myconnectindex].cheat_phase = 0;
                }
                else
                {
                    ps[myconnectindex].cheat_phase = 1;
//                    FTA(25,&ps[myconnectindex]);
                    cheatbuflen = 0;
                }
                KB_FlushKeyboardQueue();
            }
            else if(ps[myconnectindex].cheat_phase != 0)
            {
                ps[myconnectindex].cheat_phase = 0;
                KB_ClearKeyDown(sc_D);
                KB_ClearKeyDown(sc_N);
            }
        }
    }
}


int32_t nonsharedtimer;
void nonsharedkeys(void)
{
    short i,ch;
    int32_t j;
    char text[512];

    if(ud.recstat == 2)
    {
        ControlInfo noshareinfo;
        CONTROL_GetInput( &noshareinfo );
    }

    if( KB_KeyPressed( sc_F12 ) )
    {
        KB_ClearKeyDown( sc_F12 );
		takescreenshot();
        // FTA(103,&ps[myconnectindex]); done better in takescreenshot()
    }

    if( !ALT_IS_PRESSED && ud.overhead_on == 0)
        {
            if( ACTION( gamefunc_Enlarge_Screen ) )
            {
                CONTROL_ClearAction( gamefunc_Enlarge_Screen );
                if(ud.screen_size > 0)
                    sound(THUD);

				// FIX_00027: Added an extra small statusbar (HUD)
				if (ud.screen_size==4)
				{
					ud.extended_screen_size++;
					if(ud.extended_screen_size==2)
					{
						ud.extended_screen_size = 1;
						ud.screen_size -= 4;
					}
				}
				else
	                ud.screen_size -= 4;
                vscrn();
            }
            if( ACTION( gamefunc_Shrink_Screen ) )
            {
                CONTROL_ClearAction( gamefunc_Shrink_Screen );
                if(ud.screen_size < 64) sound(THUD);

				// FIX_00027: Added an extra small statusbar (HUD)
				if (ud.screen_size==4)
				{
					ud.extended_screen_size--;
					if(ud.extended_screen_size<0)
					{
						ud.extended_screen_size=0;
						ud.screen_size += 4;
					}
				}
				else
	                ud.screen_size += 4;
                vscrn();
            }

			if(ud.screen_size < 4)
				ud.extended_screen_size = 1;
			else if(ud.screen_size > 4)
				ud.extended_screen_size = 0;

        }

    if( ps[myconnectindex].cheat_phase == 1 || ps[myconnectindex].gm&(MODE_MENU|MODE_TYPE)) return;

    if( ACTION(gamefunc_See_Coop_View) && ( ud.coop == 1 || ud.recstat == 2) )
    {
        CONTROL_ClearAction( gamefunc_See_Coop_View );
        screenpeek = connectpoint2[screenpeek];
        if(screenpeek == -1) screenpeek = connecthead;
        restorepalette = 1;
    }

    if( ud.multimode > 1 && ACTION(gamefunc_Show_Opponents_Weapon) )
    {
        CONTROL_ClearAction(gamefunc_Show_Opponents_Weapon);
        ud.showweapons = 1-ud.showweapons;
        FTA(82-ud.showweapons,&ps[screenpeek],1);
    }

    if( ACTION(gamefunc_Toggle_Crosshair) )
    {
        CONTROL_ClearAction(gamefunc_Toggle_Crosshair);
        ud.crosshair = 1-ud.crosshair;
        FTA(21-ud.crosshair,&ps[screenpeek],1);
    }

    if(ud.overhead_on && ACTION(gamefunc_Map_Follow_Mode) )
    {
        CONTROL_ClearAction(gamefunc_Map_Follow_Mode);
        ud.scrollmode = 1-ud.scrollmode;
        if(ud.scrollmode)
        {
            ud.folx = ps[screenpeek].oposx;
            ud.foly = ps[screenpeek].oposy;
            ud.fola = ps[screenpeek].oang;
        }
        FTA(83+ud.scrollmode,&ps[myconnectindex],1);
    }

    if( SHIFTS_IS_PRESSED || ALT_IS_PRESSED )
    {
        i = 0;
        if( KB_KeyPressed( sc_F1) ) { KB_ClearKeyDown(sc_F1);i = 1; }
        if( KB_KeyPressed( sc_F2) ) { KB_ClearKeyDown(sc_F2);i = 2; }
        if( KB_KeyPressed( sc_F3) ) { KB_ClearKeyDown(sc_F3);i = 3; }
        if( KB_KeyPressed( sc_F4) ) { KB_ClearKeyDown(sc_F4);i = 4; }
        if( KB_KeyPressed( sc_F5) ) { KB_ClearKeyDown(sc_F5);i = 5; }
        if( KB_KeyPressed( sc_F6) ) { KB_ClearKeyDown(sc_F6);i = 6; }
        if( KB_KeyPressed( sc_F7) ) { KB_ClearKeyDown(sc_F7);i = 7; }
        if( KB_KeyPressed( sc_F8) ) { KB_ClearKeyDown(sc_F8);i = 8; }
        if( KB_KeyPressed( sc_F9) ) { KB_ClearKeyDown(sc_F9);i = 9; }
        if( KB_KeyPressed( sc_F10) ) {KB_ClearKeyDown(sc_F10);i = 10; }

        if(i)
        {
            if(SHIFTS_IS_PRESSED)
            {
                if(i == 5 && ps[myconnectindex].fta > 0 && ps[myconnectindex].ftq == 26)
                {
                    music_select++;

					// FIX_00065: Music cycling with F5 and SHIFT-F5 messed up
					if(VOLUMEALL) // Then its 1.3d reg
					{
						if(music_select == 33) music_select = 0;
					}
					else if (VOLUMEONE)
					{
						if(music_select == 6) music_select = 0;
					}
					else // assume 1.5 or plutopak
					{
						if(music_select == 44) music_select = 0;
					}

                    strcpy(text,"PLAYING ");
                    strcat(text,&music_fn[0][music_select][0]);
					MUSIC_StopSong(); // FIX_00074: Shift f5 doesn't change hi-res tunes, but only midi tunes.
                    playmusic(&music_fn[0][music_select][0]);
                    strcpy(&fta_quotes[26][0],text);
                    FTA(26,&ps[myconnectindex],1);
                    return;
                }

                adduserquote(ud.ridecule[i-1]);

                ch = 0;

                tempbuf[ch] = 4;
                tempbuf[ch+1] = 0;
                strcat((char*)tempbuf+1,ud.ridecule[i-1]);

                i = 1+strlen(ud.ridecule[i-1]);

                if(ud.multimode > 1)
                    for(ch=connecthead;ch>=0;ch=connectpoint2[ch])
                        if (ch != myconnectindex)
                            sendpacket(ch,tempbuf,i);

                pus = NUMPAGES;
                pub = NUMPAGES;

                return;

            }

            if(ud.lockout == 0)
                if(SoundToggle && ALT_IS_PRESSED && ( RTS_NumSounds() > 0 ) && rtsplaying == 0 && VoiceToggle )
            {
                rtsptr = RTS_GetSound (i-1);
                if(*rtsptr == 'C')
                    FX_PlayVOC3D( rtsptr,0,0,0,255,-i);
                else FX_PlayWAV3D( rtsptr,0,0,0,255,-i);

                rtsplaying = 7;

                if(ud.multimode > 1)
                {
                    tempbuf[0] = 7;
                    tempbuf[1] = i;

                    for(ch=connecthead;ch>=0;ch=connectpoint2[ch])
                        if(ch != myconnectindex)
                            sendpacket(ch,(uint8_t*)tempbuf,2);
                }

                pus = NUMPAGES;
                pub = NUMPAGES;

                return;
            }
        }
    }

    if(!ALT_IS_PRESSED && !SHIFTS_IS_PRESSED)
    {

        if( ud.multimode > 1 && ACTION(gamefunc_SendMessage) )
        {
            KB_FlushKeyboardQueue();
            CONTROL_ClearAction( gamefunc_SendMessage );
            ps[myconnectindex].gm |= MODE_TYPE;
            typebuf[0] = 0;
            inputloc = 0;
        }

        if( KB_KeyPressed(sc_F1) || ( ud.show_help && ( KB_KeyPressed(sc_Space) || KB_KeyPressed(sc_Enter) || KB_KeyPressed(sc_kpad_Enter) ) ) )
        {
            KB_ClearKeyDown(sc_F1);
            KB_ClearKeyDown(sc_Space);
            KB_ClearKeyDown(sc_kpad_Enter);
            KB_ClearKeyDown(sc_Enter);
            ud.show_help ++;

            if( ud.show_help > 2 )
            {
                ud.show_help = 0;
                if(ud.multimode < 2 && ud.recstat != 2) ready2send = 1;
                vscrn();
            }
            else
            {
                setview(0,0,xdim-1,ydim-1);
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 0;
                    totalclock = ototalclock;
                }
            }
        }

//        if(ud.multimode < 2)
        {
            if(ud.recstat != 2 && KB_KeyPressed( sc_F2 ) )
            {
                KB_ClearKeyDown( sc_F2 );

                if(movesperpacket == 4 && connecthead != myconnectindex)
                    return;

                FAKE_F2:
                if(sprite[ps[myconnectindex].i].extra <= 0)
                {
                    FTA(118,&ps[myconnectindex],1);
                    return;
                }
                cmenu(350);
                screencapt = 1;
                displayrooms(myconnectindex,65536);
                savetemp("duke3d.tmp",tiles[MAXTILES-1].data,160*100);
                screencapt = 0;
                FX_StopAllSounds();
                clearsoundlocks();

//                setview(0,0,xdim-1,ydim-1);
                ps[myconnectindex].gm |= MODE_MENU;

                if(ud.multimode < 2)
                {
                    ready2send = 0;
                    totalclock = ototalclock;
                    screenpeek = myconnectindex;
                }
            }

            if(KB_KeyPressed( sc_F3 ))
            {
                KB_ClearKeyDown( sc_F3 );

                if(movesperpacket == 4 && connecthead != myconnectindex)
                    return;

                cmenu(300);
                FX_StopAllSounds();
                clearsoundlocks();

//                setview(0,0,xdim-1,ydim-1);
                ps[myconnectindex].gm |= MODE_MENU;
                if(ud.multimode < 2 && ud.recstat != 2)
                {
                    ready2send = 0;
                    totalclock = ototalclock;
                }
                screenpeek = myconnectindex;
            }
        }

        if(KB_KeyPressed( sc_F4 ) && FXDevice != NumSoundCards )
        {
            KB_ClearKeyDown( sc_F4 );
            FX_StopAllSounds();
            clearsoundlocks();

            ps[myconnectindex].gm |= MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 0;
                totalclock = ototalclock;
            }
            cmenu(700);

        }

        if( KB_KeyPressed( sc_F6 ) && (ps[myconnectindex].gm&MODE_GAME))
        {
            KB_ClearKeyDown( sc_F6 );

            if(movesperpacket == 4 && connecthead != myconnectindex)
                return;

            if(lastsavedpos == -1) goto FAKE_F2;

            KB_FlushKeyboardQueue();

            if(sprite[ps[myconnectindex].i].extra <= 0)
            {
                FTA(118,&ps[myconnectindex],1);
                return;
            }
            screencapt = 1;
            displayrooms(myconnectindex,65536);
            savetemp("duke3d.tmp",tiles[MAXTILES-1].data,160*100);
            screencapt = 0;
            if( lastsavedpos >= 0 )
            {
                inputloc = strlen(&ud.savegame[lastsavedpos][0]);
                current_menu = 360+lastsavedpos;
                probey = lastsavedpos;
            }
            FX_StopAllSounds();
            clearsoundlocks();

            setview(0,0,xdim-1,ydim-1);
            ps[myconnectindex].gm |= MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 0;
                totalclock = ototalclock;
            }
        }

        if(KB_KeyPressed( sc_F7 ) )
        {
            KB_ClearKeyDown(sc_F7);
            if( ps[myconnectindex].over_shoulder_on )
                ps[myconnectindex].over_shoulder_on = 0;
            else
            {
                ps[myconnectindex].over_shoulder_on = 1;
                cameradist = 0;
                cameraclock = totalclock;
            }
            FTA(109+ps[myconnectindex].over_shoulder_on,&ps[myconnectindex],1);
        }

        if( KB_KeyPressed( sc_F5 ) && MusicDevice != NumSoundCards )
        {
            KB_ClearKeyDown( sc_F5 );
            strcpy(text,&music_fn[0][music_select][0]);
            strcat(text,".  USE SHIFT-F5 TO CHANGE.");
            strcpy(fta_quotes[26],text);
            FTA(26,&ps[myconnectindex],1);

        }

        if(KB_KeyPressed( sc_F8 ))
        {
            KB_ClearKeyDown( sc_F8 );
            ud.fta_on = !ud.fta_on;
            FTA(24-ud.fta_on,&ps[myconnectindex],1);
        }

        if(KB_KeyPressed( sc_F9 ) && (ps[myconnectindex].gm&MODE_GAME) )
        {
            KB_ClearKeyDown( sc_F9 );

            if(movesperpacket == 4 && myconnectindex != connecthead)
                return;

            if( lastsavedpos >= 0 ) cmenu(15001);
            else cmenu(25000);
            FX_StopAllSounds();
            clearsoundlocks();
            ps[myconnectindex].gm |= MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 0;
                totalclock = ototalclock;
            }
        }

        if(KB_KeyPressed( sc_F10 ))
        {
            KB_ClearKeyDown( sc_F10 );
			cmenu(500);
            FX_StopAllSounds();
            clearsoundlocks();
            ps[myconnectindex].gm |= MODE_MENU;
            if(ud.multimode < 2 && ud.recstat != 2)
            {
                ready2send = 0;
                totalclock = ototalclock;
            }
        }


        if( ud.overhead_on != 0)
        {

            j = totalclock-nonsharedtimer; nonsharedtimer += j;
            if ( ACTION( gamefunc_Enlarge_Screen ) )
                ps[myconnectindex].zoom += mulscale6(j,max(ps[myconnectindex].zoom,256));
            if ( ACTION( gamefunc_Shrink_Screen ) )
                ps[myconnectindex].zoom -= mulscale6(j,max(ps[myconnectindex].zoom,256));

            if( (ps[myconnectindex].zoom > 2048) )
                ps[myconnectindex].zoom = 2048;
            if( (ps[myconnectindex].zoom < 48) )
                ps[myconnectindex].zoom = 48;

        }
    }

    if( KB_KeyPressed(sc_Escape) && ud.overhead_on && ps[myconnectindex].newowner == -1 )
    {
        KB_ClearKeyDown( sc_Escape );
        ud.last_overhead = ud.overhead_on;
        ud.overhead_on = 0;
        ud.scrollmode = 0;
        vscrn();
    }

    if( ACTION(gamefunc_AutoRun) )
    {
        CONTROL_ClearAction(gamefunc_AutoRun);
        ud.auto_run = 1-ud.auto_run;
        FTA(85+ud.auto_run,&ps[myconnectindex],1);
    }

    if( ACTION(gamefunc_Map) )
    {
        CONTROL_ClearAction( gamefunc_Map );
        if( ud.last_overhead != ud.overhead_on && ud.last_overhead)
        {
            ud.overhead_on = ud.last_overhead;
            ud.last_overhead = 0;
        }
        else
        {
            ud.overhead_on++;
            if(ud.overhead_on == 3 ) ud.overhead_on = 0;
            ud.last_overhead = ud.overhead_on;
        }
        restorepalette = 1;
        vscrn();
    }

    if(KB_KeyPressed( sc_F11 ))
    {
        KB_ClearKeyDown( sc_F11 );
		// FIX_00030: Brightness step was not the same from the keys vs menu
        if(SHIFTS_IS_PRESSED) ud.brightness-=8; // Keyboard step must be 8, as the brightness cursor step.
        else ud.brightness+=8;

        if (ud.brightness > 56 )
            ud.brightness = 0;
        else if(ud.brightness < 0)
            ud.brightness = 56;

        setbrightness(ud.brightness>>2,&ps[myconnectindex].palette[0]);
        if(ud.brightness < 40) FTA( 29 + (ud.brightness>>3) ,&ps[myconnectindex],1);
        else if(ud.brightness < 80) FTA( 96 + (ud.brightness>>3) - 5,&ps[myconnectindex],1);
    }
}



void comlinehelp(char  **argv)
{
    printf("Command line help.  %s [/flags...]\n",argv[0]);
    puts(" ?, /?         This help message");
    puts(" /l##          Level (1-11)");
    puts(" /v#           Volume (1-4)");
    puts(" /s#           Skill (1-4)");
    puts(" /r            Record demo");
    puts(" /dFILE        Start to play demo FILE");
    puts(" /m            No monsters");
    puts(" /ns           No sound");
    puts(" /nm           No music");
    puts(" /t#           Respawn, 1 = Monsters, 2 = Items, 3 = Inventory, x = All");
    puts(" /c#           MP mode, 1 = DukeMatch(spawn), 2 = Coop, 3 = Dukematch(no spawn)");
    puts(" /q#           Fake multiplayer (2-8 players)");
    puts(" /a            Use player AI (fake multiplayer only)");
    puts(" /i#           Network mode (1/0) (multiplayer only) (default == 1)");
    puts(" /f#           Send fewer packets (1, 2, 4) (multiplayer only)");
    puts(" /gFILE, /g... Use multiple group files (must be last on command line)");
    puts(" /xFILE        Compile FILE (default GAME.CON)");
    puts(" /u#########   User's favorite weapon order (default: 3425689071)");
    puts(" /#            Load and run a game (slot 0-9)");
    puts(" /z            Skip memory check");
    puts(" -map FILE     Use a map FILE");
    puts(" -name NAME    Foward NAME");
    puts(" -net          Net mode game");
    printf("\n");
}

void checkcommandline(int argc,char  **argv)
{
    short i, j;
    char  *c;
	char  kbdKey;

    ud.fta_on = 1;
    ud.god = 0;
    ud.m_respawn_items = 0;
    ud.m_respawn_monsters = 0;
    ud.m_respawn_inventory = 0;
    ud.warp_on = 0;
    ud.cashman = 0;
    ud.m_player_skill = ud.player_skill = 2;
	ud.multimode_bot = 0;

#ifdef BETA
    return;
#endif

	printf("Commands: ");
	i=1;
	while(i < argc)
	{
   		printf("%s ",argv[i]);
   		i++;
	}
	printf("\n");

	i = 1;

    if(argc > 1)
    {
        while(i < argc)
        {
            c = argv[i];

			if (stricmp(c, "-map") == 0)
            {
				i++;
				strcpy(boardfilename,argv[i]);
				if( strchr(boardfilename,'.') == 0)
					strcat(boardfilename,".map");
				printf("Using level: '%s'.\n",boardfilename);
				continue;

			}

			if (stricmp(c, "-net") == 0)
            {
                i += 2;  // skip filename.
				// FIX_00044: Markers are now on by default in netgames (as real DOS duke3d)
				ud.m_marker = ud.marker = 1;
                continue;
            }

            if (stricmp(c, "-game_dir") == 0)
            {
				// Get the file name
				i++;
				c = argv[i];
				setGameDir(c);

				// skip over the file name now that we have it.
				i++;

                continue;
            }

            if (stricmp(c, "-stun") == 0)
            {
				g_bStun = 1;

				i++;
                continue;
            }

            if (stricmp(c, "/disableautoaim") == 0)
            {

				printf(	"\nThe Host used the /disableautoaim option to turn your Weapon AutoAim OFF\n"
						"Playing without AutoAim is usually extremely difficult and can make boring\n"
						"games ruining Duke's playability. Duke3D was not designed to play with\n"
						"AutoAim OFF like in modern FPS.\n\n"
						"Do you authorize the HOST to turn your AutoAim OFF (Y/N)? ");

				do
					kbdKey = getch()|' ';
				while(kbdKey != 'n' && kbdKey != 'y');

				printf("%c \n", kbdKey);

				if(kbdKey == 'y')
					nHostForceDisableAutoaim = 1;
				else
					nHostForceDisableAutoaim = 2;

				i++;
                continue;
            }

            if(*c == '-')
            {
                if( *(c+1) == '8' ) eightytwofifty = 1;
                i++;
                continue;
            }

            if(*c == '?')
            {
                comlinehelp(argv);
                Error(EXIT_SUCCESS, "");
            }

            if(*c == '/')
            {
                c++;
                switch(*c)
                {
                    case 'x':
                    case 'X':
                        c++;
                        if(*c)
                        {
							if(getGameDir()[0] != '\0'){
								sprintf(confilename, "%s\\%s", getGameDir(), c);
							}
							else{
								strcpy(confilename,c);
							}

                            if(SafeFileExists(confilename) == 0){
                                Error(EXIT_SUCCESS, "Could not find con file '%s'.\n",confilename);
                            }
                            else printf("Using con file: '%s'\n",confilename);
                        }
                        break;
                    case 'g':
                    case 'G':
                        c++;
                        if(*c){
							char  fullpathgrpfile[16]; // 16 not enough
							memset(fullpathgrpfile, 0, 16);

                            if( strchr(c,'.') == 0){
								strcat(c,".grp"); // crap!
							}

							if(getGameDir()[0] != '\0'){
								sprintf(fullpathgrpfile, "%s\\%s", getGameDir(), c);
							}
							else{
								strcpy(fullpathgrpfile, c);
							}

                            j = initgroupfile(fullpathgrpfile);
                            if( j == -1 )
                                printf("Could not find group file %s.\n",fullpathgrpfile);
                        }

                        break;
                    case 'a':
                    case 'A':
                        ud.playerai = 1;
                        puts("Other player AI.");
                        break;
                    case 'n':
                    case 'N':
                        c++;
                        if(*c == 's' || *c == 'S'){
                            CommandSoundToggleOff = 2;
                            puts("Sound off.");
                        }
                        else
                            if(*c == 'm' || *c == 'M'){
                            CommandMusicToggleOff = 1;
                            puts("Music off.");
                            }
                            else{
                                comlinehelp(argv);
                                Error(EXIT_SUCCESS, "");
                            }
                        break;
                    case 'i':
                    case 'I':
                        c++;
                        if(*c == '0')
                            networkmode = 0;
                        if(*c == '1')
                            networkmode = 1;
                        printf("Network Mode %d\n",networkmode);
                        break;
                    case 'c':
                    case 'C':
                        c++;
                        if(*c == '1' || *c == '2' || *c == '3' )  // C1 = DM; C2 =COOP
                            ud.m_coop = *c - '0' - 1; // 0 = DM   1 = COOP
                        else ud.m_coop = 0;

                        switch(ud.m_coop)
                        {
                            case 0:
                                puts("Dukematch (spawn).");
                                break;
                            case 1:
                                puts("Cooperative play.");
                                break;
                            case 2:
                                puts("Dukematch (no spawn).");
                                break;
                        }

                        break;
                    case 'z':
                    case 'Z':
                        memorycheckoveride = 1;
                        break;
                    case 'f':
                    case 'F':
                        c++;
                        if(*c == '1')
                            movesperpacket = 1;
                        if(*c == '2')
                            movesperpacket = 2;
                        if(*c == '4')
                        {
                            movesperpacket = 4;
                            setpackettimeout(0x3fffffff,0x3fffffff);
                        }
                        break;
                    case 't':
                    case 'T':
                        c++;
                        if(*c == '1') ud.m_respawn_monsters = 1;
                        else if(*c == '2') ud.m_respawn_items = 1;
                        else if(*c == '3') ud.m_respawn_inventory = 1;
                        else
                        {
                            ud.m_respawn_monsters = 1;
                            ud.m_respawn_items = 1;
                            ud.m_respawn_inventory = 1;
                        }
                        puts("Respawn on.");
                        break;
                    case 'm':
                    case 'M':
                        if( *(c+1) != 'a' && *(c+1) != 'A' )
                        {
                            ud.m_monsters_off = 1;
                            ud.m_player_skill = ud.player_skill = 0;
                            puts("Monsters off.");
                        }
                        break;
                    case 'w':
                    case 'W':
                        ud.coords = 1;
                        break;
                    case 'q':
                    case 'Q':
                        puts("Fake multiplayer mode.");
                        if( *(++c) == 0) ud.multimode_bot = 1;
                        else ud.multimode_bot = atol(c)%17;
						ud.multimode = ud.multimode_bot;
                        ud.m_coop = ud.coop = 0;
                        ud.m_marker = ud.marker = 1;
                        ud.m_respawn_monsters = ud.respawn_monsters = 1;
                        ud.m_respawn_items = ud.respawn_items = 1;
                        ud.m_respawn_inventory = ud.respawn_inventory = 1;

                        break;
                    case 'r':
                    case 'R':
                        ud.m_recstat = 1;
                        puts("Demo record mode on.");
                        break;
                    case 'd':
                    case 'D':
                        c++;
                        if( strchr(c,'.') == 0)
                            strcat(c,".dmo");
                        printf("Play demo %s.\n",c);
                        strcpy(firstdemofile,c);
                        break;
                    case 'l':
                    case 'L':
                        ud.warp_on = 1;
                        c++;
                        ud.m_level_number = ud.level_number = (atol(c)-1)%11;
                        break;
                    case 'j':
                    case 'J':
                        Error(EXIT_SUCCESS, "This port has a auto adaptive version system. All versions supported, simply change your duke3d.grp\n");
						break;

                    case 'v':
                    case 'V':
                        c++;
                        ud.warp_on = 1;
                        ud.m_volume_number = ud.volume_number = atol(c)-1;
                        break;
                    case 's':
                    case 'S':
                        c++;
                        ud.m_player_skill = ud.player_skill = (atol(c)%5);
                        if(ud.m_player_skill == 4)
                            ud.m_respawn_monsters = ud.respawn_monsters = 1;
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        ud.warp_on = 2 + (*c) - '0';
                        break;
                    case 'u':
                    case 'U':
                        c++;
                        j = 0;
                        if(*c)
                        {
                            puts("Using favorite weapon order(s).");
                            while(*c)
                            {
                                ud.mywchoice[j] = *c-'0';
                                c++;
                                j++;
                            }
                            while(j < 10)
                            {
                                if(j == 9)
                                    ud.mywchoice[9] = 1;
                                else
                                    ud.mywchoice[j] = 2;

                                j++;
                            }
                        }
                        else
                        {
                            puts("Using default weapon orders.");
                        }

                        break;

                    case '?': // Show help
					default:

						comlinehelp(argv);
                        Error(EXIT_SUCCESS, "");
						break;
                }
            }
            i++;
        }
    }
}



void printstr(short x, short y, uint8_t  string[81], uint8_t  attribute)
{
        uint8_t  character;
        short i, pos;

        pos = (y*80+x)<<1;
        i = 0;
        while (string[i] != 0)
        {
                character = string[i];
                printchrasm(0xb8000+(int32_t)pos,1L,(((int32_t)attribute<<8)+(int32_t)character));
                i++;
                pos+=2;
        }
}

void Logo(void)
{
    short i,soundanm;

    soundanm = 0;

    ready2send = 0;
    printf("KB_FlushKeyboardQueue()\n");
    KB_FlushKeyboardQueue();

    setview(0,0,xdim-1,ydim-1);
    clearview(0L);
    palto(0,0,0,63);

    printf("flushperms()\n");
    flushperms();
    nextpage();

    printf("MUSIC_StopSong()\n");
    MUSIC_StopSong();

	if(ud.showcinematics && numplayers < 2)
	{
		////This plays the explosion from the nuclear sign at the beginning.
		if(!VOLUMEONE)
		{
		    if(!KB_KeyWaiting() && nomorelogohack == 0)
		    {
		        getpackets();

                printf("playanm()\n");
		        playanm("logo.anm",5);
                printf("palto()\n");
		        palto(0,0,0,63);
		        KB_FlushKeyboardQueue();
		    }

            printf("clearviewm()\n");
		    clearview(0L);
		    nextpage();
		}
		printf("playmusic()\n");
		//MIDI start here
		playmusic(&env_music_fn[0][0]);

		//"REALITY IS OUR GAME" Screen
	    for(i=0;i<64;i+=7)
			palto(0,0,0,i);
	    ps[myconnectindex].palette = drealms;
	    palto(0,0,0,63);
	    rotatesprite(0,0,65536L,0,DREALMS,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
	    nextpage();
		for(i=63;i>0;i-=7)
			palto(0,0,0,i);



	    totalclock = 0;
	    while( totalclock < (120*7) && !KB_KeyWaiting() )
	        getpackets();





	    for(i=0;i<64;i+=7)
			palto(0,0,0,i);
	    clearview(0L);
	    nextpage();
	    ps[myconnectindex].palette = titlepal;
	    flushperms();
	    rotatesprite(0,0,65536L,0,BETASCREEN,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
	    KB_FlushKeyboardQueue();
	    nextpage();
	    for(i=63;i>0;i-=7)
			palto(0,0,0,i);

	    totalclock = 0;

		//Animate screen (Duke picture wiht "DUKE" "NUKEM 3D" coming from far away and hitting the screen"
	    while(totalclock < (860+120) && !KB_KeyWaiting())
	    {
	        rotatesprite(0,0,65536L,0,BETASCREEN,0,0,2+8+16+64,0,0,xdim-1,ydim-1);

	        if( totalclock > 120 && totalclock < (120+60) )
	        {
	            if(soundanm == 0)
	            {
	                soundanm = 1;
	                sound(PIPEBOMB_EXPLODE);
	            }
	            rotatesprite(160<<16,104<<16,(totalclock-120)<<10,0,DUKENUKEM,0,0,2+8,0,0,xdim-1,ydim-1);
	        }
	        else if( totalclock >= (120+60) )
	            rotatesprite(160<<16,(104)<<16,60<<10,0,DUKENUKEM,0,0,2+8,0,0,xdim-1,ydim-1);

	        if( totalclock > 220 && totalclock < (220+30) )
	        {
	            if( soundanm == 1)
	            {
	                soundanm = 2;
	                sound(PIPEBOMB_EXPLODE);
	            }

	            rotatesprite(160<<16,(104)<<16,60<<10,0,DUKENUKEM,0,0,2+8,0,0,xdim-1,ydim-1);
	            rotatesprite(160<<16,(129)<<16,(totalclock - 220 )<<11,0,THREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);
	        }
	        else if( totalclock >= (220+30) )
	            rotatesprite(160<<16,(129)<<16,30<<11,0,THREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);

			if(PLUTOPAK) // FIX_00064: Cinematics explosions were not right for 1.3/1.3d grp.
			{

				if( totalclock >= 280 && totalclock < 395 )
				{
					rotatesprite(160<<16,(151)<<16,(410-totalclock)<<12,0,PLUTOPAKSPRITE+1,0,0,2+8,0,0,xdim-1,ydim-1);
					if(soundanm == 2)
					{
						soundanm = 3;
						sound(FLY_BY);
					}
				}
				else if( totalclock >= 395 )
				{
					if(soundanm == 3)
					{
						soundanm = 4;
						sound(PIPEBOMB_EXPLODE);
					}
					rotatesprite(160<<16,(151)<<16,30<<11,0,PLUTOPAKSPRITE+1,0,0,2+8,0,0,xdim-1,ydim-1);
				}
			}

	        getpackets();
	        nextpage();
	    }
		// FIX_00077: Menu goes directly to the "NEW GAME" sub-menu when starting new game (Turrican)
		KB_FlushKeyboardQueue();
	}
	else if(numplayers > 1)
    {
		// FIX_00079: "waiting player" screen not showing up (black screen)
		playmusic(&env_music_fn[0][0]);

	    ps[myconnectindex].palette = titlepal;
	    for(i=63;i>0;i-=7) palto(0,0,0,i);

        rotatesprite(0,0,65536L,0,BETASCREEN,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
        rotatesprite(160<<16,(104)<<16,60<<10,0,DUKENUKEM,0,0,2+8,0,0,xdim-1,ydim-1);
        rotatesprite(160<<16,(129)<<16,30<<11,0,THREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);
        rotatesprite(160<<16,(151)<<16,30<<11,0,PLUTOPAKSPRITE+1,0,0,2+8,0,0,xdim-1,ydim-1);

        gametext(160,190,"WAITING FOR PLAYERS",14,2);
        nextpage();
    }
	else
	{
		// FIX_00091: Main theme starting too early (Bryzian/Turrican)
		playmusic(&env_music_fn[0][0]);
	}

    waitforeverybody();

    flushperms();
    clearview(0L);
    nextpage();

    ps[myconnectindex].palette = palette;
    sound(NITEVISION_ONOFF);

    palto(0,0,0,0);
    clearview(0L);
}

void loadtmb(void)
{
    uint8_t  tmb[8000];
    int32_t fil, l;

    fil = kopen4load("d3dtimbr.tmb",0);

    if(fil == -1)
		return;

    l = kfilelength(fil);

    kread(fil,(uint8_t  *)tmb,l);

    MUSIC_RegisterTimbreBank(tmb);

    kclose(fil);
}

/*
===================
=
= ShutDown
=
===================
*/

void ShutDown( void )
{
    SoundShutdown();
    MusicShutdown();
    uninittimer();
    uninitengine();
    CONTROL_Shutdown();
    CONFIG_WriteSetup();
    KB_Shutdown();
    CONSOLE_Term();
}


/*
===================
=
= Startup
=
===================
*/

void compilecons(void)
{
	char  userconfilename[512];

   mymembuf = (char  *)hittype;
   labelcode = (int32_t *)&sector[0];
   label = (char  *)sprite;

	sprintf(userconfilename, "%s", confilename);

   loadefs(userconfilename,mymembuf, 0);

}


void Startup(void)
{
   int i;

   // Init the Console
   CONSOLE_Init();

   KB_Startup();

   CONFIG_GetSetupFilename();
   CONFIG_ReadSetup();

   compilecons();

#ifdef AUSTRALIA
  ud.lockout = 1;
#endif

   if(CommandSoundToggleOff) SoundToggle = 0;
   if(CommandMusicToggleOff) MusicToggle = 0;

//if(VOLUMEONE)
//{
//   printf("\n*** You have run Duke Nukem 3D %ld times. ***\n",ud.executions);
//   if(ud.executions >= 50) puts("IT IS NOW TIME TO UPGRADE TO THE COMPLETE VERSION!!!\n");
//}

   CONTROL_Startup( ControllerType, &GetTime, TICRATE );

// CTW - MODIFICATION
// initengine(ScreenMode,ScreenWidth,ScreenHeight);
   initengine();
// CTW END - MODIFICATION
   inittimer(TICRATE);

   puts("Loading art header.");

  loadpics("tiles000.art", "\0");


   readsavenames();

   tiles[MIRROR].dim.width = tiles[MIRROR].dim.height = 0;

   for(i=0;i<MAXPLAYERS;i++) playerreadyflag[i] = 0;
   initmultiplayers(0,0,0);

   if(numplayers > 1)
    puts("Multiplayer initialized.");

   ps[myconnectindex].palette = (uint8_t  *) &palette[0];
   SetupGameButtons();

   if(networkmode == 255)
       networkmode = 1;

#ifdef PLATFORM_DOS
   puts("Checking music inits.");
   MusicStartup();
   puts("Checking sound inits.");
   SoundStartup();
#else
   /* SBF - wasn't sure if swapping them would harm anything. */
   puts("Checking sound inits.");
   SoundStartup();
   puts("Checking music inits.");
   MusicStartup();
#endif

   // AutoAim
	if(nHostForceDisableAutoaim)
		ud.auto_aim = 0;

   puts("loadtmb()");
   loadtmb();
}


void sendscore(char  *s)
{
    if(numplayers > 1)
      genericmultifunction(-1,s,strlen(s)+1,5);
}


void getnames(void)
{
    short i,j,l;

	// FIX_00031: Names now limited to 10 chars max that is the fragbar field limit.
    for(l=0; l<=9 && myname[l];l++)
    {
        ud.user_name[myconnectindex][l] = toupper(myname[l]);
        buf[l+2] = toupper(myname[l]);
    }

#ifdef CHECK_XDUKE_REV // must not be under "if(numplayers > 1)" so it runs in any case
	ud.rev[myconnectindex][0] = true; // always true. Used to check who we validated
	ud.rev[myconnectindex][1] = DUKE_ID;
	ud.rev[myconnectindex][2] = CHOCOLATE_DUKE_REV_X;
	ud.rev[myconnectindex][3] = CHOCOLATE_DUKE_REV_DOT_Y;
#endif

	memcpy(ud.groupefil_crc32[myconnectindex],groupefil_crc32, sizeof(groupefil_crc32));
	ud.conSize[myconnectindex] = ud.conSize[0]; // [0] still containing the original value
	ud.conCRC[myconnectindex] = ud.conCRC[0];
	ud.exeCRC[myconnectindex] = ud.exeCRC[0];

    if(numplayers > 1)
    {

        buf[0] = 6;
        buf[1] = grpVersion;

        buf[l+2] = 0;
        l += 3;

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if( i != myconnectindex )
                sendpacket(i,(uint8_t*)buf,l);

		if(nHostForceDisableAutoaim==2) // user doesn't want AA off.
			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				buf[0] = 133; // request to stop the game.
				sendpacket(i,(uint8_t*)buf,l);
			}


#ifdef CHECK_XDUKE_REV
        buf[0] = 131; // xDuke TAG ID
        buf[1] = ud.rev[myconnectindex][1];
		buf[2] = ud.rev[myconnectindex][2]; // version x
		buf[3] = ud.rev[myconnectindex][3]; // version .y

		buf[4] = 0;		// reserved
		buf[5] = 0;		// reserved...
        buf[9] = 0;		// reserved.
		// See below for single player mode.

		for(i=connecthead;i>=0;i=connectpoint2[i])
			if( i != myconnectindex )
                sendpacket(i,&buf[0],10);
#endif

  //      getpackets();


        l = 1;
        buf[0] = 9; // send weapon order

        for(i=0;i<10;i++)
        {
            ud.wchoice[myconnectindex][i] = ud.mywchoice[i];
            buf[l] = (uint8_t ) ud.mywchoice[i];
            l++;
        }

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if(i != myconnectindex)
                sendpacket(i,(uint8_t*)&buf[0],11);

		buf[0] = 134;	// GRP CRC + CON SIZE + conCRC + exeCRC
		memcpy(buf+1, groupefil_crc32, sizeof(groupefil_crc32));
		memcpy(buf+1+sizeof(groupefil_crc32), ud.conSize, sizeof(ud.conSize[0]));
		memcpy(buf+1+sizeof(groupefil_crc32)+sizeof(ud.conSize[0]), ud.conCRC, sizeof(ud.conCRC[0]));
		memcpy(buf+1+sizeof(groupefil_crc32)+sizeof(ud.conSize[0])+sizeof(ud.conCRC[0]), ud.exeCRC, sizeof(ud.exeCRC[0]));

		for(i=connecthead;i>=0;i=connectpoint2[i])
			if( i != myconnectindex )
				sendpacket(i,(uint8_t*)buf,1+sizeof(groupefil_crc32)+sizeof(ud.conSize[0])+sizeof(ud.conCRC[0])+
				sizeof(ud.exeCRC[0]));

//        getpackets();

        buf[0] = 10;
        buf[1] = ps[0].aim_mode;
        ps[myconnectindex].aim_mode = ps[0].aim_mode;

        for(i=connecthead;i>=0;i=connectpoint2[i])
            if(i != myconnectindex)
                sendpacket(i,(uint8_t*)buf,2);

//        getpackets();

        if(cp == 0)
        {
            buf[0] = 125;

            for(i=connecthead;i>=0;i=connectpoint2[i])
                if(i != myconnectindex)
                    sendpacket(i,(uint8_t*)buf,1);
        }

//        getpackets();

        waitforeverybody();

#ifdef CHECK_XDUKE_REV
		// from command "case 131:"
		for(l=0,i=connecthead;i>=0;i=connectpoint2[i])
			if(((ud.rev[i][2]<<8)+ud.rev[i][3]) != ((CHOCOLATE_DUKE_REV_X<<8)+CHOCOLATE_DUKE_REV_DOT_Y))
				l=1;
			else
				ud.rev[i][0] = true; // means we validated this guy

		if(l)
		{
			printf("\n*** One or more players do not have the same xDuke version:\n\n");
			for(l=0,i=connecthead;i>=0;i=connectpoint2[i])
				printf("Player [%-10s] is using Chocolate DukeNukem3D v%d.%d\n", ud.user_name[i],
				ud.rev[i][2],ud.rev[i][3]);
			Error(EXIT_SUCCESS, "");
		}
#endif

		// checking GRP/CON size from "case 134"
		for(l=0,i=connecthead;i>=0;i=connectpoint2[i])
			for(j=0; j<MAXGROUPFILES; j++)
			{
				if(ud.groupefil_crc32[i][j]!=ud.groupefil_crc32[myconnectindex][j] || ud.conSize[i] != ud.conSize[myconnectindex])
					l=1;
			}

		if(l)
		{
			printf("\n*** One or more players do not have the same GRP/CON version:\n\n");
			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				for(j=0; j<MAXGROUPFILES && ud.groupefil_crc32[i][j]; j++)
				{
					if(j)
						printf("                    GRP (Add-on) : %s CRC=%X\n",
						grpVersion2char_from_crc(ud.groupefil_crc32[i][j]),
						ud.groupefil_crc32[i][j]);
					else
						printf("Player [%-10s] GRP (base)   : %s CRC=%X\n", ud.user_name[i],
						grpVersion2char_from_crc(ud.groupefil_crc32[i][j]),
						ud.groupefil_crc32[i][j]);
				}
				printf("                    CON code size: %d\n\n",ud.conSize[i]);
			}
			Error(EXIT_SUCCESS,	"");
		}
    }
	else if(nHostForceDisableAutoaim==2)
	{
		nHostForceDisableAutoaim=0;
		ud.auto_aim = 2;
	}

    if(cp == 1)
        gameexit("Please put Duke Nukem 3D Atomic Edition CD in drive.");
}


const char* const baseDir="duke3d*.grp";
#ifdef _WIN32

void findGRPToUse(uint8_t * groupfilefullpath)
{
    WIN32_FIND_DATA FindFileData;
	HANDLE hFind =  INVALID_HANDLE_VALUE;
    int i=0,kbdKey ;
	char  groupfile[9][512];
	int grpID ;

	if(getGameDir()[0] != '\0')
	{
		sprintf(groupfilefullpath, "%s\\%s", getGameDir(), baseDir);
		hFind = FindFirstFile(groupfilefullpath, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			sprintf(groupfilefullpath, "%s", baseDir);
		}
		else
			FindClose(hFind);
	}
	else
		sprintf(groupfilefullpath, "%s", baseDir);

	printf("Searching '%d':\n\n",groupfilefullpath);
	hFind = FindFirstFile(groupfilefullpath,&FindFileData);

	if ( hFind==INVALID_HANDLE_VALUE )
		Error(EXIT_SUCCESS, "Can't find '%s'\n", groupfilefullpath);

	do
	{
		i++;
		sprintf(groupfile[i-1], "%s", FindFileData.cFileName);
		printf("Found GRP #%d:\t%d Bytes\t %s \n", i, FindFileData.nFileSizeLow, groupfile[i-1]);
	} while ( FindNextFile(hFind, &FindFileData) && i < 9 );

	if(i==1)
		grpID = 0;
	else
	{
		printf("\n-> Choose a base GRP file from 1 to %c: ",'0' + i);
		do
			kbdKey = getch();
		while(kbdKey < '1' || kbdKey > ('0' + i));
		printf("%c\n", kbdKey);
		grpID =  groupfile[kbdKey-'1'];

	}

	FindClose(hFind);
	if (strlen(getGameDir()) == 0)
		 sprintf(groupfilefullpath, "./%s", groupfile[grpID]);
	else
	   sprintf(groupfilefullpath, "%s//%s", getGameDir(), groupfile[grpID]);
}

#else

int dukeGRP_Match(char* filename,int length)
{
    char* cursor = filename+length-4;

    if (strncasecmp(cursor,".grp",4))
        return 0;

    return !strncasecmp(filename,"duke3d",6);
}


#include <dirent.h>
void findGRPToUse(char * groupfilefullpath){
    strcpy(groupfilefullpath,"/sd/roms/duke3d/duke3d.grp");
}

#endif

static int load_duke3d_groupfile(void)
{
	// FIX_00032: Added multi base GRP manager. Use duke3d*.grp to handle multiple grp.

	char  groupfilefullpath[512];
    groupfilefullpath[0] = '\0';

    findGRPToUse(groupfilefullpath);

    if (groupfilefullpath[0] == '\0')
        return false;


	FixFilePath(groupfilefullpath);

	return(initgroupfile(groupfilefullpath) != -1);
}

int main(int argc,char  **argv)
{
    int32_t i, j;
	int32_t filehandle;


	uint8_t  kbdKey;
	uint8_t  *exe;


	//printf(	"This is a debug version 19.7.1 only Based on 19.7\n"
	//		"Fully compliant with v19.7. Added the following:\n\n"
	//		"FIX_00086: grp loaded by smaller sucessive chunks to avoid\n"
	//		"           overloading low ram computers (Spanator)\n"
	//		"FIX_00087: intro in 1024x768 mode being slow. Undone FIX_00070\n"
	//		"           and fixed font issue again (Bryzian)\n"
	//		"FIX_00088: crash on maps using a bad palette index like the end\n"
	//		"           of roch3.map (NY00123)\n"
	//		"FIX_00089: scoreboard not shown for last player who quits a DM.\n"
	//		"           Only 19.7 affected. (Sarah)\n"
	//		"FIX_00090: Removed showinfo key. FPS were shown after CRC msg. \n"
	//		"           CRC not always removed. (Turrican)\n"
	//		"FIX_00091: Main theme starting too early (Bryzian/Turrican)\n"
	//		"FIX_00092: corrupted saved files making the following saved\n"
	//		"           files invisible (Bryzian)\n\n"
	//		"This version should not be distributed. It's not secret but it\n"
	//		"would create a bad mess in the duke community if people start\n"
	//		"using it as it may contain new unsuspected bugs. Only a select\n"
	//		"group of known dukers who know what they are doing should be using\n"
	//		"it. Please report new bugs at xd@m-klein.com or on DX forums. Thx!\n\n");

	printf("*** Chocolate DukeNukem3D v%d.%d ***\n\n", CHOCOLATE_DUKE_REV_X, CHOCOLATE_DUKE_REV_DOT_Y);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Delay(100);

	// FIX_00033: Fake multi and AI are now fully working
	ud.multimode = 1;  // xduke: must be done before checkcommandline or that will prevent Fakeplayer and AI

    if (!load_duke3d_groupfile())
    {
        Error(EXIT_SUCCESS, "Could not initialize any original BASE duke3d*.grp file\n"
							"Even if you are playing a custom GRP you still need\n"
							"an original base GRP file as Shareware/Full 1.3D GRP or\n"
							"the v1.5 ATOMIC GRP file. Such a file seems to be missing\n"
							"or is corrupted\n");
    }

	// FIX_00022: Automatically recognize the shareware grp (v1.3) + full version (1.3d) +
	//            atomic (1.4/1.5 grp) and the con files version (either 1.3 or 1.4) (JonoF's idea)

	// Detecting grp version
	// We keep the old GRP scheme detection for 19.6 compliance. Will be obsolete.
	filehandle = kopen4load("DUKEDC9.MAP",1);
	kclose(filehandle);

	if (filehandle == -1) // not DC pack
	{
		filehandle = kopen4load("DUKESW.BIN",1);
		kclose(filehandle);

		if (filehandle == -1) // not Shareware version 1.3
		{
			filehandle = kopen4load("E4L11.MAP",1);
			kclose(filehandle);

			if (filehandle == -1) // not Atomic Edition 1.4/1.5
			{
				filehandle = kopen4load("E3L11.MAP",1);
				kclose(filehandle);

				if (filehandle == -1) // not Regular version 1.3d
				{
					grpVersion = UNKNOWN_GRP;
				}
				else
				{
					grpVersion = REGULAR_GRP13D;
				}
			}
			else
			{
				grpVersion = ATOMIC_GRP14_15;
			}
		}
		else
		{
			grpVersion = SHAREWARE_GRP13;
		}
	}
	else
	{
		grpVersion = DUKEITOUTINDC_GRP;
	}

	// FIX_00062: Better support and identification for GRP and CON files for 1.3/1.3d/1.4/1.5
	if (	groupefil_crc32[0]==CRC_BASE_GRP_SHAREWARE_13 ||
				groupefil_crc32[0]==CRC_BASE_GRP_FULL_13 ||
				groupefil_crc32[0]==CRC_BASE_GRP_PLUTONIUM_14 ||
				groupefil_crc32[0]==CRC_BASE_GRP_ATOMIC_15 )
	{
		printf("GRP identified as: %s\n", grpVersion2char_from_crc(groupefil_crc32[0]));
	}
	else
	{
		printf(	"The content of your original BASE *.GRP is corrupted. CRC=%X\n"
			"You may run in troubles. Official GRP are:\n\n", groupefil_crc32[0]);

		for(i=0; i<MAX_KNOWN_GRP; i++)
			printf("%s -> CRC32=%X  Size=%d bytes\n", crc32lookup[i].name, crc32lookup[i].crc32, crc32lookup[i].size);

		printf(	"\nYou should try to get one of these GRP only as a base GRP\n"
				"Do you want to continue anyway? (Y/N): ");
		/*
        do
			kbdKey = getch() | ' ';
		while(kbdKey != 'y' && kbdKey != 'n');
		printf("%c\n", kbdKey);

		if(kbdKey == 'n')
			Error(EXIT_SUCCESS,"");
        */
	}

	// computing exe crc
	ud.exeCRC[0] = 0;
	exe = NULL;
    SDL_LockDisplay();
	filehandle = open(argv[0],O_BINARY|O_RDONLY);
	if(filehandle!=-1)
	{
		exe = malloc(filelength(filehandle));
		if(exe)
		{
			read(filehandle, exe, filelength(filehandle));
			ud.exeCRC[0] = crc32_update(exe, filelength(filehandle), ud.exeCRC[0]);
			free(exe);
		}
		close(filehandle);
	}
    SDL_UnlockDisplay();

	checkcommandline(argc,argv);

    _platform_init(argc, argv, "Duke Nukem 3D", "Duke3D");

    totalmemory = Z_AvailHeap();

    if(memorycheckoveride == 0)
    {
        if(totalmemory < (3162000-350000))
        {
            puts("You don't have enough free memory to run Duke Nukem 3D.");
            puts("The DOS \"mem\" command should report 6,800K (or 6.8 megs)");
            puts("of \"total memory free\".\n");
            printf("Duke Nukem 3D requires %d more bytes to run.\n",3162000-350000-totalmemory);
            Error(EXIT_SUCCESS, "");
        }
    }
    else
        printf("Using %d bytes for heap.\n",totalmemory);

    RegisterShutdownFunction( ShutDown );


    Startup();

    if( eightytwofifty && numplayers > 1 && (MusicDevice != NumSoundCards) )
    {
        puts("\n=========================================================================");
        puts("WARNING: 8250 UART detected.");
        puts("Music is being disabled and lower quality sound is being set.  We apologize");
        puts("for this, but it is necessary to maintain high frame rates while trying to");
        puts("play the game on an 8250.  We suggest upgrading to a 16550 or better UART");
        puts("for maximum performance.  Press any key to continue.");
        puts("=========================================================================\n");

        while( !KB_KeyWaiting() ) getpackets();
    }

	if(g_bStun)
	{
		waitforeverybody();
	}

    if(numplayers > 1) // if multimode > 1 and numplayer == 1 => fake player mode on
    {
        ud.multimode = numplayers;
        sendlogon();
    }
    else if(boardfilename[0] != 0)
    {
        ud.m_level_number = 7;
        ud.m_volume_number = 0;
        ud.warp_on = 1;
    }

    getnames();

    if(ud.multimode > 1)
    {
        playerswhenstarted = ud.multimode;

		// AddFaz fix.
		// This would cause monsters not to spawn when loading a usermap
		/*
        if(ud.warp_on == 0)
        {
            ud.m_monsters_off = 1;
            ud.m_player_skill = 0;
        }
		*/
    }

    ud.last_level = -1;

   RTS_Init(ud.rtsname);
   if(numlumps)
	   printf("Using .RTS file:%s\n",ud.rtsname);

   if (CONTROL_JoystickEnabled)
       CONTROL_CenterJoystick(CenterCenter,UpperLeft,LowerRight,CenterThrottle,CenterRudder);

   puts("Loading palette/lookups.");
    if( setgamemode(ScreenMode,ScreenWidth,ScreenHeight) < 0 )
    {
        printf("\nVESA driver for ( %i * %i ) not found/supported!\n",xdim,ydim);
        ScreenMode = 2;
        ScreenWidth = 320;
        ScreenHeight = 200;
        setgamemode(ScreenMode,ScreenWidth,ScreenHeight);
    }


    printf("genspriteremaps()\n");
    genspriteremaps();

    setbrightness(ud.brightness>>2,&ps[myconnectindex].palette[0]);

    if(KB_KeyPressed( sc_Escape ) )
		gameexit(" ");

    FX_StopAllSounds();
    clearsoundlocks();

    if(ud.warp_on > 1 && ud.multimode < 2)
    {
        clearview(0L);
        ps[myconnectindex].palette = palette;
        palto(0,0,0,0);
        rotatesprite(320<<15,200<<15,65536L,0,LOADSCREEN,0,0,2+8+64,0,0,xdim-1,ydim-1);
        menutext(160,105,0,0,"LOADING SAVED GAME...");
        nextpage();

        j = loadplayer(ud.warp_on-2);
        if(j)
            ud.warp_on = 0;
    }

    MAIN_LOOP_RESTART:

    if(ud.warp_on == 0) //if game is loaded without /V or /L cmd arguments.
	{

		if (numplayers > 1 && boardfilename[0] != 0) //check if a user map is loaded and in multiplayer.
		{
			int c;

			ud.level_number = ud.m_level_number = 7; // 7 = usermap.
            ud.volume_number = ud.m_volume_number;
			ud.player_skill = ud.m_player_skill;

            switch(ud.m_coop) //set item spawn options, as they would be if
			{      //game was started via main menu.
				case 0:
					ud.respawn_inventory = ud.m_respawn_inventory = 1;
					ud.respawn_items = ud.m_respawn_items = 1;
					break;
				case 1:
					ud.respawn_inventory = ud.m_respawn_inventory = 1;
					ud.respawn_items = ud.m_respawn_items = 0;
					break;
				case 2:
					ud.respawn_inventory = ud.m_respawn_inventory = 0;
					ud.respawn_items = ud.m_respawn_items = 0;
				break;
            }

			if( ud.m_player_skill == 4 )
			{
				ud.m_respawn_monsters = 1; //set skill
			}
			else
			{
				ud.m_respawn_monsters = 0;
			}

			waitforeverybody();

			for(c=connecthead;c>=0;c=connectpoint2[c])
            {
				resetweapons(c);
				resetinventory(c);
            }

			newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill);

            enterlevel(MODE_GAME); //start game.

		}
		else
		{
			Logo(); //play logo, (game must be started via menus).
		}
	}



	else if(ud.warp_on == 1) //if cmd arguments /V and /L are given.
    {

		if (numplayers > 1) //if in multiplayer reset everyones weapon status.
		{
			int c;

            switch(ud.m_coop)	//set item spawn options, as they would be if
			{					//game was started via main menu.
				case 0:
					ud.respawn_inventory = ud.m_respawn_inventory = 1;
					ud.respawn_items = ud.m_respawn_items = 1;
					break;
				case 1:
					ud.respawn_inventory = ud.m_respawn_inventory = 1;
					ud.respawn_items = ud.m_respawn_items = 0;
					break;
				case 2:
					ud.respawn_inventory = ud.m_respawn_inventory = 0;
					ud.respawn_items = ud.m_respawn_items = 0;
					break;
			}

			if( ud.m_player_skill == 4 )
			{
				ud.m_respawn_monsters = 1; //set skill
			}
			else
			{
				ud.m_respawn_monsters = 0;
			}

			waitforeverybody();

			for(c=connecthead;c>=0;c=connectpoint2[c])
            {
			    resetweapons(c); //without this players would spawn with no weapon.
				resetinventory(c);
            }

		}

		newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill);
		enterlevel(MODE_GAME); //start game.

    }
    else
	{
		vscrn();
	}

    if( ud.warp_on == 0 && playback() )
    {
        FX_StopAllSounds();
        clearsoundlocks();
        nomorelogohack = 1;
        goto MAIN_LOOP_RESTART;
    }

    ud.warp_on = 0;

	//The main game loop is here.
    while ( !(ps[myconnectindex].gm&MODE_END) )
    {
    	sampletimer();
        if( ud.recstat == 2 || ud.multimode > 1 || ( ud.show_help == 0 && (ps[myconnectindex].gm&MODE_MENU) != MODE_MENU ) )
            if( ps[myconnectindex].gm&MODE_GAME )
			{
                // (" It's stuck here ")
				//printf("ps[myconnectindex].gm&MODE_GAME\n");
				if( moveloop() )
				{
					continue;
				}
			}

        if( ps[myconnectindex].gm&MODE_EOL || ps[myconnectindex].gm&MODE_RESTART )
        {

            if( ps[myconnectindex].gm&MODE_EOL )
            {
                closedemowrite();

                ready2send = 0;

                i = ud.screen_size;
                ud.screen_size = 0;
                vscrn();
                ud.screen_size = i;
                dobonus(0);

                if(ud.eog)
                {
                    ud.eog = 0;
                    if(ud.multimode < 2)
                    {
						if(VOLUMEONE)
                        	doorders();

                        ps[myconnectindex].gm = MODE_MENU;
                        cmenu(0);
                        probey = 0;
                        goto MAIN_LOOP_RESTART;
                    }
                    else
                    {
                        ud.m_level_number = 0;
                        ud.level_number = 0;
                    }
                }
            }

            ready2send = 0;
            if(numplayers > 1) ps[myconnectindex].gm = MODE_GAME;

            enterlevel(ps[myconnectindex].gm);
            continue;
        }

        cheats();

        if( !CONSOLE_IsActive() )
          nonsharedkeys();


        if( (ud.show_help == 0 && ud.multimode < 2 && !(ps[myconnectindex].gm&MODE_MENU) ) || ud.multimode > 1 || ud.recstat == 2)
            i = min(max((totalclock-ototalclock)*(65536L/TICSPERFRAME),0),65536);
        else
            i = 65536;

        displayrooms(screenpeek,i);
        displayrest(i);

        if(ps[myconnectindex].gm&MODE_DEMO)
            goto MAIN_LOOP_RESTART;

        if(debug_on)
			caches();

        checksync();

		if (VOLUMEONE)
        	if(ud.show_help == 0 && show_shareware > 0 && (ps[myconnectindex].gm&MODE_MENU) == 0 )
            	rotatesprite((320-50)<<16,9<<16,65536L,0,BETAVERSION,0,0,2+8+16+128,0,0,xdim-1,ydim-1);

        nextpage();
    }

    gameexit(" ");
	return(0);
}

uint8_t  opendemoread(uint8_t  which_demo) // 0 = mine
{
    char  d[] = "demo_.dmo";
    char  *fname = d;
    uint8_t  ver;
    short i,j;

	int32 dummy;
	int32_t groupefil_crc32_from_demo[MAXGROUPFILES];

    if(which_demo == 10)
        d[4] = 'x';
    else
        d[4] = '0' + which_demo;

    ud.reccnt = 0;

     if(which_demo == 1 && firstdemofile[0] != 0)
     {
		fname = firstdemofile;
		if ((recfilep = TCkopen4load(firstdemofile,0)) == -1)
		{
			return(0);
		}
     }
     else
	 {
		 if ((recfilep = TCkopen4load(d,0)) == -1)
		 {
			return(0);
		 }
	 }

     kread(recfilep,&ud.reccnt,sizeof(int32_t));
     kread(recfilep,&ver,sizeof(uint8_t ));

	 printf("%s has version = %d\n", fname, ver);

	// FIX_00015: Backward compliance with older demos (down to demos v27, 28, 116, 117 and 118)
	if (PLUTOPAK)
	{
		if( (ver != BYTEVERSION && ver != BYTEVERSION_116 && ver != BYTEVERSION_117 && ver != BYTEVERSION_118) ) // || (ud.reccnt < 512) )
		{
			printf("%s is a demo version %d. We want v. %d, %d, %d, or %d (1.5 Atomic versions)\n",
					fname, (int) ver, BYTEVERSION_116, BYTEVERSION_117, BYTEVERSION_118, BYTEVERSION);
			kclose(recfilep);
			return 0;
		}
	}
	else // 1.3/1.3d style
	{
		if( (ver != BYTEVERSION && ver != BYTEVERSION_27 && ver != BYTEVERSION_28 && ver != BYTEVERSION_29) ) // || (ud.reccnt < 512) )
		{
			printf("%s is a demo version %d. We want v. %d, %d, %d or %d (1.3/1.3d versions)\n",
					fname, (int) ver, BYTEVERSION_27, BYTEVERSION_28, BYTEVERSION_29, BYTEVERSION);
			kclose(recfilep);
			return 0;
		}
	}

	// FIX_00062: Better support and identification for GRP and CON files for 1.3/1.3d/1.4/1.5
	if(ver==BYTEVERSION)
	{
		kread(recfilep, (int32_t *)groupefil_crc32_from_demo, sizeof(groupefil_crc32_from_demo));

		for(i=0; i<MAXGROUPFILES; i++)
			if(groupefil_crc32_from_demo[i]!=groupefil_crc32[i])
			{
				for(j=0; j<=i; j++)
				{
					printf("You have GRP #%d:  %s (CRC32=%X)\n"
						   "this demo expects %s (CRC32=%X)\n",
						j, grpVersion2char_from_crc(groupefil_crc32[j]),groupefil_crc32[j],
						grpVersion2char_from_crc(groupefil_crc32_from_demo[j]),groupefil_crc32_from_demo[j]);

				}
				kclose(recfilep);
				return 0;
			}


	}

	 ud.playing_demo_rev = ver;

	 kread(recfilep,(uint8_t  *)&ud.volume_number,sizeof(uint8_t ));
     kread(recfilep,(uint8_t  *)&ud.level_number,sizeof(uint8_t ));
     kread(recfilep,(uint8_t  *)&ud.player_skill,sizeof(uint8_t ));
     kread(recfilep,(uint8_t  *)&ud.m_coop,sizeof(uint8_t ));
     kread(recfilep,(uint8_t  *)&ud.m_ffire,sizeof(uint8_t ));
     kread(recfilep,(short *)&ud.multimode,sizeof(short));
     kread(recfilep,(short *)&ud.m_monsters_off,sizeof(short));
     kread(recfilep,(int32 *)&ud.m_respawn_monsters,sizeof(int32));
     kread(recfilep,(int32 *)&ud.m_respawn_items,sizeof(int32));
     kread(recfilep,(int32 *)&ud.m_respawn_inventory,sizeof(int32));
     kread(recfilep,(int32 *)&ud.playerai,sizeof(int32));
     kread(recfilep,(uint8_t  *)&ud.user_name[0][0],sizeof(ud.user_name));
	 // FIX_00034: Demos do not turn your run mode off anymore:
     kread(recfilep,(int32 *)&dummy /*ud.auto_run*/,sizeof(int32)); // not needed and would affect autorun status in duke3d.cfg when quitting duke from a demo
     kread(recfilep,(uint8_t  *)boardfilename,sizeof(boardfilename));
     if( boardfilename[0] != 0 )
     {
        ud.m_level_number = 7;
        ud.m_volume_number = 0;
     }

     for(i=0;i<ud.multimode;i++)
	 {
        kread(recfilep,(int32 *)&ps[i].aim_mode,sizeof(uint8_t ));

		// FIX_00080: Out Of Synch in demos. Tries recovering OOS in old demos v27/28/29/116/117/118. New: v30/v119.
		if(ver==BYTEVERSION)
			kread(recfilep,ud.wchoice[i],sizeof(ud.wchoice[0]));
	 }

     ud.god = ud.cashman = ud.eog = ud.showallmap = 0;
     ud.clipping = ud.scrollmode = ud.overhead_on = 0;
	 // FIX_00034: Demos do not turn your run mode off anymore:
     /* ud.showweapons =  */ ud.pause_on /*= ud.auto_run */ = 0; // makes no sense to reset those 2 value!

         newgame(ud.volume_number,ud.level_number,ud.player_skill);
         return(1);
}


void opendemowrite(void)
{
    char  d[] = "demo1.dmo";
    int32_t dummylong = 0;
    uint8_t  ver;
    short i;
	char  fullpathdemofilename[16];

    if(ud.recstat == 2) kclose(recfilep);

    ver = BYTEVERSION;

	// Are we loading a TC?
	if(getGameDir()[0] != '\0'){
		// Yes
		sprintf(fullpathdemofilename, "%s\\%s", getGameDir(), d);
	}
	else{
		// No
		sprintf(fullpathdemofilename, "%s", d);
	}
    SDL_LockDisplay();
// CTW - MODIFICATION
//  if ((frecfilep = fopen(d,"wb")) == -1) return;
    if ((frecfilep = fopen(fullpathdemofilename,"wb")) == NULL) return;
// CTW END - MODIFICATION
    fwrite(&dummylong,4,1,frecfilep);
    fwrite(&ver,sizeof(uint8_t ),1,frecfilep);
	// FIX_00062: Better support and identification for GRP and CON files for 1.3/1.3d/1.4/1.5
	fwrite((int32_t *)groupefil_crc32,sizeof(groupefil_crc32),1,frecfilep);
    fwrite((uint8_t  *)&ud.volume_number,sizeof(uint8_t ),1,frecfilep);
    fwrite((uint8_t  *)&ud.level_number,sizeof(uint8_t ),1,frecfilep);
    fwrite((uint8_t  *)&ud.player_skill,sizeof(uint8_t ),1,frecfilep);
    fwrite((uint8_t  *)&ud.m_coop,sizeof(uint8_t ),1,frecfilep);
    fwrite((uint8_t  *)&ud.m_ffire,sizeof(uint8_t ),1,frecfilep);
    fwrite((short *)&ud.multimode,sizeof(short),1,frecfilep);
    fwrite((short *)&ud.m_monsters_off,sizeof(short),1,frecfilep);
    fwrite((int32 *)&ud.m_respawn_monsters,sizeof(int32),1,frecfilep);
    fwrite((int32 *)&ud.m_respawn_items,sizeof(int32),1,frecfilep);
    fwrite((int32 *)&ud.m_respawn_inventory,sizeof(int32),1,frecfilep);
    fwrite((int32 *)&ud.playerai,sizeof(int32),1,frecfilep);
    fwrite((uint8_t  *)&ud.user_name[0][0],sizeof(ud.user_name),1,frecfilep);
    fwrite((int32 *)&ud.auto_run,sizeof(int32),1,frecfilep);
    fwrite((uint8_t  *)boardfilename,sizeof(boardfilename),1,frecfilep);

    for(i=0;i<ud.multimode;i++)
	{
		fwrite((int32 *)&ps[i].aim_mode,sizeof(uint8_t ),1,frecfilep); // seems wrong; prolly not needed anyway
		// FIX_00080: Out Of Synch in demos. Tries recovering OOS in old demos v27/28/29/116/117/118. New: v30/v119.
		fwrite(ud.wchoice[i],sizeof(ud.wchoice[0]),1,frecfilep);
	}
    SDL_UnlockDisplay();
    totalreccnt = 0;
    ud.reccnt = 0;
}

void record(void)
{
    short i;
    for(i=connecthead;i>=0;i=connectpoint2[i])
         {
         copybufbyte(&sync[i],&recsync[ud.reccnt],sizeof(input));
			   ud.reccnt++;
                 totalreccnt++;
                 if (ud.reccnt >= RECSYNCBUFSIZ)
                 {
              dfwrite(recsync,sizeof(input)*ud.multimode,ud.reccnt/ud.multimode,frecfilep);
                          ud.reccnt = 0;
                 }
         }
}

void closedemowrite(void)
{
    SDL_LockDisplay();
    if (ud.recstat == 1)
    {
        if (ud.reccnt > 0)
        {
            dfwrite(recsync,sizeof(input)*ud.multimode,ud.reccnt/ud.multimode,frecfilep);

            fseek(frecfilep,SEEK_SET,0L);
            fwrite(&totalreccnt,sizeof(int32_t),1,frecfilep);
            ud.recstat = ud.m_recstat = 0;
        }
        fclose(frecfilep);
        frecfilep = NULL;
    }
    SDL_UnlockDisplay();
}

// CTW - MODIFICATION
// On my XP machine, demo playback causes the game to crash shortly in.
// Only bug found so far, not sure if it's OS dependent or compiler or what.
// Seems to happen when player input starts being simulated, but just guessing.
// This change effectively disables it. The related code is still enabled.
// (This is working on Linux, so I flipped it back to '1'. --ryan.)
 uint8_t  which_demo = 1;
// CTW END - MODIFICATION

uint8_t  in_menu = 0;

// extern int32_t syncs[];
int32_t playback(void)
{
    int32_t i,j,k,l,t;
    uint8_t  foundemo;

    if( ready2send )
	{
		return 0;
	}

	/*
	if(numplayers > 1)
		return 1;
	*/

    foundemo = 0;

    RECHECK:

    in_menu = ps[myconnectindex].gm&MODE_MENU;

    pub = NUMPAGES;
    pus = NUMPAGES;

    flushperms();

	if(numplayers < 2 && ud.multimode_bot<2) foundemo = opendemoread(which_demo);

    if(foundemo == 0)
    {

        if(which_demo > 1)
        {
            which_demo = 1;
            goto RECHECK;
        }
        for(t=0;t<63;t+=7) palto(0,0,0,t);
        drawbackground();

        CONSOLE_HandleInput();
        if( !CONSOLE_IsActive())
        {
            menus();
        }
        CONSOLE_Render();
        ps[myconnectindex].palette = palette;
        nextpage();
        for(t=63;t>0;t-=7)
		{
			palto(0,0,0,t);
		}

        ud.reccnt = 0;
    }
    else
    {
        ud.recstat = 2;
        which_demo++;
        if(which_demo == 10)
		{
			which_demo = 1;
		}

        enterlevel(MODE_DEMO);
    }

    if(foundemo == 0 || in_menu || KB_KeyWaiting() || numplayers > 1)
    {
        FX_StopAllSounds();
        clearsoundlocks();
        ps[myconnectindex].gm |= MODE_MENU;
    }

    ready2send = 0;
    i = 0;

    KB_FlushKeyboardQueue();

    k = 0;

    while (ud.reccnt > 0 || foundemo == 0)
    {

        if(foundemo) while ( totalclock >= (lockclock+TICSPERFRAME) )
        {
            if ((i == 0) || (i >= RECSYNCBUFSIZ))
            {
                i = 0;
                l = min(ud.reccnt,RECSYNCBUFSIZ);
                kdfread(recsync,sizeof(input)*ud.multimode,l/ud.multimode,recfilep);
            }

            for(j=connecthead;j>=0;j=connectpoint2[j])
            {
               copybufbyte(&recsync[i],&inputfifo[movefifoend[j]&(MOVEFIFOSIZ-1)][j],sizeof(input));

			   movefifoend[j]++;
               i++;
               ud.reccnt--;
            }
            domovethings();
        }

        if(foundemo == 0)
            drawbackground();
        else
        {
            if( !CONSOLE_IsActive() )
            {
                nonsharedkeys();
            }

            j = min(max((totalclock-lockclock)*(65536/TICSPERFRAME),0),65536);
            displayrooms(screenpeek,j);
            displayrest(j);

            if(ud.multimode > 1 && ps[myconnectindex].gm )
                getpackets();
        }

        if( (ps[myconnectindex].gm&MODE_MENU) && (ps[myconnectindex].gm&MODE_EOL) )
		{
			printf("playback(1) :: goto RECHECK:\n");
			goto RECHECK;
		}

        if(ps[myconnectindex].gm&MODE_TYPE)
        {
            typemode();
            if((ps[myconnectindex].gm&MODE_TYPE) != MODE_TYPE)
                ps[myconnectindex].gm = MODE_MENU;
        }
        else
        {
            CONSOLE_HandleInput();
            if( !CONSOLE_IsActive())
            {
                menus();
            }
            CONSOLE_Render();
            if( ud.multimode > 1 )
            {
                ControlInfo noshareinfo;
                if( !CONSOLE_IsActive() )
                {
                    CONTROL_GetInput( &noshareinfo );
                    if( ACTION(gamefunc_SendMessage) )
                    {
                        KB_FlushKeyboardQueue();
                        CONTROL_ClearAction( gamefunc_SendMessage );
                        ps[myconnectindex].gm = MODE_TYPE;
                        typebuf[0] = 0;
                        inputloc = 0;
                    }
                }

            }
        }

        operatefta();

        if(ud.last_camsprite != ud.camerasprite)
        {
            ud.last_camsprite = ud.camerasprite;
            ud.camera_time = totalclock+(TICRATE*2);
        }

		if (VOLUMEONE)
			if( ud.show_help == 0 && (ps[myconnectindex].gm&MODE_MENU) == 0 )
				rotatesprite((320-50)<<16,9<<16,65536L,0,BETAVERSION,0,0,2+8+16+128,0,0,xdim-1,ydim-1);

		getpackets();
        nextpage();

        if( ps[myconnectindex].gm==MODE_END || ps[myconnectindex].gm==MODE_GAME )
        {
            if(foundemo)
                kclose(recfilep);
            ud.playing_demo_rev = 0;
			return 0;
        }
    }
    kclose(recfilep);
	ud.playing_demo_rev = 0;
    if(ps[myconnectindex].gm&MODE_MENU)
	{
		goto RECHECK;
	}

    return 1;
}

uint8_t  moveloop()
{
    int32_t i;

    if (numplayers > 1)
	{
		while (fakemovefifoplc < movefifoend[myconnectindex])
		{
			fakedomovethings();
		}
	}


    getpackets();

    if (numplayers < 2) bufferjitter = 0;
    while (movefifoend[myconnectindex]-movefifoplc > bufferjitter)
    {
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (movefifoplc == movefifoend[i]) break;
        if (i >= 0) break;
        if( domovethings() ) return 1;
    }
    return 0;
}

void fakedomovethingscorrect(void)
{
     int32_t i;
     struct player_struct *p;

     if (numplayers < 2) return;

     i = ((movefifoplc-1)&(MOVEFIFOSIZ-1));
     p = &ps[myconnectindex];

     if (p->posx == myxbak[i] && p->posy == myybak[i] && p->posz == myzbak[i]
          && p->horiz == myhorizbak[i] && p->ang == myangbak[i]) return;

     myx = p->posx; omyx = p->oposx; myxvel = p->posxv;
     myy = p->posy; omyy = p->oposy; myyvel = p->posyv;
     myz = p->posz; omyz = p->oposz; myzvel = p->poszv;
     myang = p->ang; omyang = p->oang;
     mycursectnum = p->cursectnum;
     myhoriz = p->horiz; omyhoriz = p->ohoriz;
     myhorizoff = p->horizoff; omyhorizoff = p->ohorizoff;
     myjumpingcounter = p->jumping_counter;
     myjumpingtoggle = p->jumping_toggle;
     myonground = p->on_ground;
     myhardlanding = p->hard_landing;
     myreturntocenter = p->return_to_center;

     fakemovefifoplc = movefifoplc;
     while (fakemovefifoplc < movefifoend[myconnectindex])
          fakedomovethings();

}

void fakedomovethings(void)
{
        input *syn;
        struct player_struct *p;
        int32_t i, j, k, doubvel, fz, cz, hz, lz, x, y;
        uint32_t sb_snum;
        short psect, psectlotag, tempsect, backcstat;
        uint8_t  shrunk, spritebridge;

        syn = (input *)&inputfifo[fakemovefifoplc&(MOVEFIFOSIZ-1)][myconnectindex];

        p = &ps[myconnectindex];

        backcstat = sprite[p->i].cstat;
        sprite[p->i].cstat &= ~257;

        sb_snum = syn->bits;

        psect = mycursectnum;
        psectlotag = sector[psect].lotag;
        spritebridge = 0;

        shrunk = (sprite[p->i].yrepeat < 32);

        if( ud.clipping == 0 && ( sector[psect].floorpicnum == MIRROR || psect < 0 || psect >= MAXSECTORS) )
        {
            myx = omyx;
            myy = omyy;
        }
        else
        {
            omyx = myx;
            omyy = myy;
        }

        omyhoriz = myhoriz;
        omyhorizoff = myhorizoff;
        omyz = myz;
        omyang = myang;

        getzrange(myx,myy,myz,psect,&cz,&hz,&fz,&lz,163L,CLIPMASK0);

        j = getflorzofslope(psect,myx,myy);

        if( (lz&49152) == 16384 && psectlotag == 1 && klabs(myz-j) > PHEIGHT+(16<<8) )
            psectlotag = 0;

        if( p->aim_mode == 0 && myonground && psectlotag != 2 && (sector[psect].floorstat&2) )
        {
                x = myx+(sintable[(myang+512)&2047]>>5);
                y = myy+(sintable[myang&2047]>>5);
                tempsect = psect;
                updatesector(x,y,&tempsect);
                if (tempsect >= 0)
                {
                     k = getflorzofslope(psect,x,y);
                     if (psect == tempsect)
                          myhorizoff += mulscale16(j-k,160);
                     else if (klabs(getflorzofslope(tempsect,x,y)-k) <= (4<<8))
                          myhorizoff += mulscale16(j-k,160);
                }
        }
        if (myhorizoff > 0) myhorizoff -= ((myhorizoff>>3)+1);
        else if (myhorizoff < 0) myhorizoff += (((-myhorizoff)>>3)+1);

        if(hz >= 0 && (hz&49152) == 49152)
        {
                hz &= (MAXSPRITES-1);
                if (sprite[hz].statnum == 1 && sprite[hz].extra >= 0)
                {
                    hz = 0;
                    cz = getceilzofslope(psect,myx,myy);
                }
        }

        if(lz >= 0 && (lz&49152) == 49152)
        {
                 j = lz&(MAXSPRITES-1);
                 if ((sprite[j].cstat&33) == 33)
                 {
                        psectlotag = 0;
                        spritebridge = 1;
                 }
                 if(badguy(&sprite[j]) && sprite[j].xrepeat > 24 && klabs(sprite[p->i].z-sprite[j].z) < (84<<8) )
                 {
                    j = getangle( sprite[j].x-myx,sprite[j].y-myy);
                    myxvel -= sintable[(j+512)&2047]<<4;
                    myyvel -= sintable[j&2047]<<4;
                }
        }

        if( sprite[p->i].extra <= 0 )
        {
                 if( psectlotag == 2 )
                 {
                            if(p->on_warping_sector == 0)
                            {
                                     if( klabs(myz-fz) > (PHEIGHT>>1))
                                             myz += 348;
                            }
                            clipmove(&myx,&myy,&myz,&mycursectnum,0,0,164L,(4L<<8),(4L<<8),CLIPMASK0);
                 }

                 updatesector(myx,myy,&mycursectnum);
                 pushmove(&myx,&myy,&myz,&mycursectnum,128L,(4L<<8),(20L<<8),CLIPMASK0);

                myhoriz = 100;
                myhorizoff = 0;

                 goto ENDFAKEPROCESSINPUT;
        }

        doubvel = TICSPERFRAME;

        if(p->on_crane >= 0) goto FAKEHORIZONLY;

        if(p->one_eighty_count < 0) myang += 128;

        i = 40;

        if( psectlotag == 2)
        {
                 myjumpingcounter = 0;

                 if ( sb_snum&1 )
                 {
                            if(myzvel > 0) myzvel = 0;
                            myzvel -= 348;
                            if(myzvel < -(256*6)) myzvel = -(256*6);
                 }
                 else if (sb_snum&(1<<1))
                 {
                            if(myzvel < 0) myzvel = 0;
                            myzvel += 348;
                            if(myzvel > (256*6)) myzvel = (256*6);
                 }
                 else
                 {
                    if(myzvel < 0)
                    {
                        myzvel += 256;
                        if(myzvel > 0)
                            myzvel = 0;
                    }
                    if(myzvel > 0)
                    {
                        myzvel -= 256;
                        if(myzvel < 0)
                            myzvel = 0;
                    }
                }

                if(myzvel > 2048) myzvel >>= 1;

                 myz += myzvel;

                 if(myz > (fz-(15<<8)) )
                            myz += ((fz-(15<<8))-myz)>>1;

                 if(myz < (cz+(4<<8)) )
                 {
                            myz = cz+(4<<8);
                            myzvel = 0;
                 }
        }

        else if(p->jetpack_on)
        {
                 myonground = 0;
                 myjumpingcounter = 0;
                 myhardlanding = 0;

                 if(p->jetpack_on < 11)
                            myz -= (p->jetpack_on<<7); //Goin up

                 if(shrunk) j = 512;
                 else j = 2048;

                 if (sb_snum&1)                            //A
                            myz -= j;
                 if (sb_snum&(1<<1))                       //Z
                            myz += j;

                 if(shrunk == 0 && ( psectlotag == 0 || psectlotag == 2 ) ) k = 32;
                 else k = 16;

                 if(myz > (fz-(k<<8)) )
                            myz += ((fz-(k<<8))-myz)>>1;
                 if(myz < (cz+(18<<8)) )
                            myz = cz+(18<<8);
        }
        else if( psectlotag != 2 )
        {
            if (psectlotag == 1 && p->spritebridge == 0)
            {
                 if(shrunk == 0) i = 34;
                 else i = 12;
            }
                 if(myz < (fz-(i<<8)) && (floorspace(psect)|ceilingspace(psect)) == 0 ) //falling
                 {
                            if( (sb_snum&3) == 0 && myonground && (sector[psect].floorstat&2) && myz >= (fz-(i<<8)-(16<<8) ) )
                                     myz = fz-(i<<8);
                            else
                            {
                                     myonground = 0;

                                     myzvel += (gc+80);

                                     if(myzvel >= (4096+2048)) myzvel = (4096+2048);
                            }
                 }

                 else
                 {
                            if(psectlotag != 1 && psectlotag != 2 && myonground == 0 && myzvel > (6144>>1))
                                 myhardlanding = myzvel>>10;
                            myonground = 1;

                            if(i==40)
                            {
                                     //Smooth on the ground

                                     k = ((fz-(i<<8))-myz)>>1;
                                     if( klabs(k) < 256 ) k = 0;
                                     myz += k; // ((fz-(i<<8))-myz)>>1;
                                     myzvel -= 768; // 412;
                                     if(myzvel < 0) myzvel = 0;
                            }
                            else if(myjumpingcounter == 0)
                            {
                                myz += ((fz-(i<<7))-myz)>>1; //Smooth on the water
                                if(p->on_warping_sector == 0 && myz > fz-(16<<8))
                                {
                                    myz = fz-(16<<8);
                                    myzvel >>= 1;
                                }
                            }

                            if( sb_snum&2 )
                                     myz += (2048+768);

                            if( (sb_snum&1) == 0 && myjumpingtoggle == 1)
                                     myjumpingtoggle = 0;

                            else if( (sb_snum&1) && myjumpingtoggle == 0 )
                            {
                                     if( myjumpingcounter == 0 )
                                             if( (fz-cz) > (56<<8) )
                                             {
                                                myjumpingcounter = 1;
                                                myjumpingtoggle = 1;
                                             }
                            }
                            if( myjumpingcounter && (sb_snum&1) == 0 )
                                myjumpingcounter = 0;
                 }

                 if(myjumpingcounter)
                 {
                            if( (sb_snum&1) == 0 && myjumpingtoggle == 1)
                                     myjumpingtoggle = 0;

                            if( myjumpingcounter < (1024+256) )
                            {
                                     if(psectlotag == 1 && myjumpingcounter > 768)
                                     {
                                             myjumpingcounter = 0;
                                             myzvel = -512;
                                     }
                                     else
                                     {
                                             myzvel -= (sintable[(2048-128+myjumpingcounter)&2047])/12;
                                             myjumpingcounter += 180;

                                             myonground = 0;
                                     }
                            }
                            else
                            {
                                     myjumpingcounter = 0;
                                     myzvel = 0;
                            }
                 }

                 myz += myzvel;

                 if(myz < (cz+(4<<8)) )
                 {
                            myjumpingcounter = 0;
                            if(myzvel < 0) myxvel = myyvel = 0;
                            myzvel = 128;
                            myz = cz+(4<<8);
                 }

        }

        if ( p->fist_incs ||
                     p->transporter_hold > 2 ||
                     myhardlanding ||
                     p->access_incs > 0 ||
                     p->knee_incs > 0 ||
                     (p->curr_weapon == TRIPBOMB_WEAPON &&
                      p->kickback_pic > 1 &&
                      p->kickback_pic < 4 ) )
        {
                 doubvel = 0;
                 myxvel = 0;
                 myyvel = 0;
        }
        else if ( syn->avel )          //p->ang += syncangvel * constant
        {                         //ENGINE calculates angvel for you
            int32_t tempang;

            tempang = syn->avel<<1;

            if(psectlotag == 2)
                myang += (tempang-(tempang>>3))*sgn(doubvel);
            else myang += (tempang)*sgn(doubvel);
            myang &= 2047;
        }

        if ( myxvel || myyvel || syn->fvel || syn->svel )
        {
                 if(p->steroids_amount > 0 && p->steroids_amount < 400)
                     doubvel <<= 1;

                 myxvel += ((syn->fvel*doubvel)<<6);
                 myyvel += ((syn->svel*doubvel)<<6);

                 if( ( p->curr_weapon == KNEE_WEAPON && p->kickback_pic > 10 && myonground ) || ( myonground && (sb_snum&2) ) )
                 {
                            myxvel = mulscale16(myxvel,dukefriction-0x2000);
                            myyvel = mulscale16(myyvel,dukefriction-0x2000);
                 }
                 else
                 {
                    if(psectlotag == 2)
                    {
                        myxvel = mulscale16(myxvel,dukefriction-0x1400);
                        myyvel = mulscale16(myyvel,dukefriction-0x1400);
                    }
                    else
                    {
                        myxvel = mulscale16(myxvel,dukefriction);
                        myyvel = mulscale16(myyvel,dukefriction);
                    }
                 }

                 if( abs(myxvel) < 2048 && abs(myyvel) < 2048 )
                     myxvel = myyvel = 0;

                 if( shrunk )
                 {
                     myxvel =
                         mulscale16(myxvel,(dukefriction)-(dukefriction>>1)+(dukefriction>>2));
                     myyvel =
                         mulscale16(myyvel,(dukefriction)-(dukefriction>>1)+(dukefriction>>2));
                 }
        }

FAKEHORIZONLY:
        if(psectlotag == 1 || spritebridge == 1) i = (4L<<8); else i = (20L<<8);

        clipmove(&myx,&myy,&myz,&mycursectnum,myxvel,myyvel,164L,4L<<8,i,CLIPMASK0);
        pushmove(&myx,&myy,&myz,&mycursectnum,164L,4L<<8,4L<<8,CLIPMASK0);

        if( p->jetpack_on == 0 && psectlotag != 1 && psectlotag != 2 && shrunk)
            myz += 30<<8;

        if ((sb_snum&(1<<18)) || myhardlanding)
            myreturntocenter = 9;

        if (sb_snum&(1<<13))
        {
                myreturntocenter = 9;
                if (sb_snum&(1<<5)) myhoriz += 6;
                myhoriz += 6;
        }
        else if (sb_snum&(1<<14))
        {
                myreturntocenter = 9;
                if (sb_snum&(1<<5)) myhoriz -= 6;
                myhoriz -= 6;
        }
        else if (sb_snum&(1<<3))
        {
                if (sb_snum&(1<<5)) myhoriz += 6;
                myhoriz += 6;
        }
        else if (sb_snum&(1<<4))
        {
                if (sb_snum&(1<<5)) myhoriz -= 6;
                myhoriz -= 6;
        }

        if (myreturntocenter > 0)
            if ((sb_snum&(1<<13)) == 0 && (sb_snum&(1<<14)) == 0)
        {
             myreturntocenter--;
             myhoriz += 33-(myhoriz/3);
        }

        if(p->aim_mode)
            myhoriz += syn->horz>>1;
        else
        {
            if( myhoriz > 95 && myhoriz < 105) myhoriz = 100;
            if( myhorizoff > -5 && myhorizoff < 5) myhorizoff = 0;
        }

        if (myhardlanding > 0)
        {
            myhardlanding--;
            myhoriz -= (myhardlanding<<4);
        }

        if (myhoriz > 299) myhoriz = 299;
        else if (myhoriz < -99) myhoriz = -99;

        if(p->knee_incs > 0)
        {
            myhoriz -= 48;
            myreturntocenter = 9;
        }


ENDFAKEPROCESSINPUT:

        myxbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myx;
        myybak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myy;
        myzbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myz;
        myangbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myang;
        myhorizbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myhoriz;
        fakemovefifoplc++;

        sprite[p->i].cstat = backcstat;
}


uint8_t  domovethings(void)
{
    short i, j;
    uint8_t  ch;


    for(i=connecthead;i>=0;i=connectpoint2[i])
        if( sync[i].bits&(1<<17) )
    {
        multiflag = 2;
        multiwhat = (sync[i].bits>>18)&1;
        multipos = (uint32_t) (sync[i].bits>>19)&15;
        multiwho = i;

        if( multiwhat )
        {
			// FIX_00058: Save/load game crash in both single and multiplayer
            screencapt = 1;
            displayrooms(myconnectindex,65536);
            savetemp("duke3d.tmp",tiles[MAXTILES-1].data,160*100);
            screencapt = 0;

            saveplayer( multipos );
            multiflag = 0;

            if(multiwho != myconnectindex)
            {
                strcpy(fta_quotes[122],&ud.user_name[multiwho][0]);
                strcat(fta_quotes[122]," SAVED A MULTIPLAYER GAME");
                FTA(122,&ps[myconnectindex],1);
            }
            else
            {
                strcpy(fta_quotes[122],"MULTIPLAYER GAME SAVED");
                FTA(122,&ps[myconnectindex],1);
            }
            break;
        }
        else
        {
//            waitforeverybody();

            j = loadplayer( multipos );

            multiflag = 0;

            if(j == 0)
            {
                if(multiwho != myconnectindex)
                {
                    strcpy(fta_quotes[122],&ud.user_name[multiwho][0]);
                    strcat(fta_quotes[122]," LOADED A MULTIPLAYER GAME");
                    FTA(122,&ps[myconnectindex],1);
                }
                else
                {
                    strcpy(fta_quotes[122],"MULTIPLAYER GAME LOADED");
                    FTA(122,&ps[myconnectindex],1);
                }
                return 1;
            }
        }
    }

    ud.camerasprite = -1;
    lockclock += TICSPERFRAME;

    if(earthquaketime > 0) earthquaketime--;
    if(rtsplaying > 0) rtsplaying--;

    for(i=0;i<MAXUSERQUOTES;i++)
         if (user_quote_time[i])
         {
             user_quote_time[i]--;
             if (!user_quote_time[i]) pub = NUMPAGES;
         }
     if ((klabs(quotebotgoal-quotebot) <= 16) && (ud.screen_size <= 8))
         quotebot += ksgn(quotebotgoal-quotebot);
     else
         quotebot = quotebotgoal;

    if( show_shareware > 0 )
    {
        show_shareware--;
        if(show_shareware == 0)
        {
            pus = NUMPAGES;
            pub = NUMPAGES;
        }
    }

    everyothertime++;

    for(i=connecthead;i>=0;i=connectpoint2[i])
        copybufbyte(&inputfifo[movefifoplc&(MOVEFIFOSIZ-1)][i],&sync[i],sizeof(input));
    movefifoplc++;

    updateinterpolations();

    j = -1;
    for(i=connecthead;i>=0;i=connectpoint2[i])
     {
          if ((sync[i].bits&(1<<26)) == 0) { j = i; continue; }

          closedemowrite();

          if (i == myconnectindex) gameexit(" ");
          if (screenpeek == i)
          {
                screenpeek = connectpoint2[i];
                if (screenpeek < 0) screenpeek = connecthead;
          }

          if (i == connecthead) connecthead = connectpoint2[connecthead];
          else connectpoint2[j] = connectpoint2[i];

          numplayers--;
          ud.multimode--;

          if (numplayers < 2)
              sound(GENERIC_AMBIENCE17);

          pub = NUMPAGES;
          pus = NUMPAGES;
          vscrn();

          sprintf(buf,"%s is history!",ud.user_name[i]);

          quickkill(&ps[i]);
          deletesprite(ps[i].i);

          adduserquote(buf);

          if(j < 0 && networkmode == 0 )
              gameexit( " \nThe 'MASTER/First player' just quit the game.  All\nplayers are returned from the game. This only happens in 5-8\nplayer mode as a different network scheme is used.");
      }

      if ((numplayers >= 2) && ((movefifoplc&7) == 7))
      {
            ch = (uint8_t )(randomseed&255);
            for(i=connecthead;i>=0;i=connectpoint2[i])
                 ch += ((ps[i].posx+ps[i].posy+ps[i].posz+ps[i].ang+ps[i].horiz)&255);
            syncval[myconnectindex][syncvalhead[myconnectindex]&(MOVEFIFOSIZ-1)] = ch;
            syncvalhead[myconnectindex]++;


      }

    if(ud.recstat == 1) record();

    if( ud.pause_on == 0 )
    {
        global_random = TRAND;
        movedummyplayers();//ST 13
    }

    for(i=connecthead;i>=0;i=connectpoint2[i])
    {
        cheatkeys(i);

        if( ud.pause_on == 0 )
        {
            processinput(i);
            checksectors(i);
        }
    }

    if( ud.pause_on == 0 )
    {
        movefta();//ST 2
        moveweapons();          //ST 5 (must be last)
        movetransports();       //ST 9

        moveplayers();          //ST 10
        movefallers();          //ST 12
        moveexplosions();       //ST 4

        moveactors();           //ST 1
        moveeffectors();        //ST 3

        movestandables();       //ST 6
        doanimations();
        movefx();               //ST 11
    }

    fakedomovethingscorrect();

    if( (everyothertime&1) == 0)
    {
        animatewalls();
        movecyclers();
        pan3dsound();
    }


    return 0;
}


void doorders(void)
{
    short i;

    setview(0,0,xdim-1,ydim-1);

    for(i=0;i<63;i+=7) palto(0,0,0,i);
    ps[myconnectindex].palette = palette;
    totalclock = 0;
    KB_FlushKeyboardQueue();
    rotatesprite(0,0,65536L,0,ORDERING,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;while( !KB_KeyWaiting() ) getpackets();

    for(i=0;i<63;i+=7) palto(0,0,0,i);
    totalclock = 0;
    KB_FlushKeyboardQueue();
    rotatesprite(0,0,65536L,0,ORDERING+1,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;while( !KB_KeyWaiting() ) getpackets();

    for(i=0;i<63;i+=7) palto(0,0,0,i);
    totalclock = 0;
    KB_FlushKeyboardQueue();
    rotatesprite(0,0,65536L,0,ORDERING+2,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;while( !KB_KeyWaiting() ) getpackets();

    for(i=0;i<63;i+=7) palto(0,0,0,i);
    totalclock = 0;
    KB_FlushKeyboardQueue();
    rotatesprite(0,0,65536L,0,ORDERING+3,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
    nextpage(); for(i=63;i>0;i-=7) palto(0,0,0,i);
    totalclock = 0;while( !KB_KeyWaiting() ) getpackets();
}

void dobonus(uint8_t  bonusonly)
{
    short t, gfx_offset;
//    short tinc;
    int32_t i, y,xfragtotal,yfragtotal;
    short bonuscnt;
    char text[512];

    int32_t breathe[] =
    {
         0,  30,VICTORY1+1,176,59,
        30,  60,VICTORY1+2,176,59,
        60,  90,VICTORY1+1,176,59,
        90, 120,0         ,176,59
    };

    int32_t bossmove[] =
    {
         0, 120,VICTORY1+3,86,59,
       220, 260,VICTORY1+4,86,59,
       260, 290,VICTORY1+5,86,59,
       290, 320,VICTORY1+6,86,59,
       320, 350,VICTORY1+7,86,59,
       350, 380,VICTORY1+8,86,59
    };

    bonuscnt = 0;

    for(t=0;t<64;t+=7) palto(0,0,0,t);
    setview(0,0,xdim-1,ydim-1);
    clearview(0L);
    nextpage();
    flushperms();

    FX_StopAllSounds();
    clearsoundlocks();
    FX_SetReverb(0L);

    if(bonusonly) goto FRAGBONUS;

    if(numplayers < 2 && ud.eog && ud.from_bonus == 0)
        switch(ud.volume_number)
    {
        case 0:
            if(ud.lockout == 0)
            {
                clearview(0L);
                rotatesprite(0,50<<16,65536L,0,VICTORY1,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                nextpage();
                ps[myconnectindex].palette = endingpal;
                for(t=63;t>=0;t--) palto(0,0,0,t);

                KB_FlushKeyboardQueue();
                totalclock = 0; //tinc = 0;
                while( 1 )
                {
                    clearview(0L);
                    rotatesprite(0,50<<16,65536L,0,VICTORY1,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

                    // boss
                    if( totalclock > 390 && totalclock < 780 )
                        for(t=0;t<35;t+=5) if( bossmove[t+2] && (totalclock%390) > bossmove[t] && (totalclock%390) <= bossmove[t+1] )
                    {
                        if(t==10 && bonuscnt == 1) { sound(SHOTGUN_FIRE);sound(SQUISHED); bonuscnt++; }
                        rotatesprite(bossmove[t+3]<<16,bossmove[t+4]<<16,65536L,0,bossmove[t+2],0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                    }

                    // Breathe
                    if( totalclock < 450 || totalclock >= 750 )
                    {
                        if(totalclock >= 750)
                        {
                            rotatesprite(86<<16,59<<16,65536L,0,VICTORY1+8,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                            if(totalclock >= 750 && bonuscnt == 2) { sound(DUKETALKTOBOSS); bonuscnt++; }
                        }
                        for(t=0;t<20;t+=5)
                            if( breathe[t+2] && (totalclock%120) > breathe[t] && (totalclock%120) <= breathe[t+1] )
                        {
                                if(t==5 && bonuscnt == 0)
                                {
                                    sound(BOSSTALKTODUKE);
                                    bonuscnt++;
                                }
                                rotatesprite(breathe[t+3]<<16,breathe[t+4]<<16,65536L,0,breathe[t+2],0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                        }
                    }

                    getpackets();
                    nextpage();
                    if( KB_KeyWaiting() ) break;
                }
            }

            for(t=0;t<64;t++) palto(0,0,0,t);

            KB_FlushKeyboardQueue();
            ps[myconnectindex].palette = palette;

            rotatesprite(0,0,65536L,0,3292,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
            nextpage(); for(t=63;t>0;t--) palto(0,0,0,t);
            while( !KB_KeyWaiting() ) getpackets();
            for(t=0;t<64;t++) palto(0,0,0,t);
            MUSIC_StopSong();
            FX_StopAllSounds();
            clearsoundlocks();
            break;
        case 1:
            MUSIC_StopSong();
            clearview(0L);
            nextpage();

            if(ud.lockout == 0)
            {
                playanm("cineov2.anm",1);
                KB_FlushKeyboardQueue();
                clearview(0L);
                nextpage();
            }

            sound(PIPEBOMB_EXPLODE);

            for(t=0;t<64;t++) palto(0,0,0,t);
            setview(0,0,xdim-1,ydim-1);
            KB_FlushKeyboardQueue();
            ps[myconnectindex].palette = palette;
            rotatesprite(0,0,65536L,0,3293,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
            nextpage(); for(t=63;t>0;t--) palto(0,0,0,t);
            while( !KB_KeyWaiting() ) getpackets();
            for(t=0;t<64;t++) palto(0,0,0,t);

            break;

        case 3:

            setview(0,0,xdim-1,ydim-1);

            MUSIC_StopSong();
            clearview(0L);
            nextpage();

            if(ud.lockout == 0)
            {
                KB_FlushKeyboardQueue();
                playanm("vol4e1.anm",8);
                clearview(0L);
                nextpage();
                playanm("vol4e2.anm",10);
                clearview(0L);
                nextpage();
                playanm("vol4e3.anm",11);
                clearview(0L);
                nextpage();
            }

            FX_StopAllSounds();
            clearsoundlocks();
            sound(ENDSEQVOL3SND4);
            KB_FlushKeyboardQueue();

            ps[myconnectindex].palette = palette;
            palto(0,0,0,63);
            clearview(0L);
            menutext(160,60,0,0,"THANKS TO ALL OUR");
            menutext(160,60+16,0,0,"FANS FOR GIVING");
            menutext(160,60+16+16,0,0,"US BIG HEADS.");
            menutext(160,70+16+16+16,0,0,"LOOK FOR A DUKE NUKEM 3D");
            menutext(160,70+16+16+16+16,0,0,"SEQUEL SOON.");
            nextpage();

            for(t=63;t>0;t-=3) palto(0,0,0,t);
            KB_FlushKeyboardQueue();
            while(!KB_KeyWaiting()) getpackets();
            for(t=0;t<64;t+=3) palto(0,0,0,t);

            clearview(0L);
            nextpage();

            playanm("DUKETEAM.ANM",4);

            KB_FlushKeyboardQueue();
            while(!KB_KeyWaiting()) getpackets();

            clearview(0L);
            nextpage();
            palto(0,0,0,63);

            FX_StopAllSounds();
            clearsoundlocks();
            KB_FlushKeyboardQueue();

            break;

        case 2:

            MUSIC_StopSong();
            clearview(0L);
            nextpage();
            if(ud.lockout == 0)
            {
                for(t=63;t>=0;t--) palto(0,0,0,t);
                playanm("cineov3.anm",2);
                KB_FlushKeyboardQueue();
                ototalclock = totalclock+200;
                while(totalclock < ototalclock) getpackets();
                clearview(0L);
                nextpage();

                FX_StopAllSounds();
                clearsoundlocks();
            }

            playanm("RADLOGO.ANM",3);

            if( ud.lockout == 0 && !KB_KeyWaiting() )
            {
                sound(ENDSEQVOL3SND5);
                while(Sound[ENDSEQVOL3SND5].lock>=200) getpackets();
                if(KB_KeyWaiting()) goto ENDANM;
                sound(ENDSEQVOL3SND6);
                while(Sound[ENDSEQVOL3SND6].lock>=200) getpackets();
                if(KB_KeyWaiting()) goto ENDANM;
                sound(ENDSEQVOL3SND7);
                while(Sound[ENDSEQVOL3SND7].lock>=200) getpackets();
                if(KB_KeyWaiting()) goto ENDANM;
                sound(ENDSEQVOL3SND8);
                while(Sound[ENDSEQVOL3SND8].lock>=200) getpackets();
                if(KB_KeyWaiting()) goto ENDANM;
                sound(ENDSEQVOL3SND9);
                while(Sound[ENDSEQVOL3SND9].lock>=200) getpackets();
            }

            KB_FlushKeyboardQueue();
            totalclock = 0;
            while(!KB_KeyWaiting() && totalclock < 120) getpackets();

            ENDANM:

            FX_StopAllSounds();
            clearsoundlocks();

            KB_FlushKeyboardQueue();

            clearview(0L);

            break;
    }

    FRAGBONUS:

    ps[myconnectindex].palette = palette;
    KB_FlushKeyboardQueue();
    totalclock = 0; //tinc = 0;
    bonuscnt = 0;

    MUSIC_StopSong();
    FX_StopAllSounds();
    clearsoundlocks();

    if(playerswhenstarted > 1 && ud.coop != 1 )
    {
        if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
            sound(BONUSMUSIC);

        rotatesprite(0,0,65536L,0,MENUSCREEN,16,0,2+8+16+64,0,0,xdim-1,ydim-1);
        rotatesprite(160<<16,34<<16,65536L,0,INGAMEDUKETHREEDEE,0,0,10,0,0,xdim-1,ydim-1);
        rotatesprite((260)<<16,36<<16,65536L,0,PLUTOPAKSPRITE+2,0,0,2+8,0,0,xdim-1,ydim-1);
        gametext(160,58+2,"MULTIPLAYER TOTALS",0,2+8+16);
        gametext(160,58+10,level_names[(ud.volume_number*11)+ud.last_level-1],0,2+8+16);

        gametext(160,165,"PRESS ANY KEY TO CONTINUE",0,2+8+16);


        t = 0;
        minitext(23,80,"   NAME                                           KILLS",8,2+8+16+128);
        for(i=0;i<playerswhenstarted;i++)
        {
            sprintf(text,"%-4d",i+1);
            minitext(92+(i*23),80,text,3,2+8+16+128);
        }

        for(i=0;i<playerswhenstarted;i++)
        {
            xfragtotal = 0;
            sprintf(text,"%d",i+1);

            minitext(30,90+t,text,0,2+8+16+128);
            minitext(38,90+t,ud.user_name[i],ps[i].palookup,2+8+16+128);

            for(y=0;y<playerswhenstarted;y++)
            {
                if(i == y)
                {
                    sprintf(text,"%-4d",ps[y].fraggedself);
                    minitext(92+(y*23),90+t,text,2,2+8+16+128);
                    xfragtotal -= ps[y].fraggedself;
                }
                else
                {
                    sprintf(text,"%-4d",frags[i][y]);
                    minitext(92+(y*23),90+t,text,0,2+8+16+128);
                    xfragtotal += frags[i][y];
                }

                if(myconnectindex == connecthead)
                {
                    sprintf(text,"stats %d killed %d %d\n",i+1,y+1,frags[i][y]);
                    sendscore(text);
                }
            }

            sprintf(text,"%-4d",xfragtotal);
            minitext(101+(8*23),90+t,text,2,2+8+16+128);

            t += 7;
        }

        for(y=0;y<playerswhenstarted;y++)
        {
            yfragtotal = 0;
            for(i=0;i<playerswhenstarted;i++)
            {
                if(i == y)
                    yfragtotal += ps[i].fraggedself;
                yfragtotal += frags[i][y];
            }
            sprintf(text,"%-4d",yfragtotal);
            minitext(92+(y*23),96+(8*7),text,2,2+8+16+128);
        }

        minitext(45,96+(8*7),"DEATHS",8,2+8+16+128);
        nextpage();

        for(t=0;t<64;t+=7)
            palto(0,0,0,63-t);

        KB_FlushKeyboardQueue();
        while(KB_KeyWaiting()==0) getpackets();

        if( KB_KeyPressed( sc_F12 ) )
        {
            KB_ClearKeyDown( sc_F12 );
            takescreenshot();
        }

        if(bonusonly || ud.multimode > 1) return;

        for(t=0;t<64;t+=7) palto(0,0,0,t);
    }

    if(bonusonly || ud.multimode > 1) return;

    switch(ud.volume_number)
    {
        case 1:
            gfx_offset = 5;
            break;
        default:
            gfx_offset = 0;
            break;
    }

    rotatesprite(0,0,65536L,0,BONUSSCREEN+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

    menutext(160,20-6,0,0,&level_names[(ud.volume_number*11)+ud.last_level-1][0]);
    menutext(160,36-6,0,0,"COMPLETED");

    gametext(160,192,"PRESS ANY KEY TO CONTINUE",16,2+8+16);

    if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
        sound(BONUSMUSIC);

    nextpage();
    KB_FlushKeyboardQueue();
    for(t=0;t<64;t++) palto(0,0,0,63-t);
    bonuscnt = 0;
    totalclock = 0; //tinc = 0;

    while( 1 )
    {
		sampletimer();
        if(ps[myconnectindex].gm&MODE_EOL)
        {
            rotatesprite(0,0,65536L,0,BONUSSCREEN+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

            if( totalclock > (1000000000L) && totalclock < (1000000320L) )
            {
                switch( (totalclock>>4)%15 )
                {
                    case 0:
                        if(bonuscnt == 6)
                        {
                            bonuscnt++;
                            sound(SHOTGUN_COCK);
                            switch(rand()&3)
                            {
                                case 0:
                                    sound(BONUS_SPEECH1);
                                    break;
                                case 1:
                                    sound(BONUS_SPEECH2);
                                    break;
                                case 2:
                                    sound(BONUS_SPEECH3);
                                    break;
                                case 3:
                                    sound(BONUS_SPEECH4);
                                    break;
                            }
                        }
                    case 1:
                    case 4:
                    case 5:
                        rotatesprite(199<<16,31<<16,65536L,0,BONUSSCREEN+3+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                        break;
                    case 2:
                    case 3:
                       rotatesprite(199<<16,31<<16,65536L,0,BONUSSCREEN+4+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                       break;
                }
            }
            else if( totalclock > (10240+120L) ) break;
            else
            {
                switch( (totalclock>>5)&3 )
                {
                    case 1:
                    case 3:
                        rotatesprite(199<<16,31<<16,65536L,0,BONUSSCREEN+1+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                        break;
                    case 2:
                        rotatesprite(199<<16,31<<16,65536L,0,BONUSSCREEN+2+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
                        break;
                }
            }

            menutext(160,20-6,0,0,&level_names[(ud.volume_number*11)+ud.last_level-1][0]);
            menutext(160,36-6,0,0,"COMPLETED");

            gametext(160,192,"PRESS ANY KEY TO CONTINUE",16,2+8+16);

            if( totalclock > (60*3) )
            {
                gametext(10,59+9,"Your Time:",0,2+8+16);
                gametext(10,69+9,"Par time:",0,2+8+16);
                gametext(10,78+9,"3D Realms' Time:",0,2+8+16);
                if(bonuscnt == 0)
                    bonuscnt++;

                if( totalclock > (60*4) )
                {
                    if(bonuscnt == 1)
                    {
                        bonuscnt++;
                        sound(PIPEBOMB_EXPLODE);
                    }
                    sprintf(text,"%02d:%02d",
                        (ps[myconnectindex].player_par/(26*60))%60,
                        (ps[myconnectindex].player_par/26)%60);
                    gametext((320>>2)+71,60+9,text,0,2+8+16);

                    sprintf(text,"%02d:%02d",
                        (partime[ud.volume_number*11+ud.last_level-1]/(26*60))%60,
                        (partime[ud.volume_number*11+ud.last_level-1]/26)%60);
                    gametext((320>>2)+71,69+9,text,0,2+8+16);

                    sprintf(text,"%02d:%02d",
                        (designertime[ud.volume_number*11+ud.last_level-1]/(26*60))%60,
                        (designertime[ud.volume_number*11+ud.last_level-1]/26)%60);
                    gametext((320>>2)+71,78+9,text,0,2+8+16);

                }
            }
            if( totalclock > (60*6) )
            {
                gametext(10,94+9,"Enemies Killed:",0,2+8+16);
                gametext(10,99+4+9,"Enemies Left:",0,2+8+16);

                if(bonuscnt == 2)
                {
                    bonuscnt++;
                    sound(FLY_BY);
                }

                if( totalclock > (60*7) )
                {
                    if(bonuscnt == 3)
                    {
                        bonuscnt++;
                        sound(PIPEBOMB_EXPLODE);
                    }
                    sprintf(text,"%-3hhd",ps[myconnectindex].actors_killed);
                    gametext((320>>2)+70,93+9,text,0,2+8+16);
                    if(ud.player_skill > 3 )
                    {
                        sprintf(text,"N/A");
                        gametext((320>>2)+70,99+4+9,text,0,2+8+16);
                    }
                    else
                    {
                        if( (ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed) < 0 )
                            sprintf(text,"%-3d",0);
                        else sprintf(text,"%-3d",ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed);
                        gametext((320>>2)+70,99+4+9,text,0,2+8+16);
                    }
                }
            }
            if( totalclock > (60*9) )
            {
                gametext(10,120+9,"Secrets Found:",0,2+8+16);
                gametext(10,130+9,"Secrets Missed:",0,2+8+16);
                if(bonuscnt == 4) bonuscnt++;

                if( totalclock > (60*10) )
                {
                    if(bonuscnt == 5)
                    {
                        bonuscnt++;
                        sound(PIPEBOMB_EXPLODE);
                    }
                    sprintf(text,"%-3d",ps[myconnectindex].secret_rooms);
                    gametext((320>>2)+70,120+9,text,0,2+8+16);
                    if( ps[myconnectindex].secret_rooms > 0 )
                        sprintf(text,"%-3d",(100*ps[myconnectindex].secret_rooms/ps[myconnectindex].max_secret_rooms));
                    sprintf(text,"%-3d",ps[myconnectindex].max_secret_rooms-ps[myconnectindex].secret_rooms);
                    gametext((320>>2)+70,130+9,text,0,2+8+16);
                }
            }

            if(totalclock > 10240 && totalclock < 10240+10240)
                totalclock = 1024;

            if( KB_KeyWaiting() && totalclock > (60*2) )
            {
                if( KB_KeyPressed( sc_F12 ) )
                {
                    KB_ClearKeyDown( sc_F12 );
                    takescreenshot();
                }

                if( totalclock < (60*13) )
                {
                    KB_FlushKeyboardQueue();
                    totalclock = (60*13);
                }
                else if( totalclock < (1000000000L))
                   totalclock = (1000000000L);
            }
        }
        else break;
        nextpage();
    }
}


void cameratext(short i)
{
    uint8_t  flipbits;
    int32_t x , y;

    if(!T1)
    {
        rotatesprite(24<<16,33<<16,65536L,0,CAMCORNER,0,0,2,windowx1,windowy1,windowx2,windowy2);
        rotatesprite((320-26)<<16,34<<16,65536L,0,CAMCORNER+1,0,0,2,windowx1,windowy1,windowx2,windowy2);
        rotatesprite(22<<16,163<<16,65536L,512,CAMCORNER+1,0,0,2+4,windowx1,windowy1,windowx2,windowy2);
        rotatesprite((310-10)<<16,163<<16,65536L,512,CAMCORNER+1,0,0,2,windowx1,windowy1,windowx2,windowy2);
        if(totalclock&16)
            rotatesprite(46<<16,32<<16,65536L,0,CAMLIGHT,0,0,2,windowx1,windowy1,windowx2,windowy2);
    }
    else
    {
        flipbits = (totalclock<<1)&48;
        for(x=0;x<394;x+=64)
            for(y=0;y<200;y+=64)
                rotatesprite(x<<16,y<<16,65536L,0,STATIC,0,0,2+flipbits,windowx1,windowy1,windowx2,windowy2);
    }
}

void vglass(int32_t x,int32_t y,short a,short wn,short n)
{
    int32_t z, zincs;
    short sect;

    sect = wall[wn].nextsector;
    if(sect == -1) return;
    zincs = ( sector[sect].floorz-sector[sect].ceilingz ) / n;

    for(z = sector[sect].ceilingz;z < sector[sect].floorz; z += zincs )
        EGS(sect,x,y,z-(TRAND&8191),GLASSPIECES+(z&(TRAND%3)),-32,36,36,a+128-(TRAND&255),16+(TRAND&31),0,-1,5);
}

void lotsofglass(short i,short wallnum,short n)
{
     int32_t j, xv, yv, z, x1, y1;
     short sect, a;

     sect = -1;

     if(wallnum < 0)
     {
        for(j=n-1; j >= 0 ;j--)
        {
            a = SA-256+(TRAND&511)+1024;
            EGS(SECT,SX,SY,SZ,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),1024-(TRAND&1023),i,5);
        }
        return;
     }

     j = n+1;

     x1 = wall[wallnum].x;
     y1 = wall[wallnum].y;

     xv = wall[wall[wallnum].point2].x-x1;
     yv = wall[wall[wallnum].point2].y-y1;

     x1 -= ksgn(yv);
     y1 += ksgn(xv);

     xv /= j;
     yv /= j;

     for(j=n;j>0;j--)
         {
                  x1 += xv;
                  y1 += yv;

          updatesector(x1,y1,&sect);
          if(sect >= 0)
          {
              z = sector[sect].floorz-(TRAND&(klabs(sector[sect].ceilingz-sector[sect].floorz)));
              if( z < -(32<<8) || z > (32<<8) )
                  z = SZ-(32<<8)+(TRAND&((64<<8)-1));
              a = SA-1024;
              EGS(SECT,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),-(TRAND&1023),i,5);
          }
         }
}

void spriteglass(short i,short n)
{
    int32_t j, k, a, z;

    for(j=n;j>0;j--)
    {
        a = TRAND&2047;
        z = SZ-((TRAND&16)<<8);
        k = EGS(SECT,SX,SY,z,GLASSPIECES+(j%3),TRAND&15,36,36,a,32+(TRAND&63),-512-(TRAND&2047),i,5);
        sprite[k].pal = sprite[i].pal;
    }
}

void ceilingglass(short i,short sectnum,short n)
{
     int32_t j, xv, yv, z, x1, y1;
     short a,s, startwall,endwall;

     startwall = sector[sectnum].wallptr;
     endwall = startwall+sector[sectnum].wallnum;

     for(s=startwall;s<(endwall-1);s++)
     {
         x1 = wall[s].x;
         y1 = wall[s].y;

         xv = (wall[s+1].x-x1)/(n+1);
         yv = (wall[s+1].y-y1)/(n+1);

         for(j=n;j>0;j--)
         {
              x1 += xv;
              y1 += yv;
              a = TRAND&2047;
              z = sector[sectnum].ceilingz+((TRAND&15)<<8);
              EGS(sectnum,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,(TRAND&31),0,i,5);
          }
     }
}



void lotsofcolourglass(short i,short wallnum,short n)
{
     int32_t j, xv, yv, z, x1, y1;
     short sect = -1, a, k;

     if(wallnum < 0)
     {
        for(j=n-1; j >= 0 ;j--)
        {
            a = TRAND&2047;
            k = EGS(SECT,SX,SY,SZ-(TRAND&(63<<8)),GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),1024-(TRAND&2047),i,5);
            sprite[k].pal = TRAND&15;
        }
        return;
     }

     j = n+1;
     x1 = wall[wallnum].x;
     y1 = wall[wallnum].y;

     xv = (wall[wall[wallnum].point2].x-wall[wallnum].x)/j;
     yv = (wall[wall[wallnum].point2].y-wall[wallnum].y)/j;

     for(j=n;j>0;j--)
         {
                  x1 += xv;
                  y1 += yv;

          updatesector(x1,y1,&sect);
          z = sector[sect].floorz-(TRAND&(klabs(sector[sect].ceilingz-sector[sect].floorz)));
          if( z < -(32<<8) || z > (32<<8) )
              z = SZ-(32<<8)+(TRAND&((64<<8)-1));
          a = SA-1024;
          k = EGS(SECT,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),-(TRAND&2047),i,5);
          sprite[k].pal = TRAND&7;
         }
}

void SetupGameButtons( void )
{
   CONTROL_DefineFlag(gamefunc_Move_Forward,false);
   CONTROL_DefineFlag(gamefunc_Move_Backward,false);
   CONTROL_DefineFlag(gamefunc_Turn_Left,false);
   CONTROL_DefineFlag(gamefunc_Turn_Right,false);
   CONTROL_DefineFlag(gamefunc_Strafe,false);
   CONTROL_DefineFlag(gamefunc_Fire,false);
   CONTROL_DefineFlag(gamefunc_Open,false);
   CONTROL_DefineFlag(gamefunc_Run,false);
   CONTROL_DefineFlag(gamefunc_AutoRun,false);
   CONTROL_DefineFlag(gamefunc_Jump,false);
   CONTROL_DefineFlag(gamefunc_Crouch,false);
   CONTROL_DefineFlag(gamefunc_Look_Up,false);
   CONTROL_DefineFlag(gamefunc_Look_Down,false);
   CONTROL_DefineFlag(gamefunc_Look_Left,false);
   CONTROL_DefineFlag(gamefunc_Look_Right,false);
   CONTROL_DefineFlag(gamefunc_Strafe_Left,false);
   CONTROL_DefineFlag(gamefunc_Strafe_Right,false);
   CONTROL_DefineFlag(gamefunc_Aim_Up,false);
   CONTROL_DefineFlag(gamefunc_Aim_Down,false);
   CONTROL_DefineFlag(gamefunc_Weapon_1,false);
   CONTROL_DefineFlag(gamefunc_Weapon_2,false);
   CONTROL_DefineFlag(gamefunc_Weapon_3,false);
   CONTROL_DefineFlag(gamefunc_Weapon_4,false);
   CONTROL_DefineFlag(gamefunc_Weapon_5,false);
   CONTROL_DefineFlag(gamefunc_Weapon_6,false);
   CONTROL_DefineFlag(gamefunc_Weapon_7,false);
   CONTROL_DefineFlag(gamefunc_Weapon_8,false);
   CONTROL_DefineFlag(gamefunc_Weapon_9,false);
   CONTROL_DefineFlag(gamefunc_Weapon_10,false);
   CONTROL_DefineFlag(gamefunc_Inventory,false);
   CONTROL_DefineFlag(gamefunc_Inventory_Left,false);
   CONTROL_DefineFlag(gamefunc_Inventory_Right,false);
   CONTROL_DefineFlag(gamefunc_Holo_Duke,false);
   CONTROL_DefineFlag(gamefunc_Jetpack,false);
   CONTROL_DefineFlag(gamefunc_NightVision,false);
   CONTROL_DefineFlag(gamefunc_MedKit,false);
   CONTROL_DefineFlag(gamefunc_TurnAround,false);
   CONTROL_DefineFlag(gamefunc_SendMessage,false);
   CONTROL_DefineFlag(gamefunc_Map,false);
   CONTROL_DefineFlag(gamefunc_Shrink_Screen,false);
   CONTROL_DefineFlag(gamefunc_Enlarge_Screen,false);
   CONTROL_DefineFlag(gamefunc_Center_View,false);
   CONTROL_DefineFlag(gamefunc_Holster_Weapon,false);
   CONTROL_DefineFlag(gamefunc_Show_Opponents_Weapon,false);
   CONTROL_DefineFlag(gamefunc_Map_Follow_Mode,false);
   CONTROL_DefineFlag(gamefunc_See_Coop_View,false);
   CONTROL_DefineFlag(gamefunc_Mouse_Aiming,false);
   CONTROL_DefineFlag(gamefunc_Toggle_Crosshair,false);
   CONTROL_DefineFlag(gamefunc_Steroids,false);
   CONTROL_DefineFlag(gamefunc_Quick_Kick,false);
   CONTROL_DefineFlag(gamefunc_Next_Weapon,false);
   CONTROL_DefineFlag(gamefunc_Previous_Weapon,false);
   CONTROL_DefineFlag(gamefunc_Console,false);
}

/*
===================
=
= GetTime
=
===================
*/

int32_t GetTime(void)
   {
   return totalclock;
   }


/*
===================
=
= CenterCenter
=
===================
*/

void CenterCenter(void)
   {
   printf("Center the joystick and press a button\n");
   }

/*
===================
=
= UpperLeft
=
===================
*/

void UpperLeft(void)
   {
   printf("Move joystick to upper-left corner and press a button\n");
   }

/*
===================
=
= LowerRight
=
===================
*/

void LowerRight(void)
   {
   printf("Move joystick to lower-right corner and press a button\n");
   }

/*
===================
=
= CenterThrottle
=
===================
*/

void CenterThrottle(void)
   {
   printf("Center the throttle control and press a button\n");
   }

/*
===================
=
= CenterRudder
=
===================
*/

void CenterRudder(void)
{
   printf("Center the rudder control and press a button\n");
}

// FIX_00006: better naming system for screenshots + message when pic is taken.
//            Use ./screenshots folder. Screenshot code rerwritten. Faster and
//            makes smaller files. Doesn't freeze or lag the game anymore.
void takescreenshot(void)
{
	char  szFilename[256];
	int i;
	char  score[20];
	time_t time4file;
	struct tm *tmHMS;
    char text[512];

	// xduke: Build a nice name w/ date and players name if in multi mode.
	time(&time4file);
	tmHMS = localtime(&time4file);

	sprintf(text, "Chocolate DukeNukem3D(v%d.%d) %.4d.%.2d.%.2d %.2dh%.2dm%.2ds",
			CHOCOLATE_DUKE_REV_X,
			CHOCOLATE_DUKE_REV_DOT_Y,
			tmHMS->tm_year+1900,
			tmHMS->tm_mon+1,
			tmHMS->tm_mday,
			tmHMS->tm_hour,
			tmHMS->tm_min,
			tmHMS->tm_sec);

	if(ud.multimode>1) // if more than 1 player, we add name. Then add score if DM
	{
        text[0] = '\0';
		strcat((char *)text, " [");
		for(i=connecthead;i>=0;i=connectpoint2[i])
		{
			if(!ud.user_name[i][0])
				strcat(text, "NoName");
			else
				strcat(text, &ud.user_name[i][0]);

			if(ud.m_coop==0 || ud.m_coop==2)  // if DM or DM No spawn. Add Score as well
			{
				strcat(text, "(");
                snprintf(score, sizeof(score), "%d",ps[i].frag-ps[i].fraggedself);
				strcat(text, score);
				strcat(text, ") vs ");
			}
			else
				strcat(text, " vs ");
		}
		tempbuf[strlen(text)-4]=0; // remove last vs
		strcat(text, "]");
	}
	strcat(text, ".bmp");


	// If this is a TC save it to the TC's directory
	if(getGameDir()[0] != '\0')
	{
		sprintf(szFilename, "%s\\%s", getGameDir(), SCREENSHOTPATH);
		mkdir(szFilename);
		sprintf(szFilename, "%s\\%s\\%s", getGameDir(), SCREENSHOTPATH, tempbuf);
	}
	// otherwise let's save it to the root.
	else
	{
		mkdir(SCREENSHOTPATH);
		sprintf(szFilename, "%s\\%s", SCREENSHOTPATH, tempbuf);
	}

	if(SafeFileExists(szFilename) == 0)
	{
		screencapture(szFilename,0);
		sprintf(fta_quotes[103],"SCREEN SAVED");
		sound(EXITMENUSOUND);
	}
	else
		sprintf(fta_quotes[103],"CAN'T WRITE FILE!");

	FTA(103,&ps[screenpeek],1);

}


// Rare Multiplayer, when dead, total screen screwup back again!
// E3l1 (Coop /w monsters) sprite list corrupt 50%
// Univbe exit, instead, default to screen buffer.
// Check all caches bounds and memory usages
// Fix enlarger weapon selections to perfection
// Need sounds.c
// Spawning a couple of sounds at the same time
// Check Weapon Switching
// FIRE and FIRE2
// Where should I flash the screen white???
// Jittery on subs in mp?
// Check accurate memory amounts!
// Why squish sound at hit space when dead?
// Falling Counter Not reset in mp
// Wierd small freezer
// Double freeze on player?, still firing
// Do Mouse Flip option
// Save mouse aiming
// Laser bounce off mirrors
// GEORGE:   Ten in text screen.
// Alien:
// Freeze: change
// Press space holding player
// Press space
// tank broke
// 2d mode fucked in fake mp mode
// 207
// Mail not rolling up on conveyers
// Fix all alien animations
// Do episode names in .CONS
// do syntak check for "{?????"
// Make commline parms set approiate multiplayer flags

// Check all breakables to see if they are exploding properly
// Fix freezing palette on Alien

// Do a demo make run overnite
// Fix Super Duck
// Slime Guies, use quickkick.

// Make Lasers from trip bombs reflect off mirrors
// Remember for lockout of sound swears
// Pass sender in packed, NOT
// Fatal sync give no message for TEN
// Hitting TEN BUTTON(OPTION) no TEN SCreen
// Check multioperateswitches for se 31,32
// Fix pal for ceilings (SE#18)
// case 31: sprites up one high
// E1l1 No Kill All troops in room, sleep time

// Fifo for message list

// Bloodsplat on conveyers

// Meclanical
// Increase sound
// Mouse Delay at death
// Wierd slowdown

// Footprints on stuff floating

// Ken, The inside function is called a lot in -1 sectors
// No loading Univbe message rewrite
// Expander must cycle with rest of weapons
// Duck SHOOT PIPEBOMB, red wall

// Get commit source from mark

/*
1. fix pipebomb bug
2. check george maps
4. Save/Restore check (MP and SP)
5. Check TEN
6. Get Commit fixed
8. Is mail slow?
9. Cacheing
10. Blue out "PLAY ON TEN" in MULTIPLAYER
11. Eight Player test
12. Postal.voc not found.
13. All Monsters explode in arcade,
    check SEENINE STRENGTH,
    Change 28<<8 back to 16<<8 in hitradius
    Compare 1.3d to 1.4
14. Check sounds/gfx for for parr lock
15. Player # Loaded a game
16. Replace Crane code 1.3d to 1.4
17. Fix Greenslime
18. Small Freeze sprite,below floor
19. Vesa message auto abort in mp?
20. Fucked Palette in my skip ahead in MP
21. Load in main menu
22. Rotated frag screen no game screen
23. Jibs sounds when killed other dukes
24. Ten code and /f4 mode
25. Fix All MP Glitches!!
26. Unrem Menues anim tenbn
27. buy groc,clothes,scanner
28. Why Double Defs in global and game, is so at work
29. Check that all .objs are erased
30. Check why 1.3ds gotweapon gamedef coop code no workie
31. Heavy mods to net code
32. Make sure all commline stuff works,
33. killed all waitfor???
34. 90k stack
35. double door probs
36: copy protection
* when you start a game the duke saying that is played when you choose a skill the sound is cut off.
* NEWBEASTJUMPING is not deleted at premap in multi-play
if(*c == '4') no work need objs ask ken, commit
{
movesperpacket = 4;
setpackettimeout(0x3fffffff,0x3fffffff);
}
remember, netcode load
*/
//  Ai Problem in god mode.
// Checkplayerhurtwall for forcefields bigforce
// Nuddie, posters. IMF
// Release commit.c to public?
// Document Save bug with mp
// Check moves per packet /f4 waitforeverybody over net?
// Kill IDF OBJ
// No shotguns under water @ tanker
// Unrem copyprotect
// Look for printf and puts
// Check con rewrites
// erase mmulti.c, or get newest objs
// Why nomonsters screwy in load menu in mp
// load last > 'y' == NOT
// Check xptr oos when dead rising to surface.
//    diaginal warping with shotguns
// Test white room.  Lasertripbomb arming crash
// The Bog
// Run Duke Out of windows
// Put Version number in con files
// Test diff. version playing together
// Reorganize dukecd
// Put out patch w/ two weeks testing
// Print draw3d
// Double Klick

/*
Duke Nukem V

Layout:

      Settings:
        Suburbs
          Duke inflitrating neighborhoods inf. by aliens
        Death Valley:
          Sorta like a western.  Bull-skulls half buried in the sand
          Military compound:  Aliens take over nuke-missle silo, duke
            must destroy.
          Abondend Aircraft field
        Vegas:
          Blast anything bright!  Alien lights camoflauged.
          Alien Drug factory. The Blue Liquid
        Mountainal Cave:
          Interior cave battles.
        Jungle:
          Trees, canopee, animals, a mysterious hole in the earth with
          gas seaping thru.
        Penetencury:
          Good use of spotlights:
        Mental ward:
          People whom have claimed to be slowly changing into an
          alien species

      Inventory:
        Wood,
        Metal,
        Torch,
        Rope,
        Plastique,
        Cloth,
        Wiring,
        Glue,
        Cigars,
        Food,
        Duck Tape,
        Nails,
        Piping,
        Petrol,
        Uranium,
        Gold,
        Prism,
        Power Cell,

        Hand spikes (Limited usage, they become dull)
        Oxygent     (Oxygen mixed with stimulant)


      Player Skills:
        R-Left,R-Right,Foward,Back
        Strafe, Jump, Double Flip Jump for distance
        Help, Escape
        Fire/Use
        Use Menu

    After a brief resbit, Duke decides to get back to work.

Cmdr:   "Duke, we've got a lot of scared people down there.
         Some reports even claim that people are already
         slowly changing into aliens."
Duke:   "No problem, my speciality is in croud control."
Cmdr:   "Croud control, my ass!  Remember that incident
         during the war?  You created nuthin' but death and
         destruction."
Duke:   "Not destruction, justice."
Cmdr:   "I'll take no responsibility for your actions.  Your on
         your own!  Behave your self, damnit!  You got that,
         soldger?"
Duke:   "I've always been on my own...   Face it, it's ass kickin' time,
         SIR!"
Cmdr:   "Get outta here...!"
        (Duke gives the Cmdr a hard stair, then cocks his weapon and
         walks out of the room)
Cmdr:   In a wisper: "Good luck, my friend."

        (Cut to a scene where aliens are injecting genetic material
         into an unconcious subject)

Programming:   ( the functions I need )
     Images: Polys
     Actors:
       Multi-Object sections for change (head,arms,legs,torsoe,all change)
       Facial expressions.  Pal lookup per poly?

     struct imagetype
        {
            int *itable; // AngX,AngY,AngZ,Xoff,Yoff,Zoff;
            int *idata;
            struct imagetype *prev, *next;
        }

*/


// Test frag screen name fuckup
// Test all xptrs
// Make Jibs stick to ceiling
// Save Game menu crash
// Cache len sum err
// Loading in main (MP), reset totalclock?
// White Room
// Sound hitch with repeat bits
// Rewrite saved menues so no crash
// Put a getpackets after loadplayer in menus
// Put "loading..." before waitfor in loadpla
// No ready2send = 0 for loading
// Test Joystick
// Ten
// Bog
// Test Blimp respawn
// move 1 in player???


