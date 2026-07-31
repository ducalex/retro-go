/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef _isr_public
#define _isr_public

#include "keyb.h"

//***************************************************************************
//
//    ISR.C - Interrupt Service Routines (Keyboard, timer)
//
//***************************************************************************

//***************************************************************************
//
//    ISR Constants
//
//***************************************************************************

#define VBLCOUNTER   35


#define KEYQMAX      256
#define MAXKEYBOARDSCAN         128

#define scroll_lock  0
#define num_lock     1
#define caps_lock    2

extern volatile int *KeyboardQueue;
extern volatile int Keyhead;
extern volatile int Keytail;

extern volatile int *Keyboard;   // Keyboard status array
extern volatile int *Keystate;   // Keyboard state array

#ifdef DOS
extern volatile int ticcount;     // Current ticcount (usually 70Hz)
extern volatile int fasttics;

#define GetTicCount()	ticcount
#define GetFastTics()	fasttics

#define SetFastTics(a) {fasttics=a;}
#else
int GetTicCount (void);
int GetFastTics (void);

void SetFastTics(int);
#endif

extern int KeyboardStarted;

extern const int ASCIINames[];   // Ascii -> scan code conversion
extern const int ShiftNames[];   // Shifted Ascii->scancode conversion
extern volatile boolean PausePressed;  //Game paused variable
extern volatile boolean PanicPressed;  //Panic key variable

void I_StartupTimer (void);        // Start up timer isr
void I_SetTimer0(int speed);       // Set the timer to a given speed
void I_ShutdownTimer (void);       // Shutdown timer isr
void I_SetKeyboardLEDs( int which, boolean val ); // Turns LED's on or off
void I_StartupKeyboard (void);     // Startup Keyboard isr
void I_ShutdownKeyboard (void);    // Shutdown keyboard isr
void I_Delay ( int delay );
void ISR_SetTime(int settime);
void I_SendKeyboardData
   (
   int val
   );

#endif
