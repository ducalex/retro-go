# Description
Retro-Go is composed of a frontend and several emulators.

### Supported systems:
- NES
- Gameboy / Gameboy Color
- Sega Master System
- Sega Game Gear
- Colecovision

### Compared to other similar projects for the ODROID-GO, Retro-Go brings:
- In-game menu
- Faster/smoother transitions
- RTC adjust and save
- Rewind/Fastforward
- Customizable frontend

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
A premade gif romart pack is available in the assets folder of this repository. 

Retro-Go is also compatible with the Go-Play romart pack. You can find it [on the release page](https://github.com/ducalex/retro-go/releases/tag/romart), it has higher quality and faster loading.


## Adding missing artwork
The simplest method is to create a gif file named like your rom (minus the extension) and place it in `/romart/emu/<my rom name>.gif`

Example:  
/roms/nes/Super Mario.nes  
/romart/nes/Super Mario.gif  

Important: The file must be less than 200x200px.

## CRC cache
Retro-Go caches some data to speed up cover art discovery and display.
If you have any problem the first step is to clear the cache located in /odroid/cache.


# Known issues
- Sound distortion in NES with volume above 5


# Future plans

- CMake support
- Add Lynx emulator


# Compilation
The official esp-idf version 3.3 is required and you should apply the following patch:

- [Improve SD card compatibility](https://github.com/OtherCrashOverride/esp-idf/commit/a83e557538a033e25c376eedac79663c9b7b75da)

_Note: It is possible to use esp-idf 3.2 but changes to sdkconfig might be necessary._

## Build Steps:
1. Build all subprojects: `./build_all.sh`
2. Create .fw file: `./mkfw.sh`


# Acknowledgements
- The emulators code was originally from the "Triforce" fork the official Go-Play firmware.
- A few lines of code were taken from go-emu by pelle7, as well as the esthetics was copied.
