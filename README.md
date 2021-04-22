# Description
Retro-Go is a launcher and framework to run emulators on the ODROID-GO and compatible ESP32(-S2) devices. 
It comes with many emulators!

### Supported systems:
- NES
- Gameboy
- Gameboy Color
- Sega Master System
- Sega Game Gear
- Colecovision
- PC Engine
- Lynx
- SNES (work in progress)

### Retro-Go features:
- In-game menu
- Favorites support
- GB RTC adjust and save
- GB GBC colorization palettes
- More scaling options
- Bilinear filtering
- NES color palettes
- PAL roms support
- Smoother performance
- Better compatibility
- Double/Triple Speed
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
First identify the CRC32 of your game (in the launcher press B). Now, let's assume that the CRC32 of your
NES game is ABCDE123, you must place the file (format described above) at: `sdcard:/romart/nes/A/ABCDE123.png`.

_Note: If you need to compute the CRC32 outside of retro-go, please be mindful that certain systems 
skip the file header in their CRC calculation (eg NES skips 16 bytes and Lynx skips 64 bytes). 
The number must also be zero padded to be 8 chars._


# Sound quality
The volume isn't correctly attenuated on the GO, resulting in upper volume levels that are too loud and 
lower levels that are distorted due to DAC resolution. A quick way to improve the audio is to cut one
of the speaker wire and add a `15 Ohm (or thereabout)` resistor in series. Soldering is better but not 
required, twisting the wires tightly will work just fine.
[A more involved solution can be seen here.](https://wiki.odroid.com/odroid_go/silent_volume)


# Game Boy SRAM *(Save RAM, Battery RAM, Backup RAM)*
In Retro-Go, save states will provide you with the best and most reliable save experience. That being said, please read on if you need or want SRAM saves. The SRAM format is compatible with VisualBoyAdvance so it may be used to import or export saves.

On real hardware, Game Boy games save their state to a battery-backed SRAM chip in the cartridge. A typical emulator on the deskop would save the SRAM to disk periodically or when leaving the emulator, and reload it when you restart the game. This isn't possible on the Odroid-GO because we can't detect when the device is about to be powered down and we can't save too often because it causes stuttering. That is why the auto save delay is configurable (disabled by default) and pausing the emulation (opening a menu) will also save to disk if needed. The SRAM file is then reloaded on startup (unless a save state loading was requested via "Resume").

To recap: If you set a reasonable save delay (10-30s) and you briefly open the menu before powering down, and don't use save states, you will have close to the "real hardware experience".


# Known issues
An up to date list of incompatible/broken games can be found on the [ODROID-GO forum](https://forum.odroid.com/viewtopic.php?f=159&t=37599). This is also the place to submit bug reports and feature requests.


# Future plans
- SNES emulation (In progress)
- Netplay (On hold)
- Multiple save states
- Atari 2600 or 5200 or 7800
- Neo Geo Pocket Color
- Famicom Disk System
- Better launcher fonts
- Folders support
- Recently played games
- Game Boy BIOS
- Chip sound player
- Sleep mode
- Arduboy compatibility?


# Building Retro-Go

## Prerequisites
You will need a working installation of [esp-idf](https://docs.espressif.com/projects/esp-idf/en/v4.0.2/) version 3.3.4 or 4.0.2. The legacy (make) build system isn't supported, only idf/cmake.

_Note: Other esp-idf versions will work (>=3.3.3) but I cannot provide help for them. Many are known to have problems; for example 3.3.0, 4.0.0, and 4.1.* have broken sound (but it is fixable if you really need to) and 4.2 and 4.3 have broken SD Card support._

### ESP-IDF Patches
Retro-Go will build and most likely run without any changes to esp-idf, but patches do provide significant advantages. The patches are located in `tools/patches`. Here's the list:
- `esp-idf_enable-exfat`:  Enable exFAT support. The patch is entirely optional.
- `esp-idf_esp_error_check`: This causes assertion handling to behave somewhere between [SILENT and ENABLED](https://docs.espressif.com/projects/esp-idf/en/v3.1.7/api-reference/kconfig.html#envvar-CONFIG_OPTIMIZATION_ASSERTION_LEVEL). The goal is to have ENABLED's benefits without the file size penalty. The patch is entirely optional.
- `esp-idf-X.X_sdcard-fix`: This improves SD Card compatibility significantly but can also reduce transfer speed a lot. The patch is usually required if you intend to distribute your build.
- `esp-idf-4.0-panic-hook`: This is to help users report bugs, see `Capturing crash logs` bellow for more details. The patch is entirely optional.

## Build everything and generate .fw:
1. `rg_tool.py build-fw`

For a smaller build you can also specify which apps you want, for example the launcher + nes/gameboy only:
1. `rg_tool.py build-fw launcher nofrendo-go gnuboy-go`

## Build, flash, and monitor individual apps for faster development:
1. `rg_tool.py run nofrendo-go --offset=0x100000 --port=COM3`
* Offset is required only if you use my multi-firmware AND retro-go isn't the first installed application, in which case the offset is shown in the multi-firmware.

## Capturing crash logs
When a panic occurs, Retro-Go has the ability to save debugging information to `/sd/crash.log`. This provides users with a simple way of recovering a backtrace (and often more) versus having to install drivers and serial console software. A weak hook is installed into esp-idf panic's putchar, allowing us to save each chars in RTC RAM. Then, after the system resets, we can move that data to the sd card. You will find a small esp-idf patch to enable this feature in tools/patches.


# Porting
I don't want to maintain non-ESP32 ports in this repository but let me know if I can make small changes to make your own port easier! The absolute minimum requirements for Retro-Go are roughly:
- Processor: 200Mhz 32bit little-endian with unaligned memory access support
- Memory: 2MB
- Compiler: C99 and C++03 (for lynx and snes)


# Acknowledgements
- The design of the launcher was inspired (copied) from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- The NES/GBC/SMS emulators and base library were originally from the "Triforce" fork of the [official Go-Play firmware](https://github.com/othercrashoverride/go-play) by crashoverride, Nemo1984, and many others.
- The PCE emulator is a port of [HuExpress](https://github.com/kallisti5/huexpress) and [pelle7's port](https://github.com/pelle7/odroid-go-pcengine-huexpress/) was used as reference.
- The Lynx emulator is a port of [libretro-handy](https://github.com/libretro/libretro-handy).
- The SNES emulator is a port of [Snes9x](https://github.com/snes9xgit/snes9x/).
- PNG support is provided by [luPng](https://github.com/jansol/LuPng) and miniz.
- PCE cover art is from Christian_Haitian.


# License
Everything in this project is licensed under the [GPLv2 license](COPYING) with the exception of the following components:
- components/lupng (PNG library, MIT)
- components/retro-go (Retro-Go's framework, MIT)
- handy-go/components/handy (Lynx emulator, BSD)
