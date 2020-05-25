#ifndef __DEFS_H__
#define __DEFS_H__


typedef unsigned char byte;
typedef unsigned char un8;
typedef unsigned short un16;
typedef unsigned int un32;
typedef signed char n8;
typedef signed short n16;
typedef signed int n32;
typedef un16 word;
typedef word addr;


#ifdef IS_LITTLE_ENDIAN
#define LO 0
#define HI 1
#else
#define LO 1
#define HI 0
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <math.h>

void sys_sleep(int us);
void *sys_timer();
int  sys_elapsed(void *in_ptr);

void doevents();

/* emu.c */
void emu_reset();
void emu_run();
void emu_step();
void emu_die(const char *fmt, ...);

/* save.c */
void savestate(FILE *f);
void loadstate(FILE *f);

/* debug.c */
void debug_disassemble(addr a, int c);


#endif
