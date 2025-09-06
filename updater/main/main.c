#include <rg_system.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <esp_flash_partitions.h>
#include <esp_partition.h>
#include <esp_spi_flash.h>

#ifdef RG_UPDATER_DOWNLOAD_LOCATION
#define DOWNLOAD_LOCATION RG_UPDATER_DOWNLOAD_LOCATION
#else
#define DOWNLOAD_LOCATION RG_BASE_PATH
#endif

#if CONFIG_SPI_FLASH_DANGEROUS_WRITE_ALLOWED
#define CAN_UPDATE_PARTITION_TABLE 1
#else
#define CAN_UPDATE_PARTITION_TABLE 0
#endif

#define RETRO_GO_IMG_MAGIC 0x31304752 // "RG01"
typedef struct
{
    uint32_t magic;
    uint32_t crc32;
    uint32_t timestamp;
    char target[32];
    char version[32];
    char reserved[180];
} img_info_t;

#define CONFIRM(title, message...)                                 \
    {                                                              \
        snprintf(message_buffer, sizeof(message_buffer), message); \
        if (!rg_gui_confirm(_(title), message_buffer, false))      \
            return false;                                          \
        rg_display_clear(C_BLACK);                                 \
    }
#define ALERT(title, message...)                                   \
    {                                                              \
        snprintf(message_buffer, sizeof(message_buffer), message); \
        rg_gui_alert(_(title), message_buffer);                    \
        rg_display_clear(C_BLACK);                                 \
    }
#define ERROR(message...)        \
    {                            \
        ALERT("Error", message); \
        return false;            \
    }
#define TRY(cond, error_message) \
    if (!(cond))                 \
        ERROR(error_message);

static rg_app_t *app;
static esp_partition_info_t device_partition_table[ESP_PARTITION_TABLE_MAX_ENTRIES];
static char message_buffer[256];

static bool do_update(const char *filename)
{
    esp_partition_info_t partition_table[16]; // ESP_PARTITION_TABLE_MAX_ENTRIES
    int num_partitions = 0;
    img_info_t img_info;
    int filesize = 0;
    FILE *fp;

    RG_LOGI("Filename: %s", filename);
    rg_display_clear(C_BLACK);

    TRY(fp = fopen(filename, "rb"), "File open failed");
    TRY(fseek(fp, 0, SEEK_END) == 0, "File seek failed");
    TRY(fseek(fp, -sizeof(img_info_t), SEEK_CUR) == 0, "File seek failed");
    filesize = ftell(fp); // Size without the footer
    TRY(fread(&img_info, sizeof(img_info), 1, fp), "File read failed");

    img_info.target[sizeof(img_info.target) - 1] = 0;   // Just in case
    img_info.version[sizeof(img_info.version) - 1] = 0; // Just in case

    if (img_info.magic != RETRO_GO_IMG_MAGIC) // Invalid image
    {
        CONFIRM("Warning", "File is not a valid Retro-Go image.\nContinue anyway?");
    }
    else if (strcasecmp(img_info.target, RG_TARGET_NAME) != 0) // Wrong target
    {
        CONFIRM("Warning",
                "The file appears to be for a different device.\n"
                "Current device: %s\n"
                "Image device: %s\n\n"
                "Do you want to continue anyway?",
                RG_TARGET_NAME,
                img_info.target);
    }
    else // Valid image
    {
        CONFIRM("Version", "Current version: %s\nNew version: %s\n\nContinue?", app->version, img_info.version);
    }

    // TODO: Also support images that truncate the first 0x1000, just in case
    TRY(fseek(fp, ESP_PARTITION_TABLE_OFFSET, SEEK_SET) == 0, "File seek failed");
    TRY(fread(&partition_table, sizeof(partition_table), 1, fp), "File read failed");

    if (esp_partition_table_verify(partition_table, true, &num_partitions) != ESP_OK)
        ERROR("File is not a valid esp32 image.\nCannot continue.");

    ALERT("Info", "Image contains %d partitions.", num_partitions);

    // We can work around this, this is for debugging purposes
    if (memcmp(&device_partition_table, &partition_table, sizeof(partition_table)))
        ALERT("Error", "Partition table mismatch.");

    return true;
}

void app_main(void)
{
    app = rg_system_init(32000, NULL, NULL);
    // This is a hack to hide the status bar with bogus speed info. A better solution will be
    // needed as we still want the battery icon!
    app->isLauncher = true;

    if (!rg_storage_ready())
    {
        ALERT("Error", "Storage mount failed.\nMake sure the card is FAT32.");
        rg_system_exit();
    }

    if (spi_flash_read(ESP_PARTITION_TABLE_OFFSET, &device_partition_table, sizeof(device_partition_table)) != ESP_OK)
    {
        ALERT("Error", "Failed to read device's partition table!");
        rg_system_exit();
    }

    const char *filename = app->romPath;
    while (true)
    {
        if (!filename || !*filename)
        {
            filename = rg_gui_file_picker("Select update", DOWNLOAD_LOCATION, NULL, true, true);
            if (!filename || !*filename)
                break;
        }
        if (do_update(filename))
            break;
        filename = NULL;
    }

    rg_system_exit();
}
