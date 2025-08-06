-- gameplaySP  Gameboy Advance emulator for Playstation Portable --


-- Release log --

v0.91 (minor cleanup release)

NOTE: I don't usually do minor releases but I rewrote a ton of
things in gpSP 0.9, much of it during the last few days, and although
I spent a lot of time debugging a few bugs inevitably crept in.

# Fixed some issues in the new memory handlers that crept up, hopefully
  should fix the problems between 0.8 and 0.9.
# Fixed a bug introduced in 0.9 that could cause crashes when selecting
  things in the menu (I hope, at least).
# Fixed a bug with a certain invalid opcode causing a jump to be scanned
  when there isn't one (fixes Sabre Wulf).
# Removed 2048 option for audio buffer.

v0.9 (yes, it's still beta)

NOTE: As some of you may be aware I'm pretty much tired of these
unofficial releases by people (okay, mostly single person) who
don't wish to follow my wishes. I'm in the process of asking this
person to stop, in his own language. However, I want to make
something clear. Look at the last six new features in this
changelog. I added these TODAY. I could have done them at any
time. But I didn't, because I spent many (dozens, quite possibly
hundreds) hours debugging games that people want to play. I have
always believed that this is far more important than spending time
on new features. Frankly, I'm tired of my emulator being hacked on
by other people, and if it doesn't stop I'm going to make this
project closed source.

Since I know this information is most visible when updated on the
major sites, note that it is the news posters I am especially
talking to. Next time you upload unofficial releases of my
emulator (without even knowing what's changed) bear in mind that
you're only encouraging me to stop working on this. If you want
to pick sides between me and unofficial devs, be my guest. I'll
let you decide by the contents of this release who's doing more
for my emulator.

Oh, and if you downloaded a version of gpSP that has more than
"gpSP" in its name then you're using one of their versions. Shame
on them for not even removing this threatening message, and shame
on you. Unless you're using a port I endorse (GP2X, Dreamcast, etc),
in which case everything's good.


# Fixed stereo output being reversed.
# Fixed a bug causing misaligned errors on 8bit writes to the gbc
  audio channel 3 wave data (fixes various Super Robot Wars games)
# Fixed DMA with garbage in their upper 4 bits (fixes a crash in
  Zelda: Minish Cap)
# Added double buffering to the rendering, removes line artifacts.
  Big thanks to Brunni for the idea.
# Fixed a bug preventing some SRAM based games from saving (fixes
  MMBN4-6)
# Fixed a bug causing part of EWRAM to potentially get corrupted if
  code segments loaded in EWRAM cross 32KB boundaries (fixes
  Phantasy Star 2)
# Fixed a bug causing games using movs pc in user mode (very bad
  behavior) to crash. Fixes Colin McRae Rally 2.0.
# Improved timing a bit more. Fixes GTA Advance.
# Fixed a sprite clipping bug (fixes crash in third boss of Zelda:
  Minish cap)
# Increased translation buffer size significantly (fixes Donkey Kong:
  King of Swing)
# Fixed a dynarec bug causing add pc, reg to not work in Thumb code
  (fixes crash in DBZ:LoZ, seems to fix crashes in battle in Golden
  Sun, probably fixes other games)
# Made sprites using tiles < 512 not display in modes 3-5 (fixes
  a couple minor graphical bugs)
# Removed abort on instruction 0x00000000 hack, was breaking a
  certain bugged up game (Scurge)
# Fixed bug in flags generating variable logical shifts (fixes
  SD Gundam Force)
# Fixed unaligned 16bit reads (fixes DBZ:LoZ in game)
# Redid contiguous block flag modification checking AGAIN and
  hopefully got it right this time (fixes Mario vs. Donkey Kong)
# Redid ldm/stm instructions, fixing some cases (along with the
  timing improvements fixes Mario & Luigi)
# Fixed 14bit EEPROM addressing (hopefully fixes saving in a lot
  of games)
# Completely redid memory handlers, accurately emulates open and
  BIOS reads now. Fixes Zelda: Minish Cap (roll bug, last dungeon),
  Rayman, MMBN 1 (last dungeon), probably others.
# Fixed a minor graphical glitch on the edges of the screen
  (thanks Brunni and hlide for the help!)
# Fixed crash on loading savestates from files of games not currently
  loaded, but be sure you have the exact file you loaded it from or
  gpSP will exit.
@ New memory handlers should provide performance boost for games
  that access VRAM significantly (ie 3D games)
@ Added dead flag elimination checking for logical shifts, probably
  doesn't make a noticeable difference but should have been there
  anyway.
+ Added rapidfire to the button mappings.
+ Added auto frameskip. Removed fractional frameskip (don't think
  it's very useful with auto anyway). Select auto in the graphics/
  sound menu to activate it; frameskip value will act as the
  maximum (auto is by default on). Thanks again to Brunni for some
  help with this. Frameskip options are game specific.
+ Added vsync to the rendering. Only occurs when frames aren't
  skipped. Seems to reduce tearing at least some of the time.
+ Added non-filtered video option.
+ Cheat support (Gameshark/Pro Action Replay v1-v3 only), still
  in early stages, doesn't support everything; codes may cause
  the game to crash, haven't determined yet if the codes are bad
  or the implementation is. See cheat section for more information.
+ Added ability to change audio buffer size. Does not take affect
  until you restart the game.
+ Added analog config options.
+ Added ability to set analog sensitivity and turn off analog.
+ Added ability to change the clock speed. This is a game specific
  option. Try lower speeds w/auto frameskip to save battery life.
+ Fixed savestate speed on crappy Sony sticks.

(legend: # bug fix, + feature addition, @ optimization)

v0.8 - ("unlocked" beta)

NOTE 1: It has come to my attention that there are actually BIOSes
out there that are being used that cause some games to not work.
The BIOS md5sum listed here is for the BIOS actually in GBAs and
is as accurate as you'll get, (if you have a GBA and a flashcart
you can dump it yourself, see http://wiki.pocketheaven.com/GBA_BIOS)

NOTE 2: Since I know this is as far as a lot of people here I have a
little request. Please, please, (I'd italicize this if I could)
please stop constantly asking me when the next release will be,
what it'll have, etc. And while you're at it, please stop asking me
to implement wi-fi multiplayer, cheat support, or fix all of your
games. Some things will happen in due time, other things might not
ever happen. I devote about as much time as I can to this emulator
and I carefully include as much as I can in releases to try to
minimize the number of people who will nag me next time (erm, I
mean, to make the most people happy), so I don't release every other
day or anything like that. Sorry that I can't release once a week,
but I'm a lot busier now than I was when I was first developing this
emulator. Good thing I got the first version out before that, wasn't
it?

Congratulations, you made it this far! Now read the rest of the this
thing. *_*


# Fixed bug in dead flag elimination, "alt" version no longer needed.
# Fixed EEPROM saves being saved as 32kb instead of 512bytes/8kb
+ 32MB ROM support has been added. ROMS are "demand loaded" as
  necessary and page swapped out; there might be a small loading lag,
  but I have yet to ever really notice anything.
  NOTE: 32MB ROM support only works for unzipped ROMs.
+ Save states have been added. See the save state menu for save/load
  options.
+ Support for the real-time clock (RTC) chip in Pokemon cartridegs
  and other games. The implementation is based off of VBA's, whatever
  notes on gbadev I could find, and some of my own reverse engineering
  of what the games do... it might not be totally correct. Also,
  setting the time does not work.
+ Per-game configuration. Currently this only saves frameskip and
  frameskip variation options.
+ Removed the flash type option from the menu and instead added it
  to game_config.txt. Hopefully got everything - let me know if you
  find something that isn't there. It's pretty easy to add them if you
  have to.
+ Added a display in the upper left-hand corner to indicate when
  fast-forward is on.
+ Added button bindings for save/load state.
@ Found a fix of StrmnNrmn proportion: far too much unnecessary mutex
  synchronization was going on. Removing the two offending lines of
  code gave a massive speed boost for free. Enjoy.

v0.7 - (beta than ever)

# Fixed a dynarec bug involving flags generating functions in
  contiguous conditional blocks. Fixes music in Super Mario
  Advance 2-4.
# Fixed a dynarec bug where Thumb mov imm instructions wouldn't
  set flags. Fixes Zelda: Minish Cap, Megaman Battle Network,
  probably others. Comes at a slight speed cost.
# Fixed a MIPS dynarec bug where some delay slots might not
  get filled rarely, causing chaos. Don't know if it improves
  any games.
# Improved self-modifying code detection. Makes Golden Sun,
  Golden Sun 2, and Madden 2007 sorta work but excrutiatingly
  slowly. Looking for a game-specific workaround for this - if you
  want to play these games you'll have to wait for now :(
# Fixed a bug causing the interrupt disable flag to go down
  when SWIs are entered, causing crashes/resets. Fixes
  Super Mario Advance 2-4.
# Fixed menu crashing when strings with certain characters are
  printed (for instance going to the menu after loading the
  BIOS)
# Accidentally forgot to render win0 + win1 + objwin when all
  active at the same time, many weeks ago. Added that, should fix
  some parts in games that had frozen screens.
# Fixed some issues with gpsp.cfg needing to be present and
  corrupting, hopefully. At the very least sanity checks are
  performed on the config file.
# Made it so assigning the frameskip button to something besides
  triangle actually worked as expected.
# Fixed ability to restart current game if nothing is loaded
  (ie, crash)
# Added interrupt on cpsr modification support to the dynarec
  (fixes backgrounds in Castlevania: Harmony of Dissonance)
# Added open addressing for ldm/stm instructions (fixes
  Super Mario Advance 3)
# Improved cycle accuracy a little. Don't know of anything this
  fixes, but games with idle loops will run a little better w/o
  idle loop elimination (but should still be added when possible)
# Fixed some bugs causing sound to play sometimes when it shouldn't.
@ Added dead flag elimination for Thumb code. May possibly have
  noticeable performance increases (Thumb emited coded size can
  have a reduction of 20% or more)
@ Added code generation for divide SWI. May have a small speed
  increase in some games.
+ Added analog nub support (special thanks to psp298 for the
  code)
+ Added fractional frameskip. Go below 0 to get them. A frameskip
  of 1/2 for instance means render 2 out of every 3 frames, 2/3
  means render 3 out of every 4 frames, etc. Possibly useful for
  games that are not quite fast enough at fs0 but fullspeed at
  fs1...

v0.6 - (still beta quality, look out for new bugs)

NOTE: Please include gpsp.cfg with EBOOT.PBP, this shouldn't be
 necessary but I think it is right now.

# Fixed a nasty bug that shouldn't have made it into the initial
  release; a lot of games that TECM.. erm.. crash won't anymore.
  NOTE: This doesn't mean that no game will ever crash, freeze,
  otherwise not work.
# Fixed some crashes in the GUI and the ability to "go up" past
  ms0:/PSP. Made the "go up" button square like it was supposed to
  be (instead of cross).
+ There's now a menu that you can access, by default press right
  while holding down triangle for the frameskip bar.
+ Menu option: resize screen aspect ratio, the default is now
  "scaled 3:2" which makes it look more like a normal GBA. You
  can use "fullscreen" (what it was like before) or "unscaled 3:2"
  (tiny but pixel for pixel like a GBA)
+ Menu option: You can now load new games while the current one
  is running.
+ Menu option: You can now restart the currently running game.
+ Menu option: Frameskip variation - this defaults to "uniform"
  whereas it defaulted to "random" last release. Basically, turn
  it on random if you find that frameskip causes flickering
  animations to make things disappear. Other than that it will
  generally look better on uniform.
+ GUI and file loading now have "auto repeat" on the buttons so
  they're not such a pain to navigate.
+ Menu option: Added support for 128KB flash ROM, some games
  require it (Pokemon Firered/Leaf Green, Super Mario Advance 4),
  turn it on before running the game to make sure it works.
  NOTE: There are some versions of these ROMs that have been
  hacked to get around their 128KB flash, and may not even work
  properly at all. Look out for them, these games should save
  128KB save files after you set the setting to it, IF they use
  128KB flash.
+ Menu option: Added ability to make the .sav files only update
  when you exit the emulator, use with extreme caution (in other
  words, it's not a good idea to use something like this in beta
  quality software if you care about your saves). Does NOT update
  if you exit through the home button, don't use the home button
  if you can help it.
+ Zip support thanks to SiberianSTAR. It will load the first file
  with the extension .gba or .bin that it finds.
+ Menu options are saved to gpsp.cfg. Note that it does not save
  frameskip options or flash ROM options because these are very
  per game particular.
+ The emulator will now try to save backup files to something
  more matching the backup size than a fixed 64KB.
@ Loading ROMs and the auto save of the .sav files is MUCH faster
  now. Thanks for the heads up on how to improve this from pollux!
@ While coding for the screen resize code I found that SDL just
  wasn't cutting it and had to code for the GU myself. Turns out
  the new code is faster (but because it is render code any
  improvement will be diminished to nothing as frameskip is
  increased). Special thanks to Zx-81 for the tips on this one
  and for his GU code in the PSPVBA source as an example.
@ Added some games to game_config.txt. Note that not all versions
  of these ROMs will work with these options, try to use the USA
  version if possible.

8-19-2006  v0.5 - Initial release (public beta quality)


-- About --

gameplaySP (gpSP for short) is a GBA emulator written completely from
scratch. It is still pretty young (only having started a 3 months prior
to the first release) and thus rather immature, but it does a decent
job of playing a number of games, and is being improved upon somewhat
regularly. It is currently somewhat minimalistic, in the sourcecode,
presentation, and features. Its number one focus is to deliver a GBA
gaming experience in the most playable way that PSP can manage, with
frills being secondary (although still a consideration, at least for
some of them).

Having said that, optimization was the important way in achieving this
goal, with overall compatability being a near second. Because of this
some games may not run at the favor of running more games significantly
better. Of course, the compatability will improve with time. The
compatability in the current version (0.8) is perhaps around 80%
(assuming the correct BIOS image is used).

Many games will run at their best out of the box, but some games will
run very slowly unless idle loops are taken care of. There is a supplied
ROM database, game_config.txt, that gives idle loop targets and other
settings that may help a game to run better (or at all) on a per-game
basis. Currently (as of version 0.8) a few dozen games are on this list,
mostly only USA versions. This list will continue to be updated; there's
no real telling exactly how many of the ~2500 GBA games will need to
appear here.

gpSP currently requires an authentic GBA BIOS image file to run. It will
make no effort to run without one present; this file is 16kb and should
be called gba_bios.bin and present in the same location as the EBOOT.PBP
file. Please do not ask me where to obtain this, you'll have to look
online or grab it from a GBA. Note that it is not legal to have this file
unless you own a GBA, and even then it's rather gray area.



-- Features --

gpSP mostly emulates the core Gameboy Advance system. As of right now it
does not emulate any special hardware present on various GBA cartridges.


What it emulates:

GBA CPU: All ARM7TDMI ARM and Thumb mode opcodes except block memory w/
 s-bit (probably aren't used in GBA games)
Video: Modes 0, 1, 2 almost completely, basic 3-5 support, sprites,
 windows/OBJ windows
Interrupts: HBlank, VBlank, all timers, all DMA channels, keypad
DMA: Immediate, HBlank, VBlank, sound timer triggered
Sound: Both DirectSound channels and all 4 GBC audio channels
Input: Basic GBA input delivered through PSP controls
Cartridges: Currently supports ROMs up to 32MB in size (the maximum for
GBA) with the technique of ROM page swapping to fit within PSP's RAM.
Backup: 32/64kb SRAM, 64/128kb flash, 512bit/8kb EEPROM
RTC: The real-time clock present in cartridges such as most of the
 Pokemon games and some others.


What it lacks:

Video: No mosaic, bitmap modes lack color effects (alpha, fades),
 there might be some minor inaccuracies in blending...
Cycle accuracy: Very cycle innacurate; CPU is effectively somewhat
 overclocked, meaning games with rampant idle loops will probably run
 rather poorly. DMA transfers effectively happen for free (0 cycle).
 Please do NOT use gpSP as a first source for developing GBA homebrew,
 try No$GBA instead.


Additional features it has:
- The ability to attempt to run games at faster than GBA speed (sometimes
  they can end up a lot faster, other times not so much)
- Savestates: the ability to save a game's state to a file and resume
  playing where you left off later.
- Mild cheat support


Features that it doesn't have (please don't ask me to implement these!)
- Wi-fi multiplayer


-- Controls --

The default control scheme is very simple. If you don't like it you can
change it in the configuration menu.

At the ROM selection screen:

Up/down: navigate current selection window.
Left/right: switch between file window and directory window.
Circle/start: select current entry.
Square: go one directory up.

In game:

Up/down/left/right: GBA d-pad
Circle: GBA A button
Cross: GBA B button
Square/start: GBA start button
Select: GBA select button
Left trigger: GBA left trigger
Right trigger: GBA right trigger
Triangle: Adjust frameksip

In frameskip adjustment:

Hold down triangle to keep up, press up/down to increase/decrease
frameskip, respectively. Press down at 0 to change to auto, and up
at auto to change to 0.

In the menu:

Up/down: navigate current menu.
Left/right: change value in current menu selection (if a value is present)
Circle/start: select current entry (see help for entry to see what this means)
Square: exit the current menu.


-- Frameskip --

The purpose behind frameskip is to cause the emulator to not render every
frame of graphics to make the emulation closer to fullspeed. Many games will
run fullspeed without skipping any frames, however, some (particularly more
graphically demanding ones) will require this.

Frameskip can be set to two forms, either auto or manual. Auto will attempt
to skip only as many frames as necessary to make the game full speed, and
will not skip more than 4 in a row no matter what speed the game runs at
(at this point the benefits of frameskip yield diminishing returns).

It is recommended that you keep frameskip on auto, but manual is maintained
as an option if you want it and works as follows:

Manual frameskip will only render one out of every (n + 1) frames, where n
is the current frameskip value (so 0 will render everything). Increasing
the frameskip can improve speed, especially with very graphically
intensive games.


-- Cheats --

Currently, gpSP supports some functionality of Gameshark/Pro Action Replay
cheat codes. To use these, you must first make a file with the same name
as the ROM you want the cheat code to apply to, but with the extension .cht.
Put this file in the same directory as the ROM. To make it use a normal
text editor like notepad or wordpad if you're on Windows.

To write a code, write the type of model it is, gameshark_v1, gameshark_v3,
PAR_v1, or PAR_v3. gameshark_v1/PAR_v1 and gameshark_v3/PAR_v3 respectively
are interchangeable, but v1 and v3 are not! So if you don't know which
version it is, try both to see if it'll work.

Then, after that, put a space and put the name you'd like to give the cheat.

On the next several lines, put each cheat code pair, which should look like
this:

AAAAAAAA BBBBBBBB

Then put a blank line when you're done with that code, and start a new code
immediately after it. Here's an example of what a cheat file should look
like:


gameshark_v3 MarioInfHP
995fa0d9 0c6720d2

gameshark_v3 MarioMaxHP
21d58888 c5d0e432

gameshark_v3 InfHlthBat
6f4feadb 0581b00e
79af5dc6 5ce0d2b1
dbbd5995 44b801c9
65f8924d 2fbcd3c4

gameshark_v3 StopTimer
2b399ca4 ec81f071


After you have written the .cht file, you have to enable the cheats
individually in the game menu. Go to the Cheats/Misc menu, and you will
see the cheats; turn them on here. You may turn them on and off as you
please, but note that some cheats may still hold after you turn them off,
due to the nature of the system. Restart to completely get rid of them.

IMPORTANT NOTES:

This is still very work in progress! I basically added this in only 1.5
or so hours, and I don't have a lot of time right now to work on it
before releasing. So I'll probably improve it later.

Not all of gameshark's features are supported, especially for v3. Only
basic cheats will work, more or less.

Cheats may be unstable and may crash your game. If you're having problems
turn the cheats off.

Really, there's no guarantee that ANY cheats will work; I tried a few and
some seem to work, others are questionable. Try for yourself, but don't
expect anything to actually work right now. Do expect this feature to
improve in future versions.



-- Frequently Asked Questions --

Q) How do I run this on my PSP?

A) Provided is an EBOOT.PBP which will run as is on a 1.0 firmware
   PSP or custom firmware that can run unsigned EBOOTs. On 1.5 firmwares
   you must use a kxploit tool to run it (try SeiPSPtool). On 2.0
   firmwares and higher further exploits must be used - see
   http://pspupdates.qj.net/ for more information. Note that I have NOT
   tested this emulator on any firmware version besides 1.5, and it's
   very possible that it doesn't run well, or at all on higher versions.
   Therefore I strongly recommend you downgrade if possible, and use
   Devhook to run games that require > 1.5 version firmwares.

   Be sure to include in the same directory as the EBOOT.PBP file the
   game_config.txt file included and the gba_bios.bin file which you
   must provide yourself.

   gpSP does not run on PSPs with version 2.71 or higher firmware yet,
   nor does any other homebrew executable.


Q) What is a BIOS image file? Why do I need it to run gpSP? Other GBA
   emulators don't require this...

A) The GBA BIOS image file is a copy of a ROM on the GBA system that
   has code for starting up the GBA (it shows the logo), verifying the
   game, and most importantly, providing utility functions for the games
   to use. It is the latter that gpSP needs the BIOS present for. It's
   possible to replace all of these functions with equivilent code, but
   this will take time - in the future gpSP may not require a BIOS image.


Q) I can't find this BIOS image.. please send it to me.

A) Sorry, but you're on your own. I won't send you a BIOS or tell you
   where to get one (unless you want to rip it from a GBA yourself, in
   which case I'll just give you the same link at the top). I can't do
   this because it's not legal to send it around and I don't want to
   get a reputation for illegally distributing BIOS images.


Q) How do I know I have the right BIOS?

A) If you have md5sum you can check if it has this hash:
   a860e8c0b6d573d191e4ec7db1b1e4f6
   That BIOS should work fine. I think that some others work fine too,
   although I haven't confirmed this with absolute certainty. It's also
   theoretically possible to use custom (and free) BIOS replacements,
   but I don't know of any publically availablone ones.

   As far as I'm aware there are two BIOSes floating around, I doubt
   you'll get one that isn't one of those two. There's a very easy way
   to determine which one you have - just look at the very first byte in
   a hex editor. The correct BIOS begins with 0x18, the buggy BIOS begins
   with 0x14.


Q) My favorite game won't run.

A) There probably isn't anything you can do about this, although a
   change to game_config.txt might help. gpSP is still an emulator in
   its infancy so the compatability is not superb. I don't have time
   to test too many games so I'm releasing it as a public beta to get
   a feel for some things that don't work. The next version could
   perhaps fix it, or it might never run. There are always other
   emulators of course, please try one.

   However, before nagging me there is one thing I recommend you try,
   and that is to add the option "iwram_stack_optimize = no" for the
   game in game_config.txt. See the file itself for more information
   on how to do this. If this fixes your game (and it's not already
   in the game_config.txt) please tell me about it.


Q) My favorite game is very slow.

A) Emulating GBA takes a number of resources and getting it done well
   on PSP is no simple task by any means. Some games are just going to
   overwhelm the emulator completely. Of course, there is one special
   case of game (a lot of early generation games fall under this
   category) that can be made much faster by a simple addition to the
   game_config.txt file. Wait for a new version of this file or the
   next version of the emulator and the game may be improved.

   That aside, there are still numerous optimizations that can be done,
   and I sure you future versions will be faster (I just can't tell you
   how much)

   Also, a lot of games will be sped up considerably by adding an
   idle_loop_eliminate_target line for it in game_config.txt. There
   are some more obscurer options there that can improve speed too. If
   the game is VERY slow there might be something wrong with its
   emulation that can be improved. For instance, if you can't get a game
   to run fullspeed on any frameskip you should e-mail me about it.


Q) Some games run fullspeed but the sound is messed up. Why?

A) At least 9 out of 10 times it means the game isn't really running
   full speed, but just that you can't notice the difference. Increasing
   frameskip will almost always improve sound quality in these
   situations, to a certain point (after around frameskip 3 you
   probably won't be seeing many more returns if it isn't already
   fullspeed). The rest of the time it means there's a bug somewhere else
   in the emulator, probably in the CPU core. Chances are that all you
   can do is wait for it to be fixed in a later release.


Q) The emulator crashed!

A) Most games that don't run will probably take the emulator down with
   it, or it could be an emulator bug completely unrelated to the game
   (but unlikely). Press home and wait for the next version.

   There is some information that comes up when the game crashes. This
   information may be slightly useful to me, but chances are it
   usually won't be all that interesting.

   These days games are more likely to exit with a "bad crash" error.
   This number is possibly useful to me, but to debug a game I'll have
   to reproduce the crash anyway. When this happens it's probably due to
   a bug in the CPU core that hasn't been fixed yet.


Q) Why won't my game save?

A) The game might need 128KB flash turned on and might not be listed in
   game_config.txt. See game_config.txt for more information regarding
   this. Be sure to include game_config.txt with the EBOOT.PBP file.

   Other games might simply have bugs in the save support. For now, use
   savestates as an alternative if you can't save.


Q) How do I change sound quality?

A) Right now, you can't. For those wondering, sound is locked at 44.1KHz
   (sounds a bit high? It is, but it's currently necessary to play
   everything correctly). I don't have any plans to allow changing this
   right now, because I don't think there's really much reason to be
   able to (it'd be a tiny speed boost at best and I don't think SDL even
   allows for anything besides this sampling rate on PSP)


Q) What is this emulator's name?

A) Um.. what? It's gameplaySP, isn't it? You call it gpSP for short.
   Somehow the name can't have the acronyms gbSP, gbapSP, or really
   just about anything else you feel like giving it. Oh, and if you
   really want to make me happy get the capitalization right too.
   That's gpSP, not gPSP, GPsp.. you get the idea.


Q) Does gpSP run Gameboy/Gameboy Color games? Will it later?

A) No. Even though GBA can run these games it uses separate hardware
   that proper GBA games have no access to (save for the audio chip),
   and thus there's no point including it in a GBA emulator (it
   doesn't help run GBA games). I recommend using a GB/GBC emulator
   like Rin for playing these games. It'll probably give you a lot
   more options anyway. gpSP will never actually emulate GB/GBC
   games. You'd may as well be waiting for it to emulate PS2 games...
   (that was an analogy. gpSP won't ever emulate PS2 games. >_>)


Q) Other emulators use the PSP's graphical hardware to accelerate the
   video emulation. Is this possible for gpSP?

A) I'm honestly not too sure at this point. It's definitely a rather
   complicated procedure, and I don't think it'll be possible to
   accurately accelerate alpha blending. On the other hand, affine
   transformations could perhaps receive a speed boost this way. Any
   solution would have to be hybrid hardware/software, which might be
   possible due to the nature of the PSP's VRAM. Maybe someone will
   be willing to help me think of possibilities here?

   But don't bother of you're just going to tell me to render a list
   of quads...


Q) Other emulators use the PSP's second CPU to offload the sound
   emulation. Is this possible for gpSP?

A) Yes, but it wouldn't improve it nearly as much as say, SNES9x TYL.
   This is because most of the processing that goes into sound on a GBA
   game is done in the CPU, not in dedicated audio hardware. It could
   help a little, but probably not a lot. Maybe enough to be worthwhile.
   It might also be possible to split the video rendering to the main
   CPU and the main emulation to the secondary one, but there are a lot
   of coherency issues involved.


Q) I heard gpSP can only load games 16MB or smaller in size. Is this
   true? What about zipped games?

A) As of version 0.8 gpSP can play 32MB ROMs. However, they must be
   unzipped. The reason for this is that parts of the ROM are constantly
   loaded to memory as needed, and for this to be as fast as possible the
   ROM has to be present on the memory stick in raw format.

   You might be wondering, why not just have gpSP unzip the ROM to a file
   then delete the file when it is done? The reason why is because this
   would impose a "hidden" requirement of 32MB on the user that very
   likely may not be there. Furthermore, there are only a few 32MB games
   that anyone actually wants to play. If you only have one 32MB game on
   your memstick then it'd actually require signifnicantly more free space
   to hold both the ROM and the 32MB raw file. With 2 32MB ROMs you only
   gain a around 10-25MB of free space, depending on how effective the
   compression is.


Q) Savestates? From other emulators??

A) See the savestates option in main menu. gpSP will probably never
   support savestates from other emulators, there's just too much in the
   way of emulator specific data in them.

   Savestates are currently 506,943 bytes. They would be a little smaller
   without the snapshot, but I find that very useful and it wouldn't help
   size immensely. Compression would help, but I wanted the size to be
   constant so you knew exactly how much you could hold and to improve
   save/load speed.


Q) What's with the zip support?

A) I hear stories that some games work unzipped and not zipped, so you
   might want to try unzipping them if it gives you problems. You also
   might want to try making fresh zips with WinRAR - users have
   reported some higher success rates doing this.


Q) What's with the config file? Should I make it read only?

A) There was a bug in version 0.6 that caused the config file to not
   get updated or get corrupted sometimes. Hopefully this is fixed now,
   but if it DOES get corrupted making it read only can prevent this
   from happening in the future.


Q) So when WILL the next version be released?

A) Sorry, but I almost never announce release dates. Furthermore, I'll
   probably be pretty hush hush on internal development, just to keep
   people from nagging me about it and building too much suspense.


Q) I don't like this emulator. Are there other alternatives?

A) Yes. Try PSPVBA by Zx-81 (http://zx81.dcemu.co.uk/). Overall I doubt
   the compatability is significantly higher than gpSP's anymore, but
   I'm sure there are some games it runs that gpSP doesn't.


Q) I heard there was a version of gpSP for PCs. Is that true?

A) I developed this emulator internally on PC. It might have a speed
   advantage over other PC GBA emulators, although the PSP version has
   more sophisticated optimizations. Most people have fast enough
   computers to run better GBA emulators for PC and gpSP lacks some
   important features (screen resizing) that the PSP version kinda
   hides. Even though gpSP spent a majority of its development
   gestation as a PC app it was always developed with the PSP in mind,
   so the PC version will probably not see the light of the day unless
   I get overwhelming demand for it. It is, however, possible to
   build it from the source. But I request that you don't distribute
   such builds. If you happen to find one, bear in mind that I don't
   offer any support for it, and as far as I'm concerned it won't
   exist.


Q) I hear there's a version of gpSP for other platforms too, like
   Dreamcast. And I hear they're slow! What gives?

   These are ports, done by other people (or maybe myself?). This is
   possible because gpSP is open source and its base version is fairly
   portable, but to run fast enough on anything but platforms quite a
   bit faster than PSP it at least needs a CPU specific dynarec backend.

   I don't (necessarily) maintain all builds of gpSP, so you'll have to
   contact the authors of these ports for more information. That
   notwithstanding, I try to get as involved in other ports of gpSP as
   I can.


Q) I want to modify gpSP. How can I do this, and am I at liberty to do
   so?

A) Yes, you are, under the terms of the GPL (see the included
   COPYING.DOC). You can download the sourcecode from whereever you
   downloaded this; if you can't find it please e-mail me and I'll give
   you a link to it. I would vastly appreciate it if you contacted me first
   before forking my project, especially if you're just looking to gain
   recognition without adding much to it. It's better to keep all changes
   tidy in one branch of development.

   I would like to stress this a little more seriously (hopefully those
   interested are reading this). Although you are legally entitled to
   release your own forks of gpSP it would be much more benficial to me,
   to you, and to the users of this program if you instead tried working
   with me to get your changes incorporated into the next version. I
   really don't feel like competing with other builds of my source
   anymore, so please do me a big favor and send me an e-mail if you want
   to work with gpSP.


Q) How do I build gpSP?

A) make will build it. You need to have SDL for PSP installed, as well
   as the standard PSP toolchain/PSPSDK and zlib. gpSP isn't much of a
   "build it yourself" program so please don't bother me much about how to
   build it unless you have a good reason for wanting to do so.


Q) What is with the version numbers?

A) Anything less than 1.0 means beta. Beta means that I still have major
   plans for working on it, and that I don't fully back it as being
   stable or reliable software. Of course, if it does hit 1.0, that doesn't
   mean it'll be perfect. It just means I'll have spent a lot of cumulative
   time working things out. The closer it gets to 0.9, the happier I am with
   it overall.


Q) Donations?

A) Very appreciated. exophase@gmail.com on PayPal. <3


Q) How can I contact you?

A) exophase@gmail.com, Exophase on AIM, exophase@adelphia.net on MSN. I
   welcome IMs, but if you nag me a lot you'll make me sad inside. And
   don't ask me for ROMs or the GBA BIOS. I figured this was common sense
   but apparently not.


-- Credits --

Exophase: main developer
siberianSTAR: zip support
psp298: analog nub code

Beta testers for 0.7:
theohsoawesome1
thisnamesucks837
blackdragonwave9
dagreatpeewee
xsgenji

Beta testers for 0.8:
Runaway_prisoner
theohsoawesome1
tanyareimyoko
spynghotoh2020

Beta testers for 0.9:
RunawayPrisoner (my right hand man)
Veskgar (my left hand man)
qasim

-- Special thanks --

Quasar84: He's helped me in so many ways with this. We both kinda learned
GBA together, he did little demos for me and I got them emulated. It was
great trying out your more advanced code for your own projects once you
got to them, it was equally rewarding to see your work and to be able to
run it at the same time. Least of all I wouldn't have been able to do any
of this without your constant support and presence. I really owe this
release to you.

gladius: You are an amazing GBA coder. I wouldn't have been able to get
through some tough parts without your help. Its been good talking about
ideas with you.. I'm sure you're glad to see that there's finally a GBA
emulator with dynarec ;)


Many, many others of course, probably too many to name, and I don't want
to make anyone feel bad by putting others above them (well, except those
two, heh) so if you think you should be on here, you probably should be!
Just pretend you are for now, and maybe I'll put you here next time.

