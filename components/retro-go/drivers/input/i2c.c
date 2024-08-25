#include "rg_system.h"
#include "rg_input.h"
#include "rg_i2c.h"

#include <string.h>

#define MAX_INPUTS 16

static uint16_t inputs[MAX_INPUTS];
static size_t inputs_count;

static bool driver_init(void)
{
#ifdef RG_TARGET_QTPY_GAMER
    rg_i2c_gpio_init();
#else
    rg_i2c_init();
#endif
    inputs_count = 0;
}

static bool driver_deinit(void)
{
    // Nothing to do
}

static bool driver_add_input(int port, int bitmask, int arg1, int arg2, void *arg3, size_t *index)
{
    RG_ASSERT(inputs_count < MAX_INPUTS, "Too many inputs specified!");
    inputs[inputs_count] = bitmask;
    if (index)
        *index = inputs_count;
    inputs_count++;
    return true;
}

static bool driver_read_inputs(size_t index, size_t count, int *values_out)
{
    RG_ASSERT(index < inputs_count, "index out of range");
    uint64_t data = 0;
#ifdef RG_TARGET_QTPY_GAMER
    data = ~(rg_i2c_gpio_read_port(0) | rg_i2c_gpio_read_port(1) << 8);
#else
    if (!rg_i2c_read(0x20, -1, &data, 5))
        return false;
    data = ~(data >> 8);
#endif
    for (size_t i = index, max = RG_MIN(index + count, inputs_count); i < max; ++i)
        values_out[i - index] = data & inputs[i];
    return true;
}

static bool driver_read(int port, int bitmask, int arg1, int arg2, void *arg3, int *value_out)
{
    uint64_t data;
    if (!rg_i2c_read(0x20, -1, &data, 5))
        return false;
    data >>= arg1;
    *value_out = data & bitmask;
    return true;
}

const rg_input_driver_t RG_DRIVER_INPUT_I2C = {
    .name = "I2C",
    .init = driver_init,
    .deinit = driver_deinit,
    .add_input = driver_add_input,
    .read_inputs = driver_read_inputs,
    .read = driver_read,
};
