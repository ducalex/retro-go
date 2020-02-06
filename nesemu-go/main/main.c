#include "esp_system.h"
#include "esp_partition.h"
#include "esp_heap_caps.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nofrendo.h"

#include "odroid_settings.h"
#include "odroid_system.h"
#include "odroid_sdcard.h"
#include "odroid_display.h"
#include "odroid_overlay.h"
#include "odroid_input.h"

#include "string.h"

const char* SD_BASE_PATH = "/sd";
static char* ROM_DATA;

extern bool forceConsoleReset;


char *osd_getromdata()
{
	printf("Initialized. ROM@%p\n", ROM_DATA);
	return (char*)ROM_DATA;
}


void app_main(void)
{
	printf("nesemu (%s-%s).\n", COMPILEDATE, GITREV);
	printf("A HEAP:0x%x\n", esp_get_free_heap_size());

	odroid_settings_init();
	odroid_overlay_init();
	odroid_system_init();
    odroid_input_gamepad_init();
    odroid_input_battery_level_init();

    odroid_gamepad_state bootState = odroid_input_read_raw();

    if (bootState.values[ODROID_INPUT_MENU])
    {
        // Force return to menu to recover from ROM loading crashes
        odroid_system_application_set(0);
        esp_restart();
    }

    if (bootState.values[ODROID_INPUT_START])
    {
        // Do not load save state
        forceConsoleReset = true;
    }

    if (odroid_settings_StartAction_get() == ODROID_START_ACTION_RESTART)
    {
        forceConsoleReset = true;
        odroid_settings_StartAction_set(ODROID_START_ACTION_NORMAL);
    }

	//sdcard init must be before LCD init
	esp_err_t sd_init = odroid_sdcard_open(SD_BASE_PATH);

	ili9341_init();

    if (sd_init != ESP_OK)
    {
        odroid_display_show_error(ODROID_SD_ERR_NOCARD);
        abort();
    }

	char* romPath = odroid_settings_RomFilePath_get();
    if (!romPath || strlen(romPath) < 4)
    {
        odroid_display_show_error(ODROID_SD_ERR_BADFILE);
        abort();
    }

    // Load ROM
    ROM_DATA = heap_caps_malloc(1024 * 1024, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if (!ROM_DATA) abort();

    size_t fileSize = 0;

    if (strcasecmp(romPath + (strlen(romPath) - 4), ".zip") == 0)
    {
        printf("app_main ROM: Reading compressed file: %s\n", romPath);
        fileSize = odroid_sdcard_unzip_file_to_memory(romPath, ROM_DATA, 1024 * 1024);
    }
    else
    {
        printf("app_main ROM: Reading file: %s\n", romPath);
        fileSize = odroid_sdcard_copy_file_to_memory(romPath, ROM_DATA, 1024 * 1024);
    }

    printf("app_main ROM: fileSize=%d\n", fileSize);
    if (fileSize == 0)
    {
        odroid_display_show_error(ODROID_SD_ERR_BADFILE);
    }

    free(romPath);

	printf("B HEAP:0x%x\n", esp_get_free_heap_size());

	printf("NoFrendo start!\n");
	char* args[1] = { odroid_sdcard_get_filename(romPath) };
	nofrendo_main(1, args);

	printf("NoFrendo died.\n");
    odroid_display_show_error(ODROID_EMU_ERR_CRASH);
    abort();
}
