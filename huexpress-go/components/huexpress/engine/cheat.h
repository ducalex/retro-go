#ifndef _INCLUDE_CHEAT_H
#define _INCLUDE_CHEAT_H


#include <stdio.h>
#include <unistd.h>

#include "pce.h"
#include "utils.h"


char pokebyte();
/* Change the value of a byte in the memory */

char searchbyte();
/* Search for a byte in memory */

int loadgame();
/* Load the progression */

int savegame();
/* Save the progression */

#define  MAX_FREEZED_VALUE   8

typedef struct {
	unsigned short position;
	uchar value;
} freezed_value;

extern freezed_value list_to_freeze[MAX_FREEZED_VALUE];
/* List of all the value to freeze */

extern uchar current_freezed_values;
/* Current number of values to freeze */

int freeze_value(void);

#endif
