#ifndef _RG_SDL_MIXER_H_
#define _RG_SDL_MIXER_H_

#include "SDL.h"

typedef struct _Mix_Music Mix_Music;
typedef struct _Mix_Chunk Mix_Chunk;

#define MIX_MAX_VOLUME 128
#define MIX_CHANNELS 8
#define MIX_FADING_OUT 1

int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
void Mix_CloseAudio(void);
int Mix_QuerySpec(int *frequency, Uint16 *format, int *channels);

Mix_Music *Mix_LoadMUS(const char *file);
void Mix_FreeMusic(Mix_Music *music);
int Mix_PlayMusic(Mix_Music *music, int loops);
int Mix_HaltMusic(void);
int Mix_VolumeMusic(int volume);
int Mix_PauseMusic(void);
int Mix_ResumeMusic(void);
int Mix_PausedMusic(void);
int Mix_PlayingMusic(void);
int Mix_FadeOutMusic(int ms);
int Mix_FadingMusic(void);

Mix_Chunk *Mix_LoadWAV(const char *file);
void Mix_FreeChunk(Mix_Chunk *chunk);
int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops);
int Mix_HaltChannel(int channel);
int Mix_Volume(int channel, int volume);

#endif
