// =====================================================================================================
// Stella DS/DSi Pheonix Edition - Improved Version by Dave Bernazzani (wavemotion)
//
// Copyright (c) 2020-2026 by Dave Bernazzani
//
// Copying and distribution of this emulator, it's source code and associated 
// readme files, with or without modification, are permitted in any medium without 
// royalty provided this copyright notice is used and wavemotion-dave (Phoenix-Edition),
// Alekmaul (original port) are thanked profusely along with the entire Stella Team.
//
// The StellaDS emulator is offered as-is, without any warranty.
// =====================================================================================================
#include <nds.h>
#include <fat.h>
#include <stdio.h>

#include "intro.h"
#include "StellaDS.h"
#include "highscore.h"
#include "config.h"

#include "clickNoQuit_wav.h"
#include "clickQuit_wav.h"

int main(int argc, char **argv) 
{
  // Init sound
  consoleDemoInit();
  soundEnable();
  lcdMainOnTop();
    
  // Init Fat
	if (!fatInitDefault()) {
		iprintf("Unable to initialize libfat!\n");
		return -1;
	}

  // Load the configuration database (or create it)
  LoadConfig();    
  
  // Load the favorites 
  LoadFavorites();
    
  // Init Timer
  dsInitTimer();
  
  // Init sound emulation
  dsInstallSoundEmuFIFO();
    
  // Intro and main screen
  intro_logo();
    
// Init high score (must be done after FAT init)
  highscore_init();
      
  dsInitScreenMain();
  if (argc > 1) 
  {
    dsShowScreenMain(true);
    emuState = STELLADS_PLAYINIT;
    dsLoadGame(argv[1]);
  } 
  else 
  {
      chdir("/roms");    // Try to start in roms area... doesn't matter if it fails
      chdir("a2600");    // And try to start in the subdir /a2600... doesn't matter if it fails.
      emuState = STELLADS_MENUINIT;
  }

  // Main loop of emulation
  dsMainLoop();
  	
  // Free memory to be correct 
  dsFreeEmu();
 
  return 0;
}

