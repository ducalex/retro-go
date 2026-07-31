# OpenLara for Retro-Go

This project is a port of the **OpenLara** (Tomb Raider) engine to the ESP32 platform using the **Retro-Go** ecosystem. It is specifically optimised for the ESP32-S3 and ESP32-P4 with at least 8MB of PSRAM. Original ESP32 not supported due to PSRAM limits.


## Features
- **Tomb Raider 1 PC Support**: Fully playable with correct lighting and textures.
- **Software Rasteriser**: Custom 320x240 RGB565 renderer with memory-optimised 16-bit depth buffering.
- **PSRAM Optimised**: Intelligent memory management (`RG_ALLOC`) ensures level data, textures, and sounds reside in 8MB PSRAM.
- **High-Fidelity Audio**: Optimised 11025Hz Mono pipeline with averaging downsampling.
- **Save Anywhere**: Direct, one-touch saving and loading via the Passport menu.
- **UI Scaling**: 2x font and interface scaling optimised for small 2-3" displays.

---

## Known Limitations: FMV Videos
FMV video playback is **disabled by default**. 
- **Reason**: The ESP32's CPU cannot simultaneously decode heavy video streams (via the Escape/Cinepak codecs) and maintain the performance required to run the game engine. 
- **Result**: Enabling FMV playback on handhelds causes stuttering, audio crackling, and memory pressure that leads to system panics. 
- **Status**: The engine is configured to skip all intro and cutscene videos to prioritise smooth, stable gameplay.

---

## Installation
Create an empty file called Tomb Raider.tr1 (or any name, aslong as the extension is .tr1) and place it in `/roms/openlara`

## SD Card Structure

The engine requires the original Tomb Raider 1 PC assets and must be placed in the following directory on your SD card:

`roms/openlara/data/`

### Required Folder Structure:
```text
roms/openlara/data/
├── AUDIO/
│   └── 1/              <-- Place converted .WAV files here, engine is setup to read wavs 002 to 060.
├── DATA/
│   ├── TITLE.PHD      <-- Main entry point
│   ├── TITLEH.PNG     <-- Title background (optimised)
│   ├── LEVEL1.PHD     <-- Level data
│   └── ...            <-- All other .PHD files
└── PIX/               <-- High-res loading screens (optimised)
```

---

## Media Preparation

Original Tomb Raider 1 PC files require conversion for optimal performance on the ESP32.

### Automated Conversion
Use the scripts in the `scripts` folder:
1. Copy your original assets into the `AUDIO`, `DATA`, and `LEVEL` folders.
2. Run `convert_media.bat` (Windows) or `convert_media.sh` (Linux/macOS) from the root of the data directory. ffmpeg is required. 
3. The scripts will:
    - Convert audio to **11025Hz Mono WAV**.
    - Resize background images to **320x240 PNG**.
    - Optimise the Title Screen background.
    - Remove the unsupported `FMV` folder.

Scripts/ffmpeg can be deleted once conversion is complete.

### Manual Conversion Settings
- **Audio**: 11025Hz, Mono, 16-bit PCM.
- **Backgrounds**: 320x240 PNG (Lanczos scaling recommended).

---

## Saving and Loading
- **In-Game**: Open the Passport menu. Page 0 is **SAVE PROGRESS**, Page 1 is **LOAD PROGRESS**, Page 2 is **EXIT TO TITLE**.
- **Main Menu**: Page 0 is **LOAD GAME**, Page 1 is **START GAME**, Page 2 is **EXIT GAME**.
- **Persistence**: Saves are automatically managed in `retro-go/saves/openlara/`.

---

## Credits
- **Xproger**: Original OpenLara engine.
- **ducalex**: Retro-Go firmware.
- **Core Design / Eidos**: Original Tomb Raider developers.
