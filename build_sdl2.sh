#!/bin/bash

rm -f launcher.exe retro-core.exe

gcc -Wall -DRG_TARGET_SDL2 -DRETRO_GO -DCJSON_HIDE_SYMBOLS -DSDL_MAIN_HANDLED=1 \
 -Icomponents/retro-go \
 -Icomponents/retro-go/libs/cJSON \
 -Icomponents/retro-go/libs/lodepng \
 -Ilauncher/main \
 -Wno-address-of-packed-member \
 $(sdl2-config --cflags) \
  -Dapp_main=SDL_Main \
 components/retro-go/*.c \
 components/retro-go/fonts/*.c \
 components/retro-go/libs/cJSON/*.c \
 components/retro-go/libs/lodepng/*.c \
 launcher/main/*.c \
 $(sdl2-config --libs) \
 -lstdc++ \
 -o launcher.exe -O2 -pg -no-pie


gcc -Wall -DRG_TARGET_SDL2 -DRETRO_GO -DCJSON_HIDE_SYMBOLS -DSDL_MAIN_HANDLED=1 \
 -Icomponents/retro-go \
 -Icomponents/retro-go/libs/cJSON \
 -Icomponents/retro-go/libs/lodepng \
 -Iretro-core/components/gnuboy \
 -Iretro-core/components/gw-emulator/src \
 -Iretro-core/components/gw-emulator/src/cpus \
 -Iretro-core/components/gw-emulator/src/gw_sys \
 -Iretro-core/components/handy \
 -Iretro-core/components/nofrendo \
 -Iretro-core/components/pce-go \
 -Iretro-core/components/snes9x \
 -Iretro-core/components/smsplus \
 -Iretro-core/main \
 -Wno-address-of-packed-member \
 $(sdl2-config --cflags) \
  -Dapp_main=SDL_Main \
 components/retro-go/*.c \
 components/retro-go/fonts/*.c \
 components/retro-go/libs/cJSON/*.c \
 components/retro-go/libs/lodepng/*.c \
 retro-core/components/gnuboy/*.c \
 retro-core/components/gw-emulator/src/*.c \
 retro-core/components/gw-emulator/src/cpus/*.c \
 retro-core/components/gw-emulator/src/gw_sys/*.c \
 retro-core/components/handy/*.cpp \
 retro-core/components/nofrendo/mappers/*.c \
 retro-core/components/nofrendo/nes/*.c \
 retro-core/components/nofrendo/*.c \
 retro-core/components/pce-go/*.c \
 retro-core/components/snes9x/*.c \
 retro-core/components/smsplus/*.c \
 retro-core/components/smsplus/cpu/*.c \
 retro-core/components/smsplus/sound/*.c \
 retro-core/main/*.c \
 retro-core/main/*.cpp \
 $(sdl2-config --libs) \
 -lstdc++ \
 -o retro-core.exe -O2 -pg -no-pie

./launcher.exe && ./retro-core.exe

# && gprof.exe retro-core.exe gmon.out > profile.txt
# gdb -iex 'set pagination off' -ex run ./retro-core.exe
