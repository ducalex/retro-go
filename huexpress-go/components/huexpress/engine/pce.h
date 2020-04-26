// pce.h
#ifndef _INCLUDE_PCE_H
#define _INCLUDE_PCE_H

#include "config.h"

#include "odroid_system.h"

#include "sys_dep.h"
#include "hard_pce.h"
#include "debug.h"
#include "gfx.h"

#define WIDTH   (360+64)
#define HEIGHT  256

#include "cleantypes.h"

#include "h6280.h"
#include "interupt.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define SERVER_HOSTNAME_MAX_SIZE 256

int LoadCard(char *name);
int LoadState(char *name);
int SaveState(char *name);
int ResetPCE();
void SetPalette(void);

int InitPCE(char *name);
void TrashPCE();
int RunPCE(void);

extern char volatile key_delay;
// are we allowed to press another 'COMMAND' key ?

extern char cart_name[PCE_PATH_MAX];
// the name of the rom to load

extern char *server_hostname;

extern int BaseClock, UPeriod;

extern uchar debug_on_beginning;
// Do we have to set a bp on the reset IP

struct host_sound {
	int stereo;
	int signed_sound;
	uint32 freq;
	uint16 sample_size;
};

struct host_machine {
	struct host_sound sound;
	bool want_fullscreen_aspect;
	uchar video_driver;
};

extern struct host_machine host;

extern int scroll;

#if !defined(MIN)
#define MIN(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define MAX(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })
#endif

// Video related defines

#define SpHitON    (IO_VDC_05_CR.W&0x01)
#define OverON     (IO_VDC_05_CR.W&0x02)
#define RasHitON   (IO_VDC_05_CR.W&0x04)
#define VBlankON   (IO_VDC_05_CR.W&0x08)
#define SpriteON   (IO_VDC_05_CR.W&0x40)
#define ScreenON   (IO_VDC_05_CR.W&0x80)

#define VDC_CR     0x01
#define VDC_OR     0x02
#define VDC_RR     0x04
#define VDC_DS     0x08
#define VDC_DV     0x10
#define VDC_VD     0x20
#define VDC_BSY    0x40
#define VDC_SpHit       VDC_CR
#define VDC_Over        VDC_OR
#define VDC_RasHit      VDC_RR
#define VDC_InVBlank    VDC_VD
#define VDC_DMAfinish   VDC_DV
#define VDC_SATBfinish  VDC_DS

#define SATBIntON (IO_VDC_0F_DCR.W&0x01)
#define DMAIntON  (IO_VDC_0F_DCR.W&0x02)

#define IRQ2    1
#define IRQ1    2
#define TIRQ    4

// Joystick related defines
#ifdef _DO_NOT_DEFINE_
#define J_UP      0
#define J_DOWN    1
#define J_LEFT    2
#define J_RIGHT   3
#define J_I       4
#define J_II      5
#define J_SELECT  6
#define J_START   7
#define J_AUTOI   8
#define J_AUTOII  9
#define J_PI      10
#define J_PII     11
#define J_PSELECT 12
#define J_PSTART  13
#define J_PAUTOI  14
#define J_PAUTOII 15
#define J_PXAXIS  16
#define J_PYAXIS  17
#define J_MAX     18
#endif

// Post include to avoid circular definitions

#include "romdb.h"

#include "config.h"

#include "sprite.h"

#define IO_VDC_00_MAWR  io.VDC[MAWR]
#define IO_VDC_01_MARR  io.VDC[MARR]
#define IO_VDC_02_VWR   io.VDC[VWR]
#define IO_VDC_03_vdc3  io.VDC[vdc3]
#define IO_VDC_04_vdc4  io.VDC[vdc4]
#define IO_VDC_05_CR    io.VDC[CR]
#define IO_VDC_06_RCR   io.VDC[RCR]
#define IO_VDC_07_BXR   io.VDC[BXR]
#define IO_VDC_08_BYR   io.VDC[BYR]
#define IO_VDC_09_MWR   io.VDC[MWR]
#define IO_VDC_0A_HSR   io.VDC[HSR]
#define IO_VDC_0B_HDR   io.VDC[HDR]
#define IO_VDC_0C_VPR   io.VDC[VPR]
#define IO_VDC_0D_VDW   io.VDC[VDW]
#define IO_VDC_0E_VCR   io.VDC[VCR]
#define IO_VDC_0F_DCR   io.VDC[DCR]
#define IO_VDC_10_SOUR  io.VDC[SOUR]
#define IO_VDC_11_DISTR io.VDC[DISTR]
#define IO_VDC_12_LENR  io.VDC[LENR]
#define IO_VDC_13_SATB  io.VDC[SATB]
#define IO_VDC_14       io.VDC[0x14]

#define IO_VDC_active io.VDC[io.vdc_reg]

#endif
