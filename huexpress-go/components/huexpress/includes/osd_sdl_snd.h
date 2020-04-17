#ifndef _INCLUDE_OSD_SND_H
#define _INCLUDE_OSD_SND_H

#include "cleantypes.h"
#include "sys_snd.h"
#include "sound.h"

#if 0
#include <SDL.h>
#include <SDL_mixer.h>

extern Uint8 *stream;
extern Mix_Chunk *chunk;
extern SDL_AudioCVT cvt;
extern int Callback_Stop;
extern int USE_S16;
#endif

extern int osd_sound_init();

#endif
