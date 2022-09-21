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

//***************************************************************************
//
// Public header for CONTROL.C.
//
//***************************************************************************

#ifndef _control_public
#define _control_public
#ifdef __cplusplus
extern "C" {
#endif


//***************************************************************************
//
// DEFINES
//
//***************************************************************************

#define MaxJoys             2
#define MAXGAMEBUTTONS      64

	/*
#define BUTTON(x) \
    ( \
    ((x)>31) ? \
    ((CONTROL_ButtonState2>>( (x) - 32) ) & 1) :\
    ((CONTROL_ButtonState1>> (x) ) & 1)          \
    )
	*/
	/*
#define BUTTONHELD(x) \
    ( \
    ((x)>31) ? \
    ((CONTROL_ButtonHeldState2>>((x)-32)) & 1) :\
    ((CONTROL_ButtonHeldState1>>(x)) & 1)\
    )
#define BUTTONJUSTPRESSED(x) \
    ( BUTTON( x ) && !BUTTONHELD( x ) )
#define BUTTONRELEASED(x) \
    ( !BUTTON( x ) && BUTTONHELD( x ) )
#define BUTTONSTATECHANGED(x) \
    ( BUTTON( x ) != BUTTONHELD( x ) )
	*/

//***************************************************************************
//
// TYPEDEFS
//
//***************************************************************************
typedef enum
   {
   axis_up,
   axis_down,
   axis_left,
   axis_right
   } axisdirection;

typedef enum
   {
   analog_turning=0,
   analog_strafing=1,
   analog_lookingupanddown=2,
   analog_elevation=3,
   analog_rolling=4,
   analog_moving=5,
   analog_maxtype
   } analogcontrol;

typedef enum
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
   } direction;

typedef struct
   {
   boolean   button0;
   boolean   button1;
   direction dir;
   } UserInput;


typedef struct
   {
   fixed     dx;
   fixed     dy;
   fixed     dz;
   fixed     dyaw;
   fixed     dpitch;
   fixed     droll;
   } ControlInfo;

/*
typedef struct
   {
   float     dx;
   float     dy;
   float     dz;
   float     dyaw;
   float     dpitch;
   float     droll;
   } ControlInfo;
*/

typedef enum
   {
   controltype_keyboard,
   controltype_keyboardandmouse,
   controltype_keyboardandjoystick,
   controltype_keyboardandexternal,
   controltype_keyboardandgamepad,
   controltype_keyboardandflightstick,
   controltype_keyboardandthrustmaster,
   controltype_joystickandmouse
   } controltype;


//***************************************************************************
//
// GLOBALS
//
//***************************************************************************

extern uint32   CONTROL_RudderEnabled;
extern boolean  CONTROL_MousePresent;
extern boolean  CONTROL_JoysPresent[ MaxJoys ];
extern boolean  CONTROL_MouseEnabled;
extern boolean  CONTROL_JoystickEnabled;
extern byte     CONTROL_JoystickPort;
extern uint32   CONTROL_MouseButtonState1;
extern uint32   CONTROL_MouseButtonState2;
//extern uint32   CONTROL_ButtonHeldState1;
//extern uint32   CONTROL_ButtonHeldState2;
extern uint32   CONTROL_JoyButtonState;

//***************************************************************************
//
// PROTOTYPES
//
//***************************************************************************
struct _KeyMapping
{
	boolean key_active;
	kb_scancode key1;
	kb_scancode key2;
	
	/* other mappings go here */
} KeyMapping[MAXGAMEBUTTONS];

int32 MouseMapping[MAXMOUSEBUTTONS];
int32 MouseDigitalAxeMapping[MAXMOUSEAXES][2];  // [axesX/Y][directionLeft/Right or directionUp/Down]


int  ACTION(int i);
int  RESET_ACTION(int i);
void RESBUTTON(int i);
void CONTROL_MapKey( int32 which, kb_scancode key1, kb_scancode key2 );
void CONTROL_MapButton
        (
        int32 whichfunction,
        int32 whichbutton,
        boolean clicked_or_doubleclicked
        );
void CONTROL_MapJoyButton(int32 whichfunction, int32 whichbutton, boolean doubleclicked);
void CONTROL_MapJoyHat(int32 whichfunction, int32 whichhat, int32 whichvalue);
void CONTROL_DefineFlag( int32 which, boolean toggle );
boolean CONTROL_FlagActive( int32 which );
void CONTROL_ClearAssignments( void );
void CONTROL_GetUserInput( UserInput *info );
void CONTROL_GetInput( ControlInfo *info );
void CONTROL_ClearAction( int32 whichbutton );
void CONTROL_ClearUserInput( UserInput *info );
void CONTROL_WaitRelease( void );
void CONTROL_Ack( void );
void CONTROL_CenterJoystick
   (
   void ( *CenterCenter )( void ),
   void ( *UpperLeft )( void ),
   void ( *LowerRight )( void ),
   void ( *CenterThrottle )( void ),
   void ( *CenterRudder )( void )
   );
int32 CONTROL_GetMouseSensitivity_X( void );
void CONTROL_SetMouseSensitivity_X( int32 newsensitivity );
int32 CONTROL_GetMouseSensitivity_Y( void );
void CONTROL_SetMouseSensitivity_Y( int32 newsensitivity );
void CONTROL_Startup
   (
   controltype which,
   int32 ( *TimeFunction )( void ),
   int32 ticspersecond
   );
void CONTROL_Shutdown( void );

void CONTROL_MapAnalogAxis
   (
   int32 whichaxis,
   int32 whichanalog
   );

void CONTROL_MapDigitalAxis
   (
   int32 whichaxis,
   int32 whichfunction,
   int32 direction
   );
void CONTROL_SetAnalogAxisScale
   (
   int32 whichaxis,
   float axisscale
   );
void CONTROL_SetAnalogAxisDeadzone
   (
   int32 whichaxis,
   int32 axisdeadzone
   );
int32 CONTROL_FilterDeadzone
   (
   int32 axisvalue,
   int32 axisdeadzone
   );
int32 CONTROL_GetFilteredAxisValue(int32 axis);
void CONTROL_PrintAxes( void );

void CONTROL_UpdateKeyboardState(int key, int pressed);

#ifdef __cplusplus
};
#endif
#endif

