#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>
#include <esp_system.h>
#include <driver/dac.h>
#include <string.h>
#include <unistd.h>

#include "rg_system.h"
#include "rg_audio.h"

static int audioSink = -1;
static int audioSampleRate = 0;
static int audioFilter = 0;
static bool audioMuted = false;
static int volumeLevel = RG_AUDIO_VOL_DEFAULT;
static int volumeMap[] = {0, 7, 15, 28, 39, 50, 61, 74, 88, 100};

static const char *SETTING_OUTPUT = "AudioSink";
static const char *SETTING_VOLUME = "Volume";
// static const char *SETTING_FILTER = "AudioFilter";


audio_volume_t rg_audio_get_volume(void)
{
    return volumeLevel;
}

void rg_audio_set_volume(audio_volume_t level)
{
    volumeLevel = RG_MIN(RG_AUDIO_VOL_MAX, RG_MAX(RG_AUDIO_VOL_MIN, level));
    rg_settings_set_int32(SETTING_VOLUME, volumeLevel);
    RG_LOGI("volume set to %d%%\n", volumeMap[volumeLevel]);
}

void rg_audio_init(int sample_rate)
{
    int volume = rg_settings_get_int32(SETTING_VOLUME, RG_AUDIO_VOL_DEFAULT);
    int sink = rg_settings_get_int32(SETTING_OUTPUT, RG_AUDIO_SINK_SPEAKER);

    rg_audio_deinit();

    if (sink == RG_AUDIO_SINK_SPEAKER || sink == RG_AUDIO_SINK_EXT_DAC)
    {
        i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,          // Only TX
            .sample_rate = sample_rate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           // 2-channels
            .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB,
            .dma_buf_count = 2,
            .dma_buf_len = RG_MIN(sample_rate / 50 + 1, 640),                       // The unit is stereo samples (4 bytes)
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                               // Interrupt level 1
            .use_apll = 0 //1
        };

        const i2s_pin_config_t *pin_config = NULL;
        const i2s_pin_config_t ext_dac_pins = {
            .bck_io_num = RG_GPIO_EXT_I2S_DAC_BCK,
            .ws_io_num = RG_GPIO_EXT_I2S_DAC_WS,
            .data_out_num = RG_GPIO_EXT_I2S_DAC_DATA,
            .data_in_num = -1   //Not used
        };

        if (sink == RG_AUDIO_SINK_EXT_DAC)
        {
            i2s_config.communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB;
            i2s_config.mode &= ~I2S_MODE_DAC_BUILT_IN;
            i2s_config.use_apll = 1;
            pin_config = &ext_dac_pins;
        }

        if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) == ESP_OK)
        {
            i2s_set_pin(I2S_NUM_0, pin_config);
            RG_LOGI("I2S audio ready. dac=%d, samplerate=%d, i2s=%f\n", sink, sample_rate, i2s_get_clk(I2S_NUM_0));
            audioSink = sink;
            audioSampleRate = sample_rate;
        }
        else
        {
            RG_LOGE("Failed to initialize I2S audio with dac %d !\n", sink);
        }
    }

    if (audioSink < 0)
    {
        RG_LOGI("DUMMY audio ready. samplerate=%d\n", sample_rate);
        audioSink = RG_AUDIO_SINK_DUMMY;
        audioSampleRate = sample_rate;
    }

    rg_audio_set_volume(volume);
}

void rg_audio_deinit(void)
{
    rg_audio_clear_buffer();

    if (audioSink == RG_AUDIO_SINK_SPEAKER)
    {
        gpio_num_t pin;
        i2s_driver_uninstall(I2S_NUM_0);
        dac_pad_get_io_num(DAC_CHANNEL_1, &pin);
        gpio_reset_pin(pin);
        dac_pad_get_io_num(DAC_CHANNEL_2, &pin);
        gpio_reset_pin(pin);
        RG_LOGI("Internal I2S audio terminated\n");
    }
    else if (audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        i2s_driver_uninstall(I2S_NUM_0);
        gpio_reset_pin(RG_GPIO_EXT_I2S_DAC_BCK);
        gpio_reset_pin(RG_GPIO_EXT_I2S_DAC_DATA);
        gpio_reset_pin(RG_GPIO_EXT_I2S_DAC_WS);
        RG_LOGI("External I2S audio terminated\n");
    }
    audioSink = -1;
}

static inline void filter_samples(short *samples, size_t count)
{

}

void rg_audio_submit(short *stereoAudioBuffer, size_t frameCount)
{
    size_t sampleCount = frameCount * 2;
    size_t bufferSize = sampleCount * sizeof(short);
    size_t written = 0;
    float volume = volumeMap[volumeLevel] * 0.01f;

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

    if (volume == 0.0f)
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
            const float range = magnitude * sn * volume;

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
            int32_t sample = stereoAudioBuffer[i] * volume;

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

    i2s_write(I2S_NUM_0, (const void *)stereoAudioBuffer, bufferSize, &written, 1000);
    if (written == 0) // Anything > 0 is fine
    {
        RG_PANIC("i2s_write failed.");
    }
}

void rg_audio_set_sink(audio_sink_t sink)
{
    rg_settings_set_int32(SETTING_OUTPUT, sink);
    rg_audio_deinit();
    rg_audio_init(audioSampleRate);
}

audio_sink_t rg_audio_get_sink(void)
{
    return audioSink;
}

void rg_audio_clear_buffer()
{
    if (audioSink == RG_AUDIO_SINK_SPEAKER || audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        i2s_zero_dma_buffer(I2S_NUM_0);
    }
}

void rg_audio_set_mute(bool mute)
{
    audioMuted = mute;
    rg_audio_clear_buffer();
}
