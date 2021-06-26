#include <freertos/FreeRTOS.h>
#include <esp_system.h>
#include <driver/i2c.h>
#include <stdbool.h>
#include <stdint.h>

#include "rg_system.h"
#include "rg_i2c.h"

#define USE_I2C_DRIVER (RG_GPIO_I2C_SDA != GPIO_NUM_NC \
                     && RG_GPIO_I2C_SCL != GPIO_NUM_NC)

#define TRY(x) if ((err = (x)) != ESP_OK) { goto fail; }

static bool i2cStarted = false;


bool rg_i2c_init(void)
{
#if USE_I2C_DRIVER
    const i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = RG_GPIO_I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = RG_GPIO_I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 200000,
    };
    esp_err_t err = ESP_FAIL;

    TRY(i2c_param_config(I2C_NUM_0, &i2c_config));
    TRY(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    RG_LOGI("I2C driver ready.\n");
    i2cStarted = true;
    return true;
fail:
    RG_LOGE("Failed to initialize I2C driver. err=0x%x\n", err);
#else
    RG_LOGE("I2C driver is not available on this device.");
#endif
    return false;
}

bool rg_i2c_deinit(void)
{
#if USE_I2C_DRIVER
    if (i2c_driver_delete(I2C_NUM_0) == ESP_OK)
    {
        RG_LOGI("I2C driver terminated.\n");
        i2cStarted = false;
        return true;
    }
#endif
    return false;
}

bool rg_i2c_read(uint8_t addr, int reg, void *read_data, size_t read_len)
{
    esp_err_t err = ESP_FAIL;

    if (!i2cStarted && !rg_i2c_init())
        goto fail;

#if USE_I2C_DRIVER
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
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
#endif

fail:
    RG_LOGE("Read from 0x%02x failed. reg=%d, err=0x%x", addr, reg, err);
    return false;
}

bool rg_i2c_write(uint8_t addr, int reg, const void *write_data, size_t write_len)
{
    esp_err_t err = ESP_FAIL;

    if (!i2cStarted && !rg_i2c_init())
        goto fail;

#if USE_I2C_DRIVER
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
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
#endif

fail:
    RG_LOGE("Write to 0x%02x failed. reg=%d, err=0x%x", addr, reg, err);
    return false;
}
