#include "rg_system.h"
#include "rg_i2c.h"

#include <stdlib.h>

#if defined(ESP_PLATFORM) && defined(RG_GPIO_I2C_SDA) && defined(RG_GPIO_I2C_SCL)
#include <driver/i2c.h>
#include <esp_err.h>
#define USE_I2C_DRIVER 1
#endif

static bool i2c_initialized = false;

#define TRY(x)                 \
    if ((err = (x)) != ESP_OK) \
    {                          \
        goto fail;             \
    }


bool rg_i2c_init(void)
{
#if USE_I2C_DRIVER
    const i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = RG_GPIO_I2C_SDA,
        .scl_io_num = RG_GPIO_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    esp_err_t err = ESP_FAIL;

    if (i2c_initialized)
        return true;

    TRY(i2c_param_config(I2C_NUM_0, &i2c_config));
    TRY(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    RG_LOGI("I2C driver ready (SDA:%d SCL:%d).\n", i2c_config.sda_io_num, i2c_config.scl_io_num);
    i2c_initialized = true;
    return true;
fail:
    RG_LOGE("Failed to initialize I2C driver. err=0x%x\n", err);
#else
    RG_LOGE("I2C is not available on this device.\n");
#endif
    i2c_initialized = false;
    return false;
}

bool rg_i2c_deinit(void)
{
#if USE_I2C_DRIVER
    if (i2c_initialized && i2c_driver_delete(I2C_NUM_0) == ESP_OK)
        RG_LOGI("I2C driver terminated.\n");
#endif
    i2c_initialized = false;
    return true;
}

bool rg_i2c_read(uint8_t addr, int reg, void *read_data, size_t read_len)
{
#if USE_I2C_DRIVER
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = ESP_FAIL;

    if (!cmd || !i2c_initialized)
        goto fail;

    if (reg >= 0)
    {
        TRY(i2c_master_start(cmd));
        TRY(i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true));
        TRY(i2c_master_write_byte(cmd, (uint8_t)reg, true));
    }
    TRY(i2c_master_start(cmd));
    TRY(i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true));
    TRY(i2c_master_read(cmd, read_data, read_len, I2C_MASTER_LAST_NACK));
    TRY(i2c_master_stop(cmd));
    TRY(i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(500)));
    i2c_cmd_link_delete(cmd);
    return true;
fail:
    i2c_cmd_link_delete(cmd);
    RG_LOGE("Read from 0x%02X failed. reg=0x%02X, err=0x%03X, init=%d\n", addr, reg, err, i2c_initialized);
#endif
    return false;
}

bool rg_i2c_write(uint8_t addr, int reg, const void *write_data, size_t write_len)
{
#if USE_I2C_DRIVER
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = ESP_FAIL;

    if (!cmd || !i2c_initialized)
        goto fail;

    TRY(i2c_master_start(cmd));
    TRY(i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true));
    if (reg >= 0)
    {
        TRY(i2c_master_write_byte(cmd, (uint8_t)reg, true));
    }
    TRY(i2c_master_write(cmd, (void *)write_data, write_len, true));
    TRY(i2c_master_stop(cmd));
    TRY(i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(500)));
    i2c_cmd_link_delete(cmd);
    return true;
fail:
    i2c_cmd_link_delete(cmd);
    RG_LOGE("Write to 0x%02X failed. reg=0x%02X, err=0x%03X, init=%d\n", addr, reg, err, i2c_initialized);
#endif
    return false;
}

uint8_t rg_i2c_read_byte(uint8_t addr, uint8_t reg)
{
    uint8_t value = 0;
    return rg_i2c_read(addr, reg, &value, 1) ? value : 0;
}

bool rg_i2c_write_byte(uint8_t addr, uint8_t reg, uint8_t value)
{
    return rg_i2c_write(addr, reg, &value, 1);
}


#ifdef RG_I2C_GPIO_DRIVER
static bool gpio_initialized = false;
static uint8_t gpio_address = 0x00;

#if RG_I2C_GPIO_DRIVER == 1     // AW9523

static const uint8_t gpio_input_regs[2] = {0x00, 0x01};
static const uint8_t gpio_output_regs[2] = {0x02, 0x03};
static const uint8_t gpio_direction_regs[2] = {0x04, 0x05};
static const uint8_t gpio_init_sequence[][2] = {
    {0x7F, 0x00},   // Software reset (is it really necessary?)
    {0x11, 1 << 4}, // Push-Pull mode
};
static const uint8_t gpio_deinit_sequence[][2] = {};

#elif RG_I2C_GPIO_DRIVER == 2   // PCF9539

static const uint8_t gpio_input_regs[2] = {0x00, 0x01};
static const uint8_t gpio_output_regs[2] = {0x02, 0x03};
static const uint8_t gpio_direction_regs[2] = {0x06, 0x07};
static const uint8_t gpio_init_sequence[][2] = {};
static const uint8_t gpio_deinit_sequence[][2] = {};

#elif RG_I2C_GPIO_DRIVER == 3   // MCP23017

// Mappings when IOCON.BANK = 0 (which should be default on power-on)
static const uint8_t gpio_input_regs[2] = {0x12, 0x13};
static const uint8_t gpio_output_regs[2] = {0x14, 0x15};
static const uint8_t gpio_direction_regs[2] = {0x00, 0x01};
static const uint8_t gpio_init_sequence[][2] = {};
static const uint8_t gpio_deinit_sequence[][2] = {};

#else

#error "Unknown I2C GPIO Extender driver type!"

#endif


bool rg_i2c_gpio_init(void)
{
    if (gpio_initialized)
        return true;

    if (!i2c_initialized && !rg_i2c_init())
        return false;

    gpio_address = RG_I2C_GPIO_ADDR;
    gpio_initialized = true;

    // Configure extender-specific registers if needed (disable open-drain, interrupts, inversion, etc)
    for (size_t i = 0; i < RG_COUNT(gpio_init_sequence); ++i)
        rg_i2c_write_byte(gpio_address, gpio_init_sequence[i][0], gpio_init_sequence[i][1]);

    // Now set all pins of all ports as inputs and clear output latches
    for (size_t i = 0; i < RG_COUNT(gpio_direction_regs); ++i)
        rg_i2c_write_byte(gpio_address, gpio_direction_regs[i], 0xFF);
    for (size_t i = 0; i < RG_COUNT(gpio_output_regs); ++i)
        rg_i2c_write_byte(gpio_address, gpio_output_regs[i], 0x00);

    RG_LOGI("GPIO Extender ready (driver:%d, addr:0x%02X).", RG_I2C_GPIO_DRIVER, gpio_address);
    return true;
}

bool rg_i2c_gpio_deinit(void)
{
    if (gpio_initialized)
    {
        for (size_t i = 0; i < RG_COUNT(gpio_deinit_sequence); ++i)
            rg_i2c_write_byte(gpio_address, gpio_deinit_sequence[i][0], gpio_deinit_sequence[i][1]);
        // Should we reset all pins to be high impedance?
        gpio_initialized = false;
    }
    return true;
}

bool rg_i2c_gpio_set_direction(int pin, rg_gpio_mode_t mode)
{
    uint8_t reg = gpio_direction_regs[(pin >> 3) & 1], mask = 1 << (pin & 7);
    uint8_t val = rg_i2c_read_byte(gpio_address, reg);
    return rg_i2c_write_byte(gpio_address, reg, mode ? (val | mask) : (val & ~mask));
}

uint8_t rg_i2c_gpio_read_port(int port)
{
    return rg_i2c_read_byte(gpio_address, gpio_input_regs[port & 1]);
}

bool rg_i2c_gpio_write_port(int port, uint8_t value)
{
    return rg_i2c_write_byte(gpio_address, gpio_output_regs[port & 1], value);
}

int rg_i2c_gpio_get_level(int pin)
{
    return (rg_i2c_gpio_read_port(pin >> 3) >> (pin & 7)) & 1;
}

bool rg_i2c_gpio_set_level(int pin, int level)
{
    uint8_t reg = gpio_output_regs[(pin >> 3) & 1], mask = 1 << (pin & 7);
    uint8_t val = rg_i2c_read_byte(gpio_address, reg);
    return rg_i2c_write_byte(gpio_address, reg, level ? (val | mask) : (val & ~mask));
}
#endif
