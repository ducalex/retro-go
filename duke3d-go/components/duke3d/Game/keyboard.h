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

#ifndef _keyboard_public
#define _keyboard_public
#ifdef __cplusplus
extern "C" {
#endif

/*
=============================================================================

                                    DEFINES

=============================================================================
*/

typedef uint8_t kb_scancode;

#define  sc_None         0
#define  sc_Bad          0xff
#define  sc_Comma        0x33
#define  sc_Period       0x34
#define  sc_Return       0x1c
#define  sc_Enter        sc_Return
#define  sc_Escape       0x01
#define  sc_Space        0x39
#define  sc_BackSpace    0x0e
#define  sc_Tab          0x0f
#define  sc_LeftAlt      0x38
#define  sc_LeftControl  0x1d
#define  sc_CapsLock     0x3a
#define  sc_LeftShift    0x2a
#define  sc_RightShift   0x36
#define  sc_F1           0x3b
#define  sc_F2           0x3c
#define  sc_F3           0x3d
#define  sc_F4           0x3e
#define  sc_F5           0x3f
#define  sc_F6           0x40
#define  sc_F7           0x41
#define  sc_F8           0x42
#define  sc_F9           0x43
#define  sc_F10          0x44
#define  sc_F11          0x57
#define  sc_F12          0x58
#define  sc_Kpad_Star    0x37
#define  sc_Pause        0x59
#define  sc_ScrollLock   0x46
#define  sc_NumLock      0x45
#define  sc_Slash        0x35
#define  sc_SemiColon    0x27
#define  sc_Quote        0x28
#define  sc_Tilde        0x29
#define  sc_BackSlash    0x2b

#define  sc_OpenBracket  0x1a
#define  sc_CloseBracket 0x1b

#define  sc_1            0x02
#define  sc_2            0x03
#define  sc_3            0x04
#define  sc_4            0x05
#define  sc_5            0x06
#define  sc_6            0x07
#define  sc_7            0x08
#define  sc_8            0x09
#define  sc_9            0x0a
#define  sc_0            0x0b
#define  sc_Minus        0x0c
#define  sc_Equals       0x0d
#define  sc_Plus         0x0d

#define  sc_kpad_1       0x4f
#define  sc_kpad_2       0x50
#define  sc_kpad_3       0x51
#define  sc_kpad_4       0x4b
#define  sc_kpad_5       0x4c
#define  sc_kpad_6       0x4d
#define  sc_kpad_7       0x47
#define  sc_kpad_8       0x48
#define  sc_kpad_9       0x49
#define  sc_kpad_0       0x52
#define  sc_kpad_Minus   0x4a
#define  sc_kpad_Plus    0x4e
#define  sc_kpad_Period  0x53

#define  sc_A            0x1e
#define  sc_B            0x30
#define  sc_C            0x2e
#define  sc_D            0x20
#define  sc_E            0x12
#define  sc_F            0x21
#define  sc_G            0x22
#define  sc_H            0x23
#define  sc_I            0x17
#define  sc_J            0x24
#define  sc_K            0x25
#define  sc_L            0x26
#define  sc_M            0x32
#define  sc_N            0x31
#define  sc_O            0x18
#define  sc_P            0x19
#define  sc_Q            0x10
#define  sc_R            0x13
#define  sc_S            0x1f
#define  sc_T            0x14
#define  sc_U            0x16
#define  sc_V            0x2f
#define  sc_W            0x11
#define  sc_X            0x2d
#define  sc_Y            0x15
#define  sc_Z            0x2c

// Extended scan codes

#define  sc_UpArrow      0x5a
#define  sc_DownArrow    0x6a
#define  sc_LeftArrow    0x6b
#define  sc_RightArrow   0x6c
#define  sc_Insert       0x5e
#define  sc_Delete       0x5f
#define  sc_Home         0x61
#define  sc_End          0x62
#define  sc_PgUp         0x63
#define  sc_PgDn         0x64
#define  sc_RightAlt     0x65
#define  sc_RightControl 0x66
#define  sc_kpad_Slash   0x67
#define  sc_kpad_Enter   0x68
#define  sc_PrintScreen  0x69
#define  sc_LastScanCode 0x6e

// Ascii scan codes

#define  asc_Enter       13
#define  asc_Escape      27
#define  asc_BackSpace   8
#define  asc_Tab         9
#define  asc_Space       32

#define MAXKEYBOARDSCAN  128


/*
=============================================================================

                               GLOBAL VARIABLES

=============================================================================
*/

extern uint8_t  KB_KeyDown[ MAXKEYBOARDSCAN ];   // Keyboard state array
extern kb_scancode KB_LastScan;


/*
=============================================================================

                                    MACROS

=============================================================================
*/

#define KB_GetLastScanCode()    ( KB_LastScan )

#define KB_SetLastScanCode( scancode ) { KB_LastScan = ( scancode ); }

#define KB_ClearLastScanCode() { KB_SetLastScanCode( sc_None ); }

#define KB_KeyPressed( scan )  ( KB_KeyDown[ ( scan ) ] != 0 )

#define KB_ClearKeyDown( scan ) { KB_KeyDown[ ( scan ) ] = false; }

#define KB_SetKeyDown( scan ) { KB_KeyDown[ ( scan ) ] = true; }
/*
=============================================================================

                              FUNCTION PROTOTYPES

=============================================================================
*/

void KB_KeyEvent( int scancode, int keypressed );  // Interprets scancodes
int KB_KeyWaiting( void );         // Checks if a character is waiting in the keyboard queue
uint8_t     KB_Getch( void );              // Gets the next keypress
void    KB_Addch( uint8_t  ch );           // Adds key to end of queue
void    KB_FlushKeyboardQueue( void ); // Empties the keyboard queue of all waiting characters.
void    KB_ClearKeysDown( void );      // Clears all keys down flags.
char  *  KB_ScanCodeToString( kb_scancode scancode ); // convert scancode into a string
kb_scancode KB_StringToScanCode( char  * string );  // convert a string into a scancode
void    KB_TurnKeypadOn( void );       // turn the keypad on
void    KB_TurnKeypadOff( void );      // turn the keypad off
int KB_KeypadActive( void );       // check whether keypad is active
void    KB_Startup( void );
void    KB_Shutdown( void );

#ifdef __cplusplus
};
#endif
#endif
