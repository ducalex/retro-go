#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RG_AUDIO_SINK_I2S_DAC = 0,
    RG_AUDIO_SINK_I2S_EXT,
    RG_AUDIO_SINK_BT_A2DP,
    RG_AUDIO_SINK_SDL2,
    RG_AUDIO_SINK_DUMMY,
} rg_sink_type_t;

typedef struct
{
    rg_sink_type_t type;
    uint32_t device;
    const char *name;
    // struct {
    //     void *init;
    //     void *submit;
    //     void *deinit;
    // } methods;
} rg_audio_sink_t;

typedef struct __attribute__((packed))
{
    int16_t left;
    int16_t right;
} rg_audio_frame_t;

typedef rg_audio_frame_t rg_audio_sample_t;

typedef struct
{
    int64_t busyTime;
    int32_t samples;
} rg_audio_counters_t;

typedef struct
{
    const rg_audio_sink_t *sink;
    int sampleRate;
    int filter;
    int volume;
    bool muted;
} rg_audio_t;

void rg_audio_init(int sampleRate);
void rg_audio_deinit(void);
void rg_audio_submit(const rg_audio_frame_t *frames, size_t count);
const rg_audio_t *rg_audio_get_info(void);
rg_audio_counters_t rg_audio_get_counters(void);

const rg_audio_sink_t *rg_audio_get_sinks(size_t *count);
const rg_audio_sink_t *rg_audio_get_sink(void);
void rg_audio_set_sink(rg_sink_type_t sink);

int rg_audio_get_volume(void);
void rg_audio_set_volume(int percent);
void rg_audio_set_mute(bool mute);
int rg_audio_get_sample_rate(void);
void rg_audio_set_sample_rate(int sampleRate);
