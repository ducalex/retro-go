#include "shared.h"

rg_audio_sample_t audioBuffer[AUDIO_BUFFER_LENGTH];
rg_app_t *app;


void app_main(void)
{
    app = rg_system_init(AUDIO_SAMPLE_RATE, NULL, NULL);

    RG_LOGI("configNs=%s", app->configNs);

    if (strcmp(app->configNs, "gbc") == 0 || strcmp(app->configNs, "gb") == 0)
        gbc_main();
    else if (strcmp(app->configNs, "nes") == 0)
        nes_main();
    else if (strcmp(app->configNs, "pce") == 0)
        pce_main();
    else if (strcmp(app->configNs, "sms") == 0)
        sms_main();
    else if (strcmp(app->configNs, "gg") == 0)
        sms_main();
    else if (strcmp(app->configNs, "col") == 0)
        sms_main();
    else if (strcmp(app->configNs, "gw") == 0)
        gw_main();
    else if (strcmp(app->configNs, "snes") == 0)
        snes_main();
#ifndef __TINYC__
    else if (strcmp(app->configNs, "lnx") == 0)
        lynx_main();
#endif
    else
        launcher_main();

    RG_PANIC("Never reached");
}
