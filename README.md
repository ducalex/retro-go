# Description
Retro-Go is a launcher and framework to run emulators on the ODROID-GO and compatible ESP32(-S2) devices. 
It comes with many emulators!

### Supported systems:
- NES
- Gameboy
- Gameboy Color
- Sega Master System
- Sega SG-1000
- Sega Game Gear
- Colecovision
- PC Engine
- Lynx
- SNES (work in progress)
- DOOM

### Retro-Go features:
- In-game menu
- Favorites and recently played
- GB RTC adjust and save
- GB GBC colorization palettes
- More scaling options
- Bilinear filtering
- NES color palettes
- NES NSF (music) support
- PAL roms support
- Smoother performance
- Better compatibility
- Turbo Speed/Fast forward
- Customizable launcher
- PNG cover art
- Saved state screenshots
- exFAT support
- And more!


# Screenshot
![Preview](retro-go-preview.jpg)


# Key Mappings

## In the launcher
| Button  | Action |
| ------- | ------ |
| Menu    | Version information  |
| Volume  | Options menu  |
| Select  | Previous emulator |
| Start   | Next emulator |
| A       | Start game |
| B       | File properties |
| Left    | Page up |
| Right   | Page down |

## In game
| Button  | Action |
| ------- | ------ |
| Menu    | Game menu (save/quit)  |
| Volume  | Options menu  |

Note: If you are stuck in an emulator, hold MENU while powering up the device to return to the launcher.


# Game covers 
The preferred cover art format is PNG with a resolution of 160x168 and I recommend post-processing your 
PNG with [pngquant](https://pngquant.org/) or [imagemagick](https://imagemagick.org/script/index.php)'s 
`-colors 255` for smaller file sizes. Retro-Go is also backwards-compatible with the official RAW565 Go-Play 
romart pack that you may already have.

For a quick start you can copy the folder `covers` of this repository to the root of your sdcard and 
rename it `romart`. I also periodically upload zips to the release page.

## Adding missing covers
First identify the CRC32 of your game (in the launcher press A -> Properties). Now, let's assume that 
the CRC32 of your NES game is ABCDE123, you must place the file at: `/romart/nes/A/ABCDE123.png`.

_Note: If you need to compute the CRC32 outside of retro-go, please be mindful that certain systems 
skip the file header in their CRC calculation (eg NES skips 16 bytes and Lynx skips 64 bytes). 
The number must also be zero left-padded to be 8 chars._


# Display update mode
The 40Mhz SPI speed gives us a fill rate of 28 frames per second. Because of this, I have developed a partial update mode that refreshes only the portions of the screen that have changed. This result in an effective framerate of 60 most of the time and much reduced tearing. However, this process uses more CPU and is overall more complex to deal with (scaling, filtering), so in some instances it might cause more issues than it solves. That is why there is a toggle to disable it. Use the setting that works best for you but please report issues you encounter in Partial mode so that I can keep improving it!


# Sound quality
The volume isn't correctly attenuated on the GO, resulting in upper volume levels that are too loud and 
lower levels that are distorted due to DAC resolution. A quick way to improve the audio is to cut one
of the speaker wire and add a `15 Ohm (or thereabout)` resistor in series. Soldering is better but not 
required, twisting the wires tightly will work just fine.
[A more involved solution can be seen here.](https://wiki.odroid.com/odroid_go/silent_volume)


# Game Boy SRAM *(aka Save/Battery/Backup RAM)*
In Retro-Go, save states will provide you with the best and most reliable save experience. That being said, please read on if you need or want SRAM saves. The SRAM format is compatible with VisualBoyAdvance so it may be used to import or export saves.

On real hardware, Game Boy games save their state to a battery-backed SRAM chip in the cartridge. A typical emulator on the deskop would save the SRAM to disk periodically or when leaving the emulator, and reload it when you restart the game. This isn't possible on the Odroid-GO because we can't detect when the device is about to be powered down and we can't save too often because it causes stuttering. That is why the auto save delay is configurable (disabled by default) and pausing the emulation (opening a menu) will also save to disk if needed. The SRAM file is then reloaded on startup (unless a save state loading was requested via "Resume").

To recap: If you set a reasonable save delay (10-30s) and you briefly open the menu before powering down, and don't use save states, you will have close to the "real hardware experience".


# BIOS
Some emulators support loading a BIOS. The files should be placed as follows:
- GB: `/retro-go/system/gb_bios.bin`
- GBC: `/retro-go/system/gbc_bios.bin`
- FDS: `/retro-go/system/fds_bios.bin`


# Known issues
An up to date list of incompatible/broken games can be found on the [ODROID-GO forum](https://forum.odroid.com/viewtopic.php?f=159&t=37599). This is also the place to submit bug reports and feature requests.


# Future plans / Feature requests
- Cheats support (In progress)
- Famicom Disk System (In progress)
- SGB enhanced palette support (In progress)
- SNES emulation (Stalled)
- Netplay (Stalled)
- Multiple save states
- More emulators (Atari 2600/5200/7800, NGP Color)
- Chip sound player
- Sleep mode
- Arduboy compatibility?
- WiFi file manager

I'm also working on a combined application where a single binary would contain all the emulators. This will provide faster boot times, much much smaller firmware size, faster builds, and support for platforms where having multiple binaries is inconvenient (SDL/Desktop) or impossible (some ARM mcu).


# Building Retro-Go

## Prerequisites
You will need a working installation of [esp-idf 4.0.2](https://docs.espressif.com/projects/esp-idf/en/v4.0.2/) or superior and only the CMake (default) build system is supported.

_Note: Using esp-idf 4.2 results in degraded emulation performance and should be avoided. Official builds of retro-go use 4.1.x, which I recommend._

### ESP-IDF Patches
Patching esp-idf may be required for full functionality. Patches are located in `tools/patches` and can be applied to your global esp-idf installation, they will not break your other projects/devices.
- `sdcard-fix`: This patch is mandatory for the ODROID-GO (and clones).
- `panic-hook`: This is to help users report bugs, see `Capturing crash logs` bellow for more details. The patch is optional but recommended.
- `enable-exfat`: Enable exFAT support. I don't recommended it but it works if you need it.
- `disable-boot-sha256`: This patch allows for faster boot and 5KB less flash usage! This patch is fully optional.

## Build everything and generate .fw:
1. `rg_tool.py build-fw`

For a smaller build you can also specify which apps you want, for example the launcher + nes + gameboy only:
1. `rg_tool.py build-fw launcher nofrendo-go gnuboy-go`

## Build, flash, and monitor individual apps for faster development:
1. `rg_tool.py --offset=0x100000 --port=COM3 run nofrendo-go`
* Offset is required only if you use my multi-firmware AND retro-go isn't the first installed application, in which case the offset is shown in the multi-firmware.

## Environment variables 
rg_tool.py supports a few environment variables if you want to avoid passing flags all the time:
- `RG_TOOL_TARGET` represents --target
- `RG_TOOL_OFFSET` represents --offset
- `RG_TOOL_BAUD` represents --baud
- `RG_TOOL_PORT` represents --port

## Changing the launcher's images
All images used by the launcher (headers, logos) are located in `launcher/images`. If you edit them you must run the `launcher/gen_images.py` script to regenerate `images.h`. The format must be PNG 255 colors.

## Changing or adding fonts
Fonts are found in `components/retro-go/fonts`. There are basic instructions in `fonts.h` on how to add fonts. 
In short you need to generate a font.c file and add it to fonts.h. It'll try to add better instructions soon...

## Capturing crash logs
When a panic occurs, Retro-Go has the ability to save debugging information to `/sd/crash.log`. This provides users with a simple way of recovering a backtrace (and often more) without having to install drivers and serial console software. A weak hook is installed into esp-idf panic's putchar, allowing us to save each chars in RTC RAM. Then, after the system resets, we can move that data to the sd card. You will find a small esp-idf patch to enable this feature in tools/patches.


# Porting
I don't want to maintain non-ESP32 ports in this repository but let me know if I can make small changes to make your own port easier! The absolute minimum requirements for Retro-Go are roughly:
- Processor: 200Mhz 32bit little-endian
- Memory: 2MB
- Compiler: C99 and C++03 (for lynx and snes)


# Acknowledgements
- The design of the launcher was inspired (copied) from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- The NES/GBC/SMS emulators and base library were originally from the "Triforce" fork of the [official Go-Play firmware](https://github.com/othercrashoverride/go-play) by crashoverride, Nemo1984, and many others.
- PCE-GO is a fork of [HuExpress](https://github.com/kallisti5/huexpress) and [pelle7's port](https://github.com/pelle7/odroid-go-pcengine-huexpress/) was used as reference.
- The Lynx emulator is a port of [libretro-handy](https://github.com/libretro/libretro-handy).
- The SNES emulator is a port of [Snes9x](https://github.com/snes9xgit/snes9x/).
- The DOOM engine is a port of [PrBoom 2.5.0](http://prboom.sourceforge.net/).
- PNG support is provided by [luPng](https://github.com/jansol/LuPng) and [zlib](http://zlib.net).
- PCE cover art is from [Christian_Haitian](https://github.com/christianhaitian).
- Some icons from [Rokey](https://iconarchive.com/show/seed-icons-by-rokey.html).
- Special thanks to [RGHandhelds](https://www.rghandhelds.com/) and [MyRetroGamecase](https://www.myretrogamecase.com/) for sending me a [G32](https://www.myretrogamecase.com/products/game-mini-g32-esp32-retro-gaming-console-1) device.

# License
Everything in this project is licensed under the [GPLv2 license](COPYING) with the exception of the following components:
- components/lupng (PNG library, MIT)
- components/retro-go (Retro-Go's framework, MIT)
- components/zlib (zlib library, zlib)
- handy-go/components/handy (Lynx emulator, BSD)
