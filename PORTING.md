# Table of contents
- [Prerequisites](#prerequisites)
- [Targets](#targets)
- [Porting](#porting)
- [Patching](#patching)


# Introduction

This document describes the process of adding support for a new ESP32 device (what retro-go calls a *target*).


# Prerequisites
Before doing anything, make sure that your development environment is working properly and that you can build retro-go for the default target. Please read [BUILDING.md](BUILDING.md) carefully for more information.


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

This file contains the bulk of your configurations.

First, change `RG_TARGET_NAME` to match the name of your target folder.

Most of it, you will need to figure out the correct parameters for (eg. Storage and Audio)


##### Display

If you aren't using the ILI9341 screen driver, you will need to change the `SCREEN_DRIVER` parameter. (Otherwise, just change the following settings and continue).


(You will find more display configuration in the `SPI Display` section below)


For now, only the ILI9341 is included. Increment the number. Then in `components/retro-go/rg_display.c`, find this part
```
#if RG_SCREEN_DRIVER == 0 /* ILI9341 */
#include "drivers/display/ili9341.h"
#elif RG_SCREEN_DRIVER == 99
#include "drivers/display/sdl2.h"
#else
#include "drivers/display/dummy.h"
#endif
```


Add a `#elif` for your SCREEN_DRIVER. In the body, use `#include "drivers/display/(YOUR DISPLAY DRIVER).h"` (eg. `st7789.h`).


You will need to create that file for your display. Unfortunately, there is no one-for-all way to make this. It will need the same template as the other display drivers there, but it will differ for each display. To start, check the Retro-Go issues on GitHub and try searching on Google.


Make this driver in `components/retro-go/drivers/display`


##### Input

Back in `config.h`, you will see the configuration for an I2C gamepad. If you aren't using that, you can make your own parameters based on the existing input forms in `components/retro-go/rg_input.c`


You can also write your own input driver for unique input forms. Just look at the existing code in `rg_input.c` and match that


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
