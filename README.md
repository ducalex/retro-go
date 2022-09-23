# Description
Retro-Go is a firmware to play retro games on ESP32-based devices (officially supported are 
ODROID-GO and MRGC-G32). The project consists of a launcher and half a dozen applications that 
have been heavily optimized to reduce their cpu, memory, and flash needs without reducing 
compatibility!

### Supported systems:
- Nintendo: **NES, SNES (slow), Gameboy, Gameboy Color**
- Sega: **SG-1000, Master System, Mega Drive / Genesis, Game Gear**
- Coleco: **Colecovision**
- NEC: **PC Engine**
- Atari: **Lynx**
- Others: **DOOM** (including mods!)

### Retro-Go features:
- In-game menu
- Favorites and recently played
- GB color palettes, RTC adjust and save
- NES color palettes, PAL roms, NSF support
- More emulators, applications
- Scaling and filtering options
- Better performance and compatibility
- Turbo Speed/Fast forward
- Customizable launcher
- Cover art and save state previews
- exFAT support
- And more!

### Screenshots
![Preview](retro-go-preview.jpg)


# Installation
### ODROID-GO
  1. Download `retro-go_1.x_odroid-go.fw` from the release page and copy it to `/odroid/firmware` on your sdcard.
  2. Power up the device while holding down B.
  3. Select retro-go in the files list and flash it.

### MyRetroGameCase G32 (GBC)
  1. Download `retro-go_1.x_mrgc-g32.fw` from the release page and copy it to `/espgbc/firmware` on your sdcard.
  2. Power up the device while holding down MENU (the volume knob).
  3. Select retro-go in the files list and flash it.

### Generic ESP32
This method is intended to be used when .fw support isn't available (when porting to a new device) or undesirable (devices with smaller flash).
  1. Build a .img file (refer to [Building Retro-Go](#building-retro-go) below)
  2. Flash the image: `esptool.py write_flash --flash_size detect 0x0 retro-go_*.img`  
      _Note: Your particular device may require extra steps (like holding a button during power up) or different esptool flags._

# Game covers 
Game covers should be placed in the `romart` folder at the base of your sd card. You can obtain a pre-made pack from 
this repository or from the release page. Retro-Go is also compatible with the older Go-Play romart pack.

### Adding covers
The preferred cover art format is PNG with a resolution of max 160x168 and I recommend post-processing your 
PNG with [pngquant](https://pngquant.org/) or [imagemagick](https://imagemagick.org/script/index.php)'s 
`-colors 255` for smaller file sizes.

You can use one of two naming schemes:
- Using the CRC32 (press A -> Properties in the launcher to find it). Assuming a CRC of ABCDE123, the cover file 
  will be `/romart/nes/A/ABCDE123.png`.
- Using the rom file name. Assuming a rom named `myrom.nes`, the cover file will be 
  `/romart/nes/myrom.nes.png` (notice the inclusion of the rom extension).


# Problems and solutions

### Black screen / Boot loops
Retro-Go typically detects and resolves application crashes and freezes automatically. However, if you do
get stuck in a boot loop, you can hold `DOWN` while powering up the device to return to the launcher. 

### Artifacts or tearing
Retro-Go uses partial screen updating to achieve a higher framerate and reduce tearing. This method isn't
perfect however, if you notice display issues or stuttering you can try changing the `Update` option.

### Sound quality
The volume isn't correctly attenuated on the GO, resulting in upper volume levels that are too loud and 
lower levels that are distorted due to DAC resolution. A quick way to improve the audio is to cut one
of the speaker wire and add a `33 Ohm (or thereabout)` resistor in series. Soldering is better but not 
required, twisting the wires tightly will work just fine.
[A more involved solution can be seen here.](https://wiki.odroid.com/odroid_go/silent_volume)

### Game Boy SRAM *(aka Save/Battery/Backup RAM)*
In Retro-Go, save states will provide you with the best and most reliable save experience. That being said, please 
read on if you need or want SRAM saves. The SRAM format is compatible with VisualBoyAdvance so it may be used to 
import or export saves.

You can configure automatic SRAM saving in the options menu. A longer delay will reduce stuttering at the cost 
of losing data when powering down too quickly. Also note that when *resuming* a game, Retro-Go will give priority 
to a save state if present.


# BIOS files
Some emulators support loading a BIOS. The files should be placed as follows:
- GB: `/retro-go/system/gb_bios.bin`
- GBC: `/retro-go/system/gbc_bios.bin`
- FDS: `/retro-go/system/fds_bios.bin`


# Future plans / Feature requests
- Cheats support (In progress)
- Famicom Disk System (In progress)
- SGB enhanced palette support (In progress)
- Netplay (Stalled)
- More emulators (Atari 2600/5200/7800, Neo Geo Pocket, Arduboy)
- WiFi file manager
- Chip sound player
- Sleep mode


# Building Retro-Go

## Prerequisites
You will need a working installation of esp-idf [4.1](https://docs.espressif.com/projects/esp-idf/en/release-v4.1/get-started/index.html#get-started-get-prerequisites) or superior. Esp-idf 5.0 is not currently supported but we are working on it!

_Note: Official builds use 4.1.2. Versions 4.2, 4.3, and 4.4 have unresolved performance issues at this time._

### ESP-IDF Patches
Patching esp-idf may be required for full functionality. Patches are located in `tools/patches` and can be applied to your global esp-idf installation, they will not break your other projects/devices.
- `sdcard-fix`: This patch is mandatory for the ODROID-GO (and clones).
- `panic-hook`: This is to help users report bugs, see `Capturing crash logs` below for more details. The patch is optional but recommended.
- `enable-exfat`: Enable exFAT support. I don't recommended it but it works if you need it.

## Build everything and generate .fw:
- Generate a .fw file to be installed with odroid-go-firmware (SD Card):  
    `./rg_tool.py build-fw` or `./rg_tool.py release` (clean build)
- Generate a .img to be flashed with esptool.py (Serial):  
    `./rg_tool.py build-img` or `./rg_tool.py release` (clean build)

For a smaller build you can also specify which apps you want, for example the launcher + nes + gameboy only:
1. `./rg_tool.py build-fw launcher nofrendo-go gnuboy-go`

## Build, flash, and monitor individual apps for faster development:
It would be tedious to build, move to SD, and flash a full .fw all the time during development. Instead you can:
1. Flash: `./rg_tool.py --port=COM3 flash nofrendo-go`
2. Monitor: `./rg_tool.py --port=COM3 monitor nofrendo-go`
3. Flash then monitor: `./rg_tool.py --port=COM3 run nofrendo-go`

## Environment variables 
rg_tool.py supports a few environment variables if you want to avoid passing flags all the time:
- `RG_TOOL_TARGET` represents --target
- `RG_TOOL_BAUD` represents --baud
- `RG_TOOL_PORT` represents --port

## Changing the launcher's images
All images used by the launcher (headers, logos) are located in `launcher/images`. If you edit them you must run the `launcher/gen_images.py` script to regenerate `images.c`. Magenta (rgb(255, 0, 255) / 0xF81F) is used as the transparency colour.

## Changing or adding fonts
Fonts are found in `components/retro-go/fonts`. There are basic instructions in `fonts.h` on how to add fonts. 
In short you need to generate a font.c file and add it to fonts.h. It'll try to add better instructions soon...

## Capturing crash logs
When a panic occurs, Retro-Go has the ability to save debugging information to `/sd/crash.log`. This provides users with a simple way of recovering a backtrace (and often more) without having to install drivers and serial console software. A weak hook is installed into esp-idf panic's putchar, allowing us to save each chars in RTC RAM. Then, after the system resets, we can move that data to the sd card. You will find a small esp-idf patch to enable this feature in tools/patches.

To resolve the backtrace you will need the application's elf file. If lost, you can recreate it by building the app again **using the same esp-idf and retro-go versions**. Then you can run `xtensa-esp32-elf-addr2line -ifCe app-name/build/app-name.elf`.

## Porting
I don't want to maintain non-ESP32 ports in this repository but let me know if I can make small changes to make your own port easier! The absolute minimum requirements for Retro-Go are roughly:
- Processor: 200Mhz 32bit little-endian
- Memory: 2MB
- Compiler: C99 (and C++03 for handy-go)

Whilst all applications were heavily modified or even redesigned for our constrained needs, special care is taken to keep 
Retro-Go and ESP32-specific code exclusively in their port file (main.c). This makes reusing them in your own codebase very easy!


# Acknowledgements
- The design of the launcher was inspired (copied) from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- The NES/GBC/SMS emulators and base library were originally from the "Triforce" fork of the [official Go-Play firmware](https://github.com/othercrashoverride/go-play) by crashoverride, Nemo1984, and many others.
- PCE-GO is a fork of [HuExpress](https://github.com/kallisti5/huexpress) and [pelle7's port](https://github.com/pelle7/odroid-go-pcengine-huexpress/) was used as reference.
- The Lynx emulator is a port of [libretro-handy](https://github.com/libretro/libretro-handy).
- The SNES emulator is a port of [Snes9x 2005](https://github.com/libretro/snes9x2005).
- The DOOM engine is a port of [PrBoom 2.5.0](http://prboom.sourceforge.net/).
- The Mega Drive emulator is a port of [Gwenesis](https://github.com/bzhxx/gwenesis/) by bzhxx.
- PNG support is provided by [lodepng](https://github.com/lvandeve/lodepng/).
- PCE cover art is from [Christian_Haitian](https://github.com/christianhaitian).
- Some icons from [Rokey](https://iconarchive.com/show/seed-icons-by-rokey.html).
- Background images from [es-theme-gbz35](https://github.com/rxbrad/es-theme-gbz35).
- Special thanks to [RGHandhelds](https://www.rghandhelds.com/) and [MyRetroGamecase](https://www.myretrogamecase.com/) for sending me a [G32](https://www.myretrogamecase.com/products/game-mini-g32-esp32-retro-gaming-console-1) device.
- The [ODROID-GO](https://forum.odroid.com/viewtopic.php?f=159&t=37599) community for encouraging the development of retro-go!

# License
Everything in this project is licensed under the [GPLv2 license](COPYING) with the exception of the following components:
- components/retro-go (Retro-Go's framework, zlib)
- handy-go/components/handy (Lynx emulator, zlib)
