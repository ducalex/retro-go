# StellaDS for Retro-Go

A highly optimised port of the Atari 2600 VCS emulator for the **Retro-Go** ecosystem, targeting ESP32, ESP32-S3, and ESP32-P4 handhelds.

This port brings Dave Bernazzani's legendary Nintendo DS optimisations to the ESP32, achieving a rock-solid **60 FPS** even on the original ESP32 hardware without the need for frameskip.

## 🚀 Features

*   **Full Speed:** 60 FPS (NTSC) / 50 FPS (PAL) on all supported ESP32 variants.
*   **Zero-Copy Rendering:** Direct-to-buffer rendering eliminates memory copy overhead.
*   **Optimised Memory Layout:** Relocates massive TIA lookup tables to PSRAM while keeping critical execution loops in IRAM.
*   **Retro-Go Integration:** Full support for the Retro-Go game menu, savestates, and native scaling/filtering options.
*   **Difficulty Switches:** Toggle Player 1 and Player 2 difficulty (Pro/Novice) directly from the "Emulator Options" menu.
*   **Audio:** Centered 22050Hz mono audio output optimized for handheld speakers.
*   **Difficulty Switches:** Toggle Player 1 difficulty (Pro/Novice) directly from the "Emulator Options" menu.
*   **Zip Support:** Load `.a26` ROMs directly from compressed `.zip` files.

## 🛠 Technical Highlights

To achieve 60 FPS on microcontroller hardware, this port implements several surgical optimisations:
*   **PSRAM Cache Workaround:** Bypasses unaligned 32-bit read bugs in ESP32 PSRAM by utilising safe 8-bit mask paths in the TIA rendering engine.
*   **Dynamic Stride Resync:** Synchronises the Atari colour clocks with the Retro-Go display pitch at the end of every scanline to eliminate pixel drift.
*   **Memory Safety:** Critical audio and save buffers moved to static/external memory to prevent stack overflows on memory-constrained devices.

## 🎮 Controls

*   **D-Pad:** Atari Joystick
*   **Button A / B:** Fire Button
*   **Select:** Console Select
*   **Start:** Console Reset
*   **Menu Button:** Game Menu (Savestates, Exit)
*   **Option Button:** System Options (Scaling, Filtering)

## 📜 Credits & Thanks

This project is a bridge between several amazing pieces of software:

*   **The Stella Team:** For the original Stella emulator core, the foundation of Atari 2600 emulation for decades.
*   **Dave Bernazzani ([wavemotion-dave](https://github.com/wavemotion-dave)):** A huge thank you for the **StellaDS** port. His incredible work optimising the core for the ARM9 (Nintendo DS) is the "engine room" of this project. Without his assembly-level insights and optimised lookup tables, 60 FPS on the ESP32 would not be possible.
*   **The Retro-Go Team:** For the fantastic ecosystem and hardware abstraction layer that makes cross-handheld development easy.

