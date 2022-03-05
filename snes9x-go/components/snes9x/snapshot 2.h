/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SNAPSHOT_H_
#define _SNAPSHOT_H_

#include "snes9x.h"

enum
{
	SUCCESS = 1,
	WRONG_FORMAT = -1,
	WRONG_VERSION = -2,
	FILE_NOT_FOUND = -3,
};

int S9xFreezeGame (const char *);
int S9xUnfreezeGame (const char *);

#endif
