#include "odroid_console.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "string.h"
#include "stdio.h"

void odroid_console_init(char **romPath, bool *reset, int sample_rate)
{
    printf("odroid_console_init: %d KB free\n", esp_get_free_heap_size() / 1024);

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

    reset = odroid_settings_StartAction_get() == ODROID_START_ACTION_RESTART;
    if (reset)
    {
        odroid_settings_StartAction_set(ODROID_START_ACTION_NORMAL);
    }

	//sdcard init must be before LCD init
	esp_err_t sd_init = odroid_sdcard_open();

	ili9341_init();

    if (esp_reset_reason() == ESP_RST_POWERON)
    {
        // ili9341_blank_screen();
    }

    if (esp_reset_reason() == ESP_RST_PANIC)
    {
        odroid_overlay_alert("The emulator crashed");
        odroid_system_application_set(0);
        esp_restart();
    }

    if (sd_init != ESP_OK)
    {
        odroid_display_show_error(ODROID_SD_ERR_NOCARD);
        odroid_system_halt();
    }

	*romPath = odroid_settings_RomFilePath_get();
    if (!*romPath || strlen(*romPath) < 4)
    {
        odroid_display_show_error(ODROID_SD_ERR_BADFILE);
        odroid_system_halt();
    }

    odroid_audio_init(odroid_settings_AudioSink_get(), sample_rate);

    printf("odroid_console_init: System ready!\n");
}
