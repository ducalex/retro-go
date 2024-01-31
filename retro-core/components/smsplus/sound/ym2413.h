#if 0
#ifndef _H_YM2413_
#define _H_YM2413_

/* select output bits size of output : 8 or 16 */
#define SAMPLE_BITS 16

/* compiler dependence */
#ifndef OSD_CPU_H
#define OSD_CPU_H
typedef unsigned char   UINT8;   /* unsigned  8bit */
typedef unsigned short  UINT16;  /* unsigned 16bit */
typedef unsigned int    UINT32;  /* unsigned 32bit */
typedef signed char     INT8;    /* signed  8bit   */
typedef signed short    INT16;   /* signed 16bit   */
typedef signed int      INT32;   /* signed 32bit   */
#endif

#if (SAMPLE_BITS==16)
typedef INT16 SAMP;
#endif
#if (SAMPLE_BITS==8)
typedef INT8 SAMP;
#endif

int  YM2413Init(int num, int clock, int rate);
void YM2413Shutdown(void);
void YM2413ResetChip(int which);
void YM2413Write(int which, int a, int v);
unsigned char YM2413Read(int which, int a);
void YM2413UpdateOne(int which, INT16 **buffers, int length);

typedef void (*OPLL_UPDATEHANDLER)(int param,int min_interval_us);
void YM2413SetUpdateHandler(int which, OPLL_UPDATEHANDLER UpdateHandler, int param);


#endif /*_H_YM2413_*/
#endif
