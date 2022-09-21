# Duke Nukem 3D ESP32 Port
An ESP32 port of Duke Nukem 3D - based on the Win/Mac/Linux port: Chocolate Duke3D (https://github.com/fabiensanglard/chocolate_duke3D)

## Requirements
- An ODROID-GO

or

- ESP32 WROVER (4Mb PSRAM)
- An SPI LCD screen - preferable using the ILI9341 chipset
- An SD Card and SPI or SDMMC reader
- Some input buttons

and

- The data files from Duke Nukem 3D - Atomic Edition (v1.5) or lower.  You can buy it here if you don't already have it:
(https://www.zoom-platform.com/#store-duke-nukem-3d-atomic-edition)

## SD Card Setup
On your SD Card, put all of the files from Duke Nukem 3D v1.5 (or lower) into a folder called "duke3d".

## ODROID-GO - Flash by Firmware File
- Copy /release/Duke3D.fw (https://github.com/jkirsons/Duke3D/raw/master/release/Duke3D.fw) from the GitHub repository to /odroid/firmware/Duke3D.fw on the SD Card.
- Power off the ODROID-GO, hold B, and power on the ODROID-GO.
- When the ODROID-GO menu displays, select Duke3D, press A, then press Start.
- It should take about 20 seconds to flash, and 30 seconds to load to the first intro screen.

---------------------------------------------------------------------
### Steps below are not needed if you have flashed by the firmware file
---------------------------------------------------------------------

## Configuration
Run "make menuconfig" and check the settings under ESP32-DUKE3D platform-specific configuration, and either select ODROID-GO:
![Config Image](https://github.com/jkirsons/Duke3D/raw/master/documents/config%20ODROID-GO.png)

or Custom Hardware, and configure all GPOIs:
![Config Image](https://github.com/jkirsons/Duke3D/raw/master/documents/config.png)

## Compiling
As of 19.11.2018, you cannot use the OtherCrashOveride ESP-IDF, you must use the original ESP-IDF.  

You however must comment out this line:
https://github.com/espressif/esp-idf/blob/16de6bff245dec5e63eee994f53a08252be720d4/components/driver/sdspi_host.c#L279

## LEGAL STUFF
"Duke Nukem" is a registered trademark of Apogee Software, Ltd. (a.k.a. 3D Realms).
"Duke Nukem 3D" copyright 1996 - 2003 3D Realms. All trademarks and copyrights reserved.

This repository is split into the following code bases:

### Build Engine
Folder: <b>"components/Engine"</b>

The Build Engine is licensed under the Build License - see BUILDLIC.TXT

       // "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
       // Ken Silverman's official web site: "http://www.advsys.net/ken"
       // See the included license file "BUILDLIC.TXT" for license info.

Most files in this code base have been modified, and do not represent the original content from this author.

### Game Code
Folder: <b>"components/Game"</b>

The original Duke Nukem Game code is licensed under GNU General Public License v2.0 (see LICENSE).

The Chocolate Duke modifications were not released with a license, but permission from the author to "do whatever you want with my code" (see https://github.com/fabiensanglard/chocolate_duke3D/issues/48)

Most files in this code base have been modified, and do not represent the original content from these authors.

### SDL Library
Folder: <b>"components/SDL"</b>

The SDL library function wrapper contains parts of the Simple DirectMedia Layer library that is licensed under the ZLIB license (see LICENSE_ZLIB).  

All files in this code base are either new or extensively modified, and do not represent the original SDL Library.

### SPI LCD Functions
Files: <b>"components/SDL/spi_lcd.c and components/SDL/spi_lcd.h"</b>

The SPI LCD functions are Copyright 2016-2017 Espressif Systems (Shanghai) PTE LTD and licenced under the Apache License 2.0 (see http://www.apache.org/licenses/LICENSE-2.0)

These files have been modified, and do not represent the original content from this author.

### ESP32 Wrapper
Folder: <b>"main"</b>

Parts by me are licensed under GNU General Public License v2.0 (see LICENSE).
