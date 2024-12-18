#!/bin/bash

# Supported systems: Linux / MINGW32 / MINGW64
# Required: SDL2

CC="gcc"
# BUILD_INFO="RG:$(git describe) / SDL:$(sdl2-config --version)"
CFLAGS="-no-pie -DRG_TARGET_SDL2 -DRETRO_GO -DCJSON_HIDE_SYMBOLS -DSDL_MAIN_HANDLED=1 -DRG_BUILD_INFO=\"SDL2\" -Dapp_main=SDL_Main $(sdl2-config --cflags)"
INCLUDES="-Icomponents/retro-go -Icomponents/retro-go/libs/cJSON -Icomponents/retro-go/libs/lodepng -Icomponents/retro-go/libs/miniz"
SRCFILES="components/retro-go/*.c components/retro-go/drivers/audio/*.c components/retro-go/fonts/*.c
		  components/retro-go/libs/cJSON/*.c components/retro-go/libs/lodepng/*.c components/retro-go/libs/miniz/*.c"
LIBS="$(sdl2-config --libs) -lstdc++"

echo "Cleaning..."
rm -f launcher.exe retro-core.exe gmon.out

echo "Building launcher..."
$CC $CFLAGS $INCLUDES -Ilauncher/main $SRCFILES launcher/main/*.c $LIBS -o launcher.exe

echo "Building retro-core..."
$CC $CFLAGS $INCLUDES \
	-Iretro-core/components/gnuboy \
	-Iretro-core/components/gw-emulator/src \
	-Iretro-core/components/gw-emulator/src/cpus \
	-Iretro-core/components/gw-emulator/src/gw_sys \
	-Iretro-core/components/handy \
	-Iretro-core/components/nofrendo \
	-Iretro-core/components/pce-go \
	-Iretro-core/components/snes9x \
	-Iretro-core/components/snes9x/src \
	-Iretro-core/components/smsplus \
	-Iretro-core/main \
	$SRCFILES \
	retro-core/components/gnuboy/*.c \
	retro-core/components/gw-emulator/src/*.c \
	retro-core/components/gw-emulator/src/cpus/*.c \
	retro-core/components/gw-emulator/src/gw_sys/*.c \
	retro-core/components/handy/*.cpp \
	retro-core/components/nofrendo/mappers/*.c \
	retro-core/components/nofrendo/nes/*.c \
	retro-core/components/nofrendo/*.c \
	retro-core/components/pce-go/*.c \
	retro-core/components/snes9x/src/*.c \
	retro-core/components/smsplus/*.c \
	retro-core/components/smsplus/cpu/*.c \
	retro-core/components/smsplus/sound/*.c \
	retro-core/main/*.c \
	retro-core/main/*.cpp \
	$LIBS \
	-o retro-core.exe

echo "Running"
./launcher.exe && ./retro-core.exe

# && gprof.exe retro-core.exe gmon.out > profile.txt
# gdb -iex 'set pagination off' -ex run ./retro-core.exe
