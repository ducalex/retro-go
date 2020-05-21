#include <freertos/FreeRTOS.h>
#include <odroid_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pce.h"
#include "osd.h"

#define APP_ID 40

#define AUDIO_SAMPLE_RATE 22050

// static bool netplay = false;
// --- MAIN


static bool save_state(char *pathName)
{
    return SaveState(pathName) == 0;
}


static bool load_state(char *pathName)
{
    if (LoadState(pathName) != 0)
    {
        ResetPCE(false);
        return false;
    }
    return true;
}


void app_main(void)
{
    printf("HuExpress-go (%s-%s).\n", COMPILEDATE, GITREV);

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&load_state, &save_state, NULL);

    char *romFile = odroid_system_get_path(ODROID_PATH_ROM_FILE);

    InitPCE(romFile);

    if (odroid_system_get_start_action() == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }

    RunPCE();

    printf("HuExpress died.\n");
    abort();
}
