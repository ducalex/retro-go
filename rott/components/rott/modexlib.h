#ifndef MODEXLIB_H
#define MODEXLIB_H

#include "rt_def.h"

// Resolution macros to 320x200 for Retro-Go port
#define MAXSCREENHEIGHT    200
#define MAXSCREENWIDTH     320
#define SCREENBWIDE        80
#define MAXVIEWWIDTH       320
#define SCREENWIDTH        80

//***************************************************************************
//
//    Video (ModeX) Constants & Macros (Stubbed for Retro-Go Linear Buffer)
//
//***************************************************************************

#define VGAWRITEMAP(x)
#define VGAMAPMASK(x)
#define VGAREADMAP(x)

#ifdef DOS
#define SC_INDEX                0x3C4
#define SC_DATA                 0x3C5
#define SC_MAPMASK              2
#define SC_ALLPLANES            0x0F02

#define GC_INDEX                0x3CE
#define GC_DATA                 0x3CF
#define GC_READMAP              4

#define ATR_INDEX               0x3C0
#define ATR_DATA                0x3C1

#define PEL_WRITE_ADR           0x3C8
#define PEL_READ_ADR            0x3C7
#define PEL_DATA                0x3C9

#define STATUS_REGISTER_1       0x3DA
#endif

//========================================

#define SCREENSIZE              (MAXSCREENWIDTH*MAXSCREENHEIGHT)

extern int linewidth;
extern int screensize;
extern byte *displayofs, *bufferofs;
extern byte *page1start, *page2start, *page3start;
extern int *ylookup;

extern boolean StretchScreen;
extern boolean ingame;

//========================================

void EnableScreenStretch(void);
void DisableScreenStretch(void);

#endif
