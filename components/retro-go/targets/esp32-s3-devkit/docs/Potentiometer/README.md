
**Author**: Tadeu Botelho
**E-mail**: tadeubotelho@hotmail.com
**GitHub**: https://github.com/TadeuBotelho
**Date**: February 11, 2026
**Version**: 1.0
**Tested Target**: esp32-s3-devkit

# Integrated Software

- Retro-Go 1.46 firmware fully operational
- Digital D-PAD configured on GPIOs
- Volume control via potentiometer (GPIO 7 - ADC1_CH6)
- Periodic reading with hysteresis (5%) and safe initialization verification

# Technical Characteristics

- Analog conversion: 0V → 0% volume, 3.3V → 100% volume
- Resolution: 12-bit ADC (0-4095 raw values)
- Update every 100ms
- 4-sample averaging for stability
- No reboots or panics during boot (with protection condition)

# Volume Control via Potentiometer - Technical Documentation

## Overview

This document describes the implementation of analog volume control through a potentiometer connected to the ESP32-S3, based on the existing Retro-Go battery reading architecture.

## Hardware

### Physical Connections

- **GPIO 7 (ADC1_CH6)**: Potentiometer wiper (center pin)
- **GND**: Potentiometer GND pin
- **VCC (3.3V)**: Potentiometer VCC pin

### Characteristics

- Input voltage: 0V to 3.3V
- ADC resolution: 12-bit (0-4095 raw values)
- Mapping: 0V = 0% volume, 3.3V = 100% volume

## Implementation

### 1. Configuration File (config.h)

The file [components/retro-go/targets/esp32-s3-devkit/config.h](components/retro-go/targets/esp32-s3-devkit/config.h) contains the following definitions:

c
// Volume Control Potentiometer (GPIO 7 / ADC1_CH6)
#define RG_POTENTIOMETER_ENABLED     1
#define RG_POTENTIOMETER_ADC_UNIT    ADC_UNIT_1
#define RG_POTENTIOMETER_ADC_CHANNEL ADC_CHANNEL_6
#define RG_POTENTIOMETER_ADC_ATTEN   ADC_ATTEN_DB_11
// Converts raw ADC value (0-4095) to volume percentage (0-100%)
#define RG_POTENTIOMETER_CALC_VOLUME(raw) ((int32_t)((raw) * 100 / 4095))
// Hysteresis threshold to prevent volume changes from small ADC fluctuations
#define RG_POTENTIOMETER_UPDATE_THRESHOLD 5
// Update interval in microseconds (update every 100ms)
#define RG_POTENTIOMETER_UPDATE_INTERVAL  (100 * 1000)

### 2. Reading Functions (rg_input.c)

#### rg_input_read_potentiometer_raw(int *out)

Function that reads the raw ADC value from the potentiometer. Similar to the battery reading function:

c
bool rg_input_read_potentiometer_raw(int *out)
{
    int raw_value = 0;

#ifdef ESP_PLATFORM
    // Read the potentiometer multiple times and average for stability
    for (int i = 0; i < 4; ++i)
    {
        int value = adc_get_raw(RG_POTENTIOMETER_ADC_UNIT, RG_POTENTIOMETER_ADC_CHANNEL);
        if (value < 0)
            return false;
        raw_value += value;
    }
    raw_value /= 4;
#else
    return false;
#endif

    if (out)
        *out = RG_POTENTIOMETER_CALC_VOLUME(raw_value);
    return true;
}

**Characteristics:**
- Reads the ADC 4 times and averages the result to increase stability
- Returns the value converted to volume percentage (0-100%)
- Follows the same pattern as battery reading

### 3. Input Task (input_task)

The input task was modified to include:

c
static void input_task(void *arg)
{
    // ... other variables ...
#ifdef RG_POTENTIOMETER_ENABLED
    int64_t next_potentiometer_update = 0;
    int last_potentiometer_volume = -1000;  // Initialize impossibly low
#endif

    while (input_task_running)
    {
        // ... gamepad and battery reading ...

#ifdef RG_POTENTIOMETER_ENABLED
        if (rg_system_timer() >= next_potentiometer_update)
        {
            int potentiometer_volume = 0;
            if (rg_input_read_potentiometer_raw(&potentiometer_volume))
            {
                // Hysteresis: only update if change >= RG_POTENTIOMETER_UPDATE_THRESHOLD
                if (abs(potentiometer_volume - last_potentiometer_volume) >= RG_POTENTIOMETER_UPDATE_THRESHOLD)
                {
                    int volume = RG_MAX(0, RG_MIN(100, potentiometer_volume));
                    // Critical check: only call if audio has been initialized
                    if (rg_audio_get_driver() != NULL)
                    {
                        rg_audio_set_volume(volume);
                        last_potentiometer_volume = volume;
                        RG_LOGD("Potentiometer adjusted volume to %d%%\n", volume);
                    }
                }
            }
            next_potentiometer_update = rg_system_timer() + RG_POTENTIOMETER_UPDATE_INTERVAL;
        }
#endif

        rg_task_delay(10);
    }
}

**Important Characteristics:**

1. **Hysteresis**: Volume is only updated if the change is >= 5%, preventing jitter caused by ADC noise
2. **Initialization Check**: Verifies rg_audio_get_driver() != NULL before calling rg_audio_set_volume() to avoid panic during boot
3. **Update Period**: Updates every 100ms, balancing responsiveness and CPU usage
4. **Compile-Time Condition**: All code is wrapped with #ifdef RG_POTENTIOMETER_ENABLED allowing easy disabling

### 4. ADC Initialization

In the function rg_input_init(), the potentiometer ADC is configured:

c
#ifdef RG_POTENTIOMETER_ENABLED
    RG_LOGI("Initializing ADC potentiometer driver...");
    if (RG_POTENTIOMETER_ADC_UNIT == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_MAX - 1);
        adc1_config_channel_atten(RG_POTENTIOMETER_ADC_CHANNEL, RG_POTENTIOMETER_ADC_ATTEN);
    }
    else if (RG_POTENTIOMETER_ADC_UNIT == ADC_UNIT_2)
    {
        adc2_config_channel_atten(RG_POTENTIOMETER_ADC_CHANNEL, RG_POTENTIOMETER_ADC_ATTEN);
    }
    else
    {
        RG_LOGE("Only ADC1 and ADC2 are supported for potentiometer driver!");
    }
#endif

## Functional Flow

┌─────────────────────────────────────────────────────────────┐
│  Potentiometer (0-3.3V) ──→ GPIO 7 (ADC1_CH6)              │
└────────────────────────────────┬──────────────────────────────┘
                                 │
                                 ▼
                    ┌──────────────────────────┐
                    │ ADC Reads Value (0-4095) │
                    └────────────┬─────────────┘
                                 │
                                 ▼ (Average 4 samples)
                    ┌──────────────────────────┐
                    │ Calc Volume %  (0-100)   │
                    └────────────┬─────────────┘
                                 │
                        ┌─────────┴─────────┐
                        │   Every 100ms     │
                        └────────┬──────────┘
                                 │
                        ┌────────▼────────┐
                        │  Check Hysteresis│
                        │  Δ >= 5% ?       │
                        └────────┬────────┘
                                 │
                    ┌────────────┴────────────┐
                    │ Audio Initialized?      │
                    │ (driver != NULL)        │
                    └────────────┬────────────┘
                                 │
                                 ▼
                    ┌──────────────────────────┐
                    │ rg_audio_set_volume()    │
                    │ Update UI (if applicable)│
                    └──────────────────────────┘

## Quick Disable

If you need to disable the potentiometer control quickly, modify in config.h:

c
#define RG_POTENTIOMETER_ENABLED     0  // Change to 0

Recompile and reflash. All related code will be removed by the preprocessor.

## Testing and Validation

### Log Verification

On startup, you will see:

[info] rg_input_init: Initializing ADC potentiometer driver...

When moving the potentiometer:

[debug] Potentiometer adjusted volume to 35%
[debug] Potentiometer adjusted volume to 42%

### Troubleshooting

| Problem | Possible Cause | Solution |
|----------|-----------------|---------|
| Volume does not change | GPIO 7 not properly connected | Check physical connection |
| Volume oscillates heavily | ADC noise | Increase RG_POTENTIOMETER_UPDATE_THRESHOLD |
| Boot panic | Audio not initialized in time | Check initialization order (already fixed) |
| ADC always reads 0 | Potentiometer not powered | Check VCC/GND connection |

## Comparison with Previous Solution

This solution follows the pattern established in Retro-Go for battery reading:

| Aspect | Battery | Potentiometer |
|---------|---------|---------------|
| ADC Frame | ADC1_CH3 | ADC1_CH6 |
| Update Frequency | 2 seconds | 100 ms |
| Averaging | 4 samples | 4 samples |
| Hysteresis | % level | % volume |
| Conversion | mV → % battery | RAW → % volume |

## Next Improvements (Optional)

1. **Calibration**: Allow user to calibrate potentiometer min/max
2. **Callback Function**: Add callback to notify volume changes
3. **Persistence**: Save last potentiometer position in settings
4. **Smoothing**: Implement smooth ramp between volumes to avoid clicks
5. **Visual Indicator**: Display potentiometer value (0-100%) in debug menu
6. **On-Screen Shortcuts**: Show volume percentage next to battery icon

## References

- [ESP-IDF ADC Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/adc.html)
- [Retro-Go rg_input.h](components/retro-go/rg_input.h)
- [Retro-Go rg_audio.h](components/retro-go/rg_audio.h)

