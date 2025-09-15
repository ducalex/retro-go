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
#include <esp_ota_ops.h>

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

#define FORMAT(message...) ({snprintf(message_buffer, sizeof(message_buffer), message); message_buffer; })
#define CONFIRM(title, message...)                             \
    {                                                          \
        if (!rg_gui_confirm(_(title), FORMAT(message), false)) \
            return false;                                      \
        rg_display_clear(C_BLACK);                             \
    }
#define TRY(cond, error_message)                 \
    if (!(cond))                                 \
    {                                            \
        rg_gui_alert(_("Error"), error_message); \
        return false;                            \
    }

typedef struct
{
    const esp_partition_info_t *src;
    const esp_partition_t *dst;
} partition_pair_t;

static rg_app_t *app;
// static esp_partition_info_t device_partition_table[ESP_PARTITION_TABLE_MAX_ENTRIES];

static bool do_update(const char *filename)
{
    char message_buffer[256];
    esp_partition_info_t partition_table[16]; // ESP_PARTITION_TABLE_MAX_ENTRIES
    int num_partitions = 0;
    img_info_t img_info;
    void *buffer;
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
    TRY(esp_partition_table_verify(partition_table, true, &num_partitions) == ESP_OK, "File is not a valid ESP32 image.\nCannot continue.");

    const char *current_partition = esp_ota_get_running_partition()->label;
    partition_pair_t queue[num_partitions];
    size_t queue_count = 0;

    // At this time we only flash partitions of type app and subtype ota_X
    for (int i = 0; i < num_partitions; ++i)
    {
        const esp_partition_info_t *src = &partition_table[i];
        const esp_partition_t *dst = esp_partition_find_first(src->type, ESP_PARTITION_SUBTYPE_ANY, (char *)src->label);

        if (src->type != PART_TYPE_APP || (src->subtype & 0xF0) != PART_SUBTYPE_OTA_FLAG)
            RG_LOGW("Skipping partition %.15s: Unsupported type.", (char *)src->label);
        else if (!dst)
            RG_LOGW("Skipping partition %.15s: No match found.", (char *)src->label);
        else if ((dst->subtype & 0xF0) != PART_SUBTYPE_OTA_FLAG)
            RG_LOGW("Skipping partition %.15s: Not an OTA partition.", (char *)src->label);
        else if (strncmp(current_partition, dst->label, 16) == 0)
            RG_LOGW("Skipping partition %.15s: Currently running.", (char *)src->label);
        else if (dst->size < src->pos.size)
            RG_LOGW("Skipping partition %.15s: New partition is bigger.", (char *)src->label);
        else
        {
            queue[queue_count].src = src;
            queue[queue_count].dst = dst;
            queue_count++;
        }
    }

    TRY(queue_count > 0, "Found no updatable partition!");

    int pos = 0;
    for (size_t i = 0; i < queue_count; ++i)
        pos += snprintf(message_buffer + pos, sizeof(message_buffer) - pos, "- %s\n", queue[i].dst->label);
    pos += snprintf(message_buffer + pos, sizeof(message_buffer) - pos, "\nProceed?");

    if (!rg_gui_confirm("Partitions to be updated:", message_buffer, false))
        return false;

    TRY(buffer = malloc(2 * 1024 * 1024), "Memory allocation failed");

    // TODO: Implement scrolling. Maybe on rg_gui side?
    //       Or at least support continue writing on the same line. "... Complete"
    int y_pos = 9999;
    #define SCREEN_PRINTF(color, message...)                                               \
        if (y_pos + 16 > rg_display_get_height())                                          \
            rg_display_clear(C_BLACK), y_pos = 0;                                          \
        y_pos += rg_gui_draw_text(0, y_pos, 0, FORMAT(message), color, C_BLACK, 0).height;

    SCREEN_PRINTF(C_WHITE, "Starting...");

    for (size_t i = 0; i < queue_count; ++i)
    {
        const esp_partition_info_t *src = queue[i].src;
        const esp_partition_t *dst = queue[i].dst;
        esp_ota_handle_t ota_handle;
        esp_err_t ret;
        if ((ret = esp_ota_begin(queue[i].dst, 0, &ota_handle)) != ESP_OK)
        {
            SCREEN_PRINTF(C_RED, "Skipping %s: %s", dst->label, esp_err_to_name(ret));
            continue;
        }
        SCREEN_PRINTF(C_WHITE, "Reading %s from file...", dst->label);
        if (fseek(fp, src->pos.offset, SEEK_SET) != 0 || !fread(buffer, src->pos.size, 1, fp))
        {
            SCREEN_PRINTF(C_RED, "File read error");
            continue;
        }
        SCREEN_PRINTF(C_WHITE, "Writing %s to flash...", dst->label);
        if ((ret = esp_ota_write(ota_handle, buffer, src->pos.size)) == ESP_OK) {
            SCREEN_PRINTF(C_GREEN, "Complete!");
        } else {
            SCREEN_PRINTF(C_RED, "Failed: %s", esp_err_to_name(ret));
        }
        esp_ota_end(ota_handle);
    }
    free(buffer);

    rg_gui_alert(_("All done"), "The process is complete");
    return true;
}

void app_main(void)
{
    app = rg_system_init(&(const rg_config_t){
        .storageRequired = true,
        // This is a hack to hide the status bar with bogus speed info. A better solution will be
        // needed as we still want the battery icon!
        .isLauncher = true,
    });

    // if (spi_flash_read(ESP_PARTITION_TABLE_OFFSET, &device_partition_table, sizeof(device_partition_table)) != ESP_OK)
    // {
    //     rg_gui_alert(_("Error"), "Failed to read device's partition table!");
    //     rg_system_exit();
    // }

    // const char *filename = app->romPath;
    while (true)
    {
        char *filename = rg_gui_file_picker("Select update", RG_BASE_PATH_UPDATES, NULL, true, true);
        if (!filename || !*filename)
            break;
        if (do_update(filename))
            break;
        free(filename);
    }

    rg_system_exit();
}
