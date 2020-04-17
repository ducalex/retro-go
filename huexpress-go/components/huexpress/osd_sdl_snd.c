#include "osd_sdl_snd.h"

#if 0

#include <SDL.h>

#include "utils.h"
#include "osd_sdl_music.h"


Uint8 *stream;
Mix_Chunk *chunk;
SDL_AudioCVT cvt;
int Callback_Stop;
int USE_S16;


void
osd_snd_set_volume(uchar v)
{
	#if ENABLE_TRACING_SND
	TRACE("Sound: Set Volume 0x%X\n", v);
	#endif

	Uint8 vol;
	vol = v / 2 + ((v == 0) ? 0 : 1);// v=0 <=> vol=0; v=255 <=> vol=128
	Mix_Volume(-1, vol);
}


int
osd_snd_init_sound(void)
{
	uint16 i;
	uint16 format;
	int numopened, frequency, channels;

	if (HCD_last_track > 1) {
		for (i = 1; i<=HCD_last_track; i++) {
			if ((CD_track[i].type==0)
				&& (CD_track[i].source_type==HCD_SOURCE_REGULAR_FILE)) {
				sdlmixmusic[i] = Mix_LoadMUS(CD_track[i].filename);
				if (!sdlmixmusic[i]) {
					Log("Warning: Error when loading '%s'\n",
						CD_track[i].filename);
				}
			}
			USE_S16 = 1;
		}
	} else {
		USE_S16 = 0;
	}

	numopened = Mix_QuerySpec(&frequency, &format, &channels);

	if (!numopened) {
		Log("Mixer initializing...\n");
		if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
			printf("SDL_InitSubSystem(SOUND) failed at %s:%d - %s\n",
				__FILE__, __LINE__, SDL_GetError());
			return 0;
		}

		//MIX_DEFAULT_FORMAT : AUDIO_S16SYS (system byte order).
		host.sound.stereo = 2;
		host.sound.sample_size = sbuf_size;
		host.sound.freq = option.want_snd_freq;

		if (Mix_OpenAudio((host.sound.freq), (USE_S16 ? AUDIO_S16 : AUDIO_U8),
			(USE_S16 ? 2 : host.sound.stereo), sbuf_size) < 0) {
			Log("Couldn't open audio: %s\n", Mix_GetError());
			return 0;
		}

		Mix_QuerySpec(&frequency, &format, &channels);

		if (channels == 2) {
			Log("Mixer obtained stereo.\n");
		}

		Log("Mixer obtained frequency %d.\n", frequency);

		if (format == AUDIO_U8) {
			Log("Mixer obtained format U8.\n");
		}

		host.sound.stereo = channels;
		host.sound.freq = frequency;
		host.sound.sample_size = sbuf_size;


		// sets the callback
		Callback_Stop = 0;
		Mix_AllocateChannels(1);
		Mix_ChannelFinished(sdlmixer_fill_audio);

		//FIXME: start playing silently!!
		//needed for sound fx to start
		int len = sbuf_size * host.sound.stereo;
		stream = (Uint8*)malloc(len);
		//memset(stream,0,len); //FIXME start playing silently!!

		if (!(chunk = Mix_QuickLoad_RAW(stream, len)))
			Log("Mix_QuickLoad_RAW: %s\n",Mix_GetError());

		if (Mix_PlayChannel(0, chunk, 0)==-1)
			Log("Mix_PlayChannel: %s\n", Mix_GetError());

	} else {
		Log("Mixer already initialized :\n");
		Log("allocated mixer channels : %d\n", Mix_AllocateChannels(-1));
	}

	Mix_Resume(-1);

	return 1;
}


void
osd_snd_trash_sound(void)
{
	uchar chan;
	if (sound_driver == 1)
		osd_snd_set_volume(0);

	//needed to stop properly...
	Callback_Stop = 1;
	//SDL_Delay(1000);
	Mix_CloseAudio();

	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	for (chan = 0; chan < 6; chan++)
		memset(sbuf[chan], 0, SBUF_SIZE_BYTE);

	memset(adpcmbuf, 0, SBUF_SIZE_BYTE);

	free(stream);
}
#else
void
osd_snd_set_volume(uchar v)
{
}
#endif


int osd_sound_init()
{
}
