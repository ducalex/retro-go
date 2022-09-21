#include <stdlib.h>

#include "cvars.h"
#include "cvar_defs.h"
#include "console.h"

// Required for certain cvars
#include "types.h"
#include "build.h"
#include "funct.h"

//We need a way to access duke to change the level
#include "duke3d.h"


//We need to access debug soundinfo
#include "sounddebugdefs.h"

#include "joystick.h"

#include "music.h"

// Bind our Cvars at startup. You can still add bindings after this call, but
// it is recommanded that you bind your default CVars here.
void CVARDEFS_Init()
{
    g_CV_console_text_color = 0; // Set default value
    REGCONVAR("SetConsoleColor", " - Change console color.",g_CV_console_text_color, CVARDEFS_DefaultFunction);

    g_CV_num_console_lines = 0; // Set default value
    REGCONVAR("NumConsoleLines", " - Number of visible console lines.", g_CV_num_console_lines, CVARDEFS_DefaultFunction);

    g_CV_classic = 0; // Set default value
    REGCONVAR("Classic", " - Enable Classic Mode.", g_CV_classic, CVARDEFS_DefaultFunction);

	// FIX_00022b: Sound effects are now sharper and they sound as in the real DOS duke3d.
    g_CV_CubicInterpolation = 0; // Set default value
    REGCONVAR("EnableCubic", " - Turn on/off Cubic Interpolation for VOCs.", g_CV_CubicInterpolation, CVARDEFS_DefaultFunction);

    g_CV_TransConsole = 1; // Set default value
    REGCONVAR("TransConsole", " - Toggle the transparency of the console", g_CV_TransConsole, CVARDEFS_DefaultFunction);

    g_CV_DebugJoystick = 0;
    REGCONVAR("DebugJoystick", " - Displays info on the active Joystick", g_CV_DebugJoystick, CVARDEFS_DefaultFunction);

	sounddebugActiveSounds = 0;
	sounddebugAllocateSoundCalls = 0;
	sounddebugDeallocateSoundCalls = 0;
	g_CV_DebugSound = 0;
    REGCONVAR("DebugSound", " - Displays info on the active Sounds", g_CV_DebugSound, CVARDEFS_DefaultFunction);

	g_CV_DebugFileAccess = 0;
    REGCONVAR("DebugFileAccess", " - Displays info on file access", g_CV_DebugFileAccess, CVARDEFS_DefaultFunction);
	
    REGCONVAR("TickRate", " - Changes the tick rate", g_iTickRate, CVARDEFS_DefaultFunction);
    REGCONVAR("TicksPerFrame", " - Changes the ticks per frame", g_iTicksPerFrame, CVARDEFS_DefaultFunction);

    REGCONFUNC("Quit", " - Quit game.", CVARDEFS_FunctionQuit);
    REGCONFUNC("Clear", " - Clear the console.", CVARDEFS_FunctionClear);
	REGCONFUNC("Name", " - Change player name.", CVARDEFS_FunctionName);
    REGCONFUNC("Level", " - Change level. Args: Level <episode> <mission>", CVARDEFS_FunctionLevel);
    REGCONFUNC("PlayMidi"," - Plays a MIDI file", CVARDEFS_FunctionPlayMidi);

    REGCONFUNC("Help"," - Print out help commands for console", CVARDEFS_FunctionHelp);
}

// I any of the Cvars need to render.. to it here.
void CVARDEFS_Render()
{
    if(g_CV_DebugJoystick)
    {
        int i;
        char  buf[128];
        minitext(2, 2, "Debug Joystick", 17,10+16);

        for(i = 0; i < MAXJOYAXES; i++)
        {
            sprintf(buf, "Joystick Axis%d: Raw: %d  Used:%d", i,_joystick_axis(i), CONTROL_GetFilteredAxisValue(i));
            minitext(2, (i*8)+12, buf, 23,10+16);
        }

        for(i = 0; i < MAXJOYBUTTONS; i++)
        {
            sprintf(buf, "Button%d: %d", i, _joystick_button(i));
            if(i < (MAXJOYBUTTONS/2))
            {
                minitext(2, (i*8)+(MAXJOYAXES*8)+12, buf, 23,10+16);
            }
            else
            {
                minitext(55, ((i-16)*8)+(MAXJOYAXES*8)+12, buf, 23,10+16);
            }
        }

        for(i = 0; i < MAXJOYHATS; i++)
        {
            sprintf(buf, "Hat%d: %d", i, _joystick_hat(i));
            minitext(110, (i*8)+(MAXJOYAXES*8)+12, buf, 23,10+16);
        }


    }

	if(g_CV_DebugSound)
	{
        char  buf[128];
        minitext(2, 2, "Debug Sound", 17,10+16);

		sprintf(buf, "Active sounds: %u", sounddebugActiveSounds);
		minitext(2, 10, buf, 23,10+16);
		
		sprintf(buf, "Allocate Calls: %u", sounddebugAllocateSoundCalls);
		minitext(2, 18, buf, 23,10+16);

		sprintf(buf, "Deallocate Calls: %d", sounddebugDeallocateSoundCalls);
		minitext(2, 26, buf, 23,10+16);
	}

}

// For default int functions
// If your CVAR should simply change a global 'int' variable,
// Then, use this function.
void CVARDEFS_DefaultFunction(void* var)
{
    int argc;
    cvar_binding* binding = (cvar_binding*)var;

    argc = CONSOLE_GetArgc();

    if(argc < 1)
    {
        //print out the current state of the var if no args are given
        CONSOLE_Printf("%s %d", binding->name, *(int*)binding->variable);
        return;
    }

    //change the var
    *((int*)binding->variable) = atoi( CONSOLE_GetArgv(0) );

}

// This function will quit the game
void CVARDEFS_FunctionQuit(void* var)
{
  if( numplayers > 1 )
  {
	if(ps[myconnectindex].gm&MODE_GAME)
	{
		gamequit = 1;
		CONSOLE_SetActive(0);
    }
    else
    {
		sendlogoff();
		gameexit(" ");
    }
  }
  else if( numplayers < 2 )
   gameexit(" ");
}

// This function will quit the game
void CVARDEFS_FunctionClear(void* var)
{
    CONSOLE_Reset();
}

//And the game will reflect the changes. Will also return the current name of
//The player
void CVARDEFS_FunctionName(void* var)
{
	int argc, length,i;

	argc = CONSOLE_GetArgc();
	
	//Check to see if we're changing name's, or checking the name
	if (argc==1) {

		//The Fragbar up the top doesn't look very good with more than
		//10 characters, so limit it to that
		if (strlen(CONSOLE_GetArgv(0)) > 10) {
			CONSOLE_Printf("User name must be 10 characters or less");
			return;
		}

		//Loop through the length of the new name
		for (i=0;CONSOLE_GetArgv(0)[i];i++)
		{
			//Copy it to the local copy of the name
			ud.user_name[myconnectindex][i] = toupper(CONSOLE_GetArgv(0)[i]);
			//And the packet we're going to send the other players
			tempbuf[i+2] = toupper(CONSOLE_GetArgv(0)[i]);
		}
		
		//Delimit the local copy with a null character
		ud.user_name[myconnectindex][i] = 0;
		
		//If we are online
		if(numplayers > 1)
		{
			//The packet descriptor is 6
			tempbuf[0] = 6;
			//We need to send the version of the game we're running
			//Since names used to be only sent once, this was where they
			//Checked that everyone was running the same version
			tempbuf[1] = grpVersion;
            //Delimit the buffer with a null character.
			tempbuf[i+2] =0;
			//The length will be 1 more than the last index
			length = i +3;
			
	        for(i=connecthead;i>=0;i=connectpoint2[i])
            {
	            if (i != myconnectindex)
		            //Send it to everyone
					sendpacket(i,(uint8_t*)tempbuf,length);
            }
        }	
	} 
    else 
    {
		//If there's no arguement, just print out our name
		CONSOLE_Printf("Current Name: %s", ud.user_name[myconnectindex]);
	}

 
}


// This function loads a new level
void CVARDEFS_FunctionLevel(void* var)
{
	int argc;
	short volnume,levnume, i;

	//Find out how many arguements were passed
	argc = CONSOLE_GetArgc();
	
	//If there's 2
	if (argc == 2) {
		//The episode number is the first arguement
		volnume = atoi(CONSOLE_GetArgv(0));
		
		//The level is the second
		levnume = atoi(CONSOLE_GetArgv(1));
		volnume--;
        levnume--;
                            
        // Make sure the number aren't out of range.
        // This is based on the 1.5 data files.
        if(levnume<0 || volnume<0) 
            {return;}
        if(volnume>3) 
            {return;}
        switch(volnume)
        {
        case 0: // ep1
            {
                if(levnume > 7)
                {
		            CONSOLE_Printf("Invalid Level Selection");
                    return;
                }
            }
            break;
        case 1: // ep2
        case 2: // ep3
        case 3: // ep4
            {
                if(levnume > 10)
                {
		            CONSOLE_Printf("Invalid Level Selection");
                    return;
                }
            }
            break;
   
        default:
            break;
        }

		ud.m_volume_number = ud.volume_number = volnume; //update the current volume
		ud.m_level_number = ud.level_number = levnume; //And level

		//If we're playing online
		if(numplayers > 1 && myconnectindex == connecthead)
		{
			//Fill out the game data
			tempbuf[0] = 5;
			tempbuf[1] = ud.m_level_number;
            tempbuf[2] = ud.m_volume_number;
            tempbuf[3] = ud.m_player_skill;
            tempbuf[4] = ud.m_monsters_off;
            tempbuf[5] = ud.m_respawn_monsters;
            tempbuf[6] = ud.m_respawn_items;
            tempbuf[7] = ud.m_respawn_inventory;
            tempbuf[8] = ud.m_coop;
            tempbuf[9] = ud.m_marker;
            tempbuf[10] = ud.m_ffire;

            for(i=connecthead;i>=0;i=connectpoint2[i])
            {
                sendpacket(i,(uint8_t *)tempbuf,11); //And send the packet to everyone
            }
        }
		else ps[myconnectindex].gm |= MODE_RESTART; //Otherwise just restart the game

	} 
    else 
    {
		//If there's not 2 arguements, print out the error message
		CONSOLE_Printf("Level (Episode Number) (Level Number)");
	}
}

// Tries to load a external mid file... :)
void CVARDEFS_FunctionPlayMidi(void* var)
{
    if(CONSOLE_GetArgc() < 1)
    {
        return;
    }

	PlayMusic(CONSOLE_GetArgv(0));		// Gets the first parameter and tries to load it in ( Doesn't crash if invalided )
}


// Help function and finds specific help commands...
void CVARDEFS_FunctionHelp(void* var)
{	
	int i, numArgs, numCvars;
	char  *helpcmd = CONSOLE_GetArgv(0);
	numCvars = CVAR_GetNumCvarBindings();
    numArgs = CONSOLE_GetArgc();

	if(numArgs < 1)// If no extra arugment was passed print below..
        CONSOLE_Printf("Console Command List:\n\n");
	for(i = 0; i < numCvars; i++)
	{
		cvar_binding* binding = CVAR_GetCvarBinding(i);
	    if(numArgs < 1)
        {
			CONSOLE_Printf("%s\t%5s",binding->name,binding->help);
		}
		else
        {
			// Did we find it?
			if ( strcmpi(helpcmd, binding->name) == 0 )
			{
				CONSOLE_Printf("%s\t%5s",binding->name,binding->help);
				break;
			}	
		}
	}	

}
