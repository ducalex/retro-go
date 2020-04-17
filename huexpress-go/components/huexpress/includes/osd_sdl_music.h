#ifndef _SDL_MIXER_MUSIC_H
#define _SDL_MIXER_MUSIC_H

#if 0
#include <SDL_mixer.h>
  
#include "mix.h"
#include "osd_sdl_machine.h"
#include "osd_sdl_snd.h"

//#define MAX_SONGS 100	// there cannot be more tracks...
#define MAX_SONGS 100 // there cannot be more tracks...
extern Mix_Music *sdlmixmusic[MAX_SONGS];

void sdlmixer_fill_audio(int channel);

#endif // 0 

#endif
