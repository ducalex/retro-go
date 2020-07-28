#!/bin/bash

BUILD_CMD="make -j 6"
# BUILD_CMD="idf.py build"

echo ""
echo "Building the launcher"
cd retro-go
rm -f build/main/* build/odroid/*
$BUILD_CMD || exit

echo ""
echo "Building the NES emulator"
cd ../nofrendo-go
rm -f build/main/* build/odroid/*
$BUILD_CMD || exit

echo ""
echo "Building the Gameboy emulator"
cd ../gnuboy-go
rm -f build/main/* build/odroid/*
$BUILD_CMD || exit

echo ""
echo "Building the PC Engine emulator"
cd ../huexpress-go
rm -f build/main/* build/odroid/*
$BUILD_CMD || exit

echo ""
echo "Building the SMS/Gamegear/Coleco emulator"
cd ../smsplusgx-go
rm -f build/main/* build/odroid/*
$BUILD_CMD || exit

echo ""
echo "Building the Lynx emulator"
cd ../handy-go
rm -f build/main/* build/odroid/*
$BUILD_CMD || exit

echo ""
echo "Building firmware"
cd ..
./mkfw.sh

echo ""
echo "All done!"
cd ..
