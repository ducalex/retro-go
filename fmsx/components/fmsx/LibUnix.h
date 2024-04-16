/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        LibUnix.h                        **/
/**                                                         **/
/** This file contains Unix-dependent definitions and       **/
/** declarations for the emulation library.                 **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2021                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef LIBUNIX_H
#define LIBUNIX_H

#define XImage void

#define SND_CHANNELS    16     /* Number of sound channels   */
#define SND_BITS        8
#define SND_BUFSIZE     (1<<SND_BITS)

/** PIXEL() **************************************************/
/** Unix may use multiple pixel formats.                    **/
/*************************************************************/
#if defined(BPP32) || defined(BPP24)
#define PIXEL(R,G,B)  (pixel)(((int)R<<16)|((int)G<<8)|B)
#define PIXEL2MONO(P) (((P>>16)&0xFF)+((P>>8)&0xFF)+(P&0xFF))/3)
#define RMASK 0xFF0000
#define GMASK 0x00FF00
#define BMASK 0x0000FF

#elif defined(BPP16)
#define PIXEL(R,G,B)  (pixel)(((31*(R)/255)<<11)|((63*(G)/255)<<5)|(31*(B)/255))
#define PIXEL2MONO(P) (522*(((P)&31)+(((P)>>5)&63)+(((P)>>11)&31))>>8)
#define RMASK 0xF800
#define GMASK 0x07E0
#define BMASK 0x001F

#elif defined(BPP8)
#define PIXEL(R,G,B)  (pixel)(((7*(R)/255)<<5)|((7*(G)/255)<<2)|(3*(B)/255))
#define PIXEL2MONO(P) (3264*((((P)<<1)&7)+(((P)>>2)&7)+(((P)>>5)&7))>>8)
#define RMASK 0xE0
#define GMASK 0x1C
#define BMASK 0x03
#endif

unsigned int InitAudio(unsigned int Rate,unsigned int Latency);
void TrashAudio(void);
int PauseAudio(int Switch);

#endif /* LIBUNIX_H */
