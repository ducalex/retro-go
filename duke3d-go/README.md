# Duke3D-Go: Duke Nukem 3D for Retro-Go

An optimised ESP32, ESP32-S3, and ESP32-P4 handheld port of Duke Nukem 3D, built on the Retro-Go framework and based on the Chocolate Duke3D engine.

Performance work includes direct double buffering, selective framebuffer synchronization, renderer and audio hot-path improvements, faster GRP lookup and I/O, and bounded retention of costly sounds. See [OPTIMISATION.md](OPTIMISATION.md) for technical details and ESP32, ESP32-S3, and ESP32-P4 benchmarks.

## Core Enhancements

### 1. Retro-Go Options Submenu

The options screen features a dedicated **RETRO-GO OPTIONS** menu which allows adjusting hardware settings in real-time:

* **Audio Driver Selection**: Dynamically cycles between speaker/DAC sinks (e.g., standard internal DAC vs. External I2S DAC) with safety delays to prevent driver race conditions.
* **Volume & Brightness Sliders**: Integrates with the native Retro-Go APIs to adjust backlight levels and audio volume directly.
* **Display Scaling Controls**: Toggles between `OFF`, `FIT`, `FULL`, and `ZOOM` aspect ratios.
* **Real-time Overclocking**: Toggles CPU overclock profiles (`OFF`, `LOW`, `MEDIUM`, `HIGH`).

### 2. Gameplay & Control Customizations

See [CONTROLS.md](CONTROLS.md) for the complete control layout and hotkey behavior.

* **Shift-Hotkey Notification**: Displays an on-screen `"Hotkey Active"` message whenever the Shift modifier (holding **START**) is active.
* **Expanded Gamepad Layouts**:
  - `START + B`: Looks down.
  - `START + X / OPTION`: Looks up.
* **Inverted Fly/Swim Controls**: Swaps Jump and Crouch inputs dynamically when flying or swimming (both underwater and on the water surface) for an intuitive handheld flight/swim control scheme.
* **Engine Stability**: Modified the sprite spawning logic to gracefully handle limit overflows. Instead of crashing during intensive combat (like rapid kicking), the engine now logs a warning and skips the non-essential sprite.

### 3. Interactive Cheat & Warp Menu

* Adds cheats such as God Mode, All Items/Weapons, and No Clip.
* Consolidated separate level warp cheats into dynamic, toggle-based episode warp menus (**EPISODE 1** through **EPISODE 4**).
* Uses D-pad **Left / Right** keys to cycle levels (`E1L1` -> `E1L6`, `E2L1` -> `E2L11`, etc.), and only warps to the level when the user presses **A**.
* Automatically detects the GRP type to limit visible episode rows (Shareware = 1, Full 1.3d = 3, Atomic 1.5 = 4).
* Automatically restricts Episode 1 to exactly 6 playable levels to exclude dummy shareware ordering info screens.

---

## Content Compatibility (.GRP Files)

The port identifies known game data using CRC32 checksums. The following versions are recognized:

* **v1.3d Shareware**: 11.03 MB (CRC32: `0x124011EE`)
* **v1.3d Full / Retail**: 26.52 MB (CRC32: `0xFD340065`)
* **v1.4 Plutonium Pak**: 44.34 MB (CRC32: `0x02A945E8`)
* **v1.5 Atomic Edition**: 44.35 MB (CRC32: `0xF51ADCD`)
* **DukeNano3D**: Support for minimal GRP packs based on 1.3d shareware: [DukeNano3D](https://github.com/ThomasFarstrike/DukeNano3D)

Unknown CRC32 values fall back to 1.3d Shareware behavior and are not guaranteed to work correctly.

### Setup

Place GRP files in Retro-Go's Duke3D ROM directory, normally `/sd/roms/duke3d/`, and launch the desired GRP from the Retro-Go launcher. The ROM path supplied by the selected launcher entry is used directly. ZIP files containing a GRP are also supported.

If no ROM path is supplied, the engine falls back to:

`roms/duke3d/duke3d.grp`

GRP files may otherwise be renamed, for example `Duke-Shareware.grp` or `Duke3D-Atomic.grp`. The expansion layering described below requires an Atomic base file named `duke3d.grp`.

---

## Official Expansion Packs & GRP Layering

This port supports the three official expansion packs:

1. **Duke It Out in D.C.** (`dukedc.grp`)
2. **Duke Caribbean: Life's a Beach** (`vacation.grp`)
3. **Duke: Nuclear Winter** (`nwinter.grp`)

### How to use them

* Place the expansion GRP files (for example, `dukedc.grp`) in the Duke3D ROM directory alongside the base Atomic `duke3d.grp`, which supplies the shared game assets.
* Launch the expansion GRP directly from the Retro-Go launcher.
* **GRP Layering**: The engine identifies the custom GRP, automatically mounts the base `duke3d.grp` first at index `0` for shared assets, and then mounts the expansion GRP on top at index `1` as an overlay.
* **Automatic Script Overrides**: The engine scans the mounted GRPs and local directory for expansion CON scripts such as `dukedc.con`, `vacation.con`, and `nwinter.con`. If detected, it uses the expansion definitions instead of the default `GAME.CON`.

---

## Device-Specific Configuration

### ESP32

* **Memory Limits**: Uses `4,096` total sprites and up to `256` on-screen sprites to fit within the smaller memory budget.
* **Default Resolution**: `320x200`, scaled to the display. Other listed video modes may cost substantial performance or image quality.
* **Memory Placement**: Large engine data uses PSRAM while latency-sensitive stacks, DMA buffers, and selected lookup data remain in internal memory.

### ESP32-S3

* **Default Resolution**: `320x240`.
* **Memory Limits**: Uses `8,192` total sprites and up to `512` on-screen sprites.

### ESP32-P4

* **Default Resolution**: `320x240`.
* **Memory Limits**: Uses `8,192` total sprites and up to `1,024` on-screen sprites.

---

## Storage & Path Management

Save files and configurations are organised in system-managed subdirectories to keep the SD card clean:

* **Game ROM Directory**: `roms/duke3d/`
* **Save Directory**: `retro-go/saves/duke3d/`
* **Configuration Directory**: `retro-go/config/`

---

## Legal & Licenses

* **"Duke Nukem 3D"** is a registered trademark of Apogee Software, Ltd. (3D Realms).
* **Build Engine** (Ken Silverman): Located under `components/duke3d/Engine/`, licensed under the Build License (see `BUILDLIC.txt`).
* **Game Code**: Located under `components/duke3d/Game/`, licensed under GNU General Public License v2.0 (see `LICENSE`).
* **SDL Wrapper**: Located under `components/duke3d/SDL/`, contains code under the ZLIB license (see `LICENSE_ZLIB`).
