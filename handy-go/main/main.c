#include <freertos/FreeRTOS.h>
#include <odroid_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "handy.h"

#define APP_ID 50

#define AUDIO_SAMPLE_RATE 22050

// static bool netplay = false;
// --- MAIN


static bool save_state(char *pathName)
{
    return true;
}


static bool load_state(char *pathName)
{
    return true;
}


void app_main(void)
{
    printf("Handy-go (%s-%s).\n", COMPILEDATE, GITREV);

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&load_state, &save_state, NULL);

    char *romFile = odroid_system_get_path(NULL, ODROID_PATH_ROM_FILE);

    // Init emulator

    if (odroid_system_get_start_action() == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }

    // Start emulation

    printf("Handy died.\n");
    abort();
}
