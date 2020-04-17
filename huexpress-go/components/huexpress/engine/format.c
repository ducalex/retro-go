/****************************************************************************
   format.c -- Joseph LoCicero, IV -- 22.2.96
               Dave Shadoff        -- 20.3.96
					Zeograd             -- 26.5.99

   line-formatting routines for disassembler
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "format.h"


/* lineprinters, to keep code clean. */
/*@ -bufferoverflowhigh */

void
lineprint1(char *outf, long ctr, uchar * op, char *outstring)
{
	sprintf(outf, "%02X          %s", *op, outstring);
}

void
lineprint2(char *outf, long ctr, uchar * op, char *outstring)
{
	sprintf(outf, "%02X %02X       %s", *op, *(op + 1), outstring);
}

void
lineprint3(char *outf, long ctr, uchar * op, char *outstring)
{
	sprintf(outf, "%02X %02X %02X    %s", *op, *(op + 1), *(op + 2),
			outstring);
}

void
lineprint4(char *outf, long ctr, uchar * op, char *outstring)
{
	sprintf(outf, "%02X %02X %02X %02X %s", *op, *(op + 1), *(op + 2),
			*(op + 3), outstring);
}

void
lineprint7(char *outf, long ctr, uchar * op, char *outstring)
{
	sprintf(outf, "%02X %02X %02X %02X %02X %02X %02X %s",
			*op, *(op + 1), *(op + 2), *(op + 3), *(op + 4),
			*(op + 5), *(op + 6), outstring);
}


/* common addressing-mode formatters */
/* look/act as wrappers around lineprint functions */

void
implicit(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s", str);
	lineprint1(outf, ctr, op, buf);
}

void
immed(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s #$%02X", str, *(op + 1));
	lineprint2(outf, ctr, op, buf);
}

void
relative(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	long newadd;
	int offset = (int) *(op + 1);


	if (offset >= 128)
		offset -= 256;

	newadd = (ctr + 2) + offset;

	sprintf(buf, "%-4s $%04lX", str, (unsigned long) newadd);
	lineprint2(outf, ctr, op, buf);
}

void
ind_zp(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s $%02X", str, *(op + 1));
	lineprint2(outf, ctr, op, buf);
}

void
ind_zpx(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s $%02X,X", str, *(op + 1));
	lineprint2(outf, ctr, op, buf);
}

void
ind_zpy(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s $%02X,Y", str, *(op + 1));
	lineprint2(outf, ctr, op, buf);
}

void
ind_zpind(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s ($%02X)", str, *(op + 1));
	lineprint2(outf, ctr, op, buf);
}

void
ind_zpix(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s ($%02X,X)", str, *(op + 1));
	lineprint2(outf, ctr, op, buf);
}

void
ind_zpiy(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s ($%02X),Y", str, *(op + 1));
	lineprint2(outf, ctr, op, buf);
}

void
absol(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s $%02X%02X", str, *(op + 2), *(op + 1));
	lineprint3(outf, ctr, op, buf);
}

void
absind(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s ($%02X%02X)", str, *(op + 2), *(op + 1));
	lineprint3(outf, ctr, op, buf);
}

void
absindx(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s ($%02X%02X,X)", str, *(op + 2), *(op + 1));
	lineprint3(outf, ctr, op, buf);
}

void
absx(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s $%02X%02X,X", str, *(op + 2), *(op + 1));
	lineprint3(outf, ctr, op, buf);
}

void
absy(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	sprintf(buf, "%-4s $%02X%02X,Y", str, *(op + 2), *(op + 1));
	lineprint3(outf, ctr, op, buf);
}

void
pseudorel(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];
	long newadd;
	int offset = (int) *(op + 2);

	if (offset >= 128)
		offset -= 256;

	newadd = (ctr + 3) + offset;

	sprintf(buf, "%-4s $%02X, $%04lX", str, *(op + 1),
			(unsigned long) newadd);
	lineprint3(outf, ctr, op, buf);
}

void
tst_zp(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];

	sprintf(buf, "%-4s $%02X, $%02X", str, *(op + 1), *(op + 2));
	lineprint3(outf, ctr, op, buf);
}

void
tst_abs(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];

	sprintf(buf, "%-4s $%02X, $%02X%02X", str, *(op + 1), *(op + 3),
			*(op + 2));
	lineprint4(outf, ctr, op, buf);
}

void
tst_zpx(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];

	sprintf(buf, "%-4s $%02X, $%02X", str, *(op + 1), *(op + 2));
	lineprint3(outf, ctr, op, buf);
}

void
tst_absx(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];

	sprintf(buf, "%-4s $%02X, $%02X%02X", str, *(op + 1), *(op + 3),
			*(op + 2));
	lineprint4(outf, ctr, op, buf);
}

void
xfer(char *outf, long ctr, uchar * op, char *str)
{
	char buf[256];

	sprintf(buf, "%-4s $%02X%02X, $%02X%02X, $%02X%02X",
			str, *(op + 2), *(op + 1), *(op + 4), *(op + 3), *(op + 6),
			*(op + 5));
	lineprint7(outf, ctr, op, buf);
}

/*@ =bufferoverflowhigh */
