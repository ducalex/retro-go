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

#ifndef FUNCT_H
#define FUNCT_H

#include "duke3d.h"

extern void sendscore(char  *s);
//#line "sounds.c" 25
extern void SoundStartup(void );
//#line "sounds.c" 95
extern void SoundShutdown(void );
//#line "sounds.c" 118
extern void MusicStartup(void );
//#line "sounds.c" 166
extern void MusicShutdown(void );
//#line "sounds.c" 181
extern int USRHOOKS_GetMem(void  **ptr,uint32_t size);
//#line "sounds.c" 192
extern int USRHOOKS_FreeMem(void  *ptr);
//#line "sounds.c" 200
extern void intomenusounds(void );
//#line "sounds.c" 227
extern void playmusic(char  *fn);
//#line "sounds.c" 251
extern uint8_t  loadsound(uint16_t num);
//#line "sounds.c" 277
extern int xyzsound(short num,short i,int32_t x,int32_t y,int32_t z);
//#line "sounds.c" 407
extern void sound(short num);
//#line "sounds.c" 463
extern int spritesound(uint16_t num,short i);
//#line "sounds.c" 469
extern void stopsound(short num);
//#line "sounds.c" 478
extern void stopenvsound(short num,short i);
//#line "sounds.c" 494
extern void pan3dsound(void );
//#line "sounds.c" 571
extern void TestCallBack(int32_t num);
//#line "sector.c" 9
extern short callsound(short sn,short whatsprite);
//#line "sector.c" 56
extern short check_activator_motion(short lotag);
//#line "sector.c" 93
extern uint8_t  isadoorwall(short dapic);
//#line "sector.c" 124
extern uint8_t  isanunderoperator(short lotag);
//#line "sector.c" 139
extern uint8_t  isanearoperator(short lotag);
//#line "sector.c" 161
extern short checkcursectnums(short sect);
//#line "sector.c" 169
extern int32_t ldist(spritetype *s1,spritetype *s2);
//#line "sector.c" 177
extern int32_t dist(spritetype *s1,spritetype *s2);
//#line "sector.c" 186
extern short findplayer(spritetype *s,int32_t *d);
//#line "sector.c" 214
extern short findotherplayer(short p,int32_t *d);
//#line "sector.c" 240
extern void doanimations(void );
//#line "sector.c" 301
extern int32_t getanimationgoal(int32_t *animptr);
//#line "sector.c" 315
extern int32_t setanimation(short animsect,int32_t *animptr,int32_t thegoal,int32_t thevel);
//#line "sector.c" 348
extern void animatecamsprite(void );
//#line "sector.c" 369
extern void animatewalls(void );
//#line "sector.c" 457
extern uint8_t  activatewarpelevators(short s,short d);
//#line "sector.c" 504
extern void operatesectors(short sn,short ii);
//#line "sector.c" 997
extern void operaterespawns(short low);
//#line "sector.c" 1020
extern void operateactivators(short low,short snum);
//#line "sector.c" 1089
extern void operatemasterswitches(short low);
//#line "sector.c" 1104
extern void operateforcefields(short s,short low);
//#line "sector.c" 1140
extern uint8_t  checkhitswitch(short snum,int32_t w,uint8_t  switchtype);
//#line "sector.c" 1515
extern void activatebysector(short sect,short j);
//#line "sector.c" 1532
extern void checkhitwall(short spr,short dawallnum,int32_t x,int32_t y,int32_t z,short atwith);
//#line "sector.c" 1746
extern void checkplayerhurt(struct player_struct *p,short j);
//#line "sector.c" 1816
extern uint8_t  checkhitceiling(short sn);
//#line "sector.c" 1891
extern void checkhitsprite(short i,short sn);
//#line "sector.c" 2326
extern void allignwarpelevators(void );
//#line "sector.c" 2357
extern void cheatkeys(short snum);
//#line "sector.c" 2766
extern void checksectors(short snum);
//#line "rts.c" 36
extern void RTS_AddFile(char  *filename);
//#line "rts.c" 93
extern void RTS_Init(char *filename);
//#line "rts.c" 126
extern int32 RTS_NumSounds(void );
//#line "rts.c" 141
extern int32 RTS_SoundLength(int32 lump);
//#line "rts.c" 157
extern char  *RTS_GetSoundName(int32 i);
//#line "rts.c" 174
extern void RTS_ReadLump(int32 lump,void *dest);
//#line "rts.c" 194
extern void *RTS_GetSound(int32 lump);
//#line "premap.c" 7
extern void xyzmirror(short i,short wn);
//#line "premap.c" 20
extern void vscrn(void );
//#line "premap.c" 58
int countfragbars(void);
extern void pickrandomspot(short snum);
//#line "premap.c" 80
extern void resetplayerstats(short snum);
//#line "premap.c" 187
extern void resetweapons(short snum);
//#line "premap.c" 212
extern void resetinventory(short snum);
//#line "premap.c" 232
extern void resetprestat(short snum,uint8_t  g);
//#line "premap.c" 294
extern void setupbackdrop(short backpicnum);
//#line "premap.c" 329
extern void cachespritenum(short i);
//#line "premap.c" 443
extern void cachegoodsprites(void );
//#line "premap.c" 516
extern void prelevel(uint8_t  g);
//#line "premap.c" 823
extern void newgame(uint8_t  vn,uint8_t  ln,uint8_t  sk);
//#line "premap.c" 874
extern void resetpspritevars(uint8_t  g);
//#line "premap.c" 1012
extern void resettimevars(void );
//#line "premap.c" 1043
extern void genspriteremaps(void );
//#line "premap.c" 1077
extern void waitforeverybody(void);
//#line "premap.c" 1131
extern uint8_t  checksum(int32_t sum);
//#line "premap.c" 1163
extern uint8_t  getsound(uint16_t num);
//#line "premap.c" 1189
extern void precachenecessarysounds(void );
//#line "premap.c" 1201
extern void cacheit(void );
//#line "premap.c" 1244
extern void dofrontscreens(void );
//#line "premap.c" 1285
extern void enterlevel(uint8_t  g);
//#line "player.c" 10
extern void setpal(struct player_struct *p);
//#line "player.c" 28
extern void incur_damage(struct player_struct *p);
//#line "player.c" 59
extern void quickkill(struct player_struct *p);
//#line "player.c" 73
extern void forceplayerangle(struct player_struct *p);
//#line "player.c" 85
extern void tracers(int32_t x1,int32_t y1,int32_t z1,int32_t x2,int32_t y2,int32_t z2,int32_t n);
//#line "player.c" 114
extern int32_t hits(short i);
//#line "player.c" 131
extern int32_t hitasprite(short i,short *hitsp);
//#line "player.c" 152
extern int32_t hitawall(struct player_struct *p,short *hitw);
//#line "player.c" 163
extern short aim(spritetype *s,short aang, short auto_aim);
//#line "player.c" 234
extern void shoot(short i,short atwith);
//#line "player.c" 939
extern void displayloogie(short snum);
//#line "player.c" 958
extern uint8_t  animatefist(short gs,short snum);
//#line "player.c" 986
extern uint8_t  animateknee(short gs,short snum);
//#line "player.c" 1002
extern uint8_t  animateknuckles(short gs,short snum);
//#line "player.c" 1022
extern void displaymasks(short snum);
//#line "player.c" 1043
extern uint8_t  animatetip(short gs,short snum);
//#line "player.c" 1064
extern uint8_t  animateaccess(short gs,short snum);
//#line "player.c" 1090
extern void displayweapon(short snum);
//#line "player.c" 1501
extern void getinput(short snum);
//#line "player.c" 1742
extern uint8_t  doincrements(struct player_struct *p);
//#line "player.c" 1935
extern void checkweapons(struct player_struct *p);
//#line "player.c" 1951
extern void processinput(short snum);
//#line "menues.c" 18
extern void cmenu(short cm);
//#line "menues.c" 38
extern void savetemp(char  *fn,uint8_t* daptr,int32_t dasiz);
//#line "menues.c" 49
extern void getangplayers(short snum);
//#line "menues.c" 67
extern int loadpheader(uint8_t  spot,int32 *vn,int32 *ln,int32 *psk,int32 *numplr);
//#line "menues.c" 105
extern int loadplayer(int8_t spot);
//#line "menues.c" 276
extern int saveplayer(int8_t spot);
//#line "menues.c" 421
extern void sendgameinfo(void );
//#line "menues.c" 434
extern int probe(int x,int y,int i,int n);
extern int probeXduke(int x,int y,int i,int n, int32_t spriteSize);
//#line "menues.c" 521
extern int menutext(int x,int y,short s,short p,char  *t);
//#line "menues.c" 630
extern int menutextc(int x,int y,short s,short p,char  *t);
//#line "menues.c" 727
extern void bar(int x,int y,short *p,short dainc,uint8_t  damodify,short s,short pa);
//#line "menues.c" 806
extern void dispnames(void );
//#line "menues.c" 832
extern int getfilenames(char  kind[]);
//#line "menues.c" 865
extern void sortfilenames(void);
//#line "menues.c" 886
extern void menus(void );
void gameexitanycase(void);
//#line "menues.c" 2414
extern void palto(uint8_t  r,uint8_t  g,uint8_t  b,int32_t e);
//#line "menues.c" 2436
extern void drawoverheadmap(int32_t cposx,int32_t cposy,int32_t czoom,short cang);
//#line "menues.c" 2685
extern void playanm(char  *fn,uint8_t );
//#line "gamedef.c" 122
extern short getincangle(short a,short na);
//#line "gamedef.c" 140
extern uint8_t  ispecial(uint8_t  c);
//#line "gamedef.c" 154
extern uint8_t  isaltok(uint8_t  c);
//#line "gamedef.c" 159
extern void getglobalz(short i);
//#line "gamedef.c" 203
extern void makeitfall(short i);
//#line "gamedef.c" 243
extern void getlabel(void );
//#line "gamedef.c" 267
extern int32_t keyword(void );
//#line "gamedef.c" 300
extern int32_t transword(void );
//#line "gamedef.c" 342
extern void transnum(void );
//#line "gamedef.c" 402
extern uint8_t  parsecommand(int readfromGRP );
//#line "gamedef.c" 1227
extern void passone(int readfromGRP);
//#line "gamedef.c" 1239
extern void loadefs(char  *fn,char  *mptr, int readfromGRP);
//#line "gamedef.c" 1342
extern uint8_t  dodge(spritetype *s);
//#line "gamedef.c" 1374
extern short furthestangle(short i,short angs);
//#line "gamedef.c" 1404
extern short furthestcanseepoint(short i,spritetype *ts,int32_t *dax,int32_t *day);
//#line "gamedef.c" 1444
extern void alterang(short a);
//#line "gamedef.c" 1512
extern void move(void);
//#line "gamedef.c" 1711
extern void parseifelse(int32_t condition);
//#line "gamedef.c" 1729
extern uint8_t  parse(void );
//#line "gamedef.c" 2699
extern void execute(short i,short p,int32_t x);
//#line "game.c" 63
extern void overwritesprite(int32_t thex,int32_t they,short tilenum,int8_t shade,uint8_t  stat,uint8_t  dapalnum);
//#line "game.c" 70
extern void timerhandler(void);
//#line "game.c" 75
extern int inittimer(int);
//#line "game.c" 81
extern void uninittimer(void );
//#line "game.c" 89
extern int gametext(int x,int y,char  *t,uint8_t  s,short dabits);
//#line "game.c" 136
extern int gametextpart(int x,int y,char  *t,uint8_t  s,short p);
//#line "game.c" 192
extern int minitext(int x,int y,char  *t,uint8_t  p,uint8_t  sb);
//#line "game.c" 231
extern void gamenumber(int32_t x,int32_t y,int32_t n,uint8_t  s);
//#line "game.c" 248
extern void ShutDown(void );
//#line "game.c" 260
extern void allowtimetocorrecterrorswhenquitting(void );
//#line "game.c" 280
extern void getpackets(void );
//#line "game.c" 502
extern void faketimerhandler(void);
//#line "game.c" 788
extern void checksync(void );
//#line "game.c" 815
extern void check_fta_sounds(short i);
//#line "game.c" 869
extern short inventory(spritetype *s);
//#line "game.c" 886
extern short badguy(spritetype *s);
//#line "game.c" 924
extern short badguypic(short pn);
//#line "game.c" 963
extern void myos(int32_t x,int32_t y,short tilenum,int8_t shade,uint8_t  orientation);
//#line "game.c" 976
extern void myospal(int32_t x,int32_t y,short tilenum,int8_t shade,uint8_t  orientation,uint8_t  p);
//#line "game.c" 991
extern void invennum(int32_t x,int32_t y,uint8_t  num1,uint8_t  ha,uint8_t  sbits);
//#line "game.c" 1021
extern void weaponnum(short ind,int32_t x,int32_t y,int32_t num1,int32_t num2,uint8_t  ha);
//#line "game.c" 1049
extern void weaponnum999(uint8_t  ind,int32_t x,int32_t y,int32_t num1,int32_t num2,uint8_t  ha);
//#line "game.c" 1088
extern void weapon_amounts(struct player_struct *p,int32_t x,int32_t y,int32_t u);
//#line "game.c" 1197
extern void digitalnumber(int32_t x,int32_t y,int32_t n,uint8_t  s,uint8_t  cs);
//#line "game.c" 1223
extern void scratchmarks(int32_t x,int32_t y,int32_t n,uint8_t  s,uint8_t  p);
//#line "game.c" 1238
extern void displayinventory(struct player_struct *p);
//#line "game.c" 1296
extern void displayfragbar(void );
//#line "game.c" 1318
void drawsmallweapon(short weapon, float scale, short x, short y); // xduke
extern void coolgaugetext(short snum);
//#line "game.c" 1557
extern void tics(short offx, short offy , short color);
//#line "game.c" 1572
extern void clocks(void );
//#line "game.c" 1582
extern void coords(short snum);
//#line "game.c" 1607
extern void operatefta(void);
//#line "game.c" 1654
extern void FTA(short q,struct player_struct *p, int mode);
//#line "game.c" 1668
extern void showtwoscreens(void );

//#line "game.c" 1705
extern void gameexit(char  *t);
//#line "game.c" 1752
extern short strget(short x,short y,char  *t,short dalen,short c);
//#line "game.c" 1819
extern void displayrest(int32_t smoothratio);
//#line "game.c" 2047
extern void updatesectorz(int32_t x,int32_t y,int32_t z,short *sectnum);
//#line "game.c" 2085
extern void view(struct player_struct *pp,int32_t *vx,int32_t *vy,int32_t *vz,short *vsectnum,short ang,short horiz);
//#line "game.c" 2137
extern void drawbackground(void );
//#line "game.c" 2200
extern void displayrooms(short snum,int32_t smoothratio);
//#line "game.c" 2445
extern short LocateTheLocator(short n,short sn);
//#line "game.c" 2459
extern short EGS(short whatsect,int32_t s_x,int32_t s_y,int32_t s_z,short s_pn,int8_t s_s,int8_t s_xr,int8_t s_yr,short s_a,short s_ve,int32_t s_zv,short s_ow,int8_t s_ss);
//#line "game.c" 2537
extern uint8_t  wallswitchcheck(short i);
//#line "game.c" 2588
extern short spawn(short j,short pn);
//#line "game.c" 4181
extern void animatesprites(int32_t x,int32_t y,short a,int32_t smoothratio);
//#line "game.c" 4859
extern void cheats(void );
//#line "game.c" 5303
extern void nonsharedkeys(void );
//#line "game.c" 5863
extern void comlinehelp(char  **argv);
//#line "game.c" 5889
extern void checkcommandline(int argc,char  **argv);
//#line "game.c" 6078
extern void printstr(short x,short y,uint8_t  string[],uint8_t  attribute);
//#line "game.c" 6104
extern void Logo(void );
//#line "game.c" 6187
extern void loadtmb(void );
//#line "game.c" 6210
extern void compilecons(void );
//#line "game.c" 6230
extern void Startup(void );
//#line "game.c" 6284
extern void getnames(void );
//#line "game.c" 6309
extern int main(int argc,char  **argv);
//#line "game.c" 6563
extern uint8_t  opendemoread(uint8_t  which_demo);
//#line "game.c" 6589
extern void opendemowrite(void );
//#line "game.c" 6608
extern void record(void );
//#line "game.c" 6626
extern void closedemowrite(void );
//#line "game.c" 6642
extern int32_t playback(void );
//#line "game.c" 6777
extern uint8_t  moveloop(void);
//#line "game.c" 6796
extern void fakedomovethingscorrect(void);
//#line "game.c" 6829
extern void fakedomovethings(void );
//#line "game.c" 7247
extern uint8_t  domovethings(void );
//#line "game.c" 7373
extern void displaybonuspics(short x,short y,short p);
//#line "game.c" 7396
extern void doorders(void );
//#line "game.c" 7432
extern void dobonus(uint8_t  bonusonly);
//#line "game.c" 7846
extern void cameratext(short i);
//#line "game.c" 7869
extern void vglass(int32_t x,int32_t y,short a,short wn,short n);
//#line "game.c" 7882
extern void lotsofglass(short i,short wallnum,short n);
//#line "game.c" 7924
extern void spriteglass(short i,short n);
//#line "game.c" 7937
extern void ceilingglass(short i,short sectnum,short n);
//#line "game.c" 7966
extern void lotsofcolourglass(short i,short wallnum,short n);
//#line "game.c" 8004
extern void SetupGameButtons(void );
//#line "game.c" 8068
extern int32_t GetTime(void );
//#line "game.c" 8082
extern void CenterCenter(void );
//#line "game.c" 8095
extern void UpperLeft(void );
//#line "game.c" 8108
extern void LowerRight(void );
//#line "game.c" 8121
extern void CenterThrottle(void );
//#line "game.c" 8134
extern void CenterRudder(void );
//#line "game.c" ????
extern void takescreenshot(void );
//#line "config.c" 57
extern void CONFIG_GetSetupFilename(void );
//#line "config.c" 170
extern int32 CONFIG_FunctionNameToNum(char  *func);
//#line "config.c" 192
extern char  *CONFIG_FunctionNumToName(int32 func);
//#line "config.c" 211
extern int32 CONFIG_AnalogNameToNum(char  *func);
//#line "config.c" 240
extern void CONFIG_SetDefaults(void );
//#line "config.c" 264
extern void CONFIG_ReadKeys(void );
//#line "config.c" 314
extern void CONFIG_SetupMouse(int32 scripthandle);
//#line "config.c" 376
extern void CONFIG_SetupGamePad(int32 scripthandle);
//#line "config.c" 427
extern void CONFIG_SetupJoystick(int32 scripthandle);
//#line "config.c" 485
extern void readsavenames(void );
//#line "config.c" 512
extern void CONFIG_ReadSetup(void );
//#line "config.c" 613
extern void CONFIG_WriteSetup(void );
//#line "animlib.c" 34
extern void CheckAnimStarted(uint8_t  *funcname);
//#line "animlib.c" 46
extern uint16 findpage(uint16 framenumber);
//#line "animlib.c" 71
extern void loadpage(uint16 pagenumber,uint16 *pagepointer);
//#line "animlib.c" 97
extern void CPlayRunSkipDump(byte *srcP,byte *dstP);
//#line "animlib.c" 177
extern void renderframe(uint16 framenumber,uint16 *pagepointer);
//#line "animlib.c" 214
extern void drawframe(uint16 framenumber);
//#line "animlib.c" 228
extern void ANIM_LoadAnim(byte *buffer);
//#line "animlib.c" 260
extern void ANIM_FreeAnim(void );
//#line "animlib.c" 275
extern int32 ANIM_NumFrames(void );
//#line "animlib.c" 287
extern byte *ANIM_DrawFrame(int32 framenumber);
//#line "animlib.c" 312
extern byte *ANIM_GetPalette(void );
//#line "actors.c" 7
extern void updateinterpolations(void);
//#line "actors.c" 15
extern void setinterpolation(int32_t *posptr);
//#line "actors.c" 27
extern void stopinterpolation(int32_t *posptr);
//#line "actors.c" 41
extern void dointerpolations(int32_t smoothratio);
//#line "actors.c" 55
extern void restoreinterpolations(void);
//#line "actors.c" 63
extern int32_t ceilingspace(short sectnum);
//#line "actors.c" 77
extern int32_t floorspace(short sectnum);
//#line "actors.c" 91
extern void addammo(short weapon,struct player_struct *p,short amount);
//#line "actors.c" 99
extern void addweapon(struct player_struct *p,short weapon);
//#line "actors.c" 132
extern void checkavailinven(struct player_struct *p);
//#line "actors.c" 152
extern void checkavailweapon(struct player_struct *p);
//#line "actors.c" 301
extern int32_t ifsquished(short i,short p);
//#line "actors.c" 326
extern void hitradius(short i,int32_t r,int32_t hp1,int32_t hp2,int32_t hp3,int32_t hp4);
//#line "actors.c" 556
extern int movesprite(short spritenum,int32_t xchange,int32_t ychange,int32_t zchange,uint32_t cliptype);
//#line "actors.c" 643
extern short ssp(short i,uint32_t cliptype);
//#line "actors.c" 658
extern void insertspriteq(short i);
//#line "actors.c" 666
extern void lotsofmoney(spritetype *s,short n);
//#line "actors.c" 676
extern void guts(spritetype *s,short gtype,short n,short p);
//#line "actors.c" 718
extern void setsectinterpolate(short i);
//#line "actors.c" 742
extern void clearsectinterpolate(short i);
//#line "actors.c" 760
extern void ms(short i);
//#line "actors.c" 791
extern void movefta(void );
//#line "actors.c" 882
extern short ifhitsectors(short sectnum);
//#line "actors.c" 896
extern short ifhitbyweapon(short sn);
//#line "actors.c" 970
extern void movecyclers(void );
//#line "actors.c" 1007
extern void movedummyplayers(void );
//#line "actors.c" 1053
extern void moveplayers(void );
//#line "actors.c" 1192
extern void movefx(void );
//#line "actors.c" 1294
extern void movefallers(void );
//#line "actors.c" 1389
extern void movestandables(void );
//#line "actors.c" 2234
extern void bounce(short i);
//#line "actors.c" 2273
extern void moveweapons(void );
//#line "actors.c" 2613
extern void movetransports(void );
//#line "actors.c" 2887
extern void moveeffectors(void );
//#line "actors.c" 4840
extern void moveactors(void );
//#line "actors.c" 6005
extern void moveexplosions(void );

#endif
