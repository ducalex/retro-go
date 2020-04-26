/****************************************************************************
 dis.h

 Data for TG Disassembler
 ****************************************************************************/
#ifndef _INCLUDE_DIS_H
#define _INCLUDE_DIS_H

#include "format.h"
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

#endif
