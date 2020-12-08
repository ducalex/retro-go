#pragma once

#include <stdbool.h>

typedef enum
{
    RG_AUDIO_VOL_MIN = 0,
    RG_AUDIO_VOL_MAX = 9,
    RG_AUDIO_VOL_DEFAULT = 4,
} audio_volume_t;

typedef enum
{
    RG_AUDIO_SINK_SPEAKER = 0,
    RG_AUDIO_SINK_DAC
} audio_sink_t;

typedef enum
{
    RG_AUDIO_FILTER_NONE = 0,
    RG_AUDIO_FILTER_LOW_PASS,
    RG_AUDIO_FILTER_HIGH_PASS,
    RG_AUDIO_FILTER_WEIGHTED,
} audio_filter_t;

int odroid_audio_volume_get();
void odroid_audio_volume_set(int levwl);
void odroid_audio_init(int sample_rate);
void odroid_audio_set_sink(audio_sink_t sink);
audio_sink_t odroid_audio_get_sink();
void odroid_audio_terminate();
void odroid_audio_submit(short* stereoAudioBuffer, int frameCount);
int odroid_audio_sample_rate_get();
void odroid_audio_mute(bool mute);