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

#ifdef RG_GPIO_SND_AMP_ENABLE_INVERT
#define MUTE_ENABLE 1
#define MUTE_DISABLE 0
#else
#define MUTE_ENABLE 0
#define MUTE_DISABLE 1
#endif

// We can safely assume that no application will submit more than 640 audio frames per call to
// driver_submit (32000/50). Using a single large buffer risks blocking the call needlessly because
// some apps submit more than once per cycle or there could be occasional jitter (early submission).
#define DMA_BUFFER_COUNT 4
#define DMA_BUFFER_LEN 180

#ifdef RG_AUDIO_CODEC_ES8311
#include "rg_i2c.h"
#include <string.h>
#include <stdio.h>

#ifndef RG_AUDIO_ES8311_ADDR
#define RG_AUDIO_ES8311_ADDR 0x18
#endif
#ifndef RG_AUDIO_ES8311_USE_MCLK
#define RG_AUDIO_ES8311_USE_MCLK 0
#endif

// Minimal ES8311 DAC bring-up: I2S slave, 16-bit. Internal clock is derived from the
// BCLK (SCLK) pin so no MCLK output is required: DIG_MCLK = BCLK * 8 = fs * 32 * 8 = 256 * fs.
// Register sequence and coefficients follow the esp-adf es8311 driver (FROM_SCLK_PIN path).
// Set RG_AUDIO_ES8311_USE_MCLK 1 to clock from the MCLK pin (256 * fs) instead.
static bool es8311_set_sample_rate(int sample_rate)
{
    const uint8_t addr = RG_AUDIO_ES8311_ADDR;
    // Rates the fs*32 BCLK works out for (16-bit stereo). Lower rates need 32-bit slots.
    uint8_t dac_osr = (sample_rate >= 22050) ? 0x10 : 0x20;
    uint8_t adc_osr = 0x10;
#if RG_AUDIO_ES8311_USE_MCLK
    uint8_t pre_multi = 0; // x1: MCLK pin already provides 256 * fs
#else
    uint8_t pre_multi = 3; // x8: DIG_MCLK = BCLK * 8 (esp-adf forces this for SCLK source)
    if (sample_rate < 22050)
    {
        RG_LOGW("ES8311: %d Hz not supported in BCLK-derived clock mode!", sample_rate);
        return false;
    }
#endif

    int reg06 = rg_i2c_read_byte(addr, 0x06);
    int reg07 = rg_i2c_read_byte(addr, 0x07);
    if (reg06 < 0 || reg07 < 0)
        return false;

    rg_i2c_write_byte(addr, 0x02, (0 << 5) | (pre_multi << 3)); // pre_div 1, encoded pre_multi
    rg_i2c_write_byte(addr, 0x03, (0 << 6) | adc_osr);        // single speed, adc_osr
    rg_i2c_write_byte(addr, 0x04, dac_osr);                   // dac_osr
    rg_i2c_write_byte(addr, 0x05, (0 << 4) | 0);              // adc_div 1, dac_div 1
    rg_i2c_write_byte(addr, 0x06, (reg06 & 0xE0) | (4 - 1));  // bclk_div 4 (master mode only)
    rg_i2c_write_byte(addr, 0x07, (reg07 & 0xC0) | 0x00);     // lrck_h (master mode only)
    rg_i2c_write_byte(addr, 0x08, 0xFF);                      // lrck_l (master mode only)
    return true;
}

static void es8311_codec_init(int sample_rate)
{
    const uint8_t addr = RG_AUDIO_ES8311_ADDR;

    if (!rg_i2c_init())
        return;
    if (rg_i2c_read_byte(addr, 0xFD) != 0x83) // chip ID1
    {
        RG_LOGE("ES8311 codec not found at 0x%02X!", addr);
        return;
    }
    rg_i2c_write_byte(addr, 0x00, 0x1F); // reset
    rg_usleep(20 * 1000);
    rg_i2c_write_byte(addr, 0x00, 0x00); // out of reset, I2S slave
#if RG_AUDIO_ES8311_USE_MCLK
    rg_i2c_write_byte(addr, 0x01, 0x3F); // enable all clocks, internal MCLK from MCLK pin
#else
    rg_i2c_write_byte(addr, 0x01, 0xBF); // enable all clocks, internal MCLK from BCLK (SCLK) pin
#endif
    es8311_set_sample_rate(sample_rate);
    rg_i2c_write_byte(addr, 0x09, 3 << 2); // SDP-in 16-bit I2S
    rg_i2c_write_byte(addr, 0x0A, 3 << 2); // SDP-out 16-bit I2S
    rg_i2c_write_byte(addr, 0x0D, 0x01); // power up analog circuitry
    rg_i2c_write_byte(addr, 0x0E, 0x02); // enable analog PGA/ADC modulator
    rg_i2c_write_byte(addr, 0x12, 0x00); // power up DAC
    rg_i2c_write_byte(addr, 0x13, 0x10); // enable output to HP drive
    rg_i2c_write_byte(addr, 0x1C, 0x6A); // ADC EQ bypass, cancel DC offset
    rg_i2c_write_byte(addr, 0x37, 0x08); // bypass DAC equalizer
    rg_i2c_write_byte(addr, 0x00, 0x80); // power on
    rg_i2c_write_byte(addr, 0x32, 0xBF); // DAC volume 0dB
    RG_LOGI("ES8311 codec initialized (fs=%d, clk=%s)", sample_rate,
            RG_AUDIO_ES8311_USE_MCLK ? "MCLK pin" : "BCLK-derived");
    // Diagnostic register dump (clock manager + power + DAC)
    static const uint8_t dump_regs[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x09, 0x0A,
                                        0x0D, 0x0E, 0x12, 0x13, 0x31, 0x32};
    char dump[128] = {0};
    for (size_t i = 0; i < RG_COUNT(dump_regs); ++i)
        sprintf(dump + strlen(dump), "%02X:%02X ", dump_regs[i], rg_i2c_read_byte(addr, dump_regs[i]));
    RG_LOGI("ES8311 regs: %s", dump);
}
#endif // RG_AUDIO_CODEC_ES8311

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
            .dma_buf_count = DMA_BUFFER_COUNT,
            .dma_buf_len = DMA_BUFFER_LEN,
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
            .dma_buf_count = DMA_BUFFER_COUNT,
            .dma_buf_len = DMA_BUFFER_LEN,
        #if CONFIG_IDF_TARGET_ESP32
            .use_apll = true, // External DAC may care about accuracy
        #endif
        #ifdef RG_GPIO_SND_I2S_MCLK
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        #endif
        }, 0, NULL);
        if (ret == ESP_OK)
        {
            ret = i2s_set_pin(I2S_NUM_0, &(i2s_pin_config_t) {
            #ifdef RG_GPIO_SND_I2S_MCLK
                .mck_io_num = RG_GPIO_SND_I2S_MCLK,
            #else
                .mck_io_num = GPIO_NUM_NC,
            #endif
                .bck_io_num = RG_GPIO_SND_I2S_BCK,
                .ws_io_num = RG_GPIO_SND_I2S_WS,
                .data_out_num = RG_GPIO_SND_I2S_DATA,
                .data_in_num = GPIO_NUM_NC
            });
        }
        #ifdef RG_AUDIO_CODEC_ES8311
        if (ret == ESP_OK)
            es8311_codec_init(sample_rate);
        #endif
        if (ret != ESP_OK)
            state.last_error = esp_err_to_name(ret);
    #else
        state.last_error = "This device does not support external DAC mode!";
    #endif
    }
    #ifdef RG_GPIO_SND_AMP_ENABLE
        gpio_reset_pin(RG_GPIO_SND_AMP_ENABLE);
        gpio_set_level(RG_GPIO_SND_AMP_ENABLE, MUTE_ENABLE);
        gpio_set_direction(RG_GPIO_SND_AMP_ENABLE, GPIO_MODE_OUTPUT);
    #endif
    return state.last_error == NULL;
}

static bool driver_set_sample_rates(int sampleRate)
{
    bool ok = i2s_set_sample_rates(I2S_NUM_0, sampleRate) == ESP_OK;
#ifdef RG_AUDIO_CODEC_ES8311
    if (ok && state.device == 1)
        es8311_set_sample_rate(sampleRate);
#endif
    return ok;
}

static bool driver_deinit(void)
{
    i2s_driver_uninstall(I2S_NUM_0);
    if (state.device == 0)
    {
    #if RG_AUDIO_USE_INT_DAC
        i2s_set_dac_mode(I2S_DAC_CHANNEL_DISABLE);
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
    bool use_internal_dac = state.device == 0;
    rg_audio_frame_t buffer[DMA_BUFFER_LEN];
    size_t pos = 0;

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
    #ifdef RG_GPIO_SND_AMP_ENABLE
    gpio_set_level(RG_GPIO_SND_AMP_ENABLE, mute ? MUTE_ENABLE : MUTE_DISABLE);
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
