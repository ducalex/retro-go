/* vim: set tabstop=3 expandtab:
**
** This file is in the public domain.
**
** osd.c
**
** $Id: osd.c,v 1.2 2001/04/27 14:37:11 neil Exp $
**
*/

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <noftypes.h>
#include <nofconfig.h>
#include <log.h>
#include <osd.h>
#include <nofrendo.h>

#include <version.h>

//#include "joystick.h"

#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "esp_system.h"


char configfilename[]="na";

/* This is os-specific part of main() */
int osd_main(int argc, char *argv[])
{
    // // Joystick.
    // //JoystickInit();
    // odroid_input_gamepad_init();
    //
    //
    // // Boot state overrides
    // //bool forceConsoleReset = false;
    //
    //
    // //JoystickState bootState = JoystickRead();
    // odroid_gamepad_state bootState;
    //
    //
    // if (bootState.Volume)
    // {
    //     // Force return to factory app to recover from
    //     // ROM loading crashes
    //
    //     // Set factory app
    //     const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
    //     if (partition == NULL)
    //     {
    //         abort();
    //     }
    //
    //     esp_err_t err = esp_ota_set_boot_partition(partition);
    //     if (err != ESP_OK)
    //     {
    //         abort();
    //     }
    //
    //     // Reset
    //     esp_restart();
    // }

    // if (bootState.Start)
    // {
    //     // Reset emulator if button held at startup to
    //     // override save state
    //     forceConsoleReset = true; //emu_reset();
    // }


   config.filename = configfilename;

   return main_loop(argv[0], system_nes);
}

/* File system interface */
void osd_fullname(char *fullname, const char *shortname)
{
   strncpy(fullname, shortname, PATH_MAX);
}

/* This gives filenames for storage of saves */
char *osd_newextension(char *string, char *ext)
{
   return string;
}

/* This gives filenames for storage of PCX snapshots */
int osd_makesnapname(char *filename, int len)
{
   return -1;
}
