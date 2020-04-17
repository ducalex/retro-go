/****************************************************************************
 format.h
 Function protoypes for formatting routines
 ****************************************************************************/
#ifndef FORMAT_H_
#define FORMAT_H_


#include "cleantypes.h"


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
