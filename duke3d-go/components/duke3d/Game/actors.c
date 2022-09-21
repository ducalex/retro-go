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

extern int32_t numenvsnds;
uint8_t  actor_tog;

void updateinterpolations()  //Stick at beginning of domovethings
{
	int32_t i;

	for(i=numinterpolations-1;i>=0;i--) oldipos[i] = *curipos[i];
}


void setinterpolation(int32_t *posptr)
{
	int32_t i;

	if (numinterpolations >= MAXINTERPOLATIONS) return;
	for(i=numinterpolations-1;i>=0;i--)
		if (curipos[i] == posptr) return;
	curipos[numinterpolations] = posptr;
	oldipos[numinterpolations] = *posptr;
	numinterpolations++;
}

void stopinterpolation(int32_t *posptr)
{
	int32_t i;

	for(i=numinterpolations-1;i>=startofdynamicinterpolations;i--)
		if (curipos[i] == posptr)
		{
			numinterpolations--;
			oldipos[i] = oldipos[numinterpolations];
			bakipos[i] = bakipos[numinterpolations];
			curipos[i] = curipos[numinterpolations];
		}
}

void dointerpolations(int32_t smoothratio)       //Stick at beginning of drawscreen
{
	int32_t i, j, odelta, ndelta;

	ndelta = 0; j = 0;
	for(i=numinterpolations-1;i>=0;i--)
	{
		bakipos[i] = *curipos[i];
		odelta = ndelta; ndelta = (*curipos[i])-oldipos[i];
		if (odelta != ndelta) j = mulscale16(ndelta,smoothratio);
		*curipos[i] = oldipos[i]+j;
	}
}

void restoreinterpolations()  //Stick at end of drawscreen
{
	int32_t i;

	for(i=numinterpolations-1;i>=0;i--) *curipos[i] = bakipos[i];
}

int32_t ceilingspace(int16_t sectnum)
{
    if( (sector[sectnum].ceilingstat&1) && sector[sectnum].ceilingpal == 0 )
    {
        switch(sector[sectnum].ceilingpicnum)
        {
            case MOONSKY1:
            case BIGORBIT1:
                return 1;
        }
    }
    return 0;
}

int32_t floorspace(int16_t sectnum)
{
    if( (sector[sectnum].floorstat&1) && sector[sectnum].ceilingpal == 0 )
    {
        switch(sector[sectnum].floorpicnum)
        {
            case MOONSKY1:
            case BIGORBIT1:
                return 1;
        }
    }
    return 0;
}

void addammo( int16_t weapon,struct player_struct *p,int16_t amount)
{
   p->ammo_amount[weapon] += amount;

   if( p->ammo_amount[weapon] > max_ammo_amount[weapon] )
        p->ammo_amount[weapon] = max_ammo_amount[weapon];
}

void addweapon( struct player_struct *p,int16_t weapon)
{
	int added_new_weapon = false;

    if ( p->gotweapon[weapon] == 0 )
    {
        p->gotweapon[weapon] = 1;
        if(weapon == SHRINKER_WEAPON)
            p->gotweapon[GROW_WEAPON] = 1;
	 
	// FIX_00012: added "weapon autoswitch" toggle allowing to turn the autoswitch off
	// when picking up new weapons. The weapon sound on pickup will remain on, to not 
	// affect the opponent's gameplay (so he can still hear you picking up new weapons)
  	if(p->weaponautoswitch)  // Anti antiswitch ordered
			added_new_weapon = true;
    }

	if (added_new_weapon==false || 
		ud.playing_demo_rev == BYTEVERSION_27     ||
		ud.playing_demo_rev == BYTEVERSION_28     || 
		ud.playing_demo_rev == BYTEVERSION_116    || 
		ud.playing_demo_rev == BYTEVERSION_117 )
		// don't block the weapon change on 1st pick up if playing an old demo
	{
		p->random_club_frame = 0;

		if(p->holster_weapon == 0)
		{
			p->weapon_pos = -1;
			p->last_weapon = p->curr_weapon;
		}
		else
		{
			p->weapon_pos = 10;
			p->holster_weapon = 0;
			p->last_weapon = -1;
		}

		p->kickback_pic = 0;
		p->curr_weapon = weapon;
	}

    switch(weapon)
    {
        case KNEE_WEAPON:
        case TRIPBOMB_WEAPON:
        case HANDREMOTE_WEAPON:
        case HANDBOMB_WEAPON:     break;
        case SHOTGUN_WEAPON:      spritesound(SHOTGUN_COCK,p->i);break;
        case PISTOL_WEAPON:       spritesound(INSERT_CLIP,p->i);break;
                    default:      spritesound(SELECT_WEAPON,p->i);break;
    }
	
	vscrn(); // FIX_00056: Refresh issue w/FPS, small Weapon and custom FTA, when screen resized down

}

void checkavailinven( struct player_struct *p )
{

    if(p->firstaid_amount > 0)
        p->inven_icon = 1;
    else if(p->steroids_amount > 0)
        p->inven_icon = 2;
    else if(p->holoduke_amount > 0)
        p->inven_icon = 3;
    else if(p->jetpack_amount > 0)
        p->inven_icon = 4;
    else if(p->heat_amount > 0)
        p->inven_icon = 5;
    else if(p->scuba_amount > 0)
        p->inven_icon = 6;
    else if(p->boot_amount > 0)
        p->inven_icon = 7;
    else p->inven_icon = 0;
}

void checkavailweapon( struct player_struct *p )
{
    short i,snum;
    int32 weap;

    if(p->wantweaponfire >= 0)
    {
        weap = p->wantweaponfire;
        p->wantweaponfire = -1;

        if(weap == p->curr_weapon) return;
        else if( p->gotweapon[weap] && p->ammo_amount[weap] > 0 )
        {
            addweapon(p,weap);
            return;
        }
    }

    weap = p->curr_weapon;
    if( p->gotweapon[weap] && p->ammo_amount[weap] > 0 )
        return;

    snum = sprite[p->i].yvel;

    for(i=0;i<10;i++)
    {
        weap = ud.wchoice[snum][i];
		if (VOLUMEONE)
			if(weap > 6) continue;

        if(weap == 0) weap = 9;
        else weap--;

        if( weap == 0 || ( p->gotweapon[weap] && p->ammo_amount[weap] > 0 ) )
            break;
    }

    if(i == 10) weap = 0;

    // Found the weapon

    p->last_weapon  = p->curr_weapon;
    p->random_club_frame = 0;
    p->curr_weapon  = weap;
    p->kickback_pic = 0;
    if(p->holster_weapon == 1)
    {
        p->holster_weapon = 0;
        p->weapon_pos = 10;
    }
    else p->weapon_pos   = -1;
}

 /*
void checkavailweapon( struct player_struct *p )
{
    short i,okay,check_shoot,check_bombs;

    if(p->ammo_amount[p->curr_weapon] > 0) return;
    okay = check_shoot = check_bombs = 0;

    switch(p->curr_weapon)
    {
        case PISTOL_WEAPON:
        case CHAINGUN_WEAPON:
        case SHOTGUN_WEAPON:
#ifndef VOLUMEONE
        case FREEZE_WEAPON:
        case DEVISTATOR_WEAPON:
        case SHRINKER_WEAPON:
        case GROW_WEAPON:
#endif
        case RPG_WEAPON:
        case KNEE_WEAPON:
            check_shoot = 1;
            break;
        case HANDBOMB_WEAPON:
        case HANDREMOTE_WEAPON:
#ifndef VOLUMEONE
        case TRIPBOMB_WEAPON:
#endif
            check_bombs = 1;
            break;
    }

    CHECK_SHOOT:
    if(check_shoot)
    {
        for(i = p->curr_weapon+1; i < MAX_WEAPONS;i++)
            switch(i)
            {
                case PISTOL_WEAPON:
                case CHAINGUN_WEAPON:
                case SHOTGUN_WEAPON:
#ifndef VOLUMEONE
                case FREEZE_WEAPON:
                case SHRINKER_WEAPON:
                case GROW_WEAPON:
                case DEVISTATOR_WEAPON:
#endif
                    if ( p->gotweapon[i] && p->ammo_amount[i] > 0 )
                    {
                        okay = i;
                        goto OKAY_HERE;
                    }
                    break;
            }

        for(i = p->curr_weapon-1; i > 0;i--)
            switch(i)
            {
                case PISTOL_WEAPON:
                case CHAINGUN_WEAPON:
                case SHOTGUN_WEAPON:
#ifndef VOLUMEONE
                case FREEZE_WEAPON:
                case DEVISTATOR_WEAPON:
                case SHRINKER_WEAPON:
                case GROW_WEAPON:
#endif
                    if ( p->gotweapon[i] && p->ammo_amount[i] > 0 )
                    {
                        okay = i;
                        goto OKAY_HERE;
                    }
                    break;
            }

        if( p->gotweapon[RPG_WEAPON] && p->ammo_amount[RPG_WEAPON] > 0 )
        {
            okay = RPG_WEAPON;
            goto OKAY_HERE;
        }

        if(check_bombs == 0)
            check_bombs = 1;
        else
        {
            addweapon(p,KNEE_WEAPON);
            return;
        }
    }

    if(check_bombs)
    {
        for(i = p->curr_weapon-1; i > 0;i--)
            switch(i)
            {
                case HANDBOMB_WEAPON:
#ifndef VOLUMEONE
                case TRIPBOMB_WEAPON:
#endif
                    if ( p->gotweapon[i] && p->ammo_amount[i] > 0 )
                    {
                        okay = i;
                        goto OKAY_HERE;
                    }
                    break;
            }

        for(i = p->curr_weapon+1; i < MAX_WEAPONS;i++)
            switch(i)
            {
                case HANDBOMB_WEAPON:
#ifdef VOLUMEONE
                case TRIPBOMB_WEAPON:
#endif
                    if ( p->gotweapon[i] && p->ammo_amount[i] > 0 )
                    {
                        okay = i;
                        goto OKAY_HERE;
                    }
                    break;
            }

        if(check_shoot == 0)
        {
            check_shoot = 1;
            goto CHECK_SHOOT;
        }
        else
        {
            addweapon(p,KNEE_WEAPON);
            return;
        }
    }

    OKAY_HERE:

    if(okay)
    {
        p->last_weapon  = p->curr_weapon;
        p->random_club_frame = 0;
        p->curr_weapon  = okay;
        p->kickback_pic = 0;
        if(p->holster_weapon == 1)
        {
            p->holster_weapon = 0;
            p->weapon_pos = 10;
        }
        else p->weapon_pos   = -1;
        return;
    }
}
   */

int32_t ifsquished(short i, short p)
{
    sectortype *sc;
    uint8_t  squishme;
    int32_t floorceildist;

    if(PN == APLAYER && ud.clipping)
        return 0;

    sc = &sector[SECT];
    floorceildist = sc->floorz - sc->ceilingz;

    if(sc->lotag != 23)
    {
        if(sprite[i].pal == 1)
            squishme = floorceildist < (32<<8) && (sc->lotag&32768) == 0;
        else
            squishme = floorceildist < (12<<8); // && (sc->lotag&32768) == 0;
    }
    else squishme = 0;

    if( squishme )
    {
        FTA(10,&ps[p],0);

        if(badguy(&sprite[i])) sprite[i].xvel = 0;

        if(sprite[i].pal == 1)
        {
            hittype[i].picnum = SHOTSPARK1;
            hittype[i].extra = 1;
            return 0;
        }

        return 1;
    }
    return 0;
}

void hitradius( short i, int32_t  r, int32_t  hp1, int32_t  hp2, int32_t  hp3, int32_t  hp4 )
{
    spritetype *s,*sj;
    walltype *wal;
    int32_t d, q, x1, y1;
    int32_t sectcnt, sectend, dasect, startwall, endwall, nextsect;
    short j,k,p,x,nextj,sect;
    uint8_t  statlist[] = {0,1,6,10,12,2,5};
    short *tempshort = (short *)tempbuf;

    s = &sprite[i];

    if(s->picnum == RPG && s->xrepeat < 11) goto SKIPWALLCHECK;

    if(s->picnum != SHRINKSPARK)
    {
        tempshort[0] = s->sectnum;
        dasect = s->sectnum;
        sectcnt = 0; sectend = 1;

        do
        {
            dasect = tempshort[sectcnt++];
            if(((sector[dasect].ceilingz-s->z)>>8) < r)
            {
               d = klabs(wall[sector[dasect].wallptr].x-s->x)+klabs(wall[sector[dasect].wallptr].y-s->y);
               if(d < r)
                    checkhitceiling(dasect);
               else
               {
                    d = klabs(wall[wall[wall[sector[dasect].wallptr].point2].point2].x-s->x)+klabs(wall[wall[wall[sector[dasect].wallptr].point2].point2].y-s->y);
                    if(d < r)
                        checkhitceiling(dasect);
               }
           }

           startwall = sector[dasect].wallptr;
           endwall = startwall+sector[dasect].wallnum;
           for(x=startwall,wal=&wall[startwall];x<endwall;x++,wal++)
               if( ( klabs(wal->x-s->x)+klabs(wal->y-s->y) ) < r)
           {
               nextsect = wal->nextsector;
               if (nextsect >= 0)
               {
                   for(dasect=sectend-1;dasect>=0;dasect--)
                       if (tempshort[dasect] == nextsect) break;
                   if (dasect < 0) tempshort[sectend++] = nextsect;
               }
               x1 = (((wal->x+wall[wal->point2].x)>>1)+s->x)>>1;
               y1 = (((wal->y+wall[wal->point2].y)>>1)+s->y)>>1;
               updatesector(x1,y1,&sect);
               if( sect >= 0 && cansee(x1,y1,s->z,sect,s->x,s->y,s->z,s->sectnum ) )
                   checkhitwall(i,x,wal->x,wal->y,s->z,s->picnum);
           }
        }
        while (sectcnt < sectend);
    }

    SKIPWALLCHECK:

    q = -(16<<8)+(TRAND&((32<<8)-1));

    for(x = 0;x<7;x++)
    {
        j = headspritestat[statlist[x]];
        while(j >= 0)
        {
            nextj = nextspritestat[j];
            sj = &sprite[j];

            if( x == 0 || x >= 5 || AFLAMABLE(sj->picnum) )
            {
                if( s->picnum != SHRINKSPARK || (sj->cstat&257) )
                    if( dist( s, sj ) < r )
                    {
                        if( badguy(sj) && !cansee( sj->x, sj->y,sj->z+q, sj->sectnum, s->x, s->y, s->z+q, s->sectnum) )
                            goto BOLT;
                        checkhitsprite( j, i );
                    }
            }
            else if( sj->extra >= 0 && sj != s && ( sj->picnum == TRIPBOMB || badguy(sj) || sj->picnum == QUEBALL || sj->picnum == STRIPEBALL || (sj->cstat&257) || sj->picnum == DUKELYINGDEAD ) )
            {
                if( s->picnum == SHRINKSPARK && sj->picnum != SHARK && ( j == s->owner || sj->xrepeat < 24 ) )
                {
                    j = nextj;
                    continue;
                }
                if( s->picnum == MORTER && j == s->owner)
                {
                    j = nextj;
                    continue;
                }

                if(sj->picnum == APLAYER) sj->z -= PHEIGHT;
                d = dist( s, sj );
                if(sj->picnum == APLAYER) sj->z += PHEIGHT;

                if ( d < r && cansee( sj->x, sj->y, sj->z-(8<<8), sj->sectnum, s->x, s->y, s->z-(12<<8), s->sectnum) )
                {
                    hittype[j].ang = getangle(sj->x-s->x,sj->y-s->y);

                    if ( s->picnum == RPG && sj->extra > 0)
                        hittype[j].picnum = RPG;
                    else
                    {
                        if( s->picnum == SHRINKSPARK )
                            hittype[j].picnum = SHRINKSPARK;
                        else hittype[j].picnum = RADIUSEXPLOSION;
                    }

                    if(s->picnum != SHRINKSPARK)
                    {
                        if ( d < r/3 )
                        {
                            if(hp4 == hp3) hp4++;
                            hittype[j].extra = hp3 + (TRAND%(hp4-hp3));
                        }
                        else if ( d < 2*r/3 )
                        {
                            if(hp3 == hp2) hp3++;
                            hittype[j].extra = hp2 + (TRAND%(hp3-hp2));
                        }
                        else if ( d < r )
                        {
                            if(hp2 == hp1) hp2++;
                            hittype[j].extra = hp1 + (TRAND%(hp2-hp1));
                        }

                        if( sprite[j].picnum != TANK && sprite[j].picnum != ROTATEGUN && sprite[j].picnum != RECON && sprite[j].picnum != BOSS1 && sprite[j].picnum != BOSS2 && sprite[j].picnum != BOSS3 && sprite[j].picnum != BOSS4 )
                        {
                            if(sj->xvel < 0) sj->xvel = 0;
                            sj->xvel += (s->extra<<2);
                        }

                        if( sj->picnum == PODFEM1 || sj->picnum == FEM1 ||
                            sj->picnum == FEM2 || sj->picnum == FEM3 ||
                            sj->picnum == FEM4 || sj->picnum == FEM5 ||
                            sj->picnum == FEM6 || sj->picnum == FEM7 ||
                            sj->picnum == FEM8 || sj->picnum == FEM9 ||
                            sj->picnum == FEM10 || sj->picnum == STATUE ||
                            sj->picnum == STATUEFLASH || sj->picnum == SPACEMARINE || sj->picnum == QUEBALL || sj->picnum == STRIPEBALL)
                                checkhitsprite( j, i );
                    }
                    else if(s->extra == 0) hittype[j].extra = 0;

                    if ( sj->picnum != RADIUSEXPLOSION &&
                        s->owner >= 0 && sprite[s->owner].statnum < MAXSTATUS )
                    {
                        if(sj->picnum == APLAYER)
                        {
                            p = sj->yvel;
                            if(ps[p].newowner >= 0)
                            {
                                ps[p].newowner = -1;
                                ps[p].posx = ps[p].oposx;
                                ps[p].posy = ps[p].oposy;
                                ps[p].posz = ps[p].oposz;
                                ps[p].ang = ps[p].oang;
                                updatesector(ps[p].posx,ps[p].posy,&ps[p].cursectnum);
                                setpal(&ps[p]);

                                k = headspritestat[1];
                                while(k >= 0)
                                {
                                    if(sprite[k].picnum==CAMERA1)
                                        sprite[k].yvel = 0;
                                    k = nextspritestat[k];
                                }
                            }
                        }
                        hittype[j].owner = s->owner;
                    }
                }
            }
            BOLT:
            j = nextj;
        }
    }
}


IRAM_ATTR int movesprite(short spritenum, int32_t xchange, int32_t ychange, int32_t zchange, uint32_t cliptype)
{
    int32_t daz,h, oldx, oldy;
    short retval, dasectnum, cd;
    uint8_t  bg;

    bg = badguy(&sprite[spritenum]);

    if(sprite[spritenum].statnum == 5 || (bg && sprite[spritenum].xrepeat < 4 ) )
    {
        sprite[spritenum].x += (xchange*TICSPERFRAME)>>2;
        sprite[spritenum].y += (ychange*TICSPERFRAME)>>2;
        sprite[spritenum].z += (zchange*TICSPERFRAME)>>2;
        if(bg)
            setsprite(spritenum,sprite[spritenum].x,sprite[spritenum].y,sprite[spritenum].z);
        return 0;
    }

    dasectnum = sprite[spritenum].sectnum;

	daz = sprite[spritenum].z;
    h = ((tiles[sprite[spritenum].picnum].dim.height * sprite[spritenum].yrepeat)<<1);
    daz -= h;

    if( bg )
    {
        oldx = sprite[spritenum].x;
        oldy = sprite[spritenum].y;

        if( sprite[spritenum].xrepeat > 60 )
            retval = clipmove(&sprite[spritenum].x,&sprite[spritenum].y,&daz,&dasectnum,((xchange*TICSPERFRAME)<<11),((ychange*TICSPERFRAME)<<11),1024L,(4<<8),(4<<8),cliptype);
        else
        {
            if(sprite[spritenum].picnum == LIZMAN)
                cd = 292L;
            else if( (actortype[sprite[spritenum].picnum]&3) )
                cd = sprite[spritenum].clipdist<<2;
            else
                cd = 192L;

            retval = clipmove(&sprite[spritenum].x,&sprite[spritenum].y,&daz,&dasectnum,((xchange*TICSPERFRAME)<<11),((ychange*TICSPERFRAME)<<11),cd,(4<<8),(4<<8),cliptype);
        }

        if( dasectnum < 0 || ( dasectnum >= 0 &&
            ( ( hittype[spritenum].actorstayput >= 0 && hittype[spritenum].actorstayput != dasectnum ) ||
              ( ( sprite[spritenum].picnum == BOSS2 ) && sprite[spritenum].pal == 0 && sector[dasectnum].lotag != 3 ) ||
              ( ( sprite[spritenum].picnum == BOSS1 || sprite[spritenum].picnum == BOSS2 ) && sector[dasectnum].lotag == 1 ) ||
              ( sector[dasectnum].lotag == 1 && ( sprite[spritenum].picnum == LIZMAN || ( sprite[spritenum].picnum == LIZTROOP && sprite[spritenum].zvel == 0 ) ) )
            ) )
          )
        {
                sprite[spritenum].x = oldx;
                sprite[spritenum].y = oldy;
                if(sector[dasectnum].lotag == 1 && sprite[spritenum].picnum == LIZMAN)
                    sprite[spritenum].ang = (TRAND&2047);
                else if( (hittype[spritenum].temp_data[0]&3) == 1 && sprite[spritenum].picnum != COMMANDER )
                    sprite[spritenum].ang = (TRAND&2047);
                setsprite(spritenum,oldx,oldy,sprite[spritenum].z);
                if(dasectnum < 0) dasectnum = 0;
                return (16384+dasectnum);
        }
        if( (retval&49152) >= 32768 && (hittype[spritenum].cgg==0) ) sprite[spritenum].ang += 768;
    }
    else
    {
        if(sprite[spritenum].statnum == 4)
            retval =
                clipmove(&sprite[spritenum].x,&sprite[spritenum].y,&daz,&dasectnum,((xchange*TICSPERFRAME)<<11),((ychange*TICSPERFRAME)<<11),8L,(4<<8),(4<<8),cliptype);
        else
            retval =
                clipmove(&sprite[spritenum].x,&sprite[spritenum].y,&daz,&dasectnum,((xchange*TICSPERFRAME)<<11),((ychange*TICSPERFRAME)<<11),(int32_t)(sprite[spritenum].clipdist<<2),(4<<8),(4<<8),cliptype);
    }

    if( dasectnum >= 0)
        if ( (dasectnum != sprite[spritenum].sectnum) )
            changespritesect(spritenum,dasectnum);
    daz = sprite[spritenum].z + ((zchange*TICSPERFRAME)>>3);
    if ((daz > hittype[spritenum].ceilingz) && (daz <= hittype[spritenum].floorz))
        sprite[spritenum].z = daz;
    else
        if (retval == 0)
            return(16384+dasectnum);

	return(retval);
}


IRAM_ATTR short ssp(short i,uint32_t cliptype) //The set sprite function
{
    spritetype *s;
    int32_t movetype;

    s = &sprite[i];

    movetype = movesprite(i,
        (s->xvel*(sintable[(s->ang+512)&2047]))>>14,
        (s->xvel*(sintable[s->ang&2047]))>>14,s->zvel,
        cliptype);

    return (movetype==0);
}

void insertspriteq(short i)
{
    if(spriteqamount > 0)
    {
        if(spriteq[spriteqloc] >= 0)
            sprite[spriteq[spriteqloc]].xrepeat = 0;
        spriteq[spriteqloc] = i;
        spriteqloc = (spriteqloc+1)%spriteqamount;
    }
    else sprite[i].xrepeat = sprite[i].yrepeat = 0;
}

void lotsofmoney(spritetype *s, short n)
{
    short i ,j;
    for(i=n;i>0;i--)
    {
        j = EGS(s->sectnum,s->x,s->y,s->z-(TRAND%(47<<8)),MONEY,-32,8,8,TRAND&2047,0,0,0,5);
        sprite[j].cstat = TRAND&12;
    }
}

void lotsofmail(spritetype *s, short n)
{
    short i ,j;
    for(i=n;i>0;i--)
    {
        j = EGS(s->sectnum,s->x,s->y,s->z-(TRAND%(47<<8)),MAIL,-32,8,8,TRAND&2047,0,0,0,5);
        sprite[j].cstat = TRAND&12;
    }
}

void lotsofpaper(spritetype *s, short n)
{
    short i ,j;
    for(i=n;i>0;i--)
    {
        j = EGS(s->sectnum,s->x,s->y,s->z-(TRAND%(47<<8)),PAPER,-32,8,8,TRAND&2047,0,0,0,5);
        sprite[j].cstat = TRAND&12;
    }
}


     
void guts(spritetype *s,short gtype, short n, short p)
{
    int32_t gutz,floorz;
    short i,a,j;
    uint8_t  sx,sy;
    int8_t pal;

    if(badguy(s) && s->xrepeat < 16)
        sx = sy = 8;
    else sx = sy = 32;

    gutz = s->z-(8<<8);
    floorz = getflorzofslope(s->sectnum,s->x,s->y);

    if( gutz > ( floorz-(8<<8) ) )
        gutz = floorz-(8<<8);

    if(s->picnum == COMMANDER)
        gutz -= (24<<8);

    if( badguy(s) && s->pal == 6)
        pal = 6;
    else pal = 0;

    for(j=0;j<n;j++)
    {
        a = TRAND&2047;
        i = EGS(s->sectnum,s->x+(TRAND&255)-128,s->y+(TRAND&255)-128,gutz-(TRAND&8191),gtype,-32,sx,sy,a,48+(TRAND&31),-512-(TRAND&2047),ps[p].i,5);
        if(PN == JIBS2)
        {
            sprite[i].xrepeat >>= 2;
            sprite[i].yrepeat >>= 2;
        }
        if(pal == 6)
            sprite[i].pal = 6;
    }
}

void gutsdir(spritetype *s,short gtype, short n, short p)
{
    int32_t gutz,floorz;
    short i,a,j;
    uint8_t  sx,sy;

    if(badguy(s) && s->xrepeat < 16)
        sx = sy = 8;
    else sx = sy = 32;

    gutz = s->z-(8<<8);
    floorz = getflorzofslope(s->sectnum,s->x,s->y);

    if( gutz > ( floorz-(8<<8) ) )
        gutz = floorz-(8<<8);

    if(s->picnum == COMMANDER)
        gutz -= (24<<8);

    for(j=0;j<n;j++)
    {
        a = TRAND&2047;
        i = EGS(s->sectnum,s->x,s->y,gutz,gtype,-32,sx,sy,a,256+(TRAND&127),-512-(TRAND&2047),ps[p].i,5);
    }
}

void setsectinterpolate(short i)
{
    int32_t j, k, startwall,endwall;

    startwall = sector[SECT].wallptr;
    endwall = startwall+sector[SECT].wallnum;

    for(j=startwall;j<endwall;j++)
    {
        setinterpolation(&wall[j].x);
        setinterpolation(&wall[j].y);
        k = wall[j].nextwall;
        if(k >= 0)
        {
            setinterpolation(&wall[k].x);
            setinterpolation(&wall[k].y);
            k = wall[k].point2;
            setinterpolation(&wall[k].x);
            setinterpolation(&wall[k].y);
        }
    }
}

void clearsectinterpolate(short i)
{
    short j,startwall,endwall;

    startwall = sector[SECT].wallptr;
    endwall = startwall+sector[SECT].wallnum;
    for(j=startwall;j<endwall;j++)
    {
        stopinterpolation(&wall[j].x);
        stopinterpolation(&wall[j].y);
        if(wall[j].nextwall >= 0)
        {
            stopinterpolation(&wall[wall[j].nextwall].x);
            stopinterpolation(&wall[wall[j].nextwall].y);
        }
    }
}

void ms(short i)
{
    //T1,T2 and T3 are used for all the sector moving stuff!!!

    short startwall,endwall,x;
    int32_t tx,ty,j,k;
    spritetype *s;

    s = &sprite[i];

    s->x += (s->xvel*(sintable[(s->ang+512)&2047]))>>14;
    s->y += (s->xvel*(sintable[s->ang&2047]))>>14;

    j = T2;
    k = T3;

    startwall = sector[s->sectnum].wallptr;
    endwall = startwall+sector[s->sectnum].wallnum;
    for(x=startwall;x<endwall;x++)
    {
        rotatepoint(
            0,0,
            msx[j],msy[j],
            k&2047,&tx,&ty);

        dragpoint(x,s->x+tx,s->y+ty);

        j++;
    }
}

void movefta(void)
{
    int32_t x, px, py, sx, sy;
    short i, j, p, psect, ssect, nexti;
    spritetype *s;

    i = headspritestat[2];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];
        p = findplayer(s,&x);

        ssect = psect = s->sectnum;

        if(sprite[ps[p].i].extra > 0 )
        {
            if( x < 30000 )
            {
                hittype[i].timetosleep++;
                if( hittype[i].timetosleep >= (x>>8) )
                {
                    if(badguy(s))
                    {
                        px = ps[p].oposx+64-(TRAND&127);
                        py = ps[p].oposy+64-(TRAND&127);
                        updatesector(px,py,&psect);
                        if(psect == -1)
                        {
                            i = nexti;
                            continue;
                        }
                        sx = s->x+64-(TRAND&127);
                        sy = s->y+64-(TRAND&127);
                        updatesector(px,py,&ssect);
                        if(ssect == -1)
                        {
                            i = nexti;
                            continue;
                        }
                        j = cansee(sx,sy,s->z-(TRAND%(52<<8)),s->sectnum,px,py,ps[p].oposz-(TRAND%(32<<8)),ps[p].cursectnum);
                    }
                    else
                        j = cansee(s->x,s->y,s->z-((TRAND&31)<<8),s->sectnum,ps[p].oposx,ps[p].oposy,ps[p].oposz-((TRAND&31)<<8),ps[p].cursectnum);

       //             j = 1;

                    if(j) switch(s->picnum)
                    {
                        case RUBBERCAN:
                        case EXPLODINGBARREL:
                        case WOODENHORSE:
                        case HORSEONSIDE:
                        case CANWITHSOMETHING:
                        case CANWITHSOMETHING2:
                        case CANWITHSOMETHING3:
                        case CANWITHSOMETHING4:
                        case FIREBARREL:
                        case FIREVASE:
                        case NUKEBARREL:
                        case NUKEBARRELDENTED:
                        case NUKEBARRELLEAKED:
                        case TRIPBOMB:
                            if (sector[s->sectnum].ceilingstat&1)
                                s->shade = sector[s->sectnum].ceilingshade;
                            else s->shade = sector[s->sectnum].floorshade;

                            hittype[i].timetosleep = 0;
                            changespritestat(i,6);
                            break;
                        default:
                            hittype[i].timetosleep = 0;
                            check_fta_sounds(i);
                            changespritestat(i,1);
                            break;
                    }
                    else hittype[i].timetosleep = 0;
                }
            }
            if( badguy( s ) )
            {
                if (sector[s->sectnum].ceilingstat&1)
                    s->shade = sector[s->sectnum].ceilingshade;
                else s->shade = sector[s->sectnum].floorshade;
            }
        }
        i = nexti;
    }
}

short ifhitsectors(short sectnum)
{
    short i;

    i = headspritestat[5];
    while(i >= 0)
    {
        if( PN == EXPLOSION2 && sectnum == SECT )
            return i;
        i = nextspritestat[i];
    }
    return -1;
}

short ifhitbyweapon(short sn)
{
    short j, p;
    spritetype *npc;

    if( hittype[sn].extra >= 0 )
    {
        if(sprite[sn].extra >= 0 )
        {
            npc = &sprite[sn];

            if(npc->picnum == APLAYER)
            {
                if(ud.god && hittype[sn].picnum != SHRINKSPARK ) return -1;

                p = npc->yvel;
                j = hittype[sn].owner;

                if( j >= 0 &&
                    sprite[j].picnum == APLAYER &&
                    ud.coop == 1 &&
                    ud.ffire == 0 )
                        return -1;

                npc->extra -= hittype[sn].extra;

                if(j >= 0)
                {
                    if(npc->extra <= 0 && hittype[sn].picnum != FREEZEBLAST)
                    {
                        npc->extra = 0;

                        ps[p].wackedbyactor = j;

                        if( sprite[hittype[sn].owner].picnum == APLAYER && p != sprite[hittype[sn].owner].yvel )
                            ps[p].frag_ps = sprite[j].yvel;

                        hittype[sn].owner = ps[p].i;
                    }
                }

                switch(hittype[sn].picnum)
                {
                    case RADIUSEXPLOSION:
                    case RPG:
                    case HYDRENT:
                    case HEAVYHBOMB:
                    case SEENINE:
                    case OOZFILTER:
                    case EXPLODINGBARREL:
                        ps[p].posxv +=
                            hittype[sn].extra*(sintable[(hittype[sn].ang+512)&2047])<<2;
                        ps[p].posyv +=
                            hittype[sn].extra*(sintable[hittype[sn].ang&2047])<<2;
                        break;
                    default:
                        ps[p].posxv +=
                            hittype[sn].extra*(sintable[(hittype[sn].ang+512)&2047])<<1;
                        ps[p].posyv +=
                            hittype[sn].extra*(sintable[hittype[sn].ang&2047])<<1;
                        break;
                }
            }
            else
            {
                if(hittype[sn].extra == 0 )
                    if( hittype[sn].picnum == SHRINKSPARK && npc->xrepeat < 24 )
                        return -1;

                npc->extra -= hittype[sn].extra;
                if(npc->picnum != RECON && npc->owner >= 0 && sprite[npc->owner].statnum < MAXSTATUS )
                    npc->owner = hittype[sn].owner;
            }

            hittype[sn].extra = -1;
            return hittype[sn].picnum;
        }
    }

    hittype[sn].extra = -1;
    return -1;
}

void movecyclers(void)
{
    short q, j, x, t, s, *c;
    walltype *wal;
    uint8_t  cshade;

    for(q=numcyclers-1;q>=0;q--)
    {

        c = &cyclers[q][0];
        s = c[0];
        
        t = c[3];
        j = t+(sintable[c[1]&2047]>>10);
        cshade = c[2];

        if( j < cshade ) j = cshade;
        else if( j > t )  j = t;

        c[1] += sector[s].extra;
        if(c[5])
        {
            wal = &wall[sector[s].wallptr];
            for(x = sector[s].wallnum;x>0;x--,wal++)
                if( wal->hitag != 1 )
            {
                wal->shade = j;

                if( (wal->cstat&2) && wal->nextwall >= 0)
                    wall[wal->nextwall].shade = j;

            }
            sector[s].floorshade = sector[s].ceilingshade = j;
        }
    }
}

void movedummyplayers(void)
{
    short i, p, nexti;

    i = headspritestat[13];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        p = sprite[OW].yvel;

        if( ps[p].on_crane >= 0 || sector[ps[p].cursectnum].lotag != 1 || sprite[ps[p].i].extra <= 0 )
        {
            ps[p].dummyplayersprite = -1;
            KILLIT(i);
        }
        else
        {
            if(ps[p].on_ground && ps[p].on_warping_sector == 1 && sector[ps[p].cursectnum].lotag == 1 )
            {
                CS = 257;
                SZ = sector[SECT].ceilingz+(27<<8);
                SA = ps[p].ang;
                if(T1 == 8)
                    T1 = 0;
                else T1++;
            }
            else
            {
                if(sector[SECT].lotag != 2) SZ = sector[SECT].floorz;
                CS = (short) 32768;
            }
        }

        SX += (ps[p].posx-ps[p].oposx);
        SY += (ps[p].posy-ps[p].oposy);
        setsprite(i,SX,SY,SZ);

        BOLT:

        i = nexti;
    }
}


short otherp;
void moveplayers(void) //Players
{
    short i , nexti;
    int32_t otherx;
    spritetype *s;
    struct player_struct *p;

    i = headspritestat[10];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];
        p = &ps[s->yvel];
        if(s->owner >= 0)
        {
            if(p->newowner >= 0 ) //Looking thru the camera
            {
                s->x = p->oposx;
                s->y = p->oposy;
                hittype[i].bposz = s->z = p->oposz+PHEIGHT;
                s->ang = p->oang;
                setsprite(i,s->x,s->y,s->z);
            }
            else
            {
                if(ud.multimode > 1)
                    otherp = findotherplayer(s->yvel,&otherx);
                else
                {
                    otherp = s->yvel;
                    otherx = 0;
                }

                execute(i,s->yvel,otherx);

                if(ud.multimode > 1)
                    if( sprite[ps[otherp].i].extra > 0 )
                {
                    if( s->yrepeat > 32 && sprite[ps[otherp].i].yrepeat < 32)
                    {
                        if( otherx < 1400 && p->knee_incs == 0 )
                        {
                            p->knee_incs = 1;
                            p->weapon_pos = -1;
                            p->actorsqu = ps[otherp].i;
                        }
                    }
                }
                if(ud.god)
                {
                    s->extra = max_player_health;
                    s->cstat = 257;
                    p->jetpack_amount =     1599;
                }


                if( s->extra > 0 )
                {
                    hittype[i].owner = i;

                    if(ud.god == 0)
                        if( ceilingspace(s->sectnum) || floorspace(s->sectnum) )
                            quickkill(p);
                }
                else
                {
                    
                    p->posx = s->x;
                    p->posy = s->y;
                    p->posz = s->z-(20<<8);

                    p->newowner = -1;

                    if( p->wackedbyactor >= 0 && sprite[p->wackedbyactor].statnum < MAXSTATUS )
                    {
                        p->ang += getincangle(p->ang,getangle(sprite[p->wackedbyactor].x-p->posx,sprite[p->wackedbyactor].y-p->posy))>>1;
                        p->ang &= 2047;
                    }

                }
                s->ang = p->ang;
            }
        }
        else
        {
            if(p->holoduke_on == -1)
                KILLIT(i);

            hittype[i].bposx = s->x;
            hittype[i].bposy = s->y;
            hittype[i].bposz = s->z;

            s->cstat = 0;

            if(s->xrepeat < 42)
            {
                s->xrepeat += 4;
                s->cstat |= 2;
            }
            else s->xrepeat = 42;
            if(s->yrepeat < 36)
                s->yrepeat += 4;
            else
            {
                s->yrepeat = 36;
                if(sector[s->sectnum].lotag != 2)
                    makeitfall(i);
                if(s->zvel == 0 && sector[s->sectnum].lotag == 1)
                    s->z += (32<<8);
            }

            if(s->extra < 8)
            {
                s->xvel = 128;
                s->ang = p->ang;
                s->extra++;
                
            }
            else
            {
                s->ang = 2047-p->ang;
                setsprite(i,s->x,s->y,s->z);
            }
        }

        if (sector[s->sectnum].ceilingstat&1)
            s->shade += (sector[s->sectnum].ceilingshade-s->shade)>>1;
        else
            s->shade += (sector[s->sectnum].floorshade-s->shade)>>1;

        BOLT:
        i = nexti;
    }
}


void movefx(void)
{
    short i, j, nexti, p;
    int32_t x, ht;
    spritetype *s;

    i = headspritestat[11];
    while(i >= 0)
    {
        s = &sprite[i];

        nexti = nextspritestat[i];

        switch(s->picnum)
        {
            case RESPAWN:
                if(sprite[i].extra == 66)
                {
                    j = spawn(i,SHT);
//                    sprite[j].pal = sprite[i].pal;
                    KILLIT(i);
                }
                else if(sprite[i].extra > (66-13))
                    sprite[i].extra++;
                break;

            case MUSICANDSFX:

                ht = s->hitag;

                if(T2 != SoundToggle)
                {
                    T2 = SoundToggle;
                    T1 = 0;
                }

                if(s->lotag >= 1000 && s->lotag < 2000)
                {
                    x = ldist(&sprite[ps[screenpeek].i],s);
                    if( x < ht && T1 == 0 )
                    {
                        FX_SetReverb( s->lotag - 1000 );
                        T1 = 1;
                    }
                    if( x >= ht && T1 == 1 )
                    {
                        FX_SetReverb(0);
                        FX_SetReverbDelay(0);
                        T1 = 0;
                    }
                }
                else if(s->lotag < 999 && (uint32_t)sector[s->sectnum].lotag < 9 && AmbienceToggle && sector[SECT].floorz != sector[SECT].ceilingz)
                {
                    if( (soundm[s->lotag]&2) )
                    {
                        x = dist(&sprite[ps[screenpeek].i],s);
                        if( x < ht && T1 == 0 && FX_VoiceAvailable(soundpr[s->lotag]-1) )
                        {
                            if(numenvsnds == NumVoices)
                            {
                                j = headspritestat[11];
                                while(j >= 0)
                                {
                                    if( PN == MUSICANDSFX && j != i && sprite[j].lotag < 999 && hittype[j].temp_data[0] == 1 && dist(&sprite[j],&sprite[ps[screenpeek].i]) > x )
                                    {
                                        stopenvsound(sprite[j].lotag,j);
                                        break;
                                    }
                                    j = nextspritestat[j];
                                }
                                if(j == -1) goto BOLT;
                            }
                            spritesound(s->lotag,i);
                            T1 = 1;
                        }
                        if( x >= ht && T1 == 1 )
                        {
                            T1 = 0;
                            stopenvsound(s->lotag,i);
                        }
                    }
                    if( (soundm[s->lotag]&16) )
                    {
                        if(T5 > 0) T5--;
                        else for(p=connecthead;p>=0;p=connectpoint2[p])
                            if( p == myconnectindex && ps[p].cursectnum == s->sectnum )
                        {
                            j = s->lotag+((uint32_t)global_random%(s->hitag+1));
                            sound(j);
                            T5 =  26*40 + (global_random%(26*40));
                        }
                    }
                }
                break;
        }
        BOLT:
        i = nexti;
    }
}



void movefallers(void)
{
    short i, nexti, sect, j;
    spritetype *s;
    int32_t x;

    i = headspritestat[12];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        sect = s->sectnum;

        if( T1 == 0 )
        {
            s->z -= (16<<8);
            T2 = s->ang;
            x = s->extra;
            IFHIT
            {
                if( j == FIREEXT || j == RPG || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER )
                {
                    if(s->extra <= 0)
                    {
                        T1 = 1;
                        j = headspritestat[12];
                        while(j >= 0)
                        {
                            if(sprite[j].hitag == SHT)
                            {
                                hittype[j].temp_data[0] = 1;
                                sprite[j].cstat &= (65535-64);
                                if(sprite[j].picnum == CEILINGSTEAM || sprite[j].picnum == STEAM)
                                    sprite[j].cstat |= 32768;
                            }
                            j = nextspritestat[j];
                        }
                    }
                }
                else
                {
                    hittype[i].extra = 0;
                    s->extra = x;
                }
            }
            s->ang = T2;
            s->z += (16<<8);
        }
        else if(T1 == 1)
        {
            if(s->lotag > 0)
            {
                s->lotag-=3;
                if(s->lotag <= 0)
                {
                    s->xvel = (32+(TRAND&63));
                    s->zvel = -(1024+(TRAND&1023));
                }
            }
            else
            {
                if( s->xvel > 0)
                {
                    s->xvel -= 8;
                    ssp(i,CLIPMASK0);
                }

                if( floorspace(s->sectnum) ) x = 0;
                else
                {
                    if(ceilingspace(s->sectnum))
                        x = gc/6;
                    else
                        x = gc;
                }

                if( s->z < (sector[sect].floorz-FOURSLEIGHT) )
                {
                    s->zvel += x;
                    if(s->zvel > 6144)
                        s->zvel = 6144;
                    s->z += s->zvel;
                }
                if( (sector[sect].floorz-s->z) < (16<<8) )
                {
                    j = 1+(TRAND&7);
                    for(x=0;x<j;x++) RANDOMSCRAP;
                    KILLIT(i);
                }
            }
        }

        BOLT:
        i = nexti;
    }
}

void movestandables(void)
{
    short i, j, k, m, nexti, nextj, p = 0, sect;
    int32_t l=0, x, *t;
    spritetype *s;

    i = headspritestat[6];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        t = &hittype[i].temp_data[0];
        s = &sprite[i];
        sect = s->sectnum;

        if( sect < 0 ) KILLIT(i);

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        IFWITHIN(CRANE,CRANE+3)
        {
            //t[0] = state
            //t[1] = checking sector number

            if(s->xvel) getglobalz(i);

            if( t[0] == 0 ) //Waiting to check the sector
            {
                j = headspritesect[t[1]];
                while(j>=0)
                {
                    nextj = nextspritesect[j];
                    switch( sprite[j].statnum )
                    {
                        case 1:
                        case 2:
                        case 6:
                        case 10:
                            s->ang = getangle(msx[t[4]+1]-s->x,msy[t[4]+1]-s->y);
                            setsprite(j,msx[t[4]+1],msy[t[4]+1],sprite[j].z);
                            t[0]++;
                            goto BOLT;
                    }
                    j = nextj;
                }
            }

            else if(t[0]==1)
            {
                if( s->xvel < 184 )
                {
                    s->picnum = CRANE+1;
                    s->xvel += 8;
                }
                
                if(sect == t[1])
                    t[0]++;
            }
            else if(t[0]==2 || t[0]==7)
            {
                s->z += (1024+512);

                if(t[0]==2)
                {
                    if( (sector[sect].floorz - s->z) < (64<<8) )
                        if(s->picnum > CRANE) s->picnum--;

                    if( (sector[sect].floorz - s->z) < (4096+1024))
                        t[0]++;
                }
                if(t[0]==7)
                {
                    if( (sector[sect].floorz - s->z) < (64<<8) )
                    {
                        if(s->picnum > CRANE) s->picnum--;
                        else
                        {
                            if(s->owner==-2)
                            {
                                spritesound(DUKE_GRUNT,ps[p].i);
                                p = findplayer(s,&x);
                                if(ps[p].on_crane == i)
                                    ps[p].on_crane = -1;
                            }
                            t[0]++;
                            s->owner = -1;
                        }
                    }
                }
            }
            else if(t[0]==3)
            {
                s->picnum++;
                if( s->picnum == (CRANE+2) )
                {
                    p = checkcursectnums(t[1]);
                    if(p >= 0 && ps[p].on_ground)
                    {
                        s->owner = -2;
                        ps[p].on_crane = i;
                        spritesound(DUKE_GRUNT,ps[p].i);
                        ps[p].ang = s->ang+1024;
                    }
                    else
                    {
                        j = headspritesect[t[1]];
                        while(j>=0)
                        {
                            switch( sprite[j].statnum )
                            {
                                case 1:
                                case 6:
                                    s->owner = j;
                                    break;
                            }
                            j = nextspritesect[j];
                        }
                    }

                    t[0]++;//Grabbed the sprite
                    t[2]=0;
                    goto BOLT;
                }
            }
            else if(t[0]==4) //Delay before going up
            {
                t[2]++;
                if(t[2] > 10)
                    t[0]++;
            }
            else if(t[0]==5 || t[0] == 8)
            {
                if(t[0]==8 && s->picnum < (CRANE+2))
                    if( (sector[sect].floorz-s->z) > 8192)
                        s->picnum++;

                if(s->z < msx[t[4]+2])
                {
                    t[0]++;
                    s->xvel = 0;
                }
                else
                    s->z -= (1024+512);
            }
            else if(t[0]==6)
            {
                if( s->xvel < 192 )
                    s->xvel += 8;
                s->ang = getangle(msx[t[4]]-s->x,msy[t[4]]-s->y);
                
                if( ((s->x-msx[t[4]])*(s->x-msx[t[4]])+(s->y-msy[t[4]])*(s->y-msy[t[4]]) ) < (128*128) )
                    t[0]++;
            }

            else if(t[0]==9)
                t[0] = 0;

            setsprite(msy[t[4]+2],s->x,s->y,s->z-(34<<8));

            if(s->owner != -1)
            {
                p = findplayer(s,&x);

                IFHIT
                {
                    if(s->owner == -2)
                        if(ps[p].on_crane == i)
                            ps[p].on_crane = -1;
                    s->owner = -1;
                    s->picnum = CRANE;
                    goto BOLT;
                }

                if(s->owner >= 0)
                {
                    setsprite(s->owner,s->x,s->y,s->z);

                    hittype[s->owner].bposx = s->x;
                    hittype[s->owner].bposy = s->y;
                    hittype[s->owner].bposz = s->z;

                    s->zvel = 0;
                }
                else if(s->owner == -2)
                {
                    ps[p].oposx = ps[p].posx = s->x-(sintable[(ps[p].ang+512)&2047]>>6);
                    ps[p].oposy = ps[p].posy = s->y-(sintable[ps[p].ang&2047]>>6);
                    ps[p].oposz = ps[p].posz = s->z+(2<<8);
                    setsprite(ps[p].i,ps[p].posx,ps[p].posy,ps[p].posz);
                    ps[p].cursectnum = sprite[ps[p].i].sectnum;
                }
            }

            goto BOLT;
        }

        IFWITHIN(WATERFOUNTAIN,WATERFOUNTAIN+3)
        {
            if(t[0] > 0)
            {
                if( t[0] < 20 )
                {
                    t[0]++;

                    s->picnum++;

                    if( s->picnum == ( WATERFOUNTAIN+3 ) )
                        s->picnum = WATERFOUNTAIN+1;
                }
                else
                {
                    p = findplayer(s,&x);

                    if(x > 512)
                    {
                        t[0] = 0;
                        s->picnum = WATERFOUNTAIN;
                    }
                    else t[0] = 1;
                }
            }
            goto BOLT;
        }

        if( AFLAMABLE(s->picnum) )
        {
            if(T1 == 1)
            {
                T2++;
                if( (T2&3) > 0) goto BOLT;

                if( s->picnum == TIRE && T2 == 32 )
                {
                    s->cstat = 0;
                    j = spawn(i,BLOODPOOL);
                    sprite[j].shade = 127;
                }
                else
                {
                    if(s->shade < 64) s->shade++;
                    else KILLIT(i);
                }

                j = s->xrepeat-(TRAND&7);
                if(j < 10)
                {
                    KILLIT(i);
                }

                s->xrepeat = j;

                j = s->yrepeat-(TRAND&7);
                if(j < 4) { KILLIT(i); }
                s->yrepeat = j;
            }
            if(s->picnum == BOX)
            {
                makeitfall(i);
                hittype[i].ceilingz = sector[s->sectnum].ceilingz;
            }
            goto BOLT;
        }

        if(s->picnum == TRIPBOMB)
        {
            if(T3 > 0)
            {
                T3--;
                if(T3 == 8)
                {
                    spritesound(LASERTRIP_EXPLODE,i);
                    for(j=0;j<5;j++) RANDOMSCRAP;
                    x = s->extra;
                    hitradius( i, tripbombblastradius, x>>2,x>>1,x-(x>>2),x);

                    j = spawn(i,EXPLOSION2);
                    sprite[j].ang = s->ang;
                    sprite[j].xvel = 348;
                    ssp(j,CLIPMASK0);

                    j = headspritestat[5];
                    while(j >= 0)
                    {
                        if(sprite[j].picnum == LASERLINE && s->hitag == sprite[j].hitag)
                            sprite[j].xrepeat = sprite[j].yrepeat = 0;
                        j = nextspritestat[j];
                    }
                    KILLIT(i);
                }
                goto BOLT;
            }
            else
            {
                x = s->extra;
                s->extra = 1;
                l = s->ang;
                IFHIT { T3 = 16; }
                s->extra = x;
                s->ang = l;
            }

            if( T1 < 32 )
            {
                p = findplayer(s,&x);
                if( x > 768 ) T1++;
                else if(T1 > 16) T1++;
            }
            if( T1 == 32 )
            {
                l = s->ang;
                s->ang = T6;

                T4 = s->x;T5 = s->y;
                s->x += sintable[(T6+512)&2047]>>9;
                s->y += sintable[(T6)&2047]>>9;
                s->z -= (3<<8);
                setsprite(i,s->x,s->y,s->z);

                x = hitasprite(i,&m);

                hittype[i].lastvx = x;

                s->ang = l;

                k = 0;

                while(x > 0)
                {
                    j = spawn(i,LASERLINE);
                    setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
                    sprite[j].hitag = s->hitag;
                    hittype[j].temp_data[1] = sprite[j].z;

                    s->x += sintable[(T6+512)&2047]>>4;
                    s->y += sintable[(T6)&2047]>>4;

                    if( x < 1024 )
                    {
                        sprite[j].xrepeat = x>>5;
                        break;
                    }
                    x -= 1024;
                }

                T1++;
                s->x = T4;s->y = T5;
                s->z += (3<<8);
                setsprite(i,s->x,s->y,s->z);
                T4 = 0;
                if( m >= 0 )
                {
                    T3 = 13;
                    spritesound(LASERTRIP_ARMING,i);
                }
                else T3 = 0;
            }
            if(T1 == 33)
            {
                T2++;


                T4 = s->x;T5 = s->y;
                s->x += sintable[(T6+512)&2047]>>9;
                s->y += sintable[(T6)&2047]>>9;
                s->z -= (3<<8);
                setsprite(i,s->x,s->y,s->z);

                x = hitasprite(i,&m);

                s->x = T4;s->y = T5;
                s->z += (3<<8);
                setsprite(i,s->x,s->y,s->z);

                if( hittype[i].lastvx != x )
                {
                    T3 = 13;
                    spritesound(LASERTRIP_ARMING,i);
                }
            }
            goto BOLT;
        }


        if( s->picnum >= CRACK1 && s->picnum <= CRACK4 )
        {
            if(s->hitag > 0)
            {
                t[0] = s->cstat;
                t[1] = s->ang;
                j = ifhitbyweapon(i);
                if(j == FIREEXT || j == RPG || j == RADIUSEXPLOSION || j == SEENINE || j == OOZFILTER )
                {
                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(s->hitag == sprite[j].hitag && ( sprite[j].picnum == OOZFILTER || sprite[j].picnum == SEENINE ) )
                            if(sprite[j].shade != -32)
                                sprite[j].shade = -32;
                        j = nextspritestat[j];
                    }

                    goto DETONATE;
                }
                else
                {
                    s->cstat = t[0];
                    s->ang = t[1];
                    s->extra = 0;
                }
            }
            goto BOLT;
        }

        if( s->picnum == FIREEXT )
        {
            j = ifhitbyweapon(i);
            if( j == -1 ) goto BOLT;

            for(k=0;k<16;k++)
            {
                j = EGS(SECT,SX,SY,SZ-(TRAND%(48<<8)),SCRAP3+(TRAND&3),-8,48,48,TRAND&2047,(TRAND&63)+64,-(TRAND&4095)-(sprite[i].zvel>>2),i,5);
                sprite[j].pal = 2;
            }

            spawn(i,EXPLOSION2);
            spritesound(PIPEBOMB_EXPLODE,i);
            spritesound(GLASS_HEAVYBREAK,i);

            if(s->hitag > 0)
            {
                j = headspritestat[6];
                while(j >= 0)
                {
                    if(s->hitag == sprite[j].hitag && ( sprite[j].picnum == OOZFILTER || sprite[j].picnum == SEENINE ) )
                        if(sprite[j].shade != -32)
                            sprite[j].shade = -32;
                    j = nextspritestat[j];
                }

                x = s->extra;
                spawn(i,EXPLOSION2);
                hitradius( i, pipebombblastradius,x>>2, x-(x>>1),x-(x>>2), x);
                spritesound(PIPEBOMB_EXPLODE,i);

                goto DETONATE;
            }
            else
            {
                hitradius(i,seenineblastradius,10,15,20,25);
                KILLIT(i);
            }
            goto BOLT;
        }

        if(s->picnum == OOZFILTER || s->picnum == SEENINE || s->picnum == SEENINEDEAD || s->picnum == (SEENINEDEAD+1) )
        {
            if(s->shade != -32 && s->shade != -33)
            {
                if(s->xrepeat)
                    j = (ifhitbyweapon(i) >= 0);
                else
                    j = 0;

                if( j || s->shade == -31 )
                {
                    if(j) s->lotag = 0;

                    t[3] = 1;

                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(s->hitag == sprite[j].hitag && ( sprite[j].picnum == SEENINE || sprite[j].picnum == OOZFILTER ) )
                            sprite[j].shade = -32;
                        j = nextspritestat[j];
                    }
                }
            }
            else
            {
                if(s->shade == -32)
                {
                    if(s->lotag > 0)
                    {
                        s->lotag-=3;
                        if(s->lotag <= 0) s->lotag = -99;
                    }
                    else
                        s->shade = -33;
                }
                else
                {
                    if( s->xrepeat > 0 )
                    {
                        T3++;
                        if(T3 == 3)
                        {
                            if( s->picnum == OOZFILTER )
                            {
                                T3 = 0;
                                goto DETONATE;
                            }
                            if( s->picnum != (SEENINEDEAD+1) )
                            {
                                T3 = 0;

                                if(s->picnum == SEENINEDEAD) s->picnum++;
                                else if(s->picnum == SEENINE)
                                    s->picnum = SEENINEDEAD;
                            }
                            else goto DETONATE;
                        }
                        goto BOLT;
                    }

                    DETONATE:

                    earthquaketime = 16;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if( s->hitag == sprite[j].hitag )
                        {
                            if(sprite[j].lotag == 13)
                            {
                                if( hittype[j].temp_data[2] == 0 )
                                    hittype[j].temp_data[2] = 1;
                            }
                            else if(sprite[j].lotag == 8)
                                hittype[j].temp_data[4] = 1;
                            else if(sprite[j].lotag == 18)
                            {
                                if(hittype[j].temp_data[0] == 0)
                                    hittype[j].temp_data[0] = 1;
                            }
                            else if(sprite[j].lotag == 21)
                                hittype[j].temp_data[0] = 1;
                        }
                        j = nextspritestat[j];
                    }

                    s->z -= (32<<8);

                    if( ( t[3] == 1 && s->xrepeat ) || s->lotag == -99 )
                    {
                        x = s->extra;
                        spawn(i,EXPLOSION2);
                        hitradius( i,seenineblastradius,x>>2, x-(x>>1),x-(x>>2), x);
                        spritesound(PIPEBOMB_EXPLODE,i);
                    }

                    if(s->xrepeat)
                        for(x=0;x<8;x++) RANDOMSCRAP;

                    KILLIT(i);
                }
            }
            goto BOLT;
        }

        if(s->picnum == MASTERSWITCH)
        {
            if(s->yvel == 1)
                {
                    s->hitag--;
                    if(s->hitag <= 0)
                    {
                        operatesectors(sect,i);

                        j = headspritesect[sect];
                        while(j >= 0)
                        {
                            if(sprite[j].statnum == 3)
                            {
                                switch(sprite[j].lotag)
                                {
                                    case 2:
                                    case 21:
                                    case 31:
                                    case 32:
                                    case 36:
                                        hittype[j].temp_data[0] = 1;
                                        break;
                                    case 3:
                                        hittype[j].temp_data[4] = 1;
                                        break;
                                }
                            }
                            else if(sprite[j].statnum == 6)
                            {
                                switch(sprite[j].picnum)
                                {
                                    case SEENINE:
                                    case OOZFILTER:
                                        sprite[j].shade = -31;
                                        break;
                                }
                            }
                            j = nextspritesect[j];
                        }
                        KILLIT(i);
                    }
                }
                goto BOLT;
        }

        switch(s->picnum)
        {
            case VIEWSCREEN:
            case VIEWSCREEN2:

                if(s->xrepeat == 0) KILLIT(i);

                p = findplayer(s, &x);

                if( x < 2048 )
                {
                    if( SP == 1 )
                        camsprite = i;
                }
                else if( camsprite != -1 && T1 == 1)
                {
                    camsprite = -1;
                    T1 = 0;
                    loadtile(s->picnum);
                }

                goto BOLT;

            case TRASH:

                if(s->xvel == 0) s->xvel = 1;
                IFMOVING
                {
                    makeitfall(i);
                    if(TRAND&1) s->zvel -= 256;
                    if( klabs(s->xvel) < 48 )
                        s->xvel += (TRAND&3);
                }
                else KILLIT(i);
                break;

            case SIDEBOLT1:
            case SIDEBOLT1+1:
            case SIDEBOLT1+2:
            case SIDEBOLT1+3:
                p = findplayer(s, &x);
                if( x > 20480 ) goto BOLT;

                CLEAR_THE_BOLT2:
                if(t[2])
                {
                    t[2]--;
                    goto BOLT;
                }
                if( (s->xrepeat|s->yrepeat) == 0 )
                {
                    s->xrepeat=t[0];
                    s->yrepeat=t[1];
                }
                if( (TRAND&8) == 0 )
                {
                    t[0]=s->xrepeat;
                    t[1]=s->yrepeat;
                    t[2] = global_random&4;
                    s->xrepeat=s->yrepeat=0;
                    goto CLEAR_THE_BOLT2;
                }
                s->picnum++;

                if(l&1) s->cstat ^= 2; // l not defined here. Line is met in 2nd demo with l = 0.

                if( (TRAND&1) && sector[sect].floorpicnum == HURTRAIL )
                    spritesound(SHORT_CIRCUIT,i);

                if(s->picnum == SIDEBOLT1+4) s->picnum = SIDEBOLT1;

                goto BOLT;

            case BOLT1:
            case BOLT1+1:
            case BOLT1+2:
            case BOLT1+3:
                p = findplayer(s, &x);
                if( x > 20480 ) goto BOLT;

                if( t[3] == 0 )
                    t[3]=sector[sect].floorshade;

                CLEAR_THE_BOLT:
                if(t[2])
                {
                    t[2]--;
                    sector[sect].floorshade = 20;
                    sector[sect].ceilingshade = 20;
                    goto BOLT;
                }
                if( (s->xrepeat|s->yrepeat) == 0 )
                {
                    s->xrepeat=t[0];
                    s->yrepeat=t[1];
                }
                else if( (TRAND&8) == 0 )
                {
                    t[0]=s->xrepeat;
                    t[1]=s->yrepeat;
                    t[2] = global_random&4;
                    s->xrepeat=s->yrepeat=0;
                    goto CLEAR_THE_BOLT;
                }
                s->picnum++;

                l = global_random&7;
                s->xrepeat=l+8;

                if(l&1) s->cstat ^= 2;

                if( s->picnum == (BOLT1+1) && (TRAND&7) == 0 && sector[sect].floorpicnum == HURTRAIL )
                    spritesound(SHORT_CIRCUIT,i);

                if(s->picnum==BOLT1+4) s->picnum=BOLT1;

                if(s->picnum&1)
                {
                    sector[sect].floorshade = 0;
                    sector[sect].ceilingshade = 0;
                }
                else
                {
                    sector[sect].floorshade = 20;
                    sector[sect].ceilingshade = 20;
                }
                goto BOLT;
                
            case WATERDRIP:

                if( t[1] )
                {
                    t[1]--;
                    if(t[1] == 0)
                        s->cstat &= 32767;
                }
                else
                {
                    makeitfall(i);
                    ssp(i,CLIPMASK0);
                    if(s->xvel > 0) s->xvel -= 2;

                    if(s->zvel == 0)
                    {
                        s->cstat |= 32768;

                        if(s->pal != 2 && s->hitag == 0)
                            spritesound(SOMETHING_DRIPPING,i);

                        if(sprite[s->owner].picnum != WATERDRIP)
                        {
                            KILLIT(i);
                        }
                        else
                        {
                            hittype[i].bposz = s->z = t[0];
                            t[1] = 48+(TRAND&31);
                        }
                    }
                }


                goto BOLT;

            case DOORSHOCK:
                j = klabs(sector[sect].ceilingz-sector[sect].floorz)>>9;
                s->yrepeat = j+4;
                s->xrepeat = 16;
                s->z = sector[sect].floorz;
                goto BOLT;

            case TOUCHPLATE:
                if( t[1] == 1 && s->hitag >= 0) //Move the sector floor
                {
                    x = sector[sect].floorz;

                    if(t[3] == 1)
                    {
                        if(x >= t[2])
                        {
                            sector[sect].floorz = x;
                            t[1] = 0;
                        }
                        else
                        {
                            sector[sect].floorz += sector[sect].extra;
                            p = checkcursectnums(sect);
                            if(p >= 0) ps[p].posz += sector[sect].extra;
                        }
                    }
                    else
                    {
                        if(x <= s->z)
                        {
                            sector[sect].floorz = s->z;
                            t[1] = 0;
                        }
                        else
                        {
                            sector[sect].floorz -= sector[sect].extra;
                            p = checkcursectnums(sect);
                            if(p >= 0)
                                ps[p].posz -= sector[sect].extra;
                        }
                    }
                    goto BOLT;
                }

                if(t[5] == 1) goto BOLT;

                p = checkcursectnums(sect);
                if( p >= 0 && ( ps[p].on_ground || s->ang == 512) )
                {
                    if( t[0] == 0 && !check_activator_motion(s->lotag) )
                    {
                        t[0] = 1;
                        t[1] = 1;
                        t[3] = !t[3];
                        operatemasterswitches(s->lotag);
                        operateactivators(s->lotag,p);
                        if(s->hitag > 0)
                        {
                            s->hitag--;
                            if(s->hitag == 0) t[5] = 1;
                        }
                    }
                }
                else t[0] = 0;

                if(t[1] == 1)
                {
                    j = headspritestat[6];
                    while(j >= 0)
                    {
                        if(j != i && sprite[j].picnum == TOUCHPLATE && sprite[j].lotag == s->lotag)
                        {
                            hittype[j].temp_data[1] = 1;
                            hittype[j].temp_data[3] = t[3];
                        }
                        j = nextspritestat[j];
                    }
                }
                goto BOLT;

            case CANWITHSOMETHING:
            case CANWITHSOMETHING2:
            case CANWITHSOMETHING3:
            case CANWITHSOMETHING4:
                makeitfall(i);
                IFHIT
                {
                    spritesound(VENT_BUST,i);
                    for(j=0;j<10;j++)
                        RANDOMSCRAP;

                    if(s->lotag) spawn(i,s->lotag);

                    KILLIT(i);
                }
                goto BOLT;

            case EXPLODINGBARREL:
            case WOODENHORSE:
            case HORSEONSIDE:
            case FLOORFLAME:
            case FIREBARREL:
            case FIREVASE:
            case NUKEBARREL:
            case NUKEBARRELDENTED:
            case NUKEBARRELLEAKED:
            case TOILETWATER:
            case RUBBERCAN:
            case STEAM:
            case CEILINGSTEAM:
                p = findplayer(s, &x);
                execute(i,p,x);
                goto BOLT;
            case WATERBUBBLEMAKER:
                p = findplayer(s, &x);
                execute(i,p,x);
                goto BOLT;
        }

        BOLT:
        i = nexti;
    }
}

void bounce(short i)
{
    int32_t k, l, daang, dax, day, daz, xvect, yvect, zvect;
    short hitsect;
    spritetype *s = &sprite[i];

    xvect = mulscale10(s->xvel,sintable[(s->ang+512)&2047]);
    yvect = mulscale10(s->xvel,sintable[s->ang&2047]);
    zvect = s->zvel;

    hitsect = s->sectnum;

    k = sector[hitsect].wallptr; l = wall[k].point2;
    daang = getangle(wall[l].x-wall[k].x,wall[l].y-wall[k].y);

    if ( s->z < (hittype[i].floorz+hittype[i].ceilingz)>>1)
        k = sector[hitsect].ceilingheinum;
    else
        k = sector[hitsect].floorheinum;

    dax = mulscale14(k,sintable[(daang)&2047]);
    day = mulscale14(k,sintable[(daang+1536)&2047]);
    daz = 4096;

    k = xvect*dax+yvect*day+zvect*daz;
    l = dax*dax+day*day+daz*daz;
    if ((klabs(k)>>14) < l)
    {
        k = divscale17(k,l);
        xvect -= mulscale16(dax,k);
        yvect -= mulscale16(day,k);
        zvect -= mulscale16(daz,k);
    }

    s->zvel = zvect;
    s->xvel = ksqrt(dmulscale8(xvect,xvect,yvect,yvect));
    s->ang = getangle(xvect,yvect);
}
     
void moveweapons(void)
{
    short i, j, k, nexti, p, q;
    int32_t dax,day,daz, x, ll;
    uint32_t qq;
    spritetype *s;

    i = headspritestat[4];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        if(s->sectnum < 0) KILLIT(i);

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        switch(s->picnum)
        {
            case RADIUSEXPLOSION:
            case KNEE:
                KILLIT(i);
            case TONGUE:
                T1 = sintable[(T2)&2047]>>9;
                T2 += 32;
                if(T2 > 2047) KILLIT(i);

                if(sprite[s->owner].statnum == MAXSTATUS)
                    if(badguy(&sprite[s->owner]) == 0)
                        KILLIT(i);

                s->ang = sprite[s->owner].ang;
                s->x = sprite[s->owner].x;
                s->y = sprite[s->owner].y;
                if(sprite[s->owner].picnum == APLAYER)
                    s->z = sprite[s->owner].z-(34<<8);
                for(k=0;k<T1;k++)
                {
                    q = EGS(s->sectnum,
                        s->x+((k*sintable[(s->ang+512)&2047])>>9),
                        s->y+((k*sintable[s->ang&2047])>>9),
                        s->z+((k*ksgn(s->zvel))*klabs(s->zvel/12)),TONGUE,-40+(k<<1),
                        8,8,0,0,0,i,5);
                    sprite[q].cstat = 128;
                    sprite[q].pal = 8;
                }
                q = EGS(s->sectnum,
                    s->x+((k*sintable[(s->ang+512)&2047])>>9),
                    s->y+((k*sintable[s->ang&2047])>>9),
                    s->z+((k*ksgn(s->zvel))*klabs(s->zvel/12)),INNERJAW,-40,
                    32,32,0,0,0,i,5);
                sprite[q].cstat = 128;
                if( T2 > 512 && T2 < (1024) )
                    sprite[q].picnum = INNERJAW+1;

                goto BOLT;

            case FREEZEBLAST:
                if(s->yvel < 1 || s->extra < 2 || (s->xvel|s->zvel) == 0)
                {
                    j = spawn(i,TRANSPORTERSTAR);
                    sprite[j].pal = 1;
                    sprite[j].xrepeat = 32;
                    sprite[j].yrepeat = 32;
                    KILLIT(i);
                }
            case SHRINKSPARK:
            case RPG:
            case FIRELASER:
            case SPIT:
            case COOLEXPLOSION1:

                if( s->picnum == COOLEXPLOSION1 )
                    if( Sound[WIERDSHOT_FLY].num == 0 )
                        spritesound(WIERDSHOT_FLY,i);

                p = -1;

                if(s->picnum == RPG && sector[s->sectnum].lotag == 2)
                {
                    k = s->xvel>>1;
                    ll = s->zvel>>1;
                }
                else
                {
                    k = s->xvel;
                    ll = s->zvel;
                }

                dax = s->x; day = s->y; daz = s->z;

                getglobalz(i);
                qq = CLIPMASK1;

                switch(s->picnum)
                {
                    case RPG:
                        if(hittype[i].picnum != BOSS2 && s->xrepeat >= 10 && sector[s->sectnum].lotag != 2)
                        {
                            j = spawn(i,SMALLSMOKE);
                            sprite[j].z += (1<<8);
                        }
                        break;
                }

                j = movesprite(i,
                    (k*(sintable[(s->ang+512)&2047]))>>14,
                    (k*(sintable[s->ang&2047]))>>14,ll,qq);

                if(s->picnum == RPG && s->yvel >= 0)
                    if( FindDistance2D(s->x-sprite[s->yvel].x,s->y-sprite[s->yvel].y) < 256 )
                        j = 49152|s->yvel;

                if(s->sectnum < 0) { KILLIT(i); }

                if( (j&49152) != 49152)
                    if(s->picnum != FREEZEBLAST)
                {
                    if(s->z < hittype[i].ceilingz)
                    {
                        j = 16384|(s->sectnum);
                        s->zvel = -1;
                    }
                    else
                        if( ( s->z > hittype[i].floorz && sector[s->sectnum].lotag != 1 ) ||
                            ( s->z > hittype[i].floorz+(16<<8) && sector[s->sectnum].lotag == 1 ) )
                    {
                        j = 16384|(s->sectnum);
                        if(sector[s->sectnum].lotag != 1)
                            s->zvel = 1;
                    }
                }

                if(s->picnum == FIRELASER)
                {
                    for(k=-3;k<2;k++)
                    {
                        x = EGS(s->sectnum,
                            s->x+((k*sintable[(s->ang+512)&2047])>>9),
                            s->y+((k*sintable[s->ang&2047])>>9),
                            s->z+((k*ksgn(s->zvel))*klabs(s->zvel/24)),FIRELASER,-40+(k<<2),
                            s->xrepeat,s->yrepeat,0,0,0,s->owner,5);

                        sprite[x].cstat = 128;
                        sprite[x].pal = s->pal;
                    }
                }
                else if(s->picnum == SPIT) if(s->zvel < 6144)
                    s->zvel += gc-112;

                if( j != 0 )
                {
                    if(s->picnum == COOLEXPLOSION1)
                    {
                        if( (j&49152) == 49152 && sprite[j&(MAXSPRITES-1)].picnum != APLAYER)
                            goto BOLT;
                        s->xvel = 0;
                        s->zvel = 0;
                    }

                    if( (j&49152) == 49152 )
                    {
                        j &= (MAXSPRITES-1);

                        if(s->picnum == FREEZEBLAST && sprite[j].pal == 1 )
                            if( badguy(&sprite[j]) || sprite[j].picnum == APLAYER )
                        {
                            j = spawn(i,TRANSPORTERSTAR);
                            sprite[j].pal = 1;
                            sprite[j].xrepeat = 32;
                            sprite[j].yrepeat = 32;

                            KILLIT(i);
                        }

                        checkhitsprite(j,i);

                        if(sprite[j].picnum == APLAYER)
                        {
                            p = sprite[j].yvel;
                            spritesound(PISTOL_BODYHIT,j);

                            if(s->picnum == SPIT)
                            {
                                ps[p].horiz += 32;
                                ps[p].return_to_center = 8;

                                if(ps[p].loogcnt == 0)
                                {
                                    if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                        spritesound(DUKE_LONGTERM_PAIN,ps[p].i);

                                    j = 3+(TRAND&3);
                                    ps[p].numloogs = j;
                                    ps[p].loogcnt = 24*4;
                                    for(x=0;x < j;x++)
                                    {
                                        ps[p].loogiex[x] = TRAND%xdim;
                                        ps[p].loogiey[x] = TRAND%ydim;
                                    }
                                }
                            }
                        }
                    }
                    else if( (j&49152) == 32768 )
                    {
                        j &= (MAXWALLS-1);

                        if(s->picnum != RPG && s->picnum != FREEZEBLAST && s->picnum != SPIT && ( wall[j].overpicnum == MIRROR || wall[j].picnum == MIRROR ) )
                        {
                            k = getangle(
                                    wall[wall[j].point2].x-wall[j].x,
                                    wall[wall[j].point2].y-wall[j].y);
                            s->ang = ((k<<1) - s->ang)&2047;
                            s->owner = i;
                            spawn(i,TRANSPORTERSTAR);
                            goto BOLT;
                        }
                        else
                        {
                            setsprite(i,dax,day,daz);
                            checkhitwall(i,j,s->x,s->y,s->z,s->picnum);

                            if(s->picnum == FREEZEBLAST)
                            {
                                if( wall[j].overpicnum != MIRROR && wall[j].picnum != MIRROR )
                                {
                                    s->extra >>= 1;
                                    s->yvel--;
                                }

                                k = getangle(
                                    wall[wall[j].point2].x-wall[j].x,
                                    wall[wall[j].point2].y-wall[j].y);
                                s->ang = ((k<<1) - s->ang)&2047;
                                goto BOLT;
                            }
                        }
                    }
                    else if( (j&49152) == 16384)
                    {
                        setsprite(i,dax,day,daz);

                        if(s->zvel < 0)
                        {
                            if( sector[s->sectnum].ceilingstat&1 )
                                if(sector[s->sectnum].ceilingpal == 0)
                                    KILLIT(i);

                            checkhitceiling(s->sectnum);
                        }

                        if(s->picnum == FREEZEBLAST)
                        {
                            bounce(i);
                            ssp(i,qq);
                            s->extra >>= 1;
                            if(s->xrepeat > 8)
                                s->xrepeat -= 2;
                            if(s->yrepeat > 8)
                                s->yrepeat -= 2;
                            s->yvel--;
                            goto BOLT;
                        }
                    }

                    if(s->picnum != SPIT)
                    {
                        if(s->picnum == RPG)
                        {
                            k = spawn(i,EXPLOSION2);
                            sprite[k].x = dax;
                            sprite[k].y = day;
                            sprite[k].z = daz;

                            if(s->xrepeat < 10)
                            {
                                sprite[k].xrepeat = 6;
                                sprite[k].yrepeat = 6;
                            }
                            else if( (j&49152) == 16384)
                            {
                                if( s->zvel > 0)
                                    spawn(i,EXPLOSION2BOT);
                                else { sprite[k].cstat |= 8; sprite[k].z += (48<<8); }
                            }
                        }
                        else if(s->picnum == SHRINKSPARK)
                        {
                            spawn(i,SHRINKEREXPLOSION);
                            spritesound(SHRINKER_HIT,i);
                            hitradius(i,shrinkerblastradius,0,0,0,0);
                        }
                        else if( s->picnum != COOLEXPLOSION1 && s->picnum != FREEZEBLAST && s->picnum != FIRELASER)
                        {
                            k = spawn(i,EXPLOSION2);
                            sprite[k].xrepeat = sprite[k].yrepeat = s->xrepeat>>1;
                            if( (j&49152) == 16384)
                            {
                                if( s->zvel < 0)
                                    { sprite[k].cstat |= 8; sprite[k].z += (72<<8); }
                            }
                        }
                        if( s->picnum == RPG )
                        {
                            spritesound(RPG_EXPLODE,i);

                            if(s->xrepeat >= 10)
                            {
                                x = s->extra;
                                hitradius( i,rpgblastradius, x>>2,x>>1,x-(x>>2),x);
                            }
                            else
                            {
                                x = s->extra+(global_random&3);
                                hitradius( i,(rpgblastradius>>1),x>>2,x>>1,x-(x>>2),x);
                            }
                        }
                    }
                    if(s->picnum != COOLEXPLOSION1) KILLIT(i);
                }
                if(s->picnum == COOLEXPLOSION1)
                {
                    s->shade++;
                    if(s->shade >= 40) KILLIT(i);
                }
                else if(s->picnum == RPG && sector[s->sectnum].lotag == 2 && s->xrepeat >= 10 && rnd(140))
                    spawn(i,WATERBUBBLE);

                goto BOLT;


            case SHOTSPARK1:
                p = findplayer(s,&x);
                execute(i,p,x);
                goto BOLT;
        }
        BOLT:
        i = nexti;
    }
}


void movetransports(void)
{
    uint8_t  warpspriteto;
    short i, j, k, l, p, sect, sectlotag, nexti, nextj;
    int32_t ll,onfloorz,q;

    i = headspritestat[9]; //Transporters

    while(i >= 0)
    {
        sect = SECT;
        sectlotag = sector[sect].lotag;

        nexti = nextspritestat[i];

        if(OW == i)
        {
            i = nexti;
            continue;
        }

        onfloorz = T5;

        if(T1 > 0) T1--;

        j = headspritesect[sect];
        while(j >= 0)
        {
            nextj = nextspritesect[j];

            switch(sprite[j].statnum)
            {
                case 10:    // Player

                    if( sprite[j].owner != -1 )
                    {
                        p = sprite[j].yvel;

                        ps[p].on_warping_sector = 1;

                        if( ps[p].transporter_hold == 0 && ps[p].jumping_counter == 0 )
                        {
                            if(ps[p].on_ground && sectlotag == 0 && onfloorz && ps[p].jetpack_on == 0 )
                            {
                                if(sprite[i].pal == 0)
                                {
                                    spawn(i,TRANSPORTERBEAM);
                                    spritesound(TELEPORTER,i);
                                }

                                for(k=connecthead;k>=0;k=connectpoint2[k])
                                    if(ps[k].cursectnum == sprite[OW].sectnum)
                                {
                                    ps[k].frag_ps = p;
                                    sprite[ps[k].i].extra = 0;
                                }

                                ps[p].ang = sprite[OW].ang;

                                if(sprite[OW].owner != OW)
                                {
                                    T1 = 13;
                                    hittype[OW].temp_data[0] = 13;
                                    ps[p].transporter_hold = 13;
                                }

                                ps[p].bobposx = ps[p].oposx = ps[p].posx = sprite[OW].x;
                                ps[p].bobposy = ps[p].oposy = ps[p].posy = sprite[OW].y;
                                ps[p].oposz = ps[p].posz = sprite[OW].z-PHEIGHT;

                                changespritesect(j,sprite[OW].sectnum);
                                ps[p].cursectnum = sprite[j].sectnum;

                                if(sprite[i].pal == 0)
                                {
                                    k = spawn(OW,TRANSPORTERBEAM);
                                    spritesound(TELEPORTER,k);
                                }

                                break;
                            }
                        }
                        else if( !(sectlotag == 1 && ps[p].on_ground == 1)  ) break;

                        if(onfloorz == 0 && klabs(SZ-ps[p].posz) < 6144 )
                            if( (ps[p].jetpack_on == 0 ) || (ps[p].jetpack_on && (sync[p].bits&1) ) ||
                                (ps[p].jetpack_on && (sync[p].bits&2) ) )
                        {
                            ps[p].oposx = ps[p].posx += sprite[OW].x-SX;
                            ps[p].oposy = ps[p].posy += sprite[OW].y-SY;

                            if( ps[p].jetpack_on && ( (sync[p].bits&1) || ps[p].jetpack_on < 11 ) )
                                ps[p].posz = sprite[OW].z-6144;
                            else ps[p].posz = sprite[OW].z+6144;
                            ps[p].oposz = ps[p].posz;

                            hittype[ps[p].i].bposx = ps[p].posx;
                            hittype[ps[p].i].bposy = ps[p].posy;
                            hittype[ps[p].i].bposz = ps[p].posz;

                            changespritesect(j,sprite[OW].sectnum);
                            ps[p].cursectnum = sprite[OW].sectnum;

                            break;
                        }

                        k = 0;

                        if( onfloorz && sectlotag == 1 && ps[p].on_ground && ps[p].posz > (sector[sect].floorz-(16<<8)) && ( (sync[p].bits&2) || ps[p].poszv > 2048 ) )
//                        if( onfloorz && sectlotag == 1 && ps[p].posz > (sector[sect].floorz-(6<<8)) )
                        {
                            k = 1;
                            if(screenpeek == p)
                            {
                                FX_StopAllSounds();
                                clearsoundlocks();
                            }
                            if(sprite[ps[p].i].extra > 0)
                                spritesound(DUKE_UNDERWATER,j);
                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].ceilingz+(7<<8);

                            ps[p].posxv = 4096-(TRAND&8192);
                            ps[p].posyv = 4096-(TRAND&8192);

                        }

                        if( onfloorz && sectlotag == 2 && ps[p].posz < (sector[sect].ceilingz+(6<<8)) )
                        {
                            k = 1;
//                            if( sprite[j].extra <= 0) break;
                            if(screenpeek == p)
                            {
                                FX_StopAllSounds();
                                clearsoundlocks();
                            }
                            spritesound(DUKE_GASP,j);

                            ps[p].oposz = ps[p].posz =
                                sector[sprite[OW].sectnum].floorz-(7<<8);

                            ps[p].jumping_toggle = 1;
                            ps[p].jumping_counter = 0;
                        }

                        if(k == 1)
                        {
                            ps[p].oposx = ps[p].posx += sprite[OW].x-SX;
                            ps[p].oposy = ps[p].posy += sprite[OW].y-SY;

                            if(sprite[OW].owner != OW)
                                ps[p].transporter_hold = -2;
                            ps[p].cursectnum = sprite[OW].sectnum;

                            changespritesect(j,sprite[OW].sectnum);
                            setsprite(ps[p].i,ps[p].posx,ps[p].posy,ps[p].posz+PHEIGHT);

                            setpal(&ps[p]);

                            if( (TRAND&255) < 32 )
                                spawn(j,WATERSPLASH2);

                            if(sectlotag == 1)
                                for(l = 0;l < 9;l++)
                            {
                                q = spawn(ps[p].i,WATERBUBBLE);
                                sprite[q].z += TRAND&16383;
                            }
                        }
                    }
                    break;

                case 1:
                    switch(sprite[j].picnum)
                    {
                        case SHARK:
                        case COMMANDER:
                        case OCTABRAIN:
                        case GREENSLIME:
                        case GREENSLIME+1:
                        case GREENSLIME+2:
                        case GREENSLIME+3:
                        case GREENSLIME+4:
                        case GREENSLIME+5:
                        case GREENSLIME+6:
                        case GREENSLIME+7:
                            if(sprite[j].extra > 0)
                                goto JBOLT;
                    }
                case 4:
                case 5:
                case 12:
                case 13:

                    ll = klabs(sprite[j].zvel);

                    {
                        warpspriteto = 0;
                        if( ll && sectlotag == 2 && sprite[j].z < (sector[sect].ceilingz+ll) )
                            warpspriteto = 1;

                        if( ll && sectlotag == 1 && sprite[j].z > (sector[sect].floorz-ll) )
                            warpspriteto = 1;

                        if( sectlotag == 0 && ( onfloorz || klabs(sprite[j].z-SZ) < 4096) )
                        {
                            if( sprite[OW].owner != OW && onfloorz && T1 > 0 && sprite[j].statnum != 5 )
                            {
                                T1++;
                                goto BOLT;
                            }
                            warpspriteto = 1;
                        }

                        if( warpspriteto ) switch(sprite[j].picnum)
                        {
                            case TRANSPORTERSTAR:
                            case TRANSPORTERBEAM:
                            case TRIPBOMB:
                            case BULLETHOLE:
                            case WATERSPLASH2:
                            case BURNING:
                            case BURNING2:
                            case FIRE:
                            case FIRE2:
                            case TOILETWATER:
                            case LASERLINE:
                                goto JBOLT;
                            case PLAYERONWATER:
                                if(sectlotag == 2)
                                {
                                    sprite[j].cstat &= 32767;
                                    break;
                                }
                            default:
                                if(sprite[j].statnum == 5 && !(sectlotag == 1 || sectlotag == 2) )
                                    break;

                            case WATERBUBBLE:
//                                if( rnd(192) && sprite[j].picnum == WATERBUBBLE)
  //                                 break;

                                if(sectlotag > 0)
                                {
                                    k = spawn(j,WATERSPLASH2);
                                    if( sectlotag == 1 && sprite[j].statnum == 4 )
                                    {
                                        sprite[k].xvel = sprite[j].xvel>>1;
                                        sprite[k].ang = sprite[j].ang;
                                        ssp(k,CLIPMASK0);
                                    }
                                }

                                switch(sectlotag)
                                {
                                    case 0:
                                        if(onfloorz)
                                        {
                                            if( sprite[j].statnum == 4 || ( checkcursectnums(sect) == -1 && checkcursectnums(sprite[OW].sectnum)  == -1 ) )
                                            {
                                                sprite[j].x += (sprite[OW].x-SX);
                                                sprite[j].y += (sprite[OW].y-SY);
                                                sprite[j].z -= SZ - sector[sprite[OW].sectnum].floorz;
                                                sprite[j].ang = sprite[OW].ang;

                                                hittype[j].bposx = sprite[j].x;
                                                hittype[j].bposy = sprite[j].y;
                                                hittype[j].bposz = sprite[j].z;

                                                if(sprite[i].pal == 0)
                                                {
                                                    k = spawn(i,TRANSPORTERBEAM);
                                                    spritesound(TELEPORTER,k);

                                                    k = spawn(OW,TRANSPORTERBEAM);
                                                    spritesound(TELEPORTER,k);
                                                }

                                                if( sprite[OW].owner != OW )
                                                {
                                                    T1 = 13;
                                                    hittype[OW].temp_data[0] = 13;
                                                }

                                                changespritesect(j,sprite[OW].sectnum);
                                            }
                                        }
                                        else
                                        {
                                            sprite[j].x += (sprite[OW].x-SX);
                                            sprite[j].y += (sprite[OW].y-SY);
                                            sprite[j].z = sprite[OW].z+4096;

                                            hittype[j].bposx = sprite[j].x;
                                            hittype[j].bposy = sprite[j].y;
                                            hittype[j].bposz = sprite[j].z;

                                            changespritesect(j,sprite[OW].sectnum);
                                        }
                                        break;
                                    case 1:
                                        sprite[j].x += (sprite[OW].x-SX);
                                        sprite[j].y += (sprite[OW].y-SY);
                                        sprite[j].z = sector[sprite[OW].sectnum].ceilingz+ll;

                                        hittype[j].bposx = sprite[j].x;
                                        hittype[j].bposy = sprite[j].y;
                                        hittype[j].bposz = sprite[j].z;

                                        changespritesect(j,sprite[OW].sectnum);

                                        break;
                                    case 2:
                                        sprite[j].x += (sprite[OW].x-SX);
                                        sprite[j].y += (sprite[OW].y-SY);
                                        sprite[j].z = sector[sprite[OW].sectnum].floorz-ll;

                                        hittype[j].bposx = sprite[j].x;
                                        hittype[j].bposy = sprite[j].y;
                                        hittype[j].bposz = sprite[j].z;

                                        changespritesect(j,sprite[OW].sectnum);

                                        break;
                                }

                                break;
                        }
                }
                break;

            }
            JBOLT:
            j = nextj;
        }
        BOLT:
        i = nexti;
    }
}



void moveactors(void)
{
    int32_t x, m, l, *t;
    short a, i, j, nexti, nextj, sect, p;
    spritetype *s;
    uint16_t k;

    i = headspritestat[1];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        s = &sprite[i];

        sect = s->sectnum;

        if( s->xrepeat == 0 || sect < 0 || sect >= MAXSECTORS)
            KILLIT(i);

        t = &hittype[i].temp_data[0];

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        switch(s->picnum)
        {
            case DUCK:
            case TARGET:
                if(s->cstat&32)
                {
                    t[0]++;
                    if(t[0] > 60)
                    {
                        t[0] = 0;
                        s->cstat = 128+257+16;
                        s->extra = 1;
                    }
                }
                else
                {
                    j = ifhitbyweapon(i);
                    if( j >= 0 )
                    {
                        s->cstat = 32+128;
                        k = 1;

                        j = headspritestat[1];
                        while(j >= 0)
                        {
                            if( sprite[j].lotag == s->lotag &&
                                sprite[j].picnum == s->picnum )
                            {
                                if( ( sprite[j].hitag && !(sprite[j].cstat&32) ) ||
                                    ( !sprite[j].hitag && (sprite[j].cstat&32) )
                                  )
                                {
                                    k = 0;
                                    break;
                                }
                            }

                            j = nextspritestat[j];
                        }

                        if(k == 1)
                        {
                            operateactivators(s->lotag,-1);
                            operateforcefields(i,s->lotag);
                            operatemasterswitches(s->lotag);
                        }
                    }
                }
                goto BOLT;

            case RESPAWNMARKERRED:
            case RESPAWNMARKERYELLOW:
            case RESPAWNMARKERGREEN:
                T1++;
                if(T1 > respawnitemtime)
                {
                    KILLIT(i);
                }
                if( T1 >= (respawnitemtime>>1) && T1 < ((respawnitemtime>>1)+(respawnitemtime>>2)) )
                    PN = RESPAWNMARKERYELLOW;
                else if( T1 > ((respawnitemtime>>1)+(respawnitemtime>>2)) )
                    PN = RESPAWNMARKERGREEN;
                makeitfall(i);
                break;

            case HELECOPT:
            case DUKECAR:

                s->z += s->zvel;
                t[0]++;

                if(t[0] == 4) spritesound(WAR_AMBIENCE2,i);

                if( t[0] > (26*8) )
                {
                    sound(RPG_EXPLODE);
                    for(j=0;j<32;j++) RANDOMSCRAP;
                    earthquaketime = 16;
                    KILLIT(i);
                }
                else if((t[0]&3) == 0)
                    spawn(i,EXPLOSION2);
                ssp(i,CLIPMASK0);
                break;
            case RAT:
                makeitfall(i);
                IFMOVING
                {
                    if( (TRAND&255) < 3 ) spritesound(RATTY,i);
                    s->ang += (TRAND&31)-15+(sintable[(t[0]<<8)&2047]>>11);
                }
                else
                {
                    T1++;
                    if(T1 > 1) { KILLIT(i); }
                    else s->ang = (TRAND&2047);
                }
                if(s->xvel < 128)
                    s->xvel+=2;
                s->ang += (TRAND&3)-6;
                break;
            case QUEBALL:
            case STRIPEBALL:
                if(s->xvel)
                {
                    j = headspritestat[0];
                    while(j >= 0)
                    {
                        nextj = nextspritestat[j];
                        if( sprite[j].picnum == POCKET && ldist(&sprite[j],s) < 52 ) KILLIT(i);
                        j = nextj;
                    }

                    j = clipmove(&s->x,&s->y,&s->z,&s->sectnum,
                        (((s->xvel*(sintable[(s->ang+512)&2047]))>>14)*TICSPERFRAME)<<11,
                        (((s->xvel*(sintable[s->ang&2047]))>>14)*TICSPERFRAME)<<11,
                        24L,(4<<8),(4<<8),CLIPMASK1);

                    if(j&49152)
                    {
                        if( (j&49152) == 32768 )
                        {
                            j &= (MAXWALLS-1);
                            k = getangle(
                                wall[wall[j].point2].x-wall[j].x,
                                wall[wall[j].point2].y-wall[j].y);
                            s->ang = ((k<<1) - s->ang)&2047;
                        }
                        else if( (j&49152) == 49152 )
                        {
                            j &= (MAXSPRITES-1);
                            checkhitsprite(i,j);
                        }
                    }
                    s->xvel --;
                    if(s->xvel < 0) s->xvel = 0;
                    if( s->picnum == STRIPEBALL )
                    {
                        s->cstat = 257;
                        s->cstat |= 4&s->xvel;
                        s->cstat |= 8&s->xvel;
                    }
                }
                else
                {
                    p = findplayer(s,&x);

                    if( x < 1596)
                    {

//                        if(s->pal == 12)
                        {
                            j = getincangle(ps[p].ang,getangle(s->x-ps[p].posx,s->y-ps[p].posy));
                            if( j > -64 && j < 64 && (sync[p].bits&(1<<29)) )
                                if(ps[p].toggle_key_flag == 1)
                            {
                                a = headspritestat[1];
                                while(a >= 0)
                                {
                                    if(sprite[a].picnum == QUEBALL || sprite[a].picnum == STRIPEBALL)
                                    {
                                        j = getincangle(ps[p].ang,getangle(sprite[a].x-ps[p].posx,sprite[a].y-ps[p].posy));
                                        if( j > -64 && j < 64 )
                                        {
                                            findplayer(&sprite[a],&l);
                                            if(x > l) break;
                                        }
                                    }
                                    a = nextspritestat[a];
                                }
                                if(a == -1)
                                {
                                    if(s->pal == 12)
                                        s->xvel = 164;
                                    else s->xvel = 140;
                                    s->ang = ps[p].ang;
                                    ps[p].toggle_key_flag = 2;
                                }
                            }
                        }
                    }
                    if( x < 512 && s->sectnum == ps[p].cursectnum )
                    {
                        s->ang = getangle(s->x-ps[p].posx,s->y-ps[p].posy);
                        s->xvel = 48;
                    }
                }

                break;
            case FORCESPHERE:

                if(s->yvel == 0)
                {
                    s->yvel = 1;

                    for(l=512;l<(2048-512);l+= 128)
                        for(j=0;j<2048;j += 128)
                    {
                        k = spawn(i,FORCESPHERE);
                        sprite[k].cstat = 257+128;
                        sprite[k].clipdist = 64;
                        sprite[k].ang = j;
                        sprite[k].zvel = sintable[l&2047]>>5;
                        sprite[k].xvel = sintable[(l+512)&2047]>>9;
                        sprite[k].owner = i;
                    }
                }

                if(t[3] > 0)
                {
                    if(s->zvel < 6144)
                        s->zvel += 192;
                    s->z += s->zvel;
                    if(s->z > sector[sect].floorz)
                        s->z = sector[sect].floorz;
                    t[3]--;
                    if(t[3] == 0)
                        KILLIT(i);
                }
                else if(t[2] > 10)
                {
                    j = headspritestat[5];
                    while(j >= 0)
                    {
                        if(sprite[j].owner == i && sprite[j].picnum == FORCESPHERE)
                            hittype[j].temp_data[1] = 1+(TRAND&63);
                        j = nextspritestat[j];
                    }
                    t[3] = 64;
                }

                goto BOLT;

            case RECON:

                getglobalz(i);

                if (sector[s->sectnum].ceilingstat&1)
                   s->shade += (sector[s->sectnum].ceilingshade-s->shade)>>1;
                else s->shade += (sector[s->sectnum].floorshade-s->shade)>>1;

                if( s->z < sector[sect].ceilingz+(32<<8) )
                    s->z = sector[sect].ceilingz+(32<<8);

                if( ud.multimode < 2 )
                {
                    if( actor_tog == 1)
                    {
                        s->cstat = (short)32768;
                        goto BOLT;
                    }
                    else if(actor_tog == 2) s->cstat = 257;
                }
                IFHIT
                {
                    if( s->extra < 0 && t[0] != -1 )
                    {
                        t[0] = -1;
                        s->extra = 0;
                    }
                    spritesound(RECO_PAIN,i);
                    RANDOMSCRAP;
                }

                if(t[0] == -1)
                {
                    s->z += 1024;
                    t[2]++;
                    if( (t[2]&3) == 0) spawn(i,EXPLOSION2);
                    getglobalz(i);
                    s->ang += 96;
                    s->xvel = 128;
                    j = ssp(i,CLIPMASK0);
                    if(j != 1 || s->z > hittype[i].floorz)
                    {
                        for(l=0;l<16;l++)
                            RANDOMSCRAP;
                        spritesound(LASERTRIP_EXPLODE,i);
                        spawn(i,PIGCOP);
                        ps[myconnectindex].actors_killed++;
                        KILLIT(i);
                    }
                    goto BOLT;
                }
                else
                {
                    if( s->z > hittype[i].floorz-(48<<8) )
                        s->z = hittype[i].floorz-(48<<8);
                }

                p = findplayer(s,&x);
                j = s->owner;

                // 3 = findplayerz, 4 = shoot

                if( t[0] >= 4 )
                {
                    t[2]++;
                    if( (t[2]&15) == 0 )
                    {
                        a = s->ang;
                        s->ang = hittype[i].tempang;
                        spritesound(RECO_ATTACK,i);
                        shoot(i,FIRELASER);
                        s->ang = a;
                    }
                    if( t[2] > (26*3) || !cansee(s->x,s->y,s->z-(16<<8),s->sectnum, ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum ) )
                    {
                        t[0] = 0;
                        t[2] = 0;
                    }
                    else hittype[i].tempang +=
                        getincangle(hittype[i].tempang,getangle(ps[p].posx-s->x,ps[p].posy-s->y))/3;
                }
                else if(t[0] == 2 || t[0] == 3)
                {
                    t[3] = 0;
                    if(s->xvel > 0) s->xvel -= 16;
                    else s->xvel = 0;

                    if(t[0] == 2)
                    {
                        l = ps[p].posz-s->z;
                        if( klabs(l) < (48<<8) ) t[0] = 3;
                        else s->z += sgn(ps[p].posz-s->z)<<10;
                    }
                    else
                    {
                        t[2]++;
                        if( t[2] > (26*3) || !cansee(s->x,s->y,s->z-(16<<8),s->sectnum, ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum ) )
                        {
                            t[0] = 1;
                            t[2] = 0;
                        }
                        else if( (t[2]&15) == 0 )
                        {
                            spritesound(RECO_ATTACK,i);
                            shoot(i,FIRELASER);
                        }
                    }
                    s->ang += getincangle(s->ang,getangle(ps[p].posx-s->x,ps[p].posy-s->y))>>2;
                }

                if( t[0] != 2 && t[0] != 3 )
                {
                    l = ldist(&sprite[j],s);
                    if(l <= 1524)
                    {
                        a = s->ang;
                        s->xvel >>= 1;
                    }
                    else a = getangle(sprite[j].x-s->x,sprite[j].y-s->y);

                    if(t[0] == 1 || t[0] == 4) // Found a locator and going with it
                    {
                        l = dist(&sprite[j],s);

                        if( l <= 1524 ) { if(t[0] == 1) t[0] = 0; else t[0] = 5; }
                        else
                        {
                            // Control speed here
                            if(l > 1524) { if( s->xvel < 256 ) s->xvel += 32; }
                            else
                            {
                                if(s->xvel > 0) s->xvel -= 16;
                                else s->xvel = 0;
                            }
                        }

                        if(t[0] < 2) t[2]++;

                        if( x < 6144 && t[0] < 2 && t[2] > (26*4) )
                        {
                            t[0] = 2+(TRAND&2);
                            t[2] = 0;
                            hittype[i].tempang = s->ang;
                        }
                    }

                    if(t[0] == 0 || t[0] == 5)
                    {
                        if(t[0] == 0)
                            t[0] = 1;
                        else t[0] = 4;
                        j = s->owner = LocateTheLocator(s->hitag,-1);
                        if(j == -1)
                        {
                            s->hitag = j = hittype[i].temp_data[5];
                            s->owner = LocateTheLocator(j,-1);
                            j = s->owner;
                            if(j == -1) KILLIT(i);
                        }
                        else s->hitag++;
                    }

                    t[3] = getincangle(s->ang,a);
                    s->ang += t[3]>>3;

                    if(s->z < sprite[j].z)
                        s->z += 1024;
                    else s->z -= 1024;
                }

                if(Sound[RECO_ROAM].num == 0 )
                    spritesound(RECO_ROAM,i);

                ssp(i,CLIPMASK0);

                goto BOLT;

            case OOZ:
            case OOZ2:

                getglobalz(i);

                j = (hittype[i].floorz-hittype[i].ceilingz)>>9;
                if(j > 255) j = 255;

                x = 25-(j>>1);
                if(x < 8) x = 8;
                else if(x > 48) x = 48;

                s->yrepeat = j;
                s->xrepeat = x;
                s->z = hittype[i].floorz;

                goto BOLT;

            case GREENSLIME:
            case GREENSLIME+1:
            case GREENSLIME+2:
            case GREENSLIME+3:
            case GREENSLIME+4:
            case GREENSLIME+5:
            case GREENSLIME+6:
            case GREENSLIME+7:

// #ifndef VOLUMEONE
                if( ud.multimode < 2 )
                {
                    if( actor_tog == 1)
                    {
                        s->cstat = (short)32768;
                        goto BOLT;
                    }
                    else if(actor_tog == 2) s->cstat = 257;
                }
// #endif

                t[1]+=128;

                if(sector[sect].floorstat&1)
                    KILLIT(i);

                p = findplayer(s,&x);

                if(x > 20480)
                {
                    hittype[i].timetosleep++;
                    if( hittype[i].timetosleep > SLEEPTIME )
                    {
                        hittype[i].timetosleep = 0;
                        changespritestat(i,2);
                        goto BOLT;
                    }
                }

                if(t[0] == -5) // FROZEN
                {
                    t[3]++;
                    if(t[3] > 280)
                    {
                        s->pal = 0;
                        t[0] = 0;
                        goto BOLT;
                    }
                    makeitfall(i);
                    s->cstat = 257;
                    s->picnum = GREENSLIME+2;
                    s->extra = 1;
                    s->pal = 1;
                    IFHIT
                    {
                        if(j == FREEZEBLAST) goto BOLT;
                        for(j=16; j >= 0 ;j--)
                        {
                            k = EGS(SECT,SX,SY,SZ,GLASSPIECES+(j%3),-32,36,36,TRAND&2047,32+(TRAND&63),1024-(TRAND&1023),i,5);
                            sprite[k].pal = 1;
                        }
                        spritesound(GLASS_BREAKING,i);
                        KILLIT(i);
                    }
                    else if(x < 1024 && ps[p].quick_kick == 0)
                    {
                        j = getincangle(ps[p].ang,getangle(SX-ps[p].posx,SY-ps[p].posy));
                        if( j > -128 && j < 128 )
                            ps[p].quick_kick = 14;
                    }

                    goto BOLT;
                }

                if(x < 1596)
                    s->cstat = 0;
                else s->cstat = 257;

                if(t[0] == -4) //On the player
                {
                    if( sprite[ps[p].i].extra < 1 )
                    {
                        t[0] = 0;
                        goto BOLT;
                    }

                    setsprite(i,s->x,s->y,s->z);

                    s->ang = ps[p].ang;

                    if( ( (sync[p].bits&4) || (ps[p].quick_kick > 0) ) && sprite[ps[p].i].extra > 0 )
                        if( ps[p].quick_kick > 0 || ( ps[p].curr_weapon != HANDREMOTE_WEAPON && ps[p].curr_weapon != HANDBOMB_WEAPON && ps[p].curr_weapon != TRIPBOMB_WEAPON && ps[p].ammo_amount[ps[p].curr_weapon] >= 0) )
                    {
                        for(x=0;x<8;x++)
                        {
                            j = EGS(sect,s->x,s->y,s->z-(8<<8),SCRAP3+(TRAND&3),-8,48,48,TRAND&2047,(TRAND&63)+64,-(TRAND&4095)-(s->zvel>>2),i,5);
                            sprite[j].pal = 6;
                        }

                        spritesound(SLIM_DYING,i);
                        spritesound(SQUISHED,i);
                        if( (TRAND&255) < 32 )
                        {
                            j = spawn(i,BLOODPOOL);
                            sprite[j].pal = 0;
                        }
                        ps[p].actors_killed ++;
                        t[0] = -3;
                        if(ps[p].somethingonplayer == i)
                            ps[p].somethingonplayer = -1;
                        KILLIT(i);
                    }

                    s->z = ps[p].posz+ps[p].pyoff-t[2]+(8<<8);

                    s->z += (100-ps[p].horiz)<<4;

                    if( t[2] > 512)
                        t[2] -= 128;

                    if( t[2] < 348)
                        t[2] += 128;

                    if(ps[p].newowner >= 0)
                    {
                        ps[p].newowner = -1;
                        ps[p].posx = ps[p].oposx;
                        ps[p].posy = ps[p].oposy;
                        ps[p].posz = ps[p].oposz;
                        ps[p].ang = ps[p].oang;

                        updatesector(ps[p].posx,ps[p].posy,&ps[p].cursectnum);
                        setpal(&ps[p]);

                        j = headspritestat[1];
                        while(j >= 0)
                        {
                            if(sprite[j].picnum==CAMERA1) sprite[j].yvel = 0;
                            j = nextspritestat[j];
                        }
                    }

                    if(t[3]>0)
                    {
                        short frames[] = {5,5,6,6,7,7,6,5};

                        s->picnum = GREENSLIME+frames[t[3]];

                        if( t[3] == 5 )
                        {
                            sprite[ps[p].i].extra += -(5+(TRAND&3));
                            spritesound(SLIM_ATTACK,i);
                        }

                        if(t[3] < 7) t[3]++;
                        else t[3] = 0;

                    }
                    else
                    {
                        s->picnum = GREENSLIME+5;
                        if(rnd(32))
                            t[3] = 1;
                    }

                    s->xrepeat = 20+(sintable[t[1]&2047]>>13);
                    s->yrepeat = 15+(sintable[t[1]&2047]>>13);

                    s->x = ps[p].posx + (sintable[(ps[p].ang+512)&2047]>>7);
                    s->y = ps[p].posy + (sintable[ps[p].ang&2047]>>7);

                    goto BOLT;
                }

                else if(s->xvel < 64 && x < 768)
                {
                    if(ps[p].somethingonplayer == -1)
                    {
                        ps[p].somethingonplayer = i;
                        if(t[0] == 3 || t[0] == 2) //Falling downward
                            t[2] = (12<<8);
                        else t[2] = -(13<<8); //Climbing up duke
                        t[0] = -4;
                    }
                }

                    IFHIT
                    {
                        spritesound(SLIM_DYING,i);

                        ps[p].actors_killed ++;
                        if(ps[p].somethingonplayer == i)
                            ps[p].somethingonplayer = -1;

                        if(j == FREEZEBLAST)
                        {
                            spritesound(SOMETHINGFROZE,i); t[0] = -5 ; t[3] = 0 ;
                            goto BOLT;
                        }

                        if( (TRAND&255) < 32 )
                        {
                            j = spawn(i,BLOODPOOL);
                            sprite[j].pal = 0;
                        }

                        for(x=0;x<8;x++)
                        {
                            j = EGS(sect,s->x,s->y,s->z-(8<<8),SCRAP3+(TRAND&3),-8,48,48,TRAND&2047,(TRAND&63)+64,-(TRAND&4095)-(s->zvel>>2),i,5);
                            sprite[j].pal = 6;
                        }
                        t[0] = -3;
                        KILLIT(i);
                    }
                        // All weap
                if(t[0] == -1) //Shrinking down
                {
                    makeitfall(i);

                    s->cstat &= 65535-8;
                    s->picnum = GREENSLIME+4;

//                    if(s->yrepeat > 62)
  //                      guts(s,JIBS6,5,myconnectindex);

                    if(s->xrepeat > 32) s->xrepeat -= TRAND&7;
                    if(s->yrepeat > 16) s->yrepeat -= TRAND&7;
                    else
                    {
                        s->xrepeat = 40;
                        s->yrepeat = 16;
                        t[5] = -1;
                        t[0] = 0;
                    }

                    goto BOLT;
                }
                else if(t[0] != -2) getglobalz(i);

                if(t[0] == -2) //On top of somebody
                {
                    makeitfall(i);
                    sprite[t[5]].xvel = 0;

                    l = sprite[t[5]].ang;

                    s->z = sprite[t[5]].z;
                    s->x = sprite[t[5]].x+(sintable[(l+512)&2047]>>11);
                    s->y = sprite[t[5]].y+(sintable[l&2047]>>11);

                    s->picnum =  GREENSLIME+2+(global_random&1);

                    if(s->yrepeat < 64) s->yrepeat+=2;
                    else
                    {
                        if(s->xrepeat < 32) s->xrepeat += 4;
                        else
                        {
                            t[0] = -1;
                            x = ldist(s,&sprite[t[5]]);
                            if(x < 768) sprite[t[5]].xrepeat = 0;
                        }
                    }

                    goto BOLT;
                }

                //Check randomly to see of there is an actor near
                if(rnd(32))
                {
                    j = headspritesect[sect];
                    while(j>=0)
                    {
                        switch(sprite[j].picnum)
                        {
                            case LIZTROOP:
                            case LIZMAN:
                            case PIGCOP:
                            case NEWBEAST:
                                if( ldist(s,&sprite[j]) < 768 && (klabs(s->z-sprite[j].z)<8192) ) //Gulp them
                                {
                                    t[5] = j;
                                    t[0] = -2;
                                    t[1] = 0;
                                    goto BOLT;
                                }
                        }

                        j = nextspritesect[j];
                    }
                }

                //Moving on the ground or ceiling

                if(t[0] == 0 || t[0] == 2)
                {
                    s->picnum = GREENSLIME;

                    if( (TRAND&511) == 0 )
                        spritesound(SLIM_ROAM,i);

                    if(t[0]==2)
                    {
                        s->zvel = 0;
                        s->cstat &= (65535-8);

                        if( (sector[sect].ceilingstat&1) || (hittype[i].ceilingz+6144) < s->z)
                        {
                            s->z += 2048;
                            t[0] = 3;
                            goto BOLT;
                        }
                    }
                    else
                    {
                        s->cstat |= 8;
                        makeitfall(i);
                    }

                    if( everyothertime&1 ) ssp(i,CLIPMASK0);

                    if(s->xvel > 96)
                    {
                        s->xvel -= 2;
                        goto BOLT;
                    }
                    else
                    {
                        if(s->xvel < 32) s->xvel += 4;
                        s->xvel = 64 - (sintable[(t[1]+512)&2047]>>9);

                        s->ang += getincangle(s->ang,
                               getangle(ps[p].posx-s->x,ps[p].posy-s->y))>>3;
// TJR
                    }

                    s->xrepeat = 36 + (sintable[(t[1]+512)&2047]>>11);
                    s->yrepeat = 16 + (sintable[t[1]&2047]>>13);

                    if(rnd(4) && (sector[sect].ceilingstat&1) == 0 &&
                        klabs(hittype[i].floorz-hittype[i].ceilingz)
                            < (192<<8) )
                            {
                                s->zvel = 0;
                                t[0]++;
                            }

                }

                if(t[0]==1)
                {
                    s->picnum = GREENSLIME;
                    if(s->yrepeat < 40) s->yrepeat+=8;
                    if(s->xrepeat > 8) s->xrepeat-=4;
                    if(s->zvel > -(2048+1024))
                        s->zvel -= 348;
                    s->z += s->zvel;
                    if(s->z < hittype[i].ceilingz+4096)
                    {
                        s->z = hittype[i].ceilingz+4096;
                        s->xvel = 0;
                        t[0] = 2;
                    }
                }

                if(t[0]==3)
                {
                    s->picnum = GREENSLIME+1;

                    makeitfall(i);

                    if(s->z > hittype[i].floorz-(8<<8))
                    {
                        s->yrepeat-=4;
                        s->xrepeat+=2;
                    }
                    else
                    {
                        if(s->yrepeat < (40-4)) s->yrepeat+=8;
                        if(s->xrepeat > 8) s->xrepeat-=4;
                    }

                    if(s->z > hittype[i].floorz-2048)
                    {
                        s->z = hittype[i].floorz-2048;
                        t[0] = 0;
                        s->xvel = 0;
                    }
                }
                goto BOLT;

            case BOUNCEMINE:
            case MORTER:
                j = spawn(i,FRAMEEFFECT1);
                hittype[j].temp_data[0] = 3;

            case HEAVYHBOMB:

                if( (s->cstat&32768) )
                {
                    t[2]--;
                    if(t[2] <= 0)
                    {
                        spritesound(TELEPORTER,i);
                        spawn(i,TRANSPORTERSTAR);
                        s->cstat = 257;
                    }
                    goto BOLT;
                }

                p = findplayer(s,&x);

                if( x < 1220 ) s->cstat &= ~257;
                else s->cstat |= 257;

                if(t[3] == 0 )
                {
                    j = ifhitbyweapon(i);
                    if(j >= 0)
                    {
                        t[3] = 1;
                        t[4] = 0;
                        l = 0;
                        s->xvel = 0;
                        goto DETONATEB;
                    }
                }

                if( s->picnum != BOUNCEMINE )
                {
                    makeitfall(i);

                    if( sector[sect].lotag != 1 && s->z >= hittype[i].floorz-(FOURSLEIGHT) && s->yvel < 3 )
                    {
                        if( s->yvel > 0 || (s->yvel == 0 && hittype[i].floorz == sector[sect].floorz ))
                            spritesound(PIPEBOMB_BOUNCE,i);
                        s->zvel = -((4-s->yvel)<<8);
                        if(sector[s->sectnum].lotag== 2)
                            s->zvel >>= 2;
                        s->yvel++;
                    }
                    if( s->z < hittype[i].ceilingz ) // && sector[sect].lotag != 2 )
                    {
                        s->z = hittype[i].ceilingz+(3<<8);
                        s->zvel = 0;
                    }
                }

                j = movesprite(i,
                    (s->xvel*(sintable[(s->ang+512)&2047]))>>14,
                    (s->xvel*(sintable[s->ang&2047]))>>14,
                    s->zvel,CLIPMASK0);

                if(sector[SECT].lotag == 1 && s->zvel == 0)
                {
                    s->z += (32<<8);
                    if(t[5] == 0)
                    {
                        t[5] = 1;
                        spawn(i,WATERSPLASH2);
                    }
                }
                else t[5] = 0;

                if(t[3] == 0 && ( s->picnum == BOUNCEMINE || s->picnum == MORTER ) && (j || x < 844) )
                {
                    t[3] = 1;
                    t[4] = 0;
                    l = 0;
                    s->xvel = 0;
                    goto DETONATEB;
                }

                if(sprite[s->owner].picnum == APLAYER)
                    l = sprite[s->owner].yvel;
                else l = -1;

                if(s->xvel > 0)
                {
                    s->xvel -= 5;
                    if(sector[sect].lotag == 2)
                        s->xvel -= 10;

                    if(s->xvel < 0)
                        s->xvel = 0;
                    if(s->xvel&8) s->cstat ^= 4;
                }

                if( (j&49152) == 32768 )
                {
                    j &= (MAXWALLS-1);

                    checkhitwall(i,j,s->x,s->y,s->z,s->picnum);

                    k = getangle(
                        wall[wall[j].point2].x-wall[j].x,
                        wall[wall[j].point2].y-wall[j].y);

                    s->ang = ((k<<1) - s->ang)&2047;
                    s->xvel >>= 1;
                }

                DETONATEB:

                if( ( l >= 0 && ps[l].hbomb_on == 0 ) || t[3] == 1)
                {
                    t[4]++;

                    if(t[4] == 2)
                    {
                        x = s->extra;
                        m = 0;
                        switch(s->picnum)
                        {
                            case HEAVYHBOMB: m = pipebombblastradius;break;
                            case MORTER: m = morterblastradius;break;
                            case BOUNCEMINE: m = bouncemineblastradius;break;
                        }

                        hitradius( i, m,x>>2,x>>1,x-(x>>2),x);
                        spawn(i,EXPLOSION2);
                        if( s->zvel == 0 )
                            spawn(i,EXPLOSION2BOT);
                        spritesound(PIPEBOMB_EXPLODE,i);
                        for(x=0;x<8;x++)
                            RANDOMSCRAP;
                    }

                    if(s->yrepeat)
                    {
                        s->yrepeat = 0;
                        goto BOLT;
                    }

                    if(t[4] > 20)
                    {
                        if(s->owner != i || ud.respawn_items == 0)
                        {
                            KILLIT(i);
                        }
                        else
                        {
                            t[2] = respawnitemtime;
                            spawn(i,RESPAWNMARKERRED);
                            s->cstat = (short) 32768;
                            s->yrepeat = 9;
                            goto BOLT;
                        }
                    }
                }
                else if(s->picnum == HEAVYHBOMB && x < 788 && t[0] > 7 && s->xvel == 0)
                    if( cansee(s->x,s->y,s->z-(8<<8),s->sectnum,ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum) )
                        if(ps[p].ammo_amount[HANDBOMB_WEAPON] < max_ammo_amount[HANDBOMB_WEAPON] )
                {
                    if(ud.coop >= 1 && s->owner == i)
                    {
                        for(j=0;j<ps[p].weapreccnt;j++)
                            if(ps[p].weaprecs[j] == s->picnum)
                                goto BOLT;

                        if(ps[p].weapreccnt < 255)
                            ps[p].weaprecs[ps[p].weapreccnt++] = s->picnum;
                    }

                    addammo(HANDBOMB_WEAPON,&ps[p],1);
                    spritesound(DUKE_GET,ps[p].i);

                    if( ps[p].gotweapon[HANDBOMB_WEAPON] == 0 || s->owner == ps[p].i )
                        addweapon(&ps[p],HANDBOMB_WEAPON);

                    if( sprite[s->owner].picnum != APLAYER )
                    {
                        ps[p].pals[0] = 0;
                        ps[p].pals[1] = 32;
                        ps[p].pals[2] = 0;
                        ps[p].pals_time = 32;
                    }

                    if( s->owner != i || ud.respawn_items == 0 )
                    {
                        if(s->owner == i && ud.coop >= 1)
                            goto BOLT;
                        KILLIT(i);
                    }
                    else
                    {
                        t[2] = respawnitemtime;
                        spawn(i,RESPAWNMARKERRED);
                        s->cstat = (short) 32768;
                    }
                }

                if(t[0] < 8) t[0]++;
                goto BOLT;

            case REACTORBURNT:
            case REACTOR2BURNT:
                goto BOLT;

            case REACTOR:
            case REACTOR2:

                if( t[4] == 1 )
                {
                    j = headspritesect[sect];
                    while(j >= 0)
                    {
                        switch(sprite[j].picnum)
                        {
                            case SECTOREFFECTOR:
                                if(sprite[j].lotag == 1)
                                {
                                    sprite[j].lotag = (short) 65535;
                                    sprite[j].hitag = (short) 65535;
                                }
                                break;
                            case REACTOR:
                                sprite[j].picnum = REACTORBURNT;
                                break;
                            case REACTOR2:
                                sprite[j].picnum = REACTOR2BURNT;
                                break;
                            case REACTORSPARK:
                            case REACTOR2SPARK:
                                sprite[j].cstat = (short) 32768;
                                break;
                        }
                        j = nextspritesect[j];
                    }
                    goto BOLT;
                }

                if(t[1] >= 20)
                {
                    t[4] = 1;
                    goto BOLT;
                }

                p = findplayer(s,&x);

                t[2]++;
                if( t[2] == 4 ) t[2]=0;

                if( x < 4096 )
                {
                    if( (TRAND&255) < 16 )
                    {
                        if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                            spritesound(DUKE_LONGTERM_PAIN,ps[p].i);

                        spritesound(SHORT_CIRCUIT,i);

                        sprite[ps[p].i].extra --;
                        ps[p].pals_time = 32;
                        ps[p].pals[0] = 32;
                        ps[p].pals[1] = 0;
                        ps[p].pals[2] = 0;
                    }
                    t[0] += 128;
                    if( t[3] == 0 )
                        t[3] = 1;
                }
                else t[3] = 0;

                if( t[1] )
                {
                    t[1]++;

                    t[4] = s->z;
                    s->z = sector[sect].floorz-(TRAND%(sector[sect].floorz-sector[sect].ceilingz));

                    switch( t[1] )
                    {
                        case 3:
                            //Turn on all of those flashing sectoreffector.
                            hitradius( i, 4096,
                                       impact_damage<<2,
                                       impact_damage<<2,
                                       impact_damage<<2,
                                       impact_damage<<2 );
/*
                            j = headspritestat[3];
                            while(j>=0)
                            {
                                if( sprite[j].lotag  == 3 )
                                    hittype[j].temp_data[4]=1;
                                else if(sprite[j].lotag == 12)
                                {
                                    hittype[j].temp_data[4] = 1;
                                    sprite[j].lotag = 3;
                                    sprite[j].owner = 0;
                                    hittype[j].temp_data[0] = s->shade;
                                }
                                j = nextspritestat[j];
                            }
*/
                            j = headspritestat[6];
                            while(j >= 0)
                            {
                                if(sprite[j].picnum == MASTERSWITCH)
                                    if(sprite[j].hitag == s->hitag)
                                        if(sprite[j].yvel == 0)
                                            sprite[j].yvel = 1;
                                j = nextspritestat[j];
                            }
                            break;

                        case 4:
                        case 7:
                        case 10:
                        case 15:
                            j = headspritesect[sect];
                            while(j >= 0)
                            {
                                l = nextspritesect[j];

                                if(j != i)
                                {
                                    deletesprite(j);
                                    break;
                                }
                                j = l;
                            }
                            break;
                    }
                    for(x=0;x<16;x++)
                        RANDOMSCRAP;

                    s->z = t[4];
                    t[4] = 0;

                }
                else
                {
                    IFHIT
                    {
                        for(x=0;x<32;x++)
                            RANDOMSCRAP;
                        if(s->extra < 0)
                            t[1] = 1;
                    }
                }
                goto BOLT;

            case CAMERA1:

                if( t[0] == 0 )
                {
                    t[1]+=8;
                    if(camerashitable)
                    {
                        IFHIT
                        {
                            t[0] = 1; // static
                            s->cstat = (short)32768;
                            for(x=0;x<5;x++) RANDOMSCRAP;
                            goto BOLT;
                        }
                    }

                    if(s->hitag > 0)
                    {
                        if(t[1]<s->hitag)
                            s->ang+=8;
                        else if(t[1]<(s->hitag*3))
                            s->ang-=8;
                        else if(t[1] < (s->hitag<<2) )
                            s->ang+=8;
                        else
                        {
                            t[1]=8;
                            s->ang+=16;
                        }
                    }
                }
                goto BOLT;
        }


// #ifndef VOLOMEONE
        if( ud.multimode < 2 && badguy(s) )
        {
            if( actor_tog == 1)
            {
                s->cstat = (short)32768;
                goto BOLT;
            }
            else if(actor_tog == 2) s->cstat = 257;
        }
// #endif

        p = findplayer(s,&x);

        execute(i,p,x);

        BOLT:

        i = nexti;
    }

}


void moveexplosions(void)  // STATNUM 5
{
    short i, j, nexti, sect, p;
    int32_t l, x, *t;
    spritetype *s;

    i = headspritestat[5];
    while(i >= 0)
    {
        nexti = nextspritestat[i];

        t = &hittype[i].temp_data[0];
        s = &sprite[i];
        sect = s->sectnum;

        if( sect < 0 || s->xrepeat == 0 ) KILLIT(i);

        hittype[i].bposx = s->x;
        hittype[i].bposy = s->y;
        hittype[i].bposz = s->z;

        switch(s->picnum)
        {
            case NEON1:
            case NEON2:
            case NEON3:
            case NEON4:
            case NEON5:
            case NEON6:

                if( (global_random/(s->lotag+1)&31) > 4) s->shade = -127;
                else s->shade = 127;
                goto BOLT;

            case BLOODSPLAT1:
            case BLOODSPLAT2:
            case BLOODSPLAT3:
            case BLOODSPLAT4:

                if( t[0] == 7*26 ) goto BOLT;
                s->z += 16+(TRAND&15);
                t[0]++;
                if( (t[0]%9) == 0 ) s->yrepeat++;
                goto BOLT;

            case NUKEBUTTON:
            case NUKEBUTTON+1:
            case NUKEBUTTON+2:
            case NUKEBUTTON+3:

                if(t[0])
                {
                    t[0]++;
                    if(t[0] == 8) s->picnum = NUKEBUTTON+1;
                    else if(t[0] == 16)
                    {
                        s->picnum = NUKEBUTTON+2;
                        ps[sprite[s->owner].yvel].fist_incs = 1;
                    }
                    if( ps[sprite[s->owner].yvel].fist_incs == 26 )
                        s->picnum = NUKEBUTTON+3;
                }
                goto BOLT;

            case FORCESPHERE:

                l = s->xrepeat;
                if(t[1] > 0)
                {
                    t[1]--;
                    if(t[1] == 0)
                    {
                        KILLIT(i);
                    }
                }
                if(hittype[s->owner].temp_data[1] == 0)
                {
                    if(t[0] < 64)
                    {
                        t[0]++;
                        l += 3;
                    }
                }
                else
                    if(t[0] > 64)
                    {
                        t[0]--;
                        l -= 3;
                    }

                s->x = sprite[s->owner].x;
                s->y = sprite[s->owner].y;
                s->z = sprite[s->owner].z;
                s->ang += hittype[s->owner].temp_data[0];

                if(l > 64) l = 64;
                else if(l < 1) l = 1;

                s->xrepeat = l;
                s->yrepeat = l;
                s->shade = (l>>1)-48;

                for(j=t[0];j > 0;j--)
                    ssp(i,CLIPMASK0);
                goto BOLT;
            case WATERSPLASH2:

                t[0]++;
                if(t[0] == 1 )
                {
                    if(sector[sect].lotag != 1 && sector[sect].lotag != 2)
                        KILLIT(i);
/*                    else
                    {
                        l = getflorzofslope(sect,s->x,s->y)-s->z;
                        if( l > (16<<8) ) KILLIT(i);
                    }
                    else */ if(Sound[ITEM_SPLASH].num == 0)
                        spritesound(ITEM_SPLASH,i);
                }
                if(t[0] == 3)
                {
                    t[0] = 0;
                    t[1]++;
                }
                if(t[1] == 5)
                    deletesprite(i);
                goto BOLT;

            case FRAMEEFFECT1:
			case FRAMEEFFECT1_13CON:
                if(s->owner >= 0)
                {
                    t[0]++;

                    if( t[0] > 7 )
                    {
                        KILLIT(i);
                    }
                    else if( t[0] > 4 )
                        s->cstat |= 512+2;
                    else if( t[0] > 2 )
                        s->cstat |= 2;
                    s->xoffset = sprite[s->owner].xoffset;
                    s->yoffset = sprite[s->owner].yoffset;
                }
                goto BOLT;
            case INNERJAW:
            case INNERJAW+1:

                p = findplayer(s,&x);
                if(x < 512)
                {
                    ps[p].pals_time = 32;
                    ps[p].pals[0] = 32;
                    ps[p].pals[1] = 0;
                    ps[p].pals[2] = 0;
                    sprite[ps[p].i].extra -= 4;
                }

            case FIRELASER:
                if(s->extra != 999)
                    s->extra = 999;
                else KILLIT(i);
                break;
            case TONGUE:
                KILLIT(i);
            case MONEY+1:
            case MAIL+1:
            case PAPER+1:
                hittype[i].floorz = s->z = getflorzofslope(s->sectnum,s->x,s->y);
                break;
            case MONEY:
            case MAIL:
            case PAPER:

                s->xvel = (TRAND&7)+(sintable[T1&2047]>>9);
                T1 += (TRAND&63);
                if( (T1&2047) > 512 && (T1&2047) < 1596)
                {
                    if(sector[sect].lotag == 2)
                    {
                        if(s->zvel < 64)
                            s->zvel += (gc>>5)+(TRAND&7);
                    }
                    else
                        if(s->zvel < 144)
                            s->zvel += (gc>>5)+(TRAND&7);
                }

                ssp(i,CLIPMASK0);

                if( (TRAND&3) == 0 )
                    setsprite(i,s->x,s->y,s->z);

                if(s->sectnum == -1) KILLIT(i);
                l = getflorzofslope(s->sectnum,s->x,s->y);

                if( s->z > l )
                {
                    s->z = l;

                    insertspriteq(i);
                    PN ++;

                    j = headspritestat[5];
                    while(j >= 0)
                    {
                        if(sprite[j].picnum == BLOODPOOL)
                            if(ldist(s,&sprite[j]) < 348)
                        {
                            s->pal = 2;
                            break;
                        }
                        j = nextspritestat[j];
                    }
                }

                break;

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

                if(s->xvel > 0) s->xvel--;
                else s->xvel = 0;

                if( t[5] < 30*10 )
                    t[5]++;
                else { KILLIT(i); }


                if(s->zvel > 1024 && s->zvel < 1280)
                {
                    setsprite(i,s->x,s->y,s->z);
                    sect = s->sectnum;
                }

                l = getflorzofslope(sect,s->x,s->y);
                x = getceilzofslope(sect,s->x,s->y);
                if(x == l || sect < 0 || sect >= MAXSECTORS) KILLIT(i);

                if( s->z < l-(2<<8) )
                {
                    if(t[1] < 2) t[1]++;
                    else if(sector[sect].lotag != 2)
                    {
                        t[1] = 0;
                        if( s->picnum == DUKELEG || s->picnum == DUKETORSO || s->picnum == DUKEGUN )
                        {
                            if(t[0] > 6) t[0] = 0;
                            else t[0]++;
                        }
                        else
                        {
                            if(t[0] > 2)
                                t[0] = 0;
                            else t[0]++;
                        }
                    }

                    if(s->zvel < 6144)
                    {
                        if(sector[sect].lotag == 2)
                        {
                            if(s->zvel < 1024)
                                s->zvel += 48;
                            else s->zvel = 1024;
                        }
                        else s->zvel += gc-50;
                    }

                    s->x += (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    s->y += (s->xvel*sintable[s->ang&2047])>>14;
                    s->z += s->zvel;

                }
                else
                {
                    if(t[2] == 0)
                    {
                        if( s->sectnum == -1) { KILLIT(i); }
                        if( (sector[s->sectnum].floorstat&2) ) { KILLIT(i); }
                        t[2]++;
                    }
                    l = getflorzofslope(s->sectnum,s->x,s->y);

                    s->z = l-(2<<8);
                    s->xvel = 0;

                    if(s->picnum == JIBS6)
                    {
                        t[1]++;
                        if( (t[1]&3) == 0 && t[0] < 7)
                            t[0]++;
                        if(t[1] > 20) KILLIT(i);
                    }
                    else { s->picnum = JIBS6; t[0] = 0; t[1] = 0; }
                }
                goto BOLT;

            case BLOODPOOL:
            case PUKE:

                if(t[0] == 0)
                {
                    t[0] = 1;
                    if(sector[sect].floorstat&2) { KILLIT(i); }
                    else insertspriteq(i);
                }

                makeitfall(i);

                p = findplayer(s,&x);

                s->z = hittype[i].floorz-(FOURSLEIGHT);

                if(t[2] < 32)
                {
                    t[2]++;
                    if(hittype[i].picnum == TIRE)
                    {
                        if(s->xrepeat < 64 && s->yrepeat < 64)
                        {
                            s->xrepeat += TRAND&3;
                            s->yrepeat += TRAND&3;
                        }
                    }
                    else
                    {
                        if(s->xrepeat < 32 && s->yrepeat < 32)
                        {
                            s->xrepeat += TRAND&3;
                            s->yrepeat += TRAND&3;
                        }
                    }
                }

                if(x < 844 && s->xrepeat > 6 && s->yrepeat > 6)
                {
                    if( s->pal == 0 && (TRAND&255) < 16 && s->picnum != PUKE)
                    {
                        if(ps[p].boot_amount > 0)
                            ps[p].boot_amount--;
                        else
                        {
                            if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                spritesound(DUKE_LONGTERM_PAIN,ps[p].i);
                            sprite[ps[p].i].extra --;
                            ps[p].pals_time = 32;
                            ps[p].pals[0] = 16;
                            ps[p].pals[1] = 0;
                            ps[p].pals[2] = 0;
                        }
                    }

                    if(t[1] == 1) goto BOLT;
                    t[1] = 1;

                    if(hittype[i].picnum == TIRE)
                        ps[p].footprintcount = 10;
                    else ps[p].footprintcount = 3;

                    ps[p].footprintpal = s->pal;
                    ps[p].footprintshade = s->shade;

                    if(t[2] == 32)
                    {
                        s->xrepeat -= 6;
                        s->yrepeat -= 6;
                    }
                }
                else t[1] = 0;
                goto BOLT;

            case BURNING:
            case BURNING2:
            case FECES:
            case WATERBUBBLE:
            case SMALLSMOKE:
            case EXPLOSION2:
            case SHRINKEREXPLOSION:
            case EXPLOSION2BOT:
            case BLOOD:
            case LASERSITE:
            case FORCERIPPLE:
            case TRANSPORTERSTAR:
            case TRANSPORTERBEAM:
                p = findplayer(s,&x);
                execute(i,p,x);
                goto BOLT;

            case SHELL:
            case SHOTGUNSHELL:

                ssp(i,CLIPMASK0);

                if(sect < 0 || ( sector[sect].floorz+(24<<8) ) < s->z ) KILLIT(i);

                if(sector[sect].lotag == 2)
                {
                    t[1]++;
                    if(t[1] > 8)
                    {
                        t[1] = 0;
                        t[0]++;
                        t[0] &= 3;
                    }
                    if(s->zvel < 128) s->zvel += (gc/13); // 8
                    else s->zvel -= 64;
                    if(s->xvel > 0)
                        s->xvel -= 4;
                    else s->xvel = 0;
                }
                else
                {
                    t[1]++;
                    if(t[1] > 3)
                    {
                        t[1] = 0;
                        t[0]++;
                        t[0] &= 3;
                    }
                    if(s->zvel < 512) s->zvel += (gc/3); // 52;
                    if(s->xvel > 0)
                        s->xvel --;
                    else KILLIT(i);
                }

                goto BOLT;

            case GLASSPIECES:
            case GLASSPIECES+1:
            case GLASSPIECES+2:

                makeitfall(i);

                if(s->zvel > 4096) s->zvel = 4096;
                if(sect < 0) KILLIT(i);

                if( s->z == hittype[i].floorz-(FOURSLEIGHT) && t[0] < 3)
                {
                    s->zvel = -((3-t[0])<<8)-(TRAND&511);
                    if(sector[sect].lotag == 2)
                        s->zvel >>= 1;
                    s->xrepeat >>= 1;
                    s->yrepeat >>= 1;
                    if( rnd(96) )
                      setsprite(i,s->x,s->y,s->z);
                    t[0]++;//Number of bounces
                }
                else if( t[0] == 3 ) KILLIT(i);

                if(s->xvel > 0)
                {
                    s->xvel -= 2;
                    s->cstat = ((s->xvel&3)<<2);
                }
                else s->xvel = 0;

                ssp(i,CLIPMASK0);

                goto BOLT;
        }

        IFWITHIN(SCRAP6,SCRAP5+3)
        {
                if(s->xvel > 0)
                    s->xvel--;
                else s->xvel = 0;

                if(s->zvel > 1024 && s->zvel < 1280)
                {
                    setsprite(i,s->x,s->y,s->z);
                    sect = s->sectnum;
                }

                if( s->z < sector[sect].floorz-(2<<8) )
                {
                    if(t[1] < 1) t[1]++;
                    else
                    {
                        t[1] = 0;

                        if(s->picnum < SCRAP6+8)
                        {
                            if(t[0] > 6)
                                t[0] = 0;
                            else t[0]++;
                        }
                        else
                        {
                            if(t[0] > 2)
                                t[0] = 0;
                            else t[0]++;
                        }
                    }
                    if(s->zvel < 4096) s->zvel += gc-50;
                    s->x += (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    s->y += (s->xvel*sintable[s->ang&2047])>>14;
                    s->z += s->zvel;
                }
                else
                {
                    if(s->picnum == SCRAP1 && s->yvel > 0)
                    {
                        j = spawn(i,s->yvel);
                        setsprite(j,s->x,s->y,s->z);
                        getglobalz(j);
                        sprite[j].hitag = sprite[j].lotag = 0;
                    }
                    KILLIT(i);
                }
                goto BOLT;
        }

        BOLT:
        i = nexti;
    }
}

void moveeffectors(void)   //STATNUM 3
{
    int32_t q=0, l, m, x, st, j, *t;
    short i, k, nexti, nextk, p, sh, nextj;
    spritetype *s;
    sectortype *sc;
    walltype *wal;

    fricxv = fricyv = 0;

    i = headspritestat[3];
    while(i >= 0)
    {
        nexti = nextspritestat[i];
        s = &sprite[i];

        sc = &sector[s->sectnum];
        st = s->lotag;
        sh = s->hitag;

        t = &hittype[i].temp_data[0];

        switch(st)
        {
            case 0:
            {
                int32_t zchange = 0;

                zchange = 0;

                j = s->owner;

                if( sprite[j].lotag == (short) 65535 )
                    KILLIT(i);

                q = sc->extra>>3;
                l = 0;

                if(sc->lotag == 30)
                {
                    q >>= 2;

                    if( sprite[i].extra == 1 )
                    {
                        if(hittype[i].tempang < 256)
                        {
                            hittype[i].tempang += 4;
                            if(hittype[i].tempang >= 256)
                                callsound(s->sectnum,i);
                            if(s->clipdist) l = 1;
                            else l = -1;
                        }
                        else hittype[i].tempang = 256;

                        if( sc->floorz > s->z ) //z's are touching
                        {
                            sc->floorz -= 512;
                            zchange = -512;
                            if( sc->floorz < s->z )
                                sc->floorz = s->z;
                        }

                        else if( sc->floorz < s->z ) //z's are touching
                        {
                            sc->floorz += 512;
                            zchange = 512;
                            if( sc->floorz > s->z )
                                sc->floorz = s->z;
                        }
                    }
                    else if(sprite[i].extra == 3)
                    {
                        if(hittype[i].tempang > 0)
                        {
                            hittype[i].tempang -= 4;
                            if(hittype[i].tempang <= 0)
                                callsound(s->sectnum,i);
                            if( s->clipdist ) l = -1;
                            else l = 1;
                        }
                        else hittype[i].tempang = 0;

                        if( sc->floorz > T4 ) //z's are touching
                        {
                            sc->floorz -= 512;
                            zchange = -512;
                            if( sc->floorz < T4 )
                                sc->floorz = T4;
                        }

                        else if( sc->floorz < T4 ) //z's are touching
                        {
                            sc->floorz += 512;
                            zchange = 512;
                            if( sc->floorz > T4 )
                                sc->floorz = T4;
                        }
                    }

                    s->ang += (l*q);
                    t[2] += (l*q);
                }
                else
                {
                    if( hittype[j].temp_data[0] == 0 ) break;
                    if( hittype[j].temp_data[0] == 2 ) KILLIT(i);

                    if( sprite[j].ang > 1024 )
                        l = -1;
                    else l = 1;
                    if( t[3] == 0 )
                        t[3] = ldist(s,&sprite[j]);
                    s->xvel = t[3];
                    s->x = sprite[j].x;
                    s->y = sprite[j].y;
                    s->ang += (l*q);
                    t[2] += (l*q);
                }

                if( l && (sc->floorstat&64) )
                {
                    for(p=connecthead;p>=0;p=connectpoint2[p])
                    {
                        if( ps[p].cursectnum == s->sectnum && ps[p].on_ground == 1)
                        {

                            ps[p].ang += (l*q);
                            ps[p].ang &= 2047;

                            ps[p].posz += zchange;

                            rotatepoint( sprite[j].x,sprite[j].y,
                                ps[p].posx,ps[p].posy,(q*l),
                                &m,&x);

                            ps[p].bobposx += m-ps[p].posx;
                            ps[p].bobposy += x-ps[p].posy;

                            ps[p].posx = m;
                            ps[p].posy = x;

                            if(sprite[ps[p].i].extra <= 0)
                            {
                                sprite[ps[p].i].x = m;
                                sprite[ps[p].i].y = x;
                            }
                        }
                    }

                    p = headspritesect[s->sectnum];
                    while(p >= 0)
                    {
                        if(sprite[p].statnum != 3 && sprite[p].statnum != 4)
                            if( sprite[p].picnum != LASERLINE )
                        {
                            if(sprite[p].picnum == APLAYER && sprite[p].owner >= 0)
                            {
                                p = nextspritesect[p];
                                continue;
                            }

                            sprite[p].ang += (l*q);
                            sprite[p].ang &= 2047;

                            sprite[p].z += zchange;

                            rotatepoint(sprite[j].x,sprite[j].y,
                                sprite[p].x,sprite[p].y,(q*l),
                                &sprite[p].x,&sprite[p].y);

                        }
                        p = nextspritesect[p];
                    }

                }

                ms(i);
            }

            break;
            case 1: //Nothing for now used as the pivot
                if(s->owner == -1) //Init
                {
                    s->owner = i;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if( sprite[j].lotag == 19 && sprite[j].hitag == sh )
                        {
                            t[0] = 0;
                            break;
                        }
                        j = nextspritestat[j];
                    }
                }

                break;
            case 6:
                k = sc->extra;

                if(t[4] > 0)
                {
                    t[4]--;
                    if( t[4] >= (k-(k>>3)) )
                        s->xvel -= (k>>5);
                    if( t[4] > ((k>>1)-1) && t[4] < (k-(k>>3)) )
                        s->xvel = 0;
                    if( t[4] < (k>>1) )
                        s->xvel += (k>>5);
                    if( t[4] < ((k>>1)-(k>>3)) )
                    {
                        t[4] = 0;
                        s->xvel = k;
                    }
                }
                else s->xvel = k;

                j = headspritestat[3];
                while( j >= 0)
                {
                    if( (sprite[j].lotag == 14) && (sh == sprite[j].hitag) && (hittype[j].temp_data[0] == t[0]) )
                    {
                        sprite[j].xvel = s->xvel;
//                        if( t[4] == 1 )
                        {
                            if(hittype[j].temp_data[5] == 0)
                                hittype[j].temp_data[5] = dist(&sprite[j],s);
                            x = sgn( dist(&sprite[j],s)-hittype[j].temp_data[5] );
                            if(sprite[j].extra)
                                x = -x;
                            s->xvel += x;
                        }
                        hittype[j].temp_data[4] = t[4];
                    }
                    j = nextspritestat[j];
                }
                x = 0;


            case 14:
                if(s->owner==-1)
                    s->owner = LocateTheLocator((short)t[3],(short)t[0]);

                if(s->owner == -1)
                {
                    sprintf((char *)tempbuf,"Could not find any locators for SE# 6 and 14 with a hitag of %d.\n",t[3]);
                    gameexit((char *)tempbuf);
                }

                j = ldist(&sprite[s->owner],s);

                if( j < 1024L )
                {
                    if(st==6)
                        if(sprite[s->owner].hitag&1)
                            t[4]=sc->extra; //Slow it down
                    t[3]++;
                    s->owner = LocateTheLocator(t[3],t[0]);
                    if(s->owner==-1)
                    {
                        t[3]=0;
                        s->owner = LocateTheLocator(0,t[0]);
                    }
                }

                if(s->xvel)
                {
                    x = getangle(sprite[s->owner].x-s->x,sprite[s->owner].y-s->y);
                    q = getincangle(s->ang,x)>>3;

                    t[2] += q;
                    s->ang += q;

                    if(s->xvel == sc->extra )
                    {
                        if( (sc->floorstat&1) == 0 && (sc->ceilingstat&1) == 0 )
                        {
                            if( Sound[hittype[i].lastvx].num == 0 )
                                spritesound(hittype[i].lastvx,i);
                        }
                        else if( ud.monsters_off == 0 && sc->floorpal == 0 && (sc->floorstat&1) && rnd(8) )
                        {
                            p = findplayer(s,&x);
                            if(x < 20480)
                            {
                                j = s->ang;
                                s->ang = getangle(s->x-ps[p].posx,s->y-ps[p].posy);
                                shoot(i,RPG);
                                s->ang = j;
                            }
                        }
                    }

                    if(s->xvel <= 64 && (sc->floorstat&1) == 0 && (sc->ceilingstat&1) == 0 )
                        stopsound(hittype[i].lastvx);

                    if( (sc->floorz-sc->ceilingz) < (108<<8) )
                    {
                        if(ud.clipping == 0 && s->xvel >= 192)
                            for(p=connecthead;p>=0;p=connectpoint2[p])
                                if(sprite[ps[p].i].extra > 0)
                        {
                            k = ps[p].cursectnum;
                            updatesector(ps[p].posx,ps[p].posy,&k);
                            if( ( k == -1 && ud.clipping == 0 ) || ( k == s->sectnum && ps[p].cursectnum != s->sectnum ) )
                            {
                                ps[p].posx = s->x;
                                ps[p].posy = s->y;
                                ps[p].cursectnum = s->sectnum;

                                setsprite(ps[p].i,s->x,s->y,s->z);
                                quickkill(&ps[p]);
                            }
                        }
                    }

                    m = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    x = (s->xvel*sintable[s->ang&2047])>>14;

                    for(p = connecthead;p >= 0;p=connectpoint2[p])
                       if(sector[ps[p].cursectnum].lotag != 2)
                    {
                        if(po[p].os == s->sectnum)
                        {
                            po[p].ox += m;
                            po[p].oy += x;
                        }

                        if(s->sectnum == sprite[ps[p].i].sectnum)
                        {
                            rotatepoint(s->x,s->y,ps[p].posx,ps[p].posy,q,&ps[p].posx,&ps[p].posy);

                            ps[p].posx += m;
                            ps[p].posy += x;

                            ps[p].bobposx += m;
                            ps[p].bobposy += x;

                            ps[p].ang += q;

                            if(numplayers > 1)
                            {
                                ps[p].oposx = ps[p].posx;
                                ps[p].oposy = ps[p].posy;
                            }
                            if( sprite[ps[p].i].extra <= 0 )
                            {
                                sprite[ps[p].i].x = ps[p].posx;
                                sprite[ps[p].i].y = ps[p].posy;
                            }
                        }
                    }
                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        if (sprite[j].statnum != 10 && sector[sprite[j].sectnum].lotag != 2 && sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                        {
                            rotatepoint(s->x,s->y,
                                sprite[j].x,sprite[j].y,q,
                                &sprite[j].x,&sprite[j].y);

                            sprite[j].x+= m;
                            sprite[j].y+= x;

                            sprite[j].ang+=q;

                            if(numplayers > 1)
                            {
                                hittype[j].bposx = sprite[j].x;
                                hittype[j].bposy = sprite[j].y;
                            }
                        }
                        j = nextspritesect[j];
                    }

                    ms(i);
                    setsprite(i,s->x,s->y,s->z);

                    if( (sc->floorz-sc->ceilingz) < (108<<8) )
                    {
                        if(ud.clipping == 0 && s->xvel >= 192)
                            for(p=connecthead;p>=0;p=connectpoint2[p])
                                if(sprite[ps[p].i].extra > 0)
                        {
                            k = ps[p].cursectnum;
                            updatesector(ps[p].posx,ps[p].posy,&k);
                            if( ( k == -1 && ud.clipping == 0 ) || ( k == s->sectnum && ps[p].cursectnum != s->sectnum ) )
                            {
                                ps[p].oposx = ps[p].posx = s->x;
                                ps[p].oposy = ps[p].posy = s->y;
                                ps[p].cursectnum = s->sectnum;

                                setsprite(ps[p].i,s->x,s->y,s->z);
                                quickkill(&ps[p]);
                            }
                        }

                        j = headspritesect[sprite[OW].sectnum];
                        while(j >= 0)
                        {
                            l = nextspritesect[j];
                            if (sprite[j].statnum == 1 && badguy(&sprite[j]) && sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                            {
                                k = sprite[j].sectnum;
                                updatesector(sprite[j].x,sprite[j].y,&k);
                                if( sprite[j].extra >= 0 && k == s->sectnum )
                                {
                                    gutsdir(&sprite[j],JIBS6,72,myconnectindex);
                                    spritesound(SQUISHED,i);
                                    deletesprite(j);
                                }
                            }
                            j = l;
                        }
                    }
                }

                break;

            case 30:
                if(s->owner == -1)
                {
                    t[3] = !t[3];
                    s->owner = LocateTheLocator(t[3],t[0]);
                }
                else
                {

                    if(t[4] == 1) // Starting to go
                    {
                        if( ldist( &sprite[s->owner],s ) < (2048-128) )
                            t[4] = 2;
                        else
                        {
                            if(s->xvel == 0)
                                operateactivators(s->hitag+(!t[3]),-1);
                            if(s->xvel < 256)
                                s->xvel += 16;
                        }
                    }
                    if(t[4] == 2)
                    {
                        l = FindDistance2D(sprite[s->owner].x-s->x,sprite[s->owner].y-s->y);

                        if(l <= 128)
                            s->xvel = 0;

                        if( s->xvel > 0 )
                            s->xvel -= 16;
                        else
                        {
                            s->xvel = 0;
                            operateactivators(s->hitag+(short)t[3],-1);
                            s->owner = -1;
                            s->ang += 1024;
                            t[4] = 0;
                            operateforcefields(i,s->hitag);

                            j = headspritesect[s->sectnum];
                            while(j >= 0)
                            {
                                if(sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                                {
                                    hittype[j].bposx = sprite[j].x;
                                    hittype[j].bposy = sprite[j].y;
                                }
                                j = nextspritesect[j];
                            }

                        }
                    }
                }

                if(s->xvel)
                {
                    l = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    x = (s->xvel*sintable[s->ang&2047])>>14;

                    if( (sc->floorz-sc->ceilingz) < (108<<8) )
                        if(ud.clipping == 0)
                            for(p=connecthead;p>=0;p=connectpoint2[p])
                                if(sprite[ps[p].i].extra > 0)
                    {
                        k = ps[p].cursectnum;
                        updatesector(ps[p].posx,ps[p].posy,&k);
                        if( ( k == -1 && ud.clipping == 0 ) || ( k == s->sectnum && ps[p].cursectnum != s->sectnum ) )
                        {
                            ps[p].posx = s->x;
                            ps[p].posy = s->y;
                            ps[p].cursectnum = s->sectnum;

                            setsprite(ps[p].i,s->x,s->y,s->z);
                            quickkill(&ps[p]);
                        }
                    }

                    for(p = connecthead;p >= 0;p = connectpoint2[p])
                    {
                        if( sprite[ps[p].i].sectnum == s->sectnum )
                        {
                            ps[p].posx += l;
                            ps[p].posy += x;

                            if(numplayers > 1)
                            {
                                ps[p].oposx = ps[p].posx;
                                ps[p].oposy = ps[p].posy;
                            }

                            ps[p].bobposx += l;
                            ps[p].bobposy += x;
                        }

                        if( po[p].os == s->sectnum )
                        {
                            po[p].ox += l;
                            po[p].oy += x;
                        }
                    }

                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        if(sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                        {
                            if(numplayers < 2)
                            {
                                hittype[j].bposx = sprite[j].x;
                                hittype[j].bposy = sprite[j].y;
                            }

                            sprite[j].x += l;
                            sprite[j].y += x;

                            if(numplayers > 1)
                            {
                                hittype[j].bposx = sprite[j].x;
                                hittype[j].bposy = sprite[j].y;
                            }
                        }
                        j = nextspritesect[j];
                    }

                    ms(i);
                    setsprite(i,s->x,s->y,s->z);

                    if( (sc->floorz-sc->ceilingz) < (108<<8) )
                    {
                        if(ud.clipping == 0)
                            for(p=connecthead;p>=0;p=connectpoint2[p])
                                if(sprite[ps[p].i].extra > 0)
                        {
                            k = ps[p].cursectnum;
                            updatesector(ps[p].posx,ps[p].posy,&k);
                            if( ( k == -1 && ud.clipping == 0 ) || ( k == s->sectnum && ps[p].cursectnum != s->sectnum ) )
                            {
                                ps[p].posx = s->x;
                                ps[p].posy = s->y;

                                ps[p].oposx = ps[p].posx;
                                ps[p].oposy = ps[p].posy;

                                ps[p].cursectnum = s->sectnum;

                                setsprite(ps[p].i,s->x,s->y,s->z);
                                quickkill(&ps[p]);
                            }
                        }

                        j = headspritesect[sprite[OW].sectnum];
                        while(j >= 0)
                        {
                            l = nextspritesect[j];
                            if (sprite[j].statnum == 1 && badguy(&sprite[j]) && sprite[j].picnum != SECTOREFFECTOR && sprite[j].picnum != LOCATORS )
                            {
            //                    if(sprite[j].sectnum != s->sectnum)
                                {
                                    k = sprite[j].sectnum;
                                    updatesector(sprite[j].x,sprite[j].y,&k);
                                    if( sprite[j].extra >= 0 && k == s->sectnum )
                                    {
                                        gutsdir(&sprite[j],JIBS6,24,myconnectindex);
                                        spritesound(SQUISHED,j);
                                        deletesprite(j);
                                    }
                                }

                            }
                            j = l;
                        }
                    }
                }

                break;


            case 2://Quakes
                if(t[4] > 0 && t[0] == 0 )
                {
                    if( t[4] < sh )
                        t[4]++;
                    else t[0] = 1;
                }

                if(t[0] > 0)
                {
                    t[0]++;

                    s->xvel = 3;

                    if(t[0] > 96)
                    {
                        t[0] = -1; //Stop the quake
                        t[4] = -1;
                        KILLIT(i);
                    }
                    else
                    {
                        if( (t[0]&31) ==  8 )
                        {
                            earthquaketime = 48;
                            spritesound(EARTHQUAKE,ps[screenpeek].i);
                        }

                        if( klabs( sc->floorheinum-t[5] ) < 8 )
                            sc->floorheinum = t[5];
                        else sc->floorheinum += ( sgn(t[5]-sc->floorheinum)<<4 );
                    }

                    m = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    x = (s->xvel*sintable[s->ang&2047])>>14;


                    for(p=connecthead;p>=0;p=connectpoint2[p])
                        if(ps[p].cursectnum == s->sectnum && ps[p].on_ground)
                        {
                            ps[p].posx += m;
                            ps[p].posy += x;

                            ps[p].bobposx += m;
                            ps[p].bobposy += x;
                        }

                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        nextj = nextspritesect[j];

                        if (sprite[j].picnum != SECTOREFFECTOR)
                        {
                            sprite[j].x+=m;
                            sprite[j].y+=x;
                            setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
                        }
                        j = nextj;
                    }
                    ms(i);
                    setsprite(i,s->x,s->y,s->z);
                }
                break;

            //Flashing sector lights after reactor EXPLOSION2

            case 3:

                if( t[4] == 0 ) break;
                p = findplayer(s,&x);

            //    if(t[5] > 0) { t[5]--; break; }

                if( (global_random/(sh+1)&31) < 4 && !t[2])
                {
             //       t[5] = 4+(global_random&7);
                    sc->ceilingpal = s->owner>>8;
                    sc->floorpal = s->owner&0xff;
                    t[0] = s->shade + (global_random&15);
                }
                else
                {
             //       t[5] = 4+(global_random&3);
                    sc->ceilingpal = s->pal;
                    sc->floorpal = s->pal;
                    t[0] = t[3];
                }

                sc->ceilingshade = t[0];
                sc->floorshade = t[0];

                wal = &wall[sc->wallptr];

                for(x=sc->wallnum;x > 0;x--,wal++)
                {
                    if( wal->hitag != 1 )
                    {
                        wal->shade = t[0];
                        if((wal->cstat&2) && wal->nextwall >= 0)
                        {
                            wall[wal->nextwall].shade = wal->shade;
                        }
                    }
                }

                break;

            case 4:

                if((global_random/(sh+1)&31) < 4 )
                {
                    t[1] = s->shade + (global_random&15);//Got really bright
                    t[0] = s->shade + (global_random&15);
                    sc->ceilingpal = s->owner>>8;
                    sc->floorpal = s->owner&0xff;
                    j = 1;
                }
                else
                {
                    t[1] = t[2];
                    t[0] = t[3];

                    sc->ceilingpal = s->pal;
                    sc->floorpal = s->pal;

                    j = 0;
                }

                sc->floorshade = t[1];
                sc->ceilingshade = t[1];

                wal = &wall[sc->wallptr];

                for(x=sc->wallnum;x > 0; x--,wal++)
                {
                    if(j) wal->pal = (s->owner&0xff);
                    else wal->pal = s->pal;

                    if( wal->hitag != 1 )
                    {
                        wal->shade = t[0];
                        if((wal->cstat&2) && wal->nextwall >= 0)
                            wall[wal->nextwall].shade = wal->shade;
                    }
                }

                j = headspritesect[SECT];
                while(j >= 0)
                {
                    if(sprite[j].cstat&16)
                    {
                        if (sc->ceilingstat&1)
                            sprite[j].shade = sc->ceilingshade;
                        else sprite[j].shade = sc->floorshade;
                    }

                    j = nextspritesect[j];
                }

                if(t[4]) KILLIT(i);

                break;

            //BOSS
            case 5:
                p = findplayer(s,&x);
                if(x < 8192)
                {
                    j = s->ang;
                    s->ang = getangle(s->x-ps[p].posx,s->y-ps[p].posy);
                    shoot(i,FIRELASER);
                    s->ang = j;
                }

                if(s->owner==-1) //Start search
                {
                    t[4]=0;
                    l = 0x7fffffff;
                    while(1) //Find the shortest dist
                    {
                        s->owner = LocateTheLocator((short)t[4],-1); //t[0] hold sectnum

                        if(s->owner==-1) break;

                        m = ldist(&sprite[ps[p].i],&sprite[s->owner]);

                        if(l > m)
                        {
                            q = s->owner;
                            l = m;
                        }

                        t[4]++;
                    }

                    s->owner = q;
                    s->zvel = ksgn(sprite[q].z-s->z)<<4;
                }

                if(ldist(&sprite[s->owner],s) < 1024)
                {
                    short ta;
                    ta = s->ang;
                    s->ang = getangle(ps[p].posx-s->x,ps[p].posy-s->y);
                    s->ang = ta;
                    s->owner = -1;
                    goto BOLT;

                }
                else s->xvel=256;

                x = getangle(sprite[s->owner].x-s->x,sprite[s->owner].y-s->y);
                q = getincangle(s->ang,x)>>3;
                s->ang += q;
                
                if(rnd(32))
                {
                    t[2]+=q;
                    sc->ceilingshade = 127;
                }
                else
                {
                    t[2] +=
                        getincangle(t[2]+512,getangle(ps[p].posx-s->x,ps[p].posy-s->y))>>2;
                    sc->ceilingshade = 0;
                }
                IFHIT
                {
                    t[3]++;
                    if(t[3] == 5)
                    {
                        s->zvel += 1024;
                        FTA(7,&ps[myconnectindex],0);
                    }
                }

                s->z += s->zvel;
                sc->ceilingz += s->zvel;
                sector[t[0]].ceilingz += s->zvel;
                ms(i);
                setsprite(i,s->x,s->y,s->z);
                break;

            
            case 8:
            case 9:

                // work only if its moving

                j = -1;

                if(hittype[i].temp_data[4])
                {
                    hittype[i].temp_data[4]++;
                    if( hittype[i].temp_data[4] > 8 ) KILLIT(i);
                    j = 1;
                }
                else j = getanimationgoal(&sc->ceilingz);

                if( j >= 0 )
                {
                    short sn;

                    if( (sc->lotag&0x8000) || hittype[i].temp_data[4] )
                        x = -t[3];
                    else
                        x = t[3];

                    if ( st == 9 ) x = -x;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                        if( ((sprite[j].lotag) == st ) && (sprite[j].hitag) == sh )
                        {
                            sn = sprite[j].sectnum;
                            m = sprite[j].shade;

                            wal = &wall[sector[sn].wallptr];

                            for(l=sector[sn].wallnum;l>0;l--,wal++)
                            {
                                if( wal->hitag != 1 )
                                {
                                    wal->shade+=x;

                                    if(wal->shade < m)
                                        wal->shade = m;
                                    else if(wal->shade > hittype[j].temp_data[2])
                                        wal->shade = hittype[j].temp_data[2];

                                    if(wal->nextwall >= 0)
                                        if(wall[wal->nextwall].hitag != 1)
                                            wall[wal->nextwall].shade = wal->shade;
                                }
                            }

                            sector[sn].floorshade   += x;
                            sector[sn].ceilingshade += x;

                            if(sector[sn].floorshade < m)
                                sector[sn].floorshade = m;
                            else if(sector[sn].floorshade > hittype[j].temp_data[0])
                                sector[sn].floorshade = hittype[j].temp_data[0];

                            if(sector[sn].ceilingshade < m)
                                sector[sn].ceilingshade = m;
                            else if(sector[sn].ceilingshade > hittype[j].temp_data[1])
                                sector[sn].ceilingshade = hittype[j].temp_data[1];

                        }
                        j = nextspritestat[j];
                    }
                }
                break;
            case 10:

                if( (sc->lotag&0xff) == 27 || ( sc->floorz > sc->ceilingz && (sc->lotag&0xff) != 23 ) || sc->lotag == (short) 32791 )
                {
                    j = 1;

                    if( (sc->lotag&0xff) != 27)
                        for(p=connecthead;p>=0;p=connectpoint2[p])
                            if( sc->lotag != 30 && sc->lotag != 31 && sc->lotag != 0 )
                                if(s->sectnum == sprite[ps[p].i].sectnum)
                                    j = 0;

                    if(j == 1)
                    {
                        if(t[0] > sh )
                            switch(sector[s->sectnum].lotag)
                            {
                                case 20:
                                case 21:
                                case 22:
                                case 26:
                                  if( getanimationgoal(&sector[s->sectnum].ceilingz) >= 0 )
                                      break;
                                default:
                                  activatebysector(s->sectnum,i);
                                  t[0] = 0;
                                  break;
                            }
                        else t[0]++;
                    }
                }
                else t[0]=0;
                break;
            case 11: //Swingdoor

                if( t[5] > 0)
                {
                    t[5]--;
                    break;
                }

                if( t[4] )
                {
                    short startwall,endwall;

                    startwall = sc->wallptr;
                    endwall = startwall+sc->wallnum;

                    for(j=startwall;j<endwall;j++)
                    {
                        k = headspritestat[1];
                        while(k >= 0)
                        {
                            if( sprite[k].extra > 0 && badguy(&sprite[k]) && clipinsidebox(sprite[k].x,sprite[k].y,j,256L) == 1 )
                                goto BOLT;
                            k = nextspritestat[k];
                        }

                        k = headspritestat[10];
                        while(k >= 0)
                        {
                            if( sprite[k].owner >= 0 && clipinsidebox(sprite[k].x,sprite[k].y,j,144L) == 1 )
                            {
                                t[5] = 8; // Delay
                                k = (SP>>3)*t[3];
                                t[2]-=k;
                                t[4]-=k;
                                ms(i);
                                setsprite(i,s->x,s->y,s->z);
                                goto BOLT;
                            }
                            k = nextspritestat[k];
                        }
                    }

                    k = (SP>>3)*t[3];
                    t[2]+=k;
                    t[4]+=k;
                    ms(i);
                    setsprite(i,s->x,s->y,s->z);

                    if(t[4] <= -511 || t[4] >= 512)
                    {
                        t[4] = 0;
                        t[2] &= 0xffffff00;
                        ms(i);
                        setsprite(i,s->x,s->y,s->z);
                        break;
                    }
                }
                break;
            case 12:
                if( t[0] == 3 || t[3] == 1 ) //Lights going off
                {
                    sc->floorpal = 0;
                    sc->ceilingpal = 0;

                    wal = &wall[sc->wallptr];
                    for(j = sc->wallnum;j > 0; j--, wal++)
                        if(wal->hitag != 1)
                        {
                            wal->shade = t[1];
                            wal->pal = 0;
                        }

                    sc->floorshade = t[1];
                    sc->ceilingshade = t[2];
                    t[0]=0;

                    j = headspritesect[SECT];
                    while(j >= 0)
                    {
                        if(sprite[j].cstat&16)
                        {
                            if (sc->ceilingstat&1)
                                sprite[j].shade = sc->ceilingshade;
                            else sprite[j].shade = sc->floorshade;
                        }
                        j = nextspritesect[j];

                    }

                    if(t[3] == 1) KILLIT(i);
                }
                if( t[0] == 1 ) //Lights flickering on
                {
                    if( sc->floorshade > s->shade )
                    {
                        sc->floorpal = s->pal;
                        sc->ceilingpal = s->pal;

                        sc->floorshade -= 2;
                        sc->ceilingshade -= 2;

                        wal = &wall[sc->wallptr];
                        for(j=sc->wallnum;j>0;j--,wal++)
                            if(wal->hitag != 1)
                            {
                                wal->pal = s->pal;
                                wal->shade -= 2;
                            }
                    }
                    else t[0] = 2;

                    j = headspritesect[SECT];
                    while(j >= 0)
                    {
                        if(sprite[j].cstat&16)
                        {
                            if (sc->ceilingstat&1)
                                sprite[j].shade = sc->ceilingshade;
                            else sprite[j].shade = sc->floorshade;
                        }
                        j = nextspritesect[j];
                    }
                }
                break;


            case 13:
                if( t[2] )
                {
                    j = (SP<<5)|1;

                    if( s->ang == 512 )
                    {
                        if( s->owner )
                        {
                            if( klabs(t[0]-sc->ceilingz) >= j )
                                sc->ceilingz += sgn(t[0]-sc->ceilingz)*j;
                            else sc->ceilingz = t[0];
                        }
                        else
                        {
                            if( klabs(t[1]-sc->floorz) >= j )
                                sc->floorz += sgn(t[1]-sc->floorz)*j;
                            else sc->floorz = t[1];
                        }
                    }
                    else
                    {
                        if( klabs(t[1]-sc->floorz) >= j )
                            sc->floorz += sgn(t[1]-sc->floorz)*j;
                        else sc->floorz = t[1];
                        if( klabs(t[0]-sc->ceilingz) >= j )
                            sc->ceilingz += sgn(t[0]-sc->ceilingz)*j;
                        sc->ceilingz = t[0];
                    }

                    if( t[3] == 1 )
                    {
                        //Change the shades

                        t[3]++;
                        sc->ceilingstat ^= 1;

                        if(s->ang == 512)
                        {
                            wal = &wall[sc->wallptr];
                            for(j=sc->wallnum;j>0;j--,wal++)
                                wal->shade = s->shade;

                            sc->floorshade = s->shade;

                            if(ps[0].one_parallax_sectnum >= 0)
                            {
                                sc->ceilingpicnum =
                                    sector[ps[0].one_parallax_sectnum].ceilingpicnum;
                                sc->ceilingshade  =
                                    sector[ps[0].one_parallax_sectnum].ceilingshade;
                            }
                        }
                    }
                    t[2]++;
                    if(t[2] > 256)
                        KILLIT(i);
                }


                if( t[2] == 4 && s->ang != 512)
                    for(x=0;x<7;x++) RANDOMSCRAP;
                break;


            case 15:

                if(t[4])
                {
                    s->xvel = 16;

                    if(t[4] == 1) //Opening
                    {
                        if( t[3] >= (SP>>3) )
                        {
                            t[4] = 0; //Turn off the sliders
                            callsound(s->sectnum,i);
                            break;
                        }
                        t[3]++;
                    }
                    else if(t[4] == 2)
                    {
                        if(t[3]<1)
                        {
                            t[4] = 0;
                            callsound(s->sectnum,i);
                            break;
                        }
                        t[3]--;
                    }

                    ms(i);
                    setsprite(i,s->x,s->y,s->z);
                }
                break;

            case 16: //Reactor

                t[2]+=32;
                if(sc->floorz<sc->ceilingz) s->shade=0;

                else if( sc->ceilingz < t[3] )
                {

                    //The following code check to see if
                    //there is any other sprites in the sector.
                    //If there isn't, then kill this sectoreffector
                    //itself.....

                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        if(sprite[j].picnum == REACTOR || sprite[j].picnum == REACTOR2)
                            break;
                        j = nextspritesect[j];
                    }
                    if(j == -1) { KILLIT(i); }
                    else s->shade=1;
                }

                if(s->shade) sc->ceilingz+=1024;
                else sc->ceilingz-=512;

                ms(i);
                setsprite(i,s->x,s->y,s->z);

                break;

            case 17:

                q = t[0]*(SP<<2);

                sc->ceilingz += q;
                sc->floorz += q;

                j = headspritesect[s->sectnum];
                while(j >= 0)
                {
                    if(sprite[j].statnum == 10 && sprite[j].owner >= 0)
                    {
                        p = sprite[j].yvel;
                        if(numplayers < 2)
                            ps[p].oposz = ps[p].posz;
                        ps[p].posz += q;
                        ps[p].truefz += q;
                        ps[p].truecz += q;
                        if(numplayers > 1)
                            ps[p].oposz = ps[p].posz;
                    }
                    if( sprite[j].statnum != 3 )
                    {
                        hittype[j].bposz = sprite[j].z;
                        sprite[j].z += q;
                    }

                    hittype[j].floorz = sc->floorz;
                    hittype[j].ceilingz = sc->ceilingz;

                    j = nextspritesect[j];
                }

                if( t[0] )                if(t[0]) //If in motion
                {
                    if( klabs(sc->floorz-t[2]) <= SP)
                    {
                        activatewarpelevators(i,0);
                        break;
                    }

                    if(t[0]==-1)
                    {
                        if( sc->floorz > t[3] )
                            break;
                    }
                    else if( sc->ceilingz < t[4] ) break;

                    if( t[1] == 0 ) break;
                    t[1] = 0;

                    j = headspritestat[3];
                    while(j >= 0)
                    {
                                if( i != j && (sprite[j].lotag) == 17)
                                    if( (sc->hitag-t[0]) ==
                                        (sector[sprite[j].sectnum].hitag)
                                        && sh == (sprite[j].hitag))
                                            break;
                                j = nextspritestat[j];
                    }

                    if(j == -1) break;

                    k = headspritesect[s->sectnum];
                    while(k >= 0)
                    {
                        nextk = nextspritesect[k];

                        if(sprite[k].statnum == 10 && sprite[k].owner >= 0)
                        {
                            p = sprite[k].yvel;

                            ps[p].posx += sprite[j].x-s->x;
                            ps[p].posy += sprite[j].y-s->y;
                            ps[p].posz = sector[sprite[j].sectnum].floorz-(sc->floorz-ps[p].posz);

                            hittype[k].floorz = sector[sprite[j].sectnum].floorz;
                            hittype[k].ceilingz = sector[sprite[j].sectnum].ceilingz;

                            ps[p].bobposx = ps[p].oposx = ps[p].posx;
                            ps[p].bobposy = ps[p].oposy = ps[p].posy;
                            ps[p].oposz = ps[p].posz;

                            ps[p].truefz = hittype[k].floorz;
                            ps[p].truecz = hittype[k].ceilingz;
                            ps[p].bobcounter = 0;

                            changespritesect(k,sprite[j].sectnum);
                            ps[p].cursectnum = sprite[j].sectnum;
                        }
                        else if( sprite[k].statnum != 3 )
                        {
                            sprite[k].x +=
                                sprite[j].x-s->x;
                            sprite[k].y +=
                                sprite[j].y-s->y;
                            sprite[k].z = sector[sprite[j].sectnum].floorz-
                                (sc->floorz-sprite[k].z);

                            hittype[k].bposx = sprite[k].x;
                            hittype[k].bposy = sprite[k].y;
                            hittype[k].bposz = sprite[k].z;

                            changespritesect(k,sprite[j].sectnum);
                            setsprite(k,sprite[k].x,sprite[k].y,sprite[k].z);

                            hittype[k].floorz = sector[sprite[j].sectnum].floorz;
                            hittype[k].ceilingz = sector[sprite[j].sectnum].ceilingz;

                        }
                        k = nextk;
                    }
                }
                break;

            case 18:
                if(t[0])
                {
                    if(s->pal)
                    {
                        if(s->ang == 512)
                        {
                            sc->ceilingz -= sc->extra;
                            if(sc->ceilingz <= t[1])
                            {
                                sc->ceilingz = t[1];
                                KILLIT(i);
                            }
                        }
                        else
                        {
                            sc->floorz += sc->extra;
                                j = headspritesect[s->sectnum];
                                while(j >= 0)
                                {
                                    if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                        if( ps[sprite[j].yvel].on_ground == 1 )
                                            ps[sprite[j].yvel].posz += sc->extra;
                                    if( sprite[j].zvel == 0 && sprite[j].statnum != 3 && sprite[j].statnum != 4)
                                    {
                                        hittype[j].bposz = sprite[j].z += sc->extra;
                                        hittype[j].floorz = sc->floorz;
                                    }
                                    j = nextspritesect[j];
                                }
                            if(sc->floorz >= t[1])
                            {
                                sc->floorz = t[1];
                                KILLIT(i);
                            }
                        }
                    }
                    else
                    {
                        if(s->ang == 512)
                        {
                            sc->ceilingz += sc->extra;
                            if(sc->ceilingz >= s->z)
                            {
                                sc->ceilingz = s->z;
                                KILLIT(i);
                            }
                        }
                        else
                        {
                            sc->floorz -= sc->extra;
                                j = headspritesect[s->sectnum];
                                while(j >= 0)
                                {
                                    if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                        if( ps[sprite[j].yvel].on_ground == 1 )
                                            ps[sprite[j].yvel].posz -= sc->extra;
                                    if( sprite[j].zvel == 0 && sprite[j].statnum != 3 && sprite[j].statnum != 4)
                                    {
                                        hittype[j].bposz = sprite[j].z -= sc->extra;
                                        hittype[j].floorz = sc->floorz;
                                    }
                                    j = nextspritesect[j];
                                }
                            if(sc->floorz <= s->z)
                            {
                                sc->floorz = s->z;
                                KILLIT(i);
                            }
                        }
                    }

                    t[2]++;
                    if(t[2] >= s->hitag)
                    {
                        t[2] = 0;
                        t[0] = 0;
                    }
                }
                break;

            case 19: //Battlestar galactia shields

                if(t[0])
                {
                    if(t[0] == 1)
                    {
                        t[0]++;
                        x = sc->wallptr;
                        q = x+sc->wallnum;
                        for(j=x;j<q;j++)
                            if(wall[j].overpicnum == BIGFORCE)
                            {
                                wall[j].cstat &= (128+32+8+4+2);
                                wall[j].overpicnum = 0;
                                if(wall[j].nextwall >= 0)
                                {
                                    wall[wall[j].nextwall].overpicnum = 0;
                                    wall[wall[j].nextwall].cstat &= (128+32+8+4+2);
                                }
                            }
                    }

                    if(sc->ceilingz < sc->floorz)
                        sc->ceilingz += SP;
                    else
                    {
                        sc->ceilingz = sc->floorz;

                        j = headspritestat[3];
                        while(j >= 0)
                        {
                            if(sprite[j].lotag == 0 && sprite[j].hitag==sh)
                            {
                                q = sprite[sprite[j].owner].sectnum;
                                sector[sprite[j].sectnum].floorpal = sector[sprite[j].sectnum].ceilingpal =
                                        sector[q].floorpal;
                                sector[sprite[j].sectnum].floorshade = sector[sprite[j].sectnum].ceilingshade =
                                    sector[q].floorshade;

                                hittype[sprite[j].owner].temp_data[0] = 2;
                            }
                            j = nextspritestat[j];
                        }
                        KILLIT(i);
                    }
                }
                else //Not hit yet
                {
                    IFHITSECT
                    {
                        FTA(8,&ps[myconnectindex],0);

                        l = headspritestat[3];
                        while(l >= 0)
                        {
                            x = sprite[l].lotag&0x7fff;
                            switch( x )
                            {
                                case 0:
                                    if(sprite[l].hitag == sh)
                                    {
                                        q = sprite[l].sectnum;
                                        sector[q].floorshade =
                                            sector[q].ceilingshade =
                                                sprite[sprite[l].owner].shade;
                                        sector[q].floorpal =
                                            sector[q].ceilingpal =
                                                sprite[sprite[l].owner].pal;
                                    }
                                    break;

                                case 1:
                                case 12:
//                                case 18:
                                case 19:

                                    if( sh == sprite[l].hitag )
                                        if( hittype[l].temp_data[0] == 0 )
                                        {
                                            hittype[l].temp_data[0] = 1; //Shut them all on
                                            sprite[l].owner = i;
                                        }

                                    break;
                            }
                            l = nextspritestat[l];
                        }
                    }
                }

                break;

            case 20: //Extend-o-bridge

                if( t[0] == 0 ) break;
                if( t[0] == 1 ) s->xvel = 8;
                else s->xvel = -8;

                if( s->xvel ) //Moving
                {
                    x = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                    l = (s->xvel*sintable[s->ang&2047])>>14;

                    t[3] += s->xvel;

                    s->x += x;
                    s->y += l;

                    if( t[3] <= 0 || (t[3]>>6) >= (SP>>6) )
                    {
                        s->x -= x;
                        s->y -= l;
                        t[0] = 0;
                        callsound(s->sectnum,i);
                        break;
                    }

                    j = headspritesect[s->sectnum];
                    while(j >= 0)
                    {
                        nextj = nextspritesect[j];

                        if( sprite[j].statnum != 3 && sprite[j].zvel == 0)
                        {
                            sprite[j].x += x;
                            sprite[j].y += l;
                            setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
                            if( sector[sprite[j].sectnum].floorstat&2 )
                                if(sprite[j].statnum == 2)
                                    makeitfall(j);
                        }
                        j = nextj;
                    }

                    dragpoint((short)t[1],wall[t[1]].x+x,wall[t[1]].y+l);
                    dragpoint((short)t[2],wall[t[2]].x+x,wall[t[2]].y+l);

                    for(p=connecthead;p>=0;p=connectpoint2[p])
                        if(ps[p].cursectnum == s->sectnum && ps[p].on_ground)
                        {
                            ps[p].posx += x;
                            ps[p].posy += l;

                            ps[p].oposx = ps[p].posx;
                            ps[p].oposy = ps[p].posy;

                            setsprite(ps[p].i,ps[p].posx,ps[p].posy,ps[p].posz+PHEIGHT);
                        }

                    sc->floorxpanning-=x>>3;
                    sc->floorypanning-=l>>3;

                    sc->ceilingxpanning-=x>>3;
                    sc->ceilingypanning-=l>>3;
                }

                break;

            case 21: // Cascading effect

                if( t[0] == 0 ) break;

                if( s->ang == 1536 )
                    l = (int32_t) &sc->ceilingz;
                else
                    l = (int32_t) &sc->floorz;

                if( t[0] == 1 ) //Decide if the s->sectnum should go up or down
                {
                    s->zvel = ksgn(s->z-*(int32_t *)l) * (SP<<4);
                    t[0]++;
                }

                if( sc->extra == 0 )
                {
                    *(int32_t *)l += s->zvel;

                    if(klabs(*(int32_t *)l-s->z) < 1024)
                    {
                        *(int32_t *)l = s->z;
                        KILLIT(i); //All done
                    }
                }
                else sc->extra--;
                break;

            case 22:

                if( t[1] )
                {
                    if(getanimationgoal(&sector[t[0]].ceilingz) >= 0)
                        sc->ceilingz += sc->extra*9;
                    else t[1] = 0;
                }
                break;

            case 24:
            case 34:

                if(t[4]) break;

                x = (SP*sintable[(s->ang+512)&2047])>>18;
                l = (SP*sintable[s->ang&2047])>>18;

                k = 0;

                j = headspritesect[s->sectnum];
                while(j >= 0)
                {
                    nextj = nextspritesect[j];
                    if(sprite[j].zvel >= 0)
                        switch(sprite[j].statnum)
                    {
                        case 5:
                            switch(sprite[j].picnum)
                            {
                                case BLOODPOOL:
                                case PUKE:
                                case FOOTPRINTS:
                                case FOOTPRINTS2:
                                case FOOTPRINTS3:
                                case FOOTPRINTS4:
                                case BULLETHOLE:
                                case BLOODSPLAT1:
                                case BLOODSPLAT2:
                                case BLOODSPLAT3:
                                case BLOODSPLAT4:
                                    sprite[j].xrepeat = sprite[j].yrepeat = 0;
                                    j = nextj;
                                    continue;
                                case LASERLINE:
                                    j = nextj;
                                    continue;
                            }
                        case 6:
                            if(sprite[j].picnum == TRIPBOMB) break;
                        case 1:
                        case 0:
                            if(
                                sprite[j].picnum == BOLT1 ||
                                sprite[j].picnum == BOLT1+1 ||
                                sprite[j].picnum == BOLT1+2 ||
                                sprite[j].picnum == BOLT1+3 ||
                                sprite[j].picnum == SIDEBOLT1 ||
                                sprite[j].picnum == SIDEBOLT1+1 ||
                                sprite[j].picnum == SIDEBOLT1+2 ||
                                sprite[j].picnum == SIDEBOLT1+3 ||
                                wallswitchcheck(j)
                              )
                              break;

                            if( !(sprite[j].picnum >= CRANE && sprite[j].picnum <= (CRANE+3)))
                            {
                                if( sprite[j].z > (hittype[j].floorz-(16<<8)) )
                                {
                                    hittype[j].bposx = sprite[j].x;
                                    hittype[j].bposy = sprite[j].y;

                                    sprite[j].x += x>>2;
                                    sprite[j].y += l>>2;

                                    setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);

                                    if( sector[sprite[j].sectnum].floorstat&2 )
                                        if(sprite[j].statnum == 2)
                                            makeitfall(j);
                                }
                            }
                            break;
                    }
                    j = nextj;
                }

                p = myconnectindex;
                if(ps[p].cursectnum == s->sectnum && ps[p].on_ground)
                    if( klabs(ps[p].posz-ps[p].truefz) < PHEIGHT+(9<<8) )
                {
                    fricxv += x<<3;
                    fricyv += l<<3;
                }

                sc->floorxpanning += SP>>7;

            break;

            case 35:
                if(sc->ceilingz > s->z)
                    for(j = 0;j < 8;j++)
                {
                    s->ang += TRAND&511;
                    k = spawn(i,SMALLSMOKE);
                    sprite[k].xvel = 96+(TRAND&127);
                    ssp(k,CLIPMASK0);
                    setsprite(k,sprite[k].x,sprite[k].y,sprite[k].z);
                    if( rnd(16) )
                        spawn(i,EXPLOSION2);
                }

                switch(t[0])
                {
                    case 0:
                        sc->ceilingz += s->yvel;
                        if(sc->ceilingz > sc->floorz)
                            sc->floorz = sc->ceilingz;
                        if(sc->ceilingz > s->z+(32<<8))
                            t[0]++;
                        break;
                    case 1:
                        sc->ceilingz-=(s->yvel<<2);
                        if(sc->ceilingz < t[4])
                        {
                            sc->ceilingz = t[4];
                            t[0] = 0;
                        }
                        break;
                }
                break;

            case 25: //PISTONS

                if( t[4] == 0 ) break;

                if(sc->floorz <= sc->ceilingz)
                    s->shade = 0;
                else if( sc->ceilingz <= t[3])
                    s->shade = 1;

                if(s->shade)
                {
                    sc->ceilingz += SP<<4;
                    if(sc->ceilingz > sc->floorz)
                        sc->ceilingz = sc->floorz;
                }
                else
                {
                    sc->ceilingz   -= SP<<4;
                    if(sc->ceilingz < t[3])
                        sc->ceilingz = t[3];
                }

                break;

            case 26:

                s->xvel = 32;
                l = (s->xvel*sintable[(s->ang+512)&2047])>>14;
                x = (s->xvel*sintable[s->ang&2047])>>14;

                s->shade++;
                if( s->shade > 7 )
                {
                    s->x = t[3];
                    s->y = t[4];
                    sc->floorz -= ((s->zvel*s->shade)-s->zvel);
                    s->shade = 0;
                }
                else
                    sc->floorz += s->zvel;

                j = headspritesect[s->sectnum];
                while( j >= 0 )
                {
                    nextj = nextspritesect[j];
                    if(sprite[j].statnum != 3 && sprite[j].statnum != 10)
                    {
                        hittype[j].bposx = sprite[j].x;
                        hittype[j].bposy = sprite[j].y;

                        sprite[j].x += l;
                        sprite[j].y += x;

                        sprite[j].z += s->zvel;
                        setsprite(j,sprite[j].x,sprite[j].y,sprite[j].z);
                    }
                    j = nextj;
                }

                p = myconnectindex;
                if(sprite[ps[p].i].sectnum == s->sectnum && ps[p].on_ground)
                {
                    fricxv += l<<5;
                    fricyv += x<<5;
                }

                for(p = connecthead;p >= 0;p = connectpoint2[p])
                    if(sprite[ps[p].i].sectnum == s->sectnum && ps[p].on_ground)
                        ps[p].posz += s->zvel;

                ms(i);
                setsprite(i,s->x,s->y,s->z);

                break;


            case 27:

                if(ud.recstat == 0) break;

                hittype[i].tempang = s->ang;

                p = findplayer(s,&x);

				// FIX_00013: 3rd person camera view during demo playback can now be 
				// turned off (no need to use hacked no-camera maps anymore)                
                if( /*sprite[ps[p].i].extra > 0 && myconnectindex == screenpeek */ 0) // xduke remove camera view for the 1st player (notice myconnectindex is == 1 when replaying, dosent matter who recorded)
                {
                    if( t[0] < 0 )
                    {
                        ud.camerasprite = i;
                        t[0]++;
                    }
                    else if(ud.recstat == 2 && ps[p].newowner == -1)
                    {
                        if(cansee(s->x,s->y,s->z,SECT,ps[p].posx,ps[p].posy,ps[p].posz,ps[p].cursectnum))
                        {
                            if(x < (uint32_t)sh)
                            {
                                ud.camerasprite = i;
                                t[0] = 999;
                                s->ang += getincangle(s->ang,getangle(ps[p].posx-s->x,ps[p].posy-s->y))>>3;
                                SP = 100+((s->z-ps[p].posz)/257);

                            }
                            else if(t[0] == 999)
                            {
                                if(ud.camerasprite == i)
                                    t[0] = 0;
                                else t[0] = -10;
                                ud.camerasprite = i;

                            }
                        }
                        else
                        {
                            s->ang = getangle(ps[p].posx-s->x,ps[p].posy-s->y);

                            if(t[0] == 999)
                            {
                                if(ud.camerasprite == i)
                                    t[0] = 0;
                                else t[0] = -20;
                                ud.camerasprite = i;
                            }
                        }
                    }
                }
                break;
            case 28:
                if(t[5] > 0)
                {
                    t[5]--;
                    break;
                }

                if(T1 == 0)
                {
                    p = findplayer(s,&x);
                    if( x > 15500 )
                        break;
                    T1 = 1;
                    T2 = 64 + (TRAND&511);
                    T3 = 0;
                }
                else
                {
                    T3++;
                    if(T3 > T2)
                    {
                        T1 = 0;
                        ps[screenpeek].visibility = ud.const_visibility;
                        break;
                    }
                    else if( T3 == (T2>>1) )
                        spritesound(THUNDER,i);
                    else if(T3 == (T2>>3) )
                        spritesound(LIGHTNING_SLAP,i);
                    else if( T3 == (T2>>2) )
                    {
                        j = headspritestat[0];
                        while(j >= 0)
                        {
                            if( sprite[j].picnum == NATURALLIGHTNING && sprite[j].hitag == s->hitag)
                                sprite[j].cstat |= 32768;
                            j = nextspritestat[j];
                        }
                    }
                    else if(T3 > (T2>>3) && T3 < (T2>>2) )
                    {
                        if( cansee(s->x,s->y,s->z,s->sectnum,ps[screenpeek].posx,ps[screenpeek].posy,ps[screenpeek].posz,ps[screenpeek].cursectnum ) )
                            j = 1;
                        else j = 0;

                        if( rnd(192) && (T3&1) )
                        {
                            if(j)
                                ps[screenpeek].visibility = 0;
                        }
                        else if(j)
                            ps[screenpeek].visibility = ud.const_visibility;

                        j = headspritestat[0];
                        while(j >= 0)
                        {
                            if( sprite[j].picnum == NATURALLIGHTNING && sprite[j].hitag == s->hitag)
                            {
                                if ( rnd(32) && (T3&1) )
                                {
                                    sprite[j].cstat &= 32767;
                                    spawn(j,SMALLSMOKE);

                                    p = findplayer(s,&x);
                                    x = ldist(&sprite[ps[p].i], &sprite[j]);
                                    if( x < 768 )
                                    {
                                        if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                            spritesound(DUKE_LONGTERM_PAIN,ps[p].i);
                                        spritesound(SHORT_CIRCUIT,ps[p].i);
                                        sprite[ps[p].i].extra -= 8+(TRAND&7);
                                        ps[p].pals_time = 32;
                                        ps[p].pals[0] = 16;
                                        ps[p].pals[1] = 0;
                                        ps[p].pals[2] = 0;
                                    }
                                    break;
                                }
                                else sprite[j].cstat |= 32768;
                            }

                            j = nextspritestat[j];
                        }
                    }
                }
                break;
            case 29:
                s->hitag += 64;
                l = mulscale12((int32_t)s->yvel,sintable[s->hitag&2047]);
                sc->floorz = s->z + l;
                break;
            case 31: // True Drop Floor
                if(t[0] == 1)
                {
                    // Choose dir

                    if(t[3] > 0)
                    {
                        t[3]--;
                        break;
                    }

                    if(t[2] == 1) // Retract
                    {
                        if(SA != 1536)
                        {
                            if( klabs( sc->floorz - s->z ) < SP )
                            {
                                sc->floorz = s->z;
                                t[2] = 0;
                                t[0] = 0;
                                t[3] = s->hitag;
                                callsound(s->sectnum,i);
                            }
                            else
                            {
                                l = sgn(s->z-sc->floorz)*SP;
                                sc->floorz += l;

                                j = headspritesect[s->sectnum];
                                while(j >= 0)
                                {
                                    if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                        if( ps[sprite[j].yvel].on_ground == 1 )
                                            ps[sprite[j].yvel].posz += l;
                                    if( sprite[j].zvel == 0 && sprite[j].statnum != 3 && sprite[j].statnum != 4)
                                    {
                                        hittype[j].bposz = sprite[j].z += l;
                                        hittype[j].floorz = sc->floorz;
                                    }
                                    j = nextspritesect[j];
                                }
                            }
                        }
                        else
                        {
                            if( klabs( sc->floorz - t[1] ) < SP )
                            {
                                sc->floorz = t[1];
                                callsound(s->sectnum,i);
                                t[2] = 0;
                                t[0] = 0;
                                t[3] = s->hitag;
                            }
                            else
                            {
                                l = sgn(t[1]-sc->floorz)*SP;
                                sc->floorz += l;

                                j = headspritesect[s->sectnum];
                                while(j >= 0)
                                {
                                    if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                        if( ps[sprite[j].yvel].on_ground == 1 )
                                            ps[sprite[j].yvel].posz += l;
                                    if( sprite[j].zvel == 0 && sprite[j].statnum != 3 && sprite[j].statnum != 4 )
                                    {
                                        hittype[j].bposz = sprite[j].z += l;
                                        hittype[j].floorz = sc->floorz;
                                    }
                                    j = nextspritesect[j];
                                }
                            }
                        }
                        break;
                    }

                    if( (s->ang&2047) == 1536)
                    {
                        if( klabs( s->z-sc->floorz ) < SP )
                        {
                            callsound(s->sectnum,i);
                            t[0] = 0;
                            t[2] = 1;
                            t[3] = s->hitag;
                        }
                        else
                        {
                            l = sgn(s->z-sc->floorz)*SP;
                            sc->floorz += l;

                            j = headspritesect[s->sectnum];
                            while(j >= 0)
                            {
                                if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                    if( ps[sprite[j].yvel].on_ground == 1 )
                                        ps[sprite[j].yvel].posz += l;
                                if( sprite[j].zvel == 0 && sprite[j].statnum != 3 && sprite[j].statnum != 4 )
                                {
                                    hittype[j].bposz = sprite[j].z += l;
                                    hittype[j].floorz = sc->floorz;
                                }
                                j = nextspritesect[j];
                            }
                        }
                    }
                    else
                    {
                        if( klabs( sc->floorz-t[1] ) < SP )
                        {
                            t[0] = 0;
                            callsound(s->sectnum,i);
                            t[2] = 1;
                            t[3] = s->hitag;
                        }
                        else
                        {
                            l = sgn(s->z-t[1])*SP;
                            sc->floorz -= l;

                            j = headspritesect[s->sectnum];
                            while(j >= 0)
                            {
                                if(sprite[j].picnum == APLAYER && sprite[j].owner >= 0)
                                    if( ps[sprite[j].yvel].on_ground == 1 )
                                        ps[sprite[j].yvel].posz -= l;
                                if(sprite[j].zvel == 0 && sprite[j].statnum != 3 && sprite[j].statnum != 4 )
                                {
                                    hittype[j].bposz = sprite[j].z -= l;
                                    hittype[j].floorz = sc->floorz;
                                }
                                j = nextspritesect[j];
                            }
                        }
                    }
                }
                break;

           case 32: // True Drop Ceiling
                if(t[0] == 1)
                {
                    // Choose dir

                    if(t[2] == 1) // Retract
                    {
                        if(SA != 1536)
                        {
                            if( klabs( sc->ceilingz - s->z ) <
                                (SP<<1) )
                            {
                                sc->ceilingz = s->z;
                                callsound(s->sectnum,i);
                                t[2] = 0;
                                t[0] = 0;
                            }
                            else sc->ceilingz +=
                                sgn(s->z-sc->ceilingz)*SP;
                        }
                        else
                        {
                            if( klabs( sc->ceilingz - t[1] ) <
                                (SP<<1) )
                            {
                                sc->ceilingz = t[1];
                                callsound(s->sectnum,i);
                                t[2] = 0;
                                t[0] = 0;
                            }
                            else sc->ceilingz +=
                                sgn(t[1]-sc->ceilingz)*SP;
                        }
                        break;
                    }

                    if( (s->ang&2047) == 1536)
                    {
                        if( klabs(sc->ceilingz-s->z ) <
                            (SP<<1) )
                        {
                            t[0] = 0;
                            t[2] = !t[2];
                            callsound(s->sectnum,i);
                            sc->ceilingz = s->z;
                        }
                        else sc->ceilingz +=
                            sgn(s->z-sc->ceilingz)*SP;
                    }
                    else
                    {
                        if( klabs(sc->ceilingz-t[1] ) < (SP<<1) )
                        {
                            t[0] = 0;
                            t[2] = !t[2];
                            callsound(s->sectnum,i);
                        }
                        else sc->ceilingz -= sgn(s->z-t[1])*SP;
                    }
                }
                break;

            case 33:
                if( earthquaketime > 0 && (TRAND&7) == 0 )
                    RANDOMSCRAP;
                break;
            case 36:

                if( t[0] )
                {
                    if( t[0] == 1 )
                        shoot(i,sc->extra);
                    else if( t[0] == 26*5 )
                        t[0] = 0;
                    t[0]++;
                }
                break;

            case 128: //SE to control glass breakage

                wal = &wall[t[2]];

                if(wal->cstat|32)
                {
                    wal->cstat &= (255-32);
                    wal->cstat |= 16;
                    if(wal->nextwall >= 0)
                    {
                        wall[wal->nextwall].cstat &= (255-32);
                        wall[wal->nextwall].cstat |= 16;
                    }
                }
                else break;

                wal->overpicnum++;
                if(wal->nextwall >= 0)
                    wall[wal->nextwall].overpicnum++;

                if(t[0] < t[1]) t[0]++;
                else
                {
                    wal->cstat &= (128+32+8+4+2);
                    if(wal->nextwall >= 0)
                        wall[wal->nextwall].cstat &= (128+32+8+4+2);
                    KILLIT(i);
                }
                break;

            case 130:
                if(t[0] > 80) { KILLIT(i); }
                else t[0]++;

                x = sc->floorz-sc->ceilingz;

                if( rnd(64) )
                {
                    k = spawn(i,EXPLOSION2);
                    sprite[k].xrepeat = sprite[k].yrepeat = 2+(TRAND&7);
                    sprite[k].z = sc->floorz-(TRAND%x);
                    sprite[k].ang += 256-(TRAND%511);
                    sprite[k].xvel = TRAND&127;
                    ssp(k,CLIPMASK0);
                }
                break;
            case 131:
                if(t[0] > 40) { KILLIT(i); }
                else t[0]++;

                x = sc->floorz-sc->ceilingz;

                if( rnd(32) )
                {
                    k = spawn(i,EXPLOSION2);
                    sprite[k].xrepeat = sprite[k].yrepeat = 2+(TRAND&3);
                    sprite[k].z = sc->floorz-(TRAND%x);
                    sprite[k].ang += 256-(TRAND%511);
                    sprite[k].xvel = TRAND&127;
                    ssp(k,CLIPMASK0);
                }
                break;
        }
        BOLT:
        i = nexti;
    }

         //Sloped sin-wave floors!
     for(i=headspritestat[3];i>=0;i=nextspritestat[i])
     {
          s = &sprite[i];
          if (s->lotag != 29) continue;
          sc = &sector[s->sectnum];
          if (sc->wallnum != 4) continue;
          wal = &wall[sc->wallptr+2];
          alignflorslope(s->sectnum,wal->x,wal->y,sector[wal->nextsector].floorz);
     }
}

