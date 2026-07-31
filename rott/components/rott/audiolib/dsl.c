#include "rt_def.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "dsl.h"
#include "oplplayer.h"

// Hack to avoid collision with ESP-IDF's internal assert usage
#ifdef ESP_PLATFORM
#undef assert
#define assert(x) ((void)0)
#endif

#include <rg_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef ESP_PLATFORM
#undef assert
#include <assert.h>
#endif

#define AUDIO_RATE_MULTIPLIER 2

extern volatile int MV_MixPage;
extern char *MV_MixBuffer[];

static int DSL_ErrorCode = DSL_Ok;
static volatile int mixer_initialized;

static void ( *_CallBackFunc )( void );
static volatile char *_BufferStart;
static int _BufferSize;
static int _NumDivisions;
static int _SampleRate;
static volatile bool stop_task = false;

static rg_audio_sample_t *mix_buffer = NULL;
static int mix_buffer_capacity = 0;

static SemaphoreHandle_t audio_mutex = NULL;

char *DSL_ErrorString( int ErrorNumber )
{
    switch (ErrorNumber) {
        case DSL_Ok: return "Retro-Go Audio Driver ok.";
        case DSL_SDLInitFailure: return "Audio initialization failed.";
        case DSL_MixerActive: return "Audio mixer is already active.";
        case DSL_MixerInitFailure: return "Audio mixer initialization failed.";
        default: return "Unknown Audio Driver error.";
    }
}

static void dsl_task(void *arg)
{
    while (!stop_task)
    {
        if (audio_mutex && xSemaphoreTakeRecursive(audio_mutex, 0) == pdTRUE) {
            // 1. Mixer mixes into MV_MixBuffer[MV_MixPage]
            _CallBackFunc();
        } else {
            // Game task is currently modifying audio state, yield and skip this cycle
            vTaskDelay(1);
            continue;
        }

        // 2. Read the page that was JUST mixed.
        // It's 8-bit UNSIGNED mono (128 is silence).
        uint8_t *fxptr = (uint8_t *)MV_MixBuffer[MV_MixPage];

        if (fxptr == NULL) {
            xSemaphoreGiveRecursive(audio_mutex);
            vTaskDelay(1);
            continue;
        }

        // 3. Render music into mix_buffer (Stereo 16-bit interleaved).
        // The OPL player always fills the complete requested range, including
        // silence when no song is active, so no separate clear is needed.
        // Keep the lock through FX and OPL generation so game-thread sound
        // and music operations cannot alter or free state while it is in use.
        opl_synth_player.render((void *)mix_buffer, _BufferSize);

        // 4. Mix the unsigned 8-bit mono effects into the signed stereo OPL
        // output and expand 11.025kHz to 22.05kHz in one backward pass.
        // Backward traversal preserves every source frame until it is read.
        for (int i = _BufferSize - 1; i >= 0; i--)
        {
            // Center at 128, then scale up to 16-bit
            int32_t sample = (int32_t)((int)fxptr[i] - 128) << 8;
            int32_t mixed_l = mix_buffer[i].left + sample;
            int32_t mixed_r = mix_buffer[i].right + sample;

            // Clamp to 16-bit limits
            if (mixed_l > 32767) mixed_l = 32767; else if (mixed_l < -32768) mixed_l = -32768;
            if (mixed_r > 32767) mixed_r = 32767; else if (mixed_r < -32768) mixed_r = -32768;

            rg_audio_sample_t mixed = {
                .left = (int16_t)mixed_l,
                .right = (int16_t)mixed_r,
            };
            mix_buffer[i * AUDIO_RATE_MULTIPLIER] = mixed;
            mix_buffer[i * AUDIO_RATE_MULTIPLIER + 1] = mixed;
        }

        xSemaphoreGiveRecursive(audio_mutex);

        // 5. Submit frame count, not bytes. Blocks if the ringbuffer is full.
        rg_audio_submit(mix_buffer, _BufferSize * AUDIO_RATE_MULTIPLIER);

        // Yield slightly
        vTaskDelay(1);
    }

    mixer_initialized = 0;
    vTaskDelete(NULL);
}

int DSL_Init( void )
{
    return DSL_Ok;
}

void DSL_Shutdown( void )
{
    DSL_StopPlayback();
}

int DSL_BeginBufferedPlayback( char *BufferStart,
      int BufferSize, int NumDivisions, unsigned SampleRate,
      int MixMode, void ( *CallBackFunc )( void ) )
{
    if (mixer_initialized) {
        DSL_ErrorCode = DSL_MixerActive;
        return DSL_Error;
    }

    if (audio_mutex == NULL) {
        audio_mutex = xSemaphoreCreateRecursiveMutex();
        if (audio_mutex == NULL) {
            DSL_ErrorCode = DSL_MixerInitFailure;
            return DSL_Error;
        }
    }

    _CallBackFunc = CallBackFunc;
    _BufferStart = BufferStart;
    _BufferSize = (BufferSize / NumDivisions);
    _NumDivisions = NumDivisions;
    _SampleRate = SampleRate;

    mix_buffer_capacity = _BufferSize * AUDIO_RATE_MULTIPLIER;
    mix_buffer = rg_alloc(mix_buffer_capacity * sizeof(rg_audio_sample_t), MEM_SLOW);
    if (mix_buffer == NULL) {
        mix_buffer_capacity = 0;
        DSL_ErrorCode = DSL_MixerInitFailure;
        return DSL_Error;
    }

    stop_task = false;
    mixer_initialized = 1;

    // The mixer uses about 1.4KB of stack; retain more than 2KB headroom while
    // returning scarce internal RAM to the application.
    if (xTaskCreatePinnedToCore(dsl_task, "dsl_audio", 4 * 1024,
            NULL, 15, NULL, 0) != pdPASS) {
        mixer_initialized = 0;
        free(mix_buffer);
        mix_buffer = NULL;
        mix_buffer_capacity = 0;
        DSL_ErrorCode = DSL_MixerInitFailure;
        return DSL_Error;
    }

    DSL_ErrorCode = DSL_Ok;
    return DSL_Ok;
}

void DSL_StopPlayback( void )
{
    if (mixer_initialized) {
        stop_task = true;

        // The task can be inside the blocking Retro-Go audio submission.
        // Wait until it has left the mixer before MultiVoc releases or
        // replaces any state referenced by the task.
        while (mixer_initialized) {
            vTaskDelay(1);
        }
    }

    if (mix_buffer) {
        free(mix_buffer);
        mix_buffer = NULL;
        mix_buffer_capacity = 0;
    }
}

unsigned DSL_GetPlaybackRate( void )
{
    return _SampleRate;
}

unsigned long DisableInterrupts( void )
{
    if (audio_mutex &&
        xSemaphoreTakeRecursive(audio_mutex, portMAX_DELAY) == pdTRUE) {
        return 1;
    }

    return 0;
}

void RestoreInterrupts( unsigned long flags )
{
    if (flags && audio_mutex) xSemaphoreGiveRecursive(audio_mutex);
}
