#include "SDL_audio.h"
#include <rg_system.h>
#include <string.h>
#include <stdlib.h>

static SDL_AudioSpec as;
static bool paused = true;
static rg_audio_sample_t *audio_buffer = NULL;
static uint8_t *sdl_mix_buffer = NULL;
static SemaphoreHandle_t audio_mutex = NULL;

static void audio_task(void *arg)
{
    while (1)
    {
        if (!paused && as.callback)
        {
            SDL_LockAudio();
            memset(sdl_mix_buffer, 0, as.samples * 2); // 16-bit mono/stereo
            
            as.callback(as.userdata, sdl_mix_buffer, as.samples * 2);
            SDL_UnlockAudio();
            // Wolf4SDL usually uses 16-bit mono or stereo. 
            // Retro-Go expects rg_audio_sample_t (int16_t stereo pairs).
            int num_samples = as.samples;
            int out_samples = 0;
            int16_t *src = (int16_t *)sdl_mix_buffer;
            for (int i = 0; i < num_samples; i++) {
                int16_t left, right;
                if (as.channels == 2) {
                    left = src[i * 2];
                    right = src[i * 2 + 1];
                } else {
                    left = src[i];
                    right = src[i];
                }
                
                audio_buffer[out_samples].left = left;
                audio_buffer[out_samples].right = right;
                out_samples++;
                
                if (as.freq == 11025) {
                    audio_buffer[out_samples].left = left;
                    audio_buffer[out_samples].right = right;
                    out_samples++;
                }
            }
            rg_audio_submit(audio_buffer, out_samples);
        }
        else
        {
            vTaskDelay(10);
        }
    }
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    as = *desired;
    if (obtained) *obtained = *desired;
    
    if (audio_mutex == NULL) {
        audio_mutex = xSemaphoreCreateMutex();
    }

    int max_out_samples = as.freq == 11025 ? as.samples * 2 : as.samples;
    audio_buffer = (rg_audio_sample_t *)malloc(max_out_samples * sizeof(rg_audio_sample_t));
    sdl_mix_buffer = (uint8_t *)malloc(as.samples * 4); // Enough for stereo 16-bit
    
    // Wolf's OPL synthesis and channel mixing must complete before the next
    // buffer deadline. Keep it above Retro-Go's display task so sustained
    // rendering cannot starve audio.
    rg_task_create("audio_task", audio_task, NULL, 2048, 1, RG_TASK_PRIORITY_7, 1);
    
    return 0;
}

void SDL_PauseAudio(int pause_on)
{
    paused = pause_on;
}

void SDL_CloseAudio(void)
{
    paused = true;
}

void SDL_LockAudio(void)
{
    if (audio_mutex) {
        xSemaphoreTake(audio_mutex, portMAX_DELAY);
    }
}

void SDL_UnlockAudio(void)
{
    if (audio_mutex) {
        xSemaphoreGive(audio_mutex);
    }
}

void SDL_MixAudio(Uint8 *dst, const Uint8 *src, Uint32 len, int volume)
{
    // Basic mix
    int16_t *s = (int16_t *)src;
    int16_t *d = (int16_t *)dst;
    int count = len / 2;
    
    for (int i = 0; i < count; i++) {
        int32_t mix = d[i] + (s[i] * volume / SDL_MIX_MAXVOLUME);
        if (mix > 32767) mix = 32767;
        if (mix < -32768) mix = -32768;
        d[i] = (int16_t)mix;
    }
}
