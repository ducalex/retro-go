//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
aint32_t with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include "duke3d.h"
#include "keyboard.h"

/*
=============================================================================

                               GLOBAL VARIABLES

=============================================================================
*/

byte  KB_KeyDown[ MAXKEYBOARDSCAN ];   // Keyboard state array
kb_scancode KB_LastScan;

static volatile boolean keyIsWaiting = 0;

static uint8_t  scancodeToASCII[ MAXKEYBOARDSCAN ];
static uint8_t  shiftedScancodeToASCII[ MAXKEYBOARDSCAN ];
static uint8_t  extscanToSC[ MAXKEYBOARDSCAN ];

/*
=============================================================================
FUNCTIONS
=============================================================================
*/

void keyhandler(void)
{
    static boolean gotextended = false;
    
    int rawkey = _readlastkeyhit();
    int lastkey = rawkey & 0x7f;
    
    // 128 bit means key was released.
    int pressed = !(rawkey & 0x80);
    
    if (rawkey == 0xe0 && !gotextended)
    {
    	gotextended = true;
        return;
    }

    if (rawkey == 0xe1)
    {
    	/* SBF - build doesn't actually generate this for Pause/Break */
    	STUBBED("Another extended key!");
    	return;
    }
        
    if (gotextended)
    {
    	gotextended = false;
    	
    	/* remap extended key to Duke3D equivalent */
    	lastkey = extscanToSC[lastkey];
    }
    
    if (lastkey >= MAXKEYBOARDSCAN)
    {
        STUBBED("Scancode out of range!");
        return;
    }

    if (pressed)
    {
         KB_LastScan = lastkey;
    }

    KB_KeyDown[lastkey] = pressed;

    keyIsWaiting = ((keyIsWaiting) || (KB_KeyDown[lastkey]));
    
    CONTROL_UpdateKeyboardState(lastkey, pressed);
}

void KB_KeyEvent( int scancode, int keypressed )
{
	STUBBED("KB_KeyEvent");
}

int KB_KeyWaiting( void )
{
    _handle_events();
    return keyIsWaiting;
}

uint8_t  KB_Getch( void )
{

    while (!keyIsWaiting) { _idle(); /* pull the pud. */ }
	keyIsWaiting = false;
    if (KB_LastScan >= MAXKEYBOARDSCAN)
        return(0xFF);

    if (KB_KeyDown[sc_LeftShift] || KB_KeyDown[sc_RightShift])
        return shiftedScancodeToASCII[KB_LastScan];

    return scancodeToASCII[KB_LastScan];
}

void KB_Addch( uint8_t  ch )
{
	STUBBED("KB_Addch");
}

void KB_FlushKeyboardQueue( void )
{
	_handle_events();
	keyIsWaiting = false;
	memset(KB_KeyDown, 0, sizeof(KB_KeyDown));
	// FIX_00077: Menu goes directly to the "NEW GAME" sub-menu when starting new game (Turrican)
}

void KB_ClearKeysDown( void )
{
    memset((void *) KB_KeyDown, '\0', sizeof (KB_KeyDown));
	keyIsWaiting = false;
}

const static struct {
	char * name;
	int code;
} keyname2scancode[] = {
	{ ",",		sc_Comma	},
	{ ".",		sc_Period	},
	{ "Enter",	sc_Return	},
	{ "Escape",	sc_Escape	},
	{ "Space",	sc_Space	},
	{ "BakSpc",	sc_BackSpace	},
	{ "Tab",	sc_Tab		},
	{ "LAlt",	sc_LeftAlt	},
	{ "LCtrl",	sc_LeftControl	},
	{ "CapLck",	sc_CapsLock	},
	{ "LShift",	sc_LeftShift	},
	{ "RShift",	sc_RightShift	},
	{ "F1",		sc_F1		},
	{ "F2",		sc_F2		},
	{ "F3",		sc_F3		},
	{ "F4",		sc_F4		},
	{ "F5",		sc_F5		},
	{ "F6",		sc_F6		},
	{ "F7",		sc_F7		},
	{ "F8",		sc_F8		},
	{ "F9",		sc_F9		},
	{ "F10",	sc_F10		},
	{ "F11",	sc_F11		},
	{ "F12",	sc_F12		},
	{ "Kpad*",	sc_Kpad_Star	},
	{ "Pause",	sc_Pause	},
	{ "ScrLck",	sc_ScrollLock	},
	{ "NumLck",	sc_NumLock	},  // 6 chars Max please.
	{ "/",		sc_Slash	},
	{ ";",		sc_SemiColon	},
	{ "'",		sc_Quote	},
	{ "`",		sc_Tilde	},
	{ "\\",		sc_BackSlash	},
	{ "[",		sc_OpenBracket	},
	{ "]",		sc_CloseBracket	},
	{ "1",		sc_1		},
	{ "2",		sc_2		},
	{ "3",		sc_3		},
	{ "4",		sc_4		},
	{ "5",		sc_5		},
	{ "6",		sc_6		},
	{ "7",		sc_7		},
	{ "8",		sc_8		},
	{ "9",		sc_9		},
	{ "0",		sc_0		},
	{ "-",		sc_Minus	},
	{ "=",		sc_Equals	},
	{ "+",		sc_Plus		},
	{ "Kpad1",	sc_kpad_1	},
	{ "Kpad2",	sc_kpad_2	},
	{ "Kpad3",	sc_kpad_3	},
	{ "Kpad4",	sc_kpad_4	},
	{ "Kpad5",	sc_kpad_5	},
	{ "Kpad6",	sc_kpad_6	},
	{ "Kpad7",	sc_kpad_7	},
	{ "Kpad8",	sc_kpad_8	},
	{ "Kpad9",	sc_kpad_9	},
	{ "Kpad0",	sc_kpad_0	},
	{ "Kpad-",	sc_kpad_Minus	},
	{ "Kpad+",	sc_kpad_Plus	},
	{ "Kpad.",	sc_kpad_Period	},
	{ "A",		sc_A		},
	{ "B",		sc_B		},
	{ "C",		sc_C		},
	{ "D",		sc_D		},
	{ "E",		sc_E		},
	{ "F",		sc_F		},
	{ "G",		sc_G		},
	{ "H",		sc_H		},
	{ "I",		sc_I		},
	{ "J",		sc_J		},
	{ "K",		sc_K		},
	{ "L",		sc_L		},
	{ "M",		sc_M		},
	{ "N",		sc_N		},
	{ "O",		sc_O		},
	{ "P",		sc_P		},
	{ "Q",		sc_Q		},
	{ "R",		sc_R		},
	{ "S",		sc_S		},
	{ "T",		sc_T		},
	{ "U",		sc_U		},
	{ "V",		sc_V		},
	{ "W",		sc_W		},
	{ "X",		sc_X		},
	{ "Y",		sc_Y		},
	{ "Z",		sc_Z		},
	{ "Up",		sc_UpArrow	},
	{ "Down",	sc_DownArrow	},
	{ "Left",	sc_LeftArrow	},
	{ "Right",	sc_RightArrow	},
	{ "Insert",	sc_Insert	},
	{ "Delete",	sc_Delete	},
	{ "Home",	sc_Home		},
	{ "End",	sc_End		},
	{ "PgUp",	sc_PgUp		},
	{ "PgDn",	sc_PgDn		},
	{ "RAlt",	sc_RightAlt	},
	{ "RCtrl",	sc_RightControl	},
	{ "Kpad/",	sc_kpad_Slash	},
	{ "KpdEnt",	sc_kpad_Enter	},
	{ "PrtScn",	sc_PrintScreen	},
	{ NULL,		0		}
};

char  *KB_ScanCodeToString( kb_scancode scancode )
{
	int i;
	for(i = 0; keyname2scancode[i].name != NULL; i++)
	{
        if (keyname2scancode[i].code == scancode)
            return keyname2scancode[i].name;
	}
	
	return NULL;
}

kb_scancode KB_StringToScanCode( char  * string )
{
	char * name = NULL;
	int32 i=0;
	name = keyname2scancode[i].name;
	for(;name;++i, name=keyname2scancode[i].name)
	{
		if(!stricmp(name,string))
			break;
	}
	
	return keyname2scancode[i].code;
}

void KB_TurnKeypadOn( void )
{
	STUBBED("KB_TurnKeypadOn");
}

void KB_TurnKeypadOff( void )
{
	STUBBED("KB_TurnKeypadOff");
}

int KB_KeypadActive( void )
{
	STUBBED("KB_KeypadActive");
	return false;
}

void KB_Startup( void )
{
    memset(scancodeToASCII, 0xFF, sizeof (scancodeToASCII));

    // !!! FIXME: incomplete?
    scancodeToASCII[sc_A] = 'a';
    scancodeToASCII[sc_B] = 'b';
    scancodeToASCII[sc_C] = 'c';
    scancodeToASCII[sc_D] = 'd';
    scancodeToASCII[sc_E] = 'e';
    scancodeToASCII[sc_F] = 'f';
    scancodeToASCII[sc_G] = 'g';
    scancodeToASCII[sc_H] = 'h';
    scancodeToASCII[sc_I] = 'i';
    scancodeToASCII[sc_J] = 'j';
    scancodeToASCII[sc_K] = 'k';
    scancodeToASCII[sc_L] = 'l';
    scancodeToASCII[sc_M] = 'm';
    scancodeToASCII[sc_N] = 'n';
    scancodeToASCII[sc_O] = 'o';
    scancodeToASCII[sc_P] = 'p';
    scancodeToASCII[sc_Q] = 'q';
    scancodeToASCII[sc_R] = 'r';
    scancodeToASCII[sc_S] = 's';
    scancodeToASCII[sc_T] = 't';
    scancodeToASCII[sc_U] = 'u';
    scancodeToASCII[sc_V] = 'v';
    scancodeToASCII[sc_W] = 'w';
    scancodeToASCII[sc_X] = 'x';
    scancodeToASCII[sc_Y] = 'y';
    scancodeToASCII[sc_Z] = 'z';
    scancodeToASCII[sc_0] = '0';
    scancodeToASCII[sc_1] = '1';
    scancodeToASCII[sc_2] = '2';
    scancodeToASCII[sc_3] = '3';
    scancodeToASCII[sc_4] = '4';
    scancodeToASCII[sc_5] = '5';
    scancodeToASCII[sc_6] = '6';
    scancodeToASCII[sc_7] = '7';
    scancodeToASCII[sc_8] = '8';
    scancodeToASCII[sc_9] = '9';
    scancodeToASCII[sc_Escape] = asc_Escape;
    scancodeToASCII[sc_Tab] = asc_Tab;
    scancodeToASCII[sc_Space] = asc_Space;
    scancodeToASCII[sc_Enter] = asc_Enter;
    scancodeToASCII[sc_BackSpace] = asc_BackSpace;
    scancodeToASCII[sc_Comma] = ',';
    scancodeToASCII[sc_Period] = '.';
    scancodeToASCII[sc_Kpad_Star] = '*';
    scancodeToASCII[sc_Slash] = '/';
    scancodeToASCII[sc_SemiColon] = ';';
    scancodeToASCII[sc_Quote] = '\'';
    scancodeToASCII[sc_Tilde] = '`';
    scancodeToASCII[sc_BackSlash] = '\\';
    scancodeToASCII[sc_OpenBracket] = '[';
    scancodeToASCII[sc_CloseBracket] = ']';
    scancodeToASCII[sc_Minus] = '-';
    scancodeToASCII[sc_Equals] = '=';
    scancodeToASCII[sc_Plus] = '+';
    scancodeToASCII[sc_kpad_Minus] = '-';
    scancodeToASCII[sc_kpad_Period] = '.';
    scancodeToASCII[sc_kpad_Plus] = '+';

    // !!! FIXME: incomplete?
    memset(shiftedScancodeToASCII, 0xFF, sizeof (shiftedScancodeToASCII));
    shiftedScancodeToASCII[sc_A] = 'A';
    shiftedScancodeToASCII[sc_B] = 'B';
    shiftedScancodeToASCII[sc_C] = 'C';
    shiftedScancodeToASCII[sc_D] = 'D';
    shiftedScancodeToASCII[sc_E] = 'E';
    shiftedScancodeToASCII[sc_F] = 'F';
    shiftedScancodeToASCII[sc_G] = 'G';
    shiftedScancodeToASCII[sc_H] = 'H';
    shiftedScancodeToASCII[sc_I] = 'I';
    shiftedScancodeToASCII[sc_J] = 'J';
    shiftedScancodeToASCII[sc_K] = 'K';
    shiftedScancodeToASCII[sc_L] = 'L';
    shiftedScancodeToASCII[sc_M] = 'M';
    shiftedScancodeToASCII[sc_N] = 'N';
    shiftedScancodeToASCII[sc_O] = 'O';
    shiftedScancodeToASCII[sc_P] = 'P';
    shiftedScancodeToASCII[sc_Q] = 'Q';
    shiftedScancodeToASCII[sc_R] = 'R';
    shiftedScancodeToASCII[sc_S] = 'S';
    shiftedScancodeToASCII[sc_T] = 'T';
    shiftedScancodeToASCII[sc_U] = 'U';
    shiftedScancodeToASCII[sc_V] = 'V';
    shiftedScancodeToASCII[sc_W] = 'W';
    shiftedScancodeToASCII[sc_X] = 'X';
    shiftedScancodeToASCII[sc_Y] = 'Y';
    shiftedScancodeToASCII[sc_Z] = 'Z';
    shiftedScancodeToASCII[sc_0] = ')';
    shiftedScancodeToASCII[sc_1] = '!';
    shiftedScancodeToASCII[sc_2] = '@';
    shiftedScancodeToASCII[sc_3] = '#';
    shiftedScancodeToASCII[sc_4] = '$';
    shiftedScancodeToASCII[sc_5] = '%';
    shiftedScancodeToASCII[sc_6] = '^';
    shiftedScancodeToASCII[sc_7] = '&';
    shiftedScancodeToASCII[sc_8] = '*';
    shiftedScancodeToASCII[sc_9] = '(';
    shiftedScancodeToASCII[sc_Comma] = '<';
    shiftedScancodeToASCII[sc_Period] = '>';
    shiftedScancodeToASCII[sc_Kpad_Star] = '*';
    shiftedScancodeToASCII[sc_Slash] = '?';
    shiftedScancodeToASCII[sc_SemiColon] = ':';
    shiftedScancodeToASCII[sc_Quote] = '\"';
    shiftedScancodeToASCII[sc_Tilde] = '~';
    shiftedScancodeToASCII[sc_BackSlash] = '|';
    shiftedScancodeToASCII[sc_OpenBracket] = '{';
    shiftedScancodeToASCII[sc_CloseBracket] = '}';
    shiftedScancodeToASCII[sc_Minus] = '_';
    shiftedScancodeToASCII[sc_Equals] = '+';
    shiftedScancodeToASCII[sc_Plus] = '+';
    shiftedScancodeToASCII[sc_kpad_Minus] = '-';
    shiftedScancodeToASCII[sc_kpad_Period] = '.';
    shiftedScancodeToASCII[sc_kpad_Plus] = '+';

    memset(extscanToSC, '\0', sizeof (extscanToSC));
    
    /* map extended keys to their Duke3D equivalents */
    extscanToSC[0x1C] = sc_kpad_Enter;
    extscanToSC[0x1D] = sc_RightControl;
    extscanToSC[0x35] = sc_kpad_Slash;
    extscanToSC[0x37] = sc_PrintScreen;
    extscanToSC[0x38] = sc_RightAlt;
    extscanToSC[0x47] = sc_Home;
    extscanToSC[0x48] = sc_UpArrow;
    extscanToSC[0x49] = sc_PgUp;
    extscanToSC[0x4B] = sc_LeftArrow;
    extscanToSC[0x4D] = sc_RightArrow;
    extscanToSC[0x4F] = sc_End;
    extscanToSC[0x50] = sc_DownArrow;
    extscanToSC[0x51] = sc_PgDn;
    extscanToSC[0x52] = sc_Insert;
    extscanToSC[0x53] = sc_Delete;
    
	KB_ClearKeysDown();
}

void KB_Shutdown( void )
{
	KB_ClearKeysDown();
}

