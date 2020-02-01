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
![Preview](https://raw.githubusercontent.com/ducalex/retro-go/116199c69c081de7a/screenshot.jpg)

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



# Known issues
- Sound distortion in NES with volume above 5


# Future plans

- CMake support
- Convert cover art to GIF
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
