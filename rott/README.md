# Rise of the Triad (ROTT) for Retro-Go

A high-performance port of the classic 1994 Apogee FPS **Rise of the Triad: Dark War** to the [Retro-Go](https://github.com/ducalex/retro-go) ecosystem. Optimised for ESP32, ESP32-S3, and ESP32-P4 microcontrollers.

## Features
- **Native 35Hz Engine**: Full parity with the original PC game logic and physics.
- **OPL3 MIDI Music**: Real-time FM synthesis for the complete original soundtrack.
- **Dynamic PSRAM Management**: Efficient use of external RAM to support the full commercial game.
- **Hardware Scaling**: Native 320x200 output scaled to fill any Retro-Go supported display (320x240, etc.).
- **Cross-Platform**: Supports ESP32, ESP32-S3 and ESP32-P4 Retro-Go targets.

## Changes for the Retro-Go Port
- **Memory Strategy**: Replaced standard heap allocations with a centralised `rg_memory` system using PSRAM for all major structures (map, sprites, sounds) to prevent DRAM fragmentation.
- **Rendering Pipeline**: Removed legacy Mode X planar drawing in favor of a linear 8-bit buffer, with final output handled by an optimised SDL-to-Retro-Go surface bridge.
- **Physics Sync**: Corrected the game's internal `VBLCOUNTER` to 35Hz to ensure stable collision detection and movement speed on modern hardware.
- **Audio Integration**: Ported a software OPL3 synthesiser to render MIDI music directly through the Retro-Go audio pipeline, mixing it in real-time with 8-bit sound effects.
- **Stability Fixes**: Resolved numerous alignment and data-packing issues in the original source that caused crashes on RISC-V and Xtensa architectures.

## Requirements
To play, you must place the original game files in your SD card under `roms/rott/`.

### Required Files:
- `DARKWAR.WAD` (Main game data)
- `DARKWAR.RTL` (Registered levels)
- `DARKWAR.RTC` (Registered cinematics)
- `REMOTE1.RTS` (Sound data)
- `DEMO1_3.DMO` `DEMO2_3.DMO` `DEMO3_3.DMO` `DEMO4_3.DMO` (Demo files)

*The shareware version is also supported (HUNTBGIN.WAD/RTL).*

## Switching Versions (Shareware vs. Registered)
Due to structural differences in the ROTT engine, switching between the Shareware and Registered versions requires a recompile.

1. Open `components/rott/develop.h`.
2. Find `#define SHAREWARE`.
3. Set to **`1`** for Shareware (The Hunt Begins) or **`0`** for Registered (Dark War).
4. Rebuild and flash your device.

The engine will automatically prioritise the appropriate WAD and RTL files based on this setting.

## Controls
The following default mapping is used across most Retro-Go handhelds:

| Action | Control |
| :--- | :--- |
| **Move / Turn** | D-Pad |
| **Shoot** | Button A |
| **Strafe (Hold)** | Button B |
| **Open / Use** | Button X / Start |
| **Weapon Swap** | Button Y / Select |
| **Strafe Left / Right** | L / R Shoulders |
| **ROTT Menu** | Short press Menu |
| **Retro-Go Menu** | Hold Menu |
| **Map** | Short press Option |
| **Retro-Go Options** | Hold Option |

## Credits & Acknowledgements
- **Original Developers**: [Apogee Software, Ltd.](https://www.apogeesoftware.com/) for creating this legendary title.
- **Source Port**: This port is based on the [WinRott / Master](https://icculus.org/rott/) source.
- **Retro-Go Team**: For the incredible framework that makes these ports possible.

---
*Rise of the Triad is (C) 1994-1995 Apogee Software, Ltd. This project is a non-commercial community port.*
