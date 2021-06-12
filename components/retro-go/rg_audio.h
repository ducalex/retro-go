#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_AUDIO_VOL_MIN = 0,
    RG_AUDIO_VOL_1 = 1,
    RG_AUDIO_VOL_2 = 2,
    RG_AUDIO_VOL_3 = 3,
    RG_AUDIO_VOL_4 = 4,
    RG_AUDIO_VOL_5 = 5,
    RG_AUDIO_VOL_6 = 6,
    RG_AUDIO_VOL_7 = 7,
    RG_AUDIO_VOL_8 = 8,
    RG_AUDIO_VOL_MAX = 9,
    RG_AUDIO_VOL_DEFAULT = 4,
} audio_volume_t;

typedef enum
{
    RG_AUDIO_SINK_SPEAKER = 0,
    RG_AUDIO_SINK_EXT_DAC,
    RG_AUDIO_SINK_DUMMY,
} audio_sink_t;

typedef enum
{
    RG_AUDIO_FILTER_NONE = 0,
    RG_AUDIO_FILTER_LOW_PASS,
    RG_AUDIO_FILTER_HIGH_PASS,
    RG_AUDIO_FILTER_WEIGHTED,
} audio_filter_t;

void rg_audio_init(int sample_rate);
void rg_audio_deinit(void);
void rg_audio_set_sink(audio_sink_t sink);
audio_sink_t rg_audio_get_sink(void);
void rg_audio_set_volume(audio_volume_t level);
audio_volume_t rg_audio_get_volume(void);
void rg_audio_set_mute(bool mute);
void rg_audio_submit(short *stereoAudioBuffer, size_t frameCount);
int  rg_audio_get_sample_rate(void);
void rg_audio_clear_buffer();
