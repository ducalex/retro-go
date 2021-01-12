// input.cpp: implementation of the input class.
//
//////////////////////////////////////////////////////////////////////

//Flavor - Convert from DirectInput to SDL/GP2X

#ifndef __GP32__
#include "StdAfx.h"
#endif
#include "main.h"
#include "input.h"
#include "memory.h"
///#include "menu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// address where the state of the input device(s) is stored
//unsigned char	*InputByte = get_address(0x00006F82);
unsigned char	ngpInputState = 0;
unsigned char	*InputByte = &ngpInputState;

#ifndef TARGET_PSP
// extern SDL_Joystick *joystick;
#endif
#ifdef ZOOM_SUPPORT
extern int zoom;
#endif

#ifdef __GP32__
#define DIK_UP BUTTON_UP
#define DIK_DOWN BUTTON_DOWN
#define DIK_LEFT BUTTON_LEFT
#define DIK_RIGHT BUTTON_RIGHT
#define DIK_SPACE BUTTON_A
#define DIK_N BUTTON_B
#define DIK_S BUTTON_SELECT
#define DIK_O BUTTON_START
#else
#define DIK_UP SDLK_UP
#define DIK_DOWN SDLK_DOWN
#define DIK_LEFT SDLK_LEFT
#define DIK_RIGHT SDLK_RIGHT
#define DIK_SPACE SDLK_a
#define DIK_N SDLK_b
#define DIK_S SDLK_ESCAPE
#define DIK_O SDLK_SPACE
#endif

#ifdef TARGET_OD

// SDL_Event event;

const unsigned char keyCoresp[7] = {
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
};


#endif

#define DOWN(x)	keystates[x]
#ifdef __GP32__
u16 keystates = 0;
#else
unsigned char *keystates = NULL;
#endif

struct joy_range
{
    int minx, maxx, miny, maxy;
} range;

#ifdef __GP32__
void InitInput()
#else
BOOL InitInput(HWND hwnd)
#endif
{
#ifdef TARGET_OD
	// m_sysInfo[NGP].InputKeys[KEY_UP]		= SDLK_UP;
	// m_sysInfo[NGP].InputKeys[KEY_DOWN]		= SDLK_DOWN;
	// m_sysInfo[NGP].InputKeys[KEY_LEFT]		= SDLK_LEFT;
	// m_sysInfo[NGP].InputKeys[KEY_RIGHT]		= SDLK_RIGHT;
	// m_sysInfo[NGP].InputKeys[KEY_BUTTON_A]	= SDLK_LCTRL;
	// m_sysInfo[NGP].InputKeys[KEY_BUTTON_B]	= SDLK_LALT;
	// m_sysInfo[NGP].InputKeys[KEY_SELECT]	= SDLK_ESCAPE;	// Option button
	// m_sysInfo[NGP].InputKeys[KEY_START]		= SDLK_RETURN;	// Option button
	// m_sysInfo[NGP].InputKeys[KEY_BUTTON_X]	= SDLK_SPACE;
	// m_sysInfo[NGP].InputKeys[KEY_BUTTON_Y]	= SDLK_LSHIFT;
	// m_sysInfo[NGP].InputKeys[KEY_BUTTON_R]	= SDLK_BACKSPACE;
	// m_sysInfo[NGP].InputKeys[KEY_BUTTON_L]	= SDLK_TAB;
	// m_sysInfo[NGPC].InputKeys[KEY_UP]		= SDLK_UP;
	// m_sysInfo[NGPC].InputKeys[KEY_DOWN]		= SDLK_DOWN;
	// m_sysInfo[NGPC].InputKeys[KEY_LEFT]		= SDLK_LEFT;
	// m_sysInfo[NGPC].InputKeys[KEY_RIGHT]		= SDLK_RIGHT;
	// m_sysInfo[NGPC].InputKeys[KEY_BUTTON_A]	= SDLK_LCTRL;
	// m_sysInfo[NGPC].InputKeys[KEY_BUTTON_B]	= SDLK_LALT;
	// m_sysInfo[NGPC].InputKeys[KEY_SELECT]	= SDLK_ESCAPE;	// Option button
	// m_sysInfo[NGPC].InputKeys[KEY_START]		= SDLK_RETURN;	// Option button
	// m_sysInfo[NGPC].InputKeys[KEY_BUTTON_X]	= SDLK_SPACE;
	// m_sysInfo[NGPC].InputKeys[KEY_BUTTON_Y]	= SDLK_LSHIFT;
	// m_sysInfo[NGPC].InputKeys[KEY_BUTTON_R]	= SDLK_BACKSPACE;
	// m_sysInfo[NGPC].InputKeys[KEY_BUTTON_L]	= SDLK_TAB;
#else
#ifndef TARGET_PSP
#ifndef __GP32__
    keystates = PSP_GetKeyStateArray(NULL);
#endif
	// setup standard values for input
	// NGP/NGPC:
	m_sysInfo[NGP].InputKeys[KEY_UP]			= DIK_UP;
	m_sysInfo[NGP].InputKeys[KEY_DOWN]		= DIK_DOWN;
	m_sysInfo[NGP].InputKeys[KEY_LEFT]		= DIK_LEFT;
	m_sysInfo[NGP].InputKeys[KEY_RIGHT]		= DIK_RIGHT;
	m_sysInfo[NGP].InputKeys[KEY_BUTTON_A]	= DIK_SPACE;
	m_sysInfo[NGP].InputKeys[KEY_BUTTON_B]	= DIK_N;
	m_sysInfo[NGP].InputKeys[KEY_SELECT]		= DIK_O;	// Option button
	m_sysInfo[NGPC].InputKeys[KEY_UP]			= DIK_UP;
	m_sysInfo[NGPC].InputKeys[KEY_DOWN]		= DIK_DOWN;
	m_sysInfo[NGPC].InputKeys[KEY_LEFT]		= DIK_LEFT;
	m_sysInfo[NGPC].InputKeys[KEY_RIGHT]		= DIK_RIGHT;
	m_sysInfo[NGPC].InputKeys[KEY_BUTTON_A]	= DIK_SPACE;
	m_sysInfo[NGPC].InputKeys[KEY_BUTTON_B]	= DIK_N;
	m_sysInfo[NGPC].InputKeys[KEY_SELECT]		= DIK_O;	// Option button

    range.minx = -32767;//INT_MIN;
    range.maxx = 32767;//INT_MAX;
    range.miny = -32767;//INT_MIN;
    range.maxy = 32767;//INT_MAX;
#endif /* TARGET_PSP */
#endif /* TARGET_OD */
    return TRUE;
}

#ifdef __GP32__
extern "C"
{
	int gp_getButton();
	void clearScreen();
}
int zoom=0,zoomy=16;
#endif

#ifndef TARGET_PSP
void UpdateInputState()
{
#ifdef __GP32__
	int key = gp_getButton();

	if ((key & BUTTON_R) && (key & BUTTON_L))
    {
        m_bIsActive = FALSE;//Flavor exit emulation
        return;
    }

	if (key & BUTTON_R) {
		zoom ^= 1;
		while ((key=gp_getButton())&BUTTON_R);
		if (!zoom)
			clearScreen();
	}

	if (key & BUTTON_L) {
		if (key & BUTTON_DOWN) {
			if (zoomy<32)
				zoomy+=1;
			return;
		}
		if (key & BUTTON_UP) {
			if (zoomy>0)
				zoomy-=1;
			return;
		}
	}
	if(key & BUTTON_DOWN)
	    *InputByte = 0x02;
	else if(key & BUTTON_UP)
	    *InputByte = 0x01;
	else
	    *InputByte = 0;

	if(key & BUTTON_RIGHT)
	    *InputByte |= 0x08;
	else if(key & BUTTON_LEFT)
	    *InputByte |= 0x04;

    if (key & BUTTON_A)
        *InputByte|= 0x10;
    if (key & BUTTON_B)
        *InputByte|= 0x20;
    if (key & BUTTON_SELECT)
        *InputByte|= 0x40;
#else
    //SDL_Event event;

#ifdef TARGET_PSP
    while(SDL_PollEvent(&event))
    {
    }

    if (SDL_JoystickGetButton(joystick, PSP_BUTTON_R) && SDL_JoystickGetButton(joystick, PSP_BUTTON_L))
    {
        m_bIsActive = FALSE;//Flavor exit emulation
        return;
    }

    int x_axis = SDL_JoystickGetAxis (joystick, 0);
    int y_axis = SDL_JoystickGetAxis (joystick, 1);

/*    if (x_axis < range.minx) range.minx = x_axis;
    if (y_axis < range.miny) range.miny = y_axis;
    if (x_axis > range.maxx) range.maxx = x_axis;
    if (y_axis > range.maxy) range.maxy = y_axis;

*/

    *InputByte = 0;
	if(SDL_JoystickGetButton(joystick, PSP_BUTTON_DOWN) || (y_axis > (range.miny + 2*(range.maxy - range.miny)/3)))
	{
	    *InputByte |= 0x02;
	}
	if(SDL_JoystickGetButton(joystick, PSP_BUTTON_UP) || (y_axis < (range.miny + (range.maxy - range.miny)/3)))
	{
	    *InputByte |= 0x01;
	}
	if(SDL_JoystickGetButton(joystick, PSP_BUTTON_RIGHT) || (x_axis > (range.minx + 2*(range.maxx - range.minx)/3)))
	{
	    *InputByte |= 0x08;
	}
	if(SDL_JoystickGetButton(joystick, PSP_BUTTON_LEFT) || (x_axis < (range.minx + (range.maxx - range.minx)/3)))
	{
	    *InputByte |= 0x04;
	}

    if (SDL_JoystickGetButton(joystick, PSP_BUTTON_X))
        *InputByte |= 0x10;
    if (SDL_JoystickGetButton(joystick, PSP_BUTTON_O))
        *InputByte |= 0x20;
    if (SDL_JoystickGetButton(joystick, PSP_BUTTON_TRI))
        *InputByte |= 0x40;

/* no variable zoom for now
#ifdef ZOOM_SUPPORT
    if(SDL_JoystickGetButton(joystick, PSP_BUTTON_R) && zoom<60)//272-152=120/2=60
        zoom++;
    if(SDL_JoystickGetButton(joystick, PSP_BUTTON_L) && zoom>0)
        zoom--;
#endif*/

    /*if (SDL_JoystickGetButton(joystick, PSP_BUTTON_VOLUP))
        increaseVolume();
    else if (SDL_JoystickGetButton(joystick, PSP_BUTTON_VOLDOWN))
        decreaseVolume();*/
#elif 0
    SYSTEMINFO *si;
    keystates = SDL_GetKeyState(NULL); //make sure they're updated

    while(SDL_PollEvent(&event))
    {
    }

    if (DOWN(SDLK_ESCAPE) && DOWN(SDLK_RETURN))
        m_bIsActive = FALSE;//Flavor exit emulation

    si = &m_sysInfo[NGP];
    *InputByte = 0;

    if (DOWN(si->InputKeys[KEY_BUTTON_A]))
        *InputByte|= keyCoresp[GameConf.OD_Joy[4]];
    if (DOWN(si->InputKeys[KEY_BUTTON_B]))
        *InputByte|= keyCoresp[GameConf.OD_Joy[5]];
    if (DOWN(si->InputKeys[KEY_BUTTON_X]))
        *InputByte|= keyCoresp[GameConf.OD_Joy[6]];
    if (DOWN(si->InputKeys[KEY_BUTTON_Y]))
        *InputByte|= keyCoresp[GameConf.OD_Joy[7]];
    if (DOWN(si->InputKeys[KEY_BUTTON_R]))
        *InputByte|= keyCoresp[GameConf.OD_Joy[8]];
    if (DOWN(si->InputKeys[KEY_BUTTON_L]))
        *InputByte|= keyCoresp[GameConf.OD_Joy[9]];


    if (DOWN(si->InputKeys[KEY_START]))
        *InputByte|= keyCoresp[GameConf.OD_Joy[10]];
    if (DOWN(si->InputKeys[KEY_SELECT]))
        *InputByte|= keyCoresp[GameConf.OD_Joy[11]];

    if (DOWN(si->InputKeys[KEY_UP]))
        *InputByte|= 0x01;
    if (DOWN(si->InputKeys[KEY_DOWN]))
        *InputByte|= 0x02;
    if (DOWN(si->InputKeys[KEY_LEFT]))
        *InputByte|= 0x04;
    if (DOWN(si->InputKeys[KEY_RIGHT]))
        *InputByte|= 0x08;
/*
    if (DOWN(SDLK_KP_PLUS))
        increaseVolume();
    else if (DOWN(SDLK_KP_MINUS))
        decreaseVolume();
*/
#endif
#endif
}
#endif /* TARGET_PSP */

void FreeInput()
{

}
