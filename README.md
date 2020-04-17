# Description
Retro-Go is composed of a launcher and several emulators.

### Supported systems:
- NES
- Gameboy
- Gameboy Color
- Sega Master System
- Sega Game Gear
- Colecovision
- PC Engine (Soon!)

### Compared to other similar projects for the ODROID-GO, Retro-Go brings:
- In-game menu
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
![Preview](https://raw.githubusercontent.com/ducalex/retro-go/master/assets/screenshot.jpg)

# Key Mappings

## In the launcher
| Button  | Action |
| ------- | ------ |
| Menu    | Version information  |
| Volume  | Options menu  |
| Select  | Previous emulator |
| Start   | Next emulator |
| A       | Start game |
| Left    | Page up |
| Right   | Page down |

## In game
| Button  | Action |
| ------- | ------ |
| Menu    | Game menu (save/quit)  |
| Volume  | Options menu  |


# Game covers/artwork
Retro-Go supports both RAW565 and PNG formats for cover art.

A premade PNG romart pack is available in the assets folder of this repository.

Retro-Go is also compatible with the Go-Play romart pack that you may already have.


## Adding missing artwork
The simplest method is to create a PNG file named like your rom (minus the extension) and place it in `/romart/emu/<my rom name>.png`

Example:  
/roms/nes/Super Mario.nes  
/romart/nes/Super Mario.png  

Important: The file must be at most 200x200px.

## CRC cache
Retro-Go caches some data to speed up cover art discovery and display.
If you have any problem the first step is to clear the cache located in /odroid/cache.


# Known issues
- Sound distortion in NES with volume above 5


# Future plans
- CMake support
- Rewind
- Netplay (In progress)


# Compilation
The official esp-idf version 3.3 is required and you should apply the following patch:

- [Improve SD card compatibility](https://github.com/OtherCrashOverride/esp-idf/commit/a83e557538a033e25c376eedac79663c9b7b75da)

_Note: It is possible to use esp-idf 3.2 but changes to sdkconfig will be necessary._

## Build Steps:
1. Build all subprojects: `./build_all.sh`
2. Create .fw file: `./mkfw.sh`


# Acknowledgements
- The NES/GBC/SMS/COL emulators were originally from the "Triforce" fork of the official Go-Play firmware.
- The PC Engine emulator was first ported to the GO by [pelle7](https://github.com/pelle7/odroid-go-pcengine-huexpress/).
- The aesthetics of the launcher were inspired (copied) from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- [miniz](https://github.com/richgel999/miniz) For zipped ROM and zlib API
- [luPng](https://github.com/jansol/LuPng) For basic PNG decoding


# License
Every part of this project is licensed under the MIT license with the exception of:
- gnuboy-go/components/gnuboy (GPL2)
- smsplusgx-go/components/smsplus (GPL2)
