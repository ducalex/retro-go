/*
 * A list of all symbols exported from engine.c for a game's use.
 *
 *  Put together by Ryan C. Gordon (icculus@clutteredmind.org)
 *
 * Please do NOT harrass Ken Silverman about any code modifications
 *  (including this file) to BUILD.
 */

/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file IS NOT A PART OF Ken Silverman's original release
 */

#ifndef _INCLUDE_ENGINE_H_
#define _INCLUDE_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define SUPERBUILD  /* don't touch this. */

/*
 * this must be implemented by every program that uses the BUILD engine.
 *  engine.c calls this function in several places.
 *  See Ken's test game (in game.c), and his editor (in bstub.c) for examples.
 */
void faketimerhandler(void);

/*
 * exported engine functions, in absolutely no particular order.
 *  See build.txt, build2.txt...and anything else with a .txt extention for
 *  information on using these functions that may or may not be accurate. :)
 */
int initmouse(void);
int setgamemode(uint8_t  davidoption, int32_t daxdim, int32_t daydim);
int getceilzofslope(int16_t sectnum, int32_t dax, int32_t day);
int getflorzofslope(int16_t sectnum, int32_t dax, int32_t day);
void getzsofslope(int16_t sectnum, int32_t dax, int32_t day, int32_t *ceilz, int32_t *florz);
void setaspect(int32_t daxrange, int32_t daaspect);
int insertsprite(int16_t sectnum, int16_t statnum);
void updatesector(int32_t x, int32_t y, int16_t *sectnum);
int lastwall(int16_t point);
void initspritelists(void);
int deletesprite(int16_t spritenum);
int insertspritesect(int16_t sectnum);
int deletespritesect(int16_t deleteme);
int deletespritestat (int16_t deleteme);
int insertspritestat(int16_t statnum);
int changespritesect(int16_t spritenum, int16_t newsectnum);
int changespritestat(int16_t spritenum, int16_t newstatnum);
void loadtile(int16_t tilenume);


void nextpage(void);
void drawrooms(int32_t daposx, int32_t daposy, int32_t daposz,int16_t daang, int32_t dahoriz, int16_t dacursectnum);
int loadboard(char  *filename, int32_t *daposx, int32_t *daposy,int32_t *daposz, int16_t *daang, int16_t *dacursectnum);
void drawmasks(void);
void printext256(int32_t xpos, int32_t ypos, int16_t col, int16_t backcol,char  name[82], uint8_t  fontsize);

void initengine(void);
void uninitengine(void);

int loadpics(char  *filename, char * gamedir);
int saveboard(char  *filename, int32_t *daposx, int32_t *daposy, int32_t *daposz,int16_t *daang, int16_t *dacursectnum);
void plotpixel(int32_t x, int32_t y, uint8_t  col);
uint8_t  getpixel(int32_t x, int32_t y);
void setbrightness(uint8_t  dabrightness, uint8_t  *dapal);
int screencapture(char  *filename, uint8_t  inverseit);
void getmousevalues(int16_t *mousx, int16_t *mousy, int16_t *bstatus);
int clipmove (int32_t *x, int32_t *y, int32_t *z, int16_t *sectnum, int32_t xvect,int32_t yvect, int32_t walldist, int32_t ceildist,int32_t flordist, uint32_t  cliptype);
void getzrange(int32_t x, int32_t y, int32_t z, int16_t sectnum,int32_t *ceilz, int32_t *ceilhit, int32_t *florz, int32_t *florhit,int32_t walldist, uint32_t  cliptype);
int getangle(int32_t xvect, int32_t yvect);
void alignceilslope(int16_t dasect, int32_t x, int32_t y, int32_t z);
void alignflorslope(int16_t dasect, int32_t x, int32_t y, int32_t z);
int hitscan(int32_t xs, int32_t ys, int32_t zs, int16_t sectnum,
            int32_t vx, int32_t vy, int32_t vz,
	        int16_t *hitsect, int16_t *hitwall, int16_t *hitsprite,
	        int32_t *hitx, int32_t *hity, int32_t *hitz, uint32_t  cliptype);
int inside (int32_t x, int32_t y, int16_t sectnum);
void setfirstwall(int16_t sectnum, int16_t newfirstwall);
void rotatepoint(int32_t xpivot, int32_t ypivot, int32_t x, int32_t y, int16_t daang,
			int32_t *x2, int32_t *y2);
int drawtilescreen(int32_t pictopleft, int32_t picbox);
void clearview(int32_t dacol);
void clearallviews(int32_t dacol);
void draw2dgrid(int32_t posxe, int32_t posye, int16_t ange, int32_t zoome,int16_t gride);
void draw2dscreen(int32_t posxe, int32_t posye, int16_t ange, int32_t zoome,int16_t gride);
int sectorofwall(int16_t theline);
int setsprite(int16_t spritenum, int32_t newx, int32_t newy, int32_t newz);
void dragpoint(int16_t pointhighlight, int32_t dax, int32_t day);
int ksqrt(int32_t num);
int loopnumofsector(int16_t sectnum, int16_t wallnum);
int cansee(int32_t x1, int32_t y1, int32_t z1, int16_t sect1,int32_t x2, int32_t y2, int32_t z2, int16_t sect2);
int lintersect(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2, int32_t x3, int32_t y3, int32_t x4, int32_t y4, int32_t *intx,int32_t *inty, int32_t *intz);
int rintersect(int32_t x1, int32_t y1, int32_t z1, int32_t vx, int32_t vy, int32_t vz,
               int32_t x3, int32_t y3, int32_t x4, int32_t y4, int32_t *intx,
               int32_t *inty, int32_t *intz);
uint8_t* allocatepermanenttile(int16_t tilenume, int32_t xsiz, int32_t ysiz);
void drawline256 (int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t  col);
void copytilepiece(int32_t tilenume1, int32_t sx1, int32_t sy1, int32_t xsiz, int32_t ysiz,
                   int32_t tilenume2, int32_t sx2, int32_t sy2);
int nextsectorneighborz(int16_t sectnum, int32_t thez,
                        int16_t topbottom, int16_t direction);
int neartag(int32_t xs, int32_t ys, int32_t zs, int16_t sectnum, int16_t ange,
            int16_t *neartagsector, int16_t *neartagwall, int16_t *neartagsprite,
            int32_t *neartaghitdist, int32_t neartagrange, uint8_t  tagsearch);
int pushmove(int32_t *x, int32_t *y, int32_t *z, int16_t *sectnum,
             int32_t walldist, int32_t ceildist, int32_t flordist,
             uint32_t  cliptype);

int krand(void);

void flushperms(void);
void rotatesprite(int32_t sx, int32_t sy, int32_t z, int16_t a, int16_t picnum,
                  int8_t dashade, uint8_t  dapalnum, uint8_t  dastat,
                  int32_t cx1, int32_t cy1, int32_t cx2, int32_t cy2);
void makepalookup(int32_t palnum, uint8_t  *remapbuf, int8_t r,
                  int8_t g, int8_t b, uint8_t  dastat);
void drawmapview(int32_t dax, int32_t day, int32_t zoome, int16_t ang);
void setview(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void setviewtotile(int16_t tilenume, int32_t xsiz, int32_t ysiz);
void setviewback(void);
void squarerotatetile(int16_t tilenume);
void preparemirror(int32_t dax, int32_t day, int32_t daz,
                   int16_t daang, int32_t dahoriz, int16_t dawall,
                   int16_t dasector, int32_t *tposx, int32_t *tposy,
                   short *tang);
void completemirror(void);
int clipinsidebox(int32_t x, int32_t y, int16_t wallnum, int32_t walldist);

#include "cache.h"
#include "filesystem.h"
#include "display.h"
#include "fixedPoint_math.h"
#include "tiles.h"
#include "network.h"
    
#include "esp_attr.h"

//Exported for tile module
    extern int32_t setviewcnt;
    extern int32_t bakvidoption[4];
    extern uint8_t* bakframeplace[4];
    extern int32_t bakxsiz[4], bakysiz[4];
    extern int32_t bakwindowx1[4], bakwindowy1[4];
    extern int32_t bakwindowx2[4], bakwindowy2[4];
    extern EXT_RAM_ATTR uint8_t  picsiz[MAXTILES], tilefilenum[MAXTILES];
    extern EXT_RAM_ATTR int16_t bakumost[MAXXDIM+1], bakdmost[MAXXDIM+1];
    extern int32_t numtilefiles, artfil , artfilnum, artfilplc;
    extern int32_t pow2long[32];
    extern int32_t artsize , cachesize ;
    extern EXT_RAM_ATTR int32_t tilefileoffs[MAXTILES];
    extern int32_t totalclocklock;

#ifdef __cplusplus
}
#endif

#endif
