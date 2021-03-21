#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>
#include <esp_system.h>
#include <string.h>
#include <unistd.h>

#include "rg_system.h"
#include "rg_audio.h"

static int audioSink = RG_AUDIO_SINK_SPEAKER;
static int audioSampleRate = 0;
static int audioFilter = 0;
static bool audioMuted = false;
static bool audioInitialized = false;
static int volumeLevel = RG_AUDIO_VOL_DEFAULT;
static float volumeLevels[] = {0.f, 0.06f, 0.125f, 0.187f, 0.25f, 0.35f, 0.42f, 0.60f, 0.80f, 1.f};

static const char *SETTING_OUTPUT = "AudioSink";
static const char *SETTING_VOLUME = "Volume";


audio_volume_t rg_audio_get_volume(void)
{
    return volumeLevel;
}

void rg_audio_set_volume(audio_volume_t level)
{
    level = RG_MAX(level, RG_AUDIO_VOL_MIN);
    level = RG_MIN(level, RG_AUDIO_VOL_MAX);
    volumeLevel = level;

    rg_settings_set_int32(SETTING_VOLUME, volumeLevel);

    RG_LOGI("volume set to %d%%\n", (int)(volumeLevels[level] * 100));
}

void rg_audio_init(int sample_rate)
{
    volumeLevel = rg_settings_get_int32(SETTING_VOLUME, RG_AUDIO_VOL_DEFAULT);
    audioSink = rg_settings_get_int32(SETTING_OUTPUT, RG_AUDIO_SINK_SPEAKER);
    audioSampleRate = sample_rate;

    rg_audio_deinit();

    if (audioSink == RG_AUDIO_SINK_SPEAKER)
    {
        i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,          // Only TX
            .sample_rate = audioSampleRate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           // 2-channels
            .communication_format = I2S_COMM_FORMAT_I2S_MSB,
            .dma_buf_count = 2,
            .dma_buf_len = RG_MIN(sample_rate / 50 + 1, 640),                       // The unit is stereo samples (4 bytes)
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                               // Interrupt level 1
            .use_apll = 0 //1
        };

        i2s_driver_install(RG_AUDIO_I2S_NUM, &i2s_config, 0, NULL);
        i2s_set_pin(RG_AUDIO_I2S_NUM, NULL);
        audioInitialized = true;

        RG_LOGI("SPEAKER ready. samplerate=%d, i2s=%f\n", sample_rate, i2s_get_clk(RG_AUDIO_I2S_NUM));
    }
    else if (audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
            .sample_rate = audioSampleRate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           // 2-channels
            .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
            .dma_buf_count = 2,
            .dma_buf_len = RG_MIN(sample_rate / 50 + 1, 640),                       // The unit is stereo samples (4 bytes)
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                               // Interrupt level 1
            .use_apll = 1
        };

        i2s_pin_config_t pin_config = {
            .bck_io_num = RG_GPIO_I2S_DAC_BCK,
            .ws_io_num = RG_GPIO_I2S_DAC_WS,
            .data_out_num = RG_GPIO_I2S_DAC_DATA,
            .data_in_num = -1                                                       //Not used
        };

        i2s_driver_install(RG_AUDIO_I2S_NUM, &i2s_config, 0, NULL);
        i2s_set_pin(RG_AUDIO_I2S_NUM, &pin_config);
        audioInitialized = true;

        RG_LOGI("EXT_DAC ready. samplerate=%d, i2s=%f\n", sample_rate, i2s_get_clk(RG_AUDIO_I2S_NUM));
    }
    else
    {
        RG_LOGI("DUMMY ready. samplerate=%d\n", sample_rate);
        audioSink = RG_AUDIO_SINK_DUMMY;
    }

    rg_audio_set_volume(volumeLevel);
}

void rg_audio_deinit(void)
{
    if (audioInitialized)
    {
        i2s_zero_dma_buffer(RG_AUDIO_I2S_NUM);
        i2s_driver_uninstall(RG_AUDIO_I2S_NUM);
        audioInitialized = false;
    }

    gpio_reset_pin(RG_GPIO_I2S_DAC_DATA);
    gpio_reset_pin(RG_GPIO_I2S_DAC_BCK);
    gpio_reset_pin(RG_GPIO_I2S_DAC_WS);
    gpio_reset_pin(RG_GPIO_DAC1);
    gpio_reset_pin(RG_GPIO_DAC2);
}

static inline void filter_samples(short *samples, size_t count)
{

}

void rg_audio_submit(short *stereoAudioBuffer, size_t frameCount)
{
    size_t sampleCount = frameCount * 2;
    size_t bufferSize = sampleCount * sizeof(short);
    size_t written = 0;
    float volumePercent = volumeLevels[volumeLevel];

    if (bufferSize == 0)
    {
        RG_LOGW("Empty buffer?\n");
        return;
    }

    if (audioMuted || audioSink == RG_AUDIO_SINK_DUMMY)
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
    else if (audioSink == RG_AUDIO_SINK_SPEAKER)
    {
        for (size_t i = 0; i < sampleCount; i += 2)
        {
            // Down mix stereo to mono
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
    else if (audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        for (size_t i = 0; i < sampleCount; ++i)
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

    i2s_write(RG_AUDIO_I2S_NUM, (const void *)stereoAudioBuffer, bufferSize, &written, 1000);
    if (written == 0) // Anything > 0 is fine
    {
        RG_PANIC("i2s_write failed.");
    }
}

void rg_audio_set_sink(audio_sink_t sink)
{
    rg_audio_deinit();
    rg_settings_set_int32(SETTING_OUTPUT, sink);
    rg_audio_init(audioSampleRate);
}

audio_sink_t rg_audio_get_sink(void)
{
    return audioSink;
}

int rg_audio_get_sample_rate(void)
{
    return audioSampleRate;
}

void rg_audio_set_mute(bool mute)
{
    audioMuted = mute;

    if (mute && audioInitialized)
    {
        i2s_zero_dma_buffer(RG_AUDIO_I2S_NUM);
    }
}
