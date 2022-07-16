#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool rg_i2c_init(void);
bool rg_i2c_deinit(void);
bool rg_i2c_available(void);
bool rg_i2c_read(uint8_t addr, int reg, void *read_data, size_t read_len);
bool rg_i2c_write(uint8_t addr, int reg, const void *write_data, size_t write_len);
bool rg_i2c_write_byte(uint8_t addr, uint8_t reg, uint8_t value);
uint8_t rg_i2c_read_byte(uint8_t addr, uint8_t reg);

