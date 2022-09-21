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
#include "filesystem.h"
#include "game.h"


extern uint8_t  everyothertime;
short which_palookup = 9;


static void tloadtile(short tilenume)
{
    gotpic[tilenume>>3] |= (1<<(tilenume&7));
}

void cachespritenum(short i)
{
    uint8_t  maxc;
    short j;

    if(ud.monsters_off && badguy(&sprite[i])) return;

    maxc = 1;

    switch(PN)
    {
        case HYDRENT:
            tloadtile(BROKEFIREHYDRENT);
            for(j = TOILETWATER; j < (TOILETWATER+4); j++)
                if(tiles[j].data == NULL) tloadtile(j);
            break;
        case TOILET:
            tloadtile(TOILETBROKE);
            for(j = TOILETWATER; j < (TOILETWATER+4); j++)
                if(tiles[j].data == NULL) tloadtile(j);
            break;
        case STALL:
            tloadtile(STALLBROKE);
            for(j = TOILETWATER; j < (TOILETWATER+4); j++)
                if(tiles[j].data == NULL) tloadtile(j);
            break;
        case RUBBERCAN:
            maxc = 2;
            break;
        case TOILETWATER:
            maxc = 4;
            break;
        case FEMPIC1:
            maxc = 44;
            break;
        case LIZTROOP:
        case LIZTROOPRUNNING:
        case LIZTROOPSHOOT:
        case LIZTROOPJETPACK:
        case LIZTROOPONTOILET:
        case LIZTROOPDUCKING:
            for(j = LIZTROOP; j < (LIZTROOP+72); j++)
                if(tiles[j].data == NULL)
                    tloadtile(j);
            for(j=HEADJIB1;j<LEGJIB1+3;j++)
                if(tiles[j].data == NULL)
                    tloadtile(j);
            maxc = 0;
            break;
        case WOODENHORSE:
            maxc = 5;
            for(j = HORSEONSIDE; j < (HORSEONSIDE+4); j++)
                if(tiles[j].data == NULL)
                    tloadtile(j);
            break;
        case NEWBEAST:
        case NEWBEASTSTAYPUT:
            maxc = 90;
            break;
        case BOSS1:
        case BOSS2:
        case BOSS3:
            maxc = 30;
            break;
        case OCTABRAIN:
        case OCTABRAINSTAYPUT:
        case COMMANDER:
        case COMMANDERSTAYPUT:
            maxc = 38;
            break;
        case RECON:
            maxc = 13;
            break;
        case PIGCOP:
        case PIGCOPDIVE:
            maxc = 61;
            break;
        case SHARK:
            maxc = 30;
            break;
        case LIZMAN:
        case LIZMANSPITTING:
        case LIZMANFEEDING:
        case LIZMANJUMP:
            for(j=LIZMANHEAD1;j<LIZMANLEG1+3;j++)
                if(tiles[j].data == NULL)
                    tloadtile(j);
            maxc = 80;
            break;
        case APLAYER:
            maxc = 0;
            if(ud.multimode > 1)
            {
                maxc = 5;
                for(j = 1420;j < 1420+106; j++)
                    if(tiles[j].data == NULL)
                        tloadtile(j);
            }
            break;
        case ATOMICHEALTH:
            maxc = 14;
            break;
        case DRONE:
            maxc = 10;
            break;
        case EXPLODINGBARREL:
        case SEENINE:
        case OOZFILTER:
            maxc = 3;
            break;
        case NUKEBARREL:
        case CAMERA1:
            maxc = 5;
            break;
    }

    for(j = PN; j < (PN+maxc); j++)
        if(tiles[j].data == NULL)
            tloadtile(j);
}

void cachegoodsprites(void)
{
    short i;

    if(ud.screen_size >= 8)
    {
        if(tiles[BOTTOMSTATUSBAR].data == NULL)
            tloadtile(BOTTOMSTATUSBAR);
        if( ud.multimode > 1)
        {
            if(tiles[FRAGBAR].data == NULL)
                tloadtile(FRAGBAR);
            for(i=MINIFONT;i<MINIFONT+63;i++)
                if(tiles[i].data == NULL)
                    tloadtile(i);
        }
    }

    tloadtile(VIEWSCREEN);

    for(i=STARTALPHANUM;i<ENDALPHANUM+1;i++)
        if (tiles[i].data == NULL)
            tloadtile(i);

    for(i=FOOTPRINTS;i<FOOTPRINTS+3;i++)
        if (tiles[i].data == NULL)
            tloadtile(i);

    for( i = BIGALPHANUM; i < BIGALPHANUM+82; i++)
        if(tiles[i].data == NULL)
            tloadtile(i);

    for( i = BURNING; i < BURNING+14; i++)
        if(tiles[i].data == NULL)
            tloadtile(i);

    for( i = BURNING2; i < BURNING2+14; i++)
        if(tiles[i].data == NULL)
            tloadtile(i);

    for( i = CRACKKNUCKLES; i < CRACKKNUCKLES+4; i++)
        if(tiles[i].data == NULL)
            tloadtile(i);

    for( i = FIRSTGUN; i < FIRSTGUN+3 ; i++ )
        if(tiles[i].data == NULL)
            tloadtile(i);

    for( i = EXPLOSION2; i < EXPLOSION2+21 ; i++ )
        if(tiles[i].data == NULL)
            tloadtile(i);

    tloadtile(BULLETHOLE);

    for( i = FIRSTGUNRELOAD; i < FIRSTGUNRELOAD+8 ; i++ )
        if(tiles[i].data == NULL)
            tloadtile(i);

    tloadtile(FOOTPRINTS);

    for( i = JIBS1; i < (JIBS5+5); i++)
        if(tiles[i].data == NULL)
            tloadtile(i);

    for( i = SCRAP1; i < (SCRAP1+19); i++)
        if(tiles[i].data == NULL)
            tloadtile(i);

    for( i = SMALLSMOKE; i < (SMALLSMOKE+4); i++)
        if(tiles[i].data == NULL)
            tloadtile(i);
}

uint8_t  getsound(uint16_t num)
{
    short fp;
    int32_t   l;

    if(num >= NUM_SOUNDS || SoundToggle == 0) return 0;
    if (FXDevice == NumSoundCards) return 0;

    fp = TCkopen4load(sounds[num],0);
    if(fp == -1) return 0;

    l = kfilelength( fp );
    soundsiz[num] = l;

    if( (ud.level_number == 0 && ud.volume_number == 0 && (num == 189 || num == 232 || num == 99 || num == 233 || num == 17 ) ) ||
        ( l < 12288 ) )
    {
        Sound[num].lock = 2;
        allocache(&Sound[num].ptr,l,&Sound[num].lock);
        if(Sound[num].ptr != NULL)
            kread( fp, Sound[num].ptr , l);
    }
    kclose( fp );
    return 1;
}

void precachenecessarysounds(void)
{
    short i, j;
    printf("precachenecessarysounds: %d\n", SoundToggle);

    if (FXDevice == NumSoundCards) return;
    j = 0;

    for(i=0;i<NUM_SOUNDS;i++)
        if(Sound[i].ptr == 0)
        {
            j++;
            if( (j&7) == 0 )
                getpackets();
            getsound(i);
        }
}


void cacheit(void)
{
    short i,j;

    precachenecessarysounds();

    cachegoodsprites();

    for(i=0;i<numwalls;i++)
        if( tiles[wall[i].picnum].data == NULL)
    {
        if(tiles[wall[i].picnum].data == NULL)
            tloadtile(wall[i].picnum);
        if(wall[i].overpicnum >= 0 && tiles[wall[i].overpicnum].data == NULL )
            tloadtile(wall[i].overpicnum);
    }

    for(i=0;i<numsectors;i++)
    {
        if( tiles[sector[i].floorpicnum].data == NULL )
            tloadtile( sector[i].floorpicnum );
        if( tiles[sector[i].ceilingpicnum].data == NULL )
        {
            tloadtile( sector[i].ceilingpicnum );
            if( tiles[sector[i].ceilingpicnum].data == (uint8_t*)LA)
            {
                tloadtile(LA+1);
                tloadtile(LA+2);
            }
        }

        j = headspritesect[i];
        while(j >= 0)
        {
            if(sprite[j].xrepeat != 0 && sprite[j].yrepeat != 0 && (sprite[j].cstat&32768) == 0)
                if(tiles[sprite[j].picnum].data == NULL)
                    cachespritenum(j);
            j = nextspritesect[j];
        }
    }

}

void docacheit(void)
{
    int32_t i,j;

    j = 0;

    for(i=0;i<MAXTILES;i++)
        if( (gotpic[i>>3]&(1<<(i&7))) && tiles[i].data == NULL)
    {
        loadtile((short)i);
        j++;
        if((j&7) == 0) getpackets();
    }

    clearbufbyte(gotpic,sizeof(gotpic),0L);

}



void xyzmirror(short i,short tileId)
{
    if (tiles[tileId].data == NULL)
        loadtile(tileId);
    
	setviewtotile(tileId,tiles[tileId].dim.height,tiles[tileId].dim.width);

	drawrooms(SX,SY,SZ,SA,100+sprite[i].shade,SECT);
	display_mirror = 1; animatesprites(SX,SY,SA,65536L); display_mirror = 0;
	drawmasks();

	setviewback();
	squarerotatetile(tileId);
}

void vscrn(void)
{
     int32_t ss, x1, x2, y1, y2;

	 if(ud.screen_size < 0) ud.screen_size = 0;
	 else if(ud.screen_size > 63) ud.screen_size = 64;

     if(ud.screen_size == 0) 
		 flushperms();

	 ss = max(ud.screen_size-8,0);

	 x1 = scale(ss,xdim,160);
	 x2 = xdim-x1;

	 y1 = ss; y2 = 200;
	 y1 += countfragbars();

	 if (ud.screen_size >= 8) 
		 y2 -= (ss+34);

	 y1 = scale(y1,ydim,200);
	 y2 = scale(y2,ydim,200);

	 setview(x1,y1,x2-1,y2-1);

     pub = NUMPAGES;
     pus = NUMPAGES;
}

int countfragbars(void)
{
	int32_t i, j, y = 0;

	if ( ud.screen_size > 0 && ud.coop != 1 && ud.multimode > 1)
	{
		j = 0;
		for(i=connecthead;i>=0;i=connectpoint2[i])
			if(i > j) j = i;

		if (j >= 1) y += 8;
		if (j >= 4) y += 8;
		if (j >= 8) y += 8;
		if (j >= 12) y += 8;
	}

	return(y);
}


void pickrandomspot(short snum)
{
    struct player_struct *p;
    short i;

    p = &ps[snum];

    if( ud.multimode > 1 && ud.coop != 1)
        i = TRAND%numplayersprites;
    else i = snum;

    p->bobposx = p->oposx = p->posx = po[i].ox;
    p->bobposy = p->oposy = p->posy = po[i].oy;
    p->oposz = p->posz = po[i].oz;
    p->ang = po[i].oa;
    p->cursectnum = po[i].os;
}

void resetplayerstats(short snum)
{
    struct player_struct *p;

    p = &ps[snum];

    ud.show_help        = 0;
    ud.showallmap       = 0;
    p->dead_flag        = 0;
    p->wackedbyactor    = -1;
    p->falling_counter  = 0;
    p->quick_kick       = 0;
    p->subweapon        = 0;
    p->last_full_weapon = 0;
    p->ftq              = 0;
    p->fta              = 0;
    p->tipincs          = 0;
    p->buttonpalette    = 0;
    p->actorsqu         =-1;
    p->invdisptime      = 0;
    p->refresh_inventory= 0;
    p->last_pissed_time = 0;
    p->holster_weapon   = 0;
    p->pycount          = 0;
    p->pyoff            = 0;
    p->opyoff           = 0;
    p->loogcnt          = 0;
    p->angvel           = 0;
    p->weapon_sway      = 0;
//    p->select_dir       = 0;
    p->extra_extra8     = 0;
    p->show_empty_weapon= 0;
    p->dummyplayersprite=-1;
    p->crack_time       = 0;
    p->hbomb_hold_delay = 0;
    p->transporter_hold = 0;
    p->wantweaponfire  = -1;
    p->hurt_delay       = 0;
    p->footprintcount   = 0;
    p->footprintpal     = 0;
    p->footprintshade   = 0;
    p->jumping_toggle   = 0;
    p->ohoriz = p->horiz= 140;
    p->horizoff         = 0;
    p->bobcounter       = 0;
    p->on_ground        = 0;
    p->player_par       = 0;
    p->return_to_center = 9;
    p->airleft          = 15*26;
    p->rapid_fire_hold  = 0;
    p->toggle_key_flag  = 0;
    p->access_spritenum = -1;
    if(ud.multimode > 1 && ud.coop != 1 )
        p->got_access = 7;
    else p->got_access      = 0;
    p->random_club_frame= 0;
    pus = 1;
    p->on_warping_sector = 0;
    p->spritebridge      = 0;
    p->palette = (uint8_t  *) &palette[0];

    if(p->steroids_amount < 400 )
    {
        p->steroids_amount = 0;
        p->inven_icon = 0;
    }
    p->heat_on =            0;
    p->jetpack_on =         0;
    p->holoduke_on =       -1;

    p->look_ang          = 512 - ((ud.level_number&1)<<10);

    p->rotscrnang        = 0;
    p->newowner          =-1;
    p->jumping_counter   = 0;
    p->hard_landing      = 0;
    p->posxv             = 0;
    p->posyv             = 0;
    p->poszv             = 0;
    fricxv            = 0;
    fricyv            = 0;
    p->somethingonplayer =-1;
    p->one_eighty_count  = 0;
    p->cheat_phase       = 0;

    p->on_crane          = -1;

    if(p->curr_weapon == PISTOL_WEAPON)
        p->kickback_pic  = 5;
    else p->kickback_pic = 0;

    p->weapon_pos        = 6;
    p->walking_snd_toggle= 0;
    p->weapon_ang        = 0;

    p->knuckle_incs      = 1;
    p->fist_incs = 0;
    p->knee_incs         = 0;
    p->jetpack_on        = 0;
    setpal(p);
	p->weaponautoswitch	 = 0;
	p->auto_aim          = 2;
	p->fakeplayer        = 0;
}



void resetweapons(short snum)
{
    short  weapon;
    struct player_struct *p;

    p = &ps[snum];

    for ( weapon = PISTOL_WEAPON; weapon < MAX_WEAPONS; weapon++ )
        p->gotweapon[weapon] = 0;
    for ( weapon = PISTOL_WEAPON; weapon < MAX_WEAPONS; weapon++ )
        p->ammo_amount[weapon] = 0;

    p->weapon_pos = 6;
    p->kickback_pic = 5;
    p->curr_weapon = PISTOL_WEAPON;
    p->gotweapon[PISTOL_WEAPON] = 1;
    p->gotweapon[KNEE_WEAPON] = 1;
    p->ammo_amount[PISTOL_WEAPON] = 48;
    p->gotweapon[HANDREMOTE_WEAPON] = 1;
    p->last_weapon = -1;

    p->show_empty_weapon= 0;
    p->last_pissed_time = 0;
    p->holster_weapon = 0;
}

void resetinventory(short snum)
{
    struct player_struct *p;

    p = &ps[snum];

    p->inven_icon       = 0;
    p->boot_amount = 0;
    p->scuba_on =           0;p->scuba_amount =         0;
    p->heat_amount        = 0;p->heat_on = 0;
    p->jetpack_on =         0;p->jetpack_amount =       0;
    p->shield_amount =      max_armour_amount;
    p->holoduke_on = -1;
    p->holoduke_amount =    0;
    p->firstaid_amount = 0;
    p->steroids_amount = 0;
    p->inven_icon = 0;
}


void resetprestat(short snum,uint8_t  g)
{
    struct player_struct *p;
    short i;

    p = &ps[snum];

    spriteqloc = 0;
    for(i=0;i<spriteqamount;i++) spriteq[i] = -1;

    p->hbomb_on          = 0;
    p->cheat_phase       = 0;
    p->pals_time         = 0;
    p->toggle_key_flag   = 0;
    p->secret_rooms      = 0;
    p->max_secret_rooms  = 0;
    p->actors_killed     = 0;
    p->max_actors_killed = 0;
    p->lastrandomspot = 0;
    p->weapon_pos = 6;
    p->kickback_pic = 5;
    p->last_weapon = -1;
    p->weapreccnt = 0;
    p->show_empty_weapon= 0;
    p->holster_weapon = 0;
    p->last_pissed_time = 0;

    p->one_parallax_sectnum = -1;
    p->visibility = ud.const_visibility;

    screenpeek              = myconnectindex;
    numanimwalls            = 0;
    numcyclers              = 0;
    animatecnt              = 0;
    parallaxtype            = 0;
    randomseed              = 17L;
    ud.pause_on             = 0;
    ud.camerasprite         =-1;
    ud.eog                  = 0;
    tempwallptr             = 0;
    camsprite               =-1;
    earthquaketime          = 0;

    numinterpolations = 0;
    startofdynamicinterpolations = 0;

    if( ( (g&MODE_EOL) != MODE_EOL && numplayers < 2) || (ud.coop != 1 && numplayers > 1) )
    {
        resetweapons(snum);
        resetinventory(snum);
    }
    else if(p->curr_weapon == HANDREMOTE_WEAPON)
    {
        p->ammo_amount[HANDBOMB_WEAPON]++;
        p->curr_weapon = HANDBOMB_WEAPON;
    }

    p->timebeforeexit   = 0;
    p->customexitsound  = 0;

}

void setupbackdrop(short sky)
{
    short i;

    for(i=0;i<MAXPSKYTILES;i++) pskyoff[i]=0;

    if(parallaxyscale != 65536L)
        parallaxyscale = 32768;

    switch(sky)
    {
        case CLOUDYOCEAN:
            parallaxyscale = 65536L;
            break;
        case MOONSKY1 :
            pskyoff[6]=1; pskyoff[1]=2; pskyoff[4]=2; pskyoff[2]=3;
            break;
        case BIGORBIT1: // orbit
            pskyoff[5]=1; pskyoff[6]=2; pskyoff[7]=3; pskyoff[2]=4;
            break;
        case LA:
            parallaxyscale = 16384+1024;
            pskyoff[0]=1; pskyoff[1]=2; pskyoff[2]=1; pskyoff[3]=3;
            pskyoff[4]=4; pskyoff[5]=0; pskyoff[6]=2; pskyoff[7]=3;
            break;
   }

   pskybits=3;
}

void prelevel(uint8_t  g)
{
    short i, nexti, j, startwall, endwall, lotaglist;
    short lotags[65];


    clearbufbyte(show2dsector,sizeof(show2dsector),0L);
    clearbufbyte(show2dwall,sizeof(show2dwall),0L);
    clearbufbyte(show2dsprite,sizeof(show2dsprite),0L);

    resetprestat(0,g);
    numclouds = 0;

    for(i=0;i<numsectors;i++)
    {
        sector[i].extra = 256;

        switch(sector[i].lotag)
        {
            case 20:
            case 22:
                if( sector[i].floorz > sector[i].ceilingz)
                    sector[i].lotag |= 32768;
                continue;
        }

        if(sector[i].ceilingstat&1)
        {
            if(tiles[sector[i].ceilingpicnum].data == NULL)
            {
                if(sector[i].ceilingpicnum == LA)
                    for(j=0;j<5;j++)
                        if(tiles[sector[i].ceilingpicnum+j].data == NULL)
                            tloadtile(sector[i].ceilingpicnum+j);
            }
            setupbackdrop(sector[i].ceilingpicnum);

            if(sector[i].ceilingpicnum == CLOUDYSKIES && numclouds < 127)
                clouds[numclouds++] = i;

            if(ps[0].one_parallax_sectnum == -1)
                ps[0].one_parallax_sectnum = i;
        }

        if(sector[i].lotag == 32767) //Found a secret room
        {
            ps[0].max_secret_rooms++;
            continue;
        }

        if(sector[i].lotag == -1)
        {
            ps[0].exitx = wall[sector[i].wallptr].x;
            ps[0].exity = wall[sector[i].wallptr].y;
            continue;
        }
    }

    i = headspritestat[0];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        if(sprite[i].lotag == -1 && (sprite[i].cstat&16) )
        {
            ps[0].exitx = SX;
            ps[0].exity = SY;
        }
        else switch(PN)
        {
            case GPSPEED:
                sector[SECT].extra = SLT;
                deletesprite(i);
                break;

            case CYCLER:
                if(numcyclers >= MAXCYCLERS)
                    gameexit("\nToo many cycling sectors.");
                cyclers[numcyclers][0] = SECT;
                cyclers[numcyclers][1] = SLT;
                cyclers[numcyclers][2] = SS;
                cyclers[numcyclers][3] = sector[SECT].floorshade;
                cyclers[numcyclers][4] = SHT;
                cyclers[numcyclers][5] = (SA == 1536);
                numcyclers++;
                deletesprite(i);
                break;
        }
        i = nexti;
    }

    for(i=0;i < MAXSPRITES;i++)
    {
        if(sprite[i].statnum < MAXSTATUS)
        {
            if(PN == SECTOREFFECTOR && SLT == 14)
                continue;
            spawn(-1,i);
        }
    }

    for(i=0;i < MAXSPRITES;i++)
        if(sprite[i].statnum < MAXSTATUS)
        {
            if( PN == SECTOREFFECTOR && SLT == 14 )
                spawn(-1,i);
        }

    lotaglist = 0;

    i = headspritestat[0];
    while(i >= 0)
    {
        switch(PN)
        {
            case DIPSWITCH:
            case DIPSWITCH2:
            case ACCESSSWITCH:
            case PULLSWITCH:
            case HANDSWITCH:
            case SLOTDOOR:
            case LIGHTSWITCH:
            case SPACELIGHTSWITCH:
            case SPACEDOORSWITCH:
            case FRANKENSTINESWITCH:
            case LIGHTSWITCH2:
            case POWERSWITCH1:
            case LOCKSWITCH1:
            case POWERSWITCH2:
                break;
            case DIPSWITCH+1:
            case DIPSWITCH2+1:
            case PULLSWITCH+1:
            case HANDSWITCH+1:
            case SLOTDOOR+1:
            case LIGHTSWITCH+1:
            case SPACELIGHTSWITCH+1:
            case SPACEDOORSWITCH+1:
            case FRANKENSTINESWITCH+1:
            case LIGHTSWITCH2+1:
            case POWERSWITCH1+1:
            case LOCKSWITCH1+1:
            case POWERSWITCH2+1:
                for(j=0;j<lotaglist;j++)
                    if( SLT == lotags[j] )
                        break;

                if( j == lotaglist )
                {
                    lotags[lotaglist] = SLT;
                    lotaglist++;
                    if(lotaglist > 64)
                        gameexit("\nToo many switches (64 max).");

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if(sprite[j].lotag == 12 && sprite[j].hitag == SLT)
                            hittype[j].temp_data[0] = 1;
                        j = nextspritestat[j];
                    }
                }
                break;
        }
        i = nextspritestat[i];
    }

    mirrorcnt = 0;

    for( i = 0; i < numwalls; i++ )
    {
        walltype *wal;
        wal = &wall[i];

        if(wal->overpicnum == MIRROR && (wal->cstat&32) != 0)
        {
            j = wal->nextsector;

            if(mirrorcnt > 63)
                gameexit("\nToo many mirrors (64 max.)");
            if ( (j >= 0) && sector[j].ceilingpicnum != MIRROR )
            {
                sector[j].ceilingpicnum = MIRROR;
                sector[j].floorpicnum = MIRROR;
                mirrorwall[mirrorcnt] = i;
                mirrorsector[mirrorcnt] = j;
                mirrorcnt++;
                continue;
            }
        }

        if(numanimwalls >= MAXANIMWALLS)
            gameexit("\nToo many 'anim' walls (max 512.)");

        animwall[numanimwalls].tag = 0;
        animwall[numanimwalls].wallnum = 0;

        switch(wal->overpicnum)
        {
            case FANSHADOW:
            case FANSPRITE:
                wall->cstat |= 65;
                animwall[numanimwalls].wallnum = i;
                numanimwalls++;
                break;

            case W_FORCEFIELD:
                if(tiles[W_FORCEFIELD].data == NULL)
                    for(j=0;j<3;j++)
                        tloadtile(W_FORCEFIELD+j);
            case W_FORCEFIELD+1:
            case W_FORCEFIELD+2:
                if(wal->shade > 31)
                    wal->cstat = 0;
                else wal->cstat |= 85+256;


                if(wal->lotag && wal->nextwall >= 0)
                    wall[wal->nextwall].lotag =
                        wal->lotag;

            case BIGFORCE:

                animwall[numanimwalls].wallnum = i;
                numanimwalls++;

                continue;
        }

        wal->extra = -1;

        switch(wal->picnum)
        {
            case WATERTILE2:
                for(j=0;j<3;j++)
                    if(tiles[wal->picnum+j].data == NULL)
                        tloadtile(wal->picnum+j);
                break;

            case TECHLIGHT2:
            case TECHLIGHT4:
                if(tiles[wal->picnum].data == NULL)
                    tloadtile(wal->picnum);
                break;
            case W_TECHWALL1:
            case W_TECHWALL2:
            case W_TECHWALL3:
            case W_TECHWALL4:
                animwall[numanimwalls].wallnum = i;
//                animwall[numanimwalls].tag = -1;
                numanimwalls++;
                break;
            case SCREENBREAK6:
            case SCREENBREAK7:
            case SCREENBREAK8:
                if(tiles[SCREENBREAK6].data == NULL)
                    for(j=SCREENBREAK6;j<SCREENBREAK9;j++)
                        tloadtile(j);
                animwall[numanimwalls].wallnum = i;
                animwall[numanimwalls].tag = -1;
                numanimwalls++;
                break;

            case FEMPIC1:
            case FEMPIC2:
            case FEMPIC3:

                wal->extra = wal->picnum;
                animwall[numanimwalls].tag = -1;
                if(ud.lockout)
                {
                    if(wal->picnum == FEMPIC1)
                        wal->picnum = BLANKSCREEN;
                    else wal->picnum = SCREENBREAK6;
                }

                animwall[numanimwalls].wallnum = i;
                animwall[numanimwalls].tag = wal->picnum;
                numanimwalls++;
                break;

            case SCREENBREAK1:
            case SCREENBREAK2:
            case SCREENBREAK3:
            case SCREENBREAK4:
            case SCREENBREAK5:

            case SCREENBREAK9:
            case SCREENBREAK10:
            case SCREENBREAK11:
            case SCREENBREAK12:
            case SCREENBREAK13:
            case SCREENBREAK14:
            case SCREENBREAK15:
            case SCREENBREAK16:
            case SCREENBREAK17:
            case SCREENBREAK18:
            case SCREENBREAK19:

                animwall[numanimwalls].wallnum = i;
                animwall[numanimwalls].tag = wal->picnum;
                numanimwalls++;
                break;
        }
    }

    //Invalidate textures in sector behind mirror
    for(i=0;i<mirrorcnt;i++)
    {
        startwall = sector[mirrorsector[i]].wallptr;
        endwall = startwall + sector[mirrorsector[i]].wallnum;
        for(j=startwall;j<endwall;j++)
        {
            wall[j].picnum = MIRROR;
            wall[j].overpicnum = MIRROR;
        }
    }
}


void newgame(uint8_t  vn,uint8_t  ln,uint8_t  sk)
{
    struct player_struct *p = &ps[0];
    short i;

    if(globalskillsound >= 0)
        while(Sound[globalskillsound].lock>=200);
    globalskillsound = -1;

    waitforeverybody();
    ready2send = 0;

    if( ud.m_recstat != 2 && ud.last_level >= 0 && ud.multimode > 1 && ud.coop != 1)
        dobonus(1);

	if (ud.showcinematics)
		if( ln == 0 && vn == 3 && ud.multimode < 2 && ud.lockout == 0)
		{
			playmusic(&env_music_fn[1][0]);

			flushperms();
			setview(0,0,xdim-1,ydim-1);
			clearview(0L);
			nextpage();

			playanm("vol41a.anm",6);
			clearview(0L);
			nextpage();
			playanm("vol42a.anm",7);
	//        clearview(0L);
	  //      nextpage();
			playanm("vol43a.anm",9);
			clearview(0L);
			nextpage();

			FX_StopAllSounds();
		}

    show_shareware = 26*34;

    ud.level_number =   ln;
    ud.volume_number =  vn;
    ud.player_skill =   sk;
    ud.secretlevel =    0;
    ud.from_bonus = 0;
    parallaxyscale = 0;

    ud.last_level = -1;
    lastsavedpos = -1;
    p->zoom            = 768;
    p->gm              = 0;

    if(ud.m_coop != 1)
    {
        p->curr_weapon = PISTOL_WEAPON;
        p->gotweapon[PISTOL_WEAPON] = 1;
        p->gotweapon[KNEE_WEAPON] = 1;
        p->ammo_amount[PISTOL_WEAPON] = 48;
        p->gotweapon[HANDREMOTE_WEAPON] = 1;
        p->last_weapon = -1;
    }

    display_mirror =        0;

    if(ud.multimode > 1 )
    {
        if(numplayers < 2)
        {
            connecthead = 0;
            for(i=0;i<MAXPLAYERS;i++) connectpoint2[i] = i+1;
            connectpoint2[ud.multimode-1] = -1;
        }
    }
    else
    {
        connecthead = 0;
        connectpoint2[0] = -1;
    }
}


void resetpspritevars(uint8_t  g)
{
    short i, j, nexti; //circ;
//    int32_t firstx,firsty;
    spritetype *s;
    uint8_t  aimmode[MAXPLAYERS];
    STATUSBARTYPE tsbar[MAXPLAYERS];

	#define BOT_MAX_NAME 20
	int bot_used[BOT_MAX_NAME] = { false };
	char  *bot_names[] = {	"* ELASTI",
							"* ^ZookeM^",
							"* DOOM",
							"* DRO",
							"* NOX",
							"* EXP",
							"* SKIPPY",
							"* BRYZIAN",
							"* ALI-GUN",
							"* ADDFAZ",
							"* TURRICAN",
							"* PODA",
							"* EWOLF",
							"* SOULIANE",
							"* SPANATOR",
							"* OVERLORD",
							"* COWBOYUK",
							"* JAKS",
							"* BLUEDRAG",
							"* MOE{GER}"};


    EGS(ps[0].cursectnum,ps[0].posx,ps[0].posy,ps[0].posz,
        APLAYER,0,0,0,ps[0].ang,0,0,0,10);

    if(ud.recstat != 2) for(i=0;i<MAXPLAYERS;i++)
    {
        aimmode[i] = ps[i].aim_mode;
        if(ud.multimode > 1 && ud.coop == 1 && ud.last_level >= 0)
        {
            for(j=0;j<MAX_WEAPONS;j++)
            {
                tsbar[i].ammo_amount[j] = ps[i].ammo_amount[j];
                tsbar[i].gotweapon[j] = ps[i].gotweapon[j];
            }

            tsbar[i].shield_amount = ps[i].shield_amount;
            tsbar[i].curr_weapon = ps[i].curr_weapon;
            tsbar[i].inven_icon = ps[i].inven_icon;

            tsbar[i].firstaid_amount = ps[i].firstaid_amount;
            tsbar[i].steroids_amount = ps[i].steroids_amount;
            tsbar[i].holoduke_amount = ps[i].holoduke_amount;
            tsbar[i].jetpack_amount = ps[i].jetpack_amount;
            tsbar[i].heat_amount = ps[i].heat_amount;
            tsbar[i].scuba_amount = ps[i].scuba_amount;
            tsbar[i].boot_amount = ps[i].boot_amount;
        }
    }

    resetplayerstats(0); // reset a player 

    for(i=1;i<MAXPLAYERS;i++) // reset all the others
       memcpy(&ps[i],&ps[0],sizeof(ps[0]));

	// FIX_00080: Out Of Synch in demos. Tries recovering OOS in old demos v27/28/29/116/117/118. New: v30/v119.
	if(numplayers<2 && !(g&MODE_DEMO))
		memcpy(ud.wchoice[0],ud.mywchoice,sizeof(ud.mywchoice)); 

	if(g&MODE_DEMO &&	(ud.playing_demo_rev == BYTEVERSION_27 ||
						ud.playing_demo_rev == BYTEVERSION_28 || 
						ud.playing_demo_rev == BYTEVERSION_29 ||
						ud.playing_demo_rev == BYTEVERSION_116 ||
						ud.playing_demo_rev == BYTEVERSION_117 ||
						ud.playing_demo_rev == BYTEVERSION_118 ))
		for(i=0; i<ud.multimode; i++)
			memcpy(ud.wchoice[i],ud.mywchoice,sizeof(ud.mywchoice)); // assuming.... :-(

	// FIX_00076: Added default names for bots + fixed a "killed <name>" bug in Fakeplayers with AI
	if(!(g&MODE_DEMO) && numplayers<2 && ud.multimode > 1)
		for(i=connecthead;i>=0;i=connectpoint2[i])
			if (i!=myconnectindex)
			{
				memcpy(ud.wchoice[0],ud.mywchoice,sizeof(ud.mywchoice));

				//	add bot's names
				do 
				{ 
					j = rand()%BOT_MAX_NAME; 
				} 
				while(bot_used[j]);

				strcpy(ud.user_name[i], bot_names[j]);
				bot_used[j] = true;
				ps[i].fakeplayer = 1 + ud.playerai;  // = true if fakerplayer. (==2 if AI)
			}

    if(ud.recstat != 2) for(i=0;i<MAXPLAYERS;i++)
    {
        ps[i].aim_mode = aimmode[i];
        if(ud.multimode > 1 && ud.coop == 1 && ud.last_level >= 0)
        {
            for(j=0;j<MAX_WEAPONS;j++)
            {
                ps[i].ammo_amount[j] = tsbar[i].ammo_amount[j];
                ps[i].gotweapon[j] = tsbar[i].gotweapon[j];
            }
            ps[i].shield_amount = tsbar[i].shield_amount;
            ps[i].curr_weapon = tsbar[i].curr_weapon;
            ps[i].inven_icon = tsbar[i].inven_icon;

            ps[i].firstaid_amount = tsbar[i].firstaid_amount;
            ps[i].steroids_amount= tsbar[i].steroids_amount;
            ps[i].holoduke_amount = tsbar[i].holoduke_amount;
            ps[i].jetpack_amount = tsbar[i].jetpack_amount;
            ps[i].heat_amount = tsbar[i].heat_amount;
            ps[i].scuba_amount= tsbar[i].scuba_amount;
            ps[i].boot_amount = tsbar[i].boot_amount;
        }
    }

    numplayersprites = 0;
//    circ = 2048/ud.multimode;

    which_palookup = 9;
    j = connecthead;
    i = headspritestat[10];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        if( numplayersprites == MAXPLAYERS)
            gameexit("\nToo many player sprites (max 16.)");

//        if(numplayersprites == 0)
//        {
//            firstx = ps[0].posx;
//            firsty = ps[0].posy;
//        }

        po[numplayersprites].ox = s->x;
        po[numplayersprites].oy = s->y;
        po[numplayersprites].oz = s->z;
        po[numplayersprites].oa = s->ang;
        po[numplayersprites].os = s->sectnum;

        numplayersprites++;
        if(j >= 0)
        {
            s->owner = i;
            s->shade = 0;
            s->xrepeat = 42;
            s->yrepeat = 36;
            s->cstat = 1+256;
            s->xoffset = 0;
            s->clipdist = 64;

            if( (g&MODE_EOL) != MODE_EOL || ps[j].last_extra == 0)
            {
                ps[j].last_extra = max_player_health;
                s->extra = max_player_health;
            }
            else s->extra = ps[j].last_extra;

            s->yvel = j;

            if(s->pal == 0)
            {
                s->pal = ps[j].palookup = which_palookup;
                which_palookup++;
                if( which_palookup >= 17 ) which_palookup = 9;
            }
            else ps[j].palookup = s->pal;

            ps[j].i = i;
            ps[j].frag_ps = j;
            hittype[i].owner = i;

            hittype[i].bposx = ps[j].bobposx = ps[j].oposx = ps[j].posx =        s->x;
            hittype[i].bposy = ps[j].bobposy = ps[j].oposy = ps[j].posy =        s->y;
            hittype[i].bposz = ps[j].oposz = ps[j].posz =        s->z;
            ps[j].oang  = ps[j].ang  =        s->ang;

            updatesector(s->x,s->y,&ps[j].cursectnum);

            j = connectpoint2[j];

        }
        else deletesprite(i);
        i = nexti;
    }
}

void clearfrags(void)
{
    short i;

    for(i = 0;i<MAXPLAYERS;i++)
        ps[i].frag = ps[i].fraggedself = 0;
     clearbufbyte(&frags[0][0],(MAXPLAYERS*MAXPLAYERS)<<1,0L);
}

void resettimevars(void)
{
    vel = svel = angvel = horiz = 0;

    totalclock = 0L;
    cloudtotalclock = 0L;
    ototalclock = 0L;
    lockclock = 0L;
    ready2send = 1;
}


void genspriteremaps(void)
{
    int32_t j,fp;
    int8_t look_pos;
    char  *lookfn = "lookup.dat";
    uint8_t  numl;

    fp = TCkopen4load(lookfn,0);
    if(fp != -1)
        kread(fp,(uint8_t  *)&numl,1);
    else
	{
        gameexit("\nERROR: File 'LOOKUP.DAT' not found.");
		return;
	}

    for(j=0;j < numl;j++)
    {
        kread(fp,(int8_t *)&look_pos,1);
        kread(fp,tempbuf,256);
        makepalookup((int32_t)look_pos,(uint8_t*)tempbuf,0,0,0,1);
    }

    kread(fp,&waterpal[0],768);
    kread(fp,&slimepal[0],768);
    kread(fp,&titlepal[0],768);
    kread(fp,&drealms[0],768);
    kread(fp,&endingpal[0],768);

    palette[765] = palette[766] = palette[767] = 0;
    slimepal[765] = slimepal[766] = slimepal[767] = 0;
    waterpal[765] = waterpal[766] = waterpal[767] = 0;

    kclose(fp);
}

void waitforeverybody()
{
    int32_t i;

    if (numplayers < 2) 
	{
		//printf("numplayers < 2 (num: %d)\n", numplayers);
		return;
	}
#ifdef _DEBUG_NETWORKING_
	else
	{
		printf("numplayers == %d\n", numplayers);
	}
#endif

	printf("waitforeverybody()\n");

    packbuf[0] = 250;
    for(i=connecthead;i>=0;i=connectpoint2[i])
    {

        if (i != myconnectindex)
		{
			sendpacket(i,packbuf,1);
		}
    }

    playerreadyflag[myconnectindex]++;
    do
    {
        _idle();  /* let input queue run... */
        getpackets();
        for(i=connecthead;i>=0;i=connectpoint2[i])
        {
            if (playerreadyflag[i] < playerreadyflag[myconnectindex])
			{
				break;
			}
        }
    } while (i >= 0);

}

void dofrontscreens(void)
{
    int32_t i,j;

    if(ud.recstat != 2)
    {
        ps[myconnectindex].palette = palette;
        for(j=0;j<63;j+=7) palto(0,0,0,j);
        i = ud.screen_size;
        ud.screen_size = 0;
        vscrn();
        clearview(0L);

        rotatesprite(320<<15,200<<15,65536L,0,LOADSCREEN,0,0,2+8+64,0,0,xdim-1,ydim-1);

        if( boardfilename[0] != 0 && ud.level_number == 7 && ud.volume_number == 0 )
        {
            menutext(160,90,0,0,"ENTERING USER MAP");
            gametextpal(160,90+10,boardfilename,14,2);
        }
        else
        {
            menutext(160,90,0,0,"ENTERING");
            menutext(160,90+16+8,0,0,level_names[(ud.volume_number*11) + ud.level_number]);
        }

        nextpage();

        for(j=63;j>0;j-=7) palto(0,0,0,j);

        KB_FlushKeyboardQueue();
        ud.screen_size = i;
    }
    else
    {
        clearview(0L);
        ps[myconnectindex].palette = palette;
        palto(0,0,0,0);
        rotatesprite(320<<15,200<<15,65536L,0,LOADSCREEN,0,0,2+8+64,0,0,xdim-1,ydim-1);
        menutext(160,105,0,0,"LOADING...");
        nextpage();
    }
}

void clearfifo(void)
{
    syncvaltail = 0L;
    syncvaltottail = 0L;
    syncstat = 0;
    bufferjitter = 1;
    mymaxlag = otherminlag = 0;

    movefifoplc = movefifosendplc = fakemovefifoplc = 0;
    avgfvel = avgsvel = avgavel = avghorz = avgbits = 0;
    otherminlag = mymaxlag = 0;

    clearbufbyte(myminlag,MAXPLAYERS<<2,0L);
    clearbufbyte(&loc,sizeof(input),0L);
    clearbufbyte(&sync[0],sizeof(sync),0L);
    clearbufbyte(inputfifo,sizeof(input)*MOVEFIFOSIZ*MAXPLAYERS,0L);

    clearbuf(movefifoend,MAXPLAYERS,0L);
    clearbuf(syncvalhead,MAXPLAYERS,0L);
    clearbuf(myminlag,MAXPLAYERS,0L);

//    clearbufbyte(playerquitflag,MAXPLAYERS,0x01);
}

void resetmys(void)
{
      myx = omyx = ps[myconnectindex].posx;
      myy = omyy = ps[myconnectindex].posy;
      myz = omyz = ps[myconnectindex].posz;
      myxvel = myyvel = myzvel = 0;
      myang = omyang = ps[myconnectindex].ang;
      myhoriz = omyhoriz = ps[myconnectindex].horiz;
      myhorizoff = omyhorizoff = ps[myconnectindex].horizoff;
      mycursectnum = ps[myconnectindex].cursectnum;
      myjumpingcounter = ps[myconnectindex].jumping_counter;
      myjumpingtoggle = ps[myconnectindex].jumping_toggle;
      myonground = ps[myconnectindex].on_ground;
      myhardlanding = ps[myconnectindex].hard_landing;
      myreturntocenter = ps[myconnectindex].return_to_center;
}

void enterlevel(uint8_t  g)
{
    short i;
    int32_t l;
    char  levname[256];
	char  fulllevelfilename[512];
    char text[512];

	KB_ClearKeyDown(sc_Pause); // avoid entering in pause mode.
	
    if( (g&MODE_DEMO) != MODE_DEMO ) ud.recstat = ud.m_recstat;
    ud.respawn_monsters = ud.m_respawn_monsters;
    ud.respawn_items    = ud.m_respawn_items;
    ud.respawn_inventory    = ud.m_respawn_inventory;
    ud.monsters_off = ud.m_monsters_off;
    ud.coop = ud.m_coop;
    ud.marker = ud.m_marker;
    ud.ffire = ud.m_ffire;

    if( (g&MODE_DEMO) == 0 && ud.recstat == 2)
        ud.recstat = 0;

    FX_StopAllSounds();
    clearsoundlocks();
    FX_SetReverb(0);

    i = ud.screen_size;
    ud.screen_size = 0;
    dofrontscreens();
    vscrn();
    ud.screen_size = i;

if (!VOLUMEONE)
{
    
    if( boardfilename[0] != 0 && ud.m_level_number == 7 && ud.m_volume_number == 0 )
    {
		sprintf(fulllevelfilename, "%s\\%s",  getGameDir(), boardfilename);
		if(!SafeFileExists(fulllevelfilename))
		{
			sprintf(fulllevelfilename, "%s", boardfilename);
		}

        if ( loadboard( fulllevelfilename,&ps[0].posx, &ps[0].posy, &ps[0].posz, &ps[0].ang,&ps[0].cursectnum ) == -1 )
        {
            sprintf(text,"User Map %s not found!\n",fulllevelfilename);
            gameexit(text);
        }
    }
    else
    {
		sprintf(fulllevelfilename, "%s\\%s",  getGameDir(), level_file_names[ (ud.volume_number*11)+ud.level_number]);
		if(!SafeFileExists(fulllevelfilename))
		{
			sprintf(fulllevelfilename, "%s", level_file_names[ (ud.volume_number*11)+ud.level_number]);
		}
		printf("filename=%s\n",fulllevelfilename );
		if ( loadboard(fulllevelfilename ,&ps[0].posx, &ps[0].posy, &ps[0].posz, &ps[0].ang,&ps[0].cursectnum ) == -1)
        {
			sprintf(text,"Internal Map %s not found! Not using the right grp file?\n",level_file_names[(ud.volume_number*11)+ud.level_number]);
			gameexit(text);
		}
    }

} else {

    l = strlen(level_file_names[ (ud.volume_number*11)+ud.level_number]);
    copybufbyte( level_file_names[ (ud.volume_number*11)+ud.level_number],&levname[0],l);
    levname[l] = 0; 

	printf("levname=%s\n",levname );

    if ( (ud.volume_number > 1) || loadboard( levname,&ps[0].posx, &ps[0].posy, &ps[0].posz, &ps[0].ang,&ps[0].cursectnum ) == -1)
    {
        sprintf(text,"Internal Map %s not found in Shareware grp pack!\n",level_file_names[(ud.volume_number*11)+ud.level_number]);
        gameexit(text);
    }
}

    clearbufbyte(gotpic,sizeof(gotpic),0L);

    prelevel(g);

    allignwarpelevators();
    resetpspritevars(g);

#ifdef CHECK_XDUKE_REV
	if((g&MODE_DEMO) != MODE_DEMO ) // don't check for demo
		for(i=connecthead;i>=0;i=connectpoint2[i])
		{
			// all the players must have been validated to have the same rev as we do
			if(!ud.rev[i][0] && !ps[i].fakeplayer)
				Error(EXIT_SUCCESS, "Your opponent [%s] is using an unknown version of xDuke."
									"You are using v%d.%d\n",
				ud.user_name[i], CHOCOLATE_DUKE_REV_X, CHOCOLATE_DUKE_REV_DOT_Y);
		}
#endif


    automapping = 0;

    if(ud.recstat != 2) MUSIC_StopSong();

    cacheit();
    docacheit();

    if(ud.recstat != 2)
    {
        music_select = (ud.volume_number*11) + ud.level_number;
        playmusic(&music_fn[0][music_select][0]);
    }

    if( (g&MODE_GAME) || (g&MODE_EOL) )
        ps[myconnectindex].gm = MODE_GAME;
    else if(g&MODE_RESTART)
    {
        if(ud.recstat == 2)
            ps[myconnectindex].gm = MODE_DEMO;
        else ps[myconnectindex].gm = MODE_GAME;
    }

    if( (ud.recstat == 1) && (g&MODE_RESTART) != MODE_RESTART )
        opendemowrite();

//if (VOLUMEONE)
//    if(ud.level_number == 0 && ud.recstat != 2) FTA(40,&ps[myconnectindex]);

#ifdef CHECK_XDUKE_REV
	sprintf(fta_quotes[103],"Chocolate Duke3D v%d.%d", ud.rev[myconnectindex][2], ud.rev[myconnectindex][3]);
	FTA(103,&ps[myconnectindex],1);
#endif

	if(ud.auto_aim==1 && ud.recstat != 2)
	{
		sprintf(fta_quotes[103],"Autoaim set to Bullet only");
		FTA(103,&ps[myconnectindex],1);
	}

	if(nHostForceDisableAutoaim && ud.recstat != 2)
	{
		sprintf(fta_quotes[103],"Autoaim disabled by host");
		FTA(103,&ps[myconnectindex],1);
	}
	

    for(i=connecthead;i>=0;i=connectpoint2[i])
        switch(sector[sprite[ps[i].i].sectnum].floorpicnum)
        {
            case HURTRAIL:
            case FLOORSLIME:
            case FLOORPLASMA:
                resetweapons(i);
                resetinventory(i);
                ps[i].gotweapon[PISTOL_WEAPON] = 0;
                ps[i].ammo_amount[PISTOL_WEAPON] = 0;
                ps[i].curr_weapon = KNEE_WEAPON;
                ps[i].kickback_pic = 0;
                break;
        }

      //PREMAP.C - replace near the my's at the end of the file

     resetmys();

     ps[myconnectindex].palette = palette;
     palto(0,0,0,0);

     setpal(&ps[myconnectindex]);
     flushperms();

     everyothertime = 0;
     global_random = 0;

     ud.last_level = ud.level_number+1;

     clearfifo();

     for(i=numinterpolations-1;i>=0;i--) bakipos[i] = *curipos[i];

     restorepalette = 1;

     flushpackets();
     waitforeverybody();

     palto(0,0,0,0);
     vscrn();
     clearview(0L);
     drawbackground();

     clearbufbyte(playerquitflag,MAXPLAYERS,0x01010101);
     ps[myconnectindex].over_shoulder_on = 0;

     clearfrags();

     resettimevars();  // Here we go

	 if(numplayers > 1) 
	 {
		buf[0] = 132;	// xDuke TAG ID
        buf[1] = mapCRC & 0xFF;
		buf[2] = (mapCRC>>8) & 0xFF;

		for(i=connecthead;i>=0;i=connectpoint2[i])
			if( i != myconnectindex )
				sendpacket(i,(uint8_t*)buf,3);
	 }

	 ud.mapCRC[myconnectindex] = mapCRC;

}

/*
Duke Nukem V

Layout:

      Settings:
        Suburbs
          Duke inflitrating neighborhoods inf. by aliens
        Death Valley:
          Sorta like a western.  Bull-skulls halb buried in the sand
          Military compound:  Aliens take over nuke-missle silo, duke
            must destroy.
          Abondend Aircraft field
        Vegas:
          Blast anything bright!  Alien lights camoflauged.
          Alien Drug factory. The Blue Liquid
        Mountainal Cave:
          Interior cave battles.
        Jungle:
          Trees, canopee, animals, a mysterious hole in the earth
        Penetencury:
          Good use of spotlights:
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

Programming:
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
