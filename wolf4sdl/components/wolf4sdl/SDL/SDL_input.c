#include "SDL_input.h"
#include <string.h>
#include <stdlib.h>

struct _SDL_Joystick {
    int dummy;
};

void SDL_SetModState(SDL_Keymod modstate)
{
}

SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode)
{
    return SDL_GRAB_OFF;
}

void SDL_WarpMouse(Uint16 x, Uint16 y)
{
}

int SDL_JoystickNumAxes(SDL_Joystick * joystick)
{
    return 0;
}

int SDL_JoystickNumButtons(SDL_Joystick * joystick)
{
    return 10;
}

int SDL_JoystickNumBalls(SDL_Joystick * joystick)
{
    return 0;
}

int SDL_JoystickNumHats(SDL_Joystick * joystick)
{
    return 0;
}

int SDL_NumJoysticks(void)
{
    return 1;
}

const char *SDL_JoystickName(SDL_Joystick * joystick)
{
    return "Retro-Go Gamepad";
}

SDL_Joystick *SDL_JoystickOpen(int device_index)
{
    static SDL_Joystick joy;
    return &joy;
}

int SDL_JoystickOpened(int device_index)
{
    return 1;
}

int SDL_JoystickIndex(SDL_Joystick * joystick)
{
    return 0;
}

SDL_bool SDL_JoystickGetAttached(SDL_Joystick * joystick)
{
    return SDL_TRUE;
}

SDL_JoystickID SDL_JoystickInstanceID(SDL_Joystick * joystick)
{
    return 0;
}

SDL_JoystickGUID SDL_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickGUID guid;
    memset(&guid, 0, sizeof(guid));
    return guid;
}

Uint16 SDL_JoystickGetDeviceVendor(int device_index)
{
    return 0;
}

Uint16 SDL_JoystickGetDeviceProduct(int device_index)
{
    return 0;
}

Uint16 SDL_JoystickGetDeviceProductVersion(int device_index)
{
    return 0;
}

SDL_JoystickType SDL_JoystickGetDeviceType(int device_index)
{
    return SDL_JOYSTICK_TYPE_GAMECONTROLLER;
}

SDL_JoystickID SDL_JoystickGetDeviceInstanceID(int device_index)
{
    return 0;
}

void SDL_JoystickUpdate(void)
{
}

int SDL_JoystickEventState(int state)
{
    return state;
}

Sint16 SDL_JoystickGetAxis(SDL_Joystick * joystick, int axis)
{
    return 0;
}

Uint8 SDL_JoystickGetHat(SDL_Joystick * joystick, int hat)
{
    return SDL_HAT_CENTERED;
}

Uint8 SDL_JoystickGetButton(SDL_Joystick * joystick, int button)
{
    return 0;
}

int SDL_JoystickGetBall(SDL_Joystick * joystick, int ball, int *dx, int *dy)
{
    return 0;
}

void SDL_JoystickClose(SDL_Joystick * joystick)
{
}

SDL_JoystickGUID SDL_JoystickGetGUID(SDL_Joystick * joystick)
{
    SDL_JoystickGUID guid;
    memset(&guid, 0, sizeof(guid));
    return guid;
}

Uint16 SDL_JoystickGetVendor(SDL_Joystick * joystick)
{
    return 0;
}

Uint16 SDL_JoystickGetProduct(SDL_Joystick * joystick)
{
    return 0;
}

Uint16 SDL_JoystickGetProductVersion(SDL_Joystick * joystick)
{
    return 0;
}

SDL_JoystickType SDL_JoystickGetType(SDL_Joystick * joystick)
{
    return SDL_JOYSTICK_TYPE_GAMECONTROLLER;
}

SDL_JoystickGUID SDL_JoystickGetGUIDFromString(const char *pchGUID)
{
    SDL_JoystickGUID guid;
    memset(&guid, 0, sizeof(guid));
    return guid;
}

void SDL_PrivateJoystickBatteryLevel(SDL_Joystick * joystick, SDL_JoystickPowerLevel ePowerLevel)
{
}

SDL_JoystickPowerLevel SDL_JoystickCurrentPowerLevel(SDL_Joystick * joystick)
{
    return SDL_JOYSTICK_POWER_UNKNOWN;
}

SDL_Keymod SDL_GetModState(void)
{
    return (SDL_Keymod)0;
}
