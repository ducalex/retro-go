#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>
#include <esp_system.h>
#include <string.h>
#include <unistd.h>

#include "odroid_system.h"
#include "odroid_audio.h"

static int audioSink = ODROID_AUDIO_SINK_SPEAKER;
static int audioSampleRate = 0;
static int audioFilter = 0;
static bool audioMuted = 0;
static bool audioInitialized = 0;
static int volumeLevel = ODROID_AUDIO_VOLUME_DEFAULT;
static float volumeLevels[] = {0.f, 0.06f, 0.125f, 0.187f, 0.25f, 0.35f, 0.42f, 0.60f, 0.80f, 1.f};


int odroid_audio_volume_get()
{
    return volumeLevel;
}

void odroid_audio_volume_set(int level)
{
    if (level < ODROID_AUDIO_VOLUME_MIN)
    {
        printf("odroid_audio_volume_set: level out of range (< 0) (%d)\n", level);
        level = ODROID_AUDIO_VOLUME_MIN;
    }
    else if (level > ODROID_AUDIO_VOLUME_MAX)
    {
        printf("odroid_audio_volume_set: level out of range (> max) (%d)\n", level);
        level = ODROID_AUDIO_VOLUME_MAX;
    }

    odroid_settings_Volume_set(level);

    volumeLevel = level;
}

void odroid_audio_init(int sample_rate)
{
    volumeLevel = odroid_settings_Volume_get();
    audioSink = odroid_settings_AudioSink_get();
    audioSampleRate = sample_rate;
    audioInitialized = true;

    printf("%s: sink=%d, sample_rate=%d\n", __func__, audioSink, sample_rate);

    // NOTE: buffer needs to be adjusted per AUDIO_SAMPLE_RATE
    if (audioSink == ODROID_AUDIO_SINK_SPEAKER)
    {
        i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,          // Only TX
            .sample_rate = audioSampleRate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
            .communication_format = I2S_COMM_FORMAT_I2S_MSB,
            .dma_buf_count = 2,
            .dma_buf_len = 580, // 580 stereo 16bit samples (32000 / 55fps) = 2320 bytes
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                //Interrupt level 1
            .use_apll = 0 //1
        };

        i2s_driver_install(ODROID_I2S_NUM, &i2s_config, 0, NULL);
        i2s_set_pin(ODROID_I2S_NUM, NULL);
    }
    else if (audioSink == ODROID_AUDIO_SINK_DAC)
    {
        i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
            .sample_rate = audioSampleRate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
            .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
            .dma_buf_count = 2,
            .dma_buf_len = 580, // 580 stereo 16bit samples (32000 / 55fps) = 2320 bytes
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                //Interrupt level 1
            .use_apll = 1
        };

        i2s_pin_config_t pin_config = {
            .bck_io_num = ODROID_PIN_I2S_DAC_BCK,
            .ws_io_num = ODROID_PIN_I2S_DAC_WS,
            .data_out_num = ODROID_PIN_I2S_DAC_DATA,
            .data_in_num = -1                                                       //Not used
        };

        i2s_driver_install(ODROID_I2S_NUM, &i2s_config, 0, NULL);
        i2s_set_pin(ODROID_I2S_NUM, &pin_config);

        // Disable internal amp
        gpio_set_direction(ODROID_PIN_DAC1, GPIO_MODE_OUTPUT);
        gpio_set_direction(ODROID_PIN_DAC2, GPIO_MODE_DISABLE);
        gpio_set_level(ODROID_PIN_DAC1, 0);
    }
    else
    {
        RG_PANIC("Audio Sink Unknown");
    }

    odroid_audio_volume_set(volumeLevel);

    printf("%s: I2S init done. clock=%f\n", __func__, i2s_get_clk(ODROID_I2S_NUM));
}

void odroid_audio_terminate()
{
    if (audioInitialized)
    {
        i2s_zero_dma_buffer(ODROID_I2S_NUM);
        i2s_driver_uninstall(ODROID_I2S_NUM);
        audioInitialized = false;
    }

    gpio_reset_pin(ODROID_PIN_DAC1);
    gpio_reset_pin(ODROID_PIN_DAC2);
}

static inline void filter_samples(short* samples, int count)
{

}

IRAM_ATTR void odroid_audio_submit(short* stereoAudioBuffer, int frameCount)
{
    size_t sampleCount = frameCount * 2;
    size_t bufferSize = sampleCount * sizeof(int16_t);
    size_t written = 0;
    float volumePercent = volumeLevels[volumeLevel];

    if (bufferSize == 0)
    {
        printf("%s: Empty buffer?\n", __func__);
        return;
    }

    if (audioMuted)
    {
        // Simulate i2s_write_bytes delay
        usleep((audioSampleRate * 1000) / sampleCount);
        return;
    }

    if (volumePercent == 0.0f)
    {
        // No need to even loop, just clear the buffer
        memset(stereoAudioBuffer, 0, bufferSize);
    }
    else if (audioSink == ODROID_AUDIO_SINK_SPEAKER)
    {
        for (short i = 0; i < sampleCount; i += 2)
        {
            // Down mix stero to mono
            int32_t sample = (stereoAudioBuffer[i] + stereoAudioBuffer[i + 1]) >> 1;

            // Normalize
            const float sn = (float)sample / 0x8000;

            // Scale
            const int magnitude = 127 + 127;
            const float range = magnitude * sn * volumePercent;

            uint16_t dac0, dac1;

            // Convert to differential output
            if (range > 127.f)
            {
                dac1 = (range - 127);
                dac0 = 127;
            }
            else if (range < -127.f)
            {
                dac1 = (range + 127);
                dac0 = -127;
            }
            else
            {
                dac1 = 0;
                dac0 = range;
            }

            dac0 += 0x80;
            dac1 = 0x80 - dac1;

            dac0 <<= 8;
            dac1 <<= 8;

            stereoAudioBuffer[i] = (short)dac1;
            stereoAudioBuffer[i + 1] = (short)dac0;
        }
    }
    else if (audioSink == ODROID_AUDIO_SINK_DAC)
    {
        for (short i = 0; i < sampleCount; ++i)
        {
            int32_t sample = stereoAudioBuffer[i] * volumePercent;

            // Clip
            if (sample > 32767)
                sample = 32767;
            else if (sample < -32768)
                sample = -32767;

            stereoAudioBuffer[i] = (short)sample;
        }
    }
    else
    {
        RG_PANIC("Audio Sink Unknown");
    }

    if (audioFilter)
    {
        filter_samples(stereoAudioBuffer, bufferSize);
    }

    i2s_write(ODROID_I2S_NUM, (const short *)stereoAudioBuffer, bufferSize, &written, 1000);
    if (written == 0) // Anything > 0 is fine
    {
        RG_PANIC("i2s_write failed.");
    }
}

bool odroid_audio_is_playing()
{
    return false;
}

void odroid_audio_set_sink(ODROID_AUDIO_SINK sink)
{
    odroid_settings_AudioSink_set(sink);
    audioSink = sink;

    if (audioSampleRate > 0)
    {
        odroid_audio_terminate();
        odroid_audio_init(audioSampleRate);
    }
}

ODROID_AUDIO_SINK odroid_audio_get_sink()
{
    return audioSink;
}

int odroid_audio_sample_rate_get()
{
    return audioSampleRate;
}

void odroid_audio_mute(bool mute)
{
    audioMuted = mute;

    if (mute && audioInitialized)
    {
	    i2s_zero_dma_buffer(ODROID_I2S_NUM);
    }
}
