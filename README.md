# Description
Retro-Go is a launcher and framework to run emulators on the ODROID-GO. It comes with many emulators!

### Supported systems:
- NES
- Gameboy
- Gameboy Color
- Sega Master System
- Sega Game Gear
- Colecovision
- PC Engine
- Lynx

### Compared to other similar projects for the ODROID-GO, Retro-Go brings:
- In-game menu
- Favorites support
- GB RTC adjust and save
- GB GBC colorization palettes
- More scaling options
- Bilinear filtering
- NES color palettes
- NES PAL support
- Smoother performance
- Better compatibility
- Fastforward
- Customizable launcher
- PNG cover art
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
The preferred cover art format is 8bit PNG with a resolution of 168x168 and I recommend post-processing 
with [pngquant](https://pngquant.org/). Retro-Go is also backwards-compatible with the official RAW565 
Go-Play romart pack that you may already have.

For a quick start you can copy the folder `covers` of this repository to the root of your sdcard and 
rename it `romart`.

## Adding missing covers
First identify the CRC32 of your game (in the launcher press B). Now, let's assume that the CRC32 of your
nes game is ABCDE123, you must place the file (format described above) at: `sdcard:/romart/nes/A/ABCDE123.png`.

_Note: If you need to compute the CRC32 outside of retro-go, please be mindful that certain systems 
skip the file header in their CRC calculation (eg NES skips 16 bytes and Lynx skips 64 bytes). 
The number must also be zero padded to be 8 chars._

## CRC cache
Retro-Go caches some data to speed up cover art discovery and display.
If you have any problem the first step is to delete the `sdcard:/odroid/cache` folder.

# Sound quality
The volume isn't correctly attenuated on the GO, resulting in upper volume levels that are too loud and 
lower levels that are distorted due to DAC resolution. A quick way to improve the audio is to cut one
of the speaker wire and add a `10 Ohm (or thereabout)` resistor in series. Soldering is better but not 
required, twisting the wires tightly will work just fine.
[A more involved solution can be seen here.](https://wiki.odroid.com/odroid_go/silent_volume)

# Known issues
An up to date list of incompatible/broken games can be found on the ODROID-GO forums.


# Future plans
- Rewind  (In progress)
- Cheats  (In progress)
- Netplay (In progress)


# Compilation
The official esp-idf version 3.3 or 4.0 is required and it is recommended to apply the 
sdcard-fix patch located in the tools folder. Only the cmake build system is supported.

## Build everything and generate .fw:
1. `python rg_tool.py build-fw`

For a smaller build you can also specify which apps you want, for example the launcher + nes/gameboy only:
1. `python rg_tool.py build-fw retro-go nofrendo-go gnuboy-go`

## Build, flash, and monitor individual apps for faster development:
1. `python rg_tool.py run nofrendo-go --offset=0x100000 --port=COM3`
* Offset is required only if you use my multi-firmware, in which case it is displayed in the boot menu.


# Acknowledgements
- The NES/GBC/SMS emulators and base library were originally from the "Triforce" fork of the [official Go-Play firmware](https://github.com/othercrashoverride/go-play) by crashoverride, Nemo1984, and many others.
- The [HuExpress](https://github.com/kallisti5/huexpress) (PCE) emulator was first ported to the GO by [pelle7](https://github.com/pelle7/odroid-go-pcengine-huexpress/).
- The Lynx emulator is an adaptation of [libretro-handy](https://github.com/libretro/libretro-handy).
- The aesthetics of the launcher were inspired (copied) from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- [miniz](https://github.com/richgel999/miniz) For zipped ROM and zlib API
- [luPng](https://github.com/jansol/LuPng) For basic PNG decoding
- PCE cover art is from Christian_Haitian.


# License
Everything in this project is licensed under the [GPLv2 license](COPYING) with the exception of the following components:
- components/lupng (PNG library, MIT)
- components/miniz (zlib library, MIT)
- components/retro-go (Retro-Go's framework, MIT)
- handy-go/components/handy (Lynx emulator, BSD)
