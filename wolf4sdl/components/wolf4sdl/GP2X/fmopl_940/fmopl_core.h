#ifndef __FMOPL_H_
#define __FMOPL_H_

#define HAS_YM3812 1

/* --- select emulation chips --- */
#define BUILD_YM3812 (HAS_YM3812)
#define BUILD_YM3526 (HAS_YM3526)
#define BUILD_Y8950  (HAS_Y8950)

/* select output bits size of output : 8 or 16 */
#define OPL_SAMPLE_BITS 16

/* compiler dependence */
#ifndef OSD_CPU_H
#define OSD_CPU_H
typedef unsigned char  UINT8;   /* unsigned  8bit */
typedef unsigned short UINT16;  /* unsigned 16bit */
typedef unsigned int   UINT32;  /* unsigned 32bit */
typedef signed char    INT8;    /* signed  8bit   */
typedef signed short   INT16;   /* signed 16bit   */
typedef signed int     INT32;   /* signed 32bit   */
#endif

#if (OPL_SAMPLE_BITS==16)
typedef INT16 OPLSAMPLE;
#endif
#if (OPL_SAMPLE_BITS==8)
typedef INT8 OPLSAMPLE;
#endif

#endif
