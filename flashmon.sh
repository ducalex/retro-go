#!/bin/bash
COMPORT="$1"
BINFILE="$2"
OFFSET="$3"

esptool.py $COMPORT $BINFILE.bin $OFFSET no reset
idf_monitor.py --port $COMPORT build/$BINFILE.elf
