#ifdef MODPLUG_MUSIC

#include "music_modplug.h"

static int current_output_channels=0;
static int music_swap8=0;
static int music_swap16=0;
static ModPlug_Settings settings;

int modplug_init(SDL_AudioSpec *spec)
{
	ModPlug_GetSettings(&settings);
	settings.mFlags=MODPLUG_ENABLE_OVERSAMPLING;
	current_output_channels=spec->channels;
	settings.mChannels=spec->channels>1?2:1;
	settings.mBits=spec->format&0xFF;

	music_swap8 = 0;
	music_swap16 = 0;

	switch(spec->format)
	{
		case AUDIO_U8:
		case AUDIO_S8: {
			if ( spec->format == AUDIO_S8 ) {
				music_swap8 = 1;
			}
			settings.mBits=8;
		}
		break;

		case AUDIO_S16LSB:
		case AUDIO_S16MSB: {
			/* See if we need to correct MikMod mixing */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			if ( spec->format == AUDIO_S16MSB ) {
#else
			if ( spec->format == AUDIO_S16LSB ) {
#endif
				music_swap16 = 1;
			}
			settings.mBits=16;
		}
		break;

		default: {
			Mix_SetError("Unknown hardware audio format");
			return -1;
		}

	}

	settings.mFrequency=spec->freq; /*TODO: limit to 11025, 22050, or 44100 ? */
	settings.mResamplingMode=MODPLUG_RESAMPLE_FIR;
	settings.mReverbDepth=0;
	settings.mReverbDelay=100;
	settings.mBassAmount=0;
	settings.mBassRange=50;
	settings.mSurroundDepth=0;
	settings.mSurroundDelay=10;
	settings.mLoopCount=0;
	ModPlug_SetSettings(&settings);
	return 0;
}

/* Uninitialize the music players */
void modplug_exit()
{
}

/* Set the volume for a modplug stream */
void modplug_setvolume(modplug_data *music, int volume)
{
	ModPlug_SetMasterVolume(music->file, volume*4);
}

/* Load a modplug stream from an SDL_RWops object */
modplug_data *modplug_new_RW(SDL_RWops *rw, int freerw)
{
	modplug_data *music=NULL;
	long offset,sz;
	char *buf=NULL;

	offset = SDL_RWtell(rw);
	SDL_RWseek(rw, 0, RW_SEEK_END);
	sz = SDL_RWtell(rw)-offset;
	SDL_RWseek(rw, offset, RW_SEEK_SET);
	buf=(char*)SDL_malloc(sz);
	if(buf)
	{
		if(SDL_RWread(rw, buf, sz, 1)==1)
		{
			music=(modplug_data*)SDL_malloc(sizeof(modplug_data));
			if (music)
			{
				music->playing=0;
				music->file=ModPlug_Load(buf,sz);
				if(!music->file)
				{
					SDL_free(music);
					music=NULL;
				}
			}
			else
			{
				SDL_OutOfMemory();
			}
		}
		SDL_free(buf);
	}
	else
	{
		SDL_OutOfMemory();
	}
	if (freerw) {
		SDL_RWclose(rw);
	}
	return music;
}

/* Start playback of a given modplug stream */
void modplug_play(modplug_data *music)
{
	ModPlug_Seek(music->file,0);
	music->playing=1;
}

/* Return non-zero if a stream is currently playing */
int modplug_playing(modplug_data *music)
{
	return music && music->playing;
}

/* Play some of a stream previously started with modplug_play() */
int modplug_playAudio(modplug_data *music, Uint8 *stream, int len)
{
	if (current_output_channels > 2) {
		int small_len = 2 * len / current_output_channels;
		int i;
		Uint8 *src, *dst;

		i=ModPlug_Read(music->file, stream, small_len);
		if(i<small_len)
		{
			memset(stream+i,0,small_len-i);
			music->playing=0;
		}
		/* and extend to len by copying channels */
		src = stream + small_len;
		dst = stream + len;

		switch (settings.mBits) {
			case 8:
				for ( i=small_len/2; i; --i ) {
					src -= 2;
					dst -= current_output_channels;
					dst[0] = src[0];
					dst[1] = src[1];
					dst[2] = src[0];
					dst[3] = src[1];
					if (current_output_channels == 6) {
						dst[4] = src[0];
						dst[5] = src[1];
					}
				}
				break;
			case 16:
				for ( i=small_len/4; i; --i ) {
					src -= 4;
					dst -= 2 * current_output_channels;
					dst[0] = src[0];
					dst[1] = src[1];
					dst[2] = src[2];
					dst[3] = src[3];
					dst[4] = src[0];
					dst[5] = src[1];
					dst[6] = src[2];
					dst[7] = src[3];
					if (current_output_channels == 6) {
						dst[8] = src[0];
						dst[9] = src[1];
						dst[10] = src[2];
						dst[11] = src[3];
					}
				}
				break;
		}
	} else {
		int i=ModPlug_Read(music->file, stream, len);
		if(i<len)
		{
			memset(stream+i,0,len-i);
			music->playing=0;
		}
	}
	if ( music_swap8 ) {
		Uint8 *dst;
		int i;

		dst = stream;
		for ( i=len; i; --i ) {
			*dst++ ^= 0x80;
		}
	} else
	if ( music_swap16 ) {
		Uint8 *dst, tmp;
		int i;

		dst = stream;
		for ( i=(len/2); i; --i ) {
			tmp = dst[0];
			dst[0] = dst[1];
			dst[1] = tmp;
			dst += 2;
		}
	}
	return 0;
}

/* Stop playback of a stream previously started with modplug_play() */
void modplug_stop(modplug_data *music)
{
	music->playing=0;
}

/* Close the given modplug stream */
void modplug_delete(modplug_data *music)
{
	ModPlug_Unload(music->file);
	SDL_free(music);
}

/* Jump (seek) to a given position (time is in seconds) */
void modplug_jump_to_time(modplug_data *music, double time)
{
	ModPlug_Seek(music->file,(int)(time*1000));
}

#endif
