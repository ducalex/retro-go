# Building Retro-Go

## Prerequisites
You will need a working installation of [esp-idf](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/get-started/index.html#get-started-get-prerequisites). Versions 4.3 to 5.2 are supported.

_Note: As of retro-go 1.35, I use 4.3. Version 4.1 was used for 1.20 to 1.34 versions._

### ESP-IDF Patches
Patching esp-idf may be required for full functionality. Patches are located in `tools/patches` and can be applied to your global esp-idf installation, they will not break your other projects/devices.
- `sdcard-fix`: This patch is mandatory for the ODROID-GO (and clones).
- `panic-hook`: This is to help users report bugs, see `Capturing crash logs` below for more details. The patch is optional but recommended.


## Obtaining the code


Using git is the preferred method but you can also download a zip from the project's front page and extract it if you want, Retro-Go has no external dependencies.

There are generally two active git branches on retro-go:
- `master` contains the code form the most recent release and is usually tested and known to be working
- `dev` contains code in development that will be merged to master upon the next release and is often untested

`git clone -b <branch> https://github.com/ducalex/retro-go/`


# rg_tool
ESP-IDF doesn't support managing multiple apps in one environment (launcher, prboom-go, etc). `rg_tool.py` is used instead, which passes correct arguments to `idf.py` and other tools.


To get started, run
```
./rg_tool.py
```
or 
```
python rg_tool.py
```

## Build everything and generate a firmware image:
- Generate a .fw file to be installed with odroid-go-firmware (SD Card):\
   `python rg_tool.py build-fw` or `python rg_tool.py release` (clean build)
- Generate a .img to be flashed with esptool.py (Serial):\
   `python rg_tool.py build-img` or `python rg_tool.py release` (clean build)


For a smaller build you can also specify which apps you want, for example the launcher + DOOM only:
1. `python rg_tool.py build-fw launcher prboom-go`


Note that the app named `retro-core` contains the following emulators: NES, PCE, G&W, Lynx, and SMS/GG/COL. As such, these emulators cannot be selected individually. The reason for the bundling is simply size, together they account for a mere 700KB instead of almost 3MB when they were built separately.


## Flashing an image for the first time

Once we have successfully built an image file (`.img` or `.fw`), it must be flashed to the device.

To flash a `.img` file with `rg_tool.py`, run:
```
python rg_tool.py --target (target) --port (usbport) install (apps)
```

To flash a `.img` file with `esptool.py`, run:
```
esptool.py write_flash --flash_size detect 0x0 retro-go_*.img
```

To flash a `.fw` file: 

Instructions depend on your device, refer to [README.md](README.md#installation).

## Build, flash, and monitor individual apps for faster development:

A full Retro-Go image must be flashed at least once (refer to previous section), but, after that, it is possible to flash and monitor individual apps for faster development time.

1. Flash: `python rg_tool.py --port=COM3 flash prboom-go`
2. Monitor: `python rg_tool.py --port=COM3 monitor prboom-go`
3. Flash then monitor: `python rg_tool.py --port=COM3 run prboom-go`

## Environment variables
rg_tool.py supports a few environment variables if you want to avoid passing flags all the time:
- `RG_TOOL_TARGET` represents --target
- `RG_TOOL_BAUD` represents --baud
- `RG_TOOL_PORT` represents --port

## Changing the launcher's images
All images used by the launcher (headers, logos) are located in `launcher/main/images`. If you edit them you must run the `launcher/main/gen_images.py` script to regenerate `images.c`. Magenta (rgb(255, 0, 255) / 0xF81F) is used as the transparency color.

## Capturing crash logs
When a panic occurs, Retro-Go has the ability to save debugging information to `/sd/crash.log`. This provides users with a simple way of recovering a backtrace (and often more) without having to install drivers and serial console software. A weak hook is installed into esp-idf panic's putchar, allowing us to save each chars in RTC RAM. Then, after the system resets, we can move that data to the sd card. You will find a small esp-idf patch to enable this feature in tools/patches.


To resolve the backtrace you will need the application's elf file. If lost, you can recreate it by building the app again **using the same esp-idf and retro-go versions**. Then you can run `xtensa-esp32-elf-addr2line -ifCe app-name/build/app-name.elf`.

## Porting
Instructions to port to new ESP32 devices can be found in [PORTING.md](PORTING.md).


I don't want to maintain non-ESP32 ports in this repository, but let me know if I can make small changes to make your own port easier! The absolute minimum requirements for Retro-Go are roughly:
- Processor: 200Mhz 32bit little-endian
- Memory: 2MB
- Compiler: C99 (and C++03 for handy-go)

Whilst all applications were heavily modified or even redesigned for our constrained needs, special care is taken to keep
Retro-Go and ESP32-specific code exclusively in their port file (main.c). This makes reusing them in your own codebase very easy!
