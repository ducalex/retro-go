# ESPlay Micro V2
- Status: WORK IN PROGRESS!
- Ref: https://www.makerfabs.com/esplay-micro-v2.html

This port developed for the Micro V2 listed above. Compatibility with the elusive V1 is unknown.

Issues: Menu, L, and R aren't properly mapped yet.

# Hardware info
- ESP32-WROVER-B (SoC 16MB Flash + 8MB PSRAM)
- PCF8574 I2C GPIO (To connect the extra buttons)
- UDA1334A (I2S DAC)
- YT240L010 (ILI9341 LCD)
- TP4054 (Lipo Charger IC)
- CH340C (USB to Serial)
- 3.5mm Headphone jack 0_o

The following snippet was in the `config.h`, might contain still useful information.
````
// Experimental. Caused "Menu" to be mapped to a D-pad direction.
//#define RG_GPIO_GAMEPAD_X           GPIO_NUM_NC
//#define RG_GPIO_GAMEPAD_Y           GPIO_NUM_NC
//#define RG_GPIO_GAMEPAD_SELECT      GPIO_NUM_0
//#define RG_GPIO_GAMEPAD_START       GPIO_NUM_36
//#define RG_GPIO_GAMEPAD_A           GPIO_NUM_32
//#define RG_GPIO_GAMEPAD_B           GPIO_NUM_33
//#define RG_GPIO_GAMEPAD_MENU        GPIO_NUM_35
//#define RG_GPIO_GAMEPAD_OPTION      GPIO_NUM_NC
````

# Images
![device.jpg](device.jpg)
