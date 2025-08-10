# Table of contents
- [Prerequisites](#prerequisites)
- [Targets](#targets)
- [Porting](#porting)
- [Patching](#patching)


# Introduction

This document describes the process of adding support for a new ESP32 device (what retro-go calls a *target*).


# Prerequisites

## Hardware prerequisites
Retro-Go will run on any device that use an esp32 chip (including variants like s2, s3, p4), as long as it has PSRAM (also called SPIRAM or external RAM). You will also need a serial cable (most esp32 devices expose the serial interface over USB).


## Software prerequisites
You will need a working installation of esp-idf. Retro-Go supports many versions, refer to [BUILDING.md](BUILDING.md) for more information. If you are new to esp-idf/esp32 development, you might want to try flashing a few sample programs to your device to make sure that everything works, before attempting a complicated project like retro-go.


# Targets
A target in retro-go is a set of files that describes a hardware device (drivers to use, GPIOs, button mapping, etc). Targets are folders located in `components/retro-go/targets/` and usually contain the following files:

| Name          | Description |
|---------------|-------------|
| config.h      | This is the retro-go configuration file, it describes your hardware to retro-go |
| env.py        | This is imported by rg_tool.py, it is used to set environment variables required by tooling, such as the esp32 chip type or baudrates or binary formats |
| sdkconfig     | This is the esp-idf configuration file for your board/device |


# Porting
Retro-Go uses reusable hardware components, so it is easy to set it up for your own hardware.

To get started, locate a target that is the most similar to your device and use it as a starting point. Clone the target's folder and give it a new name, then add your new target to `components/retro-go/config.h`.


## Target files

### config.h

This file contains the bulk of your configurations. There is currently no exhaustive documentation of all the options you can define in this file, but you can refer to odroid-go's config.h that contains most of them.

Don't forget to change `RG_TARGET_NAME` to match the name of your target folder.

#### Display

Retro-Go has a single output driver: ILI9341/ST7789. Thankfully most screens out there use this controller!

You will need to define the correct pinout (RG_GPIO_LCD_*) and the corrent init sequence (RG_SCREEN_INIT). To find the required sequence of commands you can refer to other example code meant for your display/device.


If you aren't using the ILI9341/ST7789 screen driver, you will need to write your own.

You can clone `components/retro-go/drivers/ili9341.h` to use as a starting point. Don't forget to add it to the top of `rg_display.c`:

```c
#if RG_SCREEN_DRIVER == 0 /* ILI9341/ST7789 */
#include "drivers/display/ili9341.h"
#elif RG_SCREEN_DRIVER == 2             // <--
#include "drivers/display/my-driver.h"  // <--
#elif RG_SCREEN_DRIVER == 99
#include "drivers/display/sdl2.h"
#else
#include "drivers/display/dummy.h"
#endif
```

#### Input

Retro-Go has five input drivers:
- GPIO: This is a button connected to a pin of the esp32
- ADC:  This is usually used with a voltage divider to put multiple buttons on a single esp32 pin
- I2C:  Also known as a GPIO extender, retro-go supports many chips (AW9523, PCF9539, MCP23017, PCF8575)
- Shift register: Like a SNES controller or a 74HC165
- Virtual: If your device doesn't have enough button, you can map combos to the missing keys. eg start+select = menu

You can combine any of them by defining the appropriate `RG_GAMEPAD_*_MAP` in your `config.h`. Refer to `rg_input.h` or other targets to see how configuration is done.


### sdkconfig

This file is used by ESP-IDF to build the code for the ESP32 itself.

Retro-Go, for the most part, doesn't care about the sdkconfig and its content will depend entirely the chip/board used. But there are things that you should keep in mind:
- The main task stack size has to be at least 8KB for retro-go
- The CPU frequency should be set to the maximum possible and power management be disabled
- SPIRAM should be enabled and configured correctly for your device

ESP-IDF provides a tool to edit it, namely `menuconfig`, but to use it in retro-go you must follow the following steps:

1. Build the launcher for your target (this will make sure you have the correct ESP32 board selected and generate a default sdkconfig)
    - `./rg_tool.py clean`
    - `./rg_tool.py --target my-target build launcher`
2. Enter the launcher directory: `cd launcher`
3. Run `idf.py menuconfig` and make the changes that you need. Make sure to save the changes and exit.
4. Optionally test the app with the new config (but do NOT run `rg_tool.py clean` at this point, your new config will be deleted)
    - `cd ..`
    - `./rg_tool.py --target my-target run launcher`
5. When you're satisfied, copy the `sdkconfig` file from the launcher to the target folder, so that it's used by all apps
    - `cd ..`
    - `mv  -f launcher/sdkconfig components/retro-go/targets/my-target/sdkconfig`

Note that any time you modify your target's `sdkconfig` file, you must run `./rg_tool.py clean` to ensure it will be applied.


### env.py

Change the target board in `env.py` to match yours


After completing all these steps, you should be able to build your apps with `rg_tool`. See [BUILDING.md](BUILDING.md#flashing-an-image-for-the-first-time) for more info about this and flashing. **Make sure to use the steps for `.img`, NOT `.fw`**


## Patching

Not everything can be done through `config.h`. Sometimes you have to patch retro-go to add special code for your hardware. Any such modification will be harder to upstream, if that is your goal, but they are easy to perform.

The build system automatically defines a constant for the target in use, eg `RG_TARGET_MY_TARGET` if your target is named `my-target` (the name is capitalized and `-` is replaced with `_`).

Anywhere in retro-go you can add code wrapped in a #ifdef block that will apply only to your target:

````c
#ifdef RG_TARGET_MY_TARGET
// do stuff
#endif
````
