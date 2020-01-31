#pragma once

#include "odroid_settings.h"

typedef enum
{
    ODROID_VOLUME_LEVEL0 = 0,
    ODROID_VOLUME_LEVEL1,
    ODROID_VOLUME_LEVEL2,
    ODROID_VOLUME_LEVEL3,
    ODROID_VOLUME_LEVEL4,
    ODROID_VOLUME_LEVEL5,
    ODROID_VOLUME_LEVEL6,
    ODROID_VOLUME_LEVEL7,
    ODROID_VOLUME_LEVEL8,
    ODROID_VOLUME_LEVEL_COUNT,
} odroid_volume_level;


odroid_volume_level odroid_audio_volume_get();
void odroid_audio_volume_set(odroid_volume_level value);
void odroid_audio_init(ODROID_AUDIO_SINK sink, int sample_rate);
void odroid_audio_set_sink(ODROID_AUDIO_SINK sink);
void odroid_audio_terminate();
void odroid_audio_submit(short* stereoAudioBuffer, int frameCount);
int odroid_audio_sample_rate_get();
void odroid_audio_clear_buffer();
