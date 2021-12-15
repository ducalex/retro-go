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
    RG_AUDIO_VOL_9 = 9,
    RG_AUDIO_VOL_MAX = 10,
    RG_AUDIO_VOL_DEFAULT = RG_AUDIO_VOL_4,
} rg_volume_t;

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

void rg_audio_init(int sample_rate);
void rg_audio_deinit(void);
void rg_audio_submit(int16_t *stereoAudioBuffer, size_t frameCount);

const rg_sink_t *rg_audio_get_sinks(size_t *count);
const rg_sink_t *rg_audio_get_sink(void);
void rg_audio_set_sink(rg_sink_type_t sink);

rg_volume_t rg_audio_get_volume(void);
void rg_audio_set_volume(rg_volume_t level);
void rg_audio_set_mute(bool mute);
