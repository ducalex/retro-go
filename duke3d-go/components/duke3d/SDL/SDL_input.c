#include "SDL_input.h"

void SDL_SetModState(SDL_Keymod modstate)
{

}

SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode)
{
    SDL_GrabMode grab = SDL_GRAB_OFF;
    return grab;
}

int SDL_ShowCursor(int toggle)
{
    return 0;
}


/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* This is the joystick API for Simple DirectMedia Layer */

#include "SDL.h"


#ifdef __WIN32__
/* Needed for checking for input remapping programs */
#include "../core/windows/SDL_windows.h"

#undef UNICODE          /* We want ASCII functions */
#include <tlhelp32.h>
#endif

static SDL_bool SDL_joystick_allows_background_events = SDL_FALSE;
static SDL_Joystick *SDL_joysticks = NULL;
static SDL_bool SDL_updating_joystick = SDL_FALSE;
//static SDL_mutex *SDL_joystick_lock = NULL; /* This needs to support recursive locks */
//static SDL_atomic_t SDL_next_joystick_instance_id;

void
SDL_LockJoysticks(void)
{
}

void
SDL_UnlockJoysticks(void)
{
}


static void SDLCALL
SDL_JoystickAllowBackgroundEventsChanged(void *userdata, const char *name, const char *oldValue, const char *hint)
{
}

int
SDL_JoystickInit(void)
{
    int status = 0;
    return status;
}

/*
 * Count the number of joysticks attached to the system
 */
int
SDL_NumJoysticks(void)
{
    int total_joysticks = 0;
    return total_joysticks;
}

/*
 * Return the next available joystick instance ID
 * This may be called by drivers from multiple threads, unprotected by any locks
 */
SDL_JoystickID SDL_GetNextJoystickInstanceID()
{
    return 0;
}

/*
 * Get the driver and device index for an API device index
 * This should be called while the joystick lock is held, to prevent another thread from updating the list
 */
SDL_bool
SDL_GetDriverAndJoystickIndex(int device_index, void **driver, int *driver_index)
{
    return SDL_FALSE;
}

/*
 * Perform any needed fixups for joystick names
 */
static const char *
SDL_FixupJoystickName(const char *name)
{
    return name;
}


/*
 * Get the implementation dependent name of a joystick
 */
const char *
SDL_JoystickNameForIndex(int device_index)
{
    const char *name = NULL;
   return name;
}

int
SDL_JoystickGetDevicePlayerIndex(int device_index)
{
    int player_index = -1;
    return player_index;
}

/*
 * Return true if this joystick is known to have all axes centered at zero
 * This isn't generally needed unless the joystick never generates an initial axis value near zero,
 * e.g. it's emulating axes with digital buttons
 */
static SDL_bool
SDL_JoystickAxesCenteredAtZero(SDL_Joystick *joystick)
{
    return SDL_FALSE;
}

/*
 * Open a joystick for use - the index passed as an argument refers to
 * the N'th joystick on the system.  This index is the value which will
 * identify this joystick in future joystick events.
 *
 * This function returns a joystick identifier, or NULL if an error occurred.
 */
SDL_Joystick *
SDL_JoystickOpen(int device_index)
{
    SDL_Joystick *joystick = NULL;
    return joystick;
}


/*
 * Checks to make sure the joystick is valid.
 */
int
SDL_PrivateJoystickValid(SDL_Joystick * joystick)
{
    int valid = 0;
    return valid;
}

/*
 * Get the number of multi-dimensional axis controls on a joystick
 */
int
SDL_JoystickNumAxes(SDL_Joystick * joystick)
{
    return -1;
}

/*
 * Get the number of hats on a joystick
 */
int
SDL_JoystickNumHats(SDL_Joystick * joystick)
{
    return -1;
}

/*
 * Get the number of trackballs on a joystick
 */
int
SDL_JoystickNumBalls(SDL_Joystick * joystick)
{
    return -1;
}

/*
 * Get the number of buttons on a joystick
 */
int
SDL_JoystickNumButtons(SDL_Joystick * joystick)
{
    return -1;
}

/*
 * Get the current state of an axis control on a joystick
 */
Sint16
SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis)
{
    Sint16 state = 0;
    return state;
}

/*
 * Get the initial state of an axis control on a joystick
 */
SDL_bool
SDL_JoystickGetAxisInitialState(SDL_Joystick * joystick, int axis, Sint16 *state)
{
    return SDL_FALSE;
}

/*
 * Get the current state of a hat on a joystick
 */
Uint8
SDL_JoystickGetHat(SDL_Joystick * joystick, int hat)
{
    Uint8 state = 0;
    return state;
}

/*
 * Get the ball axis change since the last poll
 */
int
SDL_JoystickGetBall(SDL_Joystick * joystick, int ball, int *dx, int *dy)
{
    int retval = 0;
    return retval;
}

/*
 * Get the current state of a button on a joystick
 */
Uint8
SDL_JoystickGetButton(SDL_Joystick * joystick, int button)
{
    Uint8 state = 0;
    return state;
}

/*
 * Return if the joystick in question is currently attached to the system,
 *  \return SDL_FALSE if not plugged in, SDL_TRUE if still present.
 */
SDL_bool
SDL_JoystickGetAttached(SDL_Joystick * joystick)
{
    return SDL_FALSE;
}

/*
 * Get the instance id for this opened joystick
 */
SDL_JoystickID
SDL_JoystickInstanceID(SDL_Joystick * joystick)
{
        return -1;
}

/*
 * Find the SDL_Joystick that owns this instance id
 */
SDL_Joystick *
SDL_JoystickFromInstanceID(SDL_JoystickID joyid)
{
    SDL_Joystick *joystick = NULL;
    return joystick;
}

/*
 * Get the friendly name of this joystick
 */
const char * SDL_JoystickName(SDL_Joystick * joystick)
{
    return NULL;
}

int SDL_JoystickGetPlayerIndex(SDL_Joystick * joystick)
{
    return -1;
}

int SDL_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
    return -1;
}

/*
 * Close a joystick previously opened with SDL_JoystickOpen()
 */
void SDL_JoystickClose(SDL_Joystick * joystick)
{
}

void SDL_JoystickQuit(void)
{
}


static SDL_bool
SDL_PrivateJoystickShouldIgnoreEvent()
{
    return SDL_TRUE;
}

/* These are global for SDL_sysjoystick.c and SDL_events.c */

void SDL_PrivateJoystickAdded(SDL_JoystickID device_instance)
{
}

static void UpdateEventsForDeviceRemoval()
{
}

void SDL_PrivateJoystickRemoved(SDL_JoystickID device_instance)
{
}

int
SDL_PrivateJoystickAxis(SDL_Joystick * joystick, Uint8 axis, Sint16 value)
{
    int posted = 0;
    return posted;
}

int SDL_PrivateJoystickHat(SDL_Joystick * joystick, Uint8 hat, Uint8 value)
{
    int posted = 0;
    return posted;
}

int SDL_PrivateJoystickBall(SDL_Joystick * joystick, Uint8 ball,
                        Sint16 xrel, Sint16 yrel)
{
    int posted = 0;
    return posted;
}

int SDL_PrivateJoystickButton(SDL_Joystick * joystick, Uint8 button, Uint8 state)
{
    int posted = 0;
    return posted;
}

void SDL_JoystickUpdate(void)
{
}

int SDL_JoystickEventState(int state)
{
    return state;
}

void SDL_GetJoystickGUIDInfo(SDL_JoystickGUID guid, Uint16 *vendor, Uint16 *product, Uint16 *version)
{
}

SDL_bool SDL_IsJoystickPS4(Uint16 vendor, Uint16 product)
{
    return SDL_FALSE;
}

SDL_bool SDL_IsJoystickNintendoSwitchPro(Uint16 vendor, Uint16 product)
{
    return SDL_FALSE;
}

SDL_bool SDL_IsJoystickSteamController(Uint16 vendor, Uint16 product)
{
    return SDL_FALSE;
}

SDL_bool SDL_IsJoystickXbox360(Uint16 vendor, Uint16 product)
{
    /* Filter out some bogus values here */
    if (vendor == 0x0000 && product == 0x0000) {
        return SDL_FALSE;
    }
    if (vendor == 0x0001 && product == 0x0001) {
        return SDL_FALSE;
    }
    return SDL_FALSE;
}

SDL_bool SDL_IsJoystickXboxOne(Uint16 vendor, Uint16 product)
{
    return SDL_FALSE;
}

SDL_bool SDL_IsJoystickXInput(SDL_JoystickGUID guid)
{
    return (guid.data[14] == 'x') ? SDL_TRUE : SDL_FALSE;
}

SDL_bool SDL_IsJoystickHIDAPI(SDL_JoystickGUID guid)
{
    return (guid.data[14] == 'h') ? SDL_TRUE : SDL_FALSE;
}

static SDL_bool SDL_IsJoystickProductWheel(Uint32 vidpid)
{
    return SDL_FALSE;
}

static SDL_bool SDL_IsJoystickProductFlightStick(Uint32 vidpid)
{
    return SDL_FALSE;
}

static SDL_bool SDL_IsJoystickProductThrottle(Uint32 vidpid)
{
    return SDL_FALSE;
}

static SDL_JoystickType SDL_GetJoystickGUIDType(SDL_JoystickGUID guid)
{
    return SDL_JOYSTICK_TYPE_UNKNOWN;
}

static SDL_bool SDL_IsPS4RemapperRunning(void)
{
    return SDL_FALSE;
}

SDL_bool SDL_ShouldIgnoreJoystick(const char *name, SDL_JoystickGUID guid)
{
    return SDL_TRUE;
}

/* return the guid for this index */
SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickGUID guid;
    return guid;
}

Uint16 SDL_JoystickGetDeviceVendor(int device_index)
{
    Uint16 vendor = 0;
    return vendor;
}

Uint16 SDL_JoystickGetDeviceProduct(int device_index)
{
    Uint16 product = 0;
    return product;
}

Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
{
    Uint16 version = 0;
    return version;
}

SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
{
    SDL_JoystickType type = SDL_JOYSTICK_TYPE_UNKNOWN;
    return type;
}

SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
{
    SDL_JoystickID instance_id = -1;
    return instance_id;
}

int SDL_JoystickGetDeviceIndexFromInstanceID(SDL_JoystickID instance_id)
{
    int device_index = -1;
    return device_index;
}

SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick)
{
    SDL_JoystickGUID guid;
    return guid;
}

Uint16 SDL_JoystickGetVendor(SDL_Joystick * joystick)
{
    Uint16 vendor = 0;
    return vendor;
}

Uint16 SDL_JoystickGetProduct(SDL_Joystick * joystick)
{
    Uint16 product = 0;
    return product;
}

Uint16 SDL_JoystickGetProductVersion(SDL_Joystick * joystick)
{
    Uint16 version = 0;
    return version;
}

SDL_JoystickType SDL_JoystickGetType(SDL_Joystick * joystick)
{
    SDL_JoystickType type = SDL_JOYSTICK_TYPE_UNKNOWN;
    return type;
}

/* convert the guid to a printable string */
void SDL_JoystickGetGUIDString(SDL_JoystickGUID guid, char *pszGUID, int cbGUID)
{
}

/*-----------------------------------------------------------------------------
 * Purpose: Returns the 4 bit nibble for a hex character
 * Input  : c -
 * Output : unsigned char
 *-----------------------------------------------------------------------------*/
static unsigned char nibble(char c)
{
    if ((c >= '0') && (c <= '9')) {
        return (unsigned char)(c - '0');
    }

    if ((c >= 'A') && (c <= 'F')) {
        return (unsigned char)(c - 'A' + 0x0a);
    }

    if ((c >= 'a') && (c <= 'f')) {
        return (unsigned char)(c - 'a' + 0x0a);
    }

    /* received an invalid character, and no real way to return an error */
    /* AssertMsg1(false, "Q_nibble invalid hex character '%c' ", c); */
    return 0;
}

/* convert the string version of a joystick guid to the struct */
SDL_JoystickGUID SDL_JoystickGetGUIDFromString(const char *pchGUID)
{
    SDL_JoystickGUID guid;
    return guid;
}

/* update the power level for this joystick */
void SDL_PrivateJoystickBatteryLevel(SDL_Joystick * joystick, SDL_JoystickPowerLevel ePowerLevel)
{
}

/* return its power level */
SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick)
{
    SDL_JoystickPowerLevel pl = SDL_JOYSTICK_POWER_UNKNOWN;
    return pl;
}

