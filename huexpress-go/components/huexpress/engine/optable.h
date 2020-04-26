#ifndef _DJGPP_INCLUDE_DIS_CST_H
#define _DJGPP_INCLUDE_DIS_CST_H

#include "cleantypes.h"
#include "followop.h"

/* addressing modes: */

#define AM_IMPL      0			/* implicit              */
#define AM_IMMED     1			/* immediate             */
#define AM_REL       2			/* relative              */
#define AM_ZP        3			/* zero page             */
#define AM_ZPX       4			/* zero page, x          */
#define AM_ZPY       5			/* zero page, y          */
#define AM_ZPIND     6			/* zero page indirect    */
#define AM_ZPINDX    7			/* zero page indirect, x */
#define AM_ZPINDY    8			/* zero page indirect, y */
#define AM_ABS       9			/* absolute              */
#define AM_ABSX     10			/* absolute, x           */
#define AM_ABSY     11			/* absolute, y           */
#define AM_ABSIND   12			/* absolute indirect     */
#define AM_ABSINDX  13			/* absolute indirect     */
#define AM_PSREL    14			/* pseudo-relative       */
#define AM_TST_ZP   15			/* special 'TST' addressing mode  */
#define AM_TST_ABS  16			/* special 'TST' addressing mode  */
#define AM_TST_ZPX  17			/* special 'TST' addressing mode  */
#define AM_TST_ABSX 18			/* special 'TST' addressing mode  */
#define AM_XFER     19			/* special 7-byte transfer addressing mode  */

#define MAX_MODES  (AM_XFER + 1)

/* addressing mode information: */

typedef struct mode_debug {
	uchar size;
	void (*func) (char *, long int, uchar *, char *);
} mode_struct_debug;

/* now define table contents: */
typedef struct op_debug {
	int addr_mode;
	const char *opname;
	 uint16(*following_IP) (uint16);
} operation_debug;

extern const operation_debug optable_debug[256];

extern const mode_struct_debug addr_info_debug[MAX_MODES];

#endif
