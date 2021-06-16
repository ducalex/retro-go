# Retro-Go 1.25.3  (2021-05-30)
This is a bug fix release that fixes the following issues:
- Occasional crash when saving a game
- Corrupted or missing save screenshot
- Menu dialog sometimes cut off with big fonts (now can scroll)
- Games in subfolders now shown in the launcher (still no sub navigation but everything will be listed at least)



# Retro-Go 1.25.1  (2021-04-28)
- Fixed: dialog multiline text mangling one character
- Fixed: font selection not wrapping around correctly
- Fixed: occasional crash when saving state (it might still crash but the save will at least finish first)


# Retro-Go 1.25    (2021-04-11)
- All: Proportional fonts support with correct alignment
- All: Crash log is now saved to the SD Card if possible. Please share it if you encounter a non-obvious crash.
- All: The settings format has changed to allow more flexibility (some of your settings will be lost, sorry!)
- GG/COL/SMS: Now have independent settings, you can set different filters and scaling per console!
- Launcher: Idle CRC32 caching, just let your GO sit for a while to never see "CRC32..." again!
- Launcher: Improved responsiveness


# Retro-Go 1.24    (2021-02-26)
- NES: Implemented Mapper 19 IRQ counter (fixes Pacman Championship Edition (still sound issues))
- NES: Significant performance improvement in some games
- NEX: Fixed Ninja Gaiden 3
- PCE: Fixed a rare crash when saving state on large games
- PCE: Added option to force 8bit audio
- GBC: Emulation is now running closer to real speed (60fps instead of 59, real is 59.7)
- GBC: Fixed Kirby Dreamland 2
- GBC: Fixed Fatass
- GBC: Fixed the harsh flicker when starting some games
- GBC: Hopefully improved how SRAM is handled
- SNES: Save states now working
- SNES: Fixed wrong colours in some games
- SNES: Choice of controls style
- SNES: 24Mbit ROMs support
- SNES: Performance improvements (still very slow)
- ALL: Fixed dialog sometimes crashing when pressing the wrong button
- ALL: Show emulation speed in status bar instead of just FPS
- ALL: Added soft reset option
- ALL: Enabled exFAT support
- ALL: Debug menu is now easier to access (currently pretty useless): press start when main menu is open
- ALL: Option to use LED to show disk activity (also fairly useless)

Note: You should backup your GBC saves before trying this build.

(See [1.19](https://github.com/ducalex/retro-go/releases/tag/1.19) to download cover art.)


# Retro-Go 1.23    (2021-01-19)
- GBC: Fixed palette and rtc not always reset correctly
- GBC: Fixed uninitialized values when loading/saving states (fixes minor glitches)
- NES: Compatibility improvement (fixes Dragon Warrior IV)
- SMS and GBC and NES: Small performance improvements and misc. bug fixes
- SNES: First preview! (Please don't report bugs, it is not playable yet)


# Retro-Go 1.22    (2020-12-31)
- GBC: Significant performance improvement (greatly improves Mario Tennis, Super Mario Land DX, Radikal Bikers, and more)
- GB:   Fixed Link's Awakening glitch
- PCE: Small performance improvement


# Retro-Go 1.21.1  (2020-12-26)
- PCE: Improved emulation accuracy and performance
- PCE: DDA emulation now mostly works (sound effects, voices)
- PCE: Option to crop overscan
- NES: Small performance improvement
- Launcher: A screenshot of your saved game will be shown if no cover found (configurable, not available in PCE yet)
- Tuned the auto frame skipping to reduce stutter in Pokemon


# Retro-Go 1.20.2  (2020-11-02)
- PCE: Fixed missing enemy sprites in Bonk
- PCE: Audio should be better in games like Bonk
- GBC: SRAM always generated when saving state and file now compatible with VisualBoyAdvance


# Retro-Go 1.20.1  (2020-11-01)
- PCE: Workaround for glitch in Splatterhouse (CRC D00CA74F)
- PCE: Fixed cropping offset for high res games
- PCE: Greatly improved performance for all games


# Retro-Go 1.19    (2020-10-01)
- LYNX: Rotation support
- LYNX: Added cover art
- ALL: New "medium" font size
- ALL: Slightly faster boot

Also some internal changes:
- SMS: Refactored the z80 emulation for more performance and smaller code size
- GBC: Misc cleanup for smaller code size
- ALL: Python build scripts, mkfw and gcc are no longer necessary
- ALL: Implemented a basic profiler based on GCC function instrumentation


# Retro-Go 1.18    (2020-08-24)
- RETRO: Favourites support (let me know if it can be improved)
- RETRO: Fixed misc navigation bugs
- NES: New MMC1 implementation to improve compatibility (eg: Dragon Warrior 4 now boots but crashes for other reason)
- NES: Battletoads "fixed" (workaround. still glitches, but it seems playable)
- GBC: Fixed garbled tiles in some games (eg: Speedy Gonzalez)
- PCE: Fixed horizontal lines in some games
- SMS/GG: Improved performance, 25% less frameskip
- ALL: Fixed stutter when changing a setting (on slow SD Card)
- ALL: Better panic handling that might show the crash reason now


# 2020-08-01 
- NES: Compatibility hack to fix glitches in Zen Intergalactic, Power Blade, Kirby, SMB3
- NES: Fixed volume issues above level 5
- Launcher: Misc UI improvements
- Settings are now stored on the SD card
- Retro-Go is now compiled with esp-idf 4.0


# 2020-06-29 
- Added file property dialog (press B)
- Minor UI tweaks
- Fixed crash on some devices when the battery is full or charging
- Fixed stuttering in GB/GBC emulation
- Fixed most scaling issues (some games still have horizontal bars in PCE)


# 2020-06-13 
- NES: Fixed palette not applied
- NES: Fixed some bugs with auto crop
- PCE: Fixed US encoded roms
- PCE: Fixed Legend of Hero Tonma
- PCE: Fixed scaling bug causing black screen in many games (eg Salamander)
- PCE: Implemented Street Fighter 2 mapper
- Lynx: Lynx is now playable (rotation support still missing)


# 2020-05-30 
- NES: New option to detect and crop the blank leftmost column in some games (eg SMB3)
- NES: Implemented Mapper 193 (War in the Gulf)
- NES: Fixed shadows in Aussie Rules Footy
- NES: Fixed games relying on 32K banks
- NES: Fixed more Chinese bootlegs
- NES: Improved sound on PAL games (speed is still a bit off in some games)
- NES: New save/load code that will soon be compatible with saves from popular emulators
- GB:  Fixed Donkey Kong statistics not showing (Game-specific hack)
- GBC: Fixed Fushigi no Dungeon - Fuurai no Shiren GB2 text not showing (Game-specific hack)
- PCE: Cover art improved
- All: Saves are now atomic, an interruption while saving (crash/battery/sd error/etc) will no longer corrupt your save file
- All: Battery and FPS indicators when you open in-game menu
- All: Option to select startup app (always launcher vs last used game)
- All: Firmware size is back to a mere 2.56MB (including PCE and Lynx!)


# 2020-05-04 
- PCE: Improved performance, all tested games run at 100% speed with higher refresh rate
- PCE: Save states fully implemented
- NES: Enabled support for mappers 41, 42, 46, 50, 73, 93, 160, 229 (bootlegs/multicarts)
- NES: Implemented mappers 162 and 163 (A dozen of Nanjing games)
- Smaller firmware size (3.5 to 3MB)


# 2020-04-27 
- PC Engine emulation!
- Colecovision no longer needs a BIOS
- Smaller binary sizes (though the .fw is padded to the same as before)
- License is now GPL2 for all components in the project
- NES: Option to disable overscan
- GB: DMG GREEN palette by 8xpdh
- GB: GBC's game-specific colorization support
- GB: Option to save/load SRAM automatically (careful: causes stuttering in many games)

Known issues:
- Save states support in PCE is experimental. Loading is also manual only (start a game -> Reload).


# 2020-03-20 
- NES: PAL games support
- NES: Fixed Cheetahmen 2 (Impl. Mapper 228)
- GB: Fixed Altered Space 3-D Adventure
- GB: Fixed Faceball 2000
- GB: Performance improvement


# 2020-03-13 
- NES: Color palette support (FirebrandX)
- NES: Option to disable sprite limit (sprite flicker)
- NES: Fixed SMB3 (Broken in 2020-03-10)
- NES: Fixed The Simpsons (Broken in 2020-03-10)
- NES: Fixed Owlia (Broken in 2020-02-29-exp)
- NES: Fixed Castlevania III (It's playable but there are issues) (Never worked before)
- NES: Fixed Choplifter (Never worked before)
- All: Fixed filter alignment issue (mostly visible in GB)


# 2020-03-10 
- New: Bilinear filtering available for all emulators
- Launcher: Fix SPI conflicts that froze the application
- NES: Fix crash during save/load in some games
- GBC: Fixed Worms Armageddon
- GBC: Fixed Metal Gear Solid
- NES: Fixed Teenage Mutant Ninja Turtle III
- NES: Fixed Felix the Cat
- NES: Fixed Tetris 2 + Bombliss
- Misc bug fixes


# 2020-02-23 
- Display code optimization with 5-10% FPS throughput increase
- Friendlier in-game fatal errors that allow to return to launcher
- Many bug fixes


# 2020-02-19
- GB/GBC: More of the ROM is preloaded to fix stuttering in some games
- GB/GBC: Now using partial screen updates, allowing 60fps in many games
- Turbo speed support in all emulators
- Removed interlacing. The replacement method looks better (or at least equal) and works for all three systems
- Launcher: File extensions are now hidden


# 2020-02-11 
- New scaling options
- 8MB GBC game support
- Significant size reduction
- Bug fixes


# 2020-02-05 
- Fixed Game Gear aspect ratio
- PNG support for cover art
- Option to reduce cover art loading delay
- Experimental ZIP support for NES/SMS/GG ROMS

 
# 2020-02-01 
- Initial release
