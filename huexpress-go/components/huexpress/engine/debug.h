#ifndef _INCLUDE_DEBUG_H
#define _INCLUDE_DEBUG_H

#include <stdio.h>

#include "config.h"
#include "pce.h"
#include "dis.h"
#include "optable.h"
#include "osd_gfx.h"

#define  MAX_BP        16
// Max number of breakpoints

#define  MAX_USER_BP   14
// Max number of breakpoints user definabled

#define  RESTORE_BP    14
// Place in the list of a pseudo-break point for internal usage

#define  GIVE_HAND_BP  15
// Place in the list of the breakpoint used in stepping...

// Some defines for the flag value
#define NOT_USED   0
#define ENABLED    1
#define DISABLED   2

#if ENABLE_TRACING
#define TRACE(x...) printf("DD " x)
#else
#define TRACE(x...)
#endif

#define MESSAGE_ERROR(x...) printf("!! " x)
#define MESSAGE_INFO(x...) printf(" * " x)

typedef struct {
	uint16 position;
	uchar flag;
	uchar original_op;
} Breakpoint;

extern uint16 Bp_pos_to_restore;
// Used in RESTORE_BP to know where we must restore a BP

extern Breakpoint Bp_list[MAX_BP];
// The list of all breakpoints

extern uchar save_background;
// Do we have to preserve the background

uchar Op6502(unsigned int A);

void disass_menu();
// Kind of front end for the true disassemble function

int toggle_user_breakpoint(uint16);
// Toggle the breakpoint at the specified address

void display_debug_help();
// Display a help screen

uint32 cvtnum(char *string);
// Convert a hexa string into a number

void set_bp_following(uint16 where, uchar nb);
// Set a bp at the location that follows a given address

uchar change_value(int X, int Y, uchar length, uint32 * result);
// Wait for user to enter a value and display in on screen

#endif
