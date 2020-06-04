#pragma once

#include "odroid_settings.h"
#include "stdbool.h"

#define ODROID_AUDIO_VOLUME_MIN 0
#define ODROID_AUDIO_VOLUME_MAX 9 // (sizeof(volumeLevels) / sizeof(float) - 1)
#define ODROID_AUDIO_VOLUME_DEFAULT (ODROID_AUDIO_VOLUME_MAX/2)

int odroid_audio_volume_get();
void odroid_audio_volume_set(int levwl);
void odroid_audio_init(int sample_rate);
void odroid_audio_set_sink(ODROID_AUDIO_SINK sink);
ODROID_AUDIO_SINK odroid_audio_get_sink();
void odroid_audio_terminate();
void odroid_audio_submit(short* stereoAudioBuffer, int frameCount);
int odroid_audio_sample_rate_get();
void odroid_audio_mute(bool mute);