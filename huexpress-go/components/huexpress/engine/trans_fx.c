/*
########################################
####  WILL MOVE IN THE GFX DLL/SO
########################################
*/


#include "trans_fx.h"


const char nb_fadein = 5;
const char nb_fadeout = 5;
// used to know how many functions do we have


void (*fade_in_proc[5]) (uchar *, unsigned, unsigned, unsigned, unsigned)
	= {
};

// Array of function for random calls

void (*fade_out_proc[5]) (unsigned, unsigned, unsigned, unsigned)
	= {
};
