#!/bin/bash
COMPORT="$1"
OFFSET="$2"
BINFILE="*-go"

# make -j 6 || exit

esptool.py --chip esp32 --port $COMPORT --baud 1152000 --before default_reset write_flash \
           -u --flash_mode qio --flash_freq 80m --flash_size detect $OFFSET build/$BINFILE.bin \
           || exit

idf_monitor.py --port $COMPORT build/$BINFILE.elf
