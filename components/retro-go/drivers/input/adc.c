#include "rg_system.h"
#include "rg_input.h"

#include <driver/adc.h>
#include <string.h>

#define MAX_INPUTS 8

static uint8_t current_attenuation[2][8];

static struct {int16_t unit, channel;} inputs[MAX_INPUTS];
static size_t inputs_count;

static bool adc_set_atten(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten)
{
    if (current_attenuation[unit][channel] == atten)
    {
        return true;
    }
    else if (unit == ADC_UNIT_1 && adc1_config_channel_atten(channel, atten) != ESP_OK)
    {
        current_attenuation[unit][channel] = atten;
        return true;
    }
    else if (unit == ADC_UNIT_2 && adc2_config_channel_atten(channel, atten) == ESP_OK)
    {
        current_attenuation[unit][channel] = atten;
        return true;
    }
    return false;
}

static bool adc_get_raw(adc_unit_t unit, adc_channel_t channel, int *value)
{
    if (unit == ADC_UNIT_1)
    {
        *value = adc1_get_raw(channel);
        return true;
    }
    else if (unit == ADC_UNIT_2)
    {
        return adc2_get_raw(channel, ADC_WIDTH_MAX - 1, &value) == ESP_OK;
    }
    return false;
}

static bool driver_init(void)
{
    adc1_config_width(ADC_WIDTH_MAX - 1);
    inputs_count = 0;
}

static bool driver_deinit(void)
{
    // Nothing to do
}

static bool driver_add_input(int unit, int channel, int atten, int arg2, void *arg3, size_t *index)
{
    RG_ASSERT(inputs_count < MAX_INPUTS, "Too many inputs specified!");
    if (!adc_set_atten(unit, channel, atten))
    {
        RG_LOGE("adc_set_atten(%d, %d, %d) failed.", unit, channel, atten);
        return false;
    }
    inputs[inputs_count].unit = unit;
    inputs[inputs_count].channel = channel;
    if (index) *index = inputs_count;
    inputs_count++;
    return true;
}

static bool driver_read_inputs(size_t index, size_t count, int *values_out)
{
    RG_ASSERT(index < inputs_count, "index out of range");
    for (size_t i = index, max = RG_MIN(index + count, inputs_count); i < max; ++i)
        adc_get_raw(inputs[i].unit, inputs[i].channel, &values_out[i - index]);
    return true;
}

static bool driver_read(int unit, int channel, int atten, int arg2, void *arg3, int *value_out)
{
    // FIXME: Must apply read current attenuation, apply new one, then restore at the end
    if (adc_set_atten(unit, channel, atten))
        return adc_get_raw(unit, channel, value_out);
    return false;
}

const rg_input_driver_t RG_DRIVER_INPUT_ADC = {
    .name = "ADC",
    .init = driver_init,
    .deinit = driver_deinit,
    .add_input = driver_add_input,
    .read_inputs = driver_read_inputs,
    .read = driver_read,
};
