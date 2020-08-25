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
| B       | File properties |
| Left    | Page up |
| Right   | Page down |

## In game
| Button  | Action |
| ------- | ------ |
| Menu    | Game menu (save/quit)  |
| Volume  | Options menu  |

Note: If you are stuck in an emulator, hold MENU while powering up the device to return to the launcher.


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
An up to date list of incompatible/broken games can be found on the ODROID-GO forums.


# Future plans
- Rewind
- Netplay (In progress)


# Compilation
The official esp-idf version 3.3 or 4.0 is required and it is recommended to apply the 
sdcard-fix patch located in the tools folder. Both make and cmake are supported.

## Build Steps:
1. Build all subprojects: `python build_all.py` or `python build_all.py make` (legacy)
2. Create .fw file: `python mkfw.py`


# Acknowledgements
- The NES/GBC/SMS emulators and base library were originally from the "Triforce" fork of the [official Go-Play firmware](https://github.com/othercrashoverride/go-play) by crashoverride, Nemo1984, and many others.
- The [HuExpress](https://github.com/kallisti5/huexpress) (PCE) emulator was first ported to the GO by [pelle7](https://github.com/pelle7/odroid-go-pcengine-huexpress/).
- The Lynx emulator is an adaptation of [libretro-handy](https://github.com/libretro/libretro-handy).
- The aesthetics of the launcher were inspired (copied) from [pelle7's go-emu](https://github.com/pelle7/odroid-go-emu-launcher).
- [miniz](https://github.com/richgel999/miniz) For zipped ROM and zlib API
- [luPng](https://github.com/jansol/LuPng) For basic PNG decoding
- PCE cover art is from Christian_Haitian.
- mkfw tool by crashoverride


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