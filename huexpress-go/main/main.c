#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <osd.h>

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
    rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    rg_emu_init(&load_state, &save_state, NULL);

    rg_app_desc_t *app = rg_system_get_app();

    InitPCE(app->romPath);

    if (app->startAction == EMU_START_ACTION_RESUME)
    {
        rg_emu_load_state(0);
    }

    RunPCE();

    RG_PANIC("HuExpress died.");
}
