/****************************************************************************
 defs.h
 ****************************************************************************/

#ifndef DEFS_H_
#define DEFS_H_

#include "cleantypes.h"

//
// misc typedefs:
// --------------

//
//
//
typedef struct {
#ifdef BYTEORD_LSBFIRST
  uchar bl;
  uchar bh;
#else
  uchar bh;
  uchar bl;
#endif
} BytesOfWord;

typedef union {
  uint16       w;
  BytesOfWord  b;
} Word;

typedef struct {
#ifdef BYTEORD_LSBFIRST
  Word  wl;
  Word  wh;
#else
  Word  wh;
  Word  wl;
#endif
} WordsOfLong;

typedef union {
  uint32       i;
  WordsOfLong  w;
} LongWord;

//
// 'mode_struct' and 'operation' hold information about opcodes
//   (used in disassembly and execution)
//

typedef struct mode {
  int16 size;
  void (*func_format)(char *, long, uchar *, char *);
} mode_struct;


typedef struct op {
   int (*func_exe)(void);
   int16  addr_mode;
   const char * opname;
//   short int filler[3];   // force align to power-of-2 (?)
} operation;


#endif
