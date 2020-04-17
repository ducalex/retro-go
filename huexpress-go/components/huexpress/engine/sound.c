/***************************************************************************/
/*                                                                         */
/*                         Sound Source File                               */
/*                                                                         */
/*     Initialisation, shuting down and PC Engine generation of sound      */
/*                                                                         */
/***************************************************************************/

/* Header */

#include "sound.h"

#if 0

#include <SDL_audio.h>

#include "utils.h"
#include "osd_sdl_music.h"


SDL_AudioSpec wanted;			/* For SDL Audio */
extern void sdl_fill_audio(void *data, Uint8 * stream, int len);

/* Variables definition */

uchar sound_driver = 3;
// 0 =-� No sound driver
// 1 =-� Allegro sound driver
// 2 =-� Seal sound driver
// 3 =-� SDL/SDL_Mixer driver

char MP3_playing = 0;
// is MP3 playing ?

char *sbuf[6];
// the buffer where the "DATA-TO-SEND-TO-THE-SOUND-CARD" go
// one for each channel

char *adpcmbuf;
// the buffer filled with adpcm data

uchar new_adpcm_play = 0;
// Have we begun a new adpcm sample (i.e. must be reset adpcm index/prev value)

uchar main_buf[SBUF_SIZE_BYTE];
// the mixed buffer, may be removed later for hard mixing...

uint32 CycleOld;
uint32 CycleNew;
// indicates the last time music has been "released"

/* TODO */
int BaseClock = 7170000;
// int BaseClock = 8992000;
// the freq of the internal PC Engine CPU
// the sound use a kind of "relative" frequency
// I think there's a pb with this value that cause troubles with some cd sync

uint32 ds_nChannels = 1;
// mono or stereo, to remove later

uint32 dwNewPos;

uint32 AdpcmFilledBuf = 0;
// Size (in nibbles) of adpcm buf that has been filled with new data

uchar *big_buf;

uchar gen_vol = 255;

uint32 sbuf_size = 10 * 1024;


/* Functions definition */

int
InitSound(void)
{
	for (silent = 0; silent < 6; silent++)
		sbuf[silent] = (char *) calloc(sizeof(char), SBUF_SIZE_BYTE);

	adpcmbuf = (char *) calloc(sizeof(char), SBUF_SIZE_BYTE);

	silent = 1;

	if (smode == 0)				// No sound
		return 1;

/* SDL Audio / Mixer Begin */
/*
#if !defined(SDL_mixer)
	  SDL_AudioSpec obtained;
      Log ("Initialisation of SDL sound... ");
      wanted.freq = option.want_snd_freq; // Frequency
      printf("Frequency = %d\n", option.want_snd_freq);
      wanted.format = AUDIO_U8; // Unsigned 8 bits
      wanted.channels = 1; // Mono
      wanted.samples = 512; //SBUF_SIZE_BYTE;
      wanted.size = SBUF_SIZE_BYTE;
      printf("wanted.size = %d\n",wanted.size);
      wanted.callback = sdl_fill_audio;
      wanted.userdata = main_buf;//NULL;
      
      if (SDL_OpenAudio(&wanted,&obtained) < 0) {
		  printf("Couldn't Open SDL Audio: %s\n",SDL_GetError());
#endif // !SDL_mixer
      if (Mix_OpenAudio(option.want_snd_freq,AUDIO_U8,1,512) < 0) {
		  printf("Couldn't Open SDL Mixer: %s\n",Mix_GetError());

	return FALSE;
      }

#if !defined(SDL_mixer)
      Log ("OK\nObtained frequency = %d\n",obtained.freq);
      SDL_PauseAudio(0);
#endif

      silent=0;
	  Mix_Resume(-1);
	  */
	silent = 0;

/* End of SDL Audio / Mixer */

	return 0;
}


void
TrashSound(void)
{								/* Shut down sound  */
	uchar dum;

	if (!silent) {
		for (dum = 0; dum < 6; dum++)
			free(sbuf[dum]);

		free(adpcmbuf);

		silent = 1;
	}
}


void
write_psg(int ch)
{
	uint32 Cycle;

	if (CycleNew != CycleOld) {
		Cycle = CycleNew - CycleOld;
		CycleOld = CycleNew;

		dwNewPos = (unsigned) ((float) (host.sound.freq) * (float) Cycle
							   / (float) BaseClock);
		// in fact, size of the data to write

	};


#if ENABLE_TRACING_AUDIO
	TRACE("AUDIO: New Position: %d\n", dwNewPos);
#endif

/*  SDL makes clipping automagicaly
 *  if (sound_driver == 3) {
	if (dwNewPos > wanted.size) {
		dwNewPos = wanted.size;
		fprintf(stderr, "overrun: %d\n",dwNewPos);
	}
  }
*/
	if (sound_driver == 2)		// || sound_driver == 3) /* Added 3 (SDL) */
	{
		if (dwNewPos > (uint32) host.sound.freq * SOUND_BUF_MS / 1000) {
#if ENABLE_TRACING_AUDIO
			TRACE("AUDIO: Sound buffer overrun\n");
#endif
			dwNewPos = host.sound.freq * SOUND_BUF_MS / 1000;
			// Ask it to fill the buffer
		} else if (sound_driver == 1) {
#if ENABLE_TRACING_AUDIO
			TRACE("AUDIO: dwNewPos = %d / %d\n", dwNewPos, sbuf_size);
#endif
			if (dwNewPos > sbuf_size) {
#if ENABLE_TRACING_AUDIO
				TRACE("AUDIO: Sound buffer overrun\n");
#endif
				dwNewPos = sbuf_size;
				// Ask it to fill the buffer
			}
#if ENABLE_TRACING_AUDIO
			TRACE("AUDIO: After correction, dwNewPos = %d\n", dwNewPos);
#endif
		}
	}

	Log("Buffer %d will be filled\n", ch);
	WriteBuffer(&sbuf[ch][0], ch, dwNewPos * ds_nChannels);
	// write DATA 'til dwNewPos

#if ENABLE_TRACING_AUDIO
	TRACE("AUDIO: Buffer %d has been filled\n", ch);
#endif


};


/* TODO : doesn't support repeat mode for now */
void
write_adpcm(void)
{
	uint32 Cycle;
	uint32 AdpcmUsedNibbles;

	static char index;
	static int32 previousValue;

	if (CycleNew != CycleOld) {
		Cycle = CycleNew - CycleOld;
		CycleOld = CycleNew;

		dwNewPos = (unsigned) ((float) (host.sound.freq) * (float) Cycle
							   / (float) BaseClock);
		// in fact, size of the data to write
	};

	AdpcmFilledBuf = dwNewPos;

	if (new_adpcm_play) {
		index = 0;
		previousValue = 0;
	}

	if (AdpcmFilledBuf > io.adpcm_psize)
		AdpcmFilledBuf = io.adpcm_psize;

	AdpcmUsedNibbles
		= WriteBufferAdpcm8(adpcmbuf, io.adpcm_pptr, AdpcmFilledBuf,
							&index, &previousValue);

	io.adpcm_pptr += AdpcmUsedNibbles;
	io.adpcm_pptr &= 0x1FFFF;

	if (AdpcmUsedNibbles)
		io.adpcm_psize -= AdpcmUsedNibbles;
	else
		io.adpcm_psize = 0;

	/* If we haven't played even a nibbles, it problably mean we won't ever be
	 * able to play one, so we stop the adpcm playing
	 */

#if ENABLE_TRACING_AUDIO
//  Log("size = %d\n", io.adpcm_psize);
#endif

};

//! file for dumping audio
static FILE *audio_output_file = NULL;

//! Size (in byte) of audio data dumped
static int sound_dump_length;

//! Cycle of the last sound output
static uint32 sound_dump_last_cycle;

//! Start the audio dump process
//! return 1 if audio dumping began, else 0
int
start_dump_audio(void)
{
	char audio_output_filename[PATH_MAX];
	struct tm *tm_current_time;
	time_t time_t_current_time;

	if (audio_output_file != NULL)
		return 0;

	time(&time_t_current_time);
	tm_current_time = localtime(&time_t_current_time);

	snprintf(audio_output_filename, PATH_MAX,
			 "%saudio-%04d-%02d-%02d %02d-%02d-%02d.wav", video_path,
			 tm_current_time->tm_year + 1900, tm_current_time->tm_mon + 1,
			 tm_current_time->tm_mday, tm_current_time->tm_hour,
			 tm_current_time->tm_min, tm_current_time->tm_sec);

	audio_output_file = fopen(audio_output_filename, "wb");

	sound_dump_length = 0;

	fwrite("RIFF\145\330\073\0WAVEfmt ", 16, 1, audio_output_file);
	putc(0x10, audio_output_file);	// size
	putc(0x00, audio_output_file);
	putc(0x00, audio_output_file);
	putc(0x00, audio_output_file);
	putc(1, audio_output_file);	// PCM data
	putc(0, audio_output_file);

	if (host.sound.stereo)
		putc(2, audio_output_file);	// stereo
	else
		putc(1, audio_output_file);	// mono

	putc(0, audio_output_file);

	putc(host.sound.freq, audio_output_file);	// frequency
	putc(host.sound.freq >> 8, audio_output_file);
	putc(host.sound.freq >> 16, audio_output_file);
	putc(host.sound.freq >> 24, audio_output_file);

	if (host.sound.stereo) {
		putc(host.sound.freq << 1, audio_output_file);	// size of data per second
		putc(host.sound.freq >> 7, audio_output_file);
		putc(host.sound.freq >> 15, audio_output_file);
		putc(host.sound.freq >> 23, audio_output_file);
	} else {
		putc(host.sound.freq, audio_output_file);	// size of data per second
		putc(host.sound.freq >> 8, audio_output_file);
		putc(host.sound.freq >> 16, audio_output_file);
		putc(host.sound.freq >> 24, audio_output_file);
	}

	if (host.sound.stereo)
		putc(2, audio_output_file);	// byte per sample
	else
		putc(1, audio_output_file);
	putc(0, audio_output_file);

	putc(8, audio_output_file);	// 8 bits
	putc(0, audio_output_file);

	fwrite("data\377\377\377\377", 1, 9, audio_output_file);
	osd_gfx_set_message("Audio dumping on");

	return (audio_output_file != NULL ? 1 : 0);
}


void
stop_dump_audio(void)
{
	uint32 dum;

	if (audio_output_file == NULL)
		return;

	dum = sound_dump_length + 0x2C;	// Total file size, header is 0x2C long
	fseek(audio_output_file, 4, SEEK_SET);
	putc(dum, audio_output_file);
	putc(dum >> 8, audio_output_file);
	putc(dum >> 16, audio_output_file);
	putc(dum >> 24, audio_output_file);

	dum = sound_dump_length;	// Audio stream size
	fseek(audio_output_file, 0x28, SEEK_SET);
	putc(dum, audio_output_file);
	putc(dum >> 8, audio_output_file);
	putc(dum >> 16, audio_output_file);
	putc(dum >> 24, audio_output_file);

	fclose(audio_output_file);

	osd_gfx_set_message("Audio dumping off");
}


void
dump_audio_chunck(uchar * content, int length)
{
	int cycle;
	int real_length;

	real_length = 0;

	if (audio_output_file == NULL)
		return;

	if (CycleNew != sound_dump_last_cycle) {
		cycle = CycleNew - sound_dump_last_cycle;
		sound_dump_last_cycle = CycleNew;

		real_length = (unsigned) ((float) (host.sound.freq) * (float) cycle
								  / (float) BaseClock);
		// in fact, size of the data to write
	};

	printf("given length = %5d\treal length = %5d\n", length, real_length);

	if (real_length > length)
		real_length = length;

	fwrite(content, real_length, 1, audio_output_file);

	sound_dump_length += real_length;
}
#else


uchar gen_vol = 255;
int BaseClock = 7170000;
//uint32 CycleOld;
//uint32 CycleNew;
uchar new_adpcm_play = 0;
uint32 AdpcmFilledBuf = 0;





#endif
