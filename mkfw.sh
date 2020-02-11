#!/bin/bash
# To build the .fw you need the mkfw tool, get it from one of these sources:
# - https://github.com/OtherCrashOverride/odroid-go-firmware
# - https://github.com/ducalex/odroid-go-multi-firmware

mkfw="../odroid-go-multi-firmware/tools/mkfw/mkfw"
release=`date +%Y%m%d`;

$mkfw "Retro-Go ($release)" assets/tile.raw \
	0 16 655360 frontend retro-go/build/retro-go.bin  \
	0 17 524288 nesemu nesemu-go/build/nesemu-go.bin  \
	0 18 458752 gnuboy gnuboy-go/build/gnuboy-go.bin  \
	0 19 983040 smsplusgx smsplusgx-go/build/smsplusgx-go.bin

mv firmware.fw retro-go.fw
