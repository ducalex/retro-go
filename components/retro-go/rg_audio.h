#pragma once

#include <stdbool.h>

typedef enum
{
    ODROID_AUDIO_SINK_SPEAKER = 0,
    ODROID_AUDIO_SINK_DAC
} ODROID_AUDIO_SINK;

typedef enum
{
    ODROID_AUDIO_FILTER_NONE = 0,
    ODROID_AUDIO_FILTER_LOW_PASS,
    ODROID_AUDIO_FILTER_HIGH_PASS,
    ODROID_AUDIO_FILTER_WEIGHTED,
} ODROID_AUDIO_FILTER;

int odroid_audio_volume_get();
void odroid_audio_volume_set(int levwl);
void odroid_audio_init(int sample_rate);
void odroid_audio_set_sink(ODROID_AUDIO_SINK sink);
ODROID_AUDIO_SINK odroid_audio_get_sink();
void odroid_audio_terminate();
void odroid_audio_submit(short* stereoAudioBuffer, int frameCount);
int odroid_audio_sample_rate_get();
void odroid_audio_mute(bool mute);