#include <rg_system.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_spi_flash.h>
#include <esp_partition.h>

#ifdef RG_UPDATER_DOWNLOAD_LOCATION
#define DOWNLOAD_LOCATION RG_UPDATER_DOWNLOAD_LOCATION
#else
#define DOWNLOAD_LOCATION RG_BASE_PATH
#endif

void app_main(void)
{
    rg_app_t *app = rg_system_init(32000, NULL, NULL);

    if (!rg_storage_ready())
    {
        rg_display_clear(C_SKY_BLUE);
        rg_gui_alert(_("SD Card Error"), _("Storage mount failed.\nMake sure the card is FAT32."));
        // What do at this point? Reboot? Switch back to launcher?
        goto launcher;
    }

    // Hopefully we'll receive the filename from the launcher which has a nicer file browser
    const char *filename = app->romPath;

    if (!filename || !rg_storage_exists(filename))
        filename = rg_gui_file_picker(_("Select update"), DOWNLOAD_LOCATION, NULL, true);
    if (!filename)
        goto launcher;

    RG_LOGI("Filename: %s", filename);

launcher:
    rg_system_switch_app(RG_APP_LAUNCHER, 0, 0, 0, 0);
}
