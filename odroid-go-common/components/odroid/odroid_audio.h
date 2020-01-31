#pragma once

#include "odroid_settings.h"

#define ODROID_VOLUME_LEVEL_COUNT (5)

typedef enum
{
    ODROID_VOLUME_LEVEL0 = 0,
    ODROID_VOLUME_LEVEL1 = 1,
    ODROID_VOLUME_LEVEL2 = 2,
    ODROID_VOLUME_LEVEL3 = 3,
    ODROID_VOLUME_LEVEL4 = 4,

    _ODROID_VOLUME_FILLER = 0xffffffff
} odroid_volume_level;


odroid_volume_level odroid_audio_volume_get();
void odroid_audio_volume_set(odroid_volume_level value);
void odroid_audio_volume_change();
void odroid_audio_init(ODROID_AUDIO_SINK sink, int sample_rate);
void odroid_audio_terminate();
void odroid_audio_submit(short* stereoAudioBuffer, int frameCount);
int odroid_audio_sample_rate_get();
