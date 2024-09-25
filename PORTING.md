# Patching Retro-Go
Adjust retro-go to use your drivers and hardware




# Prerequisites
You will need a working installation of [esp-idf](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/get-started/index.html#get-started-get-prerequisites). Versions 4.3 to 5.2 are supported.


Before using your own patch of retro-go, make a clean build of retro go using bare configurations
```
./rg_tool.py build-img launcher --target esp32s3-devkit-c
```

Since we aren't using other setups, like ODROID-GO, we will be building `/img` files, not `.fw`. Use `./rg_tool.py build-img` to build imgs.


**Make sure a basic setup is working before continuing to patch the code for your setup** 

If it doesn't, refer to [BUILDING.md](BUILDING.md). Also check your `esp-idf` installation and make sure you have it set up in your environment (run `idf.py --version` to check)
```
./rg_tool.py build-img launcher --target esp32s3-devkit-c
```

# Patching
Retro-Go uses reusable hardware components, so it is easy to set it up for your own hardware.


You will find the targets (configs for each device) in `components/retro-go/targets`


To make a base one, copy `esp32s3-devkit-c` and rename it.


## config.h


`config.h` has the bulk of your configurations.


First, change `RG_TARGET_NAME` to match the name of your target folder.




Most of it, you will need to figure out the correct parameters for (eg. Storage and Audio)

**Display**

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


___
**Input**

Back in `config.h`, you will see the configuration for an I2C gamepad. If you aren't using that, you can make your own parameters based on the existing input forms in `components/retro-go/rg_input.c`


You can also write your own input driver for unique input forms. Just look at the existing code in `rg_input.c` and match that




### sdkconfig
`sdkconfig` is used by ESP-IDF to build the code for the ESP32 itself. It is hard to manually write it all out, so you can use `menuconfig`


For this, go back to your root folder. Build the launcher for your target (this will make sure you have the correct ESP32 board selected).
```
./rg_tool.py clean
./rg_tool.py build launcher --target (YOUR TARGET)
```


Then `cd` into `launcher` and run `idf.py menuconfig`. Navigate to Serial Flasher Config and adjust it all to your board. Make sure to save the changes and exit.


Use this generated sdkconfig in your target with
```
cd ..
mv  -f launcher/sdkconfig components/retro-go/targets/retrotoids/sdkconfig
```
This will configure ESP-IDF to run on your board


### env.py


Change the target board in `env.py` to match yours


After completing all these steps, you should be able to build your apps with `rg_tool`. See `BUILDING.md` for more info about this and flashing. **Make sure to use the steps for `.img`, NOT `.fw`**

