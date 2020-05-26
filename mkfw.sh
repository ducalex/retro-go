#!/bin/bash
# To build the .fw you need the mkfw tool, get it from one of these sources:
# - https://github.com/OtherCrashOverride/odroid-go-firmware
# - https://github.com/ducalex/odroid-go-multi-firmware
#
# Notes:
# - Enabling netplay in an emulator increases its size by ~350KB
# - Keep at least 32KB free in a partition for future updates
# - Partitions must be 64K aligned

mkfw="../odroid-go-multi-firmware/tools/mkfw/mkfw"
release=`date +%Y%m%d`;

$mkfw "Retro-Go ($release)" assets/tile.raw \
	0  16  589824  frontend   retro-go/build/retro-go.bin         \
	0  17  851968  nofrendo   nofrendo-go/build/nofrendo-go.bin   \
	0  18  851968  gnuboy     gnuboy-go/build/gnuboy-go.bin       \
	0  19  458752  smsplusgx  smsplusgx-go/build/smsplusgx-go.bin \
	0  20  458752  huexpress  huexpress-go/build/huexpress-go.bin \
	0  21  458752  handy      handy-go/build/handy-go.bin

mv firmware.fw retro-go_$release.fw
