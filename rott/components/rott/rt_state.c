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
#include "rt_def.h"
#include "sprites.h"
#include "states.h"
#include "rt_actor.h"
#include "develop.h"
#define SPRINGDELAY 3
//MED
#include "memcheck.h"

/*
=============================================================================

					  LOW GUARD (includes sneaky)

=============================================================================
*/


extern void T_ReallyDead(objtype*ob);
extern void T_PlayDead(objtype*ob);
extern void T_Xylophone(objtype*ob);
extern void T_BloodFall(objtype*ob);
extern void SetShapeoffset(objtype*ob);
extern void RespawnPlayerobj(objtype*ob);
extern void T_ElevDisk(objtype*ob);
extern void T_Plead(objtype*ob);
extern void T_Blood(objtype*ob);
extern void T_Convert(objtype*ob);
extern void T_SlideDownScreen(objtype*ob);
extern void T_End(objtype*ob);
extern void T_Reset(objtype*ob);
extern void T_CollectorWander(objtype*ob);
extern void T_CollectorFindDoor(objtype*ob);
extern void T_AutoPath(objtype*);
extern void T_AutoRealign(objtype*);
extern void T_AutoShootAlign(objtype*);
extern void T_NME_SpinFire(objtype*);
extern void MissileMovement(objtype*);
extern void T_BatBlast(objtype*);
extern void T_DeadWait(objtype*);
extern void ActorMovement(objtype*);
extern void T_Spring(objtype*);
extern void T_SnakeFinale(objtype*);
extern void T_Special(objtype*);
extern void T_NME_Explode(objtype*);
extern void T_Guts(objtype*);
extern void T_Player(objtype*);
extern void T_ParticleGenerate(objtype*);
extern void T_Particle(objtype*);
extern void T_SpawnSoul(objtype*);
extern void T_NME_WindUp(objtype*);
extern void T_NME_Attack(objtype*);
extern void T_Saucer(objtype*);
extern void T_NMErocket(objtype*);
extern void T_NME_SpinAttack(objtype*);
extern void T_NME_HeadShoot(objtype*);
extern void T_CrushUp(objtype*);
extern void T_CrushDown(objtype*);
extern void T_HeinrichChase(objtype*);
extern void T_KristLeft(objtype*);
extern void T_KristRight(objtype*);
extern void T_KristCheckFire(objtype*);
extern void T_BoulderSpawn(objtype*);
extern void T_BoulderDrop(objtype*);
extern void T_BoulderMove(objtype*);
extern void T_DarkmonkChase(objtype*);
extern void T_DarkSnakeChase(objtype*);
extern void T_DarkSnakeSpawn(objtype*);
extern void A_DmonkAttack(objtype*);
extern void T_GenericMove(objtype*);
extern void T_Count(objtype*);
extern void T_Spears(objtype*);
extern void T_EsauSpears(objtype*);
extern void T_Spring(objtype*);
extern void T_4WayGunStand(objtype*);
extern void A_GunShoot(objtype*);
extern void A_4WayGunShoot(objtype*);
extern void T_Attack(objtype*);
extern void T_SnakePath(objtype*);
extern void T_SnakeFindPath(objtype*);
extern void T_DarkmonkLandAndFire(objtype*ob);
extern void T_DarkmonkCharge(objtype*ob);
extern void T_DarkmonkReact(objtype*ob);
extern void T_BossExplosions(objtype*ob);

statetype s_lowgrdstand	   = {true,SPR_LOWGRD_S1,0,T_Stand,0,&s_lowgrdstand};


statetype s_lowgrdpath4    = {true,SPR_LOWGRD_W41,12,T_Path,0,&s_lowgrdpath1};
statetype s_lowgrdpath3    = {true,SPR_LOWGRD_W31,12,T_Path,SF_CLOSE,&s_lowgrdpath4};
statetype s_lowgrdpath2    = {true,SPR_LOWGRD_W21,12,T_Path,0,&s_lowgrdpath3};
statetype s_lowgrdpath1    = {true,SPR_LOWGRD_W11,12,T_Path,SF_CLOSE,&s_lowgrdpath2};

statetype s_lowgrdcollide  = {false,SPR_LOWGRD_PAIN1,0,T_Collide,0,&s_lowgrdcollide};
statetype s_lowgrdcollide2 = {false,SPR_LOWGRD_PAIN2,0,T_Collide,0,&s_lowgrdcollide2};


//statetype s_lowgrduse2     = {true,SPR_LOWGRD_USE21,5,T_Use,0,&s_lowgrdpath1};
//statetype s_lowgrduse1     = {true,SPR_LOWGRD_USE11,5,T_Use,0,&s_lowgrduse2};

statetype s_lowgrdshoot4 	= {false,SPR_LOWGRD_SHOOT4,8,ActorMovement,0,&s_lowgrdchase1};
statetype s_lowgrdshoot3   = {false,SPR_LOWGRD_SHOOT3,5,A_Shoot,0,&s_lowgrdshoot4};
statetype s_lowgrdshoot2   = {false,SPR_LOWGRD_SHOOT2,20,ActorMovement,0,&s_lowgrdshoot3};
statetype s_lowgrdshoot1 	= {false,SPR_LOWGRD_SHOOT1,6,ActorMovement,0,&s_lowgrdshoot2};

statetype s_lowgrdchase4   = {true,SPR_LOWGRD_W41,12,T_Chase,0,&s_lowgrdchase1};
statetype s_lowgrdchase3   = {true,SPR_LOWGRD_W31,12,T_Chase,SF_CLOSE,&s_lowgrdchase4};
statetype s_lowgrdchase2   = {true,SPR_LOWGRD_W21,12,T_Chase,0,&s_lowgrdchase3};
statetype s_lowgrdchase1   = {true,SPR_LOWGRD_W11,12,T_Chase,SF_CLOSE,&s_lowgrdchase2};



statetype s_lowgrddead     = {false,SPR_LOWGRD_DEAD,0,T_Collide,0,&s_lowgrddead};
statetype s_lowgrddie4     = {false,SPR_LOWGRD_DIE4,8,T_Collide,0,&s_lowgrddead};
statetype s_lowgrddie3     = {false,SPR_LOWGRD_DIE3,8,T_Collide,0,&s_lowgrddie4};
statetype s_lowgrddie2     = {false,SPR_LOWGRD_DIE2,8,T_Collide,0,&s_lowgrddie3};
statetype s_lowgrddie1		= {false,SPR_LOWGRD_DIE1,15,T_Collide,0,&s_lowgrddie2};


statetype s_lowgrdcrushed2		= {false,SPR_LOWGRD_DIE3,2,NULL,0,&s_lowgrddead};
statetype s_lowgrdcrushed1		= {false,SPR_LOWGRD_DIE1,2,NULL,0,&s_lowgrdcrushed2};

/************** Sneaky stuff *******************************************/

statetype s_sneakydown     = {false,SPR_SNEAKY_DEAD,0,T_Stand,0,&s_sneakydown};
statetype s_sneakyrise4    = {false,SPR_RISE4,6,ActorMovement,0,&s_lowgrdchase1};
statetype s_sneakyrise3    = {false,SPR_RISE3,6,ActorMovement,0,&s_sneakyrise4};
statetype s_sneakyrise2    = {false,SPR_RISE2,6,ActorMovement,0,&s_sneakyrise3};
statetype s_sneakyrise1    = {false,SPR_RISE1,6,ActorMovement,0,&s_sneakyrise2};


/*
=============================================================================

					  HIGH GUARD

=============================================================================
*/



statetype s_highgrdstand	= {true,SPR_HIGHGRD_S1,0,T_Stand,0,&s_highgrdstand};


statetype s_highgrdpath4   = {true,SPR_HIGHGRD_W41,12,T_Path,0,&s_highgrdpath1};
statetype s_highgrdpath3   = {true,SPR_HIGHGRD_W31,12,T_Path,SF_CLOSE,&s_highgrdpath4};
statetype s_highgrdpath2   = {true,SPR_HIGHGRD_W21,12,T_Path,0,&s_highgrdpath3};
statetype s_highgrdpath1   = {true,SPR_HIGHGRD_W11,12,T_Path,SF_CLOSE,&s_highgrdpath2};


//statetype s_highgrdpain1 	= {2,SPR_HIGHGRD_PAIN1,10,T_Collide,0,&s_highgrdchase1};
//statetype s_highgrdpain2 	= {2,SPR_HIGHGRD_PAIN2,10,T_Collide,0,&s_highgrdchase1};


statetype s_highgrdcollide  = {false,SPR_HIGHGRD_PAIN1,0,T_Collide,0,&s_highgrdcollide};
statetype s_highgrdcollide2 = {false,SPR_HIGHGRD_PAIN2,0,T_Collide,0,&s_highgrdcollide2};


//statetype s_highgrduse2 	 = {true,SPR_HIGHGRD_USE21,10,T_Use,0,&s_highgrdpath1};
//statetype s_highgrduse1 	 = {true,SPR_HIGHGRD_USE11,10,T_Use,0,&s_highgrduse2};


statetype s_highgrdshoot4  = {false,SPR_HIGHGRD_SHOOT4,3,A_Repeat,0,&s_highgrdshoot3};
statetype s_highgrdshoot3  = {false,SPR_HIGHGRD_SHOOT3,5,A_Shoot,0,&s_highgrdshoot4};
statetype s_highgrdshoot2  = {false,SPR_HIGHGRD_SHOOT2,6,ActorMovement,0,&s_highgrdshoot3};
statetype s_highgrdshoot1 	= {false,SPR_HIGHGRD_SHOOT1,6,ActorMovement,0,&s_highgrdshoot2};


statetype s_highgrdchase4  = {true,SPR_HIGHGRD_W41,12,T_Chase,0,&s_highgrdchase1};
statetype s_highgrdchase3 = {true,SPR_HIGHGRD_W31,12,T_Chase,SF_CLOSE,&s_highgrdchase4};
statetype s_highgrdchase2 = {true,SPR_HIGHGRD_W21,12,T_Chase,0,&s_highgrdchase3};
statetype s_highgrdchase1 = {true,SPR_HIGHGRD_W11,12,T_Chase,SF_CLOSE,&s_highgrdchase2};


statetype s_highgrddead    = {false,SPR_HIGHGRD_DEAD,0,T_Collide,0,&s_highgrddead};
statetype s_highgrddie5    = {false,SPR_HIGHGRD_DIE5,0,T_Collide,0,&s_highgrddead};
statetype s_highgrddie4    = {false,SPR_HIGHGRD_DIE4,8,T_Collide,0,&s_highgrddie5};
statetype s_highgrddie3    = {false,SPR_HIGHGRD_DIE3,8,T_Collide,0,&s_highgrddie4};
statetype s_highgrddie2    = {false,SPR_HIGHGRD_DIE2,8,T_Collide,0,&s_highgrddie3};
statetype s_highgrddie1    = {false,SPR_HIGHGRD_DIE1,8,T_Collide,0,&s_highgrddie2};

statetype s_highgrdcrushed2		= {false,SPR_HIGHGRD_DIE3,2,NULL,0,&s_highgrddead};
statetype s_highgrdcrushed1		= {false,SPR_HIGHGRD_DIE1,2,NULL,0,&s_highgrdcrushed2};

/*
===========================================================================

										 STRIKEGUARD

===========================================================================
*/


statetype s_strikestand	= {true,SPR_STRIKE_S1,0,T_Stand,0,&s_strikestand};


statetype s_strikepath4    = {true,SPR_STRIKE_W41,12,T_Path,0,&s_strikepath1};
statetype s_strikepath3    = {true,SPR_STRIKE_W31,12,T_Path,SF_CLOSE,&s_strikepath4};
statetype s_strikepath2    = {true,SPR_STRIKE_W21,12,T_Path,0,&s_strikepath3};
statetype s_strikepath1    = {true,SPR_STRIKE_W11,12,T_Path,SF_CLOSE,&s_strikepath2};

statetype s_strikecollide  = {false,SPR_STRIKE_PAIN1,0,T_Collide,0,&s_strikecollide};
statetype s_strikecollide2 = {false,SPR_STRIKE_PAIN2,0,T_Collide,0,&s_strikecollide2};

statetype s_strikeshoot4 	= {false,SPR_STRIKE_SHOOT4,6,ActorMovement,0,&s_strikechase1};
statetype s_strikeshoot3 	= {false,SPR_STRIKE_SHOOT3,6,ActorMovement,0,&s_strikeshoot4};
statetype s_strikeshoot2   = {false,SPR_STRIKE_SHOOT2,5,A_Shoot,0,&s_strikeshoot3};
statetype s_strikeshoot1   = {false,SPR_STRIKE_SHOOT1,20,ActorMovement,0,&s_strikeshoot2};


//statetype s_strikeuse2      = {true,SPR_STRIKE_USE21,10,T_Use,0,&s_strikechase1};
//statetype s_strikeuse1      = {true,SPR_STRIKE_USE11,10,NULL,0,&s_strikeuse2};

statetype s_strikewait      = {true,SPR_STRIKE_S1,35,NULL,0,&s_strikechase1};

statetype s_strikerollright6    = {false,SPR_STRIKE_LROLL6,8,T_Roll,0,&s_strikeshoot1};
statetype s_strikerollright5    = {false,SPR_STRIKE_LROLL5,8,T_Roll,0,&s_strikerollright6};
statetype s_strikerollright4    = {false,SPR_STRIKE_LROLL4,7,T_Reset,0,&s_strikerollright5};
statetype s_strikerollright3    = {false,SPR_STRIKE_LROLL3,7,T_Roll,0,&s_strikerollright4};
statetype s_strikerollright2    = {false,SPR_STRIKE_LROLL2,7,T_Roll,0,&s_strikerollright3};
statetype s_strikerollright1    = {false,SPR_STRIKE_LROLL1,5,T_Roll,0,&s_strikerollright2};



statetype s_strikerollleft6    = {false,SPR_STRIKE_RROLL6,8,T_Roll,0,&s_strikeshoot1};
statetype s_strikerollleft5    = {false,SPR_STRIKE_RROLL5,8,T_Roll,0,&s_strikerollleft6};
statetype s_strikerollleft4    = {false,SPR_STRIKE_RROLL4,7,T_Reset,0,&s_strikerollleft5};
statetype s_strikerollleft3    = {false,SPR_STRIKE_RROLL3,7,T_Roll,0,&s_strikerollleft4};
statetype s_strikerollleft2    = {false,SPR_STRIKE_RROLL2,7,T_Roll,0,&s_strikerollleft3};
statetype s_strikerollleft1    = {false,SPR_STRIKE_RROLL1,5,T_Roll,0,&s_strikerollleft2};


statetype s_strikechase4   = {true,SPR_STRIKE_W41,12,T_Chase,0,&s_strikechase1};
statetype s_strikechase3   = {true,SPR_STRIKE_W31,12,T_Chase,SF_CLOSE,&s_strikechase4};
statetype s_strikechase2   = {true,SPR_STRIKE_W21,12,T_Chase,0,&s_strikechase3};
statetype s_strikechase1   = {true,SPR_STRIKE_W11,12,T_Chase,SF_CLOSE,&s_strikechase2};


statetype s_strikedead3    = {false,SPR_STRIKE_DEAD3,0,T_Collide,0,&s_strikedead3};
statetype s_strikedead2    = {false,SPR_STRIKE_DEAD2,0,T_Collide,0,&s_strikedead3};
statetype s_strikedead     = {false,SPR_STRIKE_DEAD1,7,T_Collide,0,&s_strikedead2};
statetype s_strikedie4     = {false,SPR_STRIKE_DIE4,7,T_Collide,0,&s_strikedead};
statetype s_strikedie3     = {false,SPR_STRIKE_DIE3,7,T_Collide,0,&s_strikedie4};
statetype s_strikedie2     = {false,SPR_STRIKE_DIE2,7,T_Collide,0,&s_strikedie3};
statetype s_strikedie1     = {false,SPR_STRIKE_DIE1,7,T_Collide,0,&s_strikedie2};


statetype s_strikecrushed2		= {false,SPR_STRIKE_DIE3,2,NULL,0,&s_strikedead};
statetype s_strikecrushed1		= {false,SPR_STRIKE_DIE1,2,NULL,0,&s_strikecrushed2};

/*
============================================================================

								LIGHTNING GUARD

============================================================================
*/

statetype s_blitzstand	= {true,SPR_BLITZ_S1,0,T_Stand,0,&s_blitzstand};

statetype s_blitzpath4 = {true,SPR_BLITZ_W41,12,T_Path,0,&s_blitzpath1};
statetype s_blitzpath3 = {true,SPR_BLITZ_W31,12,T_Path,SF_CLOSE,&s_blitzpath4};
statetype s_blitzpath2 = {true,SPR_BLITZ_W21,12,T_Path,0,&s_blitzpath3};
statetype s_blitzpath1 = {true,SPR_BLITZ_W11,12,T_Path,SF_CLOSE,&s_blitzpath2};



statetype s_blitzcollide  = {false,SPR_BLITZ_PAIN1,0,T_Collide,0,&s_blitzcollide};
statetype s_blitzcollide2 = {false,SPR_BLITZ_PAIN2,0,T_Collide,0,&s_blitzcollide2};

statetype s_blitzshoot4 = {false,SPR_BLITZ_SHOOT4,10,ActorMovement,0,&s_blitzchase1};
statetype s_blitzshoot3 = {false,SPR_BLITZ_SHOOT3,10,ActorMovement,0,&s_blitzshoot4};
statetype s_blitzshoot2 = {false,SPR_BLITZ_SHOOT2,5,A_Shoot,0,&s_blitzshoot3};
statetype s_blitzshoot1 = {false,SPR_BLITZ_SHOOT1,10,ActorMovement,0,&s_blitzshoot2};

statetype s_blitzrise4 = {false,SPR_BLITZ_RISE4,8,ActorMovement,0,&s_blitzchase1};
statetype s_blitzrise3 = {false,SPR_BLITZ_RISE3,8,ActorMovement,0,&s_blitzrise4};
statetype s_blitzrise2 = {false,SPR_BLITZ_RISE2,8,ActorMovement,0,&s_blitzrise3};
statetype s_blitzrise1 = {false,SPR_BLITZ_RISE1,8,ActorMovement,0,&s_blitzrise2};

statetype s_blitzuse    = {true,SPR_BLITZ_USE,10,T_Use,0,&s_blitzpath1};


statetype s_blitzsteal2 = {true,SPR_BLITZ_STEAL2,20,A_Steal,0,&s_blitzchase1};
statetype s_blitzsteal1 = {true,SPR_BLITZ_STEAL1,20,ActorMovement,0,&s_blitzsteal2};

statetype s_blitzchase4    = {true,SPR_BLITZ_W41,12,T_Chase,0,&s_blitzchase1};
statetype s_blitzchase3    = {true,SPR_BLITZ_W31,12,T_Chase,SF_CLOSE,&s_blitzchase4};
statetype s_blitzchase2    = {true,SPR_BLITZ_W21,12,T_Chase,0,&s_blitzchase3};
statetype s_blitzchase1    = {true,SPR_BLITZ_W11,12,T_Chase,SF_CLOSE,&s_blitzchase2};


statetype s_blitzdead2     = {false,SPR_BLITZ_DEAD2,0,T_Collide,0,&s_blitzdead2};
statetype s_blitzdead      = {false,SPR_BLITZ_DEAD1,0,T_Collide,0,&s_blitzdead2};
statetype s_blitzdie4      = {false,SPR_BLITZ_DIE4,7,T_Collide,0,&s_blitzdead};
statetype s_blitzdie3      = {false,SPR_BLITZ_DIE3,7,T_Collide,0,&s_blitzdie4};
statetype s_blitzdie2      = {false,SPR_BLITZ_DIE2,7,T_Collide,0,&s_blitzdie3};
statetype s_blitzdie1      = {false,SPR_BLITZ_DIE1,7,T_Collide,0,&s_blitzdie2};


statetype s_blitzstruggledead  = {false,SPR_BLITZ_RISE2,0,T_Collide,0,&s_blitzstruggledead};
statetype s_blitzstruggledie1  = {false,SPR_BLITZ_RISE2,20,T_ReallyDead,0,&s_blitzstruggledead};


statetype s_blitzfakedead  = {false,SPR_BLITZ_DEAD2,0,T_PlayDead,SF_FAKING,&s_blitzfakedead};
statetype s_blitzfakedie3  = {false,SPR_BLITZ_DEAD1,5,ActorMovement,0,&s_blitzfakedead};
statetype s_blitzfakedie2  = {false,SPR_BLITZ_DIE4,5,ActorMovement,0,&s_blitzfakedie3};
statetype s_blitzfakedie1  = {false,SPR_BLITZ_DIE3,5,ActorMovement,0,&s_blitzfakedie2};


statetype s_blitzcrushed2     = {false,SPR_BLITZ_DIE3,2,NULL,0,&s_blitzdead};
statetype s_blitzcrushed1		= {false,SPR_BLITZ_DIE1,2,NULL,0,&s_blitzcrushed2};

statetype s_blitzplead11    = {false,SPR_BLITZ_PLEAD11,0,T_Collide,0,&s_blitzplead11};
statetype s_blitzplead10    = {false,SPR_BLITZ_PLEAD10,6,NULL,0,&s_blitzplead11};
statetype s_blitzplead9    = {false,SPR_BLITZ_PLEAD9,6,NULL,0,&s_blitzplead10};
statetype s_blitzplead8    = {false,SPR_BLITZ_PLEAD8,6,NULL,0,&s_blitzplead9};
statetype s_blitzplead7    = {false,SPR_BLITZ_PLEAD7,6,NULL,0,&s_blitzplead8};




statetype s_blitzaplead4    = {false,SPR_BLITZ_PLEAD4,5,T_Plead,SF_DOWN,&s_blitzplead3};
statetype s_blitzaplead5    = {false,SPR_BLITZ_PLEAD5,5,T_Plead,SF_DOWN,&s_blitzaplead4};

statetype s_blitzplead6    = {false,SPR_BLITZ_PLEAD6,5,T_Plead,SF_DOWN,&s_blitzaplead5};
statetype s_blitzplead5    = {false,SPR_BLITZ_PLEAD5,5,T_Plead,SF_DOWN,&s_blitzplead6};
statetype s_blitzplead4    = {false,SPR_BLITZ_PLEAD4,5,T_Plead,SF_DOWN,&s_blitzplead5};
statetype s_blitzplead3    = {false,SPR_BLITZ_PLEAD3,5,T_Plead,SF_DOWN,&s_blitzplead4};
statetype s_blitzplead2    = {false,SPR_BLITZ_PLEAD2,6,ActorMovement,0,&s_blitzplead3};
statetype s_blitzplead1    = {false,SPR_BLITZ_PLEAD1,6,ActorMovement,0,&s_blitzplead2};



/*
============================================================================

								TRIAD ENFORCERS

============================================================================
*/


statetype s_enforcerstand	= {true,SPR_ENFORCER_S1,0,T_Stand,0,&s_enforcerstand};


statetype s_enforcerpath4  = {true,SPR_ENFORCER_W41,12,T_Path,0,&s_enforcerpath1};
statetype s_enforcerpath3  = {true,SPR_ENFORCER_W31,12,T_Path,SF_CLOSE,&s_enforcerpath4};
statetype s_enforcerpath2  = {true,SPR_ENFORCER_W21,12,T_Path,0,&s_enforcerpath3};
statetype s_enforcerpath1  = {true,SPR_ENFORCER_W11,12,T_Path,SF_CLOSE,&s_enforcerpath2};


statetype s_enforcerchase4    = {true,SPR_ENFORCER_W41,12,T_Chase,0,&s_enforcerchase1};
statetype s_enforcerchase3    = {true,SPR_ENFORCER_W31,12,T_Chase,SF_CLOSE,&s_enforcerchase4};
statetype s_enforcerchase2    = {true,SPR_ENFORCER_W21,12,T_Chase,0,&s_enforcerchase3};
statetype s_enforcerchase1    = {true,SPR_ENFORCER_W11,12,T_Chase,SF_CLOSE,&s_enforcerchase2};

statetype s_enforcercollide  = {false,SPR_ENFORCER_PAIN1,0,T_Collide,0,&s_enforcercollide};
statetype s_enforcercollide2 = {false,SPR_ENFORCER_PAIN2,0,T_Collide,0,&s_enforcercollide2};


//statetype s_enforceruse2    = {true,SPR_ENFORCER_USE21,10,T_Use,0,&s_enforcerchase1};
//statetype s_enforceruse1    = {true,SPR_ENFORCER_USE11,10,T_Use,0,&s_enforceruse2};

statetype s_enforcershoot4 = {false,SPR_ENFORCER_SHOOT4,6,A_Repeat,0,&s_enforcershoot3};
statetype s_enforcershoot3 = {false,SPR_ENFORCER_SHOOT3,6,A_Shoot,0,&s_enforcershoot4};
statetype s_enforcershoot2 = {false,SPR_ENFORCER_SHOOT2,6,ActorMovement,0,&s_enforcershoot3};
statetype s_enforcershoot1 = {false,SPR_ENFORCER_SHOOT1,6,ActorMovement,0,&s_enforcershoot2};


statetype s_enforcerthrow8 = {false,SPR_ENFORCER_THROW8,10,ActorMovement,0,&s_enforcerchase1};
statetype s_enforcerthrow7 = {false,SPR_ENFORCER_THROW7,10,ActorMovement,0,&s_enforcerthrow8};
statetype s_enforcerthrow6 = {false,SPR_ENFORCER_THROW6,10,ActorMovement,0,&s_enforcerthrow7};
statetype s_enforcerthrow5 = {false,SPR_ENFORCER_THROW5,10,A_MissileWeapon,0,&s_enforcerthrow6};
statetype s_enforcerthrow4 = {false,SPR_ENFORCER_THROW4,10,ActorMovement,0,&s_enforcerthrow5};
statetype s_enforcerthrow3 = {false,SPR_ENFORCER_THROW3,10,ActorMovement,0,&s_enforcerthrow4};
statetype s_enforcerthrow2 = {false,SPR_ENFORCER_THROW2,10,ActorMovement,0,&s_enforcerthrow3};
statetype s_enforcerthrow1 = {false,SPR_ENFORCER_THROW1,10,ActorMovement,0,&s_enforcerthrow2};



statetype s_grenade10      = {false,SPR_ENFORCER_GR10,2,T_Projectile,0,&s_grenade1};
statetype s_grenade9       = {false,SPR_ENFORCER_GR9,2,T_Projectile,0,&s_grenade10};
statetype s_grenade8       = {false,SPR_ENFORCER_GR8,2,T_Projectile,0,&s_grenade9};
statetype s_grenade7       = {false,SPR_ENFORCER_GR7,2,T_Projectile,0,&s_grenade8};
statetype s_grenade6       = {false,SPR_ENFORCER_GR6,2,T_Projectile,0,&s_grenade7};
statetype s_grenade5       = {false,SPR_ENFORCER_GR5,2,T_Projectile,0,&s_grenade6};
statetype s_grenade4       = {false,SPR_ENFORCER_GR4,2,T_Projectile,0,&s_grenade5};
statetype s_grenade3       = {false,SPR_ENFORCER_GR3,2,T_Projectile,0,&s_grenade4};
statetype s_grenade2       = {false,SPR_ENFORCER_GR2,2,T_Projectile,0,&s_grenade3};
statetype s_grenade1       = {false,SPR_ENFORCER_GR1,2,T_Projectile,0,&s_grenade2};

statetype s_grenade_fall6  = {false,SPR_ENFORCER_FALL6,6,T_Projectile,0,&s_grenade_fall6};
statetype s_grenade_fall5  = {false,SPR_ENFORCER_FALL5,6,T_Projectile,0,&s_grenade_fall6};
statetype s_grenade_fall4  = {false,SPR_ENFORCER_FALL4,6,T_Projectile,0,&s_grenade_fall5};
statetype s_grenade_fall3  = {false,SPR_ENFORCER_FALL3,6,T_Projectile,0,&s_grenade_fall4};
statetype s_grenade_fall2  = {false,SPR_ENFORCER_FALL1,6,T_Projectile,0,&s_grenade_fall3};
statetype s_grenade_fall1  = {false,SPR_ENFORCER_FALL1,6,T_Projectile,0,&s_grenade_fall2};

statetype s_grenadehit3     = {false,SPR_GRENADE_HIT3,5,NULL,0,NULL};
statetype s_grenadehit2     = {false,SPR_GRENADE_HIT2,5,NULL,0,&s_grenadehit3};
statetype s_grenadehit1     = {false,SPR_GRENADE_HIT1,5,NULL,0,&s_grenadehit2};


statetype s_enforcerdead      = {false,SPR_ENFORCER_DEAD,0,T_Collide,0,&s_enforcerdead};
statetype s_enforcerdie4      = {false,SPR_ENFORCER_DIE4,7,T_Collide,0,&s_enforcerdead};
statetype s_enforcerdie3      = {false,SPR_ENFORCER_DIE3,7,T_Collide,0,&s_enforcerdie4};
statetype s_enforcerdie2      = {false,SPR_ENFORCER_DIE2,7,T_Collide,0,&s_enforcerdie3};
statetype s_enforcerdie1      = {false,SPR_ENFORCER_DIE1,7,T_Collide,0,&s_enforcerdie2};



statetype s_enforcercrushed2		= {false,SPR_ENFORCER_DIE3,2,NULL,0,&s_enforcerdead};
statetype s_enforcercrushed1		= {false,SPR_ENFORCER_DIE1,2,NULL,0,&s_enforcercrushed2};

/*
============================================================================

								 ROBOT GUARD

============================================================================
*/



statetype s_robogrdstand	= {16,SPR_ROBOGRD_S11,0,T_Stand,0,&s_robogrdstand};

statetype s_robogrdpath1 	= {16,SPR_ROBOGRD_S11,20,T_AutoPath,0,&s_robogrdpath1};

statetype s_robowait = {16,SPR_ROBOGRD_S11,0,NULL,0,&s_robowait};
statetype s_roborealign = {16,SPR_ROBOGRD_S11,0,T_AutoRealign,0,&s_roborealign};
statetype s_roboalign = {16,SPR_ROBOGRD_S11,0,T_AutoShootAlign,0,&s_roboalign};

statetype s_robogrdshoot1 	= {false,SPR_ROBOGRD_S11,35,A_MissileWeapon,0,&s_roborealign};

statetype s_robogrdshuriken4 	= {false,SPR_ROBOGRD_SHURIKEN4,4,T_Projectile,0,&s_robogrdshuriken1};
statetype s_robogrdshuriken3 	= {false,SPR_ROBOGRD_SHURIKEN3,4,T_Projectile,0,&s_robogrdshuriken4};
statetype s_robogrdshuriken2 	= {false,SPR_ROBOGRD_SHURIKEN2,4,T_Projectile,0,&s_robogrdshuriken3};
statetype s_robogrdshuriken1  = {false,SPR_ROBOGRD_SHURIKEN1,4,T_Projectile,0,&s_robogrdshuriken2};

statetype s_shurikenhit3    = {false,SPR_SHURIKEN_HIT3,5,NULL,0,NULL};
statetype s_shurikenhit2    = {false,SPR_SHURIKEN_HIT2,5,NULL,0,&s_shurikenhit3};
statetype s_shurikenhit1    = {false,SPR_SHURIKEN_HIT1,5,NULL,0,&s_shurikenhit2};

statetype s_robogrdcollide  = {16,SPR_ROBOGRD_S11,0,T_Collide,0,&s_robogrdcollide};
statetype s_robogrdcollide2 = {16,SPR_ROBOGRD_S11,0,T_Collide,0,&s_robogrdcollide2};

//statetype s_robogrdchase1 	= {16,SPR_ROBOGRD_S11,10,T_RoboChase,0,&s_robogrdchase1};

statetype s_robogrddead		= {false,ROBOGRDDEAD,0,T_Collide,0,&s_robogrddead};
statetype s_robogrddie9	   = {false,ROBOGRDDIE9,3,T_Collide,0,&s_robogrddead};
statetype s_robogrddie8	   = {false,ROBOGRDDIE8,3,T_Collide,0,&s_robogrddie9};
statetype s_robogrddie7	   = {false,ROBOGRDDIE7,3,T_Collide,0,&s_robogrddie8};
statetype s_robogrddie6	   = {false,ROBOGRDDIE6,3,T_Collide,0,&s_robogrddie7};
statetype s_robogrddie5	   = {false,ROBOGRDDIE5,3,T_Collide,0,&s_robogrddie6};
statetype s_robogrddie4	   = {false,ROBOGRDDIE4,3,T_Collide,0,&s_robogrddie5};
statetype s_robogrddie3	   = {false,ROBOGRDDIE3,3,T_Collide,0,&s_robogrddie4};
statetype s_robogrddie2	   = {false,ROBOGRDDIE2,3,T_Collide,0,&s_robogrddie3};
statetype s_robogrddie1	   = {false,ROBOGRDDIE1,3,T_Collide,0,&s_robogrddie2};


statetype s_bstar4 = {false,SPR_BSTAR4,1,T_Projectile,0,&s_bstar1};
statetype s_bstar3 = {false,SPR_BSTAR3,1,T_Projectile,0,&s_bstar4};
statetype s_bstar2 = {false,SPR_BSTAR2,1,T_Projectile,0,&s_bstar3};
statetype s_bstar1 = {false,SPR_BSTAR1,1,T_Projectile,0,&s_bstar2};



/*
==========================================================================
=
=                             Explosions
=
==========================================================================
*/

statetype s_altexplosion10 = {false,SPR_EXPLOSION19,3,NULL,0,NULL};
statetype s_altexplosion9 = {false,SPR_EXPLOSION17,3,NULL,0,&s_altexplosion10};
statetype s_altexplosion8 = {false,SPR_EXPLOSION15,3,NULL,0,&s_altexplosion9};
statetype s_altexplosion7 = {false,SPR_EXPLOSION13,3,NULL,0,&s_altexplosion8};
statetype s_altexplosion6 = {false,SPR_EXPLOSION11,3,NULL,0,&s_altexplosion7};
statetype s_altexplosion5 = {false,SPR_EXPLOSION9,3,NULL,0,&s_altexplosion6};
statetype s_altexplosion4 = {false,SPR_EXPLOSION7,3,NULL,0,&s_altexplosion5};
statetype s_altexplosion3 = {false,SPR_EXPLOSION5,3,NULL,0,&s_altexplosion4};
statetype s_altexplosion2 = {false,SPR_EXPLOSION3,3,T_Explosion,0,&s_altexplosion3};
statetype s_altexplosion1 = {false,SPR_EXPLOSION1,3,NULL,0,&s_altexplosion2};





statetype s_explosion20 = {false,SPR_EXPLOSION20,2,NULL,0,NULL};
statetype s_explosion19 = {false,SPR_EXPLOSION19,2,NULL,0,&s_explosion20};
statetype s_explosion18 = {false,SPR_EXPLOSION18,2,NULL,0,&s_explosion19};
statetype s_explosion17 = {false,SPR_EXPLOSION17,2,NULL,0,&s_explosion18};
statetype s_explosion16 = {false,SPR_EXPLOSION16,2,NULL,0,&s_explosion17};
statetype s_explosion15 = {false,SPR_EXPLOSION15,2,NULL,0,&s_explosion16};
statetype s_explosion14 = {false,SPR_EXPLOSION14,2,NULL,0,&s_explosion15};
statetype s_explosion13 = {false,SPR_EXPLOSION13,2,NULL,0,&s_explosion14};
statetype s_explosion12 = {false,SPR_EXPLOSION12,2,NULL,0,&s_explosion13};
statetype s_explosion11 = {false,SPR_EXPLOSION11,2,NULL,0,&s_explosion12};
statetype s_explosion10 = {false,SPR_EXPLOSION10,2,NULL,0,&s_explosion11};
statetype s_explosion9 = {false,SPR_EXPLOSION9,2,NULL,0,&s_explosion10};
statetype s_explosion8 = {false,SPR_EXPLOSION8,2,NULL,0,&s_explosion9};
statetype s_explosion7 = {false,SPR_EXPLOSION7,2,NULL,0,&s_explosion8};
statetype s_explosion6 = {false,SPR_EXPLOSION6,2,NULL,0,&s_explosion7};
statetype s_explosion5 = {false,SPR_EXPLOSION5,2,NULL,0,&s_explosion6};
statetype s_explosion4 = {false,SPR_EXPLOSION4,2,NULL,0,&s_explosion5};
statetype s_explosion3 = {false,SPR_EXPLOSION3,2,T_Explosion,0,&s_explosion4};
statetype s_explosion2 = {false,SPR_EXPLOSION2,2,NULL,0,&s_explosion3};
statetype s_explosion1 = {false,SPR_EXPLOSION1,2,NULL,0,&s_explosion2};



statetype s_grexplosion20 = {false,SPR_GROUNDEXPL20,2,T_Special,0,NULL};
statetype s_grexplosion19 = {false,SPR_GROUNDEXPL19,2,NULL,0,&s_grexplosion20};
statetype s_grexplosion18 = {false,SPR_GROUNDEXPL18,2,NULL,0,&s_grexplosion19};
statetype s_grexplosion17 = {false,SPR_GROUNDEXPL17,2,NULL,0,&s_grexplosion18};
statetype s_grexplosion16 = {false,SPR_GROUNDEXPL16,2,NULL,0,&s_grexplosion17};
statetype s_grexplosion15 = {false,SPR_GROUNDEXPL15,2,NULL,0,&s_grexplosion16};
statetype s_grexplosion14 = {false,SPR_GROUNDEXPL14,2,NULL,0,&s_grexplosion15};
statetype s_grexplosion13 = {false,SPR_GROUNDEXPL13,2,NULL,0,&s_grexplosion14};
statetype s_grexplosion12 = {false,SPR_GROUNDEXPL12,2,NULL,0,&s_grexplosion13};
statetype s_grexplosion11 = {false,SPR_GROUNDEXPL11,2,NULL,0,&s_grexplosion12};
statetype s_grexplosion10 = {false,SPR_GROUNDEXPL10,2,NULL,0,&s_grexplosion11};
statetype s_grexplosion9 = {false,SPR_GROUNDEXPL9,2,NULL,0,&s_grexplosion10};
statetype s_grexplosion8 = {false,SPR_GROUNDEXPL8,2,NULL,0,&s_grexplosion9};
statetype s_grexplosion7 = {false,SPR_GROUNDEXPL7,2,NULL,0,&s_grexplosion8};
statetype s_grexplosion6 = {false,SPR_GROUNDEXPL6,2,NULL,0,&s_grexplosion7};
statetype s_grexplosion5 = {false,SPR_GROUNDEXPL5,2,NULL,0,&s_grexplosion6};
statetype s_grexplosion4 = {false,SPR_GROUNDEXPL4,2,NULL,0,&s_grexplosion5};
statetype s_grexplosion3 = {false,SPR_GROUNDEXPL3,2,T_Explosion,0,&s_grexplosion4};
statetype s_grexplosion2 = {false,SPR_GROUNDEXPL2,2,NULL,0,&s_grexplosion3};
statetype s_grexplosion1 = {false,SPR_GROUNDEXPL1,2,NULL,0,&s_grexplosion2};



statetype s_staticexplosion25 = {false,SPR_STATICEXPL25,2,NULL,0,NULL};
statetype s_staticexplosion24 = {false,SPR_STATICEXPL24,2,NULL,0,&s_staticexplosion25};
statetype s_staticexplosion23 = {false,SPR_STATICEXPL23,2,NULL,0,&s_staticexplosion24};
statetype s_staticexplosion22 = {false,SPR_STATICEXPL22,2,NULL,0,&s_staticexplosion23};
statetype s_staticexplosion21 = {false,SPR_STATICEXPL21,2,NULL,0,&s_staticexplosion22};
statetype s_staticexplosion20 = {false,SPR_STATICEXPL20,2,NULL,0,&s_staticexplosion21};
statetype s_staticexplosion19 = {false,SPR_STATICEXPL19,2,NULL,0,&s_staticexplosion20};
statetype s_staticexplosion18 = {false,SPR_STATICEXPL18,2,NULL,0,&s_staticexplosion19};
statetype s_staticexplosion17 = {false,SPR_STATICEXPL17,2,NULL,0,&s_staticexplosion18};
statetype s_staticexplosion16 = {false,SPR_STATICEXPL16,2,NULL,0,&s_staticexplosion17};
statetype s_staticexplosion15 = {false,SPR_STATICEXPL15,2,NULL,0,&s_staticexplosion16};
statetype s_staticexplosion14 = {false,SPR_STATICEXPL14,2,NULL,0,&s_staticexplosion15};
statetype s_staticexplosion13 = {false,SPR_STATICEXPL13,2,NULL,0,&s_staticexplosion14};
statetype s_staticexplosion12 = {false,SPR_STATICEXPL12,2,NULL,0,&s_staticexplosion13};
statetype s_staticexplosion11 = {false,SPR_STATICEXPL11,2,NULL,0,&s_staticexplosion12};
statetype s_staticexplosion10 = {false,SPR_STATICEXPL10,2,NULL,0,&s_staticexplosion11};
statetype s_staticexplosion9 = {false,SPR_STATICEXPL9,2,NULL,0,&s_staticexplosion10};
statetype s_staticexplosion8 = {false,SPR_STATICEXPL8,2,NULL,0,&s_staticexplosion9};
statetype s_staticexplosion7 = {false,SPR_STATICEXPL7,2,NULL,0,&s_staticexplosion8};
statetype s_staticexplosion6 = {false,SPR_STATICEXPL6,2,NULL,0,&s_staticexplosion7};
statetype s_staticexplosion5 = {false,SPR_STATICEXPL5,2,NULL,0,&s_staticexplosion6};
statetype s_staticexplosion4 = {false,SPR_STATICEXPL4,2,NULL,0,&s_staticexplosion5};
statetype s_staticexplosion3 = {false,SPR_STATICEXPL3,2,T_Explosion,0,&s_staticexplosion4};
statetype s_staticexplosion2 = {false,SPR_STATICEXPL2,2,NULL,0,&s_staticexplosion3};
statetype s_staticexplosion1 = {false,SPR_STATICEXPL1,2,NULL,0,&s_staticexplosion2};



statetype s_upblade16 = {false,UBLADE8,1,T_Path,0,&s_upblade1};
statetype s_upblade15 = {false,UBLADE7,2,T_Path,0,&s_upblade16};
statetype s_upblade14 = {false,UBLADE6,1,T_Path,0,&s_upblade15};
statetype s_upblade13 = {false,UBLADE5,2,T_Path,0,&s_upblade14};
statetype s_upblade12 = {false,UBLADE4,1,T_Path,0,&s_upblade13};
statetype s_upblade11 = {false,UBLADE3,2,T_Path,0,&s_upblade12};
statetype s_upblade10 = {false,UBLADE2,1,T_Path,0,&s_upblade11};
statetype s_upblade9 = {false,UBLADE9,2,T_Path,0,&s_upblade10};
statetype s_upblade8 = {false,UBLADE8,1,T_Path,0,&s_upblade9};
statetype s_upblade7 = {false,UBLADE7,2,T_Path,0,&s_upblade8};
statetype s_upblade6 = {false,UBLADE6,1,T_Path,0,&s_upblade7};
statetype s_upblade5 = {false,UBLADE5,2,T_Path,0,&s_upblade6};
statetype s_upblade4 = {false,UBLADE4,1,T_Path,0,&s_upblade5};
statetype s_upblade3 = {false,UBLADE3,2,T_Path,0,&s_upblade4};
statetype s_upblade2 = {false,UBLADE2,1,T_Path,0,&s_upblade3};
statetype s_upblade1 = {false,UBLADE1,2,T_Path,SF_SOUND,&s_upblade2};


statetype s_firejetup23 = {false,FIREJETUP23,3,T_Path,SF_CRUSH,&s_firejetup1};
statetype s_firejetup22 = {false,FIREJETUP22,3,T_Path,SF_CRUSH,&s_firejetup23};
statetype s_firejetup21 = {false,FIREJETUP21,3,T_Path,0,&s_firejetup22};
statetype s_firejetup20 = {false,FIREJETUP20,3,T_Path,0,&s_firejetup21};
statetype s_firejetup19 = {false,FIREJETUP19,3,T_Path,0,&s_firejetup20};
statetype s_firejetup18 = {false,FIREJETUP18,3,T_Path,0,&s_firejetup19};
statetype s_firejetup17 = {false,FIREJETUP17,3,T_Path,0,&s_firejetup18};
statetype s_firejetup16 = {false,FIREJETUP16,3,T_Path,SF_CRUSH,&s_firejetup17};
statetype s_firejetup15 = {false,FIREJETUP15,3,T_Path,SF_CRUSH,&s_firejetup16};
statetype s_firejetup14 = {false,FIREJETUP14,3,T_Path,SF_CRUSH,&s_firejetup15};
statetype s_firejetup13 = {false,FIREJETUP13,3,T_Path,0,&s_firejetup14};
statetype s_firejetup12 = {false,FIREJETUP12,3,T_Path,0,&s_firejetup13};
statetype s_firejetup11 = {false,FIREJETUP11,3,T_Path,0,&s_firejetup12};
statetype s_firejetup10 = {false,FIREJETUP10,3,T_Path,0,&s_firejetup11};
statetype s_firejetup9 = {false,FIREJETUP9,3,T_Path,0,&s_firejetup10};
statetype s_firejetup8 = {false,FIREJETUP8,3,T_Path,SF_CRUSH,&s_firejetup9};
statetype s_firejetup7 = {false,FIREJETUP7,3,T_Path,SF_CRUSH,&s_firejetup8};
statetype s_firejetup6 = {false,FIREJETUP6,3,T_Path,SF_CRUSH,&s_firejetup7};
statetype s_firejetup5 = {false,FIREJETUP5,3,T_Path,0,&s_firejetup6};
statetype s_firejetup4 = {false,FIREJETUP4,3,T_Path,0,&s_firejetup5};
statetype s_firejetup3 = {false,FIREJETUP3,3,T_Path,0,&s_firejetup4};
statetype s_firejetup2 = {false,FIREJETUP2,3,T_Path,0,&s_firejetup3};
statetype s_firejetup1 = {false,FIREJETUP1,70,T_Path,SF_SOUND,&s_firejetup2};



statetype s_columndownup6 = {false,CRUSHDOWN7,5,NULL,0,&s_columndowndown1};
statetype s_columndownup5 = {false,CRUSHDOWN6,5,NULL,0,&s_columndownup6};
statetype s_columndownup4 = {false,CRUSHDOWN5,5,NULL,SF_BLOCK,&s_columndownup5};
statetype s_columndownup3 = {false,CRUSHDOWN4,5,NULL,SF_BLOCK,&s_columndownup4};
statetype s_columndownup2 = {false,CRUSHDOWN3,5,NULL,SF_BLOCK,&s_columndownup3};
statetype s_columndownup1 = {false,CRUSHDOWN2,5,NULL,SF_BLOCK,&s_columndownup2};


statetype s_columndowndown8 = {false,CRUSHDOWN1,5,T_CrushDown,SF_CRUSH|SF_DOWN|SF_BLOCK,&s_columndownup1};
statetype s_columndowndown7 = {false,CRUSHDOWN2,5,T_CrushDown,SF_CRUSH|SF_DOWN|SF_BLOCK,&s_columndowndown8};
statetype s_columndowndown6 = {false,CRUSHDOWN3,5,T_CrushDown,SF_DOWN|SF_BLOCK,&s_columndowndown7};
statetype s_columndowndown5 = {false,CRUSHDOWN4,5,T_CrushDown,SF_DOWN|SF_BLOCK,&s_columndowndown6};
statetype s_columndowndown4 = {false,CRUSHDOWN5,5,T_CrushDown,SF_DOWN,&s_columndowndown5};
statetype s_columndowndown3 = {false,CRUSHDOWN6,5,T_CrushDown,SF_DOWN,&s_columndowndown4};
statetype s_columndowndown2 = {false,CRUSHDOWN7,5,T_CrushDown,SF_DOWN,&s_columndowndown3};
statetype s_columndowndown1 = {false,CRUSHDOWN8,30,T_CrushDown,SF_DOWN|SF_SOUND,&s_columndowndown2};


statetype s_spearup16 = {false,SPEARUP16,2,T_Spears,SF_CRUSH,&s_spearup1};
statetype s_spearup15 = {false,SPEARUP15,2,T_Spears,SF_CRUSH,&s_spearup16};
statetype s_spearup14 = {false,SPEARUP14,2,T_Spears,SF_CRUSH,&s_spearup15};
statetype s_spearup13 = {false,SPEARUP13,2,T_Spears,SF_CRUSH,&s_spearup14};
statetype s_spearup12 = {false,SPEARUP12,2,T_Spears,SF_CRUSH,&s_spearup13};
statetype s_spearup11 = {false,SPEARUP11,2,T_Spears,SF_CRUSH,&s_spearup12};
statetype s_spearup10 = {false,SPEARUP10,2,T_Spears,SF_CRUSH,&s_spearup11};
statetype s_spearup9 = {false,SPEARUP9,2,T_Spears,SF_DOWN,&s_spearup10};

statetype s_spearup8 = {false,SPEARUP8,35,T_Spears,SF_DOWN|SF_SOUND,&s_spearup9};
statetype s_spearup7 = {false,SPEARUP7,2,T_Spears,SF_DOWN,&s_spearup8};
statetype s_spearup6 = {false,SPEARUP6,2,T_Spears,SF_CRUSH,&s_spearup7};
statetype s_spearup5 = {false,SPEARUP5,2,T_Spears,SF_CRUSH,&s_spearup6};
statetype s_spearup4 = {false,SPEARUP4,2,T_Spears,SF_CRUSH,&s_spearup5};
statetype s_spearup3 = {false,SPEARUP3,2,T_Spears,SF_CRUSH,&s_spearup4};
statetype s_spearup2 = {false,SPEARUP2,2,T_Spears,SF_CRUSH,&s_spearup3};
statetype s_spearup1 = {false,SPEARUP1,2,T_Spears,SF_CRUSH,&s_spearup2};


statetype s_dust = {false,NOTHING,0,NULL,0,&s_dust};



statetype s_gas2 = {false,SPR42_GRATE,5,T_Count,0,&s_gas2};
statetype s_gas1 = {false,SPR42_GRATE,0,NULL,0,&s_gas1};


//================== player stuff =======================================/

statetype s_p_bazooka1 = {16,SPR_BJMISS11,3,T_Projectile,0,&s_p_bazooka1};

statetype s_p_grenade = {16,SPR_BJMISS11,3,T_Projectile,0,&s_p_grenade};

/*
statetype s_p_misssmoke4 = {false,MISSSMOKE4,7,NULL,0,NULL};
statetype s_p_misssmoke3 = {false,MISSSMOKE3,7,NULL,0,&s_p_misssmoke4};
statetype s_p_misssmoke2 = {false,MISSSMOKE2,7,NULL,0,&s_p_misssmoke3};
statetype s_p_misssmoke1 = {false,MISSSMOKE1,3,NULL,0,&s_p_misssmoke2};
*/


statetype s_basemarker8 = {false,FLASH8,3,NULL,0,&s_basemarker1};
statetype s_basemarker7 = {false,FLASH7,3,NULL,0,&s_basemarker8};
statetype s_basemarker6 = {false,FLASH6,3,NULL,0,&s_basemarker7};
statetype s_basemarker5 = {false,FLASH5,3,NULL,0,&s_basemarker6};
statetype s_basemarker4 = {false,FLASH4,3,NULL,0,&s_basemarker5};
statetype s_basemarker3 = {false,FLASH3,3,NULL,0,&s_basemarker4};
statetype s_basemarker2 = {false,FLASH2,3,NULL,0,&s_basemarker3};
statetype s_basemarker1 = {false,FLASH1,3,NULL,0,&s_basemarker2};



statetype s_flash8 = {false,FLASH8,3,NULL,0,NULL};
statetype s_flash7 = {false,FLASH7,3,NULL,0,&s_flash8};
statetype s_flash6 = {false,FLASH6,3,NULL,0,&s_flash7};
statetype s_flash5 = {false,FLASH5,3,NULL,0,&s_flash6};
statetype s_flash4 = {false,FLASH4,3,NULL,0,&s_flash5};
statetype s_flash3 = {false,FLASH3,3,NULL,0,&s_flash4};
statetype s_flash2 = {false,FLASH2,3,NULL,0,&s_flash3};
statetype s_flash1 = {false,FLASH1,3,NULL,0,&s_flash2};

statetype s_gunsmoke8 = {false,GUNSMOKE8,3,NULL,0,NULL};
statetype s_gunsmoke7 = {false,GUNSMOKE7,3,NULL,0,&s_gunsmoke8};
statetype s_gunsmoke6 = {false,GUNSMOKE6,3,NULL,0,&s_gunsmoke7};
statetype s_gunsmoke5 = {false,GUNSMOKE5,3,NULL,0,&s_gunsmoke6};
statetype s_gunsmoke4 = {false,GUNSMOKE4,3,NULL,0,&s_gunsmoke5};
statetype s_gunsmoke3 = {false,GUNSMOKE3,3,NULL,0,&s_gunsmoke4};
statetype s_gunsmoke2 = {false,GUNSMOKE2,3,NULL,0,&s_gunsmoke3};
statetype s_gunsmoke1 = {false,GUNSMOKE1,2,NULL,0,&s_gunsmoke2};

statetype s_bloodspurt8 = {false,BLOODSPURT8,7,NULL,0,NULL};
statetype s_bloodspurt7 = {false,BLOODSPURT7,7,NULL,0,&s_bloodspurt8};
statetype s_bloodspurt6 = {false,BLOODSPURT6,7,NULL,0,&s_bloodspurt7};
statetype s_bloodspurt5 = {false,BLOODSPURT5,7,NULL,0,&s_bloodspurt6};
statetype s_bloodspurt4 = {false,BLOODSPURT4,7,NULL,0,&s_bloodspurt5};
statetype s_bloodspurt3 = {false,BLOODSPURT3,7,NULL,0,&s_bloodspurt4};
statetype s_bloodspurt2 = {false,BLOODSPURT2,7,NULL,0,&s_bloodspurt3};
statetype s_bloodspurt1 = {false,BLOODSPURT1,5,NULL,0,&s_bloodspurt2};

statetype s_hitmetalwall4 = {false,HITMETALWALL4,7,NULL,0,NULL};
statetype s_hitmetalwall3 = {false,HITMETALWALL3,7,NULL,0,&s_hitmetalwall4};
statetype s_hitmetalwall2 = {false,HITMETALWALL2,7,NULL,0,&s_hitmetalwall3};
statetype s_hitmetalwall1 = {false,HITMETALWALL1,5,NULL,0,&s_hitmetalwall2};

statetype s_hitmetalactor4 = {false,HITMETALACTOR4,7,NULL,0,NULL};
statetype s_hitmetalactor3 = {false,HITMETALACTOR3,7,NULL,0,&s_hitmetalactor4};
statetype s_hitmetalactor2 = {false,HITMETALACTOR2,7,NULL,0,&s_hitmetalactor3};
statetype s_hitmetalactor1 = {false,HITMETALACTOR1,5,NULL,0,&s_hitmetalactor2};



statetype s_fireunit15 = {false,FIREW15,3,T_Firethink,0,&s_fireunit1};
statetype s_fireunit14 = {false,FIREW14,3,T_Firethink,0,&s_fireunit15};
statetype s_fireunit13 = {false,FIREW13,3,T_Firethink,0,&s_fireunit14};
statetype s_fireunit12 = {false,FIREW12,3,T_Firethink,0,&s_fireunit13};
statetype s_fireunit11 = {false,FIREW11,3,T_Firethink,0,&s_fireunit12};
statetype s_fireunit10 = {false,FIREW10,3,T_Firethink,0,&s_fireunit11};
statetype s_fireunit9 = {false,FIREW9,3,T_Firethink,0,&s_fireunit10};
statetype s_fireunit8 = {false,FIREW8,3,T_Firethink,0,&s_fireunit9};
statetype s_fireunit7 = {false,FIREW7,3,T_Firethink,0,&s_fireunit8};
statetype s_fireunit6 = {false,FIREW6,3,T_Firethink,0,&s_fireunit7};
statetype s_fireunit5 = {false,FIREW5,3,T_Firethink,0,&s_fireunit6};
statetype s_fireunit4 = {false,FIREW4,3,T_Firethink,0,&s_fireunit5};
statetype s_fireunit3 = {false,FIREW3,3,T_Firethink,0,&s_fireunit4};
statetype s_fireunit2 = {false,FIREW2,3,T_Firethink,0,&s_fireunit3};
statetype s_fireunit1 = {false,FIREW1,3,T_Firethink,0,&s_fireunit2};



statetype s_skeleton48 = {false,SKELETON48,0,T_Convert,0,&s_skeleton48};
statetype s_skeleton47 = {false,SKELETON47,2,NULL,0,&s_skeleton48};
statetype s_skeleton46 = {false,SKELETON46,2,NULL,0,&s_skeleton47};
statetype s_skeleton45 = {false,SKELETON45,2,NULL,0,&s_skeleton46};
statetype s_skeleton44 = {false,SKELETON44,2,NULL,0,&s_skeleton45};
statetype s_skeleton43 = {false,SKELETON43,2,NULL,0,&s_skeleton44};
statetype s_skeleton42 = {false,SKELETON42,2,NULL,0,&s_skeleton43};
statetype s_skeleton41 = {false,SKELETON41,2,T_Xylophone,0,&s_skeleton42};
statetype s_skeleton40 = {false,SKELETON40,2,NULL,0,&s_skeleton41};
statetype s_skeleton39 = {false,SKELETON39,2,NULL,0,&s_skeleton40};
statetype s_skeleton38 = {false,SKELETON38,2,NULL,0,&s_skeleton39};
statetype s_skeleton37 = {false,SKELETON37,2,NULL,0,&s_skeleton38};
statetype s_skeleton36 = {false,SKELETON36,2,NULL,0,&s_skeleton37};
statetype s_skeleton35 = {false,SKELETON35,2,NULL,0,&s_skeleton36};
statetype s_skeleton34 = {false,SKELETON34,2,NULL,0,&s_skeleton35};
statetype s_skeleton33 = {false,SKELETON33,2,NULL,0,&s_skeleton34};
statetype s_skeleton32 = {false,SKELETON32,2,NULL,0,&s_skeleton33};
statetype s_skeleton31 = {false,SKELETON31,2,NULL,0,&s_skeleton32};
statetype s_skeleton30 = {false,SKELETON30,2,NULL,0,&s_skeleton31};
statetype s_skeleton29 = {false,SKELETON29,2,NULL,0,&s_skeleton30};
statetype s_skeleton28 = {false,SKELETON28,2,NULL,0,&s_skeleton29};
statetype s_skeleton27 = {false,SKELETON27,2,NULL,0,&s_skeleton28};
statetype s_skeleton26 = {false,SKELETON26,2,NULL,0,&s_skeleton27};
statetype s_skeleton25 = {false,SKELETON25,2,NULL,0,&s_skeleton26};
statetype s_skeleton24 = {false,SKELETON24,2,NULL,0,&s_skeleton25};
statetype s_skeleton23 = {false,SKELETON23,2,NULL,0,&s_skeleton24};
statetype s_skeleton22 = {false,SKELETON22,2,NULL,0,&s_skeleton23};
statetype s_skeleton21 = {false,SKELETON21,2,NULL,0,&s_skeleton22};
statetype s_skeleton20 = {false,SKELETON20,2,NULL,0,&s_skeleton21};
statetype s_skeleton19 = {false,SKELETON19,2,NULL,0,&s_skeleton20};
statetype s_skeleton18 = {false,SKELETON18,2,NULL,0,&s_skeleton19};
statetype s_skeleton17 = {false,SKELETON17,2,NULL,0,&s_skeleton18};
statetype s_skeleton16 = {false,SKELETON16,2,NULL,0,&s_skeleton17};
statetype s_skeleton15 = {false,SKELETON15,2,NULL,0,&s_skeleton16};
statetype s_skeleton14 = {false,SKELETON14,2,NULL,0,&s_skeleton15};
statetype s_skeleton13 = {false,SKELETON13,2,NULL,0,&s_skeleton14};
statetype s_skeleton12 = {false,SKELETON12,2,NULL,0,&s_skeleton13};
statetype s_skeleton11 = {false,SKELETON11,2,NULL,0,&s_skeleton12};
statetype s_skeleton10 = {false,SKELETON10,2,NULL,0,&s_skeleton11};
statetype s_skeleton9 = {false,SKELETON9,2,NULL,0,&s_skeleton10};
statetype s_skeleton8 = {false,SKELETON8,2,NULL,0,&s_skeleton9};
statetype s_skeleton7 = {false,SKELETON7,2,NULL,0,&s_skeleton8};
statetype s_skeleton6 = {false,SKELETON6,2,NULL,0,&s_skeleton7};
statetype s_skeleton5 = {false,SKELETON5,2,NULL,0,&s_skeleton6};
statetype s_skeleton4 = {false,SKELETON4,2,NULL,0,&s_skeleton5};
statetype s_skeleton3 = {false,SKELETON3,2,NULL,0,&s_skeleton4};
statetype s_skeleton2 = {false,SKELETON2,2,NULL,0,&s_skeleton3};
statetype s_skeleton1 = {false,SKELETON1,2,NULL,0,&s_skeleton2};



statetype s_spring9 = {false,SPRING9,4,T_Spring,SF_DOWN,&s_spring1};
statetype s_spring8 = {false,SPRING8,4,NULL,0,&s_spring9};
statetype s_spring7 = {false,SPRING7,4,NULL,0,&s_spring8};
statetype s_spring6 = {false,SPRING6,4,NULL,0,&s_spring7};
statetype s_spring5 = {false,SPRING5,4,NULL,0,&s_spring6};
statetype s_spring4 = {false,SPRING4,15,NULL,SF_UP,&s_spring5};
statetype s_spring3 = {false,SPRING3,1,NULL,SF_UP,&s_spring4};
statetype s_spring2 = {false,SPRING2,1,NULL,SF_UP,&s_spring3};
statetype s_spring1 = {false,SPRING1,0,NULL,SF_UP,&s_spring1};


statetype s_autospring9 = {false,SPRING9,4,NULL,0,&s_autospring1};
statetype s_autospring8 = {false,SPRING8,4,NULL,0,&s_autospring9};
statetype s_autospring7 = {false,SPRING7,4,NULL,0,&s_autospring8};
statetype s_autospring6 = {false,SPRING6,4,NULL,0,&s_autospring7};
statetype s_autospring5 = {false,SPRING5,4,NULL,0,&s_autospring6};
statetype s_autospring4 = {false,SPRING4,15,NULL,SF_UP,&s_autospring5};
statetype s_autospring3 = {false,SPRING3,1,T_Spring,SF_UP,&s_autospring4};
statetype s_autospring2 = {false,SPRING2,1,T_Spring,SF_UP,&s_autospring3};
statetype s_autospring1 = {false,SPRING1,70*SPRINGDELAY,NULL,0,&s_autospring2};

//====================== remote player ==============================//

statetype s_itemspawn8 = {false,ITEMSPAWN8,6,NULL,0,NULL};
statetype s_itemspawn7 = {false,ITEMSPAWN7,6,NULL,0,&s_itemspawn8};
statetype s_itemspawn6 = {false,ITEMSPAWN6,6,NULL,0,&s_itemspawn7};
statetype s_itemspawn5 = {false,ITEMSPAWN5,6,NULL,0,&s_itemspawn6};
statetype s_itemspawn4 = {false,ITEMSPAWN4,6,NULL,0,&s_itemspawn5};
statetype s_itemspawn3 = {false,ITEMSPAWN3,6,NULL,0,&s_itemspawn4};
statetype s_itemspawn2 = {false,ITEMSPAWN2,6,NULL,0,&s_itemspawn3};
statetype s_itemspawn1 = {false,ITEMSPAWN1,6,NULL,0,&s_itemspawn2};


statetype s_player = {true,CASSATT_S1,0,T_Player,0,&s_player};

statetype s_pgunattack2 = {true,CASSATT_SHOOT11,0,T_Attack,0,&s_pgunattack2};
statetype s_pgunattack1 = {true,CASSATT_SHOOT21,5,T_Attack,0,&s_pgunattack2};

statetype s_pmissattack2 = {true,CASSATTM_SHOOT11,0,T_Attack,0,&s_pmissattack2};
statetype s_pmissattack1 = {true,CASSATTM_SHOOT21,5,T_Attack,0,&s_pmissattack2};

statetype s_pbatblast = {true,CASSATT_SHOOT21,0,T_BatBlast,0,&s_pbatblast};




statetype s_remotemove4 = {true,CASSATT_W41,5,T_Player,SF_DOWN,&s_remotemove1};
statetype s_remotemove3 = {true,CASSATT_W31,5,T_Player,SF_DOWN,&s_remotemove4};
statetype s_remotemove2 = {true,CASSATT_W21,5,T_Player,SF_DOWN,&s_remotemove3};
statetype s_remotemove1 = {true,CASSATT_W11,5,T_Player,SF_DOWN,&s_remotemove2};

statetype s_remoteinelev = {true,CASSATT_S1,700,T_Player,0,&s_remoteinelev};

statetype s_remotedead = {false,CASSATT_VDEAD,0,T_Player,0,&s_remotedead};



statetype s_remotedie6 = {false,CASSATT_VDIE6,3,T_Player,0,&s_remotedead};
statetype s_remotedie5 = {false,CASSATT_VDIE5,3,T_Player,0,&s_remotedie6};
statetype s_remotedie4 = {false,CASSATT_VDIE4,3,T_Player,0,&s_remotedie5};
statetype s_remotedie3 = {false,CASSATT_VDIE3,3,T_Player,0,&s_remotedie4};
statetype s_remotedie2 = {false,CASSATT_VDIE2,3,T_Player,0,&s_remotedie3};
statetype s_remotedie1 = {false,CASSATT_VDIE1,3,T_Player,0,&s_remotedie2};


statetype s_voidwait = {false,NOTHING,0,T_DeadWait,0,&s_voidwait};
statetype s_ashwait = {false,SKELETON48,0,T_DeadWait,0,&s_ashwait};
statetype s_deadwait = {false,CASSATT_VDEAD,0,T_DeadWait,0,&s_deadwait};
statetype s_gutwait = {false,GUTS12,0,T_DeadWait,0,&s_gutwait};
statetype s_remoteguts12 = {false,GUTS12,0,T_Player,0,&s_remoteguts12};
statetype s_remoteguts11 = {false,GUTS11,3,T_Player,0,&s_remoteguts12};
statetype s_remoteguts10 = {false,GUTS10,3,T_Player,0,&s_remoteguts11};
statetype s_remoteguts9 = {false,GUTS9,3,T_Player,0,&s_remoteguts10};
statetype s_remoteguts8 = {false,GUTS8,3,T_Player,0,&s_remoteguts9};
statetype s_remoteguts7 = {false,GUTS7,3,T_Player,0,&s_remoteguts8};
statetype s_remoteguts6 = {false,GUTS6,3,T_Player,0,&s_remoteguts7};
statetype s_remoteguts5 = {false,GUTS5,3,T_Player,0,&s_remoteguts6};
statetype s_remoteguts4 = {false,GUTS4,3,T_Player,0,&s_remoteguts5};
statetype s_remoteguts3 = {false,GUTS3,3,T_Player,0,&s_remoteguts4};
statetype s_remoteguts2 = {false,GUTS2,3,T_Player,0,&s_remoteguts3};
statetype s_remoteguts1 = {false,GUTS1,3,T_Player,0,&s_remoteguts2};

//========================================================================//

statetype s_godfire4 = {false,GODFIRE4,3,T_Projectile,0,&s_godfire1};
statetype s_godfire3 = {false,GODFIRE3,3,T_Projectile,0,&s_godfire4};
statetype s_godfire2 = {false,GODFIRE2,3,T_Projectile,0,&s_godfire3};
statetype s_godfire1 = {false,GODFIRE1,3,T_Projectile,0,&s_godfire2};




statetype s_guts12 = {false,GUTS12,0,T_Collide,SF_GUTS,&s_guts12};
statetype s_guts11 = {false,GUTS11,3,T_Collide,SF_GUTS,&s_guts12};
statetype s_guts10 = {false,GUTS10,3,T_Collide,SF_GUTS,&s_guts11};
statetype s_guts9 = {false,GUTS9,3,T_Collide,SF_GUTS,&s_guts10};
statetype s_guts8 = {false,GUTS8,3,T_Collide,SF_GUTS,&s_guts9};
statetype s_guts7 = {false,GUTS7,3,T_Collide,SF_GUTS,&s_guts8};
statetype s_guts6 = {false,GUTS6,3,T_Collide,SF_GUTS,&s_guts7};
statetype s_guts5 = {false,GUTS5,3,T_Collide,SF_GUTS,&s_guts6};
statetype s_guts4 = {false,GUTS4,3,T_Collide,SF_GUTS,&s_guts5};
statetype s_guts3 = {false,GUTS3,3,T_Collide,SF_GUTS,&s_guts4};
statetype s_guts2 = {false,GUTS2,3,T_Collide,SF_GUTS,&s_guts3};
statetype s_guts1 = {false,GUTS1,3,T_Collide,SF_GUTS,&s_guts2};

//MED
#if (SHAREWARE == 1) || (DOPEFISH == 0)
statetype s_collectorwander8 = {false,COLLECTOR15,0,T_CollectorWander,0,&s_collectorwander1};
statetype s_collectorwander7 = {false,COLLECTOR13,1,T_CollectorWander,0,&s_collectorwander8};
statetype s_collectorwander6 = {false,COLLECTOR11,0,T_CollectorWander,0,&s_collectorwander7};
statetype s_collectorwander5 = {false,COLLECTOR9,1,T_CollectorWander,0,&s_collectorwander6};
statetype s_collectorwander4 = {false,COLLECTOR7,0,T_CollectorWander,0,&s_collectorwander5};
statetype s_collectorwander3 = {false,COLLECTOR5,1,T_CollectorWander,0,&s_collectorwander4};
statetype s_collectorwander2 = {false,COLLECTOR3,0,T_CollectorWander,0,&s_collectorwander3};
statetype s_collectorwander1 = {false,COLLECTOR1,1,T_CollectorWander,0,&s_collectorwander2};
#else

statetype s_collectorwander8 = {false,DOPE8,2,T_CollectorWander,0,&s_collectorwander1};
statetype s_collectorwander7 = {false,DOPE7,2,T_CollectorWander,0,&s_collectorwander8};
statetype s_collectorwander6 = {false,DOPE6,2,T_CollectorWander,0,&s_collectorwander7};
statetype s_collectorwander5 = {false,DOPE5,2,T_CollectorWander,0,&s_collectorwander6};
statetype s_collectorwander4 = {false,DOPE4,2,T_CollectorWander,0,&s_collectorwander5};
statetype s_collectorwander3 = {false,DOPE3,2,T_CollectorWander,0,&s_collectorwander4};
statetype s_collectorwander2 = {false,DOPE2,2,T_CollectorWander,0,&s_collectorwander3};
statetype s_collectorwander1 = {false,DOPE1,2,T_CollectorWander,0,&s_collectorwander2};
#endif


//MED
#if (SHAREWARE == 1) || (DOPEFISH == 0)
statetype s_collectorfdoor8 = {false,COLLECTOR15,0,T_CollectorFindDoor,0,&s_collectorfdoor1};
statetype s_collectorfdoor7 = {false,COLLECTOR13,1,T_CollectorFindDoor,0,&s_collectorfdoor8};
statetype s_collectorfdoor6 = {false,COLLECTOR11,0,T_CollectorFindDoor,0,&s_collectorfdoor7};
statetype s_collectorfdoor5 = {false,COLLECTOR9,1,T_CollectorFindDoor,0,&s_collectorfdoor6};
statetype s_collectorfdoor4 = {false,COLLECTOR7,0,T_CollectorFindDoor,0,&s_collectorfdoor5};
statetype s_collectorfdoor3 = {false,COLLECTOR5,1,T_CollectorFindDoor,0,&s_collectorfdoor4};
statetype s_collectorfdoor2 = {false,COLLECTOR3,0,T_CollectorFindDoor,0,&s_collectorfdoor3};
statetype s_collectorfdoor1 = {false,COLLECTOR1,1,T_CollectorFindDoor,0,&s_collectorfdoor2};
#else

statetype s_collectorfdoor8 = {false,DOPE8,2,T_CollectorFindDoor,0,&s_collectorfdoor1};
statetype s_collectorfdoor7 = {false,DOPE7,2,T_CollectorFindDoor,0,&s_collectorfdoor8};
statetype s_collectorfdoor6 = {false,DOPE6,2,T_CollectorFindDoor,0,&s_collectorfdoor7};
statetype s_collectorfdoor5 = {false,DOPE5,2,T_CollectorFindDoor,0,&s_collectorfdoor6};
statetype s_collectorfdoor4 = {false,DOPE4,2,T_CollectorFindDoor,0,&s_collectorfdoor5};
statetype s_collectorfdoor3 = {false,DOPE3,2,T_CollectorFindDoor,0,&s_collectorfdoor4};
statetype s_collectorfdoor2 = {false,DOPE2,2,T_CollectorFindDoor,0,&s_collectorfdoor3};
statetype s_collectorfdoor1 = {false,DOPE1,2,T_CollectorFindDoor,0,&s_collectorfdoor2};

#endif

statetype s_timekeeper = {false,NOTHING,140,T_End,0,NULL};

statetype s_wind = {false,-1,10,T_Wind,0,&s_wind};


statetype s_deadblood8 = {false,NOTHING,0,T_Blood,0,&s_deadblood8};
statetype s_deadblood7 = {false,DEADBLOOD7,5,NULL,0,&s_deadblood8};
statetype s_deadblood6 = {false,DEADBLOOD6,5,NULL,0,&s_deadblood7};
statetype s_deadblood5 = {false,DEADBLOOD5,5,NULL,0,&s_deadblood6};
statetype s_deadblood4 = {false,DEADBLOOD4,5,NULL,0,&s_deadblood5};
statetype s_deadblood3 = {false,DEADBLOOD3,5,NULL,0,&s_deadblood4};
statetype s_deadblood2 = {false,DEADBLOOD2,5,NULL,0,&s_deadblood3};
statetype s_deadblood1 = {false,DEADBLOOD1,5,NULL,0,&s_deadblood2};

/*
statetype s_rain7 = {false,RAINDROP6,0,NULL,0,NULL};
statetype s_rain6 = {false,RAINDROP5,3,NULL,0,&s_rain7};
statetype s_rain5 = {false,RAINDROP4,0,NULL,0,&s_rain6};
statetype s_rain4 = {false,RAINDROP3,3,NULL,0,&s_rain5};
statetype s_rain3 = {false,RAINDROP2,0,NULL,0,&s_rain4};
statetype s_rain2 = {false,RAINDROP1,0,T_RainFall,0,&s_rain2};
statetype s_rain1 = {false,NOTHING,50,T_RainSpawn,0,&s_rain1};

statetype s_rainmaster = {false,NOTHING,0,T_RainMaster,0,&s_rainmaster};
*/

statetype s_pathdisk = {false,PLATFORM1,0,T_Path,0,&s_pathdisk};
statetype s_elevdisk = {false,PLATFORM1,0,T_ElevDisk,0,&s_elevdisk};

statetype s_diskmaster = {false,NOTHING,0,T_ElevDisk,0,&s_diskmaster};



statetype s_blooddrip4 = {false,WALLGIB4,3,T_BloodFall,0,&s_blooddrip1};
statetype s_blooddrip3 = {false,WALLGIB3,3,T_BloodFall,0,&s_blooddrip4};
statetype s_blooddrip2 = {false,WALLGIB2,3,T_BloodFall,0,&s_blooddrip3};
statetype s_blooddrip1 = {false,WALLGIB1,3,T_BloodFall,0,&s_blooddrip2};



//==================== Push column =====================================//


statetype s_pushcolumn1 = {false,SPR_PUSHCOLUMN1,6,T_MoveColumn,0,&s_pushcolumn1};
statetype s_pushcolumn2 = {false,SPR_PUSHCOLUMN1,6,T_MoveColumn,0,&s_pushcolumn2};
statetype s_pushcolumn3 = {false,SPR_PUSHCOLUMN1,6,T_MoveColumn,0,&s_pushcolumn3};


//=================== Wall Fire =======================================/

statetype s_wallfireball = {false,NOTHING,50,A_Wallfire,0,&s_wallfireball};

statetype s_crossfire2 = {true,SPR_CROSSFIRE31,6,T_Projectile,0,&s_crossfire1};
statetype s_crossfire1 = {true,SPR_CROSSFIRE11,6,T_Projectile,0,&s_crossfire2};

statetype s_crossdone5 = {false,SPR_CREXP5,6,NULL,0,NULL};
statetype s_crossdone4 = {false,SPR_CREXP4,6,NULL,0,&s_crossdone5};
statetype s_crossdone3 = {false,SPR_CREXP3,6,NULL,0,&s_crossdone4};
statetype s_crossdone2 = {false,SPR_CREXP2,6,NULL,0,&s_crossdone3};
statetype s_crossdone1 = {false,SPR_CREXP1,6,NULL,0,&s_crossdone2};



//=============== gib/related states ===========================================//


statetype s_bossdeath = {false,NOTHING,140,T_BossDied,0,NULL};

statetype s_megaremove     = {false,NOTHING,0,NULL,0,NULL};
statetype s_megaexplosions = {false,NOTHING,0,T_BossExplosions,0,&s_megaexplosions};
statetype s_superparticles = {false,NOTHING,0,T_ParticleGenerate,0,&s_superparticles};


statetype s_gibsdone8 = {false,PARTICLE12,2,NULL,0,NULL};
statetype s_gibsdone7 = {false,PARTICLE11,2,NULL,0,&s_gibsdone8};
//MED
statetype s_gibsdone6 = {false,PARTICLE10,2,NULL,0,&s_gibsdone7};
statetype s_gibsdone5 = {false,PARTICLE09,2,NULL,0,&s_gibsdone6};
statetype s_gibsdone4 = {false,PARTICLE08,2,NULL,0,&s_gibsdone5};
statetype s_gibsdone3 = {false,PARTICLE07,2,NULL,0,&s_gibsdone4};
statetype s_gibsdone2 = {false,PARTICLE06,2,NULL,0,&s_gibsdone3};
statetype s_gibsdone1 = {false,PARTICLE05,2,NULL,0,&s_gibsdone2};

statetype s_gibs4 = {false,PARTICLE04,2,T_Particle,0,&s_gibs1};
statetype s_gibs3 = {false,PARTICLE03,2,T_Particle,0,&s_gibs4};
statetype s_gibs2 = {false,PARTICLE02,2,T_Particle,0,&s_gibs3};
statetype s_gibs1 = {false,PARTICLE01,2,T_Particle,0,&s_gibs2};



//statetype s_head = {false,DEADHEAD,4200,NULL,0,&s_gibsdone2};

statetype s_eye3 = {false,NOTHING,7,T_SlideDownScreen,SF_EYE3,&s_eye1};
statetype s_eye2 = {false,NOTHING,7,T_SlideDownScreen,SF_EYE2,&s_eye3};
statetype s_eye1 = {false,NOTHING,7,T_SlideDownScreen,SF_EYE1,&s_eye2};

statetype s_littlesoul = {false,LITTLESOUL,0,MissileMovement,0,&s_littlesoul};
statetype s_bigsoul = {false,BIGSOUL,0,MissileMovement,0,&s_bigsoul};


statetype s_vaporized8 = {false,VAPORIZED8,0,T_Convert,0,NULL};
statetype s_vaporized7 = {false,VAPORIZED7,3,NULL,0,&s_vaporized8};
statetype s_vaporized6 = {false,VAPORIZED6,3,NULL,0,&s_vaporized7};
statetype s_vaporized5 = {false,VAPORIZED5,3,NULL,0,&s_vaporized6};
statetype s_vaporized4 = {false,VAPORIZED4,3,NULL,0,&s_vaporized5};
statetype s_vaporized3 = {false,VAPORIZED3,3,NULL,0,&s_vaporized4};
statetype s_vaporized2 = {false,VAPORIZED2,3,NULL,0,&s_vaporized3};
statetype s_vaporized1 = {false,VAPORIZED1,3,NULL,0,&s_vaporized2};

statetype s_respawn8 = {false,VAPORIZED1,0,SetShapeoffset,0,&s_player};
statetype s_respawn7 = {false,VAPORIZED2,3,NULL,0,&s_respawn8};
statetype s_respawn6 = {false,VAPORIZED3,3,NULL,0,&s_respawn7};
statetype s_respawn5 = {false,VAPORIZED4,3,NULL,0,&s_respawn6};
statetype s_respawn4 = {false,VAPORIZED5,3,NULL,0,&s_respawn5};
statetype s_respawn3 = {false,VAPORIZED6,3,NULL,0,&s_respawn4};
statetype s_respawn2 = {false,VAPORIZED7,3,NULL,0,&s_respawn3};
statetype s_respawn1 = {false,VAPORIZED8,3,NULL,0,&s_respawn2};



#if (SHAREWARE == 0)


//========================= NON-SHAREWARE STATES ============================


statetype s_scottwander7 = {false,SCOTHEAD7,4,T_CollectorWander,0,&s_scottwander1};
statetype s_scottwander6 = {false,SCOTHEAD6,4,T_CollectorWander,0,&s_scottwander7};
statetype s_scottwander5 = {false,SCOTHEAD5,4,T_CollectorWander,0,&s_scottwander6};
statetype s_scottwander4 = {false,SCOTHEAD4,4,T_CollectorWander,0,&s_scottwander5};
statetype s_scottwander3 = {false,SCOTHEAD3,4,T_CollectorWander,0,&s_scottwander4};
statetype s_scottwander2 = {false,SCOTHEAD2,4,T_CollectorWander,0,&s_scottwander3};
statetype s_scottwander1 = {false,SCOTHEAD1,4,T_CollectorWander,0,&s_scottwander2};

statetype s_scottwanderdoor7 = {false,SCOTHEAD7,4,T_CollectorWander,0,&s_scottwanderdoor1};
statetype s_scottwanderdoor6 = {false,SCOTHEAD6,4,T_CollectorWander,0,&s_scottwanderdoor7};
statetype s_scottwanderdoor5 = {false,SCOTHEAD5,4,T_CollectorWander,0,&s_scottwanderdoor6};
statetype s_scottwanderdoor4 = {false,SCOTHEAD4,4,T_CollectorWander,0,&s_scottwanderdoor5};
statetype s_scottwanderdoor3 = {false,SCOTHEAD3,4,T_CollectorWander,0,&s_scottwanderdoor4};
statetype s_scottwanderdoor2 = {false,SCOTHEAD2,4,T_CollectorWander,0,&s_scottwanderdoor3};
statetype s_scottwanderdoor1 = {false,SCOTHEAD1,4,T_CollectorWander,0,&s_scottwanderdoor2};



/*
===========================================================================

							 OVERPATROLS (op)
																			,
===========================================================================
*/

statetype s_opstand	   = {true,SPR_OP_S1,0,T_Stand,0,&s_opstand};

statetype s_oppath4 	   = {true,SPR_OP_W41,10,T_Path,0,&s_oppath1};
statetype s_oppath3 	   = {true,SPR_OP_W31,10,T_Path,SF_CLOSE,&s_oppath4};
statetype s_oppath2	   = {true,SPR_OP_W21,10,T_Path,0,&s_oppath3};
statetype s_oppath1 	   = {true,SPR_OP_W11,10,T_Path,SF_CLOSE,&s_oppath2};


statetype s_opcollide  = {false,SPR_OP_PAIN1,0,T_Collide,0,&s_opcollide};
statetype s_opcollide2 = {false,SPR_OP_PAIN2,0,T_Collide,0,&s_opcollide2};


//statetype s_opuse2 	   = {true,SPR_OP_USE21,10,T_Use,0,&s_oppath1};
//statetype s_opuse1 	   = {true,SPR_OP_USE11,10,T_Use,0,&s_opuse2};


statetype s_opshoot4 	= {false,SPR_OP_SHOOT4,10,ActorMovement,0,&s_opchase1};
statetype s_opshoot3 	= {false,SPR_OP_SHOOT3,10,ActorMovement,0,&s_opshoot4};
statetype s_opshoot2 	= {false,SPR_OP_SHOOT2,20,A_Shoot,0,&s_opshoot3};
statetype s_opshoot1 	= {false,SPR_OP_SHOOT1,6,ActorMovement,0,&s_opshoot2};



statetype s_opbolo5     = {false,SPR_OP_BOLOSHOOT5,6,ActorMovement,0,&s_opchase1};
statetype s_opbolo4     = {false,SPR_OP_BOLOSHOOT4,6,ActorMovement,0,&s_opbolo5};
statetype s_opbolo3     = {false,SPR_OP_BOLOSHOOT3,20,A_MissileWeapon,0,&s_opbolo4};
statetype s_opbolo2     = {false,SPR_OP_BOLOSHOOT2,6,ActorMovement,0,&s_opbolo3};
statetype s_opbolo1     = {false,SPR_OP_BOLOSHOOT1,6,ActorMovement,0,&s_opbolo2};


statetype s_bolocast4   = {false,SPR_BOLO4,6,T_Projectile,0,&s_bolocast4};
statetype s_bolocast3   = {false,SPR_BOLO3,6,T_Projectile,0,&s_bolocast4};
statetype s_bolocast2   = {false,SPR_BOLO2,6,T_Projectile,0,&s_bolocast3};
statetype s_bolocast1   = {false,SPR_BOLO1,6,T_Projectile,0,&s_bolocast2};


statetype s_opchase4	   = {true,SPR_OP_W41,6,T_Chase,0,&s_opchase1};
statetype s_opchase3	   = {true,SPR_OP_W31,6,T_Chase,SF_CLOSE,&s_opchase4};
statetype s_opchase2	   = {true,SPR_OP_W21,6,T_Chase,0,&s_opchase3};
statetype s_opchase1	   = {true,SPR_OP_W11,6,T_Chase,SF_CLOSE,&s_opchase2};


statetype s_opdead		= {false,SPR_OP_ALTDEAD,0,T_Collide,0,&s_opdead};
statetype s_opdie5		= {false,SPR_OP_ALTDIE5,5,T_Collide,0,&s_opdead};
statetype s_opdie4		= {false,SPR_OP_ALTDIE4,5,T_Collide,0,&s_opdie5};
statetype s_opdie3		= {false,SPR_OP_ALTDIE3,5,T_Collide,0,&s_opdie4};
statetype s_opdie2		= {false,SPR_OP_ALTDIE2,5,T_Collide,0,&s_opdie3};
statetype s_opdie1		= {false,SPR_OP_ALTDIE1,5,T_Collide,0,&s_opdie2};


statetype s_opcrushed2		= {false,SPR_OP_DIE3,2,NULL,0,&s_opdead};
statetype s_opcrushed1		= {false,SPR_OP_DIE1,2,NULL,0,&s_opcrushed2};



/*
============================================================================

								DEATH MONKS

============================================================================
*/


statetype s_dmonkstand	= {true,SPR_MONK_S1,0,T_Stand,0,&s_dmonkstand};

statetype s_dmonkpath4	= {true,SPR_MONK_W41,10,T_Path,0,&s_dmonkpath1};
statetype s_dmonkpath3  = {true,SPR_MONK_W31,10,T_Path,SF_CLOSE,&s_dmonkpath4};
statetype s_dmonkpath2	= {true,SPR_MONK_W21,10,T_Path,0,&s_dmonkpath3};
statetype s_dmonkpath1  = {true,SPR_MONK_W11,10,T_Path,SF_CLOSE,&s_dmonkpath2};


statetype s_dmonkcollide  = {false,SPR_MONK_PAIN1,0,T_Collide,0,&s_dmonkcollide};
statetype s_dmonkcollide2 = {false,SPR_MONK_PAIN2,0,T_Collide,0,&s_dmonkcollide2};


statetype s_dmonkshoot6 = {false,SPR_MONK_DRAIN6,20,ActorMovement,0,&s_dmonkchase1};
statetype s_dmonkshoot5 = {false,SPR_MONK_DRAIN5,20,ActorMovement,0,&s_dmonkshoot6};
statetype s_dmonkshoot4 = {false,SPR_MONK_DRAIN4,20,ActorMovement,0,&s_dmonkshoot3};
statetype s_dmonkshoot3 = {false,SPR_MONK_DRAIN3,20,A_Drain,0,&s_dmonkshoot4};
statetype s_dmonkshoot2 = {false,SPR_MONK_DRAIN2,20,ActorMovement,0,&s_dmonkshoot3};
statetype s_dmonkshoot1 = {false,SPR_MONK_DRAIN1,20,A_Drain,0,&s_dmonkshoot2};

statetype s_dmonkchase4	   = {true,SPR_MONK_W41,6,T_Chase,0,&s_dmonkchase1};
statetype s_dmonkchase3 	= {true,SPR_MONK_W31,6,T_Chase,SF_CLOSE,&s_dmonkchase4};
statetype s_dmonkchase2	   = {true,SPR_MONK_W21,6,T_Chase,0,&s_dmonkchase3};
statetype s_dmonkchase1 	= {true,SPR_MONK_W11,6,T_Chase,SF_CLOSE,&s_dmonkchase2};

statetype s_dmonkdead		= {false,SPR_MONK_DEAD,0,T_Collide,0,&s_dmonkdead};
statetype s_dmonkdie4		= {false,SPR_MONK_DIE4,5,T_Collide,0,&s_dmonkdead};
statetype s_dmonkdie3		= {false,SPR_MONK_DIE3,5,T_Collide,0,&s_dmonkdie4};
statetype s_dmonkdie2		= {false,SPR_MONK_DIE2,5,T_Collide,0,&s_dmonkdie3};
statetype s_dmonkdie1		= {false,SPR_MONK_DIE1,5,T_Collide,0,&s_dmonkdie2};


statetype s_dmonkcrushed2		= {false,SPR_MONK_DIE3,2,NULL,0,&s_dmonkdead};
statetype s_dmonkcrushed1		= {false,SPR_MONK_DIE1,2,NULL,0,&s_dmonkcrushed2};

/*
============================================================================

								DEATH FIRE MONKS

============================================================================
*/

statetype s_firemonkstand	= {true,SPR_FIREMONK_S1,0,T_Stand,0,&s_firemonkstand};

statetype s_firemonkpath4	= {true,SPR_FIREMONK_W41,10,T_Path,0,&s_firemonkpath1};
statetype s_firemonkpath3  = {true,SPR_FIREMONK_W31,10,T_Path,SF_CLOSE,&s_firemonkpath4};
statetype s_firemonkpath2	= {true,SPR_FIREMONK_W21,10,T_Path,0,&s_firemonkpath3};
statetype s_firemonkpath1  = {true,SPR_FIREMONK_W11,10,T_Path,SF_CLOSE,&s_firemonkpath2};


statetype s_firemonkcollide  = {false,SPR_FIREMONK_PAIN1,0,T_Collide,0,&s_firemonkcollide};
statetype s_firemonkcollide2 = {false,SPR_FIREMONK_PAIN2,0,T_Collide,0,&s_firemonkcollide2};

statetype s_firemonkcast7 = {false,SPR_FIREMONK_CAST7,6,ActorMovement,0,&s_firemonkchase1};
statetype s_firemonkcast6 = {false,SPR_FIREMONK_CAST6,40,A_MissileWeapon,0,&s_firemonkcast7};
statetype s_firemonkcast5 = {false,SPR_FIREMONK_CAST5,6,ActorMovement,0,&s_firemonkcast6};
statetype s_firemonkcast4 = {false,SPR_FIREMONK_CAST4,6,ActorMovement,0,&s_firemonkcast5};
statetype s_firemonkcast3 = {false,SPR_FIREMONK_CAST3,6,ActorMovement,0,&s_firemonkcast4};
statetype s_firemonkcast2 = {false,SPR_FIREMONK_CAST2,6,ActorMovement,0,&s_firemonkcast3};
statetype s_firemonkcast1 = {false,SPR_FIREMONK_CAST1,6,ActorMovement,0,&s_firemonkcast2};

statetype s_monkfire4 = {false,MONKFIRE4,3,T_Projectile,0,&s_monkfire1};
statetype s_monkfire3 = {false,MONKFIRE3,3,T_Projectile,0,&s_monkfire4};
statetype s_monkfire2 = {false,MONKFIRE2,3,T_Projectile,0,&s_monkfire3};
statetype s_monkfire1 = {false,MONKFIRE1,3,T_Projectile,0,&s_monkfire2};


statetype s_fireballhit3 = {false,SPR_FIREBALL_HIT3,5,NULL,0,NULL};
statetype s_fireballhit2 = {false,SPR_FIREBALL_HIT2,5,NULL,0,&s_fireballhit3};
statetype s_fireballhit1 = {false,SPR_FIREBALL_HIT1,5,NULL,0,&s_fireballhit2};

statetype s_firemonkchase4 	= {true,SPR_FIREMONK_W41,6,T_Chase,0,&s_firemonkchase1};
statetype s_firemonkchase3 	= {true,SPR_FIREMONK_W31,6,T_Chase,SF_CLOSE,&s_firemonkchase4};
statetype s_firemonkchase2	   = {true,SPR_FIREMONK_W21,6,T_Chase,0,&s_firemonkchase3};
statetype s_firemonkchase1 	= {true,SPR_FIREMONK_W11,6,T_Chase,SF_CLOSE,&s_firemonkchase2};


statetype s_firemonkdead7		= {false,SPR_FIREMONK_DEAD7,0,T_Collide,0,&s_firemonkdead7};
statetype s_firemonkdead6	   = {false,SPR_FIREMONK_DEAD6,5,T_Collide,0,&s_firemonkdead7};
statetype s_firemonkdead5	   = {false,SPR_FIREMONK_DEAD5,5,T_Collide,0,&s_firemonkdead6};
statetype s_firemonkdead4	   = {false,SPR_FIREMONK_DEAD4,5,T_Collide,0,&s_firemonkdead5};
statetype s_firemonkdead3	   = {false,SPR_FIREMONK_DEAD3,5,T_Collide,0,&s_firemonkdead4};
statetype s_firemonkdead2	   = {false,SPR_FIREMONK_DEAD2,5,T_Collide,0,&s_firemonkdead3};
statetype s_firemonkdead		= {false,SPR_FIREMONK_DEAD1,5,T_Collide,0,&s_firemonkdead2};
statetype s_firemonkdie4		= {false,SPR_FIREMONK_DIE3,5,T_Collide,0,&s_firemonkdead};
statetype s_firemonkdie3		= {false,SPR_FIREMONK_DIE3,5,T_Collide,0,&s_firemonkdie4};
statetype s_firemonkdie2		= {false,SPR_FIREMONK_DIE2,5,T_Collide,0,&s_firemonkdie3};
statetype s_firemonkdie1		= {false,SPR_FIREMONK_DIE1,5,T_Collide,0,&s_firemonkdie2};


statetype s_firemonkcrushed2		= {false,SPR_FIREMONK_DIE3,2,NULL,0,&s_firemonkdead};
statetype s_firemonkcrushed1		= {false,SPR_FIREMONK_DIE1,2,NULL,0,&s_firemonkcrushed2};

/*===========================================================================

								  INSANE WALL DUDE

============================================================================*/


statetype s_wallstand = {16,BCRAFT01,0,T_Stand,0,&s_wallstand};

statetype s_wallpath = {16,BCRAFT01,20,T_AutoPath,0,&s_wallpath};

statetype s_wallshoot = {16,BCRAFT01,15,A_MissileWeapon,0,&s_wallshoot};

statetype s_wallalign = {16,BCRAFT01,0,T_AutoShootAlign,0,&s_wallalign};
statetype s_wallwait = {16,BCRAFT01,0,NULL,0,&s_wallwait};
statetype s_wallrestore = {16,BCRAFT01,0,T_AutoRealign,0,&s_wallrestore};



statetype s_wallcollide  = {16,BCRAFT01,0,T_Collide,0,&s_wallcollide};


/*
===========================================================================

									  ESAU GUDERIAN

===========================================================================
*/




statetype s_darianstand = {true,SPR_DARIAN_S1,0,T_Stand,0,&s_darianstand};

statetype s_darianchase4	= {true,SPR_DARIAN_W41,8,T_EsauChase,0,&s_darianchase1};
statetype s_darianchase3	= {true,SPR_DARIAN_W31,8,T_EsauChase,SF_CLOSE,&s_darianchase4};
statetype s_darianchase2	= {true,SPR_DARIAN_W21,8,T_EsauChase,0,&s_darianchase3};
statetype s_darianchase1	= {true,SPR_DARIAN_W11,8,T_EsauChase,SF_CLOSE,&s_darianchase2};


statetype s_darianrise8 = {false,SPR_DARIAN_SINK1,3,NULL,0,&s_darianwait};
statetype s_darianrise7 = {false,SPR_DARIAN_SINK2,3,NULL,0,&s_darianrise8};
statetype s_darianrise6 = {false,SPR_DARIAN_SINK3,3,NULL,0,&s_darianrise7};
statetype s_darianrise5 = {false,SPR_DARIAN_SINK4,3,NULL,0,&s_darianrise6};
statetype s_darianrise4 = {false,SPR_DARIAN_SINK5,3,NULL,0,&s_darianrise5};
statetype s_darianrise3 = {false,SPR_DARIAN_SINK6,3,NULL,0,&s_darianrise4};
statetype s_darianrise2 = {false,SPR_DARIAN_SINK7,3,NULL,0,&s_darianrise3};
statetype s_darianrise1 = {false,SPR_DARIAN_SINK8,3,NULL,0,&s_darianrise2};


statetype s_dariansink9 = {false,NOTHING,110,T_EsauRise,0,&s_darianrise1};
statetype s_dariansink8 = {false,SPR_DARIAN_SINK8,3,NULL,0,&s_dariansink9};
statetype s_dariansink7 = {false,SPR_DARIAN_SINK7,3,NULL,0,&s_dariansink8};
statetype s_dariansink6 = {false,SPR_DARIAN_SINK6,3,NULL,0,&s_dariansink7};
statetype s_dariansink5 = {false,SPR_DARIAN_SINK5,3,NULL,0,&s_dariansink6};
statetype s_dariansink4 = {false,SPR_DARIAN_SINK4,3,NULL,0,&s_dariansink5};
statetype s_dariansink3 = {false,SPR_DARIAN_SINK3,3,NULL,0,&s_dariansink4};
statetype s_dariansink2 = {false,SPR_DARIAN_SINK2,3,NULL,0,&s_dariansink3};
statetype s_dariansink1 = {false,SPR_DARIAN_SINK1,3,NULL,0,&s_dariansink2};


statetype s_dariancollide  = {false,SPR_DARIAN_PAIN1,0,T_Collide,0,&s_dariancollide};
statetype s_dariancollide2 = {false,SPR_DARIAN_PAIN2,0,T_Collide,0,&s_dariancollide2};


statetype s_darianshoot4  = {false,SPR_DARIAN_SHOOT4,10,NULL,0,&s_darianchase1};
statetype s_darianshoot3  = {false,SPR_DARIAN_SHOOT3,10,NULL,0,&s_darianshoot4};
statetype s_darianshoot2  = {false,SPR_DARIAN_SHOOT2,20,A_MissileWeapon,0,&s_darianshoot3};
statetype s_darianshoot1  = {false,SPR_DARIAN_SHOOT1,10,NULL,0,&s_darianshoot2};

statetype s_dariandefend3  = {false,SPR_DARIAN_SHOOT3,10,NULL,0,&s_darianwait};
statetype s_dariandefend2  = {false,SPR_DARIAN_SHOOT2,20,A_MissileWeapon,0,&s_dariandefend3};
statetype s_dariandefend1  = {false,SPR_DARIAN_SHOOT1,10,NULL,0,&s_dariandefend2};


statetype s_darianuse4  = {true,SPR_DARIAN_USE11,10,NULL,0,&s_darianspears};
statetype s_darianuse3  = {true,SPR_DARIAN_USE21,10,T_Use,0,&s_darianuse4};
statetype s_darianuse2  = {true,SPR_DARIAN_USE11,30,NULL,0,&s_darianuse3};
statetype s_darianuse1  = {true,SPR_DARIAN_S1,40,NULL,0,&s_darianuse2};

statetype s_darianwait = {false,SPR_DARIAN_SHOOT1,0,T_EsauWait,0,&s_darianwait};

statetype s_darianspears  = {true,SPR_DARIAN_S1,280,NULL,0,&s_darianchase1};


statetype s_dspear16 = {false,SPEARDOWN7,2,T_Spears,SF_DOWN,NULL};
statetype s_dspear15 = {false,SPEARDOWN6,2,T_Spears,SF_CRUSH,&s_dspear16};
statetype s_dspear14 = {false,SPEARDOWN5,2,T_Spears,SF_CRUSH,&s_dspear15};
statetype s_dspear13 = {false,SPEARDOWN4,2,T_Spears,SF_CRUSH,&s_dspear14};
statetype s_dspear12 = {false,SPEARDOWN3,2,T_Spears,SF_CRUSH,&s_dspear13};
statetype s_dspear11 = {false,SPEARDOWN2,2,T_Spears,SF_CRUSH,&s_dspear12};
statetype s_dspear10 = {false,SPEARDOWN1,2,T_Spears,SF_CRUSH,&s_dspear11};
statetype s_dspear9 = {false,SPEARDOWN16,2,T_Spears,SF_CRUSH,&s_dspear10};
statetype s_dspear8 = {false,SPEARDOWN15,2,T_Spears,SF_CRUSH,&s_dspear9};
statetype s_dspear7 = {false,SPEARDOWN14,2,T_Spears,SF_CRUSH,&s_dspear8};
statetype s_dspear6 = {false,SPEARDOWN13,2,T_Spears,SF_CRUSH,&s_dspear7};
statetype s_dspear5 = {false,SPEARDOWN12,2,T_Spears,SF_CRUSH,&s_dspear6};
statetype s_dspear4 = {false,SPEARDOWN11,2,T_Spears,SF_CRUSH,&s_dspear5};
statetype s_dspear3 = {false,SPEARDOWN10,2,T_Spears,SF_CRUSH,&s_dspear4};
statetype s_dspear2 = {false,SPEARDOWN9,2,T_Spears,SF_DOWN,&s_dspear3};
statetype s_dspear1 = {false,SPEARDOWN8,2,T_Spears,SF_DOWN,&s_dspear2};


statetype s_dariandead2 	= {false,SPR_DARIAN_DEAD,0,T_BossDied,0,&s_dariandead2};
statetype s_dariandead1 	= {false,SPR_DARIAN_DEAD,140,NULL,0,&s_dariandead2};
statetype s_dariandead 	= {false,SPR_DARIAN_DEAD,0,NULL,0,&s_dariandead1};

statetype s_dariandie10 = {false,SPR_DARIAN_DIE10,5,NULL,0,&s_dariandead};
statetype s_dariandie9 	= {false,SPR_DARIAN_DIE9,5,NULL,0,&s_dariandie10};
statetype s_dariandie8 	= {false,SPR_DARIAN_DIE8,5,NULL,0,&s_dariandie9};
statetype s_dariandie7 	= {false,SPR_DARIAN_DIE7,5,NULL,0,&s_dariandie8};
statetype s_dariandie6 	= {false,SPR_DARIAN_DIE6,5,T_Guts,0,&s_dariandie7};
statetype s_dariandie5 	= {false,SPR_DARIAN_DIE5,5,NULL,0,&s_dariandie6};
statetype s_dariandie4 	= {false,SPR_DARIAN_DIE4,5,NULL,0,&s_dariandie5};
statetype s_dariandie3 	= {false,SPR_DARIAN_DIE3,5,NULL,0,&s_dariandie4};
statetype s_dariandie2 	= {false,SPR_DARIAN_DIE2,5,NULL,0,&s_dariandie3};
statetype s_dariandie1 	= {false,SPR_DARIAN_DIE1,70,T_Collide,0,&s_dariandie2};



/*
===========================================================================

									HEINRICH KRIST

===========================================================================
*/


statetype s_heinrichstand	= {true,SPR_KRIST_S1,0,T_Stand,0,&s_heinrichstand};

statetype s_heinrichchase 	= {true,SPR_KRIST_FOR1,10,T_HeinrichChase,0,&s_heinrichchase};


statetype s_kristleft = {true,SPR_KRIST_LEFT1,10,T_KristLeft,0,&s_kristleft};

statetype s_kristright = {true,SPR_KRIST_RIGHT1,10,T_KristRight,0,&s_kristright};

statetype s_heinrichshoot11  = {false,SPR_KRIST_SHOOT11,8,NULL,0,&s_heinrichchase};
statetype s_heinrichshoot10  = {false,SPR_KRIST_SHOOT10,8,NULL,0,&s_heinrichshoot11};
statetype s_heinrichshoot9  = {false,SPR_KRIST_SHOOT9,8,A_HeinrichShoot,0,&s_heinrichshoot10};
statetype s_heinrichshoot8  = {false,SPR_KRIST_SHOOT8,8,NULL,0,&s_heinrichshoot9};
statetype s_heinrichshoot7  = {false,SPR_KRIST_SHOOT7,8,T_KristCheckFire,0,&s_heinrichshoot8};
statetype s_heinrichshoot6  = {false,SPR_KRIST_SHOOT6,8,NULL,0,&s_heinrichshoot7};
statetype s_heinrichshoot5  = {false,SPR_KRIST_SHOOT5,8,NULL,0,&s_heinrichshoot6};
statetype s_heinrichshoot4  = {false,SPR_KRIST_SHOOT4,8,A_HeinrichShoot,0,&s_heinrichshoot5};
statetype s_heinrichshoot3  = {false,SPR_KRIST_SHOOT3,8,NULL,0,&s_heinrichshoot4};
statetype s_heinrichshoot2  = {false,SPR_KRIST_SHOOT2,8,NULL,0,&s_heinrichshoot3};
statetype s_heinrichshoot1  = {false,SPR_KRIST_SHOOT1,8,T_KristCheckFire,0,&s_heinrichshoot2};

statetype s_missile1       = {16,SPR_BJMISS11,6,T_Projectile,0,&s_missile1};


statetype s_missilehit3    = {false,SPR_MISSILEHIT3,6,NULL,0,NULL};
statetype s_missilehit2    = {false,SPR_MISSILEHIT2,6,NULL,0,&s_missilehit3};
statetype s_missilehit1    = {false,SPR_MISSILEHIT1,6,NULL,0,&s_missilehit2};

statetype s_mine4          = {false,SPR_MINE4,3,T_Projectile,0,&s_mine1};
statetype s_mine3          = {false,SPR_MINE3,3,T_Projectile,0,&s_mine4};
statetype s_mine2          = {false,SPR_MINE2,3,T_Projectile,0,&s_mine3};
statetype s_mine1          = {false,SPR_MINE1,3,T_Projectile,0,&s_mine2};

statetype s_heinrichdefend = {true,SPR_KRIST_MINERIGHT1,35,T_Heinrich_Defend,0,&s_heinrichdefend};


statetype s_heinrichooc    = {true,SPR_KRIST_DEAD1,0,T_Heinrich_Out_of_Control,0,&s_heinrichooc};

statetype s_heinrichdead 	= {false,SPR_KRIST_DEAD1,35,T_Collide,0,&s_heinrichooc};

statetype s_heinrichdie2 	= {false,SPR_KRIST_DIE2,35,T_Collide,0,&s_heinrichdead};
statetype s_heinrichdie1 	= {false,SPR_KRIST_DIE1,35,T_Collide,0,&s_heinrichdie2};

statetype s_heindead2 = {false,-1,0,T_BossDied,0,NULL};
statetype s_heindead1 = {false, -1, 140,NULL,0,&s_heindead2};
statetype s_heinexp13 = {false,SPR_EXPLOSION13,2,NULL,0,&s_heindead1};
statetype s_heinexp12 = {false,SPR_EXPLOSION12,2,NULL,0,&s_heinexp13};
statetype s_heinexp11 = {false,SPR_EXPLOSION11,2,NULL,0,&s_heinexp12};
statetype s_heinexp10 = {false,SPR_EXPLOSION10,2,NULL,0,&s_heinexp11};
statetype s_heinexp9 = {false,SPR_EXPLOSION9,2,NULL,0,&s_heinexp10};
statetype s_heinexp8 = {false,SPR_EXPLOSION8,2,NULL,0,&s_heinexp9};
statetype s_heinexp7 = {false,SPR_EXPLOSION7,2,NULL,0,&s_heinexp8};
statetype s_heinexp6 = {false,SPR_EXPLOSION6,2,NULL,0,&s_heinexp7};
statetype s_heinexp5 = {false,SPR_EXPLOSION5,2,NULL,0,&s_heinexp6};
statetype s_heinexp4 = {false,SPR_EXPLOSION4,2,NULL,0,&s_heinexp5};
statetype s_heinexp3 = {false,SPR_EXPLOSION3,2,T_Explosion,0,&s_heinexp4};
statetype s_heinexp2 = {false,SPR_EXPLOSION2,2,NULL,0,&s_heinexp3};
statetype s_heinexp1 = {false,SPR_EXPLOSION1,0,NULL,0,&s_heinexp2};


/*
===========================================================================

								  DARK MONK (TOM)

===========================================================================
*/


statetype s_darkmonkstand	= {true,TOMS1,0,T_Stand,0,&s_darkmonkstand};

statetype s_darkmonkland	= {true,TOMFLY11,6,NULL,0,&s_darkmonkstand};


statetype s_darkmonkchase2	= {true,TOMFLY21,10,T_DarkmonkChase,0,&s_darkmonkchase2};
statetype s_darkmonkchase1	= {true,TOMFLY11,3,T_DarkmonkChase,0,&s_darkmonkchase2};

statetype s_dmlandandfire = {true,TOMFLY11,0,T_DarkmonkLandAndFire,0,&s_dmlandandfire};

statetype s_darkmonkcover3	= {false,TAWAKEN1,1,NULL,0,&s_darkmonkawaken1};
statetype s_darkmonkcover2	= {false,TAWAKEN2,1,NULL,0,&s_darkmonkcover3};
statetype s_darkmonkcover1	= {false,TAWAKEN3,1,NULL,0,&s_darkmonkcover2};


statetype s_darkmonkawaken5	= {false,TAWAKEN5,3,NULL,0,&s_darkmonkchase1};
statetype s_darkmonkawaken4	= {false,TAWAKEN4,3,NULL,0,&s_darkmonkawaken5};
statetype s_darkmonkawaken3	= {false,TAWAKEN3,3,NULL,0,&s_darkmonkawaken4};
statetype s_darkmonkawaken2	= {false,TAWAKEN2,3,NULL,0,&s_darkmonkawaken3};
statetype s_darkmonkawaken1 	= {false,TAWAKEN1,3,NULL,0,&s_darkmonkawaken2};


statetype s_darkmonklightning11	= {false,TOMLG11,3,NULL,0,&s_darkmonkchase1};
statetype s_darkmonklightning10	= {false,TOMLG10,3,NULL,0,&s_darkmonklightning11};
statetype s_darkmonklightning9	= {false,TOMLG9,70,A_DmonkAttack,0,&s_darkmonklightning10};
statetype s_darkmonklightning8	= {false,TOMLG8,3,NULL,0,&s_darkmonklightning9};
statetype s_darkmonklightning7 	= {false,TOMLG7,3,NULL,0,&s_darkmonklightning8};
statetype s_darkmonklightning6	= {false,TOMLG6,3,NULL,0,&s_darkmonklightning7};
statetype s_darkmonklightning5	= {false,TOMLG5,3,NULL,0,&s_darkmonklightning6};
statetype s_darkmonklightning4	= {false,TOMLG4,3,NULL,0,&s_darkmonklightning5};
statetype s_darkmonklightning3	= {false,TOMLG3,3,NULL,0,&s_darkmonklightning4};
statetype s_darkmonklightning2 	= {false,TOMLG2,3,NULL,0,&s_darkmonklightning3};
statetype s_darkmonklightning1 	= {false,TOMLG1,3,NULL,0,&s_darkmonklightning2};

statetype s_darkmonkfspark6	= {false,TOMFS6,3,NULL,0,&s_darkmonkchase1};
statetype s_darkmonkfspark5	= {false,TOMFS5,70,A_DmonkAttack,0,&s_darkmonkfspark6};
statetype s_darkmonkfspark4	= {false,TOMFS4,3,NULL,0,&s_darkmonkfspark5};
statetype s_darkmonkfspark3	= {false,TOMFS3,3,NULL,0,&s_darkmonkfspark4};
statetype s_darkmonkfspark2 	= {false,TOMFS2,3,NULL,0,&s_darkmonkfspark3};
statetype s_darkmonkfspark1 	= {false,TOMFS1,3,NULL,0,&s_darkmonkfspark2};


statetype s_darkmonkbreathe8	= {false,TOMBR8,3,NULL,0,&s_darkmonkchase1};
statetype s_darkmonkbreathe7 	= {false,TOMBR7,3,NULL,0,&s_darkmonkbreathe8};
statetype s_darkmonkbreathe6	= {false,TOMBR6,70,A_DmonkAttack,0,&s_darkmonkbreathe7};
statetype s_darkmonkbreathe5	= {false,TOMBR5,3,NULL,0,&s_darkmonkbreathe6};
statetype s_darkmonkbreathe4	= {false,TOMBR4,3,NULL,0,&s_darkmonkbreathe5};
statetype s_darkmonkbreathe3	= {false,TOMBR3,3,NULL,0,&s_darkmonkbreathe4};
statetype s_darkmonkbreathe2 	= {false,TOMBR2,3,NULL,0,&s_darkmonkbreathe3};
statetype s_darkmonkbreathe1 	= {false,TOMBR1,70,NULL,0,&s_darkmonkbreathe2};

statetype s_darkmonksummon3	= {false,TOMBR1,3,NULL,0,&s_darkmonkchase1};
statetype s_darkmonksummon2 	= {false,TOMBR3,3,NULL,0,&s_darkmonksummon3};
statetype s_darkmonksummon1 	= {false,TOMBR2,3,NULL,0,&s_darkmonksummon2};

statetype s_snakepath = {true,TOMHEAD1,0,T_SnakePath,0,&s_snakepath};
statetype s_snakefindpath = {true,TOMHEAD1,0,T_SnakeFindPath,0,&s_snakefindpath};

statetype s_darkmonkhead	= {true,TOMHEAD1,0,T_DarkSnakeChase,0,&s_darkmonkhead};
statetype s_darkmonksnakelink	= {true,TOMHEAD1,0,T_GenericMove,0,&s_darkmonksnakelink};


statetype s_darkmonkhspawn = {false,NOTHING,78,T_DarkSnakeSpawn,0,NULL};
statetype s_darkmonkfastspawn = {false,NOTHING,35,T_DarkSnakeSpawn,0,NULL};

statetype s_darkmonkheaddead	= {false,THDIE2,0,NULL,0,&s_darkmonkheaddead};
statetype s_darkmonkheaddie1	= {false,THDIE1,1400,T_SnakeFinale,0,&s_snakefireworks1};

statetype s_snakefireworks2 = {false,THDIE2,10,T_SnakeFinale,0,&s_snakefireworks1};
statetype s_snakefireworks1 = {false,THDIE1,10,T_SnakeFinale,0,&s_snakefireworks2};

statetype s_darkmonkhball9	= {false,THBALL9,5,NULL,0,&s_darkmonkchase1};
statetype s_darkmonkhball8	= {false,THBALL8,5,NULL,0,&s_darkmonkhball9};
statetype s_darkmonkhball7 = {false,THBALL7,5,A_DmonkAttack,0,&s_darkmonkhball8};
statetype s_darkmonkhball6	= {false,THBALL6,5,NULL,0,&s_darkmonkhball7};
statetype s_darkmonkhball5	= {false,THBALL5,5,NULL,0,&s_darkmonkhball6};
statetype s_darkmonkhball4	= {false,THBALL4,5,NULL,0,&s_darkmonkhball5};
statetype s_darkmonkhball3	= {false,THBALL3,5,NULL,0,&s_darkmonkhball4};
statetype s_darkmonkhball2 = {false,THBALL2,5,NULL,0,&s_darkmonkhball3};
statetype s_darkmonkhball1 = {false,THBALL1,70,NULL,0,&s_darkmonkhball2};

statetype s_darkmonkabsorb9	= {false,THBALL9,3,NULL,0,&s_darkmonksphere1};
statetype s_darkmonkabsorb8	= {false,THBALL8,3,NULL,0,&s_darkmonkabsorb9};
statetype s_darkmonkabsorb7 = {false,THBALL7,3,NULL,0,&s_darkmonkabsorb8};
statetype s_darkmonkabsorb6	= {false,THBALL6,3,NULL,0,&s_darkmonkabsorb7};
statetype s_darkmonkabsorb5	= {false,THBALL5,3,NULL,0,&s_darkmonkabsorb6};
statetype s_darkmonkabsorb4	= {false,THBALL4,3,NULL,0,&s_darkmonkabsorb5};
statetype s_darkmonkabsorb3	= {false,THBALL3,3,NULL,0,&s_darkmonkabsorb4};
statetype s_darkmonkabsorb2 = {false,THBALL2,3,NULL,0,&s_darkmonkabsorb3};
statetype s_darkmonkabsorb1 = {false,THBALL1,3,NULL,0,&s_darkmonkabsorb2};


statetype s_darkmonksphere10 = {false,TSPHERE10,4,NULL,0,&s_darkmonkchase1};
statetype s_darkmonksphere9 = {false,TSPHERE9,4,NULL,0,&s_darkmonksphere10};
statetype s_darkmonksphere8 = {false,TSPHERE8,4,A_DmonkAttack,0,&s_darkmonksphere9};
statetype s_darkmonksphere7 = {false,TSPHERE7,4,NULL,0,&s_darkmonksphere8};
statetype s_darkmonksphere6 = {false,TSPHERE6,4,NULL,0,&s_darkmonksphere7};
statetype s_darkmonksphere5 = {false,TSPHERE5,4,NULL,0,&s_darkmonksphere6};
statetype s_darkmonksphere4 = {false,TSPHERE4,4,NULL,0,&s_darkmonksphere5};
statetype s_darkmonksphere3 = {false,TSPHERE3,4,NULL,0,&s_darkmonksphere4};
statetype s_darkmonksphere2 = {false,TSPHERE2,4,NULL,0,&s_darkmonksphere3};
statetype s_darkmonksphere1 = {false,TSPHERE1,4,NULL,0,&s_darkmonksphere2};

statetype s_dmgreenthing10 = {false,TSPHERE10,4,NULL,0,&s_darkmonkchase1};
statetype s_dmgreenthing9 = {false,TSPHERE9,4,NULL,0,&s_dmgreenthing10};
statetype s_dmgreenthing8 = {false,TSPHERE8,4,A_DmonkAttack,0,&s_dmgreenthing9};
statetype s_dmgreenthing7 = {false,TSPHERE7,4,NULL,0,&s_dmgreenthing8};
statetype s_dmgreenthing6 = {false,TSPHERE6,4,NULL,0,&s_dmgreenthing7};
statetype s_dmgreenthing5 = {false,TSPHERE5,4,NULL,0,&s_dmgreenthing6};
statetype s_dmgreenthing4 = {false,TSPHERE4,4,NULL,0,&s_dmgreenthing5};
statetype s_dmgreenthing3 = {false,TSPHERE3,4,NULL,0,&s_dmgreenthing4};
statetype s_dmgreenthing2 = {false,TSPHERE2,4,NULL,0,&s_dmgreenthing3};
statetype s_dmgreenthing1 = {false,TSPHERE1,4,NULL,0,&s_dmgreenthing2};


statetype s_energysphere4 = {false,TOMSPHERE4,4,T_Projectile,0,&s_energysphere1};
statetype s_energysphere3 = {false,TOMSPHERE3,4,T_Projectile,0,&s_energysphere4};
statetype s_energysphere2 = {false,TOMSPHERE2,4,T_Projectile,0,&s_energysphere3};
statetype s_energysphere1 = {false,TOMSPHERE1,4,T_Projectile,0,&s_energysphere2};


statetype s_lightning = {true,TOMLIGHTNING1,0,T_Projectile,0,&s_lightning};


statetype s_handball2 = {false,TOMHANDBALL2,3,T_Projectile,0,&s_handball1};
statetype s_handball1 = {false,TOMHANDBALL1,3,T_Projectile,0,&s_handball2};

statetype s_faceball2 = {false,TOMFACEBALL2,3,T_Projectile,0,&s_faceball1};
statetype s_faceball1 = {false,TOMFACEBALL1,3,T_Projectile,0,&s_faceball2};

statetype s_floorspark4 = {false,TOMFLOORSPARK4,3,T_Projectile,0,&s_floorspark1};
statetype s_floorspark3 = {false,TOMFLOORSPARK3,3,T_Projectile,0,&s_floorspark4};
statetype s_floorspark2 = {false,TOMFLOORSPARK2,3,T_Projectile,0,&s_floorspark3};
statetype s_floorspark1 = {false,TOMFLOORSPARK1,3,T_Projectile,0,&s_floorspark2};



statetype s_darkmonkreact = {true,TOMFLY11,0,T_DarkmonkReact,0,&s_darkmonkreact};

statetype s_darkmonkbball9	= {false,TBBALL9,4,NULL,0,&s_darkmonkchase1};
statetype s_darkmonkbball8	= {false,TBBALL8,4,NULL,0,&s_darkmonkbball9};
statetype s_darkmonkbball7 = {false,TBBALL7,4,A_DmonkAttack,0,&s_darkmonkbball8};
statetype s_darkmonkbball6	= {false,TBBALL6,4,NULL,0,&s_darkmonkbball7};
statetype s_darkmonkbball5	= {false,TBBALL5,4,NULL,0,&s_darkmonkbball6};
statetype s_darkmonkbball4	= {false,TBBALL4,4,NULL,0,&s_darkmonkbball5};
statetype s_darkmonkbball3	= {false,TBBALL3,4,NULL,0,&s_darkmonkbball4};
statetype s_darkmonkbball2 = {false,TBBALL2,4,NULL,0,&s_darkmonkbball3};
statetype s_darkmonkbball1 = {false,TBBALL1,70,NULL,0,&s_darkmonkbball2};


statetype s_darkmonkcharge10 = {false,TSCAREB3,140,T_DarkmonkCharge,0,&s_darkmonkchase1};
statetype s_darkmonkcharge9	= {false,TAWAKEN5,3,NULL,0,&s_darkmonkcharge10};
statetype s_darkmonkcharge8	= {false,TAWAKEN4,3,NULL,0,&s_darkmonkcharge9};
statetype s_darkmonkcharge7	= {false,TAWAKEN3,3,NULL,0,&s_darkmonkcharge8};
statetype s_darkmonkcharge6 	= {false,TAWAKEN2,3,NULL,0,&s_darkmonkcharge7};
statetype s_darkmonkcharge5	= {false,TAWAKEN1,3,NULL,0,&s_darkmonkcharge6};
statetype s_darkmonkcharge4	= {false,TAWAKEN2,3,NULL,0,&s_darkmonkcharge5};
statetype s_darkmonkcharge3	= {false,TAWAKEN3,3,NULL,0,&s_darkmonkcharge4};
statetype s_darkmonkcharge2	= {false,TAWAKEN4,3,NULL,0,&s_darkmonkcharge3};
statetype s_darkmonkcharge1 	= {false,TAWAKEN5,3,NULL,0,&s_darkmonkcharge2};


statetype s_darkmonkscare5	= {false,TSCAREB5,3,NULL,0,&s_darkmonkcharge1};
statetype s_darkmonkscare4	= {false,TSCAREB4,3,NULL,0,&s_darkmonkscare5};
statetype s_darkmonkscare3	= {false,TSCAREB3,3,NULL,0,&s_darkmonkscare4};
statetype s_darkmonkscare2 = {false,TSCAREB2,3,NULL,0,&s_darkmonkscare3};
statetype s_darkmonkscare1 = {false,TSCAREB1,3,NULL,0,&s_darkmonkscare2};

statetype s_darkmonkdead = {false,TOMDIE8,0,T_Collide,0,&s_darkmonkdead};
statetype s_darkmonkdie7 = {false,TOMDIE7,3,T_Collide,0,&s_darkmonkdead};
statetype s_darkmonkdie6 = {false,TOMDIE6,3,T_Collide,0,&s_darkmonkdie7};
statetype s_darkmonkdie5 = {false,TOMDIE5,3,T_Collide,0,&s_darkmonkdie6};
statetype s_darkmonkdie4 = {false,TOMDIE4,3,T_Collide,0,&s_darkmonkdie5};
statetype s_darkmonkdie3 = {false,TOMDIE3,3,T_Collide,0,&s_darkmonkdie4};
statetype s_darkmonkdie2 = {false,TOMDIE2,3,T_Collide,0,&s_darkmonkdie3};
statetype s_darkmonkdie1 = {false,TOMDIE1,3,T_Collide,0,&s_darkmonkdie2};



statetype s_darkmonkredhead = {true,TOMRH1,50,T_DarkSnakeChase,0,&s_darkmonkhead};
statetype s_darkmonkredlink = {true,TOMRH1,50,T_GenericMove,0,&s_darkmonksnakelink};

statetype s_redheadhit = {false,THDIE1,35,T_DarkSnakeChase,0,&s_darkmonkhead};
statetype s_redlinkhit = {false,THDIE1,35,T_GenericMove,0,&s_darkmonksnakelink};


statetype s_spithit4 = {false,SPITHIT4,3,NULL,0,NULL};
statetype s_spithit3 = {false,SPITHIT3,3,NULL,0,&s_spithit4};
statetype s_spithit2 = {false,SPITHIT2,3,NULL,0,&s_spithit3};
statetype s_spithit1 = {false,SPITHIT1,3,NULL,0,&s_spithit2};

statetype s_spit4 = {false,TOMSPIT4,3,T_Projectile,0,&s_spit1};
statetype s_spit3 = {false,TOMSPIT3,3,T_Projectile,0,&s_spit4};
statetype s_spit2 = {false,TOMSPIT2,3,T_Projectile,0,&s_spit3};
statetype s_spit1 = {false,TOMSPIT1,3,T_Projectile,0,&s_spit2};

statetype s_snakefire2 = {true,TOMHEAD1,1,T_DarkSnakeChase,SF_DOWN|SF_UP,&s_darkmonkhead};
statetype s_snakefire1 = {false,TPREPARE,30,T_DarkSnakeChase,SF_DOWN,&s_snakefire2};

statetype s_dexplosion22 = {false,-1,0,T_BossDied,0,NULL};
statetype s_dexplosion21 = {false, -1, 240,NULL,0,&s_dexplosion22};
statetype s_dexplosion20 = {false,SPR_EXPLOSION20,2,NULL,0,&s_dexplosion21};
statetype s_dexplosion19 = {false,SPR_EXPLOSION19,2,NULL,0,&s_dexplosion20};
statetype s_dexplosion18 = {false,SPR_EXPLOSION18,2,NULL,0,&s_dexplosion19};
statetype s_dexplosion17 = {false,SPR_EXPLOSION17,2,NULL,0,&s_dexplosion18};
statetype s_dexplosion16 = {false,SPR_EXPLOSION16,2,NULL,0,&s_dexplosion17};
statetype s_dexplosion15 = {false,SPR_EXPLOSION15,2,NULL,0,&s_dexplosion16};
statetype s_dexplosion14 = {false,SPR_EXPLOSION14,2,NULL,0,&s_dexplosion15};
statetype s_dexplosion13 = {false,SPR_EXPLOSION13,2,NULL,0,&s_dexplosion14};
statetype s_dexplosion12 = {false,SPR_EXPLOSION12,2,NULL,0,&s_dexplosion13};
statetype s_dexplosion11 = {false,SPR_EXPLOSION11,2,NULL,0,&s_dexplosion12};
statetype s_dexplosion10 = {false,SPR_EXPLOSION10,2,NULL,0,&s_dexplosion11};
statetype s_dexplosion9 = {false,SPR_EXPLOSION9,2,NULL,0,&s_dexplosion10};
statetype s_dexplosion8 = {false,SPR_EXPLOSION8,2,NULL,0,&s_dexplosion9};
statetype s_dexplosion7 = {false,SPR_EXPLOSION7,2,NULL,0,&s_dexplosion8};
statetype s_dexplosion6 = {false,SPR_EXPLOSION6,2,NULL,0,&s_dexplosion7};
statetype s_dexplosion5 = {false,SPR_EXPLOSION5,2,NULL,0,&s_dexplosion6};
statetype s_dexplosion4 = {false,SPR_EXPLOSION4,2,NULL,0,&s_dexplosion5};
statetype s_dexplosion3 = {false,SPR_EXPLOSION3,2,T_Explosion,0,&s_dexplosion4};
statetype s_dexplosion2 = {false,SPR_EXPLOSION2,2,NULL,0,&s_dexplosion3};
statetype s_dexplosion1 = {false,SPR_EXPLOSION1,2,NULL,0,&s_dexplosion2};


/*
===========================================================================

									OROBOT

===========================================================================
*/

statetype s_NMEdeathbuildup = {16,NMEBODY1_01,210,T_NME_Explode,0,NULL};
statetype s_NMEheadexplosion = {16,NMEHEAD1_01,140,T_Special,0,&s_grexplosion1};

statetype s_NMEstand = {16,NMEBODY1_01,0,T_Stand,0,&s_NMEstand};
statetype s_NMEhead1 = {16,NMEHEAD1_01,0,NULL,0,&s_NMEhead1};
statetype s_NMEhead2 = {16,NMEHEAD2_01,0,NULL,0,&s_NMEhead2};
statetype s_NMEchase = {16,NMEBODY1_01,20,T_OrobotChase,0,&s_NMEchase};
statetype s_NMEwheels1 = {16,NMEWHEEL1_01,0,NULL,0,&s_NMEwheels1};
statetype s_NMEwheels2 = {16,NMEWHEEL2_01,0,NULL,0,&s_NMEwheels2};
statetype s_NMEwheels3 = {16,NMEWHEEL3_01,0,NULL,0,&s_NMEwheels3};
statetype s_NMEwheels4 = {16,NMEWHEEL4_01,0,NULL,0,&s_NMEwheels4};
statetype s_NMEwheels5 = {16,NMEWHEEL5_01,0,NULL,0,&s_NMEwheels5};

statetype s_shootinghead = {16,NMEHEAD1_01,140,T_NME_HeadShoot,0,&s_shootinghead};

statetype s_NMEspinattack = {16,NMEBODY1_01,70,T_NME_SpinAttack,0,&s_NMEchase};
statetype s_NMEwheelspin = {16,NMEWHEEL1_01,70,NULL,0,&s_NMEwheels2};

statetype s_NMEminiball4 = {false,NMEMINIBALL_04,4,T_Projectile,0,&s_NMEminiball1};
statetype s_NMEminiball3 = {false,NMEMINIBALL_03,4,T_Projectile,0,&s_NMEminiball4};
statetype s_NMEminiball2 = {false,NMEMINIBALL_02,4,T_Projectile,0,&s_NMEminiball3};
statetype s_NMEminiball1 = {false,NMEMINIBALL_01,4,T_Projectile,0,&s_NMEminiball2};

statetype s_NMEsaucer4 = {false,NMESAUCER_04,6,T_Saucer,0,&s_NMEsaucer1};
statetype s_NMEsaucer3 = {false,NMESAUCER_03,6,T_Saucer,0,&s_NMEsaucer4};
statetype s_NMEsaucer2 = {false,NMESAUCER_02,6,T_Saucer,0,&s_NMEsaucer3};
statetype s_NMEsaucer1 = {false,NMESAUCER_01,6,T_Saucer,SF_SOUND,&s_NMEsaucer2};

statetype s_NMEdie = {16,NMEBODY1_01,0,T_Collide,0,&s_NMEdie};


statetype s_NMEspinfire = {16,NMEBODY1_01,0,T_NME_SpinFire,0,&s_NMEspinfire};
statetype s_NMEattack = {16,NMEBODY1_01,0,T_NME_Attack,0,&s_NMEattack};
statetype s_NMEhead1rl = {16,NMEROCKET_01,0,NULL,0,&s_NMEhead1rl};
statetype s_NMEhead2rl = {16,NMEROCKET2_01,0,NULL,0,&s_NMEhead2rl};

statetype s_NMEwindup = {16,NMEBODY1_01,0,T_NME_WindUp,0,&s_NMEwindup};

statetype s_NMEwheels120 = {16,NMEWHEEL1_01,0,NULL,0,&s_NMEwheels120};

statetype s_NMEwrotleft3 = {16,NMEWHEEL4_01,10,NULL,0,&s_NMEwrotleft3};
statetype s_NMEwrotleft2 = {16,NMEWHEEL3_01,10,NULL,0,&s_NMEwrotleft3};
statetype s_NMEwrotleft1 = {16,NMEWHEEL2_01,10,NULL,0,&s_NMEwrotleft2};

statetype s_NMEwrotright3 = {16,NMEWHEEL4_01,10,NULL,0,&s_NMEwrotleft3};
statetype s_NMEwrotright2 = {16,NMEWHEEL5_01,10,NULL,0,&s_NMEwrotleft3};
statetype s_NMEwrotright1 = {16,NMEWHEEL2_01,10,NULL,0,&s_NMEwrotleft2};

statetype  s_oshuriken4 = {false,SPR_OSHUR4,1,T_Projectile,0,&s_oshuriken1};
statetype  s_oshuriken3 = {false,SPR_OSHUR3,1,T_Projectile,0,&s_oshuriken4};
statetype  s_oshuriken2 = {false,SPR_OSHUR2,1,T_Projectile,0,&s_oshuriken3};
statetype  s_oshuriken1 = {false,SPR_OSHUR1,1,T_Projectile,0,&s_oshuriken2};

statetype  s_oshurikenhit3 = {false,SPR_OSHURHIT3,3,NULL,0,NULL};
statetype  s_oshurikenhit2 = {false,SPR_OSHURHIT2,3,NULL,0,&s_oshurikenhit3};
statetype  s_oshurikenhit1 = {false,SPR_OSHURHIT1,3,NULL,0,&s_oshurikenhit2};

statetype s_speardown16 = {false,SPEARDOWN16,2,T_Spears,SF_CRUSH,&s_speardown1};
statetype s_speardown15 = {false,SPEARDOWN15,2,T_Spears,SF_CRUSH,&s_speardown16};
statetype s_speardown14 = {false,SPEARDOWN14,2,T_Spears,SF_CRUSH,&s_speardown15};
statetype s_speardown13 = {false,SPEARDOWN13,2,T_Spears,SF_CRUSH,&s_speardown14};
statetype s_speardown12 = {false,SPEARDOWN12,2,T_Spears,SF_CRUSH,&s_speardown13};
statetype s_speardown11 = {false,SPEARDOWN11,2,T_Spears,SF_CRUSH,&s_speardown12};
statetype s_speardown10 = {false,SPEARDOWN10,2,T_Spears,SF_CRUSH,&s_speardown11};
statetype s_speardown9 = {false,SPEARDOWN9,2,T_Spears,SF_DOWN,&s_speardown10};

statetype s_speardown8 = {false,SPEARDOWN8,35,T_Spears,SF_DOWN|SF_SOUND,&s_speardown9};
statetype s_speardown7 = {false,SPEARDOWN7,2,T_Spears,SF_DOWN,&s_speardown8};
statetype s_speardown6 = {false,SPEARDOWN6,2,T_Spears,SF_CRUSH,&s_speardown7};
statetype s_speardown5 = {false,SPEARDOWN5,2,T_Spears,SF_CRUSH,&s_speardown6};
statetype s_speardown4 = {false,SPEARDOWN4,2,T_Spears,SF_CRUSH,&s_speardown5};
statetype s_speardown3 = {false,SPEARDOWN3,2,T_Spears,SF_CRUSH,&s_speardown4};
statetype s_speardown2 = {false,SPEARDOWN2,2,T_Spears,SF_CRUSH,&s_speardown3};
statetype s_speardown1 = {false,SPEARDOWN1,2,T_Spears,SF_CRUSH,&s_speardown2};



statetype s_downblade16 = {false,DBLADE8,1,T_Path,0,&s_downblade1};
statetype s_downblade15 = {false,DBLADE7,2,T_Path,0,&s_downblade16};
statetype s_downblade14 = {false,DBLADE6,1,T_Path,0,&s_downblade15};
statetype s_downblade13 = {false,DBLADE5,2,T_Path,0,&s_downblade14};
statetype s_downblade12 = {false,DBLADE4,1,T_Path,0,&s_downblade13};
statetype s_downblade11 = {false,DBLADE3,2,T_Path,0,&s_downblade12};
statetype s_downblade10 = {false,DBLADE2,1,T_Path,0,&s_downblade11};
statetype s_downblade9 = {false,DBLADE9,2,T_Path,0,&s_downblade10};
statetype s_downblade8 = {false,DBLADE8,1,T_Path,0,&s_downblade9};
statetype s_downblade7 = {false,DBLADE7,2,T_Path,0,&s_downblade8};
statetype s_downblade6 = {false,DBLADE6,1,T_Path,0,&s_downblade7};
statetype s_downblade5 = {false,DBLADE5,2,T_Path,0,&s_downblade6};
statetype s_downblade4 = {false,DBLADE4,1,T_Path,0,&s_downblade5};
statetype s_downblade3 = {false,DBLADE3,2,T_Path,0,&s_downblade4};
statetype s_downblade2 = {false,DBLADE2,1,T_Path,0,&s_downblade3};
statetype s_downblade1 = {false,DBLADE1,2,T_Path,SF_SOUND,&s_downblade2};


statetype s_firejetdown23 = {false,FIREJETDOWN23,3,T_Path,SF_CRUSH,&s_firejetdown1};
statetype s_firejetdown22 = {false,FIREJETDOWN22,3,T_Path,SF_CRUSH,&s_firejetdown23};
statetype s_firejetdown21 = {false,FIREJETDOWN21,3,T_Path,0,&s_firejetdown22};
statetype s_firejetdown20 = {false,FIREJETDOWN20,3,T_Path,0,&s_firejetdown21};
statetype s_firejetdown19 = {false,FIREJETDOWN19,3,T_Path,0,&s_firejetdown20};
statetype s_firejetdown18 = {false,FIREJETDOWN18,3,T_Path,0,&s_firejetdown19};
statetype s_firejetdown17 = {false,FIREJETDOWN17,3,T_Path,0,&s_firejetdown18};
statetype s_firejetdown16 = {false,FIREJETDOWN16,3,T_Path,SF_CRUSH,&s_firejetdown17};
statetype s_firejetdown15 = {false,FIREJETDOWN15,3,T_Path,SF_CRUSH,&s_firejetdown16};
statetype s_firejetdown14 = {false,FIREJETDOWN14,3,T_Path,SF_CRUSH,&s_firejetdown15};
statetype s_firejetdown13 = {false,FIREJETDOWN13,3,T_Path,0,&s_firejetdown14};
statetype s_firejetdown12 = {false,FIREJETDOWN12,3,T_Path,0,&s_firejetdown13};
statetype s_firejetdown11 = {false,FIREJETDOWN11,3,T_Path,0,&s_firejetdown12};
statetype s_firejetdown10 = {false,FIREJETDOWN10,3,T_Path,0,&s_firejetdown11};
statetype s_firejetdown9 = {false,FIREJETDOWN9,3,T_Path,0,&s_firejetdown10};
statetype s_firejetdown8 = {false,FIREJETDOWN8,3,T_Path,SF_CRUSH,&s_firejetdown9};
statetype s_firejetdown7 = {false,FIREJETDOWN7,3,T_Path,SF_CRUSH,&s_firejetdown8};
statetype s_firejetdown6 = {false,FIREJETDOWN6,3,T_Path,SF_CRUSH,&s_firejetdown7};
statetype s_firejetdown5 = {false,FIREJETDOWN5,3,T_Path,0,&s_firejetdown6};
statetype s_firejetdown4 = {false,FIREJETDOWN4,3,T_Path,0,&s_firejetdown5};
statetype s_firejetdown3 = {false,FIREJETDOWN3,3,T_Path,0,&s_firejetdown4};
statetype s_firejetdown2 = {false,FIREJETDOWN2,3,T_Path,0,&s_firejetdown3};
statetype s_firejetdown1 = {false,FIREJETDOWN1,70,T_Path,SF_SOUND,&s_firejetdown2};



statetype s_columnupdown6 = {false,CRUSHUP7,5,T_CrushUp,SF_DOWN,&s_columnupup1};
statetype s_columnupdown5 = {false,CRUSHUP6,2,T_CrushUp,SF_DOWN,&s_columnupdown6};
statetype s_columnupdown4 = {false,CRUSHUP5,5,T_CrushUp,SF_DOWN|SF_BLOCK,&s_columnupdown5};
statetype s_columnupdown3 = {false,CRUSHUP4,5,T_CrushUp,SF_DOWN|SF_BLOCK,&s_columnupdown4};
statetype s_columnupdown2 = {false,CRUSHUP3,5,T_CrushUp,SF_DOWN|SF_BLOCK,&s_columnupdown3};
statetype s_columnupdown1 = {false,CRUSHUP2,2,T_CrushUp,SF_DOWN|SF_BLOCK,&s_columnupdown2};


statetype s_columnupup8 = {false,CRUSHUP1,5,T_CrushUp,SF_CRUSH|SF_BLOCK,&s_columnupdown1};
statetype s_columnupup7 = {false,CRUSHUP2,2,T_CrushUp,SF_CRUSH|SF_UP|SF_BLOCK,&s_columnupup8};
statetype s_columnupup6 = {false,CRUSHUP3,5,T_CrushUp,SF_UP|SF_BLOCK,&s_columnupup7};
statetype s_columnupup5 = {false,CRUSHUP4,5,T_CrushUp,SF_UP|SF_BLOCK,&s_columnupup6};
statetype s_columnupup4 = {false,CRUSHUP5,5,T_CrushUp,SF_UP,&s_columnupup5};
statetype s_columnupup3 = {false,CRUSHUP6,2,T_CrushUp,SF_UP,&s_columnupup4};
statetype s_columnupup2 = {false,CRUSHUP7,5,T_CrushUp,SF_UP,&s_columnupup3};
statetype s_columnupup1 = {false,CRUSHUP8,30,T_CrushUp,SF_SOUND,&s_columnupup2};



statetype s_spinupblade16 = {false,SPINUBLADE_16,2,T_Path,SF_DOWN,&s_spinupblade1};
statetype s_spinupblade15 = {false,SPINUBLADE_15,2,T_Path,0,&s_spinupblade16};
statetype s_spinupblade14 = {false,SPINUBLADE_14,2,T_Path,0,&s_spinupblade15};
statetype s_spinupblade13 = {false,SPINUBLADE_13,2,T_Path,0,&s_spinupblade14};
statetype s_spinupblade12 = {false,SPINUBLADE_12,2,T_Path,0,&s_spinupblade13};
statetype s_spinupblade11 = {false,SPINUBLADE_11,2,T_Path,0,&s_spinupblade12};
statetype s_spinupblade10 = {false,SPINUBLADE_10,2,T_Path,0,&s_spinupblade11};
statetype s_spinupblade9 = {false,SPINUBLADE_09,2,T_Path,0,&s_spinupblade10};

statetype s_spinupblade8 = {false,SPINUBLADE_08,2,T_Path,0,&s_spinupblade9};
statetype s_spinupblade7 = {false,SPINUBLADE_07,2,T_Path,0,&s_spinupblade8};
statetype s_spinupblade6 = {false,SPINUBLADE_06,2,T_Path,0,&s_spinupblade7};
statetype s_spinupblade5 = {false,SPINUBLADE_05,2,T_Path,0,&s_spinupblade6};
statetype s_spinupblade4 = {false,SPINUBLADE_04,2,T_Path,0,&s_spinupblade5};
statetype s_spinupblade3 = {false,SPINUBLADE_03,2,T_Path,0,&s_spinupblade4};
statetype s_spinupblade2 = {false,SPINUBLADE_02,2,T_Path,0,&s_spinupblade3};
statetype s_spinupblade1 = {false,SPINUBLADE_01,35,T_Path,SF_UP|SF_SOUND,&s_spinupblade2};


statetype s_spindownblade16 = {false,SPINDBLADE_16,2,T_Path,SF_DOWN,&s_spindownblade1};
statetype s_spindownblade15 = {false,SPINDBLADE_15,2,T_Path,0,&s_spindownblade16};
statetype s_spindownblade14 = {false,SPINDBLADE_14,2,T_Path,0,&s_spindownblade15};
statetype s_spindownblade13 = {false,SPINDBLADE_13,2,T_Path,0,&s_spindownblade14};
statetype s_spindownblade12 = {false,SPINDBLADE_12,2,T_Path,0,&s_spindownblade13};
statetype s_spindownblade11 = {false,SPINDBLADE_11,2,T_Path,0,&s_spindownblade12};
statetype s_spindownblade10 = {false,SPINDBLADE_10,2,T_Path,0,&s_spindownblade11};
statetype s_spindownblade9 = {false,SPINDBLADE_09,2,T_Path,0,&s_spindownblade10};

statetype s_spindownblade8 = {false,SPINDBLADE_08,2,T_Path,0,&s_spindownblade9};
statetype s_spindownblade7 = {false,SPINDBLADE_07,2,T_Path,0,&s_spindownblade8};
statetype s_spindownblade6 = {false,SPINDBLADE_06,2,T_Path,0,&s_spindownblade7};
statetype s_spindownblade5 = {false,SPINDBLADE_05,2,T_Path,0,&s_spindownblade6};
statetype s_spindownblade4 = {false,SPINDBLADE_04,2,T_Path,0,&s_spindownblade5};
statetype s_spindownblade3 = {false,SPINDBLADE_03,2,T_Path,0,&s_spindownblade4};
statetype s_spindownblade2 = {false,SPINDBLADE_02,2,T_Path,0,&s_spindownblade3};
statetype s_spindownblade1 = {false,SPINDBLADE_01,35,T_Path,SF_UP|SF_SOUND,&s_spindownblade2};


statetype s_bouldersink9 = {false,BSINK9,2,NULL,0,NULL};
statetype s_bouldersink8 = {false,BSINK8,2,NULL,0,&s_bouldersink9};
statetype s_bouldersink7 = {false,BSINK7,2,NULL,0,&s_bouldersink8};
statetype s_bouldersink6 = {false,BSINK6,2,NULL,0,&s_bouldersink7};
statetype s_bouldersink5 = {false,BSINK5,2,NULL,0,&s_bouldersink6};
statetype s_bouldersink4 = {false,BSINK4,2,NULL,0,&s_bouldersink5};
statetype s_bouldersink3 = {false,BSINK3,2,NULL,0,&s_bouldersink4};
statetype s_bouldersink2 = {false,BSINK2,2,NULL,0,&s_bouldersink3};
statetype s_bouldersink1 = {false,BSINK1,2,NULL,0,&s_bouldersink2};


statetype s_boulderroll8 = {false,BOULDER41,3,T_BoulderMove,0,&s_boulderroll1};
statetype s_boulderroll7 = {false,BOULDER31,3,T_BoulderMove,0,&s_boulderroll8};
statetype s_boulderroll6 = {false,BOULDER21,3,T_BoulderMove,0,&s_boulderroll7};
statetype s_boulderroll5 = {false,BOULDER11,3,T_BoulderMove,0,&s_boulderroll6};
statetype s_boulderroll4 = {false,BOULDER41,3,T_BoulderMove,0,&s_boulderroll5};
statetype s_boulderroll3 = {false,BOULDER31,3,T_BoulderMove,0,&s_boulderroll4};
statetype s_boulderroll2 = {false,BOULDER21,3,T_BoulderMove,0,&s_boulderroll3};
statetype s_boulderroll1 = {false,BOULDER11,3,T_BoulderMove,SF_SOUND,&s_boulderroll2};




statetype s_boulderdrop12 = {false,BOULDER11,0,T_BoulderDrop,0,&s_boulderdrop12};
statetype s_boulderdrop11 = {false,BDROP11,1,T_BoulderDrop,0,&s_boulderdrop12};
statetype s_boulderdrop10 = {false,BDROP10,1,T_BoulderDrop,0,&s_boulderdrop11};
statetype s_boulderdrop9 = {false,BDROP9,2,T_BoulderDrop,0,&s_boulderdrop10};
statetype s_boulderdrop8 = {false,BDROP8,1,T_BoulderDrop,0,&s_boulderdrop9};
statetype s_boulderdrop7 = {false,BDROP7,1,T_BoulderDrop,0,&s_boulderdrop8};
statetype s_boulderdrop6 = {false,BDROP6,2,T_BoulderDrop,0,&s_boulderdrop7};
statetype s_boulderdrop5 = {false,BDROP5,3,T_BoulderDrop,0,&s_boulderdrop6};
statetype s_boulderdrop4 = {false,BDROP4,4,T_BoulderDrop,0,&s_boulderdrop5};
statetype s_boulderdrop3 = {false,BDROP3,5,T_BoulderDrop,SF_SOUND,&s_boulderdrop4};
statetype s_boulderdrop2 = {false,BDROP2,6,NULL,0,&s_boulderdrop3};
statetype s_boulderdrop1 = {false,BDROP1,6,NULL,0,&s_boulderdrop2};

statetype s_boulderspawn = {false,NOTHING,70,T_BoulderSpawn,0,&s_boulderspawn};



/*==========================================================================

									GUN STUFF

============================================================================*/

statetype s_gunfire2 = {true,GUNRISE51,5,A_GunShoot,0,&s_gunfire1};
statetype s_gunfire1 = {true,GUNFIRE1,5,A_GunShoot,0,&s_gunfire2};

statetype s_gunstand = {true,GUNRISE11,0,T_GunStand,0,&s_gunstand};

statetype s_gunraise4 = {true,GUNRISE51,2,NULL,0,&s_gunfire1};
statetype s_gunraise3 = {true,GUNRISE41,2,NULL,0,&s_gunraise4};
statetype s_gunraise2 = {true,GUNRISE31,2,NULL,0,&s_gunraise3};
statetype s_gunraise1 = {true,GUNRISE21,2,NULL,0,&s_gunraise2};

statetype s_gunlower3 = {true,GUNRISE21,2,NULL,0,&s_gunstand};
statetype s_gunlower2 = {true,GUNRISE31,2,NULL,0,&s_gunlower3};
statetype s_gunlower1 = {true,GUNRISE41,2,NULL,0,&s_gunlower2};


statetype s_gundead = {false,GUNDEAD2,0,T_Collide,0,&s_gundead};
statetype s_gundie1 = {false,GUNDEAD1,5,NULL,0,&s_gundead};



//======================================================================//

statetype s_4waygunfire1 = {true,FOURWAYFIRE01,5,A_4WayGunShoot,0,&s_4waygunfire2};
statetype s_4waygunfire2 = {true,FOURWAY01,5,NULL,0,&s_4waygunfire1};

statetype s_4waygun = {true,FOURWAY01,0,T_4WayGunStand,0,&s_4waygun};




statetype s_kessphere8 = {false,KESSPHERE8,2,T_Projectile,0,&s_kessphere1};
statetype s_kessphere7 = {false,KESSPHERE7,2,T_Projectile,0,&s_kessphere8};
statetype s_kessphere6 = {false,KESSPHERE6,2,T_Projectile,0,&s_kessphere7};
statetype s_kessphere5 = {false,KESSPHERE5,2,T_Projectile,0,&s_kessphere6};
statetype s_kessphere4 = {false,KESSPHERE4,2,T_Projectile,0,&s_kessphere5};
statetype s_kessphere3 = {false,KESSPHERE3,2,T_Projectile,0,&s_kessphere4};
statetype s_kessphere2 = {false,KESSPHERE2,2,T_Projectile,0,&s_kessphere3};
statetype s_kessphere1 = {false,KESSPHERE1,2,T_Projectile,0,&s_kessphere2};


statetype s_slop4 = {false,TOMSPIT4,3,T_Particle,0,&s_slop1};
statetype s_slop3 = {false,TOMSPIT3,3,T_Particle,0,&s_slop4};
statetype s_slop2 = {false,TOMSPIT2,3,T_Particle,0,&s_slop3};
statetype s_slop1 = {false,TOMSPIT1,3,T_Particle,0,&s_slop2};


statetype s_batblast4 = {false,BATBLAST4,3,T_Projectile,SF_BAT,&s_batblast1};
statetype s_batblast3 = {false,BATBLAST3,3,T_Projectile,SF_BAT,&s_batblast4};
statetype s_batblast2 = {false,BATBLAST2,3,T_Projectile,SF_BAT,&s_batblast3};
statetype s_batblast1 = {false,BATBLAST1,3,T_Projectile,SF_BAT,&s_batblast2};


statetype s_serialdog4 = {true,SERIALDOG_W41,5,T_Player,SF_DOGSTATE,&s_serialdog};
statetype s_serialdog3 = {true,SERIALDOG_W31,5,T_Player,SF_DOGSTATE,&s_serialdog4};
statetype s_serialdog2 = {true,SERIALDOG_W21,5,T_Player,SF_DOGSTATE,&s_serialdog3};
statetype s_serialdog = {true,SERIALDOG_W11,5,T_Player,SF_DOGSTATE,&s_serialdog2};
statetype s_serialdogattack = {true,SERIALDOG_ATTACK1,0,T_Attack,SF_DOGSTATE,&s_serialdogattack };


#endif


#include "rt_table.h"
