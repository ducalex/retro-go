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
The preferred cover art format is 8bit PNG with a resolution of less than 200x200 and I 
recommend post-processing with [pngquant](https://pngquant.org/). Retro-Go is also 
backwards-compatible with the official RAW565 Go-Play romart pack that you may already have.

A premade PNG romart pack is available in the assets folder of this repository.

## Adding missing artwork
The simplest method to add missing artwork is to create a PNG file named like your rom 
(minus the extension) and place it in `/romart/emu/<my rom name>.png`

Example:  
/roms/nes/Super Mario.nes  
/romart/nes/Super Mario.png  

## CRC cache
Retro-Go caches some data to speed up cover art discovery and display.
If you have any problem the first step is to delete the /odroid/cache folder.


# Known issues
- Sound distortion in NES with volume above 5


# Future plans
- CMake support (esp-idf 4.0)
- Rewind
- Netplay (In progress)


# Compilation
The official esp-idf version 3.3 is required and it is recommended to apply the 
`esp-idf-sdcard-compat.diff` patch located in the assets folder.

_Note: It is possible to use esp-idf 3.2 but changes to sdkconfig will be necessary._

## Build Steps:
1. Build all subprojects: `./build_all.sh`
2. Create .fw file: `./mkfw.sh`


# Acknowledgements
- The NES/GBC/SMS emulators and base library were originally from the "Triforce" fork of the [official Go-Play firmware](https://github.com/othercrashoverride/go-play) by crashoverride, Nemo1984, and many others.
- The [HuExpress](https://github.com/kallisti5/huexpress) (PCE) emulator was first ported to the GO by [pelle7](https://github.com/pelle7/odroid-go-pcengine-huexpress/).
- The Lynx emulator is an adaptation of [libretro-handy](https://github.com/libretro/libretro-handy).
- The aesthetics of the launcher were inspired (copied) from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- [miniz](https://github.com/richgel999/miniz) For zipped ROM and zlib API
- [luPng](https://github.com/jansol/LuPng) For basic PNG decoding
- PCE cover art is from Christian_Haitian.


# License
This project is licensed under the GPLv2. Some components are also available under the MIT license. 
Respective copyrights apply to each component. For the rest:
```
Retro-Go: Retro emulation for the ODROID-GO
Copyright (C) 2020 Alex Duchesne

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
```