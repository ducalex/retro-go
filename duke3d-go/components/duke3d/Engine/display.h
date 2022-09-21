/*
 * "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
 * Ken Silverman's official web site: "http://www.advsys.net/ken"
 * See the included license file "BUILDLIC.TXT" for license info.
 * This file IS NOT A PART OF Ken Silverman's original release
 */

#ifndef _INCLUDE_DISPLAY_H_
#define _INCLUDE_DISPLAY_H_

#include "platform.h"

#include "esp_attr.h"

/* set these in your _platform_init() implementation. */
int _argc;
char  **_argv;

/* !!! gads, look at all the namespace polution... */
extern int32_t xres, yres, bytesperline, imageSize, maxpages;
extern uint8_t  *screen, vesachecked;
extern int32_t buffermode, origbuffermode, linearmode;
extern uint8_t  permanentupdate, vgacompatible;
extern uint8_t  moustat;
extern int32_t *horizlookup, *horizlookup2, horizycent;
extern int32_t oxdimen, oviewingrange, oxyaspect;
extern int32_t curbrightness;
extern int32_t qsetmode;
extern int32_t  pageoffset, ydim16;
extern uint8_t* frameplace;
extern uint8_t* frameoffset;
extern EXT_RAM_ATTR uint8_t  textfont[1024], smalltextfont[1024];
extern uint8_t  pow2char[8];
extern int32_t stereomode, visualpage, activepage, whiteband, blackband;
extern int32_t searchx, searchy;
extern int32_t wx1, wy1, wx2, wy2, ydimen;
extern int32_t xdimen, xdimenrecip, halfxdimen, xdimenscale, xdimscale;

/*
 * !!! used to be static. If we ever put the original setgamemode() back, this
 * !!! can be made static again.  --ryan.
 * !!! (Actually, most of these extern declarations used to be static...rcg.)
 */
extern uint8_t  permanentlock;

// defined in the game project:
extern int32_t BFullScreen;  // defined in Game\src\config.c
extern int32_t ScreenMode;


/* these need to be implemented by YOUR driver. */
void _platform_init(int argc, char  **argv, const char  *title, const char  *iconName);
void _idle(void);
void _handle_events(void);
void *_getVideoBase(void);
void initkeys(void);
void uninitkeys(void);
void _nextpage(void);
void _uninitengine(void);
void _joystick_init(void);
void _joystick_deinit(void);
int _joystick_update(void);
int _joystick_axis(int axis);
int _joystick_hat(int hat);
int _joystick_button(int button);

void getvalidvesamodes(void);
int VBE_getPalette(int32_t start, int32_t num, uint8_t  *dapal);
int VBE_setPalette(uint8_t  *palettebuffer);


void setvmode(int mode);
uint8_t  readpixel(uint8_t  * location);
void drawpixel(uint8_t  * location, uint8_t pixel);


void drawpixel16(int32_t offset);
void fillscreen16 (int32_t input1, int32_t input2, int32_t input3);


void clear2dscreen(void);
void _updateScreenRect(int32_t x, int32_t y, int32_t w, int32_t h);

/* mouse/keystuff stuff. Also implemented in sdl_driver.c ... */
int setupmouse(void);
void readmousexy(short *x, short *y);
void readmousebstatus(short *bstatus);
void keyhandler(void);
uint8_t  _readlastkeyhit(void);

/* timer krap. */
int inittimer(int);
void uninittimer(void);

/* this is implemented in the game, and your driver calls it. */
void timerhandler(void);

/* resolution inits. sdl_driver.c ... */
int32_t _setgamemode(uint8_t  davidoption, int32_t daxdim, int32_t daydim);

uint32_t getticks();

void drawline16(int32_t XStart, int32_t YStart, int32_t XEnd, int32_t YEnd, uint8_t  Color);
void setcolor16(uint8_t color);



void initmultiplayers(uint8_t  damultioption, uint8_t  dacomrateoption, uint8_t  dapriority);

#endif


