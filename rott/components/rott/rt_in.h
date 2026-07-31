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
//***************************************************************************
//
// Public header for RT_IN.C.
//
//***************************************************************************

#ifndef _rt_in_public
#define _rt_in_public

#include "develop.h"
#include "rottnet.h"

//***************************************************************************
//
// DEFINES
//
//***************************************************************************

#define MAXLETTERS   32


//***************************************************************************
//
// TYPEDEFS
//
//***************************************************************************

typedef  enum
{
	motion_Left  = -1,
	motion_Up    = -1,
   motion_None  = 0,
   motion_Right = 1,
   motion_Down  = 1
} Motion;


typedef  enum
{
   dir_North,
   dir_NorthEast,
   dir_East,
   dir_SouthEast,
   dir_South,
   dir_SouthWest,
   dir_West,
   dir_NorthWest,
   dir_None
} Direction;

typedef byte ScanCode;

typedef  enum
{
	ctrl_Keyboard,
	ctrl_Keyboard1 = ctrl_Keyboard, ctrl_Keyboard2,
	ctrl_Joystick,
	ctrl_Joystick1 = ctrl_Joystick, ctrl_Joystick2,
	ctrl_Mouse
} ControlType;

typedef  struct
{
	boolean     button0,
					button1,
					button2,
					button3;
	int         x,
					y;
	Motion      xaxis,
					yaxis;
	Direction   dir;
} CursorInfo;


typedef  CursorInfo  ControlInfo;


typedef  struct
{
   ScanCode button0,
            button1,
            upleft,
            up,
            upright,
            left,
            right,
            downleft,
            down,
            downright;
} KeyboardDef;


typedef struct
{
   word  joyMinX,joyMinY,
         threshMinX,threshMinY,
         threshMaxX,threshMaxY,
         joyMaxX,joyMaxY,
         joyMultXL,joyMultYL,
         joyMultXH,joyMultYH;
} JoystickDef;


typedef struct
{
   boolean messageon;
   boolean directed;
   boolean inmenu;
   int     remoteridicule;
   int     towho;
   int     textnum;
   int     length;
} ModemMessage;


//***************************************************************************
//
// GLOBALS
//
//***************************************************************************

extern boolean MousePresent;
extern boolean JoysPresent[MaxJoys];
extern boolean JoyPadPresent;
extern int     mouseadjustment;
extern int     threshold;

extern boolean  Paused;
extern volatile int LastScan;
/* extern KeyboardDef KbdDefs;
extern JoystickDef JoyDefs[];
extern ControlType Controls[MAXPLAYERS]; */

extern boolean  SpaceBallPresent;
extern boolean  CybermanPresent;
extern boolean  AssassinPresent;
extern char LastASCII;
extern volatile int LastScan;

extern byte Joy_xb,
            Joy_yb,
            Joy_xs,
            Joy_ys;
extern word Joy_x,
            Joy_y;

extern int LastLetter;
extern char LetterQueue[MAXLETTERS];
extern ModemMessage MSG;

extern const char ScanChars[128];

//***************************************************************************
//
// PROTOTYPES
//
//***************************************************************************

void INL_GetMouseDelta(int *x,int *y);
word IN_GetMouseButtons (void);
void IN_IgnoreMouseButtons( void );
boolean INL_StartMouse (void);
void INL_ShutMouse (void);
void IN_Startup(void);
void IN_Default (boolean gotit, ControlType in);
void IN_Shutdown (void);
void IN_SetKeyHook(void (*hook)());
void IN_ClearKeysDown (void);
void IN_ReadControl (int player, ControlInfo *info);
void IN_SetControlType (int player, ControlType type);
ScanCode IN_WaitForKey (void);
char IN_WaitForASCII (void);
void IN_StartAck (void);
boolean IN_CheckAck (void);
void IN_Ack (void);
boolean IN_UserInput (long delay);
void IN_GetJoyAbs (word joy, word *xp, word *yp);
void INL_GetJoyDelta (word joy, int *dx, int *dy);
word INL_GetJoyButtons (word joy);
//word IN_GetJoyButtonsDB (word joy);
void INL_SetJoyScale (word joy);
void IN_SetupJoy (word joy, word minx, word maxx, word miny, word maxy);
boolean INL_StartJoy (word joy);
void INL_ShutJoy (word joy);
byte IN_JoyButtons (void);
void IN_UpdateKeyboard (void);
void IN_ClearKeyboardQueue (void);
int IN_InputUpdateKeyboard (void);
void IN_PumpEvents (void);
void QueueLetterInput (void);

#endif
