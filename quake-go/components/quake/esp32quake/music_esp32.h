#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "rg_audio.h"

bool Music_Init(void);
bool Music_IsEnabled(void);
void Music_SetEnabled(bool enabled);
void Music_Play(const char *path, unsigned track, bool loop);
void Music_Stop(void);
void Music_Pause(bool paused);
void Music_MenuPause(bool paused);
void Music_SetVolume(float volume);
void Music_Mix(rg_audio_frame_t *frames, size_t count);
void Music_Shutdown(void);
