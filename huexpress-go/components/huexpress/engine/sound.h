#ifndef _SOUND_H_
#define _SOUND_H_


#include "pce.h"
#include "debug.h"


extern uchar sound_driver;


#include "cleantypes.h"

#define SOUND_BUF_MS	200
// I think it's too long but for a beginning it'll be enough

//#define SBUF_SIZE_BYTE  44100*SOUND_BUF_MS/1000
// Calculated for mono sound
#define SBUF_SIZE_BYTE 1024*8


extern uchar *big_buf;

extern uchar gen_vol;

extern uint32 sbuf_size;

extern char *sbuf[6];			//[SBUF_SIZE_BYTE];
// the buffer where the "DATA-TO-SEND-TO-THE-SOUND-CARD" go
// one for each channel

extern char *adpcmbuf;

extern uchar new_adpcm_play;
// Have we begun a new adpcm sample (i.e. must be reset adpcm index/prev value)

#ifndef SDL
extern char main_buf[SBUF_SIZE_BYTE];
// the mixed buffer, may be removed later for hard mixing...
#else
extern uchar main_buf[SBUF_SIZE_BYTE];
// the mixed buffer, may be removed later for hard mixing...
#endif

//extern uint32 CycleOld;
//extern uint32 CycleNew;
// indicates the last time music has been "released"

extern uint32 dwNewPos;

extern uint32 AdpcmFilledBuf;

int InitSound(void);
void TrashSound(void);
void write_psg(int ch);
void WriteBuffer(char *, int, unsigned);

void write_adpcm(void);
void dump_audio_chunck(uchar * content, int length);

int start_dump_audio(void);
void stop_dump_audio(void);


#endif							// HDEF_SOUND_H
