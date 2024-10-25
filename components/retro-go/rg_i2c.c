#include "rg_system.h"
#include "rg_i2c.h"

#include <stdlib.h>

#if defined(ESP_PLATFORM) && defined(RG_GPIO_I2C_SDA) && defined(RG_GPIO_I2C_SCL)
#include <driver/i2c.h>
#include <esp_err.h>
#define USE_I2C_DRIVER 1
#endif

static bool i2c_initialized = false;
static bool gpio_extender_initialized = false;
static uint8_t gpio_extender_address = 0x00;

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

#if RG_I2C_DRIVER == 1

#define AW9523_REG_INPUT0    0x00 ///< Register for reading input values
#define AW9523_REG_OUTPUT0   0x02 ///< Register for writing output values
#define AW9523_REG_POLARITY0 0x04 ///< Register for polarity inversion of inputs
#define AW9523_REG_CONFIG0   0x06 ///< Register for configuring direction

#else

#define AW9523_REG_CHIPID     0x10 ///< Register for hardcode chip ID
#define AW9523_REG_SOFTRESET  0x7F ///< Register for soft resetting
#define AW9523_REG_INPUT0     0x00 ///< Register for reading input values
#define AW9523_REG_OUTPUT0    0x02 ///< Register for writing output values
#define AW9523_REG_CONFIG0    0x04 ///< Register for configuring direction
#define AW9523_REG_INTENABLE0 0x06 ///< Register for enabling interrupt
#define AW9523_REG_GCR        0x11 ///< Register for general configuration
#define AW9523_REG_LEDMODE    0x12 ///< Register for configuring const current

#endif


bool rg_i2c_gpio_init(void)
{
    if (gpio_extender_initialized)
        return true;

    if (!i2c_initialized && !rg_i2c_init())
        return false;

    gpio_extender_initialized = true;
#if RG_I2C_DRIVER == 1
    gpio_extender_address = 0x74;

    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_OUTPUT0, 0xFF);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_OUTPUT0 + 1, 0xFF);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_POLARITY0, 0x00);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_POLARITY0 + 1, 0x00);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_CONFIG0,  0xFF);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_CONFIG0 + 1,  0xFF);
#else
    gpio_extender_address = 0x58;

    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_SOFTRESET, 0);
    rg_usleep(10 * 1000);

    uint8_t id = rg_i2c_read_byte(gpio_extender_address, AW9523_REG_CHIPID);
    if (id != 0x23)
    {
        RG_LOGE("AW9523 invalid ID 0x%x found", id);
        return false;
    }

    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_CONFIG0, 0xFF);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_CONFIG0 + 1, 0xFF);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_LEDMODE, 0xFF);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_LEDMODE + 1, 0xFF);
    rg_i2c_write_byte(gpio_extender_address, AW9523_REG_GCR, 1 << 4);
#endif
    return true;
}

bool rg_i2c_gpio_deinit(void)
{
    gpio_extender_initialized = false;
    gpio_extender_address = 0;
    return true;
}

bool rg_i2c_gpio_set_direction(int pin, int mode)
{
    uint8_t reg = AW9523_REG_CONFIG0 + (pin >> 3), mask = 1 << (pin & 7);
    uint8_t val = rg_i2c_read_byte(gpio_extender_address, reg);
    return rg_i2c_write_byte(gpio_extender_address, reg, mode ? (val | mask) : (val & ~mask));
}

uint8_t rg_i2c_gpio_read_port(int port)
{
    return rg_i2c_read_byte(gpio_extender_address, AW9523_REG_INPUT0 + port);
}

bool rg_i2c_gpio_write_port(int port, uint8_t value)
{
    return rg_i2c_write_byte(gpio_extender_address, AW9523_REG_OUTPUT0 + port, value);
}

int rg_i2c_gpio_get_level(int pin)
{
    return (rg_i2c_gpio_read_port(pin >> 3) >> (pin & 7)) & 1;
}

bool rg_i2c_gpio_set_level(int pin, int level)
{
    uint8_t reg = AW9523_REG_OUTPUT0 + (pin >> 3), mask = 1 << (pin & 7);
    uint8_t val = rg_i2c_read_byte(gpio_extender_address, reg);
    return rg_i2c_write_byte(gpio_extender_address, reg, level ? (val | mask) : (val & ~mask));
}
