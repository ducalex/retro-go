#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool rg_i2c_init(void);
bool rg_i2c_deinit(void);
bool rg_i2c_read(uint8_t addr, int reg, void *read_data, size_t read_len);
bool rg_i2c_write(uint8_t addr, int reg, const void *write_data, size_t write_len);
int rg_i2c_read_byte(uint8_t addr, uint8_t reg);
bool rg_i2c_write_byte(uint8_t addr, uint8_t reg, uint8_t value);


// GPIO extender
typedef enum
{
    RG_GPIO_OUTPUT,
    RG_GPIO_INPUT,
    RG_GPIO_INPUT_PULLUP,
} rg_gpio_mode_t;

bool rg_i2c_gpio_init(void);
bool rg_i2c_gpio_deinit(void);
bool rg_i2c_gpio_configure_port(int port, uint8_t mask, rg_gpio_mode_t mode);
int rg_i2c_gpio_read_port(int port);
bool rg_i2c_gpio_write_port(int port, uint8_t value);
// For the following functions `pin` is calculated as such: (port_num * 8) + port_pin_num
bool rg_i2c_gpio_set_direction(int pin, rg_gpio_mode_t mode);
int rg_i2c_gpio_get_level(int pin);
bool rg_i2c_gpio_set_level(int pin, int level);
