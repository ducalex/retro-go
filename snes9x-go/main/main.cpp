extern "C" {
#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

#include <snes9x.h>

#define APP_ID 90

#define AUDIO_SAMPLE_RATE   (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50)

static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_video_frame_t update1;
static rg_video_frame_t update2;
static rg_video_frame_t *currentUpdate = &update1;
static rg_video_frame_t *previousUpdate = &update2;

// static bool netplay = false;
// --- MAIN

static bool save_state(char *pathName)
{
    bool ret = false;

    return ret;
}


static bool load_state(char *pathName)
{
    bool ret = false;

    return ret;
}


extern "C" void app_main(void)
{
    rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    rg_emu_init(&load_state, &save_state, NULL);
}
