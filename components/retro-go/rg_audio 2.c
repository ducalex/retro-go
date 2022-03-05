#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <driver/i2s.h>
#include <driver/dac.h>
#include <string.h>
#include <unistd.h>

#include "rg_system.h"
#include "rg_audio.h"

static int audioSink = -1;
static int audioSampleRate = 0;
static int audioFilter = 0;
static int audioVolume = 50;
static bool audioMuted = false;
static SemaphoreHandle_t audioDevLock;
static int64_t dummyBusyUntil = 0;

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
static const char *SETTING_FILTER = "AudioFilter";

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 2, 0)
    #define I2S_COMM_FORMAT_STAND_I2S (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB)
    #define I2S_COMM_FORMAT_STAND_MSB (I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB)
#endif

#ifdef RG_TARGET_QTPY_GAMER
    #define SPEAKER_DAC_MODE I2S_DAC_CHANNEL_LEFT_EN
#else
    #define SPEAKER_DAC_MODE I2S_DAC_CHANNEL_BOTH_EN
#endif


void rg_audio_init(int sampleRate)
{
    RG_ASSERT(audioSink == -1, "Audio sink already initialized!");

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
        .sample_rate = sampleRate,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .dma_buf_count = 2,
        .dma_buf_len = RG_MIN(sampleRate / 50 + 1, 640), // The unit is stereo samples (4 bytes)
        .intr_alloc_flags = 0, // ESP_INTR_FLAG_LEVEL1
        .use_apll = 0
    };
    esp_err_t ret = ESP_FAIL;

    if (audioDevLock == NULL)
        audioDevLock = xSemaphoreCreateMutex();

    xSemaphoreTake(audioDevLock, 1000);

    audioSampleRate = sampleRate;
    audioSink = (int)rg_settings_get_number(NS_GLOBAL, SETTING_OUTPUT, sinks[1].type);
    audioFilter = (int)rg_settings_get_number(NS_GLOBAL, SETTING_FILTER, audioFilter);
    audioVolume = (int)rg_settings_get_number(NS_GLOBAL, SETTING_VOLUME, audioVolume);
    // audioMuted = false;

    if (audioSink == RG_AUDIO_SINK_SPEAKER)
    {
        if ((ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL)) == ESP_OK)
        {
            ret = i2s_set_dac_mode(SPEAKER_DAC_MODE);
        }
    }
    else if (audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        i2s_config.mode = I2S_MODE_MASTER | I2S_MODE_TX;
        i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
        i2s_config.use_apll = 1;
        if ((ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL)) == ESP_OK)
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

    if (RG_GPIO_SND_AMP_ENABLE != GPIO_NUM_NC)
    {
        gpio_set_direction(RG_GPIO_SND_AMP_ENABLE, GPIO_MODE_OUTPUT);
        gpio_set_level(RG_GPIO_SND_AMP_ENABLE, audioMuted ? 0 : 1);
    }

    if (ret == ESP_OK)
    {
        RG_LOGI("Audio ready. sink='%s', samplerate=%d, volume=%d\n",
            rg_audio_get_sink()->name, audioSampleRate, audioVolume);
    }
    else
    {
        RG_LOGE("Failed to initialize audio. sink='%s' %d, samplerate=%d, err=0x%x\n",
            rg_audio_get_sink()->name, audioSink, audioSampleRate, ret);
        audioSink = RG_AUDIO_SINK_DUMMY;
    }

    xSemaphoreGive(audioDevLock);
}

void rg_audio_deinit(void)
{
    // We'll go ahead even if we can't acquire the lock...
    xSemaphoreTake(audioDevLock, 1000);

    if (audioSink == RG_AUDIO_SINK_SPEAKER)
    {
        i2s_driver_uninstall(I2S_NUM_0);
        if (SPEAKER_DAC_MODE & I2S_DAC_CHANNEL_RIGHT_EN)
            dac_output_disable(DAC_CHANNEL_1);
        if (SPEAKER_DAC_MODE & I2S_DAC_CHANNEL_LEFT_EN)
            dac_output_disable(DAC_CHANNEL_2);
        dac_i2s_disable();
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

    xSemaphoreGive(audioDevLock);
}

static inline void filter_samples(short *samples, size_t count)
{

}

void rg_audio_submit(int16_t *stereoAudioBuffer, size_t frameCount)
{
    size_t sampleCount = frameCount * 2;
    size_t bufferSize = sampleCount * sizeof(*stereoAudioBuffer);
    size_t written = 0;
    float volume = audioVolume * 0.01f;

    if (bufferSize == 0)
        return;

    if (!xSemaphoreTake(audioDevLock, 0))
        return;

    if (audioMuted)
    {
        // Some DACs do not like if we stop sending sound when muted. On some devices we can
        // disable the AMP, but on others we must send silence to avoid buzzing...
        volume = 0.f;
    }
    else if (audioFilter)
    {
        filter_samples(stereoAudioBuffer, bufferSize);
    }

    if (audioSink == RG_AUDIO_SINK_DUMMY)
    {
        usleep(RG_MAX(dummyBusyUntil - get_elapsed_time(), 1000));
        dummyBusyUntil = get_elapsed_time() + ((audioSampleRate * 1000) / sampleCount);
        written = bufferSize;
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

            // Push the differential output in the upper byte of each channel
            stereoAudioBuffer[i + 0] = ((dac1 - 0x80) << 8);
            stereoAudioBuffer[i + 1] = ((dac0 + 0x80) << 8);
        }
        i2s_write(I2S_NUM_0, stereoAudioBuffer, bufferSize, &written, 1000);
    }
    else if (audioSink == RG_AUDIO_SINK_EXT_DAC)
    {
        for (size_t i = 0; i < sampleCount; ++i)
        {
            int sample = stereoAudioBuffer[i] * volume;

            // Attenuate because it's too loud on some devices
            #ifdef RG_TARGET_MRGC_G32
            sample >>= 1;
            #endif

            // Clip
            if (sample > 32767)
                sample = 32767;
            else if (sample < -32768)
                sample = -32767;

            stereoAudioBuffer[i] = sample;
        }
        i2s_write(I2S_NUM_0, stereoAudioBuffer, bufferSize, &written, 1000);
    }

    if (written < bufferSize)
    {
        RG_LOGW("Submission error: %d/%d bytes sent.\n", written, bufferSize);
    }

    xSemaphoreGive(audioDevLock);
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

int rg_audio_get_volume(void)
{
    return audioVolume;
}

void rg_audio_set_volume(int percent)
{
    audioVolume = RG_MIN(RG_MAX(percent, 0), 100);
    rg_settings_set_number(NS_GLOBAL, SETTING_VOLUME, audioVolume);
    RG_LOGI("Volume set to %d%%\n", audioVolume);
}

void rg_audio_set_mute(bool mute)
{
    if (xSemaphoreTake(audioDevLock, 1000) == pdTRUE)
    {
        if (RG_GPIO_SND_AMP_ENABLE != GPIO_NUM_NC)
            gpio_set_level(RG_GPIO_SND_AMP_ENABLE, mute ? 0 : 1);
        if (audioSink == RG_AUDIO_SINK_SPEAKER || audioSink == RG_AUDIO_SINK_EXT_DAC)
            i2s_zero_dma_buffer(I2S_NUM_0);
        xSemaphoreGive(audioDevLock);
    }
    audioMuted = mute;
}
