#ifdef MODPLUG_MUSIC

#include "modplug.h"
#include "SDL_rwops.h"
#include "SDL_audio.h"
#include "SDL_mixer.h"

typedef struct {
	ModPlugFile *file;
	int playing;
} modplug_data;

int modplug_init(SDL_AudioSpec *mixer);

/* Uninitialize the music players */
void modplug_exit(void);

/* Set the volume for a modplug stream */
void modplug_setvolume(modplug_data *music, int volume);

/* Load a modplug stream from an SDL_RWops object */
modplug_data *modplug_new_RW(SDL_RWops *rw, int freerw);

/* Start playback of a given modplug stream */
void modplug_play(modplug_data *music);

/* Return non-zero if a stream is currently playing */
int modplug_playing(modplug_data *music);

/* Play some of a stream previously started with modplug_play() */
int modplug_playAudio(modplug_data *music, Uint8 *stream, int len);

/* Stop playback of a stream previously started with modplug_play() */
void modplug_stop(modplug_data *music);

/* Close the given modplug stream */
void modplug_delete(modplug_data *music);

/* Jump (seek) to a given position (time is in seconds) */
void modplug_jump_to_time(modplug_data *music, double time);

#endif
