#include "esp_log.h"
#include "bootloader_common.h"
#include "esp_rom_sys.h"
#include "soc/reset_reasons.h"

static const char *TAG = "boot_hook";

// This is the dummy function that forces the linker to include this file
void bootloader_hooks_include(void) { }

void bootloader_after_init(void)
{
    // Retrieve the raw hardware reset reason for CPU 0
    soc_reset_reason_t rst_reason = esp_rom_get_reset_reason(0);
    
    ESP_LOGI(TAG, "Hardware Reset Reason: %d", rst_reason);

    // If the reset reason is NOT a software reset (clean restart requested by system/app)
    if (rst_reason != RESET_REASON_CORE_SW && rst_reason != RESET_REASON_CPU0_SW)
    {
        ESP_LOGW(TAG, "Non-software reset detected. Resetting otadata to load launcher by default.");

        if (bootloader_common_erase_part_type_data(NULL, true))
        {
            ESP_LOGI(TAG, "otadata partition successfully erased!");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to erase otadata partition!");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Software reset detected. Continuing to boot configured partition.");
    }
}

