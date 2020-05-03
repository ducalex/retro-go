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

extern uchar running_mode;

int disassemble();

// Break points

// Print format
extern void implicit(char *, long, uchar *, char *);
extern void immed(char *, long, uchar *, char *);
extern void relative(char *, long, uchar *, char *);
extern void ind_zp(char *, long, uchar *, char *);
extern void ind_zpx(char *, long, uchar *, char *);
extern void ind_zpy(char *, long, uchar *, char *);
extern void ind_zpind(char *, long, uchar *, char *);
extern void ind_zpix(char *, long, uchar *, char *);
extern void ind_zpiy(char *, long, uchar *, char *);
extern void absol(char *, long, uchar *, char *);
extern void absx(char *, long, uchar *, char *);
extern void absy(char *, long, uchar *, char *);
extern void absind(char *, long, uchar *, char *);
extern void absindx(char *, long, uchar *, char *);
extern void pseudorel(char *, long, uchar *, char *);
extern void tst_zp(char *, long, uchar *, char *);
extern void tst_abs(char *, long, uchar *, char *);
extern void tst_zpx(char *, long, uchar *, char *);
extern void tst_absx(char *, long, uchar *, char *);
extern void xfer(char *, long, uchar *, char *);

#endif
