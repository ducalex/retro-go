#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>
#include <driver/dac.h>
#include <unistd.h>

#include "rg_system.h"
#include "rg_audio.h"

static int audioSink = -1;
// static const rg_sink_t *audioSink;
static int audioSampleRate = 0;
static int audioFilter = 0;
static bool audioMuted = false;
static int volumeLevel = RG_AUDIO_VOL_DEFAULT;
static int volumeMap[] = {0, 7, 15, 28, 39, 47, 56, 65, 74, 88, 100};

static const rg_sink_t sinks[] = {
    {RG_AUDIO_SINK_DUMMY,   0, "Dummy"},
#if RG_AUDIO_USE_SPEAKER
    {RG_AUDIO_SINK_SPEAKER, 0, "Speaker"},
#endif
#if RG_AUDIO_USE_EXT_DAC
    {RG_AUDIO_SINK_EXT_DAC, 0, "Ext DAC"},
#endif
    // {RG_AUDIO_SINK_BT_A2DP, 0, "Bluetooth"},
};

static const char *SETTING_OUTPUT = "AudioSink";
static const char *SETTING_VOLUME = "Volume";
// static const char *SETTING_FILTER = "AudioFilter";

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 2, 0)
    #define I2S_COMM_FORMAT_STAND_I2S (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
    #define I2S_COMM_FORMAT_STAND_MSB (I2S_COMM_FORMAT_I2S |I2S_COMM_FORMAT_I2S_LSB)
#endif


void rg_audio_init(int sample_rate)
{
    int volume = rg_settings_get_number(NS_GLOBAL, SETTING_VOLUME, RG_AUDIO_VOL_DEFAULT);
    int sinkType = rg_settings_get_number(NS_GLOBAL, SETTING_OUTPUT, sinks[1].type);

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = sample_rate,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = 2,
        .dma_buf_len = RG_MIN(sample_rate / 50 + 1, 640), // The unit is stereo samples (4 bytes)
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,         // Interrupt level 1
        .use_apll = 0
    };
    esp_err_t ret = ESP_FAIL;

    if (audioSink != -1)
        rg_audio_deinit();

    audioSampleRate = sample_rate;
    audioSink = sinkType;

    if (audioSink == RG_AUDIO_SINK_SPEAKER)
    {
        i2s_config.mode |= I2S_MODE_DAC_BUILT_IN;
        if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) == ESP_OK)
        {
            ret = i2s_set_pin(I2S_NUM_0, NULL);
        }
    }
    else if (audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
        i2s_config.use_apll = 1;
        if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) == ESP_OK)
        {
            ret = i2s_set_pin(I2S_NUM_0, &(i2s_pin_config_t) {
                .bck_io_num = RG_GPIO_SND_I2S_BCK,
                .ws_io_num = RG_GPIO_SND_I2S_WS,
                .data_out_num = RG_GPIO_SND_I2S_DATA,
                .data_in_num = GPIO_NUM_NC
            });
        }
    }
    else if (audioSink == RG_AUDIO_SINK_DUMMY)
    {
        ret = ESP_OK;
    }

    if (ret == ESP_OK)
    {
        RG_LOGI("Audio ready. sink='%s', samplerate=%d\n", rg_audio_get_sink()->name, sample_rate);
    }
    else
    {
        RG_LOGE("Failed to initialize audio. sink='%s' %d, samplerate=%d, err=0x%x\n",
            rg_audio_get_sink()->name, sinkType, sample_rate, ret);
        audioSink = RG_AUDIO_SINK_DUMMY;
    }

    rg_audio_set_volume(volume);
    rg_audio_set_mute(false);
}

void rg_audio_deinit(void)
{
    rg_audio_set_mute(true);

    if (audioSink == RG_AUDIO_SINK_SPEAKER)
    {
        gpio_num_t pin;
        i2s_driver_uninstall(I2S_NUM_0);
        dac_pad_get_io_num(DAC_CHANNEL_1, &pin);
        gpio_reset_pin(pin);
        dac_pad_get_io_num(DAC_CHANNEL_2, &pin);
        gpio_reset_pin(pin);
    }
    else if (audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        i2s_driver_uninstall(I2S_NUM_0);
        gpio_reset_pin(RG_GPIO_SND_I2S_BCK);
        gpio_reset_pin(RG_GPIO_SND_I2S_DATA);
        gpio_reset_pin(RG_GPIO_SND_I2S_WS);
    }

    if (RG_GPIO_SND_AMP_ENABLE != GPIO_NUM_NC)
    {
        gpio_reset_pin(RG_GPIO_SND_AMP_ENABLE);
    }

    RG_LOGI("Audio terminated. sink='%s'\n", rg_audio_get_sink()->name);
    audioSink = -1;
}

static inline void filter_samples(short *samples, size_t count)
{

}

void rg_audio_submit(int16_t *stereoAudioBuffer, size_t frameCount)
{
    size_t sampleCount = frameCount * 2;
    size_t bufferSize = sampleCount * sizeof(*stereoAudioBuffer);
    size_t written = 0;
    float volume = volumeMap[volumeLevel] * 0.01f;

    if (bufferSize == 0)
    {
        RG_LOGW("Empty buffer?\n");
        return;
    }

    if (audioFilter)
    {
        filter_samples(stereoAudioBuffer, bufferSize);
    }

    if (audioMuted || audioSink == RG_AUDIO_SINK_DUMMY)
    {
        // Simulate i2s_write_bytes delay
        usleep((audioSampleRate * 1000) / sampleCount);
    }
    else if (audioSink == RG_AUDIO_SINK_SPEAKER)
    {
        // In speaker mode we use dac left and right as a single channel
        // to increase resolution.
        float multiplier = (127 + 127) * volume;
        for (size_t i = 0; i < sampleCount; i += 2)
        {
            // Down mix stereo to mono
            int sample = (stereoAudioBuffer[i] + stereoAudioBuffer[i + 1]) >> 1;

            // Scale
            int range = (float)sample / 0x8000 * multiplier;
            int dac0, dac1;

            // Convert to differential output
            if (range > 127)
            {
                dac1 = (range - 127);
                dac0 = 127;
            }
            else if (range < -127)
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

            stereoAudioBuffer[i] = dac1;
            stereoAudioBuffer[i + 1] = dac0;
        }
        i2s_write(I2S_NUM_0, (const void *)stereoAudioBuffer, bufferSize, &written, 1000);
        RG_ASSERT(written > 0, "i2s_write failed.");
    }
    else if (audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        for (size_t i = 0; i < sampleCount; ++i)
        {
            int sample = stereoAudioBuffer[i] * volume;

            // Clip
            if (sample > 32767)
                sample = 32767;
            else if (sample < -32768)
                sample = -32767;

            stereoAudioBuffer[i] = sample;
        }
        i2s_write(I2S_NUM_0, (const void *)stereoAudioBuffer, bufferSize, &written, 1000);
        RG_ASSERT(written > 0, "i2s_write failed.");
    }
    else
    {
        RG_PANIC("Audio Sink Unknown");
    }
}

const rg_sink_t *rg_audio_get_sinks(size_t *count)
{
    if (count) *count = RG_COUNT(sinks);
    return sinks;
}

const rg_sink_t *rg_audio_get_sink(void)
{
    for (size_t i = 0; i < RG_COUNT(sinks); ++i)
        if (sinks[i].type == audioSink)
            return &sinks[i];
    return &sinks[0];
}

void rg_audio_set_sink(rg_sink_type_t sink)
{
    rg_settings_set_number(NS_GLOBAL, SETTING_OUTPUT, sink);
    rg_audio_deinit();
    rg_audio_init(audioSampleRate);
}

rg_volume_t rg_audio_get_volume(void)
{
    return volumeLevel;
}

void rg_audio_set_volume(rg_volume_t level)
{
    volumeLevel = RG_MIN(RG_MAX(level, RG_AUDIO_VOL_MIN), RG_AUDIO_VOL_MAX);
    rg_settings_set_number(NS_GLOBAL, SETTING_VOLUME, volumeLevel);
    RG_LOGI("Volume set to %d%% (%d)\n", volumeMap[volumeLevel], volumeLevel);
}

void rg_audio_set_mute(bool mute)
{
    if (RG_GPIO_SND_AMP_ENABLE != GPIO_NUM_NC)
    {
        gpio_set_direction(RG_GPIO_SND_AMP_ENABLE, GPIO_MODE_OUTPUT);
        gpio_set_level(RG_GPIO_SND_AMP_ENABLE, mute ? 0 : 1);
    }
    if (audioSink == RG_AUDIO_SINK_SPEAKER || audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        i2s_zero_dma_buffer(I2S_NUM_0);
    }
    audioMuted = mute;
}
