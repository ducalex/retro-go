# Wolf4SDL for Retro-Go

A high-performance port of Wolfenstein 3D to Retro-Go, optimised specifically for ESP32 and ESP32-S3 targets.

This port is based on the [Wolf4SDL](https://github.com/lazd/wolf4sdl) engine and incorporates critical optimisations for embedded hardware.

## Key Enhancements & Changes

### 1. Native 320x240 Resolution
Unlike standard ports that often letterbox or use manual Y-offset hacks, this engine runs natively at 320x240.
- **HUD Alignment**: The 40-pixel status bar is anchored to the absolute bottom of the screen.
- **3D Viewport**: Expanded to fill the top 200 pixels (plus scaling).
- **Menu Centering**: Menu assets (originally 320x200) are automatically centered vertically.

### 2. Deep PSRAM Optimisation
To resolve memory panics during heavy SD card operations (like saving/loading), the engine's memory architecture was refactored:
- **Global Data Migration**: Large data structures (`tilemap`, `actorat`, `objlist`, etc.) have been moved from precious internal RAM to PSRAM.
- **Math Tables**: All lookup tables (`finetangent`, `sintable`, etc.) are dynamically allocated in PSRAM.
- **Result**: Freed over **120KB** of internal RAM, ensuring rock-solid stability during save-game operations and level transitions.

### 3. Smart Path Probing
The engine supports a clean library view by allowing data files to be stored in a subfolder (see [Installation](#installation)). It automatically detects if data is in the root or a `data/` subfolder relative to your launcher selection.

### 4. Robust Weapon Cycling
Implemented a reliable "Circular Search" logic for weapon switching:
- Pressing **Y** or **SELECT** will cycle through your arsenal.
- Automatically skips guns you don't own or those without ammo.
- Always allows fallback to the Knife.

## Installation

### Folder Structure
You can set up your SD card in two ways. The "Clean View" is recommended to keep your launcher menu tidy.

**Option A: Clean View (Recommended)**
Store only a placeholder file in the root to launch the game, and move the bulk data to a `data/` folder.
- `roms/wolf3d/Wolf3d.wl6` (An empty file or a renamed copy of the executable, needs to be .wl6)
- `roms/wolf3d/data/` (Place all `.wl6` or `.vga` files here)

**Option B: Standard View**
Place all files directly in the game folder.
- `roms/wolf3d/` (Place all game data files here)

Both layouts are detected when launching from Retro-Go or directly from an
application `.bin`. The `data/` subdirectory is preferred when both locations
contain game data. If neither location contains recognized Wolf3D/SOD data, the
port displays an error and exits cleanly.

### Persistent Data
- **Save Games**: Automatically stored in `retro-go/saves/wolf3d/`
- **Configuration**: Automatically stored in `retro-go/config/wolf3d/`

## Supported Versions
This build is **setup for Wolfenstein 3D Registered v1.4 only**. 

The engine uses compile-time defines so while the loader can technically read data from other versions, the engine's internal asset indices (Lump IDs) are hardcoded for v1.4 Registered. Using other versions (like Shareware, v1.1, or v1.2) will result in jumbled graphics since the pointers will not match or will panic. See towards the bottom on how to re-compile, if you require a different version.

**Tip**: You can patch your game files to v1.4 with the 'Patch Utility' found here: [maniacsvault.net/ecwolf/download.php](https://maniacsvault.net/ecwolf/download.php)

## Controls

| Retro-Go Key | Action |
| :--- | :--- |
| **D-Pad** | Movement |
| **A** | Fire |
| **B** | Strafe (Hold) |
| **X** / **Start** | Open / Use |
| **Y** / **Select** | Next Weapon (Cycle) |
| **L** / **R** | Strafe Left / Right |
| **Option** | Run (Hold); hold for 0.5 seconds for Retro-Go Options |
| **Menu** | Main Menu / Escape; hold for 0.5 seconds for the Retro-Go game menu |

Full-screen scaling is the default on first launch. It can be changed from the
Retro-Go Options menu, and the selected display mode is retained.

## Developer Section: Recompiling for Other Versions

The engine uses compile-time defines in `components/wolf4sdl/wl_main/version.h` to set asset indices. To support a different version, you must edit this file and recompile.

### Default Setup (v1.4 Registered)
```cpp
//#define SPEAR
//#define UPLOAD
#define GOODTIMES
#define CARMACIZED
```

### Example: Shareware v1.4
```cpp
//#define SPEAR
#define UPLOAD
//#define GOODTIMES
#define CARMACIZED
```

### Example: Spear of Destiny
```cpp
#define SPEAR
//#define UPLOAD
//#define GOODTIMES
#define CARMACIZED
```

## Acknowledgements
- **[jkirsons/wolf4sdl](https://github.com/jkirsons/wolf4sdl)**: For the foundational ESP32 porting work and audio driver logic.
- **[lazd/wolf4sdl](https://github.com/lazd/wolf4sdl)**: For the modern, clean C++ base of the Wolf4SDL engine.
- **[Retro-Go](https://github.com/dubaldessari/retro-go)**: For the incredible multi-platform framework.
