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

const char* SD_BASE_PATH = "/sd";
static char* ROM_DATA;

extern bool forceConsoleReset;


char *osd_getromdata()
{
	printf("Initialized. ROM@%p\n", ROM_DATA);
	return (char*)ROM_DATA;
}


int app_main(void)
{
	printf("nesemu (%s-%s).\n", COMPILEDATE, GITREV);
	printf("A HEAP:0x%x\n", esp_get_free_heap_size());

	odroid_settings_init();
	odroid_overlay_init();
	odroid_system_init();
    odroid_input_gamepad_init();
    odroid_input_battery_level_init();

	//sdcard init must be before LCD init
	esp_err_t sd_init = odroid_sdcard_open(SD_BASE_PATH);

	ili9341_init();

    if (sd_init != ESP_OK)
    {
        odroid_display_show_sderr(ODROID_SD_ERR_NOCARD);
        abort();
    }

    odroid_gamepad_state bootState = odroid_input_read_raw();

    if (bootState.values[ODROID_INPUT_MENU])
    {
        // Force return to menu to recover from
        // ROM loading crashes
        odroid_system_application_set(0);
        esp_restart();
    }

    if (bootState.values[ODROID_INPUT_START])
    {
        // Reset emulator if button held at startup to
        // override save state
        forceConsoleReset = true; //emu_reset();
    }

    if (odroid_settings_StartAction_get() == ODROID_START_ACTION_RESTART)
    {
        forceConsoleReset = true;
        odroid_settings_StartAction_set(ODROID_START_ACTION_NORMAL);
    }

	// Load ROM
    ROM_DATA = heap_caps_malloc(1024 * 1024, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if (!ROM_DATA) abort();

	char* romPath = odroid_settings_RomFilePath_get();
    char *fileName = odroid_sdcard_get_filename(romPath);
    printf("osd_getromdata: Reading from sdcard.\n");

    size_t fileSize = odroid_sdcard_copy_file_to_memory(romPath, ROM_DATA);
    printf("app_main: fileSize=%d\n", fileSize);
    if (fileSize == 0)
    {
        odroid_display_show_sderr(ODROID_SD_ERR_BADFILE);
        abort();
    }

    free(romPath);

	printf("B HEAP:0x%x\n", esp_get_free_heap_size());

	printf("NoFrendo start!\n");

	char* args[1] = { fileName };
	nofrendo_main(1, args);

	printf("NoFrendo died.\n");
	asm("break.n 1");
    return 0;
}
