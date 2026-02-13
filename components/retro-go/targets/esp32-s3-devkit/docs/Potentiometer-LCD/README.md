# Potentiometer-based LCD Brightness Control (ESP32-S3 devkit)

## Summary

-   Implementation of an analog potentiometer-based screen brightness
    control (GPIO 6 / ADC1_CH5) for the `esp32-s3-devkit` target in
    Retro-Go v1.46.
-   This solution mirrors the existing volume control implementation
    (potentiometer on GPIO 7 / ADC1_CH6).

------------------------------------------------------------------------

## What Was Changed / Added

### Target Configuration Defines

File: `components/retro-go/targets/esp32-s3-devkit/config.h`

-   `RG_BRIGHTNESS_POTENTIOMETER_ENABLED` --- Enable the feature\
-   `RG_BRIGHTNESS_POTENTIOMETER_ADC_UNIT` --- ADC unit
    (e.g. `ADC_UNIT_1`)\
-   `RG_BRIGHTNESS_POTENTIOMETER_ADC_CHANNEL` --- ADC channel
    (`ADC_CHANNEL_5`)\
-   `RG_BRIGHTNESS_POTENTIOMETER_ADC_ATTEN` --- Attenuation
    (`ADC_ATTEN_DB_11`)\
-   `RG_BRIGHTNESS_POTENTIOMETER_CALC_BRIGHTNESS(raw)` --- Conversion
    macro (raw → 0--100%)\
-   `RG_BRIGHTNESS_POTENTIOMETER_UPDATE_THRESHOLD` --- Hysteresis
    threshold (e.g. 5%)\
-   `RG_BRIGHTNESS_POTENTIOMETER_UPDATE_INTERVAL` --- Update interval
    (e.g. 100 ms)

------------------------------------------------------------------------

### Modifications in `components/retro-go/rg_input.c`

-   Added `rg_input_read_brightness_potentiometer_raw()`

    -   Reads 4 samples from ADC1_CH5\
    -   Averages samples\
    -   Converts result to 0--100%

-   Added periodic block inside `input_task()`:

    -   Runs every `RG_BRIGHTNESS_POTENTIOMETER_UPDATE_INTERVAL`

    -   Applies hysteresis logic

    -   When delta exceeds
        `RG_BRIGHTNESS_POTENTIOMETER_UPDATE_THRESHOLD`, calls:

        ``` c
        rg_display_set_backlight(clamped_value);
        ```

-   Added safety check before setting brightness:

    ``` c
    if (rg_display_get_backlight() >= 0)
    ```

    Ensures the display driver is initialized before adjusting
    brightness.

-   Added ADC initialization in `rg_input_init()`:

    -   Configure ADC1 width\
    -   Configure attenuation\
    -   Use ADC2 when applicable

------------------------------------------------------------------------

## Wiring Diagram (Summary)

Standard 3-pin potentiometer:

-   Middle pin (wiper) → GPIO 6 (ADC1_CH5)\
-   One outer pin → 3.3V\
-   Other outer pin → GND

Recommended value: **10kΩ**

If noise is observed: - Add 100 nF capacitor between wiper and GND -
Improve grounding - Increase sample averaging

------------------------------------------------------------------------

## Behavior and Design Decisions

-   4-sample averaging reduces ADC noise.
-   Configurable hysteresis prevents flickering caused by small ADC
    fluctuations.
-   Minimum brightness is clamped to 1% to prevent accidental full
    blackout.
-   Brightness updates only occur after display initialization to avoid
    startup instability.

------------------------------------------------------------------------

## How to Build and Test

From project root:

``` bash
python rg_tool.py --target=esp32-s3-devkit clean
python rg_tool.py --target=esp32-s3-devkit build-img
```

Flash example:

``` bash
esptool.py -p /dev/ttyUSB0 -b 460800 --chip esp32s3 write_flash 0x0 build/retro-go_esp32-s3-devkit.img
idf.py monitor
```

Look for log messages such as:

-   `Initializing ADC brightness potentiometer driver...`
-   `Brightness potentiometer adjusted backlight to %d%%`

------------------------------------------------------------------------

## Debugging Tips

If brightness does not respond:

-   Verify wiring (wiper → GPIO6)
-   Confirm `RG_BRIGHTNESS_POTENTIOMETER_ENABLED` is defined
-   Enable debug logs and check ADC initialization
-   Add small decoupling capacitor (100 nF) if signal is noisy

------------------------------------------------------------------------

## Modified files

Target config: config.h — added brightness potentiometer defines.
Input driver: rg_input.c — added ADC init, rg_input_read_brightness_potentiometer_raw(), and input_task() brightness handling.

------------------------------------------------------------------------

## Future Improvements

-   Implement smooth fade transitions when brightness changes
-   Add menu configuration options (invert direction, change range
    limits)
-   Allow optional non-linear brightness curve

------------------------------------------------------------------------

## Test:

Potentiometer-based Model DMG-01 - LCD Brightness Control (ESP32-S3 devkit) for Retro Go Target "esp32-s3-devkit":
- https://www.youtube.com/watch?v=7YaU5v6iK4M

------------------------------------------------------------------------

## License / Credit

Implementation follows Retro-Go project structure and coding patterns.\
May be adapted for other targets by adjusting GPIO and ADC channel
definitions.
