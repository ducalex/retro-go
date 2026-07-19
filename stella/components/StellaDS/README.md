# StellaDS - PHOENIX EDITION
StellaDS - An Atari 2600 VCS Emulator for the DS/DSi

To use this emulator, you must use compatible roms with a26/bin format.

If you want to launch the games directly via Twilight Menu++, you will need the file extension to be .a26 (you can just rename .bin). If you are just launching the StellaDS.nds emulator directly, you can load either .bin or .a26 extensions fine.

_**Have you played Atari today?!**_

![StellaDS](https://github.com/wavemotion-dave/StellaDS/blob/master/arm9/gfx/bgTop.png)

Features :
-----------------------
A solid level of compatibility to make games playable. Most games you remember fondly run full speed on a DSi in native 134MHz mode. For the older DS-LITE and DS-PHAT, many of the simple 8k, 4k and 2k games will play perfectly More sophisticated bank-switching schemes really require the DSi for the best experience.
 
Generally the internal database uses the NTSC No-Intro or ROMHUNTER roms (these are the good ones!). 

* Full console emulation including Reset, Select, Difficulty Switches, TV Type as touch-screen switches.
* Virtually all popular bankswitching is supported up to 512K of ROM and 32K of RAM (similar to the Harmony Encore specifications).
* Wide range of controllers including joystick, paddles, driving controller, booster grip, keypad, Genesis 2-button and more.
* Savekey Support with backing 32K EEPROM.
* DPC+ Arm-Assisted games supported! (DSi or above needed for full speed).
* CDF/CDFJ/CDFJ+ Arm-Assisted games supported! (DSi or above needed for full speed).
* FA2 and Cherity supported with backing EEPROM support.
* High score support with up to 10 scores for each game.
* Manuals included for more than 100 of the common games.
* Keypad overlay for Star Raiders.
* Frame Blending to help smooth out flicker and make the games shine.
* Ability to set Favorites on your game collection - press SELECT to mark a game as 'LIKE' or 'LOVE'.
* Save/Restore state for all game carts

Copyright :
-----------------------
StellaDS Phoenix-Edition is Copyright (c) 2020-2026 Dave Bernazzani (wavemotion-dave)

As long as there is no commercial use (i.e. no profit is made), copying and distribution of this emulator, its source code and associated readme files, with or without modification, are permitted in any medium without royalty provided this copyright notice is used and wavemotion-dave (Phoenix-Edition), Alekmaul (original port) and the Stella Team are thanked profusely.

Most of this code is based on the Stella project which has a GPL 2.0 license - please see their github page for details on the original (and vastly improved) codebase: https://github.com/stella-emu/stella 

The StellaDS emulator is offered as-is, without any warranty.

Where does this fit in with the mainline Stella? :
-----------------------
In general, it doesn't. StellaDS is based on an amalgamation of different Stella releases. Modern Stella is a work of art - and is generally cycle accurate and robust in so many
areas that closely mimic the real hardware honed over decades of hard work. That level of accuracy will not run fast enough on the venerable DS handheld. So StellaDS baseline is probably closest to mainline 
Stella 2.8.4 with some healthy fixes and enhancements taken liberally from Stella 3, 4, 5 and 6 to ensure as many games as possible will run correctly. The biggest difference here is that StellaDS is not cycle accurate and doesn't have the vastly improved TIA core of the later mainline Stella builds.
In addition, some optimizations had to be performed that will place this emulator a solid grade below the mainline Stella in terms of accuracy. The goal is to get reasonably close to accurate as possible while providing enough speed to make the games playable on the little handheld. Do not expect perfection - you have been warned! Any bugs, error and omissions found in StellaDS are completely and totally on me - be happy you didn't pay anything for it!

Known Issues :
-----------------------
* Any ARM-Assisted games (DPC+ or any flavor of CDF/CDFJ/+) are generally complex enough that you will need a DSi running at the 2X CPU speed (via Unlaunch or Twilight Menu++) to get a good experience from it. If you are running from an R4 or similar flashcart, you will be running in DS compatibility mode (67MHz) and the ARM games will not run anywhere near full speed.
* KC Munchkin Monster Maze does not run correctly with the optimized 'AR' cart handler in StellaDS.
* Games utilizing the TIA direct audio (games like Quadrun, the opening tune of Ms. Pac-Man, etc) and Fast Fetcher Music (Pitfall II, Stay Frosty 2, Mappy, Draconian, BOOM, etc) are handled with the new WAVE DIRECT audio driver but it's not perfect. Expect the sound to be passable but not great. The scratchiness you hear is a result of emulation and is not a reflection of the amazing music in these games that needs better emulation to make it shine (or real hardware).
* Graphical glitches on the following games due to imperfect TIA / timing emulation:  GI Joe Cobra Strike (minor graphical glitches), Pole Position (minor road glitch), Meltdown (left two columns not perfectly round), Tapper (counters extended slightly), Treasure Island (PAL game - sprites jump). Likely there are other games that will have minor artifacts.
* Game filenames can be no longer than 200 characters including the extension. Rename to shorter if needed.
* No more than 1500 files can be shown in a directory. You can break up your games into multiple directories if needed.

Strongest Recommendation:
-----------------------
If you're a fan of classic emulators on the DS/DSi, I urge you to get a DSi XL (USA) or LL (Japan) version. Or a 2DS/3DS if you can get homebrews to run on those. The reason is that the original DSi has a fast LCD fade and as such some of the games will be a little hard to see or appear to flicker due to the way the original Atari game is coded. For a real TV or a slow LCD fade, this looks much better. StellaDS includes an option to reduce the flicker - in configuration you can turn on Blending. This comes at a performance pentaly - for many games, the DSi can handle that penalty. For the ARM-Assisted games (DPC+, CDF/CDFJ/CDFJ+) that penalty will often be too great which is why the full gamut of Atari games will always render best on an XL/LL screen.

Installation:
-----------------------
Copy StellaDS.nds to your flash cart or SD card of choice. Most launchers will auto patch the game to run so you should be good to go... press on the CART slot to pick and load a game (.bin or .a26). For some older launchers or flash-carts you might need to DLDI patch the StellaDS.nds for your system (but it's unlikely as most launchers will auto-patch when you run the emulator).

Game Autodetection:
-----------------------
The Atari 2600 utilizes a growing number of bankswitching scheme. StellaDS 
currently supports 40 different bank switching schemes (see full list further below) - 
and it usually will auto-detect the right one. If for some reason it does not, you 
can go into the configuration (gear icon) and manually select a new scheme. Be 
sure to hit START to save your config and then try re-loading the game (the bank
switching requires a reload of the game). In the catastrophic event that the detected
bankswitching scheme causes the emulation to crash, you can try loading the
game using the Y (instead of A) button which will load the game but NOT start
it running... this should let you go in and adjust the back switching scheme
and other settings (PAL vs NTSC, etc).

Configuration:
-----------------------
The gear icon on the main screen allows you to set a number of configuration 
parameters related specifically to the game you are running. In addition to
the parameters shown, the difficulty switches (Left/Right A/B) are also saved
on a per-game basis.  If you change any configuration settings, you really 
should re-load the game after saving out your configuration - some settings
such as Bank Switching and NTSC vs PAL are only applied when the game is first
loaded.

Here is a description of the configuration items you can adjust. Be sure to 
press the START button to save out your configuration after making changes:

* Controller:      Most games use the Left-Joystick (with a Savekey in the right port) but you can change this.
* Bankswitch:      The system tries to auto-detect the correct bankswitching scheme but you can override that here if needed.
* Frame Blend:     Normal is fastest. Flicker free combines even/odd frames and it's pretty fast (10% hit to emulation speed which is no problem on most games) and effective at eliminating flicker. Background/Black mode is the slowest and will help with games that show background/black on one frame and sprites on the other (a simple blending will cause the sprites to disappear - Missile Command is one such game).
* TV Type:         NTSC vs PAL. Reload your game after saving this setting.
* Palette:         Three different palette types are supported. One optimized for the DS (default), the mainline Stella palette and the Z26 palette. You can also set the global palette for loading up all future games (page 2 of options).
* Sound:           Four different levels including the new WAVE DIRECT if you're using a game that does direct sound output (such as Quadrun, Stay Frosty 2, Mappy or Draconian). Most games should auto-detect this correctly. You can also set the global sound for loading up all future games (page 2 of options).
* ABXY Button:     Default to FIRE button but you can change this. You can now set to Screen Pan UP or Screen Pan DN to scroll the screen for more complex games. See Screen Settings below.
* HBLANK Zero:     Whether system clears pixels on horizontal blanks. Turn off at your own risk (it will speed up emulation which helps with older DS-LITE)
* VBLANK Zero:     Whether system clears pixels on vertical blanks. Turn off at your own risk (it will speed up emulation which helps with older DS-LITE)
* Analog Sens:     Default is 10 (1.0x speed). You can adjust how the paddle emulation responds.
* Start Scanline:  Starting scanline setting. When the TV first starts output of scanlines.
* Num Scanlines:   Number of scanlines to display. Don't touch if you don't understand this.
* Offset/Scale:    You can ajust the screen scaling and offset positions for the game. If you scale down, be aware that some pixel lines will not render - such is life with a very small 256x192 pixel DS screen.
* ARM THUMB:       SAFE and Optimized are roughly the same - Optimized is preferred as is slightly faster and recommended for ARM-assisted games. Optimized-No-Collisions is generally fine for most of the new CDF/CDFJ/CDFJ+ games that don't need TIA hardware collision detection. One final experimental setting is to enable some level of frameskip for the really hard-hitting newest ARM games (e.g. Elevator Agent).
* BUS Mode:        For the DSi and above, this will default to 'Accurate' and for the older DS hardware (or running in DS compatibility mode on an R4 cart) it will be set to 'Optimized' to gain speed. If you encounter a glitch with a game, try setting this to 'Accurate'.
* RAM Clear:       Normally set to 'Random' but you can force the Atari VCS RAM to all zeros. A few games might care - but most won't.

These options are spread across two (2) pages ... use the L/R shoulder buttons to switch pages. This gets you access to some global settings for sound quality and default color palette (after saving, new games loaded will use the global settings by default and you can tweak individual games as desired).


Screen Settings:
-----------------------
If you are only running Classic games - the default screen settings should be fine. But if you're running some of the newer homebrews created over the past decade, 
you will want to configure your screen properly.  For many games, you simply can't fit all the content on the 192 pixel lines of a DS/DSi. For these you
can either cut off the top, bottom or scale the screen. When these options aren't enough, you can set your Y offset so that some of the screen is cut-off top or bottom and then
you can map one of the DS buttons to 'SCREEN PAN UP' or 'SCREEN PAN DOWN' which will momentarily pan up or down. This works great for Champ Games offerings where the score
display is normally off-screen while you play and then you can tap a button to briefly see it.

PAL vs NTSC:
-----------------------
StellaDS supports PAL games but be warned... in the very early days of video
games, most games were developed as NTSC for the American Market and
only later ported to a PAL system for our European friends. Some game makers
did a good job (Imagic). Some did an OK job (Atari). And some did not do well
(Activision). That means that some games just don't run at the right speed 
for the PAL market - since Activision did not optimize for PAL, the games run
17% slower due to the frame rate difference. To make matters worse, the main
benefit of PAL is that the programmer has more scanlines to work with but the
DS only has 192 pixel rows... so with PAL we must compress the image and
that results in missing pixel rows. All of this to say: while PAL is supported
you are generally going to get the most genuine Atari 2600 experience with
NTSC roms. 

Bank Switching Supported:
-----------------------
<pre>
Name        ROM         RAM
2K          2K          --
CV          2K          1K
4K          4K          --
0840        8K          --
UA          8K          --
WD          8K          64B
F8          8K          --
F8SC        8K          128B
FE/SCABS    8K          --
wF8         8K          --
03E0        8K          --
0FA0        8K          --
AR          2K          6K
DPC         8K (+ 2K Display Data)  
DPC+        24K         8K (ARM assisted)
CDF/CDFJ    32K         8K (ARM assisted)
CDFJ+       256K        32K (ARM assisted)
E0          8K          --
FA          12K         256 bytes
E7          8K/12K/16K  1K + (4) 256 byte pages
F6          16K         -- 
F6SC        16K         128B
JANE        16K         -- (Tarzan Prototype)
CTY         28K         64B  (+256 EEPROM)
FA2         29K         256B (+256 EEPROM)
F4          32K         --
F4SC        32K         128B
EF          64K         --
EFSC        64K         128B
F0/MB       64K         --
X07         64K         --
DF          128K        --
DFSC        128K        128B
SB          128K-512K   --
BF          256K        --
BFSC        256K        128B
TV Boy      256K        --
3F          512K        --
3E          512K        32K
3E+         64K         32K
</pre>

Backing Files Used by StellaDS:
-----------------------
In the /data directory of your SD card you will see some files auto-created by StellaDS. You are free to delete or back these files up as you see fit. If you delete a file, it will be re-created 'clean' on the next boot of the emulator.

* StellaDS.DAT - this contains the per-game settings (basically when you press START on the Configuration screen and it saves out your settings for the current game). The easiest way to set all your games back to default settings is to remove this file.
* StellaDS.HI - this is the high-score file on a per-game basis. Up to 10 scores are saved per game by pressing the little Golden Chalice icon on the main screen.
* StellaDS.EE - this is the SaveKey 32K EEPROM file for games that utilize a SaveKey. If you remove it, a new clean blank copy will be created.
* StellaDS.FAV - this is the 4K favorites file where up to 1024 favorites (like/love - using the SELECT key on the file menu) are stored.
* Various .SAV files - these are the Save State / Restore files (one per game with the same base name as the game ROM).
 
Champ Games Support :
-----------------------
![Champ Games](https://github.com/wavemotion-dave/StellaDS/blob/master/png/champ.png)

StellaDS supports all current Champ Games offerings through Turbo Arcade and Elevator Agent. These games represent the cutting edge of what is possible on the Atari 2600 using an ARM co-processor to help move data through the system. These games push the limits of the emulator. To run these advanced arm-assisted games, you will need a DSi or greater running either Twilight Menu++ or Unlaunch to unlock the 2X CPU (134MHz). The original DS (or the DSi/2DS/3DS running with an R4 cart) will only run in DS compatibility mode (67MHz) which is too slow to render these games full speed.

It is also recommended that you use an XL/LL model - these larger screens have a slower LCD fade resulting in a picture that more closely resembles an old TV. This means less flicker overall.

Be aware that most of these games utilize a significant amount of overscan. The DS/DSi/XL/LL only has 192 pixels of vertical resolution and this means the extra 16-20 pixels of game information will render-off screen. StellaDS will always show the main gameplay portion of the screen and utilizes a screen pan UP/DN to shift in the score or status (non-gameplay) information. For example, in GORF, you will not see the score at the top normally... but if you press the X button (pan UP), the screen will momentarily shift up to show you the score and then automatically pan back down. It's quite smooth and is very serviceable in normal gameplay.

![Gorf](https://github.com/wavemotion-dave/StellaDS/blob/master/png/gorf.bmp)
![Gorf](https://github.com/wavemotion-dave/StellaDS/blob/master/png/gorf-shift.bmp)

All Champ Games should run at or near full speed with the exception of the two most complex games: Turbo Arcade (which dips framerate slightly for some complex rendering scenes but is generally not noticeable for gameplay) and Elevator Agent which runs at 55+ (often up to full speed) but is variable enough that you will hear some slight wavering in the music output (but still quite playable and enjoyable). Try the DEMO roms first to be sure you're happy with the emulation quality here. Be aware that any performance issues related to these cutting-edge games is a reflection of my emulation and not the superior craftsmanship of the games themselves. You can see a full set of Champ Games offerings at https://champ.games

How to use StellaDS :
---------------------
Put the a26/bin files where you want on your SD or flashcard. 

Place StellaDS.NDS on your SD or flashcard.

That's all, StellaDS can be used now :) !

When the emulator starts, click on the cartridge slot to choose a file. you are use Up/Down 
to select a file, then use A to load it.

Controls :
 * D-Pad   : the Joystick
 * A,B,X,Y : Fire button (configurable)
 * SELECT  : SELECT switch
 * START   : START switch
 * R-Trig + D-Pad  : Shift display offset in the D-PAD direction
 * L-Trig + D-Pad  : Change Scaling of the Y-Screen (UP/DOWN scaling only)
 * L-Trig + R-Trig + A:  Swap LCD top/bottom. 
 * Hold L+R+Y shoulder buttons for ~1 second to take a snapshot of the screen (written to SD card as a .BMP file)
 * Use stylus on buttons for other icon-based actions on bottom screen.
 
Compile Instructions :
-----------------------
I'm using the following:
* devkitpro-pacman version 6.0.1-2
* gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
* libnds 1.8.2-1

I use Xubuntu (22.04.3 LTS) and the Pacman repositories (devkitpro-pacman version 6.0.1-2).  I'm told it should also build under 
Windows but I've never done it and don't know how.

If you try to build on a newer gcc, you will likely find it bloats the code a bit and you'll run out of  memory.
If this happens, first try pulling some of the  declarations in areas that aren't heavily utilized.

Thanks and Credits :
-----------------------
* To Bradford W. Mott and Stephen Anthony and various contributors for Stella (http://stella.sourceforge.net/)
* To Robz for Twilight-Menu++ which rekindled emulation on the classic handheld (https://github.com/DS-Homebrew/TWiLightMenu)
* To Wintermute for devkitpro and libnds (http://www.devkitpro.org)
* To Alekmaul and The Chuckster for porting Stella to the DS.
* To John Champeau for his support in helping me optimize the emualtor for his advanced arm-assisted games.

Version History:
-----------------------
8.1b : 25-Jan-2026 by Dave Bernazzani (wavemotion)
  * New NTSC (60Hz) and PAL (50Hz) 'True-Sync' to reduce tearing and other video artifacts from screen rendering on the DS/DSi.

8.1 : 14-Dec-2025 by Dave Bernazzani (wavemotion)
  * Added new Favorites system so you can LIKE/LOVE any game - press SELECT on the Game ROM loading screen.
  * TIA optimizations for another frame of improved performance.
  * CPU optimizations for another frame of improved performance.
  * Thumbulator optimizations for the LDR3 instruction to help some CDFJ games.
  * Most VBLANK and HBLANK optimizations now removed in favor of improved accuracy (improved performance allows us to ditch most of this).
  * Refactor memory to buy back almost 250K of DS RAM for future expansion.

8.0 : 09-Dec-2024 by Dave Bernazzani (wavemotion)
  * Added music fetchers for more complete  Chetiry (CTY) emulation. Save state works with CTY. Use the 60K Chetiry rom for music.
  * Fixed holding RESET (DS Start button) and SELECT (DS Select Button) when loading a game. This allows you to do things like the Double-Shot trick in Space Invaders.

V7.9 : 04-Dec-2024 by Dave Bernazzani (wavemotion)
  * Fix so holding shoulder buttons on Bumper Bash does NOT invoke the screen snapshot.
  * Fix for bumper bash right bumper position so it now shows correctly.
  * Improved GI Joe Cobra Strike snake graphics so it's much closer to the way a real TIA chip would draw it.
  * Fix for Labyrinth (AR) graphical glitches at top/bottom of walls.
  * Improved difficulty switch handling so startup values for games like Asteroids works correctly.
  * Improved RIOT timer emulation - Acid Drop, Panda Chase, Zoo Fun and a few other PAL roms now work.
  * Improved keyboard/keypad emulation so games like Alpha Beam with Ernie, Oscar's Trash Can Race and Magicard now work.
  * Improved emulation of Starpath Supercharger (AR) games (Dragonstomper is random) and support smaller game loads (Cubis now runs).
  * Fixed Casino paddles (correct paddle emulation so you can select all card positions in game variation 4).
  * Improved paddle emulation - you can now press and hold Y to slow the paddle movement down or X to speed it up.
  * Internal cart database updated to Romhunter v19 (full NTSC with select PAL titles).
  * Fix for save/load states on CDFJ+ carts (Gorf and Turbo Arcade will now save/load correctly).
  * Wave Direct improvements - applying the same logic for all games that utilize digital speech and improving sounds.
  * Added new Stretch-X config option to stretch the screen horizontally (you can use offset/stretch to fill the screen for
    some Activision games which purposely hide the HMOVE blanks on the left side of the screen).
  * Pressing and holding the Game Select and Game Reset buttons now keeps them depressed until you let up your finger.

V7.8 : 30-Nov-2024 by Dave Bernazzani (wavemotion)
  * Further tweaks to the improved 'Wave Direct' to prevent audio gaps.
  * First pass at allowing 'Wave Direct' for DS-Phat/Lite (only for simple games like Quadrun, Berzerk VE and Open Sesame).
  * Tweaks to the DPC audio driver for improved Pitfall II music.
  * Added support for some of the Voice Enhanced Berzerk hacks such as DrVsDaleks.
  * Improved Meltdown emulation - added hack for NUSIZ0/1 to improve screen rendering. Not perfect but closer...
  * Fixed Pole Position speedometer zero so it shows correctly and other minor graphical fixes (HMOVE timing)
  * Fixed Grand Prix graphical glitch on rocks/scenery (HMOVE timing).
  * Fixed Moon Patrol graphical glitch on right side of screen (HMOVE timing).
  * Fixed Double Dragon graphical glitch on strength meter (HMOVE timing).
  * Apply RSYNC improved TIA handling across the board (was only being applied to select games).
  
V7.7 : 24-Nov-2024 by Dave Bernazzani (wavemotion)
  * Improved 'Wave Direct' sound to help with digitized speech effects of Quadrun, Berzerk VE and Open Sesame.
  * Tweaked handling of the new Champ Games - Tuthankham.

V7.6 : 25-Aug-2024 by Dave Bernazzani (wavemotion)
  * Fixed config setting of RAM randomized vs clear.
  * Fixed AR (Starpath/Supercharger) games to prevent memory overwrite.
  * Big improvement to Starpath Supercharger (AR) cart rendering for 3-7 frames of improvement.
  * Numerous comment cleanups and other minor refactors to improve the codebase.

V7.5 : 23-Aug-2024 by Dave Bernazzani (wavemotion)
  * Overhaul and refactor of cart/device handling to gain a 3-4% speed boost across the board.
  * Improved memory handling to use a bit less of the precious DS resources.
  * Fixed multi-load Starpath Supercharger (AR) carts so they work again (broken for almost a year!)
  * Cleanup as time permitted

V7.4 : 19-Aug-2024 by Dave Bernazzani (wavemotion)
  * Adding save/restore state handling for all games. Use the new down-arrow icon in the lower left.
  * Added 03E0 banking for Parker Bros. Brazilian carts.
  * Added 0FA0 banking for Fotomania Brazilian carts.

V7.3 : 05-Jun-2024 by Dave Bernazzani (wavemotion)
  * Added wF8 banking scheme for the new dump of Smurf and Zaxxon.
  * Added JANE banking scheme for the new Tarzan prototype release.
  * Added support for Champ Games Tutankham Arcade.

V7.2 : 01-Jan-2024 by Dave Bernazzani (wavemotion)
  * Updated copyright as we cross into the new year!
  * Another partial frame of performance on CDFJ/+ games.
  * Cleanup this readme file to reflect latest changes in the emulator.

V7.1 : 23-Dec-2023 by Dave Bernazzani (wavemotion)
  * Squeezed out one extra frame of performance on CDFJ/+ games. 
  * New light frameskip applied to Draconian for a performance boost.
  * Minor fixes and tweaks to make all the latest homebrew games look and play their best.

V7.0 : 10-Dec-2023 by Dave Bernazzani (wavemotion)
  * Champ Games Edition! Major overhaul to the CDFJ/+ driver to squeeze out every bit of performance.
  * Fix for Genesis gamepad emulation (detection now works).
  * Other minor cleanups and tweaks as time permitted.

V6.9 : 06-Dec-2023 by Dave Bernazzani (wavemotion)
  * Improved the ARM Thumb driver and the CDFJ driver to squeeze out two more frames of performance.
  * Fixed PAN UP / PAN DOWN handling on the A-button and you can now pan even if another button is pressed (useful for Turbo Arcade).
  * Minor cleanups as time permitted.

V6.8 : 01-Jul-2023 by Dave Bernazzani (wavemotion)
  * Improved RSYNC so Extra Terrestrials (1984 by Skill Screen Games) works.
  * Improved keypad handling so Magicard and similar keypad/keyboard game work.
  * Added a few of the ultra-rare games discovered in the last decade to the internal database.
  * Updated internal database to ROMHUNTER v18 for the classic-era games.
  * Starpath Supercharger (AR) carts now clear RAM by default (prevent lock-ups).
  * Added Dual Keypad/Keyboards for the few game that need them (BASIC, Magicard mostly).

V6.7 : 24-Jun-2023 by Dave Bernazzani (wavemotion)
  * Fix for Sword of Surtr so it runs correctly.
  * Hold L+R shoulder buttons for ~1 second to take a snapshot of the screen (written to SD card)
  * DSi now defaults to the 'Accurate' BUS MODE for maximum compatibility.
  * Improved data bus handling for undriven pins in Tia::Peek() for improved compatibility.
  * A few more tweaks to a few more games to make them as accurate as possible.
  
V6.6 : 20-Jun-2023 by Dave Bernazzani (wavemotion)
  * Fix for Meltdown prototype so it doesn't crash.
  * Fix for Pleiades to fix graphical glitches.
  * Fix for Atom Smasher prototype so it doesn't crash on start (wrong bank scheme detected).
  * Fix for E7 banking so it handles 8K, 12K and 16K roms.
  * Fix for Flash Gordon to eliminate graphical glitches.
  * Fix for Elf Adventure prototype so it runs.
  * Fix for Star Gunner so it doesn't glitch.
  * Fix for Warlords graphical glitches.
  * Fix for Worm War I graphical glitches.
  * Fix for A-Star not starting.
  * Fix for Hugo Hunt graphical glitches.
  * Improved random() generator for more robust RAM clear / handling on startup and added config to either randomize RAM or clear it at start.
  * Added new option to use the 'Compatible' BUS driver which will handles things like invalid reads and drives unused TIA bits (a few games rely on this - but it does slow down emulation slightly).
  
V6.5 : 21-Dec-2022 by Dave Bernazzani (wavemotion)
  * Polished release - a few more tweaks, a few more optimizations and everything is running as fast science allows on the DS/DSi.
  * Removed "ghost read" and "ghost writes" on the 6502 emulation for a bit more speed.
  * Use of gcc "likely/unlikely" in a few key spots to help the compiler optimize.

V6.4 : 16-Dec-2022 by Dave Bernazzani (wavemotion)
  * Reduced stack memory so we don't crash when first creating a Savekey EE file.
  * Improved 6502 handling to localize the PC for a bit of a performance boost.
  * Added the 3E+ banking scheme.
  * New global palette and sound options on the Configuration Menu page 2 (use L/R keys).
  * Other small improvements as time permitted.

V6.3 : 11-Dec-2022 by Dave Bernazzani (wavemotion)
  * Improved performance across the board. More games play at the right speed even on the older DS-Lite/Phat.
  * New QuadTari support - for now it's just Dual Joysticks + SaveKey which is useful for games like Robotwar.
  
V6.2 : 05-Dec-2022 by Dave Bernazzani (wavemotion)
  * New WAVE DIRECT sound handling for fast-fetching music. Draconian, Mappy, Stay Frosty 2, Stella's Stocking, Pitfall II and Quadrun have much improved sound. 
  * New palette options to tweak the colors to your liking.
  * Memory re-org for a bit more speed but also to recover some valuable resources so more features can be added in the future.
  * Added a 2nd page (not yet populated) for possible future options. This requires a new configuration file format - your old one will be reset. Sorry!
  * Fixed CDFJ+ fetchers causing problems (including incorrect handling of SaveKey).
  * Other small improvements as time permitted.
  
V6.1 : 01-Dec-2022 by Dave Bernazzani (wavemotion)
  * CDFJ+ games are now supported up to 256K of ROM and 32K of RAM. Turbo Arcade is playable but isn't yet full speed.
  * More speedup in ARM Thumb processing.
  * New Screen Pan Up and Pan Down handling - this can be mapped to any DS button (XYAB) to help with games that use more than 192 pixel lines (really useful for CDFJ games from Champ Games).

V6.0 : 27-Nov-2022 by Dave Bernazzani (wavemotion)
  * CDF/CDFJ games are now supported (but not CDFJ+)
  * Big speedup in ARM Thumb processing to render all DPC+ games full-speed on the DSi or above.

V5.9 : 21-Nov-2022 by Dave Bernazzani (wavemotion)
  * Minor fixes for some games to render them more accurately including the new Chaotic Grill homebrew.
  * Improved ARM Thumbulator for another frame of performance.
  * Minor cleanups and optimizations across the board.
  
V5.8 : 21-Oct-2022 by Dave Bernazzani (wavemotion)
  * Fixed colors in Medieval Mayhem (and a few other games).
  * Fixed stars to show properly in Stay Frosty and Rabbit Transit.
  * Streamlined DPC+ so Space Rocks and Stay Frosty 2 are nearly full speed.
  * Minor cleanups and optmizations across the board.
  
V5.7 : 18-Oct-2022 by Dave Bernazzani (wavemotion)
  * Fixed Sword of Surtr so it plays properly.
  * Added TWIN STICK controller so games like Rail Slider are playable!
  * More DPC+ optmizations... Space Rocks is almost full speed!

V5.6 : 14-Oct-2022 by Dave Bernazzani (wavemotion)
  * Massive speedup improvement for DPC+ games. Most play full speed on the DSi.
  * Fixed graphical glitches on Space Rocks homebrew.
  * Other cleanups and optimizations across the board to shine things up.
  
V5.5 : 12-Oct-2022 by Dave Bernazzani (wavemotion)
  * Added DPC+ with ARM Thumbulator to provide some preliminary support for ARM-assisted games.
  * Improved execution speed of the emulator and improved the frame blending algorithm.

V5.4 : 07-Oct-2022 by Dave Bernazzani (wavemotion)
  * Added DF bankswitching scheme (missed when DFSC was added).
  * New core execution loop that gives us another 2 frames of performance.
  * Optimized F4 driver to make some of the 32K games playable on DS-Lite.
  * Cleanup across the board.
  
V5.3 : 03-Oct-2022 by Dave Bernazzani (wavemotion)
  * Added Chetiry (CTY) bank switching scheme with EEPROM support (but no music fetchers).
  * Improved memory handling across the board for another frame of performance.
  * Fixed SP+ so it runs properly again.
  * Improved driver for Midnight Magic and Realsports Tennis so both now run at 60FPS!

V5.2 : 30-Sep-2022 by Dave Bernazzani (wavemotion)
  * Fixed goof in F6, F4 and related F6SC and F4SC drivers! 
  * Another frame of increased performance
  * More accurate AR cart handling for Supercharger games

V5.1 : 29-Sep-2022 by Dave Bernazzani (wavemotion)
  * Improved all bankswitching for more speed and greater compatibility.
  * New F6 driver for DS-LITE to support 16K games - many at full speed!
  * Cleanups across the board.

V5.0 : 27-Sep-2022 by Dave Bernazzani (wavemotion)
  * Added new bankswitching scheme:  X07 (Stella's Stocking).
  * Improved 4K/2K driver to squeeze out a couple more frames of performance.
  * New special F8 driver to support more 8K games on the DS-Lite.
  * Lots of cleanup across the board in the database to make more games playable.

V4.9 : 18-Sep-2022 by Dave Bernazzani (wavemotion)
  * Added new bankswitching schemes:  TVBOY, UASW and 0840 (Econobanking)
  * Fixed UA bankswitching scheme to allow more games to run

V4.8 : 11-Sep-2022 by Dave Bernazzani (wavemotion)
  * Fixed Super Bank (SB) scheme.

V4.7 : 24-Mar-2022 by Dave Bernazzani (wavemotion)
  * Added per-game configuration settings.
  * Fixed 3E/3F and UA bankswitching schemes.

V4.6 : 20-Mar-2022 by Dave Bernazzani (wavemotion)
  * Added SaveKey support - standard games will have a virtual SaveKey in the right controller 
    jack and will be auto-backed to a 32K /data/StellaDS.EE file.
  * Added EF, EFSC, DFSC and SB (SuperBank) support for games as large as 256K!
  
V4.5 : 05-Nov-2021 by Dave Bernazzani (wavemotion)
  * New sound output core - no more zingers!

V4.4 : 22-Aug-2021 by Dave Bernazzani (wavemotion)
  * Minor cleanups across the board. Improved AR cart speed by ~1FPS.
  * Added horizontal stretch - use the Left Shoulder + L/R D-Pad.
  * The Y button is now auto-fire.

V4.3 : 09-Aug-2021 by Dave Bernazzani (wavemotion)
  * Massive overhaul of the non-bank-switched engine so now the older DS-Lite
    and DS-Phat will run many games at full speed.

V4.2 : 02-Aug-2021 by Dave Bernazzani (wavemotion)
  * New overlay graphic for Star Raiders (plus manual)
  * A half dozen odd games got their controllers straightened out
  * About 100 graphical tweaks for various games to make them look as good as possible.

V4.1 : 28-July-2021 by Dave Bernazzani (wavemotion)
  * Found another 1-2% speed up in TIA rendering!
  * Bumper Bash now plays correctly (you can use the shoulder buttons too).
  * Kool-Aid-Man now plays correctly 
  * Meltdown graphics improvements (but not fixed... but playable).
  * Alpha Beam, Big Bird and Cookie Monster games all work with joystick controls now.

V4.0 : 24-July-2021 by Dave Bernazzani (wavemotion)
  * Found another 3-5% speed up in TIA rendering! Official Frogger now at 60FPS.
  * Press and hold L+R+A to swap LCD screens.
  * Other minor cleanups and polish.
  
V3.9 : 22-July-2021 by Dave Bernazzani (wavemotion)
  * Finally fixed graphical glitches in Rabbit Transit and Dragonstomper.
  * Many cleanups and improvements to the TIA handling.
  * Added UA and FE bankswitching detection schemes.
  * Another 100 games added to the compatibility database.
  
V3.8 : 18-July-2021 by Dave Bernazzani (wavemotion)
  * Massive update to database to support 50+ of the best Atari Hacks.
  * Support for WD bankswitching - Pursuit of the Pink Panther playable.
  * Genesis Controller support for all the awesome games that take advantage of it.
  * 120 Game Manuals!!

V3.7 : 12-July-2021 by Dave Bernazzani (wavemotion)
  * 100 Game Manuals!!
  * Fix for Thwocker prototypes.
  * Minor screen tweaks to tighten up some displays.
  * New info graphic.

V3.6 : 10-July-2021 by Dave Bernazzani (wavemotion)
  * PAL Support!  PAL games will render in proper 50Hz / 312 Scanlines.
  * Database of known games expanded from 850 to 1900.
  * Long rom filenames now scroll when selecting a game.
  * Total instruction manuals: 80!

V3.5 : 5-July-2021 by Dave Bernazzani (wavemotion)
  * Sound on Quadrun improved.
  * Total instruction manuals: 75!

V3.4 : 28-Jun-2021 by Dave Bernazzani (wavemotion)
  * Added new flicker reduction algorithms to help with games
    like Frogger, Asteroids, Astroblast, Pac-Man, etc.
  * Total instruction manuals: 50!

V3.3 : 26-Jun-2021 by Dave Bernazzani (wavemotion)
  * Added [TIME], [LOW] and [ALPHA] as high-score options.
  * Fixed small graphical glitches in F8, F6 and F4 games.
  * Another 5 instruction manuals added.

V3.2 : 24-Jun-2021 by Dave Bernazzani (wavemotion)
  * Vastly improved high score internals to support future enhancements.
  * Cleanup of paddle handling - games like Tac-Scan now work.

V3.1 : 22-Jun-2021 by Dave Bernazzani (wavemotion)
  * Up to 30 instruction manuals added.
  * Fix for difficulty switches (they reset on new game)
  * Fix for High Score initials so it recalls last known entry.

V3.0 : 21-Jun-2021 by Dave Bernazzani (wavemotion)
  * Reworked the page access and bank switching to eek out another 2-3% speed!
  * Cleanup for the CPU Core
  * More instruction manuals added (about 20 games total now)

V2.9 : 19-Jun-2021 by Dave Bernazzani (wavemotion)
  * High Score Support!
  * Preliminary Instruction Manual support (only a dozen games so far)

V2.8 : 11-Jun-2021 by Dave Bernazzani (wavemotion)
  * A couple dozen micro-tweaks to the sound driver to squeeze out a bit more
    performance. Elevators Amiss is now running at 60FPS! All of the Starpath
    Supercharger games are running in the high 50s and many are at 60FPS.

V2.7 : 4-Jun-2021 by Dave Bernazzani (wavemotion)
  * A few more games added to the compatibility list.
  * A minor optimization for fetching PC contents - yielding 1 or 2FPS in many games.

V2.6 : 3-Jun-2021 by Dave Bernazzani (wavemotion)
  * Fixed ability to scale/move screen with Paddle games.
  * Tweaked more than 100 game screen offsets to better center/show them by default.
  * Minor cleanups to the code and memory.

V2.5 : 1-Jun-2021 by Dave Bernazzani (wavemotion)
  * Restored PAL palette selection when choosing a game.
  * Eeked out 1 or 2 FPS on Starpath Supercharger games by limiting the number
    of vertical scanlines we will show. Elevators Amiss is now close to 54FPS!

V2.4 : 31-May-2021 by Dave Bernazzani (wavemotion)
  * A bit more juice squeezed out of the TIA. Added cart-specific options
    to bypass VerticalBlank zero (not all games need that memory cleared if
    we are dealing with a static screen... and this buys us CPU cycles!) and,
    somewhat more dangerously, HorizontalBlank clearing can be disabled for
    more speed. Only a few of the more stubborn games utilize these!
  * Removed PAL/NTSC option... only NTSC is supported (all games were released
    in NTSC except a dozen PAL exclusives which have long since been converted
    to run on NTSC - Search Atariage).
  * General cleanup and minor memory/code optimizations to get the most out 
    of the emulator.
  * Added 2 more lines of resolution before the top of the screen and 5 more
    below... this allows the games that utilize underscan and/or significant
    overscan to show properly.

V2.3 : 30-May-2021 by Dave Bernazzani (wavemotion)
  * And more improvement/stramline of TIA rendering to give a 10% boost
    for most games 0 this brings Elevators Amiss up to 51FPS and Phaser
    Patrol to 55 with gusts up to 59FPS!
  
V2.2 : 29-May-2021 by Dave Bernazzani (wavemotion)
  * Improved handling of TIA to trade off a little bit of speed on the fast
    games to gain 10-15% speed boost on the hard-to-render games. This makes
    games like Fantastic Voyage, Super Breakout, Elevators Amiss and many 
    other games run much closer to full speed!

V2.1 : 23-Mar-2021 by Dave Bernazzani (wavemotion)
  * Improved Screen Scaling (using Left/Right shoulder buttons + Arrow Keys)
  * Fix for file selection bug
  * Minor cleanup and improvements to squeeze out the last bit of juice from the emulation!

V2.0 : 26-Jan-2021 by Dave Bernazzani (wavemotion)
  * Faster AR processing rendering some of the more stubborn games playable.
  * Faster screen rendering for more complicated games. Simple games take a small
    performance hit (think: games with solid backgrounds) but those were already 
    running plenty fast enough - to we took the trade off to make some of the 
    more complicated games faster - this is an overall win for most games!
  * General cleanup - more games rendering properly (screen offset/scaling tweaks).

V1.9 : 24-Jan-2021 by Dave Bernazzani (wavemotion)
  * Screen tweaks to more than 400 games to make them look as good as possible.
  * More games added to the compatibility table - many homebrews.
  * Faster AR (Starpath Supercharger) processing so more of those games are playable.

V1.8 : 19-Jan-2021 by Dave Bernazzani (wavemotion)
  * Booster Grip Implemented (Omega Race and Thrust+ working)
  * Improved timing accuracy so games like Venture Reloaded work.
  * Implemented "AR" handling so Supercharger carts work.
  * Improved timing across the board... about 5% faster than 1.7
  * DPC+ Carts now recognized... and message will come up stating they are not supported.
  * L-TRIG+DPAD bumps screen offsets.   R-TRIG+DPAD changes Y-Scaling  

V1.7 : 31-Dec-2020 by Dave Bernazzani (wavemotion)
  * Very minor cleanup and polish.

V1.6 : 05-Dec-2020 by Dave Bernazzani (wavemotion)
  * Star Raiders working now.
  * All Tigervision (3F) games working.
  * Boulder Dash Demo V2 (3E game) works!
  * Minor graphical cleanups and better keypad touch input support.
  * Compatibility.txt list updated.

V1.5 : 05-Dec-2020 by Dave Bernazzani (wavemotion)
  * General cleanup. Ready for wider distribution.

V1.1k : 04-Dec-2020 by Dave Bernazzani (wavemotion)
  * Added Sound Toggle. Off gets you 10% speed boost.
  
V1.1j : 03-Dec-2020 by Dave Bernazzani (wavemotion)
  * Added info icon.
  * Better handling of touch screen.
  * Skip drawing frames that are off-screen.
  * Fixed sound gliches long present in the emulator.
  * Fixed color for Barnstorming bars (more red, less purple).
  * Other optimizations as time allowed.

V1.1h : 01-Dec-2020 by Dave Bernazzani (wavemotion)
  * Added full paddle emulation via new Paddle Icon
  * Added Keyboard / Keypad emulation via new Keypad Icon
  * Added Driver Controller emulation (for Indy500 only for now)
  * More core cleanups and minor timing improvements
  * Rebranding PHEONIX EDITION

V1.1g : 29-Nov-2020 by Dave Bernazzani (wavemotion)
  * Added paddle support - use the stylus on the bottom of screen.
  * New ROM mapping table for most acurate position of screen and will
    automatically enable the right controller and Flicker Free mode.
  * Better and more accurate timing (helps FPS measurement)
  * Switched to using unused VRAM to help with DMA copies.
  * Left/Right trigger to position screen now happens during Vertical Blank (cleaner).

V1.1f : 28-Nov-2020 by Dave Bernazzani (wavemotion)
  * Switched to dual-channel DMA memory copy for frame buffer. Slight speed improvement.
  * New button handling. See button map below the history section.
  * Added Flicker-Free switch... please use with caution as this takes significant CPU 
    power to make work as it ORs the previous two frames to help with 30Hz flicker that
    some games produce (Stellar Track, Yars Revenge, Asteroids, etc). 
  * Added ability to shift the screen up and down by 1 pixel per press of the shoulder buttons.

V1.1e : 27-Nov-2020 by Dave Bernazzani (wavemotion)
  * Slight improvement on TIA speed (1-2% speed improvement on many games).
  * Correctly identify games that use the right joystick (Home Run, Surround, Air-Sea Battle, 
    Wizard of Wor, Basketball, Star Ship and Slot Racers) and switch the DS controls
    to compensate (those games were unplayable before this).
  * Added Compatibility.txt for a list of games and their framerates.

V1.1d : 26-Nov-2020 by Dave Bernazzani (wavemotion)
  * Improved bank switching algorithm to speed it up. Makes F6/F8 games faster.
  * Improved speed in Pitfall II. Up to about 45 FPS - almost (but not quite) playable.
  * Faster display of ROMs list.
  * Emulator Version number now on ROM selection list.

V1.1c : 25-Nov-2020 by Dave Bernazzani (wavemotion)
  * Starting with RocketRobz 2-Apr-2020 baseline Stella DS codebase... 
  * Added graphical difficulty switches for both Left and Right players.
  * Used  a bit more of the DSi screen resolution so that the occasional missing graphic doesn't get 
    compressed away (e.g. if you fire the laser in Chopper Command you can see it sometimes disappears at 
    certain vertical positions of the Chopper... this is no longer an issue). Generally this results in a
    few missing rows of pixels at the very bottom of the screen which is a better trade-off for 95% of 
    games where almost no action takes place at the extremes.
  * Fixed all the on-screen touch button handling such that it's not so glitchy... press SELECT switch or
    START switch and it actually works every time (it's amazing what a little debounce code will do!). 
    I increased the hot-spots where you can press for each switch so that you can cleanly operate it with
    a finger or thumb... I hated getting out the Stylus for the small hit-boxes that were originally used.
  * Improved the emulator core to reorder a few operations, streamline a few others and one semi-major hack 
    that I'm not proud of to use a global integer for CPU cycles to avoid any structure/class overhead which
    yields a 5-15% speedup (depending on the game).  More games now run full speed and other games are elevated
    to "playable" with these speed improvements. 
  * Start and Select buttons now map to RESET (often used to start a game) and SELECT switches. That only seems logical. 
  * Remapped the FPS display to the right shoulder switch. I also only output the FPS once per second instead of 
    the wasteful 60x per second which means you can run with the FPS enabled with very little (but not zero!) impact on emulation.
  * Press (and hold) the left shoulder switch to run the game at max speed... some games run at 120FPS max and others can't even get to 60... but it's improving!
  * The Power Off button no longer makes that hideous screeching noise. And if you decide not to quit, it renders the screen back properly.
  * Fixed the ROM loading problems when the filenames were > 29 characters long.
  * Other minor improvements as time allowed...

V1.0 : 20/05/2011 (Alek Maul)
  * Initial release based on Stella 1.4.2 
  * Compiled with last version of Devkitpro/libnds, so DSi compatible \o/
  * Sound engine written from scratch
  * Video engine modified to refresh screen after each line
  * New palette (better colors) added to stella
  * New menu added to stella
  * Add support for colour / B&W TV switch (from Stella 2.2)
  * Add support for more cards recognition (3E)
  * Add support for PAL/NTSC palette
  
