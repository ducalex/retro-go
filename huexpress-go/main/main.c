#include "freertos/FreeRTOS.h"
#include <odroid_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pce.h"

#include "./odroid_debug.h"

#define APP_ID 40

#define AUDIO_SAMPLE_RATE 22050

void app_main(void)
{
    printf("huexpress (%s-%s).\n", COMPILEDATE, GITREV);

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(NULL, NULL, NULL);

    char *romFile = odroid_system_get_path(NULL, ODROID_PATH_ROM_FILE);

    strcpy(config_basepath, "/sd/odroid/data/pce");
    strcpy(tmp_basepath, "");
    strcpy(video_path, "");
    strcpy(ISO_filename, "");
    strcpy(sav_basepath,"/sd/odroid/data");
    strcpy(sav_path, "pce");
    strcpy(log_filename,"");

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
