#!/bin/bash

echo ""
echo "Building the launcher"
cd retro-go
rm -f build/main/* # Force rebuild main to update date
make -j 6 || exit

echo ""
echo "Building the Gameboy emulator"
cd ../gnuboy-go
rm -f build/main/*
make -j 6 || exit

echo ""
echo "Building the PC Engine emulator"
cd ../huexpress-go
rm -f build/main/*
make -j 6 || exit

echo ""
echo "Building the NES emulator"
cd ../nofrendo-go
rm -f build/main/*
make -j 6 || exit

echo ""
echo "Building the SMS/Gamegear/Coleco emulator"
cd ../smsplusgx-go
rm -f build/main/*
make -j 6 || exit

echo ""
echo "All done!"
cd ..
