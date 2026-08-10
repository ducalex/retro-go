#ifndef SDL_audio_h_
#define SDL_audio_h_
#include "SDL.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "SDL_rwops.h"
#include "SDL_mixer.h"

// Needed for calling the actual sound output.
#define SAMPLECOUNT		256
#define SAMPLERATE		7042 * 1	// Hz
#define SAMPLESIZE		2   	// 16bit

/** A structure to hold a set of audio conversion filters and buffers */
typedef struct SDL_AudioCVT {
	int needed;			/**< Set to 1 if conversion possible */
	Uint16 src_format;		/**< Source audio format */
	Uint16 dst_format;		/**< Target audio format */
	double rate_incr;		/**< Rate conversion increment */
	Uint8 *buf;			/**< Buffer to hold entire audio data */
	int    len;			/**< Length of original audio buffer */
	int    len_cvt;			/**< Length of converted audio buffer */
	int    len_mult;		/**< buffer must be len*len_mult big */
	double len_ratio; 	/**< Given len, final size is len*len_ratio */
	void (SDLCALL *filters[10])(struct SDL_AudioCVT *cvt, Uint16 format);
	int filter_index;		/**< Current audio conversion function */
} SDL_AudioCVT;

typedef struct{
  int freq;
  Uint16 format;
  Uint8 channels;
  Uint8 silence;
  Uint16 samples;
  Uint32 size;
  void (*callback)(void *userdata, Uint8 *stream, int len);
  void *userdata;
} SDL_AudioSpec;


int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
void SDL_PauseAudio(int pause_on);
void SDL_CloseAudio(void);
int SDL_BuildAudioCVT(SDL_AudioCVT *cvt, Uint16 src_format, Uint8 src_channels, int src_rate, Uint16 dst_format, Uint8 dst_channels, int dst_rate);
int SDL_ConvertAudio(SDL_AudioCVT *cvt);
void SDL_LockAudio(void);
void SDL_UnlockAudio(void);
void SDL_MixAudio(Uint8 * dst, const Uint8 * src, Uint32 len, int volume);

/**
 *  This function loads a WAVE from the data source, automatically freeing
 *  that source if \c freesrc is non-zero.  For example, to load a WAVE file,
 *  you could do:
 *  \code
 *      SDL_LoadWAV_RW(SDL_RWFromFile("sample.wav", "rb"), 1, ...);
 *  \endcode
 *
 *  If this function succeeds, it returns the given SDL_AudioSpec,
 *  filled with the audio data format of the wave data, and sets
 *  \c *audio_buf to a malloc()'d buffer containing the audio data,
 *  and sets \c *audio_len to the length of that audio buffer, in bytes.
 *  You need to free the audio buffer with SDL_FreeWAV() when you are
 *  done with it.
 *
 *  This function returns NULL and sets the SDL error message if the
 *  wave file cannot be opened, uses an unknown data format, or is
 *  corrupt.  Currently raw and MS-ADPCM WAVE files are supported.
 */
extern DECLSPEC SDL_AudioSpec *SDLCALL SDL_LoadWAV_RW(SDL_RWops * src,
                                                      int freesrc,
                                                      SDL_AudioSpec * spec,
                                                      Uint8 ** audio_buf,
                                                      Uint32 * audio_len);

/**
 *  Loads a WAV from a file.
 *  Compatibility convenience function.
 */
#define SDL_LoadWAV(file, spec, audio_buf, audio_len) \
    SDL_LoadWAV_RW(SDL_RWFromFile(file, "rb"),1, spec,audio_buf,audio_len)



/**
 *  \brief Audio format flags.
 *
 *  These are what the 16 bits in SDL_AudioFormat currently mean...
 *  (Unspecified bits are always zero).
 *
 *  \verbatim
    ++-----------------------sample is signed if set
    ||
    ||       ++-----------sample is bigendian if set
    ||       ||
    ||       ||          ++---sample is float if set
    ||       ||          ||
    ||       ||          || +---sample bit size---+
    ||       ||          || |                     |
    15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    \endverbatim
 *
 *  There are macros in SDL 2.0 and later to query these bits.
 */
typedef Uint16 SDL_AudioFormat;

/**
 *  \name Audio flags
 */
/* @{ */

#define SDL_AUDIO_MASK_BITSIZE       (0xFF)
#define SDL_AUDIO_MASK_DATATYPE      (1<<8)
#define SDL_AUDIO_MASK_ENDIAN        (1<<12)
#define SDL_AUDIO_MASK_SIGNED        (1<<15)
#define SDL_AUDIO_BITSIZE(x)         (x & SDL_AUDIO_MASK_BITSIZE)
#define SDL_AUDIO_ISFLOAT(x)         (x & SDL_AUDIO_MASK_DATATYPE)
#define SDL_AUDIO_ISBIGENDIAN(x)     (x & SDL_AUDIO_MASK_ENDIAN)
#define SDL_AUDIO_ISSIGNED(x)        (x & SDL_AUDIO_MASK_SIGNED)
#define SDL_AUDIO_ISINT(x)           (!SDL_AUDIO_ISFLOAT(x))
#define SDL_AUDIO_ISLITTLEENDIAN(x)  (!SDL_AUDIO_ISBIGENDIAN(x))
#define SDL_AUDIO_ISUNSIGNED(x)      (!SDL_AUDIO_ISSIGNED(x))

/**
 *  \name Audio format flags
 *
 *  Defaults to LSB byte order.
 */
/* @{ */
#define AUDIO_U8        0x0008  /**< Unsigned 8-bit samples */
#define AUDIO_S8        0x8008  /**< Signed 8-bit samples */
#define AUDIO_U16LSB    0x0010  /**< Unsigned 16-bit samples */
#define AUDIO_S16LSB    0x8010  /**< Signed 16-bit samples */
#define AUDIO_U16MSB    0x1010  /**< As above, but big-endian byte order */
#define AUDIO_S16MSB    0x9010  /**< As above, but big-endian byte order */
#define AUDIO_U16       AUDIO_U16LSB
#define AUDIO_S16       AUDIO_S16LSB
/* @} */

/**
 *  \name int32 support
 */
/* @{ */
#define AUDIO_S32LSB    0x8020  /**< 32-bit integer samples */
#define AUDIO_S32MSB    0x9020  /**< As above, but big-endian byte order */
#define AUDIO_S32       AUDIO_S32LSB
/* @} */

/**
 *  \name float32 support
 */
/* @{ */
#define AUDIO_F32LSB    0x8120  /**< 32-bit floating point samples */
#define AUDIO_F32MSB    0x9120  /**< As above, but big-endian byte order */
#define AUDIO_F32       AUDIO_F32LSB
/* @} */

/**
 *  \name Native audio byte ordering
 */
/* @{ */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define AUDIO_U16SYS    AUDIO_U16LSB
#define AUDIO_S16SYS    AUDIO_S16LSB
#define AUDIO_S32SYS    AUDIO_S32LSB
#define AUDIO_F32SYS    AUDIO_F32LSB
#else
#define AUDIO_U16SYS    AUDIO_U16MSB
#define AUDIO_S16SYS    AUDIO_S16MSB
#define AUDIO_S32SYS    AUDIO_S32MSB
#define AUDIO_F32SYS    AUDIO_F32MSB
#endif
/* @} */

/**
 *  SDL Audio Device IDs.
 *
 *  A successful call to SDL_OpenAudio() is always device id 1, and legacy
 *  SDL audio APIs assume you want this device ID. SDL_OpenAudioDevice() calls
 *  always returns devices >= 2 on success. The legacy calls are good both
 *  for backwards compatibility and when you don't care about multiple,
 *  specific, or capture devices.
 */
typedef Uint32 SDL_AudioDeviceID;
#define SDL_MIX_MAXVOLUME 128

#endif
