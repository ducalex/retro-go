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
#ifndef _states_public
#define _states_public

#include "develop.h"

#if (SHAREWARE == 0)
#define MAXSTATES 1300+17+8+32+32
#else
#define MAXSTATES 660+17+8+32+32
#endif

#define SF_CLOSE   0x01
#define SF_CRUSH   0x02
#define SF_UP      0x04
#define SF_DOWN    0x08
#define SF_SOUND   0x10
#define SF_BLOCK   0x20
#define SF_EYE1    0
#define SF_EYE2    1
#define SF_EYE3    2
#define SF_DOGSTATE 0x40
#define SF_BAT      0x80
#define SF_FAKING   0x80
#define SF_DEAD     0x80

typedef struct  statestruct
{
		  byte            rotate;
		  short           shapenum;  // a shapenum of -1 means get from ob->temp1
		  short           tictime;
		  void            (*think) ();
		  signed char     condition;
		  struct  statestruct     *next;
} statetype;

extern   statetype * statetable[MAXSTATES];

extern   statetype s_lowgrdstand;
extern   statetype s_lowgrdpath4;
extern   statetype s_lowgrdpath3;
extern   statetype s_lowgrdpath2;
extern   statetype s_lowgrdpath1;
extern   statetype s_lowgrdcollide;
extern   statetype s_lowgrdcollide2;
//extern   statetype s_lowgrduse1;
extern   statetype s_lowgrdshoot1;
extern   statetype s_lowgrdchase1;
extern   statetype s_lowgrddie1;
extern   statetype s_lowgrdcrushed1;
extern   statetype s_lowgrdcrushed1;
extern   statetype s_sneakydown;
extern   statetype s_sneakyrise1;


extern   statetype s_highgrdstand;
extern   statetype s_highgrdpath1;
extern   statetype s_highgrdcollide;
extern   statetype s_highgrdcollide2;
//extern   statetype s_highgrduse1;
extern   statetype s_highgrdshoot1;
extern   statetype s_highgrdshoot3;
extern   statetype s_highgrdchase1;
extern   statetype s_highgrddie1;
extern   statetype s_highgrdcrushed1;


extern   statetype s_strikestand;
extern   statetype s_strikepath1;
extern   statetype s_strikecollide;
extern   statetype s_strikecollide2;
extern   statetype s_strikeshoot1;
//extern   statetype s_strikeuse1;
extern   statetype s_strikerollright1;
extern   statetype s_strikerollright3;
extern   statetype s_strikerollleft1;
extern   statetype s_strikerollleft3;
extern   statetype s_strikechase1;
extern   statetype s_strikedie1;
extern   statetype s_strikewait;
extern   statetype s_strikecrushed1;


extern   statetype s_blitzstand;
extern   statetype s_blitzpath1;
extern   statetype s_blitzcollide;
extern   statetype s_blitzcollide2;
extern   statetype s_blitzshoot1;
extern   statetype s_blitzuse;
extern   statetype s_blitzsteal1;
extern   statetype s_blitzchase1;
extern   statetype s_blitzdie1;
extern   statetype s_blitzdie3;
extern   statetype s_blitzplead1;
extern   statetype s_blitzplead3;
extern   statetype s_blitzplead4;
extern   statetype s_blitzplead7;
extern   statetype s_blitzaplead5;
extern   statetype s_blitzaplead4;
extern   statetype s_blitzcrushed1;
extern   statetype s_blitzfakedie1;
extern   statetype s_blitzrise2;
extern   statetype s_blitzstruggledie1;
extern   statetype s_blitzstruggledead;

extern   statetype s_enforcerstand;
extern   statetype s_enforcerpath1;
extern   statetype s_enforcercollide;
extern   statetype s_enforcercollide2;
//extern   statetype s_enforceruse1;
extern   statetype s_enforcershoot1;
extern   statetype s_enforcershoot3;
extern   statetype s_enforcerthrow1;
extern   statetype s_grenade1;
extern   statetype s_grenadehit1;
extern   statetype s_enforcerchase1;
extern   statetype s_enforcerdie1;
extern   statetype s_grenade_fall1;
extern   statetype s_grenade_fall6;
extern   statetype s_enforcercrushed1;


extern   statetype s_robogrdstand;
extern   statetype s_robogrdpath1;
extern   statetype s_robogrdshoot1;
extern   statetype s_robogrdshuriken1;
extern   statetype s_shurikenhit1;
extern   statetype s_robogrdchase1;
extern   statetype s_robogrddie1;
extern   statetype s_robogrdcollide;
extern   statetype s_robogrdcollide2;
extern   statetype s_robogrdcrushed1;
extern   statetype s_roboalign;
extern   statetype s_robowait;
extern   statetype s_roborealign;





extern statetype s_altexplosion10;
extern statetype s_altexplosion9 ;
extern statetype s_altexplosion8 ;
extern statetype s_altexplosion7 ;
extern statetype s_altexplosion6 ;
extern statetype s_altexplosion5 ;
extern statetype s_altexplosion4 ;
extern statetype s_altexplosion3  ;
extern statetype s_altexplosion2  ;
extern statetype s_altexplosion1  ;



extern   statetype s_explosion1;
extern   statetype s_explosion2;
extern   statetype s_explosion3;
extern   statetype s_explosion4;
extern   statetype s_explosion5;
extern   statetype s_explosion6;
extern   statetype s_explosion7;
extern   statetype s_explosion8;
extern   statetype s_explosion9;
extern   statetype s_explosion10;
extern   statetype s_explosion11;
extern   statetype s_explosion12;
extern   statetype s_explosion13;
extern   statetype s_explosion14;
extern   statetype s_explosion15;
extern   statetype s_explosion16;
extern   statetype s_explosion17;
extern   statetype s_explosion18;
extern   statetype s_explosion19;
extern   statetype s_explosion20;

extern   statetype s_grexplosion1;
extern   statetype s_grexplosion2;
extern   statetype s_grexplosion3;
extern   statetype s_grexplosion4;
extern   statetype s_grexplosion5;
extern   statetype s_grexplosion6;
extern   statetype s_grexplosion7;
extern   statetype s_grexplosion8;
extern   statetype s_grexplosion9;
extern   statetype s_grexplosion10;
extern   statetype s_grexplosion11;
extern   statetype s_grexplosion12;
extern   statetype s_grexplosion13;
extern   statetype s_grexplosion14;
extern   statetype s_grexplosion15;
extern   statetype s_grexplosion16;
extern   statetype s_grexplosion17;
extern   statetype s_grexplosion18;
extern   statetype s_grexplosion19;
extern   statetype s_grexplosion20;



extern   statetype s_staticexplosion1;
extern   statetype s_staticexplosion2;
extern   statetype s_staticexplosion3;
extern   statetype s_staticexplosion4;
extern   statetype s_staticexplosion5;
extern   statetype s_staticexplosion6;
extern   statetype s_staticexplosion7;
extern   statetype s_staticexplosion8;
extern   statetype s_staticexplosion9;
extern   statetype s_staticexplosion10;
extern   statetype s_staticexplosion11;
extern   statetype s_staticexplosion12;
extern   statetype s_staticexplosion13;
extern   statetype s_staticexplosion14;
extern   statetype s_staticexplosion15;
extern   statetype s_staticexplosion16;
extern   statetype s_staticexplosion17;
extern   statetype s_staticexplosion18;
extern   statetype s_staticexplosion19;
extern   statetype s_staticexplosion20;
extern   statetype s_staticexplosion21;
extern   statetype s_staticexplosion22;
extern   statetype s_staticexplosion23;
extern   statetype s_staticexplosion24;
extern   statetype s_staticexplosion25;

extern   statetype s_upblade1;

extern   statetype s_firejetup1;


extern   statetype s_columndowndown1;


extern   statetype s_spearup1;
extern   statetype s_pushcolumn1;
extern   statetype s_pushcolumn2;
extern   statetype s_pushcolumn3;

extern   statetype s_wallfireball;
extern   statetype s_crossfire1;
extern   statetype s_crossdone1;




extern   statetype s_fireunit1;
extern   statetype s_firespan1;

extern   statetype s_p_bazooka1;
extern   statetype s_p_bazooka2;

extern   statetype s_p_grenade;
extern   statetype s_p_gfall1;
extern   statetype s_p_gfall2;
extern   statetype s_p_gfall3;
extern   statetype s_p_gfall4;

extern   statetype s_gunsmoke1;
extern   statetype s_bloodspurt1;
extern   statetype s_hitmetalwall1;
extern   statetype s_hitmetalactor1;






extern   statetype s_dust;



extern   statetype s_skeleton1;


extern   statetype s_gas2;
extern   statetype s_gas1;


extern   statetype s_spring1;
extern   statetype s_spring2;




extern   statetype s_player;
extern   statetype s_free;

extern   statetype s_pgunattack1;
extern   statetype s_pmissattack1;
extern   statetype s_pgunattack2;
extern   statetype s_pmissattack2;
extern   statetype s_remoteinelev;
extern   statetype s_remotemove1;


extern   statetype s_godfire1;



extern   statetype s_remotedie1;


extern   statetype s_guts1;
extern   statetype s_guts12;


extern   statetype s_bossdeath;
extern   statetype s_megaexplosions;

extern   statetype s_superparticles;
extern   statetype s_gibs1;
extern   statetype s_gibsdone1;
extern   statetype s_bigsoul;
extern   statetype s_littlesoul;
extern   statetype s_vaporized1;
extern   statetype s_autospring1;
extern   statetype s_pbatblast;



extern   statetype s_collectorwander1;
extern   statetype s_collectorfdoor1;
extern   statetype s_tag;
extern   statetype s_timekeeper;
extern   statetype s_skeleton48;
extern   statetype s_skeleton24;

extern   statetype s_wind;
extern   statetype s_remoteguts1;
extern   statetype s_voidwait;
extern   statetype s_ashwait;
extern   statetype s_deadwait;
extern   statetype s_gutwait;
extern   statetype s_vaporized8;
extern   statetype s_remoteguts12;
extern   statetype s_eye1;
extern   statetype s_itemspawn1;
extern   statetype s_deadblood1;


extern   statetype s_flash1;

extern   statetype s_elevdisk;
extern   statetype s_pathdisk;
extern   statetype s_megaremove;

extern   statetype s_respawn1;
extern   statetype s_basemarker1;

extern   statetype s_blooddrip1;

extern   statetype s_diskmaster;
extern   statetype s_bstar1;


#if (SHAREWARE == 0)

extern   statetype s_scottwander1;
extern   statetype s_scottwanderdoor1;

extern   statetype s_opstand;
extern   statetype s_oppath1;
extern   statetype s_opcollide;
extern   statetype s_opcollide2;
extern   statetype s_opgiveup;
//extern   statetype s_opuse1;
extern   statetype s_opshoot1;
extern   statetype s_opbolo1;
extern   statetype s_bolocast1;
extern   statetype s_opchase1;
extern   statetype s_opdie1;
extern   statetype s_opcrushed1;


extern   statetype s_dmonkstand;
extern   statetype s_dmonkpath1;
extern   statetype s_dmonkshoot1;
extern   statetype s_dmonkshoot2;
extern   statetype s_dmonkchase1;
extern   statetype s_dmonkdie1;
extern   statetype s_dmonkcollide;
extern   statetype s_dmonkcollide2;
extern   statetype s_dmonkcrushed1;
extern   statetype s_dmonkshoot5;
extern   statetype s_dmonkshoot3;
extern   statetype s_dmonkshoot4;


extern   statetype s_firemonkstand;
extern   statetype s_firemonkpath1;
extern   statetype s_firemonkcast1;
extern   statetype s_monkfire1;
extern   statetype s_fireballhit1;
extern   statetype s_firemonkchase1;
extern   statetype s_firemonkdie1;
extern   statetype s_firemonkcollide;
extern   statetype s_firemonkcollide2;
extern   statetype s_firemonkcrushed1;


extern   statetype s_wallstand;
extern   statetype s_wallpath;
extern   statetype s_wallshoot;
extern   statetype s_wallcollide;
extern   statetype s_wallalign;
extern   statetype s_wallwait;
extern   statetype s_wallrestore;


extern   statetype s_darianstand;
extern   statetype s_darianchase1;
extern   statetype s_darianuse;
extern   statetype s_darianshoot1;
extern   statetype s_dariancollide;
extern   statetype s_dariancollide2;
extern   statetype s_dariandie1;
extern   statetype s_darianspears;
extern   statetype s_darianuse1;
extern   statetype s_dariansink1;
extern   statetype s_dariansink9;
extern   statetype s_darianrise1;
extern   statetype s_darianwait;
extern   statetype s_dariandefend1;


extern   statetype s_heinrichstand;
extern   statetype s_heinrichshoot1;
extern   statetype s_heinrichshoot4;
extern   statetype s_heinrichshoot9;
extern   statetype s_heinrichooc;
extern   statetype s_heinrichchase;
extern   statetype s_heinexp1;
extern   statetype s_kristleft;
extern   statetype s_kristright;


extern   statetype s_missile1;
extern   statetype s_missilehit1;
extern   statetype s_mine1;
extern   statetype s_heinrichchase1;
extern   statetype s_heinrichuse;
extern   statetype s_heinrichmine;
extern   statetype s_heinrichdie1;
extern   statetype s_heinrichdead;
extern   statetype s_heinrichdefend;

extern   statetype s_dexplosion22;
extern   statetype s_dexplosion21;
extern   statetype s_dexplosion20;
extern   statetype s_dexplosion19;
extern   statetype s_dexplosion18;
extern   statetype s_dexplosion17;
extern   statetype s_dexplosion16;
extern   statetype s_dexplosion15;
extern   statetype s_dexplosion14;
extern   statetype s_dexplosion13;
extern   statetype s_dexplosion12;
extern   statetype s_dexplosion11;
extern   statetype s_dexplosion10;
extern   statetype s_dexplosion9;
extern   statetype s_dexplosion8;
extern   statetype s_dexplosion7;
extern   statetype s_dexplosion6;
extern   statetype s_dexplosion5;
extern   statetype s_dexplosion4;
extern   statetype s_dexplosion3;
extern   statetype s_dexplosion2;
extern   statetype s_dexplosion1;
extern   statetype s_dspear1;



extern   statetype s_NMEchase;
extern   statetype s_NMEdie;
extern   statetype s_NMEhead1;
extern   statetype s_NMEhead2;
extern   statetype s_NMEwheels1;
extern   statetype s_NMEwheels2;
extern   statetype s_NMEwheels3;
extern   statetype s_NMEwheels4;
extern   statetype s_NMEwheels5;
extern   statetype s_NMEwindup;
extern   statetype s_NMEwheels120;

extern   statetype s_NMEwrotleft3;
extern   statetype s_NMEwrotleft2;
extern   statetype s_NMEwrotleft1;

extern   statetype s_NMEwrotright3;
extern   statetype s_NMEwrotright2;
extern   statetype s_NMEwrotright1;
extern   statetype s_NMEminiball1;
extern   statetype s_NMEattack;
extern   statetype s_NMEsaucer1;
extern   statetype s_NMEhead1rl;
extern   statetype s_NMEhead2rl;
extern   statetype s_NMEspinattack;
extern   statetype s_NMEwheelspin;
extern   statetype s_NMEcollide;
extern   statetype s_NMEdeathbuildup;
extern   statetype s_NMEheadexplosion;
extern   statetype s_NMEstand;
extern   statetype s_NMEspinfire;
extern   statetype s_shootinghead;
extern   statetype s_oshuriken1;
extern   statetype s_oshurikenhit1;




extern   statetype s_darkmonkstand;
extern   statetype s_darkmonkcharge1;
extern   statetype s_darkmonkreact;
extern   statetype s_darkmonkland;
extern   statetype s_darkmonkchase1;
extern   statetype s_darkmonkcover1;
extern   statetype s_darkmonkawaken1;
extern   statetype s_darkmonklightning1;
extern   statetype s_darkmonkfspark1;
extern   statetype s_darkmonkbreathe1;
extern   statetype s_darkmonksummon1;
extern   statetype s_darkmonkhead;
extern   statetype s_darkmonkhspawn;
extern   statetype s_darkmonksneer1;
extern   statetype s_darkmonkheadhappy;
extern   statetype s_darkmonkheaddie1;
extern   statetype s_darkmonkhball1;
extern   statetype s_darkmonksphere1;
extern   statetype s_darkmonksphere8;
extern   statetype s_darkmonkbball1;
extern   statetype s_darkmonkscare1;
extern   statetype s_darkmonkdie1;
extern   statetype s_darkmonkredhead;
extern   statetype s_redheadhit;
extern   statetype s_darkmonksnakelink;
extern   statetype s_darkmonkredlink;
extern   statetype s_redlinkhit;
extern   statetype s_energysphere1;
extern   statetype s_lightning;
extern   statetype s_handball2;
extern   statetype s_handball1;
extern   statetype s_faceball2;
extern   statetype s_faceball1;
extern   statetype s_floorspark1;
extern   statetype s_dmlandandfire;

extern   statetype s_darkmonkhball7;
extern   statetype s_darkmonkbball7;
extern   statetype s_darkmonklightning9;
extern   statetype s_darkmonkfspark5;
extern   statetype s_darkmonkbreathe6;
extern   statetype s_darkmonkabsorb1;
extern   statetype s_dmgreenthing1;
extern   statetype s_dmgreenthing8;
extern   statetype s_darkmonkfastspawn;
extern   statetype s_spit1;
extern   statetype s_spithit1;
extern   statetype s_snakefire1;
extern   statetype s_snakepath;
extern   statetype s_snakefindpath;
extern   statetype s_snakefireworks1;
extern   statetype s_snakefireworks2;


extern   statetype s_speardown1;


extern   statetype s_downblade1;


extern   statetype s_firejetdown1;

extern   statetype s_columnupup1;
extern   statetype s_columnupup2;
extern   statetype s_columnupup8;
extern   statetype s_columnupup7;
extern   statetype s_columnupdown1;
extern   statetype s_columnupdown6;

extern   statetype s_spinupblade1;
extern   statetype s_spindownblade1;

extern   statetype s_boulderdrop12;
extern   statetype s_boulderdrop10;
extern   statetype s_boulderdrop8;


extern   statetype s_boulderdrop1;
extern   statetype s_boulderspawn;
extern   statetype s_bouldersink1;
extern   statetype s_boulderroll1;

extern   statetype s_gunfire1;
extern   statetype s_gunfire2;
extern   statetype s_gundie1;
extern   statetype s_gunstand;
extern   statetype s_gunraise1;
extern   statetype s_gunlower1;
extern   statetype s_4waygunfire1;
extern   statetype s_4waygunfire2;
extern   statetype s_4waygun;


extern   statetype s_kessphere1;
extern   statetype s_batblast1;
extern   statetype s_slop1;



extern   statetype s_serialdog4;
extern   statetype s_serialdog3;
extern   statetype s_serialdog2;
extern   statetype s_serialdog;
extern   statetype s_serialdogattack;
extern   statetype s_doguse;
extern   statetype s_doglick;
extern   statetype s_dogwait;



#endif






#endif
