#include <freertos/FreeRTOS.h>
#include <odroid_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pce.h"

#define APP_ID 40

#define AUDIO_SAMPLE_RATE 22050

static bool netplay = false;

static bool saveSRAM = false;
static int  sramSaveTimer = 0;
// --- MAIN


static bool SaveState(char *pathName)
{
    // return state_save(pathName) == 0;
    return true;
}


static bool LoadState(char *pathName)
{
    ResetPCE();
    // if (state_load(pathName) != 0)
    // {
    //     emu_reset();

    //     if (saveSRAM) sram_load();

    //     return false;
    // }
    return true;
}


void app_main(void)
{
    printf("huexpress (%s-%s).\n", COMPILEDATE, GITREV);

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    char *romFile = odroid_system_get_path(NULL, ODROID_PATH_ROM_FILE);

	// Initialise the host machine
	osd_init_machine();
	osd_init_input();

    InitPCE(romFile);

	osd_gfx_init();
	osd_snd_init_sound();

    RunPCE();

    printf("Huexpress died.\n");
    abort();
}
