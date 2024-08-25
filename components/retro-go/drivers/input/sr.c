#include "rg_system.h"
#include "rg_input.h"

#include <driver/gpio.h>
#include <string.h>

#define MAX_INPUTS 16

static uint16_t inputs[MAX_INPUTS];
static size_t inputs_count;

static bool driver_init(void)
{
    gpio_set_direction(RG_GPIO_GAMEPAD_CLOCK, GPIO_MODE_OUTPUT);
    gpio_set_direction(RG_GPIO_GAMEPAD_LATCH, GPIO_MODE_OUTPUT);
    gpio_set_direction(RG_GPIO_GAMEPAD_DATA, GPIO_MODE_INPUT);
    gpio_set_level(RG_GPIO_GAMEPAD_LATCH, 0);
    gpio_set_level(RG_GPIO_GAMEPAD_CLOCK, 1);
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
    if (index) *index = inputs_count;
    inputs_count++;
    return true;
}

static bool driver_read_inputs(size_t index, size_t count, int *values_out)
{
    RG_ASSERT(index < inputs_count, "index out of range");
    uint32_t buttons = 0;
    gpio_set_level(RG_GPIO_GAMEPAD_LATCH, 0);
    rg_usleep(5);
    gpio_set_level(RG_GPIO_GAMEPAD_LATCH, 1);
    rg_usleep(1);
    for (int i = 0; i < 16; i++)
    {
        buttons |= gpio_get_level(RG_GPIO_GAMEPAD_DATA) << (15 - i);
        gpio_set_level(RG_GPIO_GAMEPAD_CLOCK, 0);
        rg_usleep(1);
        gpio_set_level(RG_GPIO_GAMEPAD_CLOCK, 1);
        rg_usleep(1);
    }
    for (size_t i = index, max = RG_MIN(index + count, inputs_count); i < max; ++i)
        values_out[i - index] = (buttons & inputs[i]) == inputs[i];
    return true;
}

static bool driver_read(int port, int bitmask, int arg1, int arg2, void *arg3, int *value_out)
{
    return false;
}

const rg_input_driver_t RG_DRIVER_INPUT_SR = {
    .name = "SHIFTREG",
    .init = driver_init,
    .deinit = driver_deinit,
    .add_input = driver_add_input,
    .read_inputs = driver_read_inputs,
    .read = driver_read,
};
