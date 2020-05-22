extern "C" {
#include <freertos/FreeRTOS.h>
#include <odroid_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

#include <handy.h>

#define APP_ID 50

#define AUDIO_SAMPLE_RATE 22050

static CSystem *lynx = NULL;

// static bool netplay = false;
// --- MAIN


static bool save_state(char *pathName)
{
    return true;
}


static bool load_state(char *pathName)
{
    lynx->Reset();
    return true;
}


static UBYTE* lynx_display_callback(ULONG objref)
{
    return 0;
}


extern "C" void app_main(void)
{
    printf("\n========================================\n\n");
    printf("Handy-go (%s).\n", PROJECT_VER);

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&load_state, &save_state, NULL);

    char *romFile = odroid_system_get_path(ODROID_PATH_ROM_FILE);

    // Init emulator
    lynx = new CSystem(romFile, "bios", true);
    lynx->SetButtonData(0);
    lynx->DisplaySetAttributes(
        MIKIE_NO_ROTATE,
        MIKIE_PIXEL_FORMAT_16BPP_565,
        SCREEN_WIDTH * 2,
        lynx_display_callback,
        0);

    if (odroid_system_get_start_action() == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }

    // Start emulation
    while (1)
    {
      lynx->Update();
    }

    printf("Handy died.\n");
    abort();
}
