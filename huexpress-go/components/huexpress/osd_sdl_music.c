#if 0
#include "osd_sdl_music.h"
#include "utils.h"

Mix_Music *sdlmixmusic[MAX_SONGS];


/* Callback for SDL Mixer */
void
sdlmixer_fill_audio(int channel)
{
	if (Callback_Stop == 1) {
		// free things and
		//stop calling backs...
		if (chunk)
			Mix_FreeChunk(chunk);

		Log("stop playing back psg\n");
		return;
	}

	uchar lvol, rvol;
	int i;

	if (!cvt.len) { // once only?
		memset(stream,0,sbuf_size*host.sound.stereo);
		if (!SDL_BuildAudioCVT (&cvt, AUDIO_U8, host.sound.stereo, host.sound.freq,
			AUDIO_S16, 2, host.sound.freq)) {
			Log("SDL_BuildAudioCVT failed...audio callback stopped.\n");
			return;//#warning: avoid leaving without calling again ?
		}
		Log("SDL_BuildAudioCVT init ok.\n");

 		cvt.len = sbuf_size*host.sound.stereo;
		cvt.buf = (Uint8*) malloc(cvt.len*cvt.len_mult); //for in place conversion
	}

	for (i = 0; i < 6; i++)
		WriteBuffer(sbuf[i], i, cvt.len);

	write_adpcm();

	// Adjust the final post-mixed left/right volumes.  0-15 * 1.22 comes out to
	// (0..18) which when multiplied by the ((-127..127) * 7) we get in the final
	// stream mix below we have (-16002..16002) which we then divide by 128 to get
	// a nice unsigned 8-bit value of 128 + (-125..125).

	if (host.sound.stereo) {
		lvol = (io.psg_volume >> 4) * 1.22;
		rvol = (io.psg_volume & 0x0F) * 1.22;
	} else {
		//
		// Use the average of the two channels for mono
		//
		lvol = rvol = (((io.psg_volume >> 4) * 1.22) + ((io.psg_volume & 0x0F) * 1.22)) / 2;
	}

    //
    // Mix streams and apply master volume.
    //

	if (USE_S16 == 1) {
    	for (i = 0; i < cvt.len ; i++) {
      		cvt.buf[i] = 128 + ((uint32) ((sbuf[0][i] + sbuf[1][i] + sbuf[2][i] + sbuf[3][i] + sbuf[4][i] + sbuf[5][i]
				+ adpcmbuf[i]) * (!(i % 2) ? lvol : rvol)) >> 7);
		}
    	// convert audio data
    	SDL_ConvertAudio(&cvt);
    	chunk = Mix_QuickLoad_RAW(cvt.buf, cvt.len_cvt);
	} else {
    	for (i = 0; i < cvt.len ; i++)
			stream[i] = 128 + ((uint32) ((sbuf[0][i] + sbuf[1][i] + sbuf[2][i] + sbuf[3][i] + sbuf[4][i] + sbuf[5][i]
				+ adpcmbuf[i]) * (!(i % 2) ? lvol : rvol)) >> 7);
		chunk = Mix_QuickLoad_RAW(stream, cvt.len);
	}

	if (!chunk) {
		Log("Mix_QuickLoad_RAW: %s\n",Mix_GetError());
//		return;//#warning: avoid leaving without calling again ?
	}
//	chunk->allocated = 1;
	if (Mix_PlayChannel(channel,chunk,0)==-1) {
		Log("Mix_PlayChannel: %s\n",Mix_GetError());
//		return;//#warning: avoid leaving without calling again ?
	}

//#warning: test dump
	if (dump_snd) { // We also have to write data into a file
		dump_audio_chunck(cvt.buf, cvt.len_cvt);
	}
}

#endif
