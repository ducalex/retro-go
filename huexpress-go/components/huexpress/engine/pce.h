// pce.h
#ifndef _INCLUDE_PCE_H
#define _INCLUDE_PCE_H

#include "config.h"

#include "sys_dep.h"
#include "hard_pce.h"
#include "debug.h"
#include "gfx.h"

#define WIDTH   (360+64)
#define HEIGHT  256

#include "cleantypes.h"

#include "h6280.h"
#include "globals.h"
#include "interupt.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


#define SERVER_HOSTNAME_MAX_SIZE 256

int CartLoad(char *name);
int ResetPCE();
int32 InitMachine(void);
void TrashMachine(void);
int32 Joysticks(void);
void delete_file_tmp(char *name, int dummy, int dummy2);
uchar TimerInt();

void init_log_file();
int InitPCE(char *name);
void TrashPCE();
int RunPCE(void);

void SetPalette(void);

extern FILE *out_snd;
// The file used to put sound into

extern char volatile key_delay;
// are we allowed to press another 'COMMAND' key ?

extern volatile uint32 message_delay;
// if different of zero, we must display the message pointed by pmessage

extern char short_cart_name[PCE_PATH_MAX];
// Just the filename without the extension (with a dot)
// you just have to add your own extension...

extern char cart_name[PCE_PATH_MAX];
// the name of the rom to load

extern uchar populus;

extern uchar *PopRAM;
// Now dynamicaly allocated
// ( size of popRAMsize bytes )
// If someone could explain me why we need it
// the version I have works well without this trick

extern const uint32 PopRAMsize;
// I don't really know if it must be 0x8000 or 0x10000

extern char *server_hostname;
// Name of the server to connect to

extern int32 smode;
// what sound card type should we use? (0 means the silent one,
// my favorite : the fastest!!! ; and -1 means AUTODETECT;
// later will avoid autodetection if wanted)

extern char silent;
// use sound?

extern int BaseClock, UPeriod;

extern uchar US_encoded_card;
// Do we have to swap even and odd bytes in the rom

extern uchar debug_on_beginning;
// Do we have to set a bp on the reset IP

extern uint32 timer_60;
// how many times do the interrupt have been called

extern uchar bcdbin[0x100];

extern uchar binbcd[0x100];

struct host_sound {
	int stereo;
	int signed_sound;
	uint32 freq;
	uint16 sample_size;
};

struct host_machine {
	struct host_sound sound;
};

extern struct host_machine host;

struct hugo_options {
	int want_fullscreen_aspect;
	int want_supergraphx_emulation;
#if defined(ENABLE_NETPLAY)
	netplay_type want_netplay;
	char server_hostname[SERVER_HOSTNAME_MAX_SIZE];
	uchar local_input_mapping[5];
#endif
};

extern struct hugo_options option;

extern uchar video_driver;

#if !defined(MIN)
#define MIN(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a < _b ? _a : _b; })
#define MAX(a,b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b);_a > _b ? _a : _b; })
#endif

// Video related defines

extern uchar can_write_debug;

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

#include <time.h>

#ifdef SOUND
#include "sound.h"
#endif							// SOUND

#ifdef MY_VDC_VARS

extern pair IO_VDC_00_MAWR ;
extern pair IO_VDC_01_MARR ;
extern pair IO_VDC_02_VWR  ;
extern pair IO_VDC_03_vdc3 ;
extern pair IO_VDC_04_vdc4 ;
extern pair IO_VDC_05_CR   ;
extern pair IO_VDC_06_RCR  ;
extern pair IO_VDC_07_BXR  ;
extern pair IO_VDC_08_BYR  ;
extern pair IO_VDC_09_MWR  ;
extern pair IO_VDC_0A_HSR  ;
extern pair IO_VDC_0B_HDR  ;
extern pair IO_VDC_0C_VPR  ;
extern pair IO_VDC_0D_VDW  ;
extern pair IO_VDC_0E_VCR  ;
extern pair IO_VDC_0F_DCR  ;
extern pair IO_VDC_10_SOUR ;
extern pair IO_VDC_11_DISTR;
extern pair IO_VDC_12_LENR ;
extern pair IO_VDC_13_SATB ;
extern pair IO_VDC_14      ;
extern pair *IO_VDC_active_ref;

#define IO_VDC_reset { \
    IO_VDC_00_MAWR.W=0; \
    IO_VDC_01_MARR.W=0; \
    IO_VDC_02_VWR.W=0; \
    IO_VDC_03_vdc3.W=0; \
    IO_VDC_04_vdc4.W=0; \
    IO_VDC_05_CR.W=0; \
    IO_VDC_06_RCR.W=0; \
    IO_VDC_07_BXR.W=0; \
    IO_VDC_08_BYR.W=0; \
    IO_VDC_09_MWR.W=0; \
    IO_VDC_0A_HSR.W=0; \
    IO_VDC_0B_HDR.W=0; \
    IO_VDC_0C_VPR.W=0; \
    IO_VDC_0D_VDW.W=0; \
    IO_VDC_0E_VCR.W=0; \
    IO_VDC_0F_DCR.W=0; \
    IO_VDC_10_SOUR.W=0; \
    IO_VDC_11_DISTR.W=0; \
    IO_VDC_12_LENR.W=0; \
    IO_VDC_13_SATB.W=0; \
    IO_VDC_14.W=0; \
    IO_VDC_active_ref = &IO_VDC_00_MAWR; \
    }

#define IO_VDC_active_set(value_) { \
    io.vdc_reg = value_; \
    switch(io.vdc_reg) { \
    case 0x00: IO_VDC_active_ref = &IO_VDC_00_MAWR; break; \
    case 0x01: IO_VDC_active_ref = &IO_VDC_01_MARR; break; \
    case 0x02: IO_VDC_active_ref = &IO_VDC_02_VWR; break; \
    case 0x03: IO_VDC_active_ref = &IO_VDC_03_vdc3; break; \
    case 0x04: IO_VDC_active_ref = &IO_VDC_04_vdc4; break; \
    case 0x05: IO_VDC_active_ref = &IO_VDC_05_CR; break; \
    case 0x06: IO_VDC_active_ref = &IO_VDC_06_RCR; break; \
    case 0x07: IO_VDC_active_ref = &IO_VDC_07_BXR; break; \
    case 0x08: IO_VDC_active_ref = &IO_VDC_08_BYR; break; \
    case 0x09: IO_VDC_active_ref = &IO_VDC_09_MWR; break; \
    case 0x0A: IO_VDC_active_ref = &IO_VDC_0A_HSR; break; \
    case 0x0B: IO_VDC_active_ref = &IO_VDC_0B_HDR; break; \
    case 0x0C: IO_VDC_active_ref = &IO_VDC_0C_VPR; break; \
    case 0x0D: IO_VDC_active_ref = &IO_VDC_0D_VDW; break; \
    case 0x0E: IO_VDC_active_ref = &IO_VDC_0E_VCR; break; \
    case 0x0F: IO_VDC_active_ref = &IO_VDC_0F_DCR; break; \
    case 0x10: IO_VDC_active_ref = &IO_VDC_10_SOUR; break; \
    case 0x11: IO_VDC_active_ref = &IO_VDC_11_DISTR; break; \
    case 0x12: IO_VDC_active_ref = &IO_VDC_12_LENR; break; \
    case 0x13: IO_VDC_active_ref = &IO_VDC_13_SATB; break; \
    case 0x14: IO_VDC_active_ref = &IO_VDC_14; break; \
    default: \
        printf("Reg invalid: 0x%0X\n", io.vdc_reg); \
        abort(); \
    } \
    }

#define IO_VDC_active (*IO_VDC_active_ref)

#else

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

#define IO_VDC_reset {}
#define IO_VDC_active_set(value_) io.vdc_reg = value_;
#define IO_VDC_active io.VDC[io.vdc_reg]

#endif

#endif
