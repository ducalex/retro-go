/****************************************************************************
 dis.h

 Data for TG Disassembler
 ****************************************************************************/
#ifndef _INCLUDE_DIS_H
#define _INCLUDE_DIS_H

#include "hard_pce.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>

/* Some defines */

#define  _PC_ M.PC.W

#define  OPBUF_SIZE    7		/* max size of opcodes */
#define  MAX_TRY       5
#define  NB_LINE       20

#define PLAIN_RUN 0
#define STEPPING  1
#define TRACING   2

extern uint8_t running_mode;

int disassemble();

// Break points

// Print format
extern void implicit(char *, long, uint8_t *, char *);
extern void immed(char *, long, uint8_t *, char *);
extern void relative(char *, long, uint8_t *, char *);
extern void ind_zp(char *, long, uint8_t *, char *);
extern void ind_zpx(char *, long, uint8_t *, char *);
extern void ind_zpy(char *, long, uint8_t *, char *);
extern void ind_zpind(char *, long, uint8_t *, char *);
extern void ind_zpix(char *, long, uint8_t *, char *);
extern void ind_zpiy(char *, long, uint8_t *, char *);
extern void absol(char *, long, uint8_t *, char *);
extern void absx(char *, long, uint8_t *, char *);
extern void absy(char *, long, uint8_t *, char *);
extern void absind(char *, long, uint8_t *, char *);
extern void absindx(char *, long, uint8_t *, char *);
extern void pseudorel(char *, long, uint8_t *, char *);
extern void tst_zp(char *, long, uint8_t *, char *);
extern void tst_abs(char *, long, uint8_t *, char *);
extern void tst_zpx(char *, long, uint8_t *, char *);
extern void tst_absx(char *, long, uint8_t *, char *);
extern void xfer(char *, long, uint8_t *, char *);

#endif
