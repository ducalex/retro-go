# ES3C28P
- Status: Fully supported
- Ref: LCDWIKI/QDtech ES3C28P, commonly sold as a "2.8 inch ESP32-S3 touchscreen development board"

An inexpensive off-the-shelf ESP32-S3 touchscreen board. It has no buttons and no SD
slot, so it uses the touchscreen as its gamepad and stores ROMs in a FAT partition on
the internal flash.

# Hardware info
- Module: ESP32-S3-WROOM-1 N16R8 (16MB flash, 8MB octal PSRAM)
- Display: 2.8" 320x240 ILI9341 over SPI, PWM backlight
- Touch: FT6336U capacitive controller on I2C
- Audio: ES8311 codec on I2S with an NS4150 amplifier and onboard speaker
- USB-C connector, native USB serial/JTAG
- Only physical button is BOOT (GPIO0), mapped to MENU

# Notes
- The octal PSRAM occupies GPIO33-37, so those pins are unavailable.
- The panel is natively inverted and needs `INVON` (0x21) in its init sequence.
- The touch controller's reset line (GPIO18) must be left unconfigured. Driving it
  low locks up the I2C bus; the FT6336U resets by itself at power-on.
- The amplifier gate is active-low (LOW enables the amp), hence
  `RG_GPIO_SND_AMP_ENABLE_INVERT`.
- The LCD reset line is bonded to CHIP_PU/EN, so there is no separate reset GPIO.
- The board has battery pads and a divider on GPIO9, but no protection IC and no cell
  fitted from the factory, so no battery driver is configured.

# Layout
The touch gamepad uses a landscape layout: d-pad bottom-left (the zones overlap so
diagonals work), B/A bottom-right, SELECT/START top-centre and MENU/OPTION in the top
corners. The zones are defined by `RG_GAMEPAD_TOUCH_MAP` in `config.h` and are easy to
move around.
