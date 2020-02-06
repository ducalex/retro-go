#!/bin/bash

echo "Building the interface"
cd retro-go
make -j 6 || exit

echo "Building the NES emulator"
cd ../nesemu-go
make -j 6 || exit

echo "Building the Gameboy emulator"
cd ../gnuboy-go
make -j 6 || exit

echo "Building the SMS/Gamegear/Coleco emulator"
cd ../smsplusgx-go
make -j 6 || exit

cd ..

echo "All done!"
