#include "rg_system.h"
#include "rg_input.h"

#include <driver/gpio.h>
#include <string.h>

#define MAX_INPUTS 16

static uint8_t inputs[MAX_INPUTS];
static size_t inputs_count;

static bool driver_init(void)
{
    inputs_count = 0;
}

static bool driver_deinit(void)
{
    // Nothing to do
}

static bool driver_add_input(int port, int pin_num, int pull_mode, int arg2, void *arg3, size_t *index)
{
    RG_ASSERT(inputs_count < MAX_INPUTS, "Too many inputs specified!");
    RG_ASSERT_ARG(port == 0 && pin_num >= 0 && pin_num < 255);

    if (gpio_set_direction(pin_num, GPIO_MODE_INPUT) == ESP_OK)
    {
        gpio_set_pull_mode(pin_num, pull_mode);
        inputs[inputs_count] = pin_num;
        if (index) *index = inputs_count;
        inputs_count++;
        return true;
    }

    RG_LOGE("gpio_set_direction(%d) failed", pin_num);
    return false;
}

static bool driver_read_inputs(size_t index, size_t count, int *values_out)
{
    RG_ASSERT(index < inputs_count, "index out of range");
    for (size_t i = index, max = RG_MIN(index + count, inputs_count); i < max; ++i)
        values_out[i - index] = gpio_get_level(inputs[i]);
    return true;
}

static bool driver_read(int port, int pin_num, int pull_mode, int arg2, void *arg3, int *value_out)
{
    RG_ASSERT_ARG(port == 0 && pin_num >= 0 && pin_num < 255);
    // TODO: Must read current direction and pull mode, then set input/pull and read, then restore direction/pull
    return false;
}

const rg_input_driver_t RG_DRIVER_INPUT_GPIO = {
    .name = "GPIO",
    .init = driver_init,
    .deinit = driver_deinit,
    .add_input = driver_add_input,
    .read_inputs = driver_read_inputs,
    .read = driver_read,
};
