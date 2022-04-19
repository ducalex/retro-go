#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_AUDIO_SINK_SPEAKER = 0,
    RG_AUDIO_SINK_EXT_DAC,
    RG_AUDIO_SINK_BT_A2DP,
    RG_AUDIO_SINK_DUMMY,
} rg_sink_type_t;

typedef struct
{
    rg_sink_type_t type;
    uint32_t device;
    const char *name;
} rg_sink_t;

void rg_audio_init(int sampleRate);
void rg_audio_deinit(void);
void rg_audio_submit(int16_t *stereoAudioBuffer, size_t frameCount);

const rg_sink_t *rg_audio_get_sinks(size_t *count);
const rg_sink_t *rg_audio_get_sink(void);
void rg_audio_set_sink(rg_sink_type_t sink);

int  rg_audio_get_volume(void);
void rg_audio_set_volume(int percent);
void rg_audio_set_mute(bool mute);
int  rg_audio_get_sample_rate(int sampleRate);
void rg_audio_set_sample_rate(int sampleRate);
