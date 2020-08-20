#!/bin/bash

COMPORT="$1"
OFFSET="$2"
BINFILE="build/*-go"

esptool.py --chip esp32 --port $COMPORT --baud 1152000 --before default_reset write_flash \
           -u --flash_mode qio --flash_freq 80m --flash_size detect $OFFSET $BINFILE.bin \
           || exit

idf_monitor.py --port $COMPORT $BINFILE.elf

# make monitor // too slow to start
