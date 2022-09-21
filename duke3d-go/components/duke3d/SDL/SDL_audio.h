#ifndef SDL_audio_h_
#define SDL_audio_h_
#include "SDL.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Needed for calling the actual sound output.
#define SAMPLECOUNT		256
#define SAMPLERATE		11025 * 1	// Hz
#define SAMPLESIZE		2   	// 16bit

typedef struct{
  int needed;
  Uint16 src_format;
  Uint16 dest_format;
  double rate_incr;
  Uint8 *buf;
  int len;
  int len_cvt;
  int len_mult;
  double len_ratio;
  //void (*filters[10])(struct SDL_AudioCVT *cvt, Uint16 format);
  int filter_index;
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

#endif
