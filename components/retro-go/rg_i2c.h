#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool rg_i2c_init(void);
bool rg_i2c_deinit(void);
bool rg_i2c_read(uint8_t addr, int reg, void *read_data, size_t read_len);
bool rg_i2c_write(uint8_t addr, int reg, const void *write_data, size_t write_len);
