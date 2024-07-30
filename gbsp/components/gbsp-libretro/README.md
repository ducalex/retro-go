
gpSP for libretro
=================

This is a fork of notaz's gpSP (https://github.com/notaz/gpsp) for libretro
frontends (like Retroarch). Check the original_readme.txt file for more info.

The current maintainer/main contributor is davidgfnet (check out the repo at
https://github.com/davidgfnet/gpsp). This version has a bunch of fixes and
features.

Feature list
============

gpSP features a dynamic recompiler that makes it quite fast (compared to other
emulators at least). It supports x86/x64, ARMv6/7 and ARMv8 and MIPS (32 and 64
bits), for other platforms an interpreter is available (albeit slower). Both
little and big endian systems are supported. Some supported platforms are PSP,
PS2, GameCube/Wii, Nintendo 3DS and Switch, Dingux/OpenDingux and of course
PC and Android.

At the moment this emulator lacks a native UI and must be played using some
libretro frontend (we recommend Retroarch). A list of available frontends can
be found at https://docs.libretro.com/development/frontends/

Many new features (compared to the original release) are:

 - Wireless Adapter networked multiplayer!
 - Rumble support (including Gameboy Player emulation)
 - New video renderer, fixes many graphical bugs & adds many effects (mosaic).
 - Many long standing issues have been fixed.
 - Slightly better performance (for some games at least!)
 - Better audio (fixed many audio related bugs).
 - Ships an opensource BIOS replacement,we recommend using the original though.

Planned features (aka the TODO list)
====================================

Some features I'd like to see (in loose priority order):

 - GBA link emulation (for some games, perhaps with patches).
 - Improve RFU (Wireless Adapter) emulation through research.
 - Bringing back the native UI for PC, PSP and perhaps PS2/3DS/Wii.
 - A native UI with Multiplayer support for portable devices with wifi support.
 - A better BIOS emulation and perhaps a newer better open BIOS.
 - Dynarec rewrite: make it easier to add new drcs and share more code.
 - Adding some funny DRCs like PowerPC or SH4.


