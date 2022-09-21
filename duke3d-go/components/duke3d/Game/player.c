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

// Savage Baggage Masters

#include "duke3d.h"

int32 turnheldtime; //MED
int32 lastcontroltime; //MED

void setpal(struct player_struct *p)
{
    if(p->heat_on) p->palette = slimepal;
    else switch(sector[p->cursectnum].ceilingpicnum)
    {
        case FLOORSLIME:
        case FLOORSLIME+1:
        case FLOORSLIME+2:
            p->palette = slimepal;
            break;
        default:
            if(sector[p->cursectnum].lotag == 2) p->palette = waterpal;
            else p->palette = palette;
            break;
    }
    restorepalette = 1;
}

void incur_damage( struct player_struct *p )
{
    int32_t  damage = 0L, shield_damage = 0L;

    sprite[p->i].extra -= p->extra_extra8>>8;

    damage = sprite[p->i].extra - p->last_extra;

    if ( damage < 0 )
    {
        p->extra_extra8 = 0;

        if ( p->shield_amount > 0 )
        {
            shield_damage =  damage * (20 + (TRAND%30)) / 100;
            damage -= shield_damage;

            p->shield_amount += shield_damage;

            if ( p->shield_amount < 0 )
            {
                damage += p->shield_amount;
                p->shield_amount = 0;
            }
        }

        sprite[p->i].extra = p->last_extra + damage;
    }
}

void quickkill(struct player_struct *p)
{
    p->pals[0] = 48;
    p->pals[1] = 48;
    p->pals[2] = 48;
    p->pals_time = 48;

    sprite[p->i].extra = 0;
    sprite[p->i].cstat |= 32768;
    if(ud.god == 0) guts(&sprite[p->i],JIBS6,8,myconnectindex);
    return;
}

void forceplayerangle(struct player_struct *p)
{
    short n;

    n = 128-(TRAND&255);

    p->horiz += 64;
    p->return_to_center = 9;
    p->look_ang = n>>1;
    p->rotscrnang = n>>1;
}

void tracers(int32_t x1,int32_t y1,int32_t z1,int32_t x2,int32_t y2,int32_t z2,int32_t n)
{
     int32_t i, xv, yv, zv;
     short sect = -1;

	 i = n+1;
	 xv = (x2-x1)/i;
	 yv = (y2-y1)/i;
	 zv = (z2-z1)/i;

     if( ( klabs(x1-x2)+klabs(y1-y2) ) < 3084 )
         return;

	 for(i=n;i>0;i--)
	 {
		  x1 += xv;
		  y1 += yv;
		  z1 += zv;
		  updatesector(x1,y1,&sect);
          if(sect >= 0)
          {
              if(sector[sect].lotag == 2)
                  EGS(sect,x1,y1,z1,WATERBUBBLE,-32,4+(TRAND&3),4+(TRAND&3),TRAND&2047,0,0,ps[0].i,5);
              else
                  EGS(sect,x1,y1,z1,SMALLSMOKE,-32,14,14,0,0,0,ps[0].i,5);
          }
	 }
}

int32_t hits(short i)
{
    int32_t sx,sy,sz;
    short sect,hw,hs;
    int32_t zoff;

    if(PN == APLAYER) zoff = (40<<8);
    else zoff = 0;

    hitscan(SX,SY,SZ-zoff,SECT,
        sintable[(SA+512)&2047],
        sintable[SA&2047],
        0,&sect,&hw,&hs,&sx,&sy,&sz,CLIPMASK1);

    return ( FindDistance2D( sx-SX,sy-SY ) );
}

int32_t hitasprite(short i,short *hitsp)
{
    int32_t sx,sy,sz,zoff;
    short sect,hw;

    if(badguy(&sprite[i]) )
        zoff = (42<<8);
    else if(PN == APLAYER) zoff = (39<<8);
    else zoff = 0;

    hitscan(SX,SY,SZ-zoff,SECT,
        sintable[(SA+512)&2047],
        sintable[SA&2047],
        0,&sect,&hw,hitsp,&sx,&sy,&sz,CLIPMASK1);

    if(hw >= 0 && (wall[hw].cstat&16) && badguy(&sprite[i]) )
        return((1<<30));

    return ( FindDistance2D(sx-SX,sy-SY) );
}

/*
int32_t hitaspriteandwall(short i,short *hitsp,short *hitw,short *x, short *y)
{
    int32_t sz;
    short sect;

    hitscan(SX,SY,SZ,SECT,
        sintable[(SA+512)&2047],
        sintable[SA&2047],
        0,&sect,hitw,hitsp,x,y,&sz,CLIPMASK1);

    return ( FindDistance2D(*x-SX,*y-SY) );
}
*/


int32_t hitawall(struct player_struct *p,short *hitw)
{
    int32_t sx,sy,sz;
    short sect,hs;

    hitscan(p->posx,p->posy,p->posz,p->cursectnum,
        sintable[(p->ang+512)&2047],
        sintable[p->ang&2047],
        0,&sect,hitw,&hs,&sx,&sy,&sz,CLIPMASK0);

    return ( FindDistance2D(sx-p->posx,sy-p->posy) );
}

short aim(spritetype *s,short aang, short auto_aim)
{
    uint8_t  gotshrinker,gotfreezer;
    short i, j, a, k, cans;
    short aimstats[] = {10,13,1,2};
    int32_t dx1, dy1, dx2, dy2, dx3, dy3, smax, sdist;
    int32_t xv, yv;

    a = s->ang;

    j = -1;
//    if(s->picnum == APLAYER && ps[s->yvel].aim_mode) return -1;

    gotshrinker = s->picnum == APLAYER && ps[s->yvel].curr_weapon == SHRINKER_WEAPON;
    gotfreezer = s->picnum == APLAYER && ps[s->yvel].curr_weapon == FREEZE_WEAPON;

    smax = 0x7fffffff;

    dx1 = sintable[(a+512-aang)&2047];
    dy1 = sintable[(a-aang)&2047];
    dx2 = sintable[(a+512+aang)&2047];
    dy2 = sintable[(a+aang)&2047];

    dx3 = sintable[(a+512)&2047];
    dy3 = sintable[a&2047];

	// FIX_00015: Backward compliance with older demos (down to demos v27, 28, 116 and 117 only)

	//if player has AutoAim ON do the function.
	if((auto_aim && nHostForceDisableAutoaim == 0)||
		ud.playing_demo_rev == BYTEVERSION_27     ||
		ud.playing_demo_rev == BYTEVERSION_28     || 
		ud.playing_demo_rev == BYTEVERSION_116    || 
		ud.playing_demo_rev == BYTEVERSION_117) 
		// don't disable any autoaim in case we are playing an old demo
	{
	    for(k=0;k<4;k++)
	    {
			if( j >= 0 )
	            break;
			for(i=headspritestat[aimstats[k]];i >= 0;i=nextspritestat[i])
	            if( sprite[i].xrepeat > 0 && sprite[i].extra >= 0 && (sprite[i].cstat&(257+32768)) == 257)
					if( badguy(&sprite[i]) || k < 2 )
				{
	                if(badguy(&sprite[i]) || PN == APLAYER || PN == SHARK)
					{
	                    if( PN == APLAYER &&
//							  ud.ffire == 0 &&
							ud.coop == 1 &&
							s->picnum == APLAYER &&
							s != &sprite[i])
	                            continue;
	
						if(gotshrinker && sprite[i].xrepeat < 30 )
						{
	                        switch(PN)
							{
	                            case SHARK:
									if(sprite[i].xrepeat < 20) continue;
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
	                                continue;
							}
						}
						if(gotfreezer && sprite[i].pal == 1) continue;
					}
	
					xv = (SX-s->x);
					yv = (SY-s->y);
	
					if( (dy1*xv) <= (dx1*yv) )
	                    if( ( dy2*xv ) >= (dx2*yv) )
					{
	                    sdist = mulscale(dx3,xv,14) + mulscale(dy3,yv,14);
						if( sdist > 512 && sdist < smax )
						{
	                        if(s->picnum == APLAYER)
								a = (klabs(scale(SZ-s->z,10,sdist)-(ps[s->yvel].horiz+ps[s->yvel].horizoff-100)) < 100);
							else a = 1;
	
							if(PN == ORGANTIC || PN == ROTATEGUN )
	                            cans = cansee(SX,SY,SZ,SECT,s->x,s->y,s->z-(32<<8),s->sectnum);
							else cans = cansee(SX,SY,SZ-(32<<8),SECT,s->x,s->y,s->z-(32<<8),s->sectnum);
	
							if( a && cans )
							{
	                            smax = sdist;
								j = i;
							}
						}
					}
				}
		}
	}

    return j;
}




void shoot(short i,short atwith)
{
    short sect, hitsect, hitspr, hitwall, l, sa, p, j, k, scount;
    int32_t sx, sy, sz, vel, zvel, hitx, hity, hitz, x, oldzvel, dal;
    uint8_t  sizx,sizy;
    spritetype *s;

    s = &sprite[i];
    sect = s->sectnum;
    zvel = 0;

    if( s->picnum == APLAYER )
    {
        p = s->yvel;

        sx = ps[p].posx;
        sy = ps[p].posy;
        sz = ps[p].posz+ps[p].pyoff+(4<<8);
        sa = ps[p].ang;

        ps[p].crack_time = 777;

    }
    else
    {
        p = -1;
        sa = s->ang;
        sx = s->x;
        sy = s->y;
        sz = s->z-((s->yrepeat*tiles[s->picnum].dim.height)<<1)+(4<<8);
        if(s->picnum != ROTATEGUN)
        {
            sz -= (7<<8);
            if(badguy(s) && PN != COMMANDER)
            {
                sx += (sintable[(sa+1024+96)&2047]>>7);
                sy += (sintable[(sa+512+96)&2047]>>7);
            }
        }
    }

    switch(atwith)
    {
        case BLOODSPLAT1:
        case BLOODSPLAT2:
        case BLOODSPLAT3:
        case BLOODSPLAT4:

            if(p >= 0)
                sa += 64 - (TRAND&127);
            else sa += 1024 + 64 - (TRAND&127);
            zvel = 1024-(TRAND&2047);
        case KNEE:
            if(atwith == KNEE )
            {
                if(p >= 0)
                {
                    zvel = (100-ps[p].horiz-ps[p].horizoff)<<5;
                    sz += (6<<8);
                    sa += 15;
                }
                else
                {
                    j = ps[findplayer(s,&x)].i;
                    zvel = ( (sprite[j].z-sz)<<8 ) / (x+1);
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
            }

            hitscan(sx,sy,sz,sect,
                sintable[(sa+512)&2047],
                sintable[sa&2047],zvel<<6,
                &hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);

            if( atwith == BLOODSPLAT1 ||
                atwith == BLOODSPLAT2 ||
                atwith == BLOODSPLAT3 ||
                atwith == BLOODSPLAT4 )
            {
                if( FindDistance2D(sx-hitx,sy-hity) < 1024 )
                    if( hitwall >= 0 && wall[hitwall].overpicnum != BIGFORCE )
                        if( ( wall[hitwall].nextsector >= 0 && hitsect >= 0 &&
                            sector[wall[hitwall].nextsector].lotag == 0 &&
                                sector[hitsect].lotag == 0 &&
                                    sector[wall[hitwall].nextsector].lotag == 0 &&
                                        (sector[hitsect].floorz-sector[wall[hitwall].nextsector].floorz) > (16<<8) ) ||
                                            ( wall[hitwall].nextsector == -1 && sector[hitsect].lotag == 0 ) )
                                                if( (wall[hitwall].cstat&16) == 0)
                {
                    if(wall[hitwall].nextsector >= 0)
                    {
                        k = headspritesect[wall[hitwall].nextsector];
                        while(k >= 0)
                        {
                            if(sprite[k].statnum == 3 && sprite[k].lotag == 13)
                                return;
                            k = nextspritesect[k];
                        }
                    }

                    if( wall[hitwall].nextwall >= 0 &&
                        wall[wall[hitwall].nextwall].hitag != 0 )
                            return;

                    if(wall[hitwall].hitag == 0)
                    {
                        k = spawn(i,atwith);
                        sprite[k].xvel = -12;
                        sprite[k].ang = getangle(wall[hitwall].x-wall[wall[hitwall].point2].x,
                            wall[hitwall].y-wall[wall[hitwall].point2].y)+512;
                        sprite[k].x = hitx;
                        sprite[k].y = hity;
                        sprite[k].z = hitz;
                        sprite[k].cstat |= (TRAND&4);
                        ssp(k,CLIPMASK0);
                        setsprite(k,sprite[k].x,sprite[k].y,sprite[k].z);
                        if( PN == OOZFILTER || PN == NEWBEAST )
                            sprite[k].pal = 6;
                    }
                }
                return;
            }

            if(hitsect < 0) break;

            if( ( klabs(sx-hitx)+klabs(sy-hity) ) < 1024 )
            {
                if(hitwall >= 0 || hitspr >= 0)
                {
                    j = EGS(hitsect,hitx,hity,hitz,KNEE,-15,0,0,sa,32,0,i,4);
                    sprite[j].extra += (TRAND&7);
                    if(p >= 0)
                    {
                        k = spawn(j,SMALLSMOKE);
                        sprite[k].z -= (8<<8);
                        spritesound(KICK_HIT,j);
                    }

                    if ( p >= 0 && ps[p].steroids_amount > 0 && ps[p].steroids_amount < 400 )
                        sprite[j].extra += (max_player_health>>2);

                    if( hitspr >= 0 && sprite[hitspr].picnum != ACCESSSWITCH && sprite[hitspr].picnum != ACCESSSWITCH2 )
                    {
                        checkhitsprite(hitspr,j);
                        if(p >= 0) checkhitswitch(p,hitspr,1);
                    }

                    else if( hitwall >= 0 )
                    {
                        if( wall[hitwall].cstat&2 )
                            if(wall[hitwall].nextsector >= 0)
                                if(hitz >= (sector[wall[hitwall].nextsector].floorz) )
                                    hitwall = wall[hitwall].nextwall;

                        if( hitwall >= 0 && wall[hitwall].picnum != ACCESSSWITCH && wall[hitwall].picnum != ACCESSSWITCH2 )
                        {
                            checkhitwall(j,hitwall,hitx,hity,hitz,atwith);
                            if(p >= 0) checkhitswitch(p,hitwall,0);
                        }
                    }
                }
                else if(p >= 0 && zvel > 0 && sector[hitsect].lotag == 1)
                {
                    j = spawn(ps[p].i,WATERSPLASH2);
                    sprite[j].x = hitx;
                    sprite[j].y = hity;
                    sprite[j].ang = ps[p].ang; // Total tweek
                    sprite[j].xvel = 32;
                    ssp(i,CLIPMASK0);
                    sprite[j].xvel = 0;

                }
            }

            break;

        case SHOTSPARK1:
        case SHOTGUN:
        case CHAINGUN:

            if( s->extra >= 0 ) s->shade = -96;

            if(p >= 0)
            {
				j = aim( s, AUTO_AIM_ANGLE, ps[p].auto_aim!= 0 );
                if(j >= 0)
                {
                    dal = ((sprite[j].xrepeat*tiles[sprite[j].picnum].dim.height)<<1)+(5<<8);
                    switch(sprite[j].picnum)
                    {
                        case GREENSLIME:
                        case GREENSLIME+1:
                        case GREENSLIME+2:
                        case GREENSLIME+3:
                        case GREENSLIME+4:
                        case GREENSLIME+5:
                        case GREENSLIME+6:
                        case GREENSLIME+7:
                        case ROTATEGUN:
                            dal -= (8<<8);
                            break;
                    }
                    zvel = ( ( sprite[j].z-sz-dal )<<8 ) / ldist(&sprite[ps[p].i], &sprite[j]) ;
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }

                if(atwith == SHOTSPARK1)
                {
                    if(j == -1)
                    {
                        sa += 16-(TRAND&31);
                        zvel = (100-ps[p].horiz-ps[p].horizoff)<<5;
                        zvel += 128-(TRAND&255);
                    }
                }
                else
                {
                    sa += 16-(TRAND&31);
                    if(j == -1) zvel = (100-ps[p].horiz-ps[p].horizoff)<<5;
                    zvel += 128-(TRAND&255);
                }
                sz -= (2<<8);
            }
            else
            {
                j = findplayer(s,&x);
                sz -= (4<<8);
                zvel = ( (ps[j].posz-sz) <<8 ) / (ldist(&sprite[ps[j].i], s ) );
                if(s->picnum != BOSS1)
                {
                    zvel += 128-(TRAND&255);
                    sa += 32-(TRAND&63);
                }
                else
                {
                    zvel += 128-(TRAND&255);
                    sa = getangle(ps[j].posx-sx,ps[j].posy-sy)+64-(TRAND&127);
                }
            }

            s->cstat &= ~257;
            hitscan(sx,sy,sz,sect,
                sintable[(sa+512)&2047],
                sintable[sa&2047],
                zvel<<6,&hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);
            s->cstat |= 257;

            if(hitsect < 0) return;

            if( (TRAND&15) == 0 && sector[hitsect].lotag == 2 )
                tracers(hitx,hity,hitz,sx,sy,sz,8-(ud.multimode>>1));

            if(p >= 0)
            {
                k = EGS(hitsect,hitx,hity,hitz,SHOTSPARK1,-15,10,10,sa,0,0,i,4);
                sprite[k].extra = *actorscrptr[atwith];
                sprite[k].extra += (TRAND%6);

                if( hitwall == -1 && hitspr == -1)
                {
                    if( zvel < 0 )
                    {
                        if( sector[hitsect].ceilingstat&1 )
                        {
                            sprite[k].xrepeat = 0;
                            sprite[k].yrepeat = 0;
                            return;
                        }
                        else
                            checkhitceiling(hitsect);
                    }
                    spawn(k,SMALLSMOKE);
                }

                if(hitspr >= 0)
                {
                    checkhitsprite(hitspr,k);
                    if( sprite[hitspr].picnum == APLAYER && (ud.coop != 1 || ud.ffire == 1) )
                    {
                        l = spawn(k,JIBS6);
                        sprite[k].xrepeat = sprite[k].yrepeat = 0;
                        sprite[l].z += (4<<8);
                        sprite[l].xvel = 16;
                        sprite[l].xrepeat = sprite[l].yrepeat = 24;
                        sprite[l].ang += 64-(TRAND&127);
                    }
                    else spawn(k,SMALLSMOKE);

                    if(p >= 0 && (
                        sprite[hitspr].picnum == DIPSWITCH ||
                        sprite[hitspr].picnum == DIPSWITCH+1 ||
                        sprite[hitspr].picnum == DIPSWITCH2 ||
                        sprite[hitspr].picnum == DIPSWITCH2+1 ||
                        sprite[hitspr].picnum == DIPSWITCH3 ||
                        sprite[hitspr].picnum == DIPSWITCH3+1 ||
                        sprite[hitspr].picnum == HANDSWITCH ||
                        sprite[hitspr].picnum == HANDSWITCH+1) )
                    {
                        checkhitswitch(p,hitspr,1);
                        return;
                    }
                }
                else if( hitwall >= 0 )
                {
                    spawn(k,SMALLSMOKE);

                    if( isadoorwall(wall[hitwall].picnum) == 1 )
                        goto SKIPBULLETHOLE;
                    if(p >= 0 && (
                        wall[hitwall].picnum == DIPSWITCH ||
                        wall[hitwall].picnum == DIPSWITCH+1 ||
                        wall[hitwall].picnum == DIPSWITCH2 ||
                        wall[hitwall].picnum == DIPSWITCH2+1 ||
                        wall[hitwall].picnum == DIPSWITCH3 ||
                        wall[hitwall].picnum == DIPSWITCH3+1 ||
                        wall[hitwall].picnum == HANDSWITCH ||
                        wall[hitwall].picnum == HANDSWITCH+1) )
                    {
                        checkhitswitch(p,hitwall,0);
                        return;
                    }

                    if(wall[hitwall].hitag != 0 || ( wall[hitwall].nextwall >= 0 && wall[wall[hitwall].nextwall].hitag != 0 ) )
                        goto SKIPBULLETHOLE;

                    if( hitsect >= 0 && sector[hitsect].lotag == 0 )
                        if( wall[hitwall].overpicnum != BIGFORCE )
                            if( (wall[hitwall].nextsector >= 0 && sector[wall[hitwall].nextsector].lotag == 0 ) ||
                                ( wall[hitwall].nextsector == -1 && sector[hitsect].lotag == 0 ) )
                                    if( (wall[hitwall].cstat&16) == 0)
                    {
                        if(wall[hitwall].nextsector >= 0)
                        {
                            l = headspritesect[wall[hitwall].nextsector];
                            while(l >= 0)
                            {
                                if(sprite[l].statnum == 3 && sprite[l].lotag == 13)
                                    goto SKIPBULLETHOLE;
                                l = nextspritesect[l];
                            }
                        }

                        l = headspritestat[5];
                        while(l >= 0)
                        {
                            if(sprite[l].picnum == BULLETHOLE)
                                if(dist(&sprite[l],&sprite[k]) < (12+(TRAND&7)) )
                                    goto SKIPBULLETHOLE;
                            l = nextspritestat[l];
                        }
                        l = spawn(k,BULLETHOLE);
                        sprite[l].xvel = -1;
                        sprite[l].ang = getangle(wall[hitwall].x-wall[wall[hitwall].point2].x,
                            wall[hitwall].y-wall[wall[hitwall].point2].y)+512;
                        ssp(l,CLIPMASK0);
                    }

                    SKIPBULLETHOLE:

                    if( wall[hitwall].cstat&2 )
                        if(wall[hitwall].nextsector >= 0)
                            if(hitz >= (sector[wall[hitwall].nextsector].floorz) )
                                hitwall = wall[hitwall].nextwall;

                    checkhitwall(k,hitwall,hitx,hity,hitz,SHOTSPARK1);
                }
            }
            else
            {
                k = EGS(hitsect,hitx,hity,hitz,SHOTSPARK1,-15,24,24,sa,0,0,i,4);
                sprite[k].extra = *actorscrptr[atwith];

                if( hitspr >= 0 )
                {
                    checkhitsprite(hitspr,k);
                    if( sprite[hitspr].picnum != APLAYER )
                        spawn(k,SMALLSMOKE);
                    else sprite[k].xrepeat = sprite[k].yrepeat = 0;
                }
                else if( hitwall >= 0 )
                    checkhitwall(k,hitwall,hitx,hity,hitz,SHOTSPARK1);
            }

            if( (TRAND&255) < 4 )
                xyzsound(PISTOL_RICOCHET,k,hitx,hity,hitz);

            return;

        case FIRELASER:
        case SPIT:
        case COOLEXPLOSION1:

            if( s->extra >= 0 ) s->shade = -96;

            scount = 1;
            if(atwith == SPIT) vel = 292;
            else
            {
                if(atwith == COOLEXPLOSION1)
                {
                    if(s->picnum == BOSS2) vel = 644;
                    else vel = 348;
                    sz -= (4<<7);
                }
                else
                {
                    vel = 840;
                    sz -= (4<<7);
                }
            }

            if(p >= 0)
            {
				j = aim( s, AUTO_AIM_ANGLE, ps[p].auto_aim==2 );
                if(j >= 0)
                {
                    dal = ((sprite[j].xrepeat*tiles[sprite[j].picnum].dim.height)<<1)-(12<<8);
                    zvel = ((sprite[j].z-sz-dal)*vel ) / ldist(&sprite[ps[p].i], &sprite[j]) ;
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
                else
                    zvel = (100-ps[p].horiz-ps[p].horizoff)*98;
            }
            else
            {
                j = findplayer(s,&x);
//                sa = getangle(ps[j].oposx-sx,ps[j].oposy-sy);
                sa += 16-(TRAND&31);
                zvel = ( ( (ps[j].oposz - sz + (3<<8) ) )*vel ) / ldist(&sprite[ps[j].i],s);
            }

            oldzvel = zvel;

            if(atwith == SPIT) { sizx = 18;sizy = 18,sz -= (10<<8); }
            else
            {
                if( atwith == FIRELASER )
                {
                    if(p >= 0)
                    {
                        
                        sizx = 34;
                        sizy = 34;
                    }
                    else
                    {
                        sizx = 18;
                        sizy = 18;
                    }
                }
                else
                {
                    sizx = 18;
                    sizy = 18;
                }
            }

            if(p >= 0) sizx = 7,sizy = 7;

            while(scount > 0)
            {
                j = EGS(sect,sx,sy,sz,atwith,-127,sizx,sizy,sa,vel,zvel,i,4);
                sprite[j].extra += (TRAND&7);

                if(atwith == COOLEXPLOSION1)
                {
                    sprite[j].shade = 0;
                    if(PN == BOSS2)
                    {
                        l = sprite[j].xvel;
                        sprite[j].xvel = 1024;
                        ssp(j,CLIPMASK0);
                        sprite[j].xvel = l;
                        sprite[j].ang += 128-(TRAND&255);
                    }
                }

                sprite[j].cstat = 128;
                sprite[j].clipdist = 4;

                sa = s->ang+32-(TRAND&63);
                zvel = oldzvel+512-(TRAND&1023);

                scount--;
            }

            return;

        case FREEZEBLAST:
            sz += (3<<8);
        case RPG:

            if( s->extra >= 0 ) s->shade = -96;

            scount = 1;
            vel = 644;

            j = -1;

            if(p >= 0)
			{
                j = aim( s, 48, ps[p].auto_aim==2);
                if(j >= 0)
                {
                    dal = ((sprite[j].xrepeat*tiles[sprite[j].picnum].dim.height)<<1)+(8<<8);
                    zvel = ( (sprite[j].z-sz-dal)*vel ) / ldist(&sprite[ps[p].i], &sprite[j]);
                    if( sprite[j].picnum != RECON )
                        sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
                else zvel = (100-ps[p].horiz-ps[p].horizoff)*81;
                if(atwith == RPG)
                    spritesound(RPG_SHOOT,i);

            }
            else
            {
                j = findplayer(s,&x);
                sa = getangle(ps[j].oposx-sx,ps[j].oposy-sy);
                if(PN == BOSS3)
                    sz -= (32<<8);
                else if(PN == BOSS2)
                {
                    vel += 128;
                    sz += 24<<8;
                }

                l = ldist(&sprite[ps[j].i],s);
                zvel = ( (ps[j].oposz-sz)*vel) / l;

                if( badguy(s) && (s->hitag&face_player_smart) )
                    sa = s->ang+(TRAND&31)-16;
            }

            if( p >= 0 && j >= 0)
               l = j;
            else l = -1;

            j = EGS(sect,
                sx+(sintable[(348+sa+512)&2047]/448),
                sy+(sintable[(sa+348)&2047]/448),
                sz-(1<<8),atwith,0,14,14,sa,vel,zvel,i,4);

            sprite[j].extra += (TRAND&7);
            if(atwith != FREEZEBLAST)
                sprite[j].yvel = l;
            else
            {
                sprite[j].yvel = numfreezebounces;
                sprite[j].xrepeat >>= 1;
                sprite[j].yrepeat >>= 1;
                sprite[j].zvel -= (2<<4);
            }

            if(p == -1)
            {
                if(PN == BOSS3)
                {
                    if(TRAND&1)
                    {
                        sprite[j].x -= sintable[sa&2047]>>6;
                        sprite[j].y -= sintable[(sa+1024+512)&2047]>>6;
                        sprite[j].ang -= 8;
                    }
                    else
                    {
                        sprite[j].x += sintable[sa&2047]>>6;
                        sprite[j].y += sintable[(sa+1024+512)&2047]>>6;
                        sprite[j].ang += 4;
                    }
                    sprite[j].xrepeat = 42;
                    sprite[j].yrepeat = 42;
                }
                else if(PN == BOSS2)
                {
                    sprite[j].x -= sintable[sa&2047]/56;
                    sprite[j].y -= sintable[(sa+1024+512)&2047]/56;
                    sprite[j].ang -= 8+(TRAND&255)-128;
                    sprite[j].xrepeat = 24;
                    sprite[j].yrepeat = 24;
                }
                else if(atwith != FREEZEBLAST)
                {
                    sprite[j].xrepeat = 30;
                    sprite[j].yrepeat = 30;
                    sprite[j].extra >>= 2;
                }
            }
            else if(ps[p].curr_weapon == DEVISTATOR_WEAPON)
            {
                sprite[j].extra >>= 2;
                sprite[j].ang += 16-(TRAND&31);
                sprite[j].zvel += 256-(TRAND&511);

                if( ps[p].hbomb_hold_delay )
                {
                    sprite[j].x -= sintable[sa&2047]/644;
                    sprite[j].y -= sintable[(sa+1024+512)&2047]/644;
                }
                else
                {
                    sprite[j].x += sintable[sa&2047]>>8;
                    sprite[j].y += sintable[(sa+1024+512)&2047]>>8;
                }
                sprite[j].xrepeat >>= 1;
                sprite[j].yrepeat >>= 1;
            }

            sprite[j].cstat = 128;
            if(atwith == RPG)
                sprite[j].clipdist = 4;
            else
                sprite[j].clipdist = 40;

            break;

        case HANDHOLDINGLASER:

            if(p >= 0)
                zvel = (100-ps[p].horiz-ps[p].horizoff)*32;
            else zvel = 0;

            hitscan(sx,sy,sz-ps[p].pyoff,sect,
                sintable[(sa+512)&2047],
                sintable[sa&2047],
                zvel<<6,&hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);

            j = 0;
            if(hitspr >= 0) break;

            if(hitwall >= 0 && hitsect >= 0)
                if( ((hitx-sx)*(hitx-sx)+(hity-sy)*(hity-sy)) < (290*290) )
            {
                if( wall[hitwall].nextsector >= 0)
                {
                    if( sector[wall[hitwall].nextsector].lotag <= 2 && sector[hitsect].lotag <= 2 )
                        j = 1;
                }
                else if( sector[hitsect].lotag <= 2 )
                    j = 1;
            }

            if(j == 1)
            {
                k = EGS(hitsect,hitx,hity,hitz,TRIPBOMB,-16,4,5,sa,0,0,i,6);

                sprite[k].hitag = k;
                spritesound(LASERTRIP_ONWALL,k);
                sprite[k].xvel = -20;
                ssp(k,CLIPMASK0);
                sprite[k].cstat = 16;
                hittype[k].temp_data[5] = sprite[k].ang = getangle(wall[hitwall].x-wall[wall[hitwall].point2].x,wall[hitwall].y-wall[wall[hitwall].point2].y)-512;

                if(p >= 0)
                    ps[p].ammo_amount[TRIPBOMB_WEAPON]--;

            }
            return;

        case BOUNCEMINE:
        case MORTER:

            if( s->extra >= 0 ) s->shade = -96;

            j = ps[findplayer(s,&x)].i;
            x = ldist(&sprite[j],s);

            zvel = -x>>1;

            if(zvel < -4096)
                zvel = -2048;
            vel = x>>4;

            EGS(sect,
                sx+(sintable[(512+sa+512)&2047]>>8),
                sy+(sintable[(sa+512)&2047]>>8),
                sz+(6<<8),atwith,-64,32,32,sa,vel,zvel,i,1);
            break;

        case GROWSPARK:

            if(p >= 0)
            {
                j = aim( s, AUTO_AIM_ANGLE, ps[p].auto_aim==2);
                if(j >= 0)
                {
                    dal = ((sprite[j].xrepeat*tiles[sprite[j].picnum].dim.height)<<1)+(5<<8);
                    switch(sprite[j].picnum)
                    {
                        case GREENSLIME:
                        case GREENSLIME+1:
                        case GREENSLIME+2:
                        case GREENSLIME+3:
                        case GREENSLIME+4:
                        case GREENSLIME+5:
                        case GREENSLIME+6:
                        case GREENSLIME+7:
                        case ROTATEGUN:
                            dal -= (8<<8);
                            break;
                    }
                    zvel = ( ( sprite[j].z-sz-dal )<<8 ) / (ldist(&sprite[ps[p].i], &sprite[j]) );
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
                else
                {
                    sa += 16-(TRAND&31);
                    zvel = (100-ps[p].horiz-ps[p].horizoff)<<5;
                    zvel += 128-(TRAND&255);
                }

                sz -= (2<<8);
            }
            else
            {
                j = findplayer(s,&x);
                sz -= (4<<8);
                zvel = ( (ps[j].posz-sz) <<8 ) / (ldist(&sprite[ps[j].i], s ) );
                zvel += 128-(TRAND&255);
                sa += 32-(TRAND&63);
            }

            k = 0;

//            RESHOOTGROW:

            s->cstat &= ~257;
            hitscan(sx,sy,sz,sect,
                sintable[(sa+512)&2047],
                sintable[sa&2047],
                zvel<<6,&hitsect,&hitwall,&hitspr,&hitx,&hity,&hitz,CLIPMASK1);

            s->cstat |= 257;

            j = EGS(sect,hitx,hity,hitz,GROWSPARK,-16,28,28,sa,0,0,i,1);

            sprite[j].pal = 2;
            sprite[j].cstat |= 130;
            sprite[j].xrepeat = sprite[j].yrepeat = 1;

            if( hitwall == -1 && hitspr == -1 && hitsect >= 0)
            {
                if( zvel < 0 && (sector[hitsect].ceilingstat&1) == 0)
                    checkhitceiling(hitsect);
            }
            else if(hitspr >= 0) checkhitsprite(hitspr,j);
            else if(hitwall >= 0 && wall[hitwall].picnum != ACCESSSWITCH && wall[hitwall].picnum != ACCESSSWITCH2 )
            {
            /*    if(wall[hitwall].overpicnum == MIRROR && k == 0)
                {
                    l = getangle(
                        wall[wall[hitwall].point2].x-wall[hitwall].x,
                        wall[wall[hitwall].point2].y-wall[hitwall].y);

                    sx = hitx;
                    sy = hity;
                    sz = hitz;
                    sect = hitsect;
                    sa = ((l<<1) - sa)&2047;
                    sx += sintable[(sa+512)&2047]>>12;
                    sy += sintable[sa&2047]>>12;

                    k++;
                    goto RESHOOTGROW;
                }
                else */
                    checkhitwall(j,hitwall,hitx,hity,hitz,atwith);
            }

            break;
        case SHRINKER:
            if( s->extra >= 0 ) s->shade = -96;
            if(p >= 0)
            {
                j = aim( s, AUTO_AIM_ANGLE, ps[p].auto_aim==2);
                if(j >= 0)
                {
                    dal = ((sprite[j].xrepeat*tiles[sprite[j].picnum].dim.height)<<1);
                    zvel = ( (sprite[j].z-sz-dal-(4<<8))*768) / (ldist( &sprite[ps[p].i], &sprite[j]));
                    sa = getangle(sprite[j].x-sx,sprite[j].y-sy);
                }
                else zvel = (100-ps[p].horiz-ps[p].horizoff)*98;
            }
            else if(s->statnum != 3)
            {
                j = findplayer(s,&x);
                l = ldist(&sprite[ps[j].i],s);
                zvel = ( (ps[j].oposz-sz)*512) / l ;
            }
            else zvel = 0;

            j = EGS(sect,
                sx+(sintable[(512+sa+512)&2047]>>12),
                sy+(sintable[(sa+512)&2047]>>12),
                sz+(2<<8),SHRINKSPARK,-16,28,28,sa,768,zvel,i,4);

            sprite[j].cstat = 128;
            sprite[j].clipdist = 32;


            return;
    }
    return;
}

void displayloogie(short snum)
{
    int32_t i, a, x, y, z;

    if(ps[snum].loogcnt == 0) return;

    y = (ps[snum].loogcnt<<2);
    for(i=0;i<ps[snum].numloogs;i++)
    {
        a = klabs(sintable[((ps[snum].loogcnt+i)<<5)&2047])>>5;
        z = 4096+((ps[snum].loogcnt+i)<<9);
        x = (-sync[snum].avel)+(sintable[((ps[snum].loogcnt+i)<<6)&2047]>>10);

        rotatesprite(
            (ps[snum].loogiex[i]+x)<<16,(200+ps[snum].loogiey[i]-y)<<16,z-(i<<8),256-a,
            LOOGIE,0,0,2,0,0,xdim-1,ydim-1);
    }
}

uint8_t  animatefist(short gs,short snum)
{
    short looking_arc,fisti,fistpal;
    int32_t fistzoom, fistz;

    fisti = ps[snum].fist_incs;
    if(fisti > 32) fisti = 32;
    if(fisti <= 0) return 0;

    looking_arc = klabs(ps[snum].look_ang)/9;

    fistzoom = 65536L - (sintable[(512+(fisti<<6))&2047]<<2);
    if(fistzoom > 90612L)
        fistzoom = 90612L;
    if(fistzoom < 40920)
        fistzoom = 40290;
    fistz = 194 + (sintable[((6+fisti)<<7)&2047]>>9);

    if(sprite[ps[snum].i].pal == 1)
        fistpal = 1;
    else
        fistpal = sector[ps[snum].cursectnum].floorpal;

    rotatesprite(
        (-fisti+222+(sync[snum].avel>>4))<<16,
        (looking_arc+fistz)<<16,
        fistzoom,0,FIST,gs,fistpal,2,0,0,xdim-1,ydim-1);

    return 1;
}

uint8_t  animateknee(short gs,short snum)
{
    short knee_y[] = {0,-8,-16,-32,-64,-84,-108,-108,-108,-72,-32,-8};
    short looking_arc, pal;

    if(ps[snum].knee_incs > 11 || ps[snum].knee_incs == 0 || sprite[ps[snum].i].extra <= 0) return 0;

    looking_arc = knee_y[ps[snum].knee_incs] + klabs(ps[snum].look_ang)/9;

    looking_arc -= (ps[snum].hard_landing<<3);

    if(sprite[ps[snum].i].pal == 1)
        pal = 1;
    else
    {
        pal = sector[ps[snum].cursectnum].floorpal;
        if(pal == 0)
            pal = ps[snum].palookup;
    }

    myospal(105+(sync[snum].avel>>4)-(ps[snum].look_ang>>1)+(knee_y[ps[snum].knee_incs]>>2),looking_arc+280-((ps[snum].horiz-ps[snum].horizoff)>>4),KNEE,gs,4,pal);

    return 1;
}

uint8_t  animateknuckles(short gs,short snum)
{
    short knuckle_frames[] = {0,1,2,2,3,3,3,2,2,1,0};
    short looking_arc, pal;

    if(ps[snum].knuckle_incs == 0 || sprite[ps[snum].i].extra <= 0) return 0;

    looking_arc = klabs(ps[snum].look_ang)/9;

    looking_arc -= (ps[snum].hard_landing<<3);

    if(sprite[ps[snum].i].pal == 1)
        pal = 1;
    else
        pal = sector[ps[snum].cursectnum].floorpal;

    myospal(160+(sync[snum].avel>>4)-(ps[snum].look_ang>>1),looking_arc+180-((ps[snum].horiz-ps[snum].horizoff)>>4),CRACKKNUCKLES+knuckle_frames[ps[snum].knuckle_incs>>1],gs,4,pal);

    return 1;
}



int32_t lastvisinc;

void displaymasks(short snum)
{
    short p;

    if(sprite[ps[snum].i].pal == 1)
        p = 1;
    else
        p = sector[ps[snum].cursectnum].floorpal;

     if(ps[snum].scuba_on)
	 {
        if(ud.screen_size > 4)
        {
            rotatesprite((43<<16),(200-8-((tiles[SCUBAMASK].dim.height)<<16)),65536,0,SCUBAMASK,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite(((320-43)<<16),(200-8-((tiles[SCUBAMASK].dim.height)<<16)),65536,1024,SCUBAMASK,0,p,2+4+16,windowx1,windowy1,windowx2,windowy2);
        }
        else
        {
            rotatesprite((43<<16),(200-((tiles[SCUBAMASK].dim.height)<<16)),65536,0,SCUBAMASK,0,p,2+16,windowx1,windowy1,windowx2,windowy2);
            rotatesprite(((320-43)<<16),(200-((tiles[SCUBAMASK].dim.height)<<16)),65536,1024,SCUBAMASK,0,p,2+4+16,windowx1,windowy1,windowx2,windowy2);
        }
	 }
}

uint8_t  animatetip(short gs,short snum)
{
    short p,looking_arc;
    short tip_y[] = {0,-8,-16,-32,-64,-84,-108,-108,-108,-108,-108,-108,-108,-108,-108,-108,-96,-72,-64,-32,-16};

    if(ps[snum].tipincs == 0) return 0;

    looking_arc = klabs(ps[snum].look_ang)/9;
    looking_arc -= (ps[snum].hard_landing<<3);

    if(sprite[ps[snum].i].pal == 1)
        p = 1;
    else
        p = sector[ps[snum].cursectnum].floorpal;

/*    if(ps[snum].access_spritenum >= 0)
        p = sprite[ps[snum].access_spritenum].pal;
    else
        p = wall[ps[snum].access_wallnum].pal;
  */
    myospal(170+(sync[snum].avel>>4)-(ps[snum].look_ang>>1),
        (tip_y[ps[snum].tipincs]>>1)+looking_arc+240-((ps[snum].horiz-ps[snum].horizoff)>>4),TIP+((26-ps[snum].tipincs)>>4),gs,0,p);

    return 1;
}

uint8_t  animateaccess(short gs,short snum)
{
    short access_y[] = {0,-8,-16,-32,-64,-84,-108,-108,-108,-108,-108,-108,-108,-108,-108,-108,-96,-72,-64,-32,-16};
    short looking_arc;
    uint8_t  p;

    if(ps[snum].access_incs == 0 || sprite[ps[snum].i].extra <= 0) return 0;

    looking_arc = access_y[ps[snum].access_incs] + klabs(ps[snum].look_ang)/9;
    looking_arc -= (ps[snum].hard_landing<<3);

    if(ps[snum].access_spritenum >= 0)
        p = sprite[ps[snum].access_spritenum].pal;
    else p = 0;
//    else
//        p = wall[ps[snum].access_wallnum].pal;

    if((ps[snum].access_incs-3) > 0 && (ps[snum].access_incs-3)>>3)
        myospal(170+(sync[snum].avel>>4)-(ps[snum].look_ang>>1)+(access_y[ps[snum].access_incs]>>2),looking_arc+266-((ps[snum].horiz-ps[snum].horizoff)>>4),HANDHOLDINGLASER+(ps[snum].access_incs>>3),gs,0,p);
    else
        myospal(170+(sync[snum].avel>>4)-(ps[snum].look_ang>>1)+(access_y[ps[snum].access_incs]>>2),looking_arc+266-((ps[snum].horiz-ps[snum].horizoff)>>4),HANDHOLDINGACCESS,gs,4,p);

    return 1;
}

short fistsign;

void displayweapon(short snum)
{
    int32_t gun_pos, looking_arc, cw;
    int32_t weapon_xoffset, i, j;
    uint8_t  o,pal;
    int8_t gs;
    struct player_struct *p;
    short *kb;
    
    

    p = &ps[snum];
    kb = &p->kickback_pic;

    o = 0;

    looking_arc = klabs(p->look_ang)/9;

    gs = sprite[p->i].shade;
    if(gs > 24) gs = 24;

    if(p->newowner >= 0 || ud.camerasprite >= 0 || p->over_shoulder_on > 0 || (sprite[p->i].pal != 1 && sprite[p->i].extra <= 0) || animatefist(gs,snum) || animateknuckles(gs,snum) || animatetip(gs,snum) || animateaccess(gs,snum) )
        return;

    animateknee(gs,snum);

    gun_pos = 80-(p->weapon_pos*p->weapon_pos);

    weapon_xoffset =  (160)-90;
    weapon_xoffset -= (sintable[((p->weapon_sway>>1)+512)&2047]/(1024+512));
    weapon_xoffset -= 58 + p->weapon_ang;
    if( sprite[p->i].xrepeat < 32 )
        gun_pos -= klabs(sintable[(p->weapon_sway<<2)&2047]>>9);
    else gun_pos -= klabs(sintable[(p->weapon_sway>>1)&2047]>>10);

    gun_pos -= (p->hard_landing<<3);

    if(p->last_weapon >= 0)
        cw = p->last_weapon;
    else cw = p->curr_weapon;

    j = 14-p->quick_kick;
    if(j != 14)
    {
        if(sprite[p->i].pal == 1)
            pal = 1;
        else
        {
            pal = sector[p->cursectnum].floorpal;
            if(pal == 0)
                pal = p->palookup;
        }


        if( j < 5 || j > 9 )
            myospal(weapon_xoffset+80-(p->look_ang>>1),
                looking_arc+250-gun_pos,KNEE,gs,o|4,pal);
        else myospal(weapon_xoffset+160-16-(p->look_ang>>1),
            looking_arc+214-gun_pos,KNEE+1,gs,o|4,pal);
    }

    if( sprite[p->i].xrepeat < 40 )
    {
        if(p->jetpack_on == 0 )
        {
            i = sprite[p->i].xvel;
            looking_arc += 32-(i>>1);
            fistsign += i>>1;
        }
        cw = weapon_xoffset;
        weapon_xoffset += sintable[(fistsign)&2047]>>10;
        myos(weapon_xoffset+250-(p->look_ang>>1),
             looking_arc+258-(klabs(sintable[(fistsign)&2047]>>8)),
             FIST,gs,o);
        weapon_xoffset = cw;
        weapon_xoffset -= sintable[(fistsign)&2047]>>10;
        myos(weapon_xoffset+40-(p->look_ang>>1),
             looking_arc+200+(klabs(sintable[(fistsign)&2047]>>8)),
             FIST,gs,o|4);
    }
    else 
	{	
		// FIX_00026: Weapon can now be hidden (on your screen only).
		if(!ud.hideweapon || cw==KNEE_WEAPON || cw == HANDREMOTE_WEAPON)
		{
			switch(cw)
			{
				case KNEE_WEAPON:
					if( (*kb) > 0 )
					{
						if(sprite[p->i].pal == 1)
							pal = 1;
						else
						{
							pal = sector[p->cursectnum].floorpal;
							if(pal == 0)
								pal = p->palookup;
						}

						if( (*kb) < 5 || (*kb) > 9 )
							myospal(weapon_xoffset+220-(p->look_ang>>1),
								looking_arc+250-gun_pos,KNEE,gs,o,pal);
						else
							myospal(weapon_xoffset+160-(p->look_ang>>1),
							looking_arc+214-gun_pos,KNEE+1,gs,o,pal);
					}
					break;

				case TRIPBOMB_WEAPON:
					if(sprite[p->i].pal == 1)
						pal = 1;
					else
						pal = sector[p->cursectnum].floorpal;

					weapon_xoffset += 8;
					gun_pos -= 10;

					if((*kb) > 6)
						looking_arc += ((*kb)<<3);
					else if((*kb) < 4)
						myospal(weapon_xoffset+142-(p->look_ang>>1),
								looking_arc+234-gun_pos,HANDHOLDINGLASER+3,gs,o,pal);

					myospal(weapon_xoffset+130-(p->look_ang>>1),
							looking_arc+249-gun_pos,
							HANDHOLDINGLASER+((*kb)>>2),gs,o,pal);
					myospal(weapon_xoffset+152-(p->look_ang>>1),
							looking_arc+249-gun_pos,
							HANDHOLDINGLASER+((*kb)>>2),gs,o|4,pal);

					break;

				case RPG_WEAPON:
					if(sprite[p->i].pal == 1)
						pal = 1;
					else pal = sector[p->cursectnum].floorpal;

					weapon_xoffset -= sintable[(768+((*kb)<<7))&2047]>>11;
					gun_pos += sintable[(768+(((*kb)<<7)&2047))]>>11;

					if(*kb > 0)
					{
						if(*kb < 8)
						{
							myospal(weapon_xoffset+164,(looking_arc<<1)+176-gun_pos,
									RPGGUN+((*kb)>>1),gs,o,pal);
						}
					}

					myospal(weapon_xoffset+164,(looking_arc<<1)+176-gun_pos,
							RPGGUN,gs,o,pal);

					break;

				case SHOTGUN_WEAPON:
					if(sprite[p->i].pal == 1)
						pal = 1;
					else
						pal = sector[p->cursectnum].floorpal;

					weapon_xoffset -= 8;

					switch(*kb)
					{
						case 1:
						case 2:
							myospal(weapon_xoffset+168-(p->look_ang>>1),looking_arc+201-gun_pos,
							SHOTGUN+2,-128,o,pal);
						case 0:
						case 6:
						case 7:
						case 8:
							myospal(weapon_xoffset+146-(p->look_ang>>1),looking_arc+202-gun_pos,
								SHOTGUN,gs,o,pal);
							break;
						case 3:
						case 4:
						case 5:
						case 9:
						case 10:
						case 11:
						case 12:
							if( *kb > 1 && *kb < 5 )
							{
								gun_pos -= 40;
								weapon_xoffset += 20;

								myospal(weapon_xoffset+178-(p->look_ang>>1),looking_arc+194-gun_pos,
									SHOTGUN+1+((*(kb)-1)>>1),-128,o,pal);
							}

							myospal(weapon_xoffset+158-(p->look_ang>>1),looking_arc+220-gun_pos,
								SHOTGUN+3,gs,o,pal);

							break;
						case 13:
						case 14:
						case 15:
							myospal(32+weapon_xoffset+166-(p->look_ang>>1),looking_arc+210-gun_pos,
								SHOTGUN+4,gs,o,pal);
							break;
						case 16:
						case 17:
						case 18:
						case 19:
							myospal(64+weapon_xoffset+170-(p->look_ang>>1),looking_arc+196-gun_pos,
								SHOTGUN+5,gs,o,pal);
							break;
						case 20:
						case 21:
						case 22:
						case 23:
							myospal(64+weapon_xoffset+176-(p->look_ang>>1),looking_arc+196-gun_pos,
								SHOTGUN+6,gs,o,pal);
							break;
						case 24:
						case 25:
						case 26:
						case 27:
							myospal(64+weapon_xoffset+170-(p->look_ang>>1),looking_arc+196-gun_pos,
								SHOTGUN+5,gs,o,pal);
							break;
						case 28:
						case 29:
						case 30:
							myospal(32+weapon_xoffset+156-(p->look_ang>>1),looking_arc+206-gun_pos,
								SHOTGUN+4,gs,o,pal);
							break;
							}
					break;



				case CHAINGUN_WEAPON:
					if(sprite[p->i].pal == 1)
						pal = 1;
					else
						pal = sector[p->cursectnum].floorpal;

					if(*kb > 0)
						gun_pos -= sintable[(*kb)<<7]>>12;

					if(*kb > 0 && sprite[p->i].pal != 1) weapon_xoffset += 1-(rand()&3);

					myospal(weapon_xoffset+168-(p->look_ang>>1),looking_arc+260-gun_pos,
						CHAINGUN,gs,o,pal);
					switch(*kb)
					{
						case 0:
							myospal(weapon_xoffset+178-(p->look_ang>>1),looking_arc+233-gun_pos,
								CHAINGUN+1,gs,o,pal);
							break;
						default:
							if(*kb > 4 && *kb < 12)
							{
								i = 0;
								if(sprite[p->i].pal != 1) i = rand()&7;
								myospal(i+weapon_xoffset-4+140-(p->look_ang>>1),i+looking_arc-((*kb)>>1)+208-gun_pos,
									CHAINGUN+5+((*kb-4)/5),gs,o,pal);
								if(sprite[p->i].pal != 1) i = rand()&7;
								myospal(i+weapon_xoffset-4+184-(p->look_ang>>1),i+looking_arc-((*kb)>>1)+208-gun_pos,
									CHAINGUN+5+((*kb-4)/5),gs,o,pal);
							}
							if(*kb < 8)
							{
								i = rand()&7;
								myospal(i+weapon_xoffset-4+162-(p->look_ang>>1),i+looking_arc-((*kb)>>1)+208-gun_pos,
									CHAINGUN+5+((*kb-2)/5),gs,o,pal);
								myospal(weapon_xoffset+178-(p->look_ang>>1),looking_arc+233-gun_pos,
									CHAINGUN+1+((*kb)>>1),gs,o,pal);
							}
							else myospal(weapon_xoffset+178-(p->look_ang>>1),looking_arc+233-gun_pos,
								CHAINGUN+1,gs,o,pal);
							break;
					}
					break;
				case PISTOL_WEAPON:
					if(sprite[p->i].pal == 1)
						pal = 1;
					else
						pal = sector[p->cursectnum].floorpal;

					if( (*kb) < 5)
					{
						short kb_frames[] = {0,1,2,0,0},l;

						l = 195-12+weapon_xoffset;

						if((*kb) == 2)
							l -= 3;
						myospal(
							(l-(p->look_ang>>1)),
							(looking_arc+244-gun_pos),
							FIRSTGUN+kb_frames[*kb],
							gs,2,pal);
					}
					else
					{
						if((*kb) < 10)
							myospal(194-(p->look_ang>>1),looking_arc+230-gun_pos,FIRSTGUN+4,gs,o,pal);
						else if((*kb) < 15)
						{
							myospal(244-((*kb)<<3)-(p->look_ang>>1),looking_arc+130-gun_pos+((*kb)<<4),FIRSTGUN+6,gs,o,pal);
							myospal(224-(p->look_ang>>1),looking_arc+220-gun_pos,FIRSTGUN+5,gs,o,pal);
						}
						else if((*kb) < 20)
						{
							myospal(124+((*kb)<<1)-(p->look_ang>>1),looking_arc+430-gun_pos-((*kb)<<3),FIRSTGUN+6,gs,o,pal);
							myospal(224-(p->look_ang>>1),looking_arc+220-gun_pos,FIRSTGUN+5,gs,o,pal);
						}
						else if((*kb) < 23)
						{
							myospal(184-(p->look_ang>>1),looking_arc+235-gun_pos,FIRSTGUN+8,gs,o,pal);
							myospal(224-(p->look_ang>>1),looking_arc+210-gun_pos,FIRSTGUN+5,gs,o,pal);
						}
						else if((*kb) < 25)
						{
							myospal(164-(p->look_ang>>1),looking_arc+245-gun_pos,FIRSTGUN+8,gs,o,pal);
							myospal(224-(p->look_ang>>1),looking_arc+220-gun_pos,FIRSTGUN+5,gs,o,pal);
						}
						else if((*kb) < 27)
							myospal(194-(p->look_ang>>1),looking_arc+235-gun_pos,FIRSTGUN+5,gs,o,pal);
					}

					break;
				case HANDBOMB_WEAPON:
				{
					if(sprite[p->i].pal == 1)
						pal = 1;
					else
						pal = sector[p->cursectnum].floorpal;

					if((*kb))
					{
						uint8_t  throw_frames[]
							= {0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2};

						if((*kb) < 7)
							gun_pos -= 10*(*kb);        //D
						else if((*kb) < 12)
							gun_pos += 20*((*kb)-10); //U
						else if((*kb) < 20)
							gun_pos -= 9*((*kb)-14);  //D

						myospal(weapon_xoffset+190-(p->look_ang>>1),looking_arc+250-gun_pos,HANDTHROW+throw_frames[(*kb)],gs,o,pal);
					}
					else
						myospal(weapon_xoffset+190-(p->look_ang>>1),looking_arc+260-gun_pos,HANDTHROW,gs,o,pal);
				}
				break;

				case HANDREMOTE_WEAPON:
				{
						int8_t remote_frames[] = {0,1,1,2,1,1,0,0,0,0,0};
						if(sprite[p->i].pal == 1)
							pal = 1;
						else
							pal = sector[p->cursectnum].floorpal;

						weapon_xoffset = -48;

						if((*kb))
							myospal(weapon_xoffset+150-(p->look_ang>>1),looking_arc+258-gun_pos,HANDREMOTE+remote_frames[(*kb)],gs,o,pal);
						else
							myospal(weapon_xoffset+150-(p->look_ang>>1),looking_arc+258-gun_pos,HANDREMOTE,gs,o,pal);
					}
					break;
				case DEVISTATOR_WEAPON:
					if(sprite[p->i].pal == 1)
						pal = 1;
					else
						pal = sector[p->cursectnum].floorpal;

					if((*kb))
					{
						uint8_t  cycloidy[] = {0,4,12,24,12,4,0};

						i = sgn((*kb)>>2);

						if(p->hbomb_hold_delay)
						{
							myospal( (cycloidy[*kb]>>1)+weapon_xoffset+268-(p->look_ang>>1),cycloidy[*kb]+looking_arc+238-gun_pos,DEVISTATOR+i,-32,o,pal);
							myospal(weapon_xoffset+30-(p->look_ang>>1),looking_arc+240-gun_pos,DEVISTATOR,gs,o|4,pal);
						}
						else
						{
							myospal( -(cycloidy[*kb]>>1)+weapon_xoffset+30-(p->look_ang>>1),cycloidy[*kb]+looking_arc+240-gun_pos,DEVISTATOR+i,-32,o|4,pal);
							myospal(weapon_xoffset+268-(p->look_ang>>1),looking_arc+238-gun_pos,DEVISTATOR,gs,o,pal);
						}
					}
					else
					{
						myospal(weapon_xoffset+268-(p->look_ang>>1),looking_arc+238-gun_pos,DEVISTATOR,gs,o,pal);
						myospal(weapon_xoffset+30-(p->look_ang>>1),looking_arc+240-gun_pos,DEVISTATOR,gs,o|4,pal);
					}
					break;

				case FREEZE_WEAPON:
					if(sprite[p->i].pal == 1)
						pal = 1;
					else
						pal = sector[p->cursectnum].floorpal;

					if((*kb))
					{
						uint8_t  cat_frames[] = { 0,0,1,1,2,2 };

						if(sprite[p->i].pal != 1)
						{
							weapon_xoffset += rand()&3;
							looking_arc += rand()&3;
						}
						gun_pos -= 16;
						myospal(weapon_xoffset+210-(p->look_ang>>1),looking_arc+261-gun_pos,FREEZE+2,-32,o,pal);
						myospal(weapon_xoffset+210-(p->look_ang>>1),looking_arc+235-gun_pos,FREEZE+3+cat_frames[*kb%6],-32,o,pal);
					}
					else myospal(weapon_xoffset+210-(p->look_ang>>1),looking_arc+261-gun_pos,FREEZE,gs,o,pal);

					break;

				case SHRINKER_WEAPON:
				case GROW_WEAPON:
					weapon_xoffset += 28;
					looking_arc += 18;
					if(sprite[p->i].pal == 1)
						pal = 1;
					else
						pal = sector[p->cursectnum].floorpal;
					if((*kb) == 0)
					{
						if(cw == GROW_WEAPON)
						{
							myospal(weapon_xoffset+184-(p->look_ang>>1),
								looking_arc+240-gun_pos,SHRINKER+2,
								16-(sintable[p->random_club_frame&2047]>>10),
								o,2);

							myospal(weapon_xoffset+188-(p->look_ang>>1),
							looking_arc+240-gun_pos,SHRINKER-2,gs,o,pal);
						}
						else
						{
							myospal(weapon_xoffset+184-(p->look_ang>>1),
							looking_arc+240-gun_pos,SHRINKER+2,
							16-(sintable[p->random_club_frame&2047]>>10),
							o,0);

							myospal(weapon_xoffset+188-(p->look_ang>>1),
							looking_arc+240-gun_pos,SHRINKER,gs,o,pal);
						}
					}
					else
					{
						if(sprite[p->i].pal != 1)
						{
							weapon_xoffset += rand()&3;
							gun_pos += (rand()&3);
						}

						if(cw == GROW_WEAPON)
						{
							myospal(weapon_xoffset+184-(p->look_ang>>1),
								looking_arc+240-gun_pos,SHRINKER+3+((*kb)&3),-32,
								o,2);

							myospal(weapon_xoffset+188-(p->look_ang>>1),
							looking_arc+240-gun_pos,SHRINKER-1,gs,o,pal);

						}
						else
						{
							myospal(weapon_xoffset+184-(p->look_ang>>1),
							looking_arc+240-gun_pos,SHRINKER+3+((*kb)&3),-32,
							o,0);

							myospal(weapon_xoffset+188-(p->look_ang>>1),
							looking_arc+240-gun_pos,SHRINKER+1,gs,o,pal);
						}
					}
					break;
			}
		}
	}

    displayloogie(snum);

}

#define TURBOTURNTIME (TICRATE/8) // 7
#define NORMALTURN   15
#define PREAMBLETURN 5
#define NORMALKEYMOVE 40
#define MAXVEL       ((NORMALKEYMOVE*2)+10)
#define MAXSVEL      ((NORMALKEYMOVE*2)+10)
#define MAXANGVEL    127
#define MAXHORIZ     127

int32_t myaimmode = 0, myaimstat = 0, omyaimstat = 0;

void getinput(short snum)
{

    short j, daang;
// MED
    ControlInfo info;
    int32 tics;
    boolean running;
    int32 turnamount;
    int32 keymove;
    int32 momx,momy;
    struct player_struct *p;

	// FIX_00038: Improved Mouse accuracy (losses of integer computation)    
	static fixed previousInfoDyaw = 0;
	static fixed previousInfoDpitch = 0;
	static fixed previousInfoDyawSvel = 0;

    momx = momy = 0;
    p = &ps[snum];

    CONTROL_GetInput( &info );

	// FIX_00021: Duke was moving when moving the mouse up/down. Y axis move is disabled.
	info.dz = 0; // remove y axis

    if( (p->gm&MODE_MENU) || (p->gm&MODE_TYPE) || (ud.pause_on && !KB_KeyPressed(sc_Pause)) )
    {
         loc.fvel = vel = 0;
         loc.svel = svel = 0;
         loc.avel = angvel = 0;
         loc.horz = horiz = 0;
         loc.bits = (((int32_t)gamequit)<<26);
         info.dz = info.dyaw = 0;
         return;
    }

    tics = totalclock-lastcontroltime;
    lastcontroltime = totalclock;


    if (MouseAiming)
          myaimmode = ACTION(gamefunc_Mouse_Aiming); // mouse aiming button is temporary
     else
	 {		// mouse aiming button is a toggle
		  omyaimstat = myaimstat; myaimstat = ACTION(gamefunc_Mouse_Aiming);
		  if (myaimstat > omyaimstat)
          {
				myaimmode ^= 1;
                FTA(44+myaimmode,p,1);
          }
	 }

	// FIX_00039: Toggle autoaim between Normal (full) and partial (on bullet weapons only)
	if( ACTION(gamefunc_Auto_Aim) && nHostForceDisableAutoaim == 0)
	{
		ud.auto_aim++;
		ud.auto_aim = ((ud.auto_aim-1)%2)+1;
		sprintf(fta_quotes[103],"AUTOAIM %s", ud.auto_aim?(ud.auto_aim==1)?"BULLET ONLY":"NORMAL (FULL)":"OFF");
		vscrn();	// FIX_00056: Refresh issue w/FPS, small Weapon and custom FTA, when screen resized down
					// This is because we use the same FTA for BULLET and NORMAL text: duke doesn't see we 
					// changed the text and doesnt issue a refresh 
		FTA(103,&ps[screenpeek],1); // Originally reserved for "screen saved". Now used dynamically. 
		CONTROL_ClearAction(gamefunc_Auto_Aim);
	}

    if(multiflag == 1)
    {
        loc.bits =   1<<17;
        loc.bits |=   multiwhat<<18;
		loc.bits |=   multipos<<19;
		multiflag = 0;
        return;
    }

    loc.bits =   ACTION(gamefunc_Jump);
    loc.bits |=   ACTION(gamefunc_Crouch)<<1;
    loc.bits |=   ACTION(gamefunc_Fire)<<2;
    loc.bits |=   ACTION(gamefunc_Aim_Up)<<3;
    loc.bits |=   ACTION(gamefunc_Aim_Down)<<4;
    loc.bits |=   ACTION(gamefunc_Run)<<5;
	loc.bits |=   (ud.auto_aim==2)<<6; // 2 = normal, 1 = bullet only, 0 = disabled (not implemented)
	loc.bits |=   ud.weaponautoswitch<<7;

    j=0;
    if (ACTION(gamefunc_Weapon_1))
       j = 1;
    if (ACTION(gamefunc_Weapon_2))
       j = 2;
    if (ACTION(gamefunc_Weapon_3))
       j = 3;
    if (ACTION(gamefunc_Weapon_4))
       j = 4;
    if (ACTION(gamefunc_Weapon_5))
       j = 5;
    if (ACTION(gamefunc_Weapon_6))
       j = 6;

    if (ACTION(gamefunc_Previous_Weapon))
        j = 11;
    if (ACTION(gamefunc_Next_Weapon))
        j = 12;

    if (!VOLUMEONE)
	{
		if (ACTION(gamefunc_Weapon_7))
	        j = 7;
	    if (ACTION(gamefunc_Weapon_8))
	       j = 8;
	    if (ACTION(gamefunc_Weapon_9))
	       j = 9;
	    if (ACTION(gamefunc_Weapon_10))
	       j = 10;
	}

    loc.bits |=   j<<8;
    loc.bits |=   ACTION(gamefunc_Steroids)<<12;
    loc.bits |=   ACTION(gamefunc_Look_Up)<<13;
    loc.bits |=   ACTION(gamefunc_Look_Down)<<14;
    loc.bits |=   ACTION(gamefunc_NightVision)<<15;
	if(ud.gitdat_mdk)
	{
		if(sprite[ps[myconnectindex].i].extra < max_player_health && ps[myconnectindex].firstaid_amount) // avoid medkit overloading controls
			loc.bits |=   ACTION(gamefunc_MedKit)<<16; 
	}
	else
	{
		loc.bits |=   ACTION(gamefunc_MedKit)<<16; 
	}
    loc.bits |=   ACTION(gamefunc_Center_View)<<18;
	loc.bits |=   ACTION(gamefunc_Holster_Weapon)<<19;
    if(ACTION(gamefunc_Hide_Weapon))
	{
		ud.hideweapon = !ud.hideweapon;
		vscrn(); // FIX_00056: Refresh issue w/FPS, small Weapon and custom FTA, when screen resized down
		CONTROL_ClearAction(gamefunc_Hide_Weapon);
	}
	loc.bits |=   ACTION(gamefunc_Inventory_Left)<<20;
    loc.bits |=   KB_KeyPressed(sc_Pause)<<21;
    loc.bits |=   ACTION(gamefunc_Quick_Kick)<<22;
    loc.bits |=   myaimmode<<23;
    loc.bits |=   ACTION(gamefunc_Holo_Duke)<<24;
    loc.bits |=   ACTION(gamefunc_Jetpack)<<25;
    loc.bits |=   (((int32_t)gamequit)<<26);
    loc.bits |=   ACTION(gamefunc_Inventory_Right)<<27;
    loc.bits |=   ACTION(gamefunc_TurnAround)<<28;
    loc.bits |=   ACTION(gamefunc_Open)<<29;
    loc.bits |=   ACTION(gamefunc_Inventory)<<30;
    loc.bits |=   KB_KeyPressed(sc_Escape)<<31;

    running = ACTION(gamefunc_Run)|ud.auto_run;
    svel = vel = angvel = horiz = 0;

    if( CONTROL_JoystickEnabled )
	{
        if ( running ) 
		{
			info.dz *= 2;
		}
	}

    if( ACTION(gamefunc_Strafe) )
	{
		svel = -(info.dyaw+previousInfoDyawSvel)/8;
	}
    else
	{ 
		angvel = (info.dyaw+previousInfoDyaw)/64; 
	}

	previousInfoDyaw = (previousInfoDyaw+info.dyaw)%64; // % xduke: dont waste mouse tics
	previousInfoDyawSvel = (previousInfoDyawSvel+info.dyaw)%8;

	// svel -= info.dx;
	svel = -info.dx>>6; // This helps the analog feel a bit.

    vel = -info.dz>>6;

	// Account for which mode we're in. (1, 2 or 7)
	switch(ControllerType)
	{
		case controltype_keyboardandjoystick:

		case controltype_joystickandmouse:

			if(CONTROL_JoystickEnabled)
			{
				if(ud.mouseflip)
				{
					horiz = -(info.dpitch+previousInfoDpitch)/(314-128);
				}
				else
				{
					horiz = (info.dpitch+previousInfoDpitch)/(314-128);
				}
				horiz = (horiz>=0)?horiz+1:horiz; // xduke: fix assymetry (speed of 2 is like -1)
				previousInfoDpitch = (previousInfoDpitch+info.dpitch)%(314-128);
				info.dpitch = 0;
			}
			break;

		default:
			// If Mouse aim active
			if( myaimmode )
			{        
				//
				//
				if(ud.mouseflip)
				{
					horiz = -(info.dpitch+previousInfoDpitch)/(314-128);
				}
				else
				{
					horiz = (info.dpitch+previousInfoDpitch)/(314-128);
				}
				horiz = (horiz>=0)?horiz+1:horiz; // xduke: fix assymetry (speed of 2 is like -1)
				previousInfoDpitch = (previousInfoDpitch+info.dpitch)%(314-128);
				info.dpitch = 0;         
			}
			break;
	}



    if (running)
    {
        turnamount = NORMALTURN<<1;
        keymove = NORMALKEYMOVE<<1;
    }
    else
    {
        turnamount = NORMALTURN;
        keymove = NORMALKEYMOVE;
    }

    if (ACTION(gamefunc_Strafe))
    {
        if ( ACTION(gamefunc_Turn_Left))
           {
           svel -= -keymove;
           }
        if ( ACTION(gamefunc_Turn_Right))
           {
           svel -= keymove;
           }
    }
	else
    {
        if ( ACTION(gamefunc_Turn_Left))
           {
           turnheldtime += tics;
           if (turnheldtime>=TURBOTURNTIME)
              {
              angvel -= turnamount;
              }
           else
              {
              angvel -= PREAMBLETURN;
              }
           }
        else if ( ACTION(gamefunc_Turn_Right))
           {
           turnheldtime += tics;
           if (turnheldtime>=TURBOTURNTIME)
              {
              angvel += turnamount;
              }
           else
              {
              angvel += PREAMBLETURN;
              }
           }
        else
           {
           turnheldtime=0;
           }
    }

    if ( ACTION( gamefunc_Strafe_Left ) )
        {
        svel += keymove;
        angvel = (info.dyaw+previousInfoDyaw)/64;
        }

    if ( ACTION( gamefunc_Strafe_Right ) )
        {
        svel += -keymove;
        angvel = (info.dyaw+previousInfoDyaw)/64;
        }

    if ( ACTION(gamefunc_Move_Forward) )
        vel += keymove;

    if ( ACTION(gamefunc_Move_Backward) )
        vel += -keymove;

    if(vel < -MAXVEL) vel = -MAXVEL;
    if(vel > MAXVEL) vel = MAXVEL;
    if(svel < -MAXSVEL) svel = -MAXSVEL;
    if(svel > MAXSVEL) svel = MAXSVEL;
    if(angvel < -MAXANGVEL) angvel = -MAXANGVEL;
    if(angvel > MAXANGVEL) angvel = MAXANGVEL;
    if(horiz < -MAXHORIZ) horiz = -MAXHORIZ;
    if(horiz > MAXHORIZ) horiz = MAXHORIZ;

    if(ud.scrollmode && ud.overhead_on)
    {
        ud.folfvel = vel;
        ud.folavel = angvel;
        loc.fvel = 0;
        loc.svel = 0;
        loc.avel = 0;
        loc.horz = 0;
        return;
    }

    if( numplayers > 1 )
        daang = myang;
    else daang = p->ang;

    momx = mulscale9(vel,sintable[(daang+2560)&2047]);
    momy = mulscale9(vel,sintable[(daang+2048)&2047]);

    momx += mulscale9(svel,sintable[(daang+2048)&2047]);
    momy += mulscale9(svel,sintable[(daang+1536)&2047]);

    momx += fricxv;
    momy += fricyv;
	
	loc.fvel = momx;
	loc.svel = momy;
    loc.avel = angvel;
//	if(loc.avel)
//		printf("getinput loc.avel=%d\n", loc.avel);
    loc.horz = horiz;
}


uint8_t  doincrements(struct player_struct *p)
{
    int32_t snum;

    snum = sprite[p->i].yvel;
//    j = sync[snum].avel;
//    p->weapon_ang = -(j/5);

    p->player_par++;

    if(p->invdisptime > 0)
        p->invdisptime--;

    if(p->tipincs > 0) p->tipincs--;

    if(p->last_pissed_time > 0 )
    {
        p->last_pissed_time--;

        if( p->last_pissed_time == (26*219) )
        {
            spritesound(FLUSH_TOILET,p->i);
            if(snum == screenpeek || ud.coop == 1)
                spritesound(DUKE_PISSRELIEF,p->i);
        }

        if( p->last_pissed_time == (26*218) )
        {
            p->holster_weapon = 0;
            p->weapon_pos = 10;
        }
    }

    if(p->crack_time > 0)
    {
        p->crack_time--;
        if(p->crack_time == 0)
        {
            p->knuckle_incs = 1;
            p->crack_time = 777;
        }
    }

    if( p->steroids_amount > 0 && p->steroids_amount < 400)
    {
        p->steroids_amount--;
        if(p->steroids_amount == 0)
            checkavailinven(p);
        if( !(p->steroids_amount&7) )
            if(snum == screenpeek || ud.coop == 1)
                spritesound(DUKE_HARTBEAT,p->i);
    }

    if(p->heat_on && p->heat_amount > 0)
    {
        p->heat_amount--;
        if( p->heat_amount == 0 )
        {
            p->heat_on = 0;
            checkavailinven(p);
            spritesound(NITEVISION_ONOFF,p->i);
            setpal(p);
        }
    }

    if( p->holoduke_on >= 0 )
    {
        p->holoduke_amount--;
        if(p->holoduke_amount <= 0)
        {
            spritesound(TELEPORTER,p->i);
            p->holoduke_on = -1;
            checkavailinven(p);
        }
    }

    if( p->jetpack_on && p->jetpack_amount > 0 )
    {
        p->jetpack_amount--;
        if(p->jetpack_amount <= 0)
        {
            p->jetpack_on = 0;
            checkavailinven(p);
            spritesound(DUKE_JETPACK_OFF,p->i);
            stopsound(DUKE_JETPACK_IDLE);
            stopsound(DUKE_JETPACK_ON);
        }
    }

    if(p->quick_kick > 0 && sprite[p->i].pal != 1)
    {
        p->quick_kick--;
        if( p->quick_kick == 8 )
            shoot(p->i,KNEE);
    }

    if(p->access_incs && sprite[p->i].pal != 1)
    {
        p->access_incs++;
        if(sprite[p->i].extra <= 0)
            p->access_incs = 12;
        if(p->access_incs == 12)
        {
            if(p->access_spritenum >= 0)
            {
                checkhitswitch(snum,p->access_spritenum,1);
                switch(sprite[p->access_spritenum].pal)
                {
                    case 0:p->got_access &= (0xffff-0x1);break;
                    case 21:p->got_access &= (0xffff-0x2);break;
                    case 23:p->got_access &= (0xffff-0x4);break;
                }
                p->access_spritenum = -1;
            }
            else
            {
                checkhitswitch(snum,p->access_wallnum,0);
                switch(wall[p->access_wallnum].pal)
                {
                    case 0:p->got_access &= (0xffff-0x1);break;
                    case 21:p->got_access &= (0xffff-0x2);break;
                    case 23:p->got_access &= (0xffff-0x4);break;
                }
            }
        }

        if(p->access_incs > 20)
        {
            p->access_incs = 0;
            p->weapon_pos = 10;
            p->kickback_pic = 0;
        }
    }

    if(p->scuba_on == 0 && sector[p->cursectnum].lotag == 2)
    {
        if(p->scuba_amount > 0)
        {
            p->scuba_on = 1;
            p->inven_icon = 6;
            FTA(76,p,0);
        }
        else
        {
            if(p->airleft > 0)
                p->airleft--;
            else
            {
                p->extra_extra8 += 32;
                if(p->last_extra < (max_player_health>>1) && (p->last_extra&3) == 0)
                    spritesound(DUKE_LONGTERM_PAIN,p->i);
            }
        }
    }
    else if(p->scuba_amount > 0 && p->scuba_on)
    {
        p->scuba_amount--;
        if(p->scuba_amount == 0)
        {
            p->scuba_on = 0;
            checkavailinven(p);
        }
    }

    if(p->knuckle_incs)
    {
        p->knuckle_incs ++;
        if(p->knuckle_incs==10)
        {
            if(totalclock > 1024)
                if(snum == screenpeek || ud.coop == 1)
            {
                if(rand()&1)
                    spritesound(DUKE_CRACK,p->i);
                else spritesound(DUKE_CRACK2,p->i);
            }
            spritesound(DUKE_CRACK_FIRST,p->i);
        }
        else if( p->knuckle_incs == 22 || (sync[snum].bits&(1<<2)))
            p->knuckle_incs=0;

        return 1;
    }
    return 0;
}

short weapon_sprites[MAX_WEAPONS] = { KNEE, FIRSTGUNSPRITE, SHOTGUNSPRITE,
        CHAINGUNSPRITE, RPGSPRITE, HEAVYHBOMB, SHRINKERSPRITE, DEVISTATORSPRITE,
        TRIPBOMBSPRITE, FREEZESPRITE, HEAVYHBOMB, SHRINKERSPRITE};

void checkweapons(struct player_struct *p)
{
    short cw;

    cw = p->curr_weapon;

    if(cw < 1 || cw >= MAX_WEAPONS) return;

    if(cw)
    {
        if(TRAND&1)
            spawn(p->i,weapon_sprites[cw]);
        else switch(cw)
        {
            case RPG_WEAPON:
            case HANDBOMB_WEAPON:
                spawn(p->i,EXPLOSION2);
                break;
        }
    }
}

void processinput(short snum)
{
    int32_t j, i, k, doubvel, fz, cz, hz, lz, truefdist, x, y;
    uint8_t  shrunk;
    uint32_t sb_snum;
    short psect, psectlotag,*kb, tempsect, pi;
    struct player_struct *p;
    spritetype *s;
    char text[512];

    p = &ps[snum];
    pi = p->i;
    s = &sprite[pi];

    kb = &p->kickback_pic;

    if(p->cheat_phase <= 0) sb_snum = sync[snum].bits;
    else sb_snum = 0;

    psect = p->cursectnum;
    if(psect == -1)
    {
        if(s->extra > 0 && ud.clipping == 0)
        {
            quickkill(p);
            spritesound(SQUISHED,pi);
        }
        psect = 0;
    }

    psectlotag = sector[psect].lotag;
    p->spritebridge = 0;

    shrunk = (s->yrepeat < 32);
    getzrange(p->posx,p->posy,p->posz,psect,&cz,&hz,&fz,&lz,163L,CLIPMASK0);

    j = getflorzofslope(psect,p->posx,p->posy);

    p->truefz = j;
    p->truecz = getceilzofslope(psect,p->posx,p->posy);

    truefdist = klabs(p->posz-j);
    if( (lz&49152) == 16384 && psectlotag == 1 && truefdist > PHEIGHT+(16<<8) )
        psectlotag = 0;

    hittype[pi].floorz = fz;
    hittype[pi].ceilingz = cz;

    p->ohoriz = p->horiz;
    p->ohorizoff = p->horizoff;

    if( p->aim_mode == 0 && p->on_ground && psectlotag != 2 && (sector[psect].floorstat&2) )
    {
          x = p->posx+(sintable[(p->ang+512)&2047]>>5);
          y = p->posy+(sintable[p->ang&2047]>>5);
          tempsect = psect;
          updatesector(x,y,&tempsect);
          if (tempsect >= 0)
          {
              k = getflorzofslope(psect,x,y);
              if (psect == tempsect)
                  p->horizoff += mulscale16(j-k,160);
              else if (klabs(getflorzofslope(tempsect,x,y)-k) <= (4<<8))
                  p->horizoff += mulscale16(j-k,160);
          }
     }
     if (p->horizoff > 0) p->horizoff -= ((p->horizoff>>3)+1);
     else if (p->horizoff < 0) p->horizoff += (((-p->horizoff)>>3)+1);

    if( hz >= 0 && (hz&49152) == 49152)
    {
        hz &= (MAXSPRITES-1);

        if(sprite[hz].statnum == 1 && sprite[hz].extra >= 0)
        {
            hz = 0;
            cz = p->truecz;
        }
    }

    if(lz >= 0 && (lz&49152) == 49152)
    {
        j = lz&(MAXSPRITES-1);

        if( (sprite[j].cstat&33) == 33 )
        {
            psectlotag = 0;
            p->footprintcount = 0;
            p->spritebridge = 1;
        }
        else if(badguy(&sprite[j]) && sprite[j].xrepeat > 24 && klabs(s->z-sprite[j].z) < (84<<8) )
        {
            j = getangle(sprite[j].x-p->posx,sprite[j].y-p->posy);
            p->posxv -= sintable[(j+512)&2047]<<4;
            p->posyv -= sintable[j&2047]<<4;
        }
    }


    if ( s->extra > 0 ) incur_damage( p );
    else
    {
        s->extra = 0;
        p->shield_amount = 0;
    }

    p->last_extra = s->extra;

    if(p->loogcnt > 0) p->loogcnt--;
    else p->loogcnt = 0;

    if(p->fist_incs)
    {
        p->fist_incs++;
        if(p->fist_incs == 28)
        {
            if(ud.recstat == 1) closedemowrite();
            sound(PIPEBOMB_EXPLODE);
            p->pals[0] = 64;
            p->pals[1] = 64;
            p->pals[2] = 64;
            p->pals_time = 48;
        }
        if(p->fist_incs > 42)
        {
            if(p->buttonpalette && ud.from_bonus == 0)
            {
                ud.from_bonus = ud.level_number+1;
                if(ud.secretlevel > 0 && ud.secretlevel < 12) ud.level_number = ud.secretlevel-1;
                ud.m_level_number = ud.level_number;
            }
            else
            {
                if(ud.from_bonus)
                {
                    ud.level_number = ud.from_bonus;
                    ud.m_level_number = ud.level_number;
                    ud.from_bonus = 0;
                }
                else
                {
                    if(ud.level_number == ud.secretlevel && ud.from_bonus > 0 )
                        ud.level_number = ud.from_bonus;
                    else ud.level_number++;

                    if(ud.level_number > 10) ud.level_number = 0;
                    ud.m_level_number = ud.level_number;

                }
            }
            for(i=connecthead;i>=0;i=connectpoint2[i])
                ps[i].gm = MODE_EOL;
            p->fist_incs = 0;

            return;
        }
    }

    if(p->timebeforeexit > 1 && p->last_extra > 0)
    {
        p->timebeforeexit--;
        if(p->timebeforeexit == 26*5)
        {
            FX_StopAllSounds();
            clearsoundlocks();
            if(p->customexitsound >= 0)
            {
                sound(p->customexitsound);
                FTA(102,p,0);
            }
        }
        else if(p->timebeforeexit == 1)
        {
            for(i=connecthead;i>=0;i=connectpoint2[i])
                ps[i].gm = MODE_EOL;
            if(ud.from_bonus)
            {
                ud.level_number = ud.from_bonus;
                ud.m_level_number = ud.level_number;
                ud.from_bonus = 0;
            }
            else
            {
                ud.level_number++;
                ud.m_level_number = ud.level_number;
            }
            return;
        }
    }

    if(p->pals_time > 0)
        p->pals_time--;

    if(p->fta > 0)
    {
        p->fta--;
        if(p->fta == 0)
        {
            pub = NUMPAGES;
            pus = NUMPAGES;
            p->ftq = 0;
        }
    }

    if( s->extra <= 0 )
    {
        if(p->dead_flag == 0)
        {
            if(s->pal != 1)
            {
                p->pals[0] = 63;
                p->pals[1] = 0;
                p->pals[2] = 0;
                p->pals_time = 63;
                p->posz -= (16<<8);
                s->z -= (16<<8);
            }

            if(ud.recstat == 1 && ud.multimode < 2)
                closedemowrite();

            if(s->pal != 1)
                p->dead_flag = (512-((TRAND&1)<<10)+(TRAND&255)-512)&2047;

            p->jetpack_on = 0;
            p->holoduke_on = -1;

            stopsound(DUKE_JETPACK_IDLE);
            if(p->scream_voice > FX_Ok)
            {
                FX_StopSound(p->scream_voice);
                testcallback(DUKE_SCREAM);
                p->scream_voice = FX_Ok;
            }

            if( s->pal != 1 && (s->cstat&32768) == 0) s->cstat = 0;

            if( ud.multimode > 1 && ( s->pal != 1 || (s->cstat&32768) ) )
            {
                if(p->frag_ps != snum)
                {
                    ps[p->frag_ps].frag++;
                    frags[p->frag_ps][snum]++;

                    if( ud.user_name[p->frag_ps][0] != 0)
                    {
                        if(snum == screenpeek)
                        {
                            sprintf(&fta_quotes[115][0],"KILLED BY %s",&ud.user_name[p->frag_ps][0]);
                            FTA(115,p,1);
                        }
                        else if(screenpeek == p->frag_ps)
						// FIX_00076: Added default names for bots + fixed a "killed <name>" bug in Fakeplayers with AI
                        {
                            sprintf(&fta_quotes[116][0],"KILLED %s",&ud.user_name[snum][0]);
                            FTA(116,&ps[p->frag_ps],1);
                        }
                    }
                    else
                    {
                        if(snum == screenpeek)
                        {
                            sprintf(&fta_quotes[115][0],"KILLED BY PLAYER %d",1+p->frag_ps);
                            FTA(115,p,1);
                        }
                        else if(screenpeek == p->frag_ps)
                        {
                            sprintf(&fta_quotes[116][0],"KILLED PLAYER %d",1+snum);
                            FTA(116,&ps[p->frag_ps],1);
                        }
                    }
                }
                else p->fraggedself++;

                if(myconnectindex == connecthead)
                {
                    sprintf(text,"frag %d killed %d\n",p->frag_ps+1,snum+1);
                    sendscore(text);
//                    printf(tempbuf);
                }

                p->frag_ps = snum;
                pus = NUMPAGES;
            }
        }

        if( psectlotag == 2 )
        {
            if(p->on_warping_sector == 0)
            {
                if( klabs(p->posz-fz) > (PHEIGHT>>1))
                    p->posz += 348;
            }
            else
            {
                s->z -= 512;
                s->zvel = -348;
            }

            clipmove(&p->posx,&p->posy,
                &p->posz,&p->cursectnum,
                0,0,164L,(4L<<8),(4L<<8),CLIPMASK0);
//            p->bobcounter += 32;
        }

		p->oposx = p->posx;
		p->oposy = p->posy;
		p->oposz = p->posz;
		p->oang = p->ang;
		p->opyoff = p->pyoff;

        p->horiz = 100;
        p->horizoff = 0;

        updatesector(p->posx,p->posy,&p->cursectnum);

        pushmove(&p->posx,&p->posy,&p->posz,&p->cursectnum,128L,(4L<<8),(20L<<8),CLIPMASK0);

        if( fz > cz+(16<<8) && s->pal != 1)
            p->rotscrnang = (p->dead_flag + ( (fz+p->posz)>>7))&2047;

        p->on_warping_sector = 0;

        return;
    }

    if(p->transporter_hold > 0)
    {
        p->transporter_hold--;
        if(p->transporter_hold == 0 && p->on_warping_sector)
            p->transporter_hold = 2;
    }
    if(p->transporter_hold < 0)
        p->transporter_hold++;

    if(p->newowner >= 0)
    {
        i = p->newowner;
		p->posx = SX;
        p->posy = SY;
        p->posz = SZ;
        p->ang =  SA;
        p->posxv = p->posyv = s->xvel = 0;
        p->look_ang = 0;
        p->rotscrnang = 0;
        doincrements(p);

        if(p->curr_weapon == HANDREMOTE_WEAPON) goto SHOOTINCODE;

        return;
    }

	p->weaponautoswitch = (sb_snum&(1<<7))?1:0;
	p->auto_aim = (sb_snum&(1<<6))?2:1; // 2 == normal == full; 1 == partial; 0 = none (not implemented)

	doubvel = TICSPERFRAME;

    if (p->rotscrnang > 0) p->rotscrnang -= ((p->rotscrnang>>1)+1);
    else if (p->rotscrnang < 0) p->rotscrnang += (((-p->rotscrnang)>>1)+1);

    p->look_ang -= (p->look_ang>>2);

	// 1<<6: toggle ud.auto_aim
	if(	((ud.playing_demo_rev == BYTEVERSION_27 ||
		ud.playing_demo_rev == BYTEVERSION_28 || 
		ud.playing_demo_rev == BYTEVERSION_116 || 
		ud.playing_demo_rev == BYTEVERSION_117) &&
		sb_snum&(1<<6)) ||
		(ACTION(gamefunc_Look_Left) && (p->gm&MODE_GAME) && 
		!(p->gm&MODE_MENU) && !(p->gm&MODE_TYPE) && !(ud.pause_on) && (ud.recstat != 2)))
	{
		p->look_ang -= 152;
		p->rotscrnang += 24;
	}

	// 1<<7 : ANTIWEAPONSWITCH
	if(	((ud.playing_demo_rev == BYTEVERSION_27 ||
		ud.playing_demo_rev == BYTEVERSION_28 || 
		ud.playing_demo_rev == BYTEVERSION_116 || 
		ud.playing_demo_rev == BYTEVERSION_117) &&
		sb_snum&(1<<7)) ||
		(
         ACTION(gamefunc_Look_Right) && (p->gm&MODE_GAME) &&
		!(p->gm&MODE_MENU) && !(p->gm&MODE_TYPE) && !(ud.pause_on) && (ud.recstat != 2)
         )
         )
	{
		p->look_ang += 152;
		p->rotscrnang -= 24;
	}

	if(p->on_crane >= 0)
        goto HORIZONLY;

    j = ksgn(sync[snum].avel);
    /*
    if( j && ud.screen_tilting == 2)
    {
        k = 4;
        if(sb_snum&(1<<5)) k <<= 2;
        p->rotscrnang -= k*j;
        p->look_ang += k*j;
    }
    */

    if( s->xvel < 32 || p->on_ground == 0 || p->bobcounter == 1024 )
    {
        if( (p->weapon_sway&2047) > (1024+96) )
            p->weapon_sway -= 96;
        else if( (p->weapon_sway&2047) < (1024-96) )
            p->weapon_sway += 96;
        else p->weapon_sway = 1024;
    }
    else p->weapon_sway = p->bobcounter;

    s->xvel =
        ksqrt( (p->posx-p->bobposx)*(p->posx-p->bobposx)+(p->posy-p->bobposy)*(p->posy-p->bobposy));
    if(p->on_ground) p->bobcounter += sprite[p->i].xvel>>1;

    if( ud.clipping == 0 && ( sector[p->cursectnum].floorpicnum == MIRROR || p->cursectnum < 0 || p->cursectnum >= MAXSECTORS) )
    {
        p->posx = p->oposx;
        p->posy = p->oposy;
    }
    else
    {
        p->oposx = p->posx;
        p->oposy = p->posy;
    }

    p->bobposx = p->posx;
    p->bobposy = p->posy;

    p->oposz = p->posz;
    p->opyoff = p->pyoff;
    p->oang = p->ang;

    if(p->one_eighty_count < 0)
    {
        p->one_eighty_count += 128;
        p->ang += 128;
    }

    // Shrinking code

    i = 40;

    if( psectlotag == 2)
    {
        p->jumping_counter = 0;

        p->pycount += 32;
        p->pycount &= 2047;
        p->pyoff = sintable[p->pycount]>>7;

        if( Sound[DUKE_UNDERWATER].num == 0 )
            spritesound(DUKE_UNDERWATER,pi);

        if ( sb_snum&1 )
        {
            if(p->poszv > 0) p->poszv = 0;
            p->poszv -= 348;
            if(p->poszv < -(256*6)) p->poszv = -(256*6);
        }
        else if (sb_snum&(1<<1))
        {
            if(p->poszv < 0) p->poszv = 0;
            p->poszv += 348;
            if(p->poszv > (256*6)) p->poszv = (256*6);
        }
        else
        {
            if(p->poszv < 0)
            {
                p->poszv += 256;
                if(p->poszv > 0)
                    p->poszv = 0;
            }
            if(p->poszv > 0)
            {
                p->poszv -= 256;
                if(p->poszv < 0)
                    p->poszv = 0;
            }
        }

        if(p->poszv > 2048)
            p->poszv >>= 1;

        p->posz += p->poszv;

        if(p->posz > (fz-(15<<8)) )
            p->posz += ((fz-(15<<8))-p->posz)>>1;

        if(p->posz < (cz+(4<<8)) )
        {
            p->posz = cz+(4<<8);
            p->poszv = 0;
        }

        if( p->scuba_on && (TRAND&255) < 8 )
        {
            j = spawn(pi,WATERBUBBLE);
            sprite[j].x +=
                sintable[(p->ang+512+64-(global_random&128))&2047]>>6;
            sprite[j].y +=
                sintable[(p->ang+64-(global_random&128))&2047]>>6;
            sprite[j].xrepeat = 3;
            sprite[j].yrepeat = 2;
            sprite[j].z = p->posz+(8<<8);
        }
    }

    else if(p->jetpack_on)
    {
        p->on_ground = 0;
        p->jumping_counter = 0;
        p->hard_landing = 0;
        p->falling_counter = 0;

        p->pycount += 32;
        p->pycount &= 2047;
        p->pyoff = sintable[p->pycount]>>7;

        if(p->jetpack_on < 11)
        {
            p->jetpack_on++;
            p->posz -= (p->jetpack_on<<7); //Goin up
        }
        else if(p->jetpack_on == 11 && Sound[DUKE_JETPACK_IDLE].num < 1)
            spritesound(DUKE_JETPACK_IDLE,pi);

        if(shrunk) j = 512;
        else j = 2048;

        if ( sb_snum&1 )                            //A (soar high)
        {
            p->posz -= j;
            p->crack_time = 777;
        }

        if (sb_snum&(1<<1))                            //Z (soar low)
        {
            p->posz += j;
            p->crack_time = 777;
        }

        if( shrunk == 0 && (psectlotag == 0 || psectlotag == 2)) k = 32;
        else k = 16;

        if( psectlotag != 2 && p->scuba_on == 1 )
            p->scuba_on = 0;

        if(p->posz > (fz-(k<<8)) )
            p->posz += ((fz-(k<<8))-p->posz)>>1;
        if(p->posz < (hittype[pi].ceilingz+(18<<8)) )
            p->posz = hittype[pi].ceilingz+(18<<8);

    }
    else if( psectlotag != 2 )
    {
        if(p->airleft != 15*26)
            p->airleft = 15*26; //Aprox twenty seconds.

        if(p->scuba_on == 1)
            p->scuba_on = 0;

        if( psectlotag == 1 && p->spritebridge == 0)
        {
            if(shrunk == 0)
            {
                i = 34;
                p->pycount += 32;
                p->pycount &= 2047;
                p->pyoff = sintable[p->pycount]>>6;
            }
            else i = 12;

            if(shrunk == 0 && truefdist <= PHEIGHT)
            {
                if(p->on_ground == 1)
                {
                    if( p->dummyplayersprite == -1 )
                        p->dummyplayersprite =
                            spawn(pi,PLAYERONWATER);

                    p->footprintcount = 6;
                    if(sector[p->cursectnum].floorpicnum == FLOORSLIME)
                        p->footprintpal = 8;
                    else p->footprintpal = 0;
                    p->footprintshade = 0;
                }
            }
        }
        else
        {
            if(p->footprintcount > 0 && p->on_ground)
                if( (sector[p->cursectnum].floorstat&2) != 2 )
            {
                for(j=headspritesect[psect];j>=0;j=nextspritesect[j])
                    if( sprite[j].picnum == FOOTPRINTS || sprite[j].picnum == FOOTPRINTS2 || sprite[j].picnum == FOOTPRINTS3 || sprite[j].picnum == FOOTPRINTS4 )
                        if (klabs(sprite[j].x-p->posx) < 384)
                            if (klabs(sprite[j].y-p->posy) < 384)
                                break;
                if(j < 0)
                {
                    p->footprintcount--;
                    if( sector[p->cursectnum].lotag == 0 && sector[p->cursectnum].hitag == 0 )
                    {
                        switch(TRAND&3)
                        {
                            case 0:  j = spawn(pi,FOOTPRINTS); break;
                            case 1:  j = spawn(pi,FOOTPRINTS2); break;
                            case 2:  j = spawn(pi,FOOTPRINTS3); break;
                            default: j = spawn(pi,FOOTPRINTS4); break;
                        }
                        sprite[j].pal = p->footprintpal;
                        sprite[j].shade = p->footprintshade;
                    }
                }
            }
        }

        if(p->posz < (fz-(i<<8)) ) //falling
        {
            if( (sb_snum&3) == 0 && p->on_ground && (sector[psect].floorstat&2) && p->posz >= (fz-(i<<8)-(16<<8) ) )
                p->posz = fz-(i<<8);
            else
            {
                p->on_ground = 0;
                p->poszv += (gc+80); // (TICSPERFRAME<<6);
                if(p->poszv >= (4096+2048)) p->poszv = (4096+2048);
                if(p->poszv > 2400 && p->falling_counter < 255)
                {
                    p->falling_counter++;
                    if( p->falling_counter == 38 )
                        p->scream_voice = spritesound(DUKE_SCREAM,pi);
                }

                if( (p->posz+p->poszv) >= (fz-(i<<8)) ) // hit the ground
                    if(sector[p->cursectnum].lotag != 1)
                    {
                        if( p->falling_counter > 62 ) quickkill(p);

                        else if( p->falling_counter > 9 )
                        {
                            j = p->falling_counter;
                            s->extra -= j-(TRAND&3);
                            if(s->extra <= 0)
                            {
                                spritesound(SQUISHED,pi);
                                p->pals[0] = 63;
                                p->pals[1] = 0;
                                p->pals[2] = 0;
                                p->pals_time = 63;
                            }
                            else
                            {
                                spritesound(DUKE_LAND,pi);
                                spritesound(DUKE_LAND_HURT,pi);
                            }

                            p->pals[0] = 16;
                            p->pals[1] = 0;
                            p->pals[2] = 0;
                            p->pals_time = 32;
                        }
                        else if(p->poszv > 2048) spritesound(DUKE_LAND,pi);
                    }
            }
        }

        else
        {
            p->falling_counter = 0;
            if(p->scream_voice > FX_Ok)
            {
                FX_StopSound(p->scream_voice);
                p->scream_voice = FX_Ok;
            }

            if(psectlotag != 1 && psectlotag != 2 && p->on_ground == 0 && p->poszv > (6144>>1))
                p->hard_landing = p->poszv>>10;

            p->on_ground = 1;

            if( i==40 )
            {
                //Smooth on the ground

                k = ((fz-(i<<8))-p->posz)>>1;
                if( klabs(k) < 256 ) k = 0;
                p->posz += k;
                p->poszv -= 768;
                if(p->poszv < 0) p->poszv = 0;
            }
            else if(p->jumping_counter == 0)
            {
                p->posz += ((fz-(i<<7))-p->posz)>>1; //Smooth on the water
                if(p->on_warping_sector == 0 && p->posz > fz-(16<<8))
                {
                    p->posz = fz-(16<<8);
                    p->poszv >>= 1;
                }
            }

            p->on_warping_sector = 0;

            if( (sb_snum&2) )
            {
                p->posz += (2048+768);
                p->crack_time = 777;
            }

            if( (sb_snum&1) == 0 && p->jumping_toggle == 1)
                p->jumping_toggle = 0;

            else if( (sb_snum&1) && p->jumping_toggle == 0 )
            {
                if( p->jumping_counter == 0 )
                    if( (fz-cz) > (56<<8) )
                    {
                        p->jumping_counter = 1;
                        p->jumping_toggle = 1;
                    }
            }

            if( p->jumping_counter && (sb_snum&1) == 0 )
                p->jumping_toggle = 0;
        }

        if(p->jumping_counter)
        {
            if( (sb_snum&1) == 0 && p->jumping_toggle == 1)
                p->jumping_toggle = 0;

            if( p->jumping_counter < (1024+256) )
            {
                if(psectlotag == 1 && p->jumping_counter > 768)
                {
                    p->jumping_counter = 0;
                    p->poszv = -512;
                }
                else
                {
                    p->poszv -= (sintable[(2048-128+p->jumping_counter)&2047])/12;
                    p->jumping_counter += 180;
                    p->on_ground = 0;
                }
            }
            else
            {
                p->jumping_counter = 0;
                p->poszv = 0;
            }
        }

        p->posz += p->poszv;

        if(p->posz < (cz+(4<<8)))
        {
            p->jumping_counter = 0;
            if(p->poszv < 0)
                p->posxv = p->posyv = 0;
            p->poszv = 128;
            p->posz = cz+(4<<8);
        }
    }

    //Do the quick lefts and rights

    if ( p->fist_incs ||
         p->transporter_hold > 2 ||
         p->hard_landing ||
         p->access_incs > 0 ||
         p->knee_incs > 0 ||
         (p->curr_weapon == TRIPBOMB_WEAPON &&
          *kb > 1 &&
          *kb < 4 ) )
    {
        doubvel = 0;
        p->posxv = 0;
        p->posyv = 0;
    }
    else if ( sync[snum].avel )          //p->ang += syncangvel * constant
    {                         //ENGINE calculates angvel for you
        int32_t tempang;

        tempang = sync[snum].avel<<1;

        if( psectlotag == 2 ) p->angvel =(tempang-(tempang>>3))*ksgn(doubvel);
        else p->angvel = tempang*ksgn(doubvel);

        p->ang += p->angvel;
        p->ang &= 2047;
        p->crack_time = 777;
		}

    if(p->spritebridge == 0)
    {
        j = sector[s->sectnum].floorpicnum;

        if( j == PURPLELAVA || sector[s->sectnum].ceilingpicnum == PURPLELAVA )
        {
            if(p->boot_amount > 0)
            {
                p->boot_amount--;
                p->inven_icon = 7;
                if(p->boot_amount <= 0)
                    checkavailinven(p);
            }
            else
            {
                if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                    spritesound(DUKE_LONGTERM_PAIN,pi);
                p->pals[0] = 0; p->pals[1] = 8; p->pals[2] = 0;
                p->pals_time = 32;
                s->extra--;
            }
        }

        k = 0;

        if(p->on_ground && truefdist <= PHEIGHT+(16<<8))
        {
            switch(j)
            {
                case HURTRAIL:
                    if( rnd(32) )
                    {
                        if(p->boot_amount > 0)
                            k = 1;
                        else
                        {
                            if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                spritesound(DUKE_LONGTERM_PAIN,pi);
                            p->pals[0] = 64; p->pals[1] = 64; p->pals[2] = 64;
                            p->pals_time = 32;
                            s->extra -= 1+(TRAND&3);
                            if(Sound[SHORT_CIRCUIT].num < 1)
                                spritesound(SHORT_CIRCUIT,pi);
                        }
                    }
                    break;
                case FLOORSLIME:
                    if( rnd(16) )
                    {
                        if(p->boot_amount > 0)
                            k = 1;
                        else
                        {
                            if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                spritesound(DUKE_LONGTERM_PAIN,pi);
                            p->pals[0] = 0; p->pals[1] = 8; p->pals[2] = 0;
                            p->pals_time = 32;
                            s->extra -= 1+(TRAND&3);
                        }
                    }
                    break;
                case FLOORPLASMA:
                    if( rnd(32) )
                    {
                        if( p->boot_amount > 0 )
                            k = 1;
                        else
                        {
                            if(Sound[DUKE_LONGTERM_PAIN].num < 1)
                                spritesound(DUKE_LONGTERM_PAIN,pi);
                            p->pals[0] = 8; p->pals[1] = 0; p->pals[2] = 0;
                            p->pals_time = 32;
                            s->extra -= 1+(TRAND&3);
                        }
                    }
                    break;
            }
        }

        if( k )
        {
            FTA(75,p,0);
            p->boot_amount -= 2;
            if(p->boot_amount <= 0)
                checkavailinven(p);
        }
    }

    if ( p->posxv || p->posyv || sync[snum].fvel || sync[snum].svel )
    {
        p->crack_time = 777;

        k = sintable[p->bobcounter&2047]>>12;

        if(truefdist < PHEIGHT+(8<<8) )
        {
            if( k == 1 || k == 3 )
        {
            if(p->spritebridge == 0 && p->walking_snd_toggle == 0 && p->on_ground)
            {
                switch( psectlotag )
                {
                    case 0:

                        if(lz >= 0 && (lz&(MAXSPRITES-1))==49152 )
                            j = sprite[lz&(MAXSPRITES-1)].picnum;
                        else j = sector[psect].floorpicnum;

                        switch(j)
                        {
                            case PANNEL1:
                            case PANNEL2:
                                spritesound(DUKE_WALKINDUCTS,pi);
                                p->walking_snd_toggle = 1;
                                break;
                        }
                        break;
                    case 1:
                        if((TRAND&1) == 0)
                            spritesound(DUKE_ONWATER,pi);
                        p->walking_snd_toggle = 1;
                        break;
                }
            }
        }
        else{
            if(p->walking_snd_toggle > 0)
            {
              p->walking_snd_toggle --;
            }
        }
        }
        
        if(p->jetpack_on == 0 && p->steroids_amount > 0 && p->steroids_amount < 400)
            doubvel <<= 1;

        p->posxv += ((sync[snum].fvel*doubvel)<<6);
        p->posyv += ((sync[snum].svel*doubvel)<<6);

        if( ( p->curr_weapon == KNEE_WEAPON && *kb > 10 && p->on_ground ) || ( p->on_ground && (sb_snum&2) ) )
        {
            p->posxv = mulscale(p->posxv,dukefriction-0x2000,16);
            p->posyv = mulscale(p->posyv,dukefriction-0x2000,16);
        }
        else
        {
            if(psectlotag == 2)
            {
                p->posxv = mulscale(p->posxv,dukefriction-0x1400,16);
                p->posyv = mulscale(p->posyv,dukefriction-0x1400,16);
            }
            else
            {
                p->posxv = mulscale(p->posxv,dukefriction,16);
                p->posyv = mulscale(p->posyv,dukefriction,16);
            }
        }

        if( abs(p->posxv) < 2048 && abs(p->posyv) < 2048 )
            p->posxv = p->posyv = 0;

        if( shrunk )
        {
            p->posxv =
                mulscale16(p->posxv,dukefriction-(dukefriction>>1)+(dukefriction>>2));
            p->posyv =
                mulscale16(p->posyv,dukefriction-(dukefriction>>1)+(dukefriction>>2));
        }
    }

    HORIZONLY:

        if(psectlotag == 1 || p->spritebridge == 1) i = (4L<<8);
        else i = (20L<<8);

        if(sector[p->cursectnum].lotag == 2) k = 0;
        else k = 1;

        if(ud.clipping)
        {
            j = 0;
            p->posx += p->posxv>>14;
            p->posy += p->posyv>>14;
            updatesector(p->posx,p->posy,&p->cursectnum);
            changespritesect(pi,p->cursectnum);
        }
        else
            j = clipmove(&p->posx,&p->posy,
                &p->posz,&p->cursectnum,
                p->posxv,p->posyv,164L,(4L<<8),i,CLIPMASK0);

        if(p->jetpack_on == 0 && psectlotag != 2 && psectlotag != 1 && shrunk)
            p->posz += 32<<8;

        if(j)
            checkplayerhurt(p,j);

        if(p->jetpack_on == 0)
        {
            if( s->xvel > 16 )
            {
                if( psectlotag != 1 && psectlotag != 2 && p->on_ground )
                {
                    p->pycount += 52;
                    p->pycount &= 2047;
                    p->pyoff =
                        klabs(s->xvel*sintable[p->pycount])/1596;
                }
            }
            else if( psectlotag != 2 && psectlotag != 1 )
                p->pyoff = 0;
        }

        // RBG***
        setsprite(pi,p->posx,p->posy,p->posz+PHEIGHT);

        if( psectlotag < 3 )
        {
            psect = s->sectnum;
            if( ud.clipping == 0 && sector[psect].lotag == 31)
            {
                if( sprite[sector[psect].hitag].xvel && hittype[sector[psect].hitag].temp_data[0] == 0)
                {
                    quickkill(p);
                    return;
                }
            }
        }

        if(truefdist < PHEIGHT && p->on_ground && psectlotag != 1 && shrunk == 0 && sector[p->cursectnum].lotag == 1)
            if( Sound[DUKE_ONWATER].num == 0 )
                spritesound(DUKE_ONWATER,pi);

        if (p->cursectnum != s->sectnum)
            changespritesect(pi,p->cursectnum);

        if(ud.clipping == 0)
            j = ( pushmove(&p->posx,&p->posy,&p->posz,&p->cursectnum,164L,(4L<<8),(4L<<8),CLIPMASK0) < 0 && furthestangle(pi,8) < 512 );
        else j = 0;

        if(ud.clipping == 0)
        {
            if( klabs(hittype[pi].floorz-hittype[pi].ceilingz) < (48<<8) || j )
            {
                if ( !(sector[s->sectnum].lotag&0x8000) && ( isanunderoperator(sector[s->sectnum].lotag) ||
                    isanearoperator(sector[s->sectnum].lotag) ) )
                        activatebysector(s->sectnum,pi);
                if(j)
                {
                    quickkill(p);
                    return;
                }
            }
            else if( klabs(fz-cz) < (32<<8) && isanunderoperator(sector[psect].lotag) )
                activatebysector(psect,pi);
        }

        if( sb_snum&(1<<18) || p->hard_landing)
            p->return_to_center = 9;

        if( sb_snum&(1<<13) )
        {
            p->return_to_center = 9;
            if( sb_snum&(1<<5) ) p->horiz += 12;
            p->horiz += 12;
        }

        else if( sb_snum&(1<<14) )
        {
            p->return_to_center = 9;
            if( sb_snum&(1<<5) ) p->horiz -= 12;
            p->horiz -= 12;
        }

        else if( sb_snum&(1<<3) )
        {
            if( sb_snum&(1<<5) ) p->horiz += 6;
            p->horiz += 6;
        }

        else if( sb_snum&(1<<4) )
        {
            if( sb_snum&(1<<5) ) p->horiz -= 6;
            p->horiz -= 6;
        }
        if(p->return_to_center > 0)
            if( (sb_snum&(1<<13)) == 0 && (sb_snum&(1<<14)) == 0 )
        {
            p->return_to_center--;
            p->horiz += 33-(p->horiz/3);
        }

        if(p->hard_landing > 0)
        {
            p->hard_landing--;
            p->horiz -= (p->hard_landing<<4);
        }

        if(p->aim_mode)
            p->horiz += sync[snum].horz>>1;
        else
        {
             if( p->horiz > 95 && p->horiz < 105) p->horiz = 100;
             if( p->horizoff > -5 && p->horizoff < 5) p->horizoff = 0;
        }

        if(p->horiz > 299) p->horiz = 299;
        else if(p->horiz < -99) p->horiz = -99;

    //Shooting code/changes

    if( p->show_empty_weapon > 0)
    {
        p->show_empty_weapon--;
        if(p->show_empty_weapon == 0)
        {
            if(p->last_full_weapon == GROW_WEAPON)
                p->subweapon |= (1<<GROW_WEAPON);
            else if(p->last_full_weapon == SHRINKER_WEAPON)
                p->subweapon &= ~(1<<GROW_WEAPON);
            addweapon( p, p->last_full_weapon );
            return;
        }
    }

    if(p->knee_incs > 0)
    {
        p->knee_incs++;
        p->horiz -= 48;
        p->return_to_center = 9;
        if(p->knee_incs > 15)
        {
            p->knee_incs = 0;
            p->holster_weapon = 0;
            if(p->weapon_pos < 0)
                p->weapon_pos = -p->weapon_pos;
            if(p->actorsqu >= 0 && dist(&sprite[pi],&sprite[p->actorsqu]) < 1400 )
            {
                guts(&sprite[p->actorsqu],JIBS6,7,myconnectindex);
                spawn(p->actorsqu,BLOODPOOL);
                spritesound(SQUISHED,p->actorsqu);
                switch(sprite[p->actorsqu].picnum)
                {
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
                        if(sprite[p->actorsqu].yvel)
                            operaterespawns(sprite[p->actorsqu].yvel);
                        break;
                }

                if(sprite[p->actorsqu].picnum == APLAYER)
                {
                    quickkill(&ps[sprite[p->actorsqu].yvel]);
                    ps[sprite[p->actorsqu].yvel].frag_ps = snum;
                }
                else if(badguy(&sprite[p->actorsqu]))
                {
                    deletesprite(p->actorsqu);
                    p->actors_killed++;
                }
                else deletesprite(p->actorsqu);
            }
            p->actorsqu = -1;
        }
        else if(p->actorsqu >= 0)
            p->ang += getincangle(p->ang,getangle(sprite[p->actorsqu].x-p->posx,sprite[p->actorsqu].y-p->posy))>>2;
    }

    if( doincrements(p) ) return;

    if(p->weapon_pos != 0)
    {
        if(p->weapon_pos == -9)
        {
            if(p->last_weapon >= 0)
            {
                p->weapon_pos = 10;
//                if(p->curr_weapon == KNEE_WEAPON) *kb = 1;
                p->last_weapon = -1;
            }
            else if(p->holster_weapon == 0)
                p->weapon_pos = 10;
        }
        else p->weapon_pos--;
    }

    // HACKS

    SHOOTINCODE:

    if( p->curr_weapon == SHRINKER_WEAPON || p->curr_weapon == GROW_WEAPON )
        p->random_club_frame += 64; // Glowing

    if(p->rapid_fire_hold == 1)
    {
        if( sb_snum&(1<<2) ) return;
        p->rapid_fire_hold = 0;
    }

    if(shrunk || p->tipincs || p->access_incs)
        sb_snum &= ~(1<<2);
    else if ( shrunk == 0 && (sb_snum&(1<<2)) && (*kb) == 0 && p->fist_incs == 0 &&
         p->last_weapon == -1 && ( p->weapon_pos == 0 || p->holster_weapon == 1 ) )
    {

        p->crack_time = 777;

        if(p->holster_weapon == 1)
        {
            if( p->last_pissed_time <= (26*218) && p->weapon_pos == -9)
            {
                p->holster_weapon = 0;
                p->weapon_pos = 10;
                FTA(74,p,1);
            }
        }
        else switch(p->curr_weapon)
        {
            case HANDBOMB_WEAPON:
                p->hbomb_hold_delay = 0;
                if( p->ammo_amount[HANDBOMB_WEAPON] > 0 )
                    (*kb)=1;
                break;
            case HANDREMOTE_WEAPON:
                p->hbomb_hold_delay = 0;
                (*kb) = 1;
                break;

            case PISTOL_WEAPON:
                if( p->ammo_amount[PISTOL_WEAPON] > 0 )
                {
                    p->ammo_amount[PISTOL_WEAPON]--;
                    (*kb) = 1;
                }
                break;


            case CHAINGUN_WEAPON:
                if( p->ammo_amount[CHAINGUN_WEAPON] > 0 ) // && p->random_club_frame == 0)
                    (*kb)=1;
                break;

            case SHOTGUN_WEAPON:
                if( p->ammo_amount[SHOTGUN_WEAPON] > 0 && p->random_club_frame == 0 )
                    (*kb)=1;
                break;

            case TRIPBOMB_WEAPON:
				if (VOLUMEONE) break;
                if ( p->ammo_amount[TRIPBOMB_WEAPON] > 0 )
                {
                    int32_t sx,sy,sz;
                    short sect,hw,hitsp;

                    hitscan( p->posx, p->posy, p->posz,
                             p->cursectnum, sintable[(p->ang+512)&2047],
                             sintable[p->ang&2047], (100-p->horiz-p->horizoff)*32,
                             &sect, &hw, &hitsp, &sx, &sy, &sz,CLIPMASK1);

                    if(sect < 0 || hitsp >= 0)
                        break;

                    if( hw >= 0 && sector[sect].lotag > 2 )
                        break;

                    if(hw >= 0 && wall[hw].overpicnum >= 0)
                        if(wall[hw].overpicnum == BIGFORCE)
                            break;

                    j = headspritesect[sect];
                    while(j >= 0)
                    {
                        if( sprite[j].picnum == TRIPBOMB &&
                            klabs(sprite[j].z-sz) < (12<<8) && ((sprite[j].x-sx)*(sprite[j].x-sx)+(sprite[j].y-sy)*(sprite[j].y-sy)) < (290*290) )
                                    break;
                        j = nextspritesect[j];
                    }

                    if(j == -1 && hw >= 0 && (wall[hw].cstat&16) == 0 )
                        if( ( wall[hw].nextsector >= 0 && sector[wall[hw].nextsector].lotag <= 2 ) || ( wall[hw].nextsector == -1 && sector[sect].lotag <= 2 ) )
                            if( ( (sx-p->posx)*(sx-p->posx) + (sy-p->posy)*(sy-p->posy) ) < (290*290) )
                    {
                        p->posz = p->oposz;
                        p->poszv = 0;
                        (*kb) = 1;
                    }
                }
                break;

            case SHRINKER_WEAPON:
            case GROW_WEAPON:
				if (VOLUMEONE) break;
                if( p->curr_weapon == GROW_WEAPON )
                {
                    if( p->ammo_amount[GROW_WEAPON] > 0 )
                    {
                        (*kb) = 1;
                        spritesound(EXPANDERSHOOT,pi);
                    }
                }
                else if( p->ammo_amount[SHRINKER_WEAPON] > 0)
                {
                    (*kb) = 1;
                    spritesound(SHRINKER_FIRE,pi);
                }
                break;

            case FREEZE_WEAPON:
				if (VOLUMEONE) break;
                if( p->ammo_amount[FREEZE_WEAPON] > 0 )
                {
                    (*kb) = 1;
                    spritesound(CAT_FIRE,pi);
                }
                break;
            case DEVISTATOR_WEAPON:
				if (VOLUMEONE) break;
				if( p->ammo_amount[DEVISTATOR_WEAPON] > 0 )
                {
                    (*kb) = 1;
                    p->hbomb_hold_delay = !p->hbomb_hold_delay;
                    spritesound(CAT_FIRE,pi);
                }
                break;

            case RPG_WEAPON:
                if ( p->ammo_amount[RPG_WEAPON] > 0)
                    (*kb) = 1;
                break;

            case KNEE_WEAPON:
                if(p->quick_kick == 0) (*kb) = 1;
                break;
        }
    }
    else if((*kb))
    {
        switch( p->curr_weapon )
        {
            case HANDBOMB_WEAPON:

                if( (*kb) == 6 && (sb_snum&(1<<2)) )
                {
                    p->rapid_fire_hold = 1;
                    break;
                }
                (*kb)++;
                if((*kb)==12)
                {
                    p->ammo_amount[HANDBOMB_WEAPON]--;

                    if(p->on_ground && (sb_snum&2) )
                    {
                        k = 15;
                        i = ((p->horiz+p->horizoff-100)*20);
                    }
                    else
                    {
                        k = 140;
                        i = -512-((p->horiz+p->horizoff-100)*20);
                    }

                    j = EGS(p->cursectnum,
                        p->posx+(sintable[(p->ang+512)&2047]>>6),
                        p->posy+(sintable[p->ang&2047]>>6),
                        p->posz,HEAVYHBOMB,-16,9,9,
                        p->ang,(k+(p->hbomb_hold_delay<<5)),i,pi,1);

                    if(k == 15)
                    {
                        sprite[j].yvel = 3;
                        sprite[j].z += (8<<8);
                    }

                    k = hits(pi);
                    if( k < 512 )
                    {
                        sprite[j].ang += 1024;
                        sprite[j].zvel /= 3;
                        sprite[j].xvel /= 3;
                    }

                    p->hbomb_on = 1;

                }
                else if( (*kb) < 12 && (sb_snum&(1<<2)) )
                    p->hbomb_hold_delay++;
                else if( (*kb) > 19 )
                {
                    (*kb) = 0;
                    p->curr_weapon = HANDREMOTE_WEAPON;
                    p->last_weapon = -1;
                    p->weapon_pos = 10;
                }

                break;


            case HANDREMOTE_WEAPON:

                (*kb)++;

                if((*kb) == 2)
                {
                    p->hbomb_on = 0;
                }

                if((*kb) == 10)
                {
                    (*kb) = 0;
                    if(p->ammo_amount[HANDBOMB_WEAPON] > 0)
                        addweapon(p,HANDBOMB_WEAPON);
                    else checkavailweapon(p);
                }
                break;

            case PISTOL_WEAPON:
                if( (*kb)==1)
                {
                    shoot(pi,SHOTSPARK1);
                    spritesound(PISTOL_FIRE,pi);

                    lastvisinc = totalclock+32;
                    p->visibility = 0;
                }
                else if((*kb) == 2)
                    spawn(pi,SHELL);

                (*kb)++;

                if((*kb) >= 5)
                {
                    if( p->ammo_amount[PISTOL_WEAPON] <= 0 || (p->ammo_amount[PISTOL_WEAPON]%12) )
                    {
                        (*kb)=0;
                        checkavailweapon(p);
                    }
                    else
                    {
                        switch((*kb))
                        {
                            case 5:
                                spritesound(EJECT_CLIP,pi);
                                break;
                            case 8:
                                spritesound(INSERT_CLIP,pi);
                                break;
                        }
                    }
                }

                if((*kb) == 27)
                {
                    (*kb) = 0;
                    checkavailweapon(p);
                }

                break;

            case SHOTGUN_WEAPON:

                (*kb)++;

                if(*kb == 4)
                {
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);
                    shoot(pi,SHOTGUN);

                    p->ammo_amount[SHOTGUN_WEAPON]--;

                    spritesound(SHOTGUN_FIRE,pi);

                    lastvisinc = totalclock+32;
                    p->visibility = 0;
                }

                switch(*kb)
                {
                    case 13:
                        checkavailweapon(p);
                        break;
                    case 15:
                        spritesound(SHOTGUN_COCK,pi);
                        break;
                    case 17:
                    case 20:
                        p->kickback_pic++;
                        break;
                    case 24:
                        j = spawn(pi,SHOTGUNSHELL);
                        sprite[j].ang += 1024;
                        ssp(j,CLIPMASK0);
                        sprite[j].ang += 1024;
                        p->kickback_pic++;
                        break;
                    case 31:
                        *kb = 0;
                        return;
                }
                break;

            case CHAINGUN_WEAPON:

                (*kb)++;

                if( *(kb) <= 12 )
                {
                    if( ((*(kb))%3) == 0 )
                    {
                        p->ammo_amount[CHAINGUN_WEAPON]--;

                        if( (*(kb)%3) == 0 )
                        {
                            j = spawn(pi,SHELL);

                            sprite[j].ang += 1024;
                            sprite[j].ang &= 2047;
                            sprite[j].xvel += 32;
                            sprite[j].z += (3<<8);
                            ssp(j,CLIPMASK0);
                        }

                        spritesound(CHAINGUN_FIRE,pi);
                        shoot(pi,CHAINGUN);
                        lastvisinc = totalclock+32;
                        p->visibility = 0;
                        checkavailweapon(p);

                        if( ( sb_snum&(1<<2) ) == 0 )
                        {
                            *kb = 0;
                            break;
                        }
                    }
                }
                else if((*kb) > 10)
                {
                    if( sb_snum&(1<<2) ) *kb = 1;
                    else *kb = 0;
                }

                break;

            case SHRINKER_WEAPON:
            case GROW_WEAPON:

                if(p->curr_weapon == GROW_WEAPON)
                {
                    if((*kb) > 3)
                    {
                        *kb = 0;
                        if( screenpeek == snum ) pus = 1;
                        p->ammo_amount[GROW_WEAPON]--;
                        shoot(pi,GROWSPARK);

                        p->visibility = 0;
                        lastvisinc = totalclock+32;
                        checkavailweapon(p);
                    }
                    else (*kb)++;
                }
                else
                {
                    if( (*kb) > 10)
                    {
                        (*kb) = 0;

                        p->ammo_amount[SHRINKER_WEAPON]--;
                        shoot(pi,SHRINKER);

                        p->visibility = 0;
                        lastvisinc = totalclock+32;
                        checkavailweapon(p);
                    }
                    else (*kb)++;
                }
                break;

            case DEVISTATOR_WEAPON:
                if(*kb)
                {
                    (*kb)++;

                    if( (*kb) & 1 )
                    {
                        p->visibility = 0;
                        lastvisinc = totalclock+32;
                        shoot(pi,RPG);
                        p->ammo_amount[DEVISTATOR_WEAPON]--;
                        checkavailweapon(p);
                    }
                    if((*kb) > 5) (*kb) = 0;
                }
                break;
            case FREEZE_WEAPON:

                if( (*kb) < 4 )
                {
                    (*kb)++;
                    if( (*kb) == 3 )
                    {
                        p->ammo_amount[FREEZE_WEAPON]--;
                        p->visibility = 0;
                        lastvisinc = totalclock+32;
                        shoot(pi,FREEZEBLAST);
                        checkavailweapon(p);
                    }
                    if(s->xrepeat < 32)
                        { *kb = 0; break; }
                }
                else
                {
                    if( sb_snum&(1<<2))
                    {
                        *kb = 1;
                        spritesound(CAT_FIRE,pi);
                    }
                    else *kb = 0;
                }
                break;

            case TRIPBOMB_WEAPON:
                if(*kb < 4)
                {
                    p->posz = p->oposz;
                    p->poszv = 0;
                    if( (*kb) == 3 )
                        shoot(pi,HANDHOLDINGLASER);
                }
                if((*kb) == 16)
                {
                    (*kb) = 0;
                    checkavailweapon(p);
                    p->weapon_pos = -9;
                }
                else (*kb)++;
                break;
            case KNEE_WEAPON:
                (*kb)++;

                if( (*kb) == 7) shoot(pi,KNEE);
                else if( (*kb) == 14)
                {
                    if( sb_snum&(1<<2) )
                        *kb = 1+(TRAND&3);
                    else *kb = 0;
                }

                if(p->wantweaponfire >= 0)
                    checkavailweapon(p);
                break;

            case RPG_WEAPON:
                (*kb)++;
                if( (*kb) == 4 )
                {
                    p->ammo_amount[RPG_WEAPON]--;
                    lastvisinc = totalclock+32;
                    p->visibility = 0;
                    shoot(pi,RPG);
                    checkavailweapon(p);
                }
                else if( *kb == 20 )
                    *kb = 0;
                break;
        }
    }
}



//UPDATE THIS FILE OVER THE OLD GETSPRITESCORE/COMPUTERGETINPUT FUNCTIONS
static int getspritescore(int32_t snum, int32_t dapicnum)
{
    switch(dapicnum)
    {
        case FIRSTGUNSPRITE: return(5);
        case CHAINGUNSPRITE: return(50);
        case RPGSPRITE: return(200);
        case FREEZESPRITE: return(25);
        case SHRINKERSPRITE: return(80);
        case HEAVYHBOMB: return(60);
        case TRIPBOMBSPRITE: return(50);
        case SHOTGUNSPRITE: return(120);
        case DEVISTATORSPRITE: return(120);

        case FREEZEAMMO: if (ps[snum].ammo_amount[FREEZE_WEAPON] < max_ammo_amount[FREEZE_WEAPON]) return(10); else return(0);
        case AMMO: if (ps[snum].ammo_amount[SHOTGUN_WEAPON] < max_ammo_amount[SHOTGUN_WEAPON]) return(10); else return(0);
        case BATTERYAMMO: if (ps[snum].ammo_amount[CHAINGUN_WEAPON] < max_ammo_amount[CHAINGUN_WEAPON]) return(20); else return(0);
        case DEVISTATORAMMO: if (ps[snum].ammo_amount[DEVISTATOR_WEAPON] < max_ammo_amount[DEVISTATOR_WEAPON]) return(25); else return(0);
        case RPGAMMO: if (ps[snum].ammo_amount[RPG_WEAPON] < max_ammo_amount[RPG_WEAPON]) return(50); else return(0);
        case CRYSTALAMMO: if (ps[snum].ammo_amount[SHRINKER_WEAPON] < max_ammo_amount[SHRINKER_WEAPON]) return(10); else return(0);
        case HBOMBAMMO: if (ps[snum].ammo_amount[HANDBOMB_WEAPON] < max_ammo_amount[HANDBOMB_WEAPON]) return(30); else return(0);
        case SHOTGUNAMMO: if (ps[snum].ammo_amount[SHOTGUN_WEAPON] < max_ammo_amount[SHOTGUN_WEAPON]) return(25); else return(0);

        case COLA: if (sprite[ps[snum].i].extra < 100) return(10); else return(0);
        case SIXPAK: if (sprite[ps[snum].i].extra < 100) return(30); else return(0);
        case FIRSTAID: if (ps[snum].firstaid_amount < 100) return(100); else return(0);
        case SHIELD: if (ps[snum].shield_amount < 100) return(50); else return(0);
        case STEROIDS: if (ps[snum].steroids_amount < 400) return(30); else return(0);
        case AIRTANK: if (ps[snum].scuba_amount < 6400) return(30); else return(0);
        case JETPACK: if (ps[snum].jetpack_amount < 1600) return(100); else return(0);
        case HEATSENSOR: if (ps[snum].heat_amount < 1200) return(10); else return(0);
        case ACCESSCARD: return(1);
        case BOOTS: if (ps[snum].boot_amount < 200) return(50); else return(0);
        case ATOMICHEALTH: if (sprite[ps[snum].i].extra < max_player_health) return(50); else return(0);
        case HOLODUKE: if (ps[snum].holoduke_amount < 2400) return(30); else return(0);
    }
    return(0);
}

static int32_t fdmatrix[12][12] =
{
 //KNEE PIST SHOT CHAIN RPG PIPE SHRI DEVI WALL FREE HAND EXPA
   {  128,  -1,  -1,  -1, 128,  -1,  -1,  -1, 128,  -1, 128,  -1 },   //KNEE
   { 1024,1024,1024,1024,2560, 128,2560,2560,1024,2560,2560,2560 },   //PIST
   {  512, 512, 512, 512,2560, 128,2560,2560,1024,2560,2560,2560 },   //SHOT
   {  512, 512, 512, 512,2560, 128,2560,2560,1024,2560,2560,2560 },   //CHAIN
   { 2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560 },   //RPG
   {  512, 512, 512, 512,2048, 512,2560,2560, 512,2560,2560,2560 },   //PIPE
   {  128, 128, 128, 128,2560, 128,2560,2560, 128, 128, 128, 128 },   //SHRI
   { 1536,1536,1536,1536,2560,1536,1536,1536,1536,1536,1536,1536 },   //DEVI
   {   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },   //WALL
   {  128, 128, 128, 128,2560, 128,2560,2560, 128, 128, 128, 128 },   //FREE
   { 2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560,2560 },   //HAND
   {  128, 128, 128, 128,2560, 128,2560,2560, 128, 128, 128, 128 }    //EXPA
};

static int32_t goalx[MAXPLAYERS], goaly[MAXPLAYERS], goalz[MAXPLAYERS];
static int32_t goalsect[MAXPLAYERS], goalwall[MAXPLAYERS], goalsprite[MAXPLAYERS];
static int32_t goalplayer[MAXPLAYERS], clipmovecount[MAXPLAYERS];
short searchsect[MAXSECTORS], searchparent[MAXSECTORS];
uint8_t  dashow2dsector[(MAXSECTORS+7)>>3];
void computergetinput(int32_t snum, input *syn)
{
    int32_t i, j, k, l, x1, y1, z1, x2, y2, z2, x3, y3, z3, dx, dy;
    int32_t dist, daang, zang, fightdist, damyang, damysect;
    int32_t startsect, endsect, splc, send, startwall, endwall;
    short dasect, dawall, daspr;
    struct player_struct *p;
    walltype *wal;

    p = &ps[snum];
    syn->fvel = 0;
    syn->svel = 0;
    syn->avel = 0;
    syn->horz = 0;
    syn->bits = 0;

    z2 = y2 = x2 = 0;
    x1 = sprite[p->i].x;
    y1 = sprite[p->i].y;
    z1 = sprite[p->i].z;
    damyang = sprite[p->i].ang;
    damysect = sprite[p->i].sectnum;
    if ((numplayers >= 2) && (snum == myconnectindex))
        { x1 = myx; y1 = myy; z1 = myz+PHEIGHT; damyang = myang; damysect = mycursectnum; }

    if (!(numframes&7))
    {
        x2 = sprite[ps[goalplayer[snum]].i].x;
        y2 = sprite[ps[goalplayer[snum]].i].y;
        z2 = sprite[ps[goalplayer[snum]].i].z;
		
		if (!cansee(x1,y1,z1-(48<<8),damysect,x2,y2,z2-(48<<8),sprite[ps[goalplayer[snum]].i].sectnum))
            goalplayer[snum] = snum;
    }

    if ((goalplayer[snum] == snum) || (ps[goalplayer[snum]].dead_flag != 0))
    {
        j = 0x7fffffff;
        for(i=connecthead;i>=0;i=connectpoint2[i])
            if (i != snum)
            {
                dist = ksqrt((sprite[ps[i].i].x-x1)*(sprite[ps[i].i].x-x1)+(sprite[ps[i].i].y-y1)*(sprite[ps[i].i].y-y1));

                x2 = sprite[ps[i].i].x;
                y2 = sprite[ps[i].i].y;
                z2 = sprite[ps[i].i].z;
                if (!cansee(x1,y1,z1-(48<<8),damysect,x2,y2,z2-(48<<8),sprite[ps[i].i].sectnum))
                    dist <<= 1;

                if (dist < j) { j = dist; goalplayer[snum] = i; }
            }
    }

    x2 = sprite[ps[goalplayer[snum]].i].x;
    y2 = sprite[ps[goalplayer[snum]].i].y;
    z2 = sprite[ps[goalplayer[snum]].i].z;

    if (p->dead_flag) syn->bits |= (1<<29);
    if ((p->firstaid_amount > 0) && (p->last_extra < max_player_health))
        syn->bits |= (1<<16);

    for(j=headspritestat[4];j>=0;j=nextspritestat[j])
    {
        switch (sprite[j].picnum)
        {
            case TONGUE: k = 4; break;
            case FREEZEBLAST: k = 4; break;
            case SHRINKSPARK: k = 16; break;
            case RPG: k = 16; break;
            default: k = 0; break;
        }
        if (k)
        {
            x3 = sprite[j].x;
            y3 = sprite[j].y;
            z3 = sprite[j].z;
            for(l=0;l<=8;l++)
            {
                if (tmulscale11(x3-x1,x3-x1,y3-y1,y3-y1,(z3-z1)>>4,(z3-z1)>>4) < 3072)
                {
                    dx = sintable[(sprite[j].ang+512)&2047];
                    dy = sintable[sprite[j].ang&2047];
                    if ((x1-x3)*dy > (y1-y3)*dx) i = -k*512; else i = k*512;
                    syn->fvel -= mulscale17(dy,i);
                    syn->svel += mulscale17(dx,i);
                }
                if (l < 7)
                {
                    x3 += (mulscale14(sprite[j].xvel,sintable[(sprite[j].ang+512)&2047])<<2);
                    y3 += (mulscale14(sprite[j].xvel,sintable[sprite[j].ang&2047])<<2);
                    z3 += (sprite[j].zvel<<2);
                }
                else
                {
                    hitscan(sprite[j].x,sprite[j].y,sprite[j].z,sprite[j].sectnum,
                     mulscale14(sprite[j].xvel,sintable[(sprite[j].ang+512)&2047]),
                     mulscale14(sprite[j].xvel,sintable[sprite[j].ang&2047]),
                     (int32_t)sprite[j].zvel,
                     &dasect,&dawall,&daspr,&x3,&y3,&z3,CLIPMASK1);
                }
            }
        }
    }

    if ((ps[goalplayer[snum]].dead_flag == 0) &&
        ((cansee(x1,y1,z1,damysect,x2,y2,z2,sprite[ps[goalplayer[snum]].i].sectnum)) ||
         (cansee(x1,y1,z1-(24<<8),damysect,x2,y2,z2-(24<<8),sprite[ps[goalplayer[snum]].i].sectnum)) ||
         (cansee(x1,y1,z1-(48<<8),damysect,x2,y2,z2-(48<<8),sprite[ps[goalplayer[snum]].i].sectnum))))
    {
        syn->bits |= (1<<2);

        if ((p->curr_weapon == HANDBOMB_WEAPON) && (!(rand()&7)))
            syn->bits &= ~(1<<2);

        if (p->curr_weapon == TRIPBOMB_WEAPON)
            syn->bits |= ((rand()%MAX_WEAPONS)<<8);

        if (p->curr_weapon == RPG_WEAPON)
        {
            hitscan(x1,y1,z1-PHEIGHT,damysect,sintable[(damyang+512)&2047],sintable[damyang&2047],
                (100-p->horiz-p->horizoff)*32,&dasect,&dawall,&daspr,&x3,&y3,&z3,CLIPMASK1);
            if ((x3-x1)*(x3-x1)+(y3-y1)*(y3-y1) < 2560*2560) syn->bits &= ~(1<<2);
        }


        fightdist = fdmatrix[p->curr_weapon][ps[goalplayer[snum]].curr_weapon];
        if (fightdist < 128) fightdist = 128;
        dist = ksqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)); if (dist == 0) dist = 1;
        daang = getangle(x2+(ps[goalplayer[snum]].posxv>>14)-x1,y2+(ps[goalplayer[snum]].posyv>>14)-y1);
        zang = 100-((z2-z1)*8)/dist;
        fightdist = max(fightdist,(klabs(z2-z1)>>4));

        if (sprite[ps[goalplayer[snum]].i].yrepeat < 32)
            { fightdist = 0; syn->bits &= ~(1<<2); }
        if (sprite[ps[goalplayer[snum]].i].pal == 1)
            { fightdist = 0; syn->bits &= ~(1<<2); }

        if (dist < 256) syn->bits |= (1<<22);

        x3 = x2+((x1-x2)*fightdist/dist);
        y3 = y2+((y1-y2)*fightdist/dist);
        syn->fvel += (x3-x1)*2047/dist;
        syn->svel += (y3-y1)*2047/dist;

            //Strafe attack
        if (fightdist)
        {
            j = totalclock+snum*13468;
            i = sintable[(j<<6)&2047];
            i += sintable[((j+4245)<<5)&2047];
            i += sintable[((j+6745)<<4)&2047];
            i += sintable[((j+15685)<<3)&2047];
            dx = sintable[(sprite[ps[goalplayer[snum]].i].ang+512)&2047];
            dy = sintable[sprite[ps[goalplayer[snum]].i].ang&2047];
            if ((x1-x2)*dy > (y1-y2)*dx) i += 8192; else i -= 8192;
            syn->fvel += ((sintable[(daang+1024)&2047]*i)>>17);
            syn->svel += ((sintable[(daang+512)&2047]*i)>>17);
        }

        syn->avel = min(max((((daang+1024-damyang)&2047)-1024)>>1,-127),127);
        syn->horz = min(max((zang-p->horiz)>>1,-MAXHORIZ),MAXHORIZ);
        syn->bits |= (1<<23);
        return;
    }

    goalsect[snum] = -1;
    if (goalsect[snum] < 0)
    {
        goalwall[snum] = -1;
        startsect = sprite[p->i].sectnum;
        endsect = sprite[ps[goalplayer[snum]].i].sectnum;

        clearbufbyte(dashow2dsector,(MAXSECTORS+7)>>3,0L);
        searchsect[0] = startsect;
        searchparent[0] = -1;
        dashow2dsector[startsect>>3] |= (1<<(startsect&7));
        for(splc=0,send=1;splc<send;splc++)
        {
            startwall = sector[searchsect[splc]].wallptr;
            endwall = startwall + sector[searchsect[splc]].wallnum;
            for(i=startwall,wal=&wall[startwall];i<endwall;i++,wal++)
            {
                j = wal->nextsector; if (j < 0) continue;

                dx = ((wall[wal->point2].x+wal->x)>>1);
                dy = ((wall[wal->point2].y+wal->y)>>1);
                if ((getceilzofslope(j,dx,dy) > getflorzofslope(j,dx,dy)-(28<<8)) && ((sector[j].lotag < 15) || (sector[j].lotag > 22)))
                    continue;
                if (getflorzofslope(j,dx,dy) < getflorzofslope(searchsect[splc],dx,dy)-(72<<8))
                    continue;
                if ((dashow2dsector[j>>3]&(1<<(j&7))) == 0)
                {
                    dashow2dsector[j>>3] |= (1<<(j&7));
                    searchsect[send] = (short)j;
                    searchparent[send] = (short)splc;
                    send++;
                    if (j == endsect)
                    {
                        clearbufbyte(dashow2dsector,(MAXSECTORS+7)>>3,0L);
                        for(k=send-1;k>=0;k=searchparent[k])
                            dashow2dsector[searchsect[k]>>3] |= (1<<(searchsect[k]&7));

                        for(k=send-1;k>=0;k=searchparent[k])
                            if (!searchparent[k]) break;

                        goalsect[snum] = searchsect[k];
                        startwall = sector[goalsect[snum]].wallptr;
                        endwall = startwall+sector[goalsect[snum]].wallnum;
                        x3 = y3 = 0;
                        for(i=startwall;i<endwall;i++)
                        {
                            x3 += wall[i].x;
                            y3 += wall[i].y;
                        }
                        x3 /= (endwall-startwall);
                        y3 /= (endwall-startwall);

                        startwall = sector[startsect].wallptr;
                        endwall = startwall+sector[startsect].wallnum;
                        l = 0; k = startwall;
                        for(i=startwall;i<endwall;i++)
                        {
                            if (wall[i].nextsector != goalsect[snum]) continue;
                            dx = wall[wall[i].point2].x-wall[i].x;
                            dy = wall[wall[i].point2].y-wall[i].y;

                            //if (dx*(y1-wall[i].y) <= dy*(x1-wall[i].x))
                            //   if (dx*(y2-wall[i].y) >= dy*(x2-wall[i].x))
                                    if ((x3-x1)*(wall[i].y-y1) <= (y3-y1)*(wall[i].x-x1))
                                        if ((x3-x1)*(wall[wall[i].point2].y-y1) >= (y3-y1)*(wall[wall[i].point2].x-x1))
                                            { k = i; break; }

                            dist = ksqrt(dx*dx+dy*dy);
                            if (dist > l) { l = dist; k = i; }
                        }
                        goalwall[snum] = k;
                        daang = ((getangle(wall[wall[k].point2].x-wall[k].x,wall[wall[k].point2].y-wall[k].y)+1536)&2047);
                        goalx[snum] = ((wall[k].x+wall[wall[k].point2].x)>>1)+(sintable[(daang+512)&2047]>>8);
                        goaly[snum] = ((wall[k].y+wall[wall[k].point2].y)>>1)+(sintable[daang&2047]>>8);
                        goalz[snum] = sector[goalsect[snum]].floorz-(32<<8);
                        break;
                    }
                }
            }

            for(i=headspritesect[searchsect[splc]];i>=0;i=nextspritesect[i])
                if (sprite[i].lotag == 7)
                {
                    j = sprite[sprite[i].owner].sectnum;
                    if ((dashow2dsector[j>>3]&(1<<(j&7))) == 0)
                    {
                        dashow2dsector[j>>3] |= (1<<(j&7));
                        searchsect[send] = (short)j;
                        searchparent[send] = (short)splc;
                        send++;
                        if (j == endsect)
                        {
                            clearbufbyte(dashow2dsector,(MAXSECTORS+7)>>3,0L);
                            for(k=send-1;k>=0;k=searchparent[k])
                                dashow2dsector[searchsect[k]>>3] |= (1<<(searchsect[k]&7));

                            for(k=send-1;k>=0;k=searchparent[k])
                                if (!searchparent[k]) break;

                            goalsect[snum] = searchsect[k];
                            startwall = sector[startsect].wallptr;
                            endwall = startwall+sector[startsect].wallnum;
                            l = 0; k = startwall;
                            for(i=startwall;i<endwall;i++)
                            {
                                dx = wall[wall[i].point2].x-wall[i].x;
                                dy = wall[wall[i].point2].y-wall[i].y;
                                dist = ksqrt(dx*dx+dy*dy);
                                if ((wall[i].nextsector == goalsect[snum]) && (dist > l))
                                    { l = dist; k = i; }
                            }
                            goalwall[snum] = k;
                            daang = ((getangle(wall[wall[k].point2].x-wall[k].x,wall[wall[k].point2].y-wall[k].y)+1536)&2047);
                            goalx[snum] = ((wall[k].x+wall[wall[k].point2].x)>>1)+(sintable[(daang+512)&2047]>>8);
                            goaly[snum] = ((wall[k].y+wall[wall[k].point2].y)>>1)+(sintable[daang&2047]>>8);
                            goalz[snum] = sector[goalsect[snum]].floorz-(32<<8);
                            break;
                        }
                    }
                }
            if (goalwall[snum] >= 0) break;
        }
    }

    if ((goalsect[snum] < 0) || (goalwall[snum] < 0))
    {
        if (goalsprite[snum] < 0)
        {
            for(k=0;k<4;k++)
            {
                i = (rand()%numsectors);
                for(j=headspritesect[i];j>=0;j=nextspritesect[j])
                {
                    if ((sprite[j].xrepeat <= 0) || (sprite[j].yrepeat <= 0)) continue;
                    if (getspritescore(snum,sprite[j].picnum) <= 0) continue;
                    if (cansee(x1,y1,z1-(32<<8),damysect,sprite[j].x,sprite[j].y,sprite[j].z-(4<<8),i))
                        { goalx[snum] = sprite[j].x; goaly[snum] = sprite[j].y; goalz[snum] = sprite[j].z; goalsprite[snum] = j; break; }
                }
            }
        }
        x2 = goalx[snum];
        y2 = goaly[snum];
        dist = ksqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)); if (!dist) return;
        daang = getangle(x2-x1,y2-y1);
        syn->fvel += (x2-x1)*2047/dist;
        syn->svel += (y2-y1)*2047/dist;
        syn->avel = min(max((((daang+1024-damyang)&2047)-1024)>>3,-127),127);
    }
    else
        goalsprite[snum] = -1;

    x3 = p->posx; y3 = p->posy; z3 = p->posz; dasect = p->cursectnum;
    i = clipmove(&x3,&y3,&z3,&dasect,p->posxv,p->posyv,164L,4L<<8,4L<<8,CLIPMASK0);
    if (!i)
    {
        x3 = p->posx; y3 = p->posy; z3 = p->posz+(24<<8); dasect = p->cursectnum;
        i = clipmove(&x3,&y3,&z3,&dasect,p->posxv,p->posyv,164L,4L<<8,4L<<8,CLIPMASK0);
    }
    if (i)
    {
        clipmovecount[snum]++;

        j = 0;
        if ((i&0xc000) == 32768)  //Hit a wall (49152 for sprite)
            if (wall[i&(MAXWALLS-1)].nextsector >= 0)
            {
                if (getflorzofslope(wall[i&(MAXWALLS-1)].nextsector,p->posx,p->posy) <= p->posz+(24<<8)) j |= 1;
                if (getceilzofslope(wall[i&(MAXWALLS-1)].nextsector,p->posx,p->posy) >= p->posz-(24<<8)) j |= 2;
            }
        if ((i&0xc000) == 49152) j = 1;
        if (j&1) if (clipmovecount[snum] == 4) syn->bits |= (1<<0);
        if (j&2) syn->bits |= (1<<1);

            //Strafe attack
        daang = getangle(x2-x1,y2-y1);
        if ((i&0xc000) == 32768)
            daang = getangle(wall[wall[i&(MAXWALLS-1)].point2].x-wall[i&(MAXWALLS-1)].x,wall[wall[i&(MAXWALLS-1)].point2].y-wall[i&(MAXWALLS-1)].y);
        j = totalclock+snum*13468;
        i = sintable[(j<<6)&2047];
        i += sintable[((j+4245)<<5)&2047];
        i += sintable[((j+6745)<<4)&2047];
        i += sintable[((j+15685)<<3)&2047];
        syn->fvel += ((sintable[(daang+1024)&2047]*i)>>17);
        syn->svel += ((sintable[(daang+512)&2047]*i)>>17);

        if ((clipmovecount[snum]&31) == 2) syn->bits |= (1<<29);
        if ((clipmovecount[snum]&31) == 17) syn->bits |= (1<<22);
        if (clipmovecount[snum] > 32) { goalsect[snum] = -1; goalwall[snum] = -1; clipmovecount[snum] = 0; }

        goalsprite[snum] = -1;
    }
    else
        clipmovecount[snum] = 0;

    if ((goalsect[snum] >= 0) && (goalwall[snum] >= 0))
    {
        x2 = goalx[snum];
        y2 = goaly[snum];
        dist = ksqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)); if (!dist) return;
        daang = getangle(x2-x1,y2-y1);
        if ((goalwall[snum] >= 0) && (dist < 4096))
            daang = ((getangle(wall[wall[goalwall[snum]].point2].x-wall[goalwall[snum]].x,wall[wall[goalwall[snum]].point2].y-wall[goalwall[snum]].y)+1536)&2047);
        syn->fvel += (x2-x1)*2047/dist;
        syn->svel += (y2-y1)*2047/dist;
        syn->avel = min(max((((daang+1024-damyang)&2047)-1024)>>3,-127),127);
    }
}

