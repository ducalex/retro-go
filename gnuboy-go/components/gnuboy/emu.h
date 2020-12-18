#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <rg_system.h>

#ifdef IS_LITTLE_ENDIAN
#define LO 0
#define HI 1
#else
#define LO 1
#define HI 0
#endif

typedef uint8_t byte;
typedef uint8_t un8;
typedef uint16_t un16;
typedef uint32_t un32;
typedef int8_t n8;
typedef int16_t n16;
typedef int32_t n32;
typedef uint16_t word;
typedef unsigned int addr_t; // Most efficient but at least 16 bits

/* Implemented by the port */
extern void sys_vsync(void);
extern void sys_panic(char *);

/* emu.c */
void emu_init();
void emu_reset();
void emu_run(bool draw);
void emu_die(const char *fmt, ...);

/* debug.c */
void debug_disassemble(addr_t a, int c);

#endif
