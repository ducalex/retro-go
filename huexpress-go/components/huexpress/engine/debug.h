#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

#if ENABLE_TRACING
#define TRACE(x...) printf("DD " x)
#else
#define TRACE(x...)
#endif

#define  MAX_BP        16
// Max number of breakpoints

#define  MAX_USER_BP   14
// Max number of breakpoints user definabled

#define  RESTORE_BP    14
// Place in the list of a pseudo-break point for internal usage

#define  GIVE_HAND_BP  15
// Place in the list of the breakpoint used in stepping...

// Some defines for the flag value
#define BP_NOT_USED   0
#define BP_ENABLED    1
#define BP_DISABLED   2

typedef struct {
	uint16 position;
	uchar flag;
	uchar original_op;
} Breakpoint;

extern uint16 Bp_pos_to_restore;
// Used in RESTORE_BP to know where we must restore a BP

extern Breakpoint Bp_list[MAX_BP];
// The list of all breakpoints

int handle_bp0();
int handle_bp1();
int handle_bp2();
int handle_bp3();
int handle_bp4();
int handle_bp5();
int handle_bp6();
int handle_bp7();
int handle_bp8();
int handle_bp9();
int handle_bp10();
int handle_bp11();
int handle_bp12();
int handle_bp13();
int handle_bp14();
int handle_bp15();

#endif
