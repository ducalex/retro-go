/* Demo ESP LittleFS Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_idf_version.h"
#include "esp_flash.h"
#include "esp_chip_info.h"
#include "spi_flash_mmap.h"


#include "esp_littlefs.h"

static const char *TAG = "demo_esp_littlefs";

void app_main(void)
{
        printf("Demo LittleFs implementation by esp_littlefs!\n");
        printf("   https://github.com/joltwallet/esp_littlefs\n");

        /* Print chip information */
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);
        printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
               CONFIG_IDF_TARGET,
               chip_info.cores,
               (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
               (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

        printf("silicon revision %d, ", chip_info.revision);

        uint32_t size_flash_chip = 0;
        esp_flash_get_size(NULL, &size_flash_chip);
        printf("%uMB %s flash\n", (unsigned int)size_flash_chip >> 20,
               (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

        printf("Free heap: %u\n", (unsigned int) esp_get_free_heap_size());

        printf("Now we are starting the LittleFs Demo ...\n");

        ESP_LOGI(TAG, "Initializing LittleFS");

        esp_vfs_littlefs_conf_t conf = {
            .base_path = "/littlefs",
            .partition_label = "littlefs",
            .format_if_mount_failed = true,
            .dont_mount = false,
        };

        // Use settings defined above to initialize and mount LittleFS filesystem.
        // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
        esp_err_t ret = esp_vfs_littlefs_register(&conf);

        if (ret != ESP_OK)
        {
                if (ret == ESP_FAIL)
                {
                        ESP_LOGE(TAG, "Failed to mount or format filesystem");
                }
                else if (ret == ESP_ERR_NOT_FOUND)
                {
                        ESP_LOGE(TAG, "Failed to find LittleFS partition");
                }
                else
                {
                        ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
                }
                return;
        }

        size_t total = 0, used = 0;
        ret = esp_littlefs_info(conf.partition_label, &total, &used);
        if (ret != ESP_OK)
        {
                ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        }
        else
        {
                ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
        }

        // Use POSIX and C standard library functions to work with files.
        // First create a file.
        ESP_LOGI(TAG, "Opening file");
        FILE *f = fopen("/littlefs/hello.txt", "w");
        if (f == NULL)
        {
                ESP_LOGE(TAG, "Failed to open file for writing");
                return;
        }
        fprintf(f, "LittleFS Rocks!\n");
        fclose(f);
        ESP_LOGI(TAG, "File written");

        // Check if destination file exists before renaming
        struct stat st;
        if (stat("/littlefs/foo.txt", &st) == 0)
        {
                // Delete it if it exists
                unlink("/littlefs/foo.txt");
        }

        // Rename original file
        ESP_LOGI(TAG, "Renaming file");
        if (rename("/littlefs/hello.txt", "/littlefs/foo.txt") != 0)
        {
                ESP_LOGE(TAG, "Rename failed");
                return;
        }

        // Open renamed file for reading
        ESP_LOGI(TAG, "Reading file");
        f = fopen("/littlefs/foo.txt", "r");
        if (f == NULL)
        {
                ESP_LOGE(TAG, "Failed to open file for reading");
                return;
        }

        char line[128];
        char *pos;

        fgets(line, sizeof(line), f);
        fclose(f);
        // strip newline
        pos = strchr(line, '\n');
        if (pos)
        {
                *pos = '\0';
        }
        ESP_LOGI(TAG, "Read from file: '%s'", line);

        ESP_LOGI(TAG, "Reading from flashed filesystem example.txt");
        f = fopen("/littlefs/example.txt", "r");
        if (f == NULL)
        {
                ESP_LOGE(TAG, "Failed to open file for reading");
                return;
        }
        fgets(line, sizeof(line), f);
        fclose(f);
        // strip newline
        pos = strchr(line, '\n');
        if (pos)
        {
                *pos = '\0';
        }
        ESP_LOGI(TAG, "Read from file: '%s'", line);

        // All done, unmount partition and disable LittleFS
        esp_vfs_littlefs_unregister(conf.partition_label);
        ESP_LOGI(TAG, "LittleFS unmounted");
}
