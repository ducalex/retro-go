#include "rg_system.h"
#include "rg_audio.h"

#if RG_AUDIO_USE_INT_DAC || RG_AUDIO_USE_EXT_DAC

#ifndef ESP_PLATFORM
#error "I2S support can only be built inside esp-idf!"
#elif !CONFIG_IDF_TARGET_ESP32 && RG_AUDIO_USE_INT_DAC
#error "Your chip has no DAC! Please set RG_AUDIO_USE_INT_DAC to 0 in your target file."
#endif

#include <driver/gpio.h>
#include <driver/i2s.h>
#if RG_AUDIO_USE_INT_DAC
#include <driver/dac.h>
#endif

static struct {
    const char *last_error;
    int device;
    int volume;
    bool muted;
} state;

static bool driver_init(int device, int sample_rate)
{
    state.last_error = NULL;
    state.device = device;

    if (state.device == 0)
    {
    #if RG_AUDIO_USE_INT_DAC
        esp_err_t ret = i2s_driver_install(I2S_NUM_0, &(i2s_config_t){
            .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
            .sample_rate = sample_rate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_MSB,
            .intr_alloc_flags = 0, // ESP_INTR_FLAG_LEVEL1
            .dma_buf_count = 4, // Goal is to have ~800 samples over 2-8 buffers (3x270 or 5x180 are pretty good)
            .dma_buf_len = 180, // The unit is stereo samples (4 bytes) (optimize for 533 usage)
        }, 0, NULL);
        if (ret == ESP_OK)
            ret = i2s_set_dac_mode(RG_AUDIO_USE_INT_DAC);
        if (ret != ESP_OK)
            state.last_error = esp_err_to_name(ret);
    #else
        state.last_error = "This device does not support internal DAC mode!";
    #endif
    }
    else if (state.device == 1)
    {
    #if RG_AUDIO_USE_EXT_DAC
        esp_err_t ret = i2s_driver_install(I2S_NUM_0, &(i2s_config_t){
            .mode = I2S_MODE_MASTER | I2S_MODE_TX,
            .sample_rate = sample_rate,
            .bits_per_sample = 16,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = 0, // ESP_INTR_FLAG_LEVEL1
            .dma_buf_count = 4, // Goal is to have ~800 samples over 2-8 buffers (3x270 or 5x180 are pretty good)
            .dma_buf_len = 180, // The unit is stereo samples (4 bytes) (optimize for 533 usage)
        #if CONFIG_IDF_TARGET_ESP32
            .use_apll = true, // External DAC may care about accuracy
        #endif
        }, 0, NULL);
        if (ret == ESP_OK)
        {
            ret = i2s_set_pin(I2S_NUM_0, &(i2s_pin_config_t) {
                .mck_io_num = GPIO_NUM_NC,
                .bck_io_num = RG_GPIO_SND_I2S_BCK,
                .ws_io_num = RG_GPIO_SND_I2S_WS,
                .data_out_num = RG_GPIO_SND_I2S_DATA,
                .data_in_num = GPIO_NUM_NC
            });
        }
        if (ret != ESP_OK)
            state.last_error = esp_err_to_name(ret);
    #else
        state.last_error = "This device does not support external DAC mode!";
    #endif
    }
    return state.last_error == NULL;
}

static bool driver_set_sample_rates(int sampleRate)
{
    return i2s_set_sample_rates(I2S_NUM_0, sampleRate) == ESP_OK;
}

static bool driver_deinit(void)
{
    i2s_driver_uninstall(I2S_NUM_0);
    if (state.device == 0)
    {
    #if RG_AUDIO_USE_INT_DAC
        if (RG_AUDIO_USE_INT_DAC & I2S_DAC_CHANNEL_RIGHT_EN)
            dac_output_disable(DAC_CHANNEL_1);
        if (RG_AUDIO_USE_INT_DAC & I2S_DAC_CHANNEL_LEFT_EN)
            dac_output_disable(DAC_CHANNEL_2);
        dac_i2s_disable();
    #endif
    }
    else if (state.device == 1)
    {
    #if RG_AUDIO_USE_EXT_DAC
        gpio_reset_pin(RG_GPIO_SND_I2S_BCK);
        gpio_reset_pin(RG_GPIO_SND_I2S_DATA);
        gpio_reset_pin(RG_GPIO_SND_I2S_WS);
    #endif
    }
    #ifdef RG_GPIO_SND_AMP_ENABLE
    gpio_reset_pin(RG_GPIO_SND_AMP_ENABLE);
    #endif
    return true;
}

static bool driver_submit(const rg_audio_frame_t *frames, size_t count)
{
    float volume = state.muted ? 0.f : (state.volume * 0.01f);
    rg_audio_frame_t buffer[180];
    size_t pos = 0;

    bool use_internal_dac = state.device == 0;

    for (size_t i = 0; i < count; ++i)
    {
        int left = frames[i].left * volume;
        int right = frames[i].right * volume;

        if (use_internal_dac)
        {
        #if RG_AUDIO_USE_INT_DAC == 1
            left = ((left + right) >> 1) + 0x8000; // the internal DAC expects unsigned data
            right = 0;
        #elif RG_AUDIO_USE_INT_DAC == 2
            left = 0;
            right = ((left + right) >> 1) + 0x8000; // the internal DAC expects unsigned data
        #elif RG_AUDIO_USE_INT_DAC == 3
            // In two channel mode we use left and right as a differential mono output to increase resolution.
            int sample = (left + right) >> 1;
            if (sample > 0x7F00)
            {
                left = 0x8000 + (sample - 0x7F00);
                right = -0x8000 + 0x7F00;
            }
            else if (sample < -0x7F00)
            {
                left = 0x8000 + (sample + 0x7F00);
                right = -0x8000 + -0x7F00;
            }
            else
            {
                left = 0x8000;
                right = -0x8000 + sample;
            }
        #endif
        }

        // Clipping   (not necessary, we have (int16 * vol) and volume is never more than 1.0)
        // if (left > 32767) left = 32767; else if (left < -32768) left = -32767;
        // if (right > 32767) right = 32767; else if (right < -32768) right = -32767;

        // Queue
        buffer[pos].left = left;
        buffer[pos].right = right;

        if (i == count - 1 || ++pos == RG_COUNT(buffer))
        {
            size_t written;
            if (i2s_write(I2S_NUM_0, (void *)buffer, pos * 4, &written, 1000) != ESP_OK)
                RG_LOGW("I2S Submission error! Written: %d/%d\n", written, pos * 4);
            pos = 0;
        }
    }
    return true;
}

static bool driver_set_mute(bool mute)
{
    i2s_zero_dma_buffer(I2S_NUM_0);
    #if defined(RG_GPIO_SND_AMP_ENABLE)
        gpio_set_direction(RG_GPIO_SND_AMP_ENABLE, GPIO_MODE_OUTPUT);
        #ifdef RG_GPIO_SND_AMP_ENABLE_INVERT
            gpio_set_level(RG_GPIO_SND_AMP_ENABLE, mute ? 1 : 0);
        #else
            gpio_set_level(RG_GPIO_SND_AMP_ENABLE, mute ? 0 : 1);
        #endif
    #endif
    state.muted = mute;
    return true;
}

static bool driver_set_volume(int volume)
{
    state.volume = volume;
    return true;
}

static const char *driver_get_error(void)
{
    return state.last_error;
}

const rg_audio_driver_t rg_audio_driver_i2s = {
    .name = "i2s",
    .init = driver_init,
    .deinit = driver_deinit,
    .submit = driver_submit,
    .set_mute = driver_set_mute,
    .set_volume = driver_set_volume,
    .set_sample_rate = driver_set_sample_rates,
    .get_error = driver_get_error,
};

#endif // RG_AUDIO_USE_INT_DAC || RG_AUDIO_USE_EXT_DAC
