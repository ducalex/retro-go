#include "freertos/FreeRTOS.h"
#include "odroid_audio.h"
#include "string.h"
#include "unistd.h"
#include "esp_system.h"
#include "driver/i2s.h"
#include "driver/rtc_io.h"

#define I2S_NUM (I2S_NUM_0)

static int audioSink = ODROID_AUDIO_SINK_SPEAKER;
static int audioSampleRate = 0;
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
            //.mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
            .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
            .sample_rate = audioSampleRate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
            .communication_format = I2S_COMM_FORMAT_I2S_MSB,
            //.communication_format = I2S_COMM_FORMAT_PCM,
            .dma_buf_count = 8,
            //.dma_buf_len = 1472 / 2,  // (368samples * 2ch * 2(short)) = 1472
            .dma_buf_len = 534,  // (416samples * 2ch * 2(short)) = 1664
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                //Interrupt level 1
            .use_apll = 0 //1
        };

        i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

        i2s_set_pin(I2S_NUM, NULL);
        i2s_set_dac_mode(/*I2S_DAC_CHANNEL_LEFT_EN*/ I2S_DAC_CHANNEL_BOTH_EN);
    }
    else if (audioSink == ODROID_AUDIO_SINK_DAC)
    {
        i2s_config_t i2s_config = {
            .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
            .sample_rate = audioSampleRate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
            .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
            .dma_buf_count = 8,
            //.dma_buf_len = 1472 / 2,  // (368samples * 2ch * 2(short)) = 1472
            .dma_buf_len = 534,  // (416samples * 2ch * 2(short)) = 1664
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                //Interrupt level 1
            .use_apll = 1
        };

        i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

        i2s_pin_config_t pin_config = {
            .bck_io_num = 4,
            .ws_io_num = 12,
            .data_out_num = 15,
            .data_in_num = -1                                                       //Not used
        };
        i2s_set_pin(I2S_NUM, &pin_config);

        // Disable internal amp
        gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
        gpio_set_direction(GPIO_NUM_26, GPIO_MODE_DISABLE);
        gpio_set_level(GPIO_NUM_25, 0);
    }
    else
    {
        abort();
    }

    odroid_audio_volume_set(volumeLevel);
}

void odroid_audio_terminate()
{
    if (audioInitialized)
    {
        i2s_zero_dma_buffer(I2S_NUM);
        i2s_driver_uninstall(I2S_NUM);
        audioInitialized = false;
    }

    gpio_reset_pin(GPIO_NUM_25);
    gpio_reset_pin(GPIO_NUM_26);
}

IRAM_ATTR void odroid_audio_submit(short* stereoAudioBuffer, int frameCount)
{
    size_t sampleCount = frameCount * 2;
    size_t bufferSize = sampleCount * sizeof(int16_t);
    size_t written = 0;
    float volumePercent = volumeLevels[volumeLevel];

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
        abort();
    }

    i2s_write(I2S_NUM, (const short *)stereoAudioBuffer, bufferSize, &written, 1000);
    if (written == 0) // Anything > 0 is fine
    {
        printf("odroid_audio_submit: i2s_write failed.\n");
        abort();
    }
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
	    i2s_zero_dma_buffer(I2S_NUM);
    }
}
