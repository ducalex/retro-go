/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SNAPSHOT_H_
#define _SNAPSHOT_H_

#include "snes9x.h"

#define SNAPSHOT_MAGIC			"#!s9x-rg"
#define SNAPSHOT_VERSION		11

#define SUCCESS					(1)
#define WRONG_FORMAT			(-1)
#define WRONG_VERSION			(-2)
#define FILE_NOT_FOUND			(-3)
#define WRONG_MOVIE_SNAPSHOT	(-4)
#define NOT_A_MOVIE_SNAPSHOT	(-5)
#define SNAPSHOT_INCONSISTENT	(-6)

// Snapshot Messages
#define SAVE_INFO_SNAPSHOT				"Saved"
#define SAVE_INFO_LOAD					"Loaded"
#define SAVE_INFO_OOPS					"Auto-saving 'oops' snapshot"
#define SAVE_ERR_WRONG_FORMAT			"File not in Snes9x snapshot format"
#define SAVE_ERR_WRONG_VERSION			"Incompatible snapshot version"
#define SAVE_ERR_ROM_NOT_FOUND			"ROM image \"%s\" for snapshot not found"
#define SAVE_ERR_SAVE_NOT_FOUND			"Snapshot %s does not exist"

bool8 S9xFreezeGame (const char *);
bool8 S9xUnfreezeGame (const char *);

#endif
