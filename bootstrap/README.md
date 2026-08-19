# Retro-Go Bootstrap Flasher

An application for Retro-Go that allows you to dynamically flash and boot either official Retro-Go binary files or third-party native ESP32 binaries (e.g., custom games, utility apps, Arduino sketches) directly from your SD card without getting locked out of the Retro-Go launcher. Most useful for devices with limited 4MB flash, aswell as devices that have a lot of third party binary support. Note: The binary files need to be compatible with your actual device!

---

## The Problem & The Solution

Normally, ESP32 Retro-Go apps launch each other by updating the OTA data partition (`otadata`) and performing a software restart. Because standard third-party ESP32 applications do not contain Retro-Go framework code, they have no built-in way to reset the boot partition back to the launcher when you exit. If you boot into a standard third-party binary, you are stuck there; even powering the console off and on will simply boot you straight back into that same app, locking you out of Retro-Go.

**Retro-Go Bootstrap Flasher** solves this by combining a dynamic flasher application with a customised second-stage bootloader recovery hook. If you ever get stuck, **simply turn the physical power switch off and back on** to immediately return to your Retro-Go launcher!

---

## Features

*   💾 **Dynamic Binary & Core Flashing**: Flashes standalone ESP32 `.bin` files and dynamic emulator cores from your SD card directly to a dedicated target partition (`bootstrapped`) on the fly.
*   ⚡ **Smart Reflash Prevention (Instant Boot)**: Remembers the path of the last flashed binary in NVS (`LastFlashedApp`). Consecutive launches on the same core bypass flashing and boot instantly in 1–2 seconds!
*   🎮 **Multi-Emulator Support on 4MB Flash**: Store emulator binaries on the SD card (e.g. `/roms/cores/snes9x.bin`, `/roms/cores/fmsx.bin`, `/roms/cores/gwenesis.bin`). The Launcher detects them, shows their console tabs, flashes the core when needed, and boots straight into your selected ROM.
*   🛡️ **Bootloader Recovery Hook (Lockout Safe)**: Integrates a custom second-stage bootloader hook (`bootloader_after_init`) that intercepts cold boots. Power-cycling the device (hardware power switch off/on) automatically resets `otadata` and returns to the Retro-Go launcher (OTA slot 0).

---

## How to Set Up Your SD Card

### 1. Standalone Applications & Ports (Apps tab)
Place single-use `.bin` binaries into `/roms/apps/`:
```text
SD Card root/
└── roms/
    └── apps/
        ├── Doom.bin
        ├── MiniTV.bin
        └── ArduinoSketch.bin
```
These will appear under the **Apps** tab in Retro-Go.

### 2. Dynamic Emulator Cores
Place compiled Retro-Go emulator `.bin` binaries into `/roms/cores/`:
```text
SD Card root/
└── roms/
    ├── cores/
    │   ├── snes9x.bin
    │   ├── fmsx.bin
    │   ├── gwenesis.bin
    │   ├── gbsp.bin
    │   └── prboom-go.bin
    ├── snes/
    │   └── SuperMarioWorld.smc
    └── msx/
        └── MetalGear.rom
```
* The launcher will automatically detect the core in `/roms/cores/` and display its corresponding tab (e.g., **Super Nintendo**, **MSX**, **Sega Mega Drive**).
* Selecting a game will flash the core (if not already flashed) and boot directly into the game.
* When you exit the game via the Retro-Go menu, you return cleanly to the Launcher.
* Selecting another game on the same console will **boot instantly without reflashing**!

---

## How It Works

### 1. The Flashing Flow:
1. When you select a game or application, the Launcher determines if the core needs to be flashed:
    * **If already in `bootstrapped`**: Launcher boots directly into the game with zero flashing delay.
    * **If not flashed**: Launcher saves target boot parameters in NVS and switches to `bootstrap`.
2. `bootstrap` erases and flashes the core block-by-block with a graphical progress bar.
3. Once complete, `bootstrap` sets `LastFlashedApp` and boots into the selected partition with the target ROM parameters.

### 2. The Power-Cycle Recovery Hook:
Because third-party binaries do not contain Retro-Go library code, they cannot write to `otadata` to return you to the launcher when you exit. To solve this, we implemented a custom hook inside the second-stage bootloader at `0x1000`:

*   **Software Reset**: If the reset was triggered by a software command (e.g., when `bootstrap` launches the game), the hook lets it boot into the game partition normally.
*   **Hardware Reset (Power Switch)**: If the device is physically powered off and on, the hook erases the `otadata` sector. The ESP32 bootloader then automatically defaults to booting OTA slot 0 (the Retro-Go `launcher`).

> [!NOTE]
> The custom bootloader hook is implemented inside `launcher/bootloader_components/boot_hooks/hooks.c` and is statically compiled into the custom `bootloader.bin`. The linking phase uses `-Wl,--undefined=bootloader_hooks_include` to guarantee the GNU Linker does not optimise out these weak overrides.

---

## What Works Well

*   **Flashing standard ESP32 binaries & Retro-Go cores**: Works with both standalone Arduino/IDF binaries and full Retro-Go emulator cores.
*   **Flash Wear Reduction**: Eliminates write cycles for consecutive game sessions on the same emulator core.
*   **Full Multi-Emulator on 4MB Devices**: Overcomes flash limitations by loading heavy cores dynamically from SD card.
*   **Failsafe Recovery**: Power-cycle always returns safely to the launcher.

---

## Limitations (What *Doesn't* Work Well)

*   **Size Constraints**: The binary size is strictly limited by the `bootstrapped` partition size (default: 1.5 MB). Refer to the [Customising the Partition Size](#customising-the-partition-size) section below to adjust this limit.
*   **No Soft Exit in Non-Retro-Go Third-Party Apps**: Standalone third-party apps without Retro-Go menu integration still require toggling the hardware power switch to return to the launcher. (Native Retro-Go cores exit cleanly via the in-game menu).
*   **Hardware Conflict Risks**: Third-party binaries must be compiled for your specific hardware target pinout.

---

## Customising the Partition Size

By default, the `bootstrapped` partition is configured with a size of **1.5 MB** (`1572864` bytes) to ensure the standard 4MB flash layout has plenty of room for all core Retro-Go applications and emulators. 

If you need to flash larger third-party applications, you can easily increase the partition size by editing `rg_tool.py`.

### How to Modify the Partition Size:
1. Open the [rg_tool.py] file in a text editor.
2. Search for the following line:
   ```
   args += ["0", str(ota_next_id), "1048576", "bootstrapped", "none"]
   ```
3. Change `"1572864"` to the decimal byte size corresponding to your desired target size:

| Desired Size | Decimal Value to Use in `rg_tool.py` | Notes / Hardware Guidance |
| :--- | :--- | :--- |
| **1.0 MB** | `"1048576"` | Safely fits all standard 4MB flash targets with core emulators. |
| **1.5 MB** | `"1572864"` | *(Default)* Should *just* fit standard 4MB flash targets when building firmware image (e.g., `launcher` `bootstrap` & `retro-core` only). |
| **2.0 MB** | `"2097152"` | Extremely tight on 4MB flash; requires compiling only the absolute barebones launcher and bootstrap. |
| **2.5 MB** | `"2621440"` | Ideal for 8MB or 16MB flash targets. |
| **3.0 MB** | `"3145728"` | Ideal for 8MB or 16MB flash targets. |
| **4.0 MB** | `"4194304"` | Recommended only for 8MB or 16MB flash targets. |

> [WARNING]
> If you are using a **4MB flash** device, your total firmware image size cannot exceed 4MB (4,194,304 bytes). If you set the `bootstrapped` partition to **1.5MB** or **2.0MB**, you must compile a lightweight image to avoid partition overlap or compile errors.

---



## Installation & Compilation

To build a lightweight, 4MB-flash-compliant Retro-Go image containing `launcher`, `retro-core`, and `bootstrap`, run:

```bash
python rg_tool.py --target cyd release launcher retro-core bootstrap
```

This will produce the image file within 4MB flash.

---

## Building & Adding SD Emulator Cores

You can compile any Retro-Go emulator or port individually and place it on your SD card without needing to reflash the main firmware image:

### 1. Compile the Core Binary
Run the build command for your target hardware (e.g. `cyd`):

```bash
python rg_tool.py --target cyd build <core_name>
```

**Examples:**
* **Super Nintendo (SNES)**:
  ```bash
  python rg_tool.py --target cyd build snes9x
  ```
* **Multiple**:
  ```bash
  python rg_tool.py --target cyd build snes9x gwenesis fmsx prboom-go
  ```

### 2. Copy the Binary to SD Card
After the build completes, copy the generated `.bin` file from the app's `build/` directory into `/roms/cores/` on your SD card:
```text
<core_name>/build/<core_name>.bin  ──>  SD:/roms/cores/<core_name>.bin
```
*Example:* Copy `snes9x/build/snes9x.bin` $\rightarrow$ `/roms/cores/snes9x.bin`.

### 3. Adding New Emulators to the Launcher
If adding a newly ported emulator, register it in `launcher/main/applications.c`:
```c
application("System Name", "rom_folder", "ext1 ext2 zip", "core_name", 0);
```
*Example:* application("Commodore 64", "c64", "crt zip prg d64 t64", "frodo", 0);

As long as `/roms/cores/<core_name>.bin` exists on the SD card, the system tab will automatically appear in your Retro-Go menu!



