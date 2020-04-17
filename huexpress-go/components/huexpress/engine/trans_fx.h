/*
########################################
####  WILL MOVE IN THE GFX DLL/SO
########################################
*/


/*************************************************************************/
/*	 								 */
/*                   TRANSitions_FX.H                                    */
/*                                                                       */
/*     Some little transitions to make things appear 'cuter'             */
/*                                                                       */
/*     If you have any idea for 'not-so-hard-to-program-nor-too-slow' FX */
/*     then do it or tell someone (e.g. ME [Zeograd]) to do so ;)        */
/*                                                                       */
/*************************************************************************/

#ifndef _TRANS_FX_H_
#define _TRANS_FX_H_


#include <string.h>
#include "cleantypes.h"
#include "pce.h"

extern const char nb_fadein;
extern const char nb_fadeout;
// used to know how many functions we have

extern void (*fade_in_proc[5]) (uchar *, unsigned, unsigned, unsigned,
								unsigned);

extern void (*fade_out_proc[5]) (unsigned, unsigned, unsigned, unsigned);


#endif
