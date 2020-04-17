#ifndef _INCLUDE_OSD_MACHINE
#define _INCLUDE_OSD_MACHINE

#include "cleantypes.h"
#include "pce.h"
#include "sys_dep.h"

#if defined(ENABLE_NETPLAY)
#include "netplay.h"
#endif

extern uchar gamepad;
// gamepad detected ?

extern char dump_snd;
// Do we write sound to file

extern int *fd[4];
// handle for joypad devices


// declaration of the actual callback to call 60 times a second
uint32 interrupt_60hz(uint32, void*);

int osd_init_machine(void);

/*****************************************************************************

    Function: osd_shut_machine

    Description: Deinitialize all stuff that have been inited in osd_int_machine
    Parameters: none
    Return: nothing

*****************************************************************************/
void osd_shut_machine (void);

/*****************************************************************************

    Function: osd_keypressed

    Description: Tells if a key is available for future call of osd_readkey
    Parameters: none
    Return: 0 is no key is available
            else any non zero value

*****************************************************************************/
char osd_keypressed(void);

/*****************************************************************************

    Function: osd_readkey

    Description: Return the first available key stroke, waiting if needed
    Parameters: none
    Return: the key value (currently, lower byte is ascii and higher is scancode)

*****************************************************************************/
uint16 osd_readkey(void);

#endif
