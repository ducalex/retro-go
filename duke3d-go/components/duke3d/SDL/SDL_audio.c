#include "SDL_audio.h"

SDL_AudioSpec as;
bool paused = true;
bool locked = false;
xSemaphoreHandle xSemaphoreAudio = NULL;

IRAM_ATTR void updateTask(void *arg)
{
  while(1)
  {
	  if(!paused && /*xSemaphoreAudio != NULL*/ !locked ){
			unsigned char sdl_buffer[SAMPLECOUNT * SAMPLESIZE * 2] = {0};
			(*as.callback)(NULL, sdl_buffer, SAMPLECOUNT*SAMPLESIZE );
			rg_audio_submit(sdl_buffer, SAMPLECOUNT);
	  } else
		  vTaskDelay( 5 );
  }
}

void SDL_AudioInit()
{
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
	SDL_AudioInit();
	memset(obtained, 0, sizeof(SDL_AudioSpec));
	obtained->freq = SAMPLERATE;
	obtained->format = 16;
	obtained->channels = 1;
	obtained->samples = SAMPLECOUNT;
	obtained->callback = desired->callback;
	memcpy(&as,obtained,sizeof(SDL_AudioSpec));

	//xSemaphoreAudio = xSemaphoreCreateBinary();
	// xTaskCreatePinnedToCore(&updateTask, "updateTask", 4000, NULL, 2, NULL, 1);
	printf("audio task started\n");
	return 0;
}

void SDL_PauseAudio(int pause_on)
{
	paused = pause_on;
}

void SDL_CloseAudio(void)
{

}

int SDL_BuildAudioCVT(SDL_AudioCVT *cvt, Uint16 src_format, Uint8 src_channels, int src_rate, Uint16 dst_format, Uint8 dst_channels, int dst_rate)
{
	cvt->len_mult = 1;
	return 0;
}

IRAM_ATTR int SDL_ConvertAudio(SDL_AudioCVT *cvt)
{

	Sint16 *sbuf = cvt->buf;
	Uint16 *ubuf = cvt->buf;

	int32_t dac0;
	int32_t dac1;

	for(int i = cvt->len-2; i >= 0; i-=2)
	{
		Sint16 range = sbuf[i/2] >> 8;

		// Convert to differential output
		if (range > 127)
		{
			dac1 = (range - 127);
			dac0 = 127;
		}
		else if (range < -127)
		{
			dac1  = (range + 127);
			dac0 = -127;
		}
		else
		{
			dac1 = 0;
			dac0 = range;
		}

		dac0 += 0x80;
		dac1 = 0x80 - dac1;

		dac0 <<= 8;
		dac1 <<= 8;

		ubuf[i] = (int16_t)dac1;
        ubuf[i + 1] = (int16_t)dac0;
	}

	return 0;
}

void SDL_LockAudio(void)
{
	locked = true;
	//if( xSemaphoreAudio != NULL )
	//	xSemaphoreTake( xSemaphoreAudio, 100 );
}

void SDL_UnlockAudio(void)
{
    locked = false;
	//if( xSemaphoreAudio != NULL )
	//	 xSemaphoreGive( xSemaphoreAudio );
}

