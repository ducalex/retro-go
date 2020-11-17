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
#include <esp_attr.h>
#include <math.h>

/* Implemented by the port */
extern void sys_vsync(void);
extern void sys_panic(char *);

/* emu.c */
void emu_init();
void emu_reset();
void emu_run(_Bool draw);
void emu_die(const char *fmt, ...);

/* debug.c */
void debug_disassemble(addr a, int c);


#endif
