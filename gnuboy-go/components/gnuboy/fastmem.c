

#include "fastmem.h"

#include <esp_attr.h>

#define D 0 /* direct */
#define C 1 /* direct cgb-only */
#define R 2 /* io register */
#define S 3 /* sound register */
#define W 4 /* wave pattern */

#define F 0xFF /* fail */

//DRAM_ATTR const byte himask[256];

const byte DRAM_ATTR hi_rmap[256] =
{
	0, 0, R, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S,
	S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, C, 0, C,
	0, C, C, C, C, C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, C, C, C, C, 0, 0, 0, 0,
	C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const byte DRAM_ATTR hi_wmap[256] =
{
	R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R,
	S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S,
	S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S,
	S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S,
	R, R, R, R, R, R, R, R, R, R, R, R, 0, R, 0, R,
	0, C, C, C, C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, R, R, R, R, 0, 0, 0, 0,
	R, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, R
};


void sound_write();
static void no_write()
{
}
