# NM-CYD-C5
- Status: supported
- Ref: https://github.com/RockBase-iot/NM-CYD-C5

# Hardware info
- Module: ESP32-C5-WROOM-1
- CPU: single-core RISC-V, 240 MHz
- Memory: 16 MB Flash, 8 MB PSRAM
- Display: 2.8 inch 240x320 ST7789 over SPI
- Storage: microSD over the same SPI bus as the display and XPT2046 touch controller

# Known issues
- Requires ESP-IDF 5.5 or newer for ESP32-C5 support.
- The on-board XPT2046 touch controller is mapped to virtual gamepad zones. It is single-touch, so it is usable for launcher navigation and basic testing but does not replace a physical multi-button gamepad.
- Touch layout in landscape mode: left side is the d-pad, right middle is B/A, top-right is Menu, bottom-right is Select/Start.
- Set `RG_ENABLE_TOUCH_GAMEPAD` to `0` in the target config to disable the virtual touch buttons entirely.
- Audio defaults to the dummy sink because the board documentation does not list an on-board DAC or speaker.
- The single-core CPU has less scheduling headroom than existing dual-core ESP32/S3 targets; start with `launcher` and `retro-core` before testing heavier applications.
