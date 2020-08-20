#!/bin/bash

BUILD_CMD="make -j 6 app"
CLEAN_CMD="make clean"
# BUILD_CMD="idf.py app"
# CLEAN_CMD="idf.py fullclean"

echo ""
echo "Building the launcher"
cd retro-go
$CLEAN_CMD
$BUILD_CMD || exit

echo ""
echo "Building the NES emulator"
cd ../nofrendo-go
$CLEAN_CMD
$BUILD_CMD || exit

echo ""
echo "Building the Gameboy emulator"
cd ../gnuboy-go
$CLEAN_CMD
$BUILD_CMD || exit

echo ""
echo "Building the PC Engine emulator"
cd ../huexpress-go
$CLEAN_CMD
$BUILD_CMD || exit

echo ""
echo "Building the SMS/Gamegear/Coleco emulator"
cd ../smsplusgx-go
$CLEAN_CMD
$BUILD_CMD || exit

echo ""
echo "Building the Lynx emulator"
cd ../handy-go
$CLEAN_CMD
$BUILD_CMD || exit

echo ""
echo "Building firmware"
cd ..
./mkfw.sh

echo ""
echo "All done!"
cd ..
