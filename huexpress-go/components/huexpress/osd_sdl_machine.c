#include "utils.h"
#include "osd_sdl_machine.h"
#include "myadd.h"

#if 0

int netplay_mode;
char initial_path[PATH_MAX] = "";
// prefered path for for searching

uchar* osd_gfx_buffer = NULL;

uchar gamepad = 0;
// gamepad detected ?

uchar* XBuf;
// The screen buffer where we draw before blitting it on screen

int gamepad_driver = 0;
// what kind of jypad must we have to handle

char dump_snd = 0;
// Do we write sound to file

char synchro;
// � fond, � fond, � fond? (french joke ;)

int vwidth, vheight;
// size of visible part of the screen (I got troubles with allegro screen->* values!)

SDL_TimerID timerId;
// handle for the timer callback

uint32 interrupt_60hz(uint32, void*);
// declaration of the actual callback to call 60 times a second

int
osd_init_machine(void)
{
	MESSAGE_INFO("Emulator initialization\n");

	if (SDL_Init(SDL_INIT_TIMER)) {
		MESSAGE_ERROR("Could not initialise SDL : %s\n", SDL_GetError());
		return 0;
	}

	atexit(SDL_Quit);

	if (!(XBuf = (uchar*)malloc(XBUF_WIDTH * XBUF_HEIGHT)))
	{
		MESSAGE_ERROR("Initialization failed...\n");
		TRACE("%s, couldn't malloc XBuf\n", __func__);
		return (0);
	}

	MESSAGE_INFO("Clearing buffers...\n");
	bzero(XBuf, XBUF_WIDTH * XBUF_HEIGHT);

	MESSAGE_INFO("Initiating sound...\n");
	InitSound();

	osd_gfx_buffer = XBuf + 32 + 64 * XBUF_WIDTH;
		// We skip the left border of 32 pixels and the 64 first top lines

	timerId = SDL_AddTimer(1000 / 60, interrupt_60hz, NULL);
	if (timerId)
		MESSAGE_INFO("SDL timer initialization successful\n");
	else {
		MESSAGE_ERROR("SDL timer initialization failed...\n");
		TRACE("Couldn't SDL_AddTimer in %s\n", __func__);
	}

	return 1;
}


/*****************************************************************************

    Function: osd_shut_machine

    Description: Deinitialize all stuff that have been inited in osd_int_machine
    Parameters: none
    Return: nothing

*****************************************************************************/
void
osd_shut_machine (void)
{
	free(XBuf);

	if (timerId > 0)
		SDL_RemoveTimer(timerId);

	if (dump_snd)
		fclose(out_snd);

	TrashSound();

	SDL_Quit();

	wipe_directory(tmp_basepath);
}

/*****************************************************************************

    Function: osd_keypressed

    Description: Tells if a key is available for future call of osd_readkey
    Parameters: none
    Return: 0 is no key is available
            else any non zero value

*****************************************************************************/
char
osd_keypressed(void)
{
	#warning TODO: implement keypressed with sdl
	return 0;
}


/*****************************************************************************

    Function: osd_readkey

    Description: Return the first available key stroke, waiting if needed
    Parameters: none
    Return: the key value (currently, lower byte is ascii and higher is scancode)

*****************************************************************************/
uint16
osd_readkey(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				return event.key.keysym.scancode;
			case SDL_QUIT:
				return 0;
		}
	}
	return 0;
}

#else

int
osd_init_machine(void)
{
    //
}

#endif