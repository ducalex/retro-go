#!/bin/bash

# Notes:
# - Enabling netplay in an emulator increases its size by ~350KB
# - Keep at least 32KB free in a partition for future updates
# - Partitions must be 64K aligned

basedir="$(dirname "$(readlink -f "$0")")"
mkfw="./tools/mkfw/mkfw"

release=`date +%Y%m%d`;

if [ ! -f "$mkfw" ]; then
	echo "Building mkfw..."
	cd "`dirname "$mkfw"`"
	make
fi

cd "$basedir"

$mkfw "Retro-Go ($release)" assets/tile.raw \
	0  16  524288  launcher   retro-go/build/retro-go.bin         \
	0  17  458752  nofrendo   nofrendo-go/build/nofrendo-go.bin   \
	0  18  458752  gnuboy     gnuboy-go/build/gnuboy-go.bin       \
	0  19  524288  smsplusgx  smsplusgx-go/build/smsplusgx-go.bin \
	0  20  458752  huexpress  huexpress-go/build/huexpress-go.bin \
	0  21  458752  handy      handy-go/build/handy-go.bin

mv firmware.fw retro-go_$release.fw
