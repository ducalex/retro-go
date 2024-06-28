@echo off

del launcher.exe retro-core.exe


tcc -Wall -DRG_TARGET_SDL2 -DRETRO_GO -DCJSON_HIDE_SYMBOLS -D_HAVE_STDINT_H=1 ^
 -Icomponents/retro-go ^
 -Icomponents/retro-go/libs/cJSON ^
 -Icomponents/retro-go/libs/lodepng ^
 -Ilauncher/main ^
 components/retro-go/*.c ^
 components/retro-go/fonts/*.c ^
 components/retro-go/libs/cJSON/*.c ^
 components/retro-go/libs/lodepng/*.c ^
 launcher/main/*.c ^
 -lSDL2 ^
 -o launcher.exe || goto :fail



tcc -Wall -DRG_TARGET_SDL2 -DRETRO_GO -DCJSON_HIDE_SYMBOLS -D_HAVE_STDINT_H=1 ^
 -Icomponents/retro-go ^
 -Icomponents/retro-go/libs/cJSON ^
 -Icomponents/retro-go/libs/lodepng ^
 -Iretro-core/components/gnuboy ^
 -Iretro-core/components/gw-emulator/src ^
 -Iretro-core/components/gw-emulator/src/cpus ^
 -Iretro-core/components/gw-emulator/src/gw_sys ^
 -Iretro-core/components/nofrendo ^
 -Iretro-core/components/pce-go ^
 -Iretro-core/components/smsplus ^
 -Iretro-core/main ^
 components/retro-go/*.c ^
 components/retro-go/fonts/*.c ^
 components/retro-go/libs/cJSON/*.c ^
 components/retro-go/libs/lodepng/*.c ^
 retro-core/components/gnuboy/*.c ^
 retro-core/components/gw-emulator/src/*.c ^
 retro-core/components/gw-emulator/src/cpus/*.c ^
 retro-core/components/gw-emulator/src/gw_sys/*.c ^
 retro-core/components/nofrendo/mappers/*.c ^
 retro-core/components/nofrendo/nes/*.c ^
 retro-core/components/nofrendo/*.c ^
 retro-core/components/pce-go/*.c ^
 retro-core/components/smsplus/*.c ^
 retro-core/components/smsplus/cpu/*.c ^
 retro-core/components/smsplus/sound/*.c ^
 retro-core/main/*.c ^
 -lSDL2 ^
 -o retro-core.exe || goto :fail


:loop
launcher.exe && exit
retro-core.exe && exit
goto loop


exit


:fail
echo Compilation failed
