#include "SDL_audio.h"
#include "freertos/semphr.h"
#include "rg_system.h"
#include "rg_audio.h"
#include "oplplayer.h"
#include <string.h>

SDL_AudioSpec as;
bool paused = true;
bool locked = false;

static int16_t *mono_buffer = NULL;
static int16_t *music_buffer = NULL;
static rg_audio_frame_t *stereo_buffer = NULL;
TaskHandle_t audio_task_handle = NULL;
static bool audio_task_running = false;

#define ENGINE_RATE 11025
#define HW_RATE     22050
#define AUDIO_TASK_CORE 1
#define AUDIO_TASK_PRIORITY RG_TASK_PRIORITY_8

IRAM_ATTR void updateTask(void *arg)
{
  audio_task_running = true;
  if (!mono_buffer) mono_buffer = malloc(SAMPLECOUNT * sizeof(int16_t));
  if (!music_buffer) music_buffer = malloc(SAMPLECOUNT * 2 * sizeof(int16_t));
  if (!stereo_buffer) stereo_buffer = malloc(SAMPLECOUNT * 2 * sizeof(rg_audio_frame_t));

  while(audio_task_running)
  {
	  if(!paused && !locked && as.callback){
			// Ask engine for sound effects (at 11025Hz)
            memset(mono_buffer, 0, SAMPLECOUNT * sizeof(int16_t));
			(*as.callback)(as.userdata, (uint8_t *)mono_buffer, SAMPLECOUNT * sizeof(int16_t));

            // Ask OPL synth for music (at 11025Hz)
            memset(music_buffer, 0, SAMPLECOUNT * 2 * sizeof(int16_t));
            opl_synth_player.render(music_buffer, SAMPLECOUNT);

            for (int i = 0; i < SAMPLECOUNT; i++) {
                int32_t mixed_l = mono_buffer[i] + music_buffer[i*2];
                int32_t mixed_r = mono_buffer[i] + music_buffer[i*2+1];
                
                // Clamp
                if (mixed_l > 32767) mixed_l = 32767; else if (mixed_l < -32768) mixed_l = -32768;
                if (mixed_r > 32767) mixed_r = 32767; else if (mixed_r < -32768) mixed_r = -32768;
                
                // 2x Oversampling (duplicate samples) to fill HW_RATE buffer
                stereo_buffer[i*2].left = mixed_l;
                stereo_buffer[i*2].right = mixed_r;
                stereo_buffer[i*2+1].left = mixed_l;
                stereo_buffer[i*2+1].right = mixed_r;
            }

			rg_audio_submit(stereo_buffer, SAMPLECOUNT * 2);
            vTaskDelay(pdMS_TO_TICKS(1)); // Yield
	  } else {
		  vTaskDelay(pdMS_TO_TICKS(10));
      }
  }
  
  if (mono_buffer) { free(mono_buffer); mono_buffer = NULL; }
  if (music_buffer) { free(music_buffer); music_buffer = NULL; }
  if (stereo_buffer) { free(stereo_buffer); stereo_buffer = NULL; }
  audio_task_handle = NULL;
  vTaskDelete(NULL);
}

void SDL_AudioInit()
{
}

void set_overclock_safe(int level)
{
    static int last_level = -1;

    // Skip if level hasn't changed — avoids redundant PLL I2C manipulation
    if (level == last_level)
        return;

#ifndef CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ
#define CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ 240
#endif
    int default_cpu_speed = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;

    // Only touch PLL when actually overclocking. At level 0 the default clock
    // is already correct and cpu_speed falls back to CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ (240).
    if (level != 0) {
        // Temporarily raise app.sampleRate baseline to 25000 Hz. This ensures
        // that Retro-Go's internal overclock sample-rate scaling calculations
        // (app.sampleRate * default_cpu_speed / real_mhz) never request a rate below 20000 Hz.
        rg_system_reinit(25000, NULL, NULL);

        // Set the overclock level. Retro-Go will configure a safe internal rate.
        rg_system_set_overclock(level);
    }

    // Retrieve actual CPU clock speed for this level.
    int cpu_speed = rg_system_get_cpu_speed();
    if (cpu_speed <= 0) {
        cpu_speed = default_cpu_speed;
    }

    // Determine target sample rate.
    int target_rate = (HW_RATE * default_cpu_speed) / cpu_speed;
#if CONFIG_IDF_TARGET_ESP32P4
    target_rate = HW_RATE;
#elif CONFIG_IDF_TARGET_ESP32
    if (strcmp(rg_audio_get_sink()->name, "Ext DAC") == 0) {
        target_rate = HW_RATE;
    } else if (target_rate < 20000) {
        // Clamp sample rate to a minimum of 20000 Hz on ESP32 Speaker to prevent 
        // built-in DAC divider register limit overflows (> 255) in ESP-IDF.
        target_rate = 20000;
    }
#endif

    // Sync app.sampleRate baseline in Retro-Go to match target_rate.
    int baseline_rate = (target_rate * cpu_speed) / default_cpu_speed;
    rg_system_reinit(baseline_rate, NULL, NULL);

    // Apply sample rate configuration to audio hardware.
    rg_audio_set_sample_rate(target_rate);

    // Calculate and apply tempo scaling multiplier to compensate for any residual hardware clock skew.
    double expected_rate = ((double)HW_RATE * (double)default_cpu_speed) / cpu_speed;
#if CONFIG_IDF_TARGET_ESP32P4
    expected_rate = (double)HW_RATE;
#elif CONFIG_IDF_TARGET_ESP32
    if (strcmp(rg_audio_get_sink()->name, "Ext DAC") == 0) {
        expected_rate = (double)HW_RATE;
    }
#endif
    double multiplier = expected_rate / (double)target_rate;
    I_OPL_SetTempoMultiplier(multiplier);

    last_level = level;
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
	SDL_AudioInit();
	memset(obtained, 0, sizeof(SDL_AudioSpec));
    
	obtained->freq = ENGINE_RATE;
	obtained->format = desired->format;
	obtained->channels = 1;
	obtained->samples = SAMPLECOUNT;
	obtained->callback = desired->callback;
    obtained->userdata = desired->userdata;
	memcpy(&as, obtained, sizeof(SDL_AudioSpec));

    opl_synth_player.init(ENGINE_RATE); // Synth renders at 11kHz
    set_overclock_safe(rg_system_get_overclock());

	xTaskCreatePinnedToCore(&updateTask, "audioTask", 8192, NULL,
                            AUDIO_TASK_PRIORITY, &audio_task_handle, AUDIO_TASK_CORE);
	printf("audio task started at %d Hz output (Rendering at %d Hz)\n", HW_RATE, ENGINE_RATE);
	return 0;
}

void SDL_PauseAudio(int pause_on)
{
	paused = pause_on;
}

void SDL_CloseAudio(void)
{
    if (audio_task_running) {
        audio_task_running = false;
        int retry = 500;
        while (audio_task_handle != NULL && retry-- > 0) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void audio_shutdown(void)
{
    RG_LOGI("audio_shutdown: Muting and stopping audio task...");
    as.callback = NULL;
    if (rg_audio_get_sink()) {
        rg_audio_set_mute(true);
    }
    SDL_CloseAudio();
}

int SDL_BuildAudioCVT(SDL_AudioCVT *cvt, Uint16 src_format, Uint8 src_channels, int src_rate, Uint16 dst_format, Uint8 dst_channels, int dst_rate)
{
	cvt->len_mult = 1;
	return 0;
}

int SDL_ConvertAudio(SDL_AudioCVT *cvt)
{
	return 0;
}

void SDL_LockAudio(void)
{
	locked = true;
}

void SDL_UnlockAudio(void)
{
    locked = false;
}
