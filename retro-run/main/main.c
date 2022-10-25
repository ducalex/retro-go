#include "shared.h"

rg_audio_sample_t audioBuffer[AUDIO_BUFFER_LENGTH];

rg_video_update_t updates[2];
rg_video_update_t *currentUpdate = &updates[0];
rg_video_update_t *previousUpdate = NULL;

rg_app_t *app;


void app_main(void)
{
    app = rg_system_init(AUDIO_SAMPLE_RATE, NULL, NULL);

    RG_LOGI("This is an experimental wrapper to bundle multiple apps in a single binary.");
    RG_LOGI("configNs=%s", app->configNs);

    if (strcmp(app->configNs, "gbc") == 0 || strcmp(app->configNs, "gb") == 0)
        gbc_main();
    else if (strcmp(app->configNs, "nes") == 0)
        nes_main();
    else if (strcmp(app->configNs, "pce") == 0)
        pce_main();
    else if (strcmp(app->configNs, "gw") == 0)
        gw_main();
    else if (strcmp(app->configNs, "lnx") == 0)
        lnx_main();
    else if (strcmp(app->configNs, "snes") == 0)
        snes_main();

    RG_PANIC("Unknown emulator!");
}
