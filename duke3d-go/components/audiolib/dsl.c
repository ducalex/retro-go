#include <stdlib.h>
#include <string.h>

#include "dsl.h"
#include "util.h"

#include "SDL.h"
//#include "SDL_mixer.h"

extern volatile int MV_MixPage;

static int DSL_ErrorCode = DSL_Ok;

static int mixer_initialized;

static void ( *_CallBackFunc )( void );
static volatile char *_BufferStart;
static int _BufferSize;
static int _NumDivisions;
static int _SampleRate;
static int _remainder;

//static Mix_Chunk *blank;
static unsigned char *blank_buf;

static SDL_AudioCVT audio_cvt; // used for format conversion

/*
possible todo ideas: cache sdl/sdl mixer error messages.
*/

char *DSL_ErrorString( int ErrorNumber )
{
	char *ErrorString;
	
	switch (ErrorNumber) {
		case DSL_Warning:
		case DSL_Error:
			ErrorString = DSL_ErrorString(DSL_ErrorCode);
			break;
		
		case DSL_Ok:
			ErrorString = "SDL Driver ok.";
			break;
		
		case DSL_SDLInitFailure:
			ErrorString = "SDL Audio initialization failed.";
			break;
		
		case DSL_MixerActive:
			ErrorString = "SDL Mixer already initialized.";
			break;	
	
		case DSL_MixerInitFailure:
			ErrorString = "SDL Mixer initialization failed.";
			break;
			
		default:
			ErrorString = "Unknown SDL Driver error.";
			break;
	}
	
	return ErrorString;
}

static void DSL_SetErrorCode(int ErrorCode)
{
	DSL_ErrorCode = ErrorCode;
}

IRAM_ATTR void audio_cb( void *udata, unsigned char *stream, int len )
//static void mixer_callback(int chan, void *stream, int len, void *udata)
{
	audio_cvt.buf = stream;
	audio_cvt.len = len;
		
	Uint8 *stptr;
	Uint8 *fxptr;
	int copysize;
	
	/* len should equal _BufferSize, else this is screwed up */

	stptr = (Uint8 *)stream;
	
	if (_remainder > 0) {
		copysize = min(len, _remainder);
		
		fxptr = (Uint8 *)(&_BufferStart[MV_MixPage * 
			_BufferSize]);
		
		memcpy(stptr, fxptr+(_BufferSize-_remainder), copysize);
		
		len -= copysize;
		_remainder -= copysize;
		
		stptr += copysize;
	}

	while (len > 0) {
		// new buffer 
		
		_CallBackFunc();
		
		fxptr = (Uint8 *)(&_BufferStart[MV_MixPage * 
			_BufferSize]);

		copysize = min(len, _BufferSize);
		
		memcpy(stptr, fxptr, copysize);
		
		len -= copysize;
		
		stptr += copysize;
	}
	
	_remainder = len;


	SDL_ConvertAudio(&audio_cvt);
	
}

int DSL_Init( void )
{
	DSL_SetErrorCode(DSL_Ok);
	
	if (SDL_InitSubSystem(SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE) < 0) {
		DSL_SetErrorCode(DSL_SDLInitFailure);
		
		return DSL_Error;
	}
	

	
	return DSL_Ok;
}

void DSL_Shutdown( void )
{
	DSL_StopPlayback();
}

int   DSL_BeginBufferedPlayback( char *BufferStart,
      int BufferSize, int NumDivisions, unsigned SampleRate,
      int MixMode, void ( *CallBackFunc )( void ) )
{
	Uint16 format;
	int channels;
	int chunksize;
	int blah;
		
	if (mixer_initialized) {
		DSL_SetErrorCode(DSL_MixerActive);
		
		return DSL_Error;
	}
	
	_CallBackFunc = CallBackFunc;
	_BufferStart = BufferStart;
	_BufferSize = (BufferSize / NumDivisions);
	_NumDivisions = NumDivisions;
	_SampleRate = SampleRate;

	_remainder = 0;
	
	//format = (MixMode & SIXTEEN_BIT) ? AUDIO_S16LSB : AUDIO_U8;
	channels = (MixMode & STEREO) ? 2 : 1;

 /*
	I find 50ms to be ideal, at least with my hardware. This clamping mechanism
	was added because it seems the above remainder handling isn't so nice --kode54
 */
	chunksize = (5 * SampleRate) / 100;

	blah = _BufferSize;
	if (MixMode & SIXTEEN_BIT) blah >>= 1;
	if (MixMode & STEREO) blah >>= 1;

	if (chunksize % blah) chunksize += blah - (chunksize % blah);

	//if (Mix_OpenAudio(SampleRate, format, channels, chunksize) < 0) {
	//	DSL_SetErrorCode(DSL_MixerInitFailure);
		
	//	return DSL_Error;
	//}

/*
	Mix_SetPostMix(mixer_callback, NULL);
*/
	/* have to use a channel because postmix will overwrite the music... */
	//Mix_RegisterEffect(0, mixer_callback, NULL, NULL);
	
	/* create a dummy sample just to allocate that channel */
	//blank_buf = (Uint8 *)malloc(4096);
	//memset(blank_buf, 0, 4096);
	
	//blank = Mix_QuickLoad_RAW(blank_buf, 4096);
		
	//Mix_PlayChannel(0, blank, -1);
	
	SDL_AudioSpec desired;
    SDL_AudioSpec obtained;

	desired.freq = 44000;
	desired.format = AUDIO_S16SYS; //: AUDIO_S8;
	desired.channels = 1;
	desired.samples = 2048;
	desired.callback = audio_cb;

    SDL_OpenAudio(&desired, &obtained);

	SDL_BuildAudioCVT(&audio_cvt, desired.format, desired.channels, desired.freq, obtained.format, obtained.channels, obtained.freq);
	
	SDL_PauseAudio(0); // unpause

	mixer_initialized = 1;
	
	return DSL_Ok;
}

void DSL_StopPlayback( void )
{
	if (mixer_initialized) {
		//Mix_HaltChannel(0);
	}
	
	//if (blank != NULL) {
		//Mix_FreeChunk(blank);
	//}
	
	//blank = NULL;
	
	if (blank_buf  != NULL) {
		free(blank_buf);
	}
	
	blank_buf = NULL;
	
	if (mixer_initialized) {
		//Mix_CloseAudio();
	}
	
	mixer_initialized = 0;
}

unsigned DSL_GetPlaybackRate( void )
{
	return _SampleRate;
}

uint32_t DisableInterrupts( void )
{
	return 0;
}

void RestoreInterrupts( uint32_t flags )
{
}
