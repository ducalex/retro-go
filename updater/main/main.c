#include <rg_system.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_partition.h>
#include <esp_flash_partitions.h>

#if defined(RG_TARGET_ODROID_GO)
#define DOWNLOAD_LOCATION RG_STORAGE_ROOT "/odroid/firmware"
#else
#define DOWNLOAD_LOCATION RG_STORAGE_ROOT "/espgbc/firmware"
#endif

const char ODROID_HEADER[] = "ODROIDGO_FIRMWARE_V00_01";
const char ESPLAY_HEADER[] = "ESPLAY_FIRMWARE_V00_01";

#define FIRMWARE_HEADER_SIZE (24)
#define FIRMWARE_DESCRIPTION_SIZE (40)
#define FIRMWARE_PARTS_MAX (20)
#define FIRMWARE_TILE_SIZE (86 * 48)

typedef struct
{
    uint8_t type;
    uint8_t subtype;
    uint8_t _reserved0;
    uint8_t _reserved1;
    char     label[16];
    uint32_t flags;
    uint32_t length;
    uint32_t dataLength;
    uint8_t data[];
} fw_partition_t;

typedef struct
{
    char header[FIRMWARE_HEADER_SIZE];
    char description[FIRMWARE_DESCRIPTION_SIZE];
    uint16_t tile[FIRMWARE_TILE_SIZE];
    // uint32_t checksum;
} fw_t;

typedef struct
{
    uint8_t padding[0x8000];
    esp_partition_info_t partitions[ESP_PARTITION_TABLE_MAX_ENTRIES];
} img_t;

void update_partition(esp_partition_type_t type, esp_partition_subtype_t subtype, const char *label, void *data, size_t data_len)
{
    const esp_partition_t *part = esp_partition_find_first(type, subtype, label);

    rg_display_clear(C_BLACK);
    rg_gui_draw_text(0, 32, RG_SCREEN_WIDTH, label, C_WHITE, C_BLACK, RG_TEXT_BIGGER|RG_TEXT_ALIGN_CENTER);

    if (!part)
    {
        // We don't seem to have this partition and we can't rewrite the partition table to add it...
        rg_gui_alert("Unknown partition", label);
    }
    else if (strncmp(label, "updater", 8) == 0)
    {
        // Can't update self, skip
    }
    else if (data_len > part->size)
    {
        // If it's a .img we should truncate and try anyway, who knows if the extra is empty and it would work!
        rg_gui_alert("New size won't fit current partition!", label);
    }
    else if (data_len > 0)
    {
        if (type == ESP_PARTITION_TYPE_DATA)
        {
            // maybe we should ask the user if they want to overwrite it? Could be a filesystem
        }
        rg_gui_draw_text(0, 64, RG_SCREEN_WIDTH, "Erasing....", C_WHITE, C_BLACK, RG_TEXT_BIGGER|RG_TEXT_ALIGN_CENTER);
        esp_partition_erase_range(part, 0, part->size);
        rg_gui_draw_text(0, 64, RG_SCREEN_WIDTH, "Writing....", C_WHITE, C_BLACK, RG_TEXT_BIGGER|RG_TEXT_ALIGN_CENTER);
        esp_partition_write(part, 0, data, data_len);
    }
    else
    {
        // Do nothing, could be a data partition
    }
}

void app_main(void)
{
    rg_app_t *app = rg_system_init(32000, NULL, NULL);

    if (!rg_storage_ready())
    {
        rg_display_clear(C_SKY_BLUE);
        rg_gui_alert("SD Card Error", "Storage mount failed.\nMake sure the card is FAT32.");
        // What do at this point? Reboot? Switch back to launcher?
        goto launcher;
    }

    const char *filename;

    if (app->romPath && strlen(app->romPath) && rg_storage_exists(app->romPath))
        filename = app->romPath;
    else
        filename = rg_gui_file_picker("Select update", DOWNLOAD_LOCATION, NULL, true);

    RG_LOGI("Filename: %s", filename);

    if (!filename)
        goto launcher;

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        rg_gui_alert("Error", "File open failed");
        goto launcher;
    }

    uint8_t *partition_buffer = malloc(2 * 1024 * 1024);
    uint8_t *header_buffer = calloc(1, 0x10000);
    if (!header_buffer || !partition_buffer)
    {
        rg_gui_alert("Error", "Out of memory");
        goto launcher;
    }

    // No need to check this one, the header check will tell us if all is good
    fread(header_buffer, 0x10000, 1, fp);

    const fw_t *fw = (const fw_t *)header_buffer;
    const img_t *img = (const img_t *)header_buffer;

    // .fw format
    if (memcmp(fw->header, ODROID_HEADER, 24) == 0 || memcmp(fw->header, ESPLAY_HEADER, 22) == 0)
    {
        size_t next_entry = 24;

        if (memcmp(fw->header, ESPLAY_HEADER, 22) == 0)
        {
            // ESPLAY header is 2 bytes shorter, just shift the data and everything else is good
            memmove((void *)fw + 2, fw, sizeof(fw_t) - 2);
            next_entry = 22;
        }

        if (!rg_gui_confirm("Flash fw update?", fw->description, false))
        {
            goto launcher;
        }

        // Here we should check the checksum blah blah blah

        while (!feof(fp))
        {
            fw_partition_t part;
            if (fseek(fp, next_entry, SEEK_SET) != 0 || !fread(&part, sizeof(part), 1, fp))
                break;
            next_entry = ftell(fp) + part.dataLength;
            if (fread(partition_buffer, part.dataLength, 1, fp))
                update_partition(part.type, part.subtype, part.label, partition_buffer, part.dataLength);
            else
                rg_gui_alert("File read error", part.label);
        }
    }

    // .img format
    else if (img->partitions[0].magic == ESP_PARTITION_MAGIC)
    {
        if (!rg_gui_confirm("Flash img update?", "", false))
        {
            goto launcher;
        }

        for (size_t i = 0; i < ESP_PARTITION_TABLE_MAX_ENTRIES; ++i)
        {
            const esp_partition_info_t *part = &img->partitions[i];
            if (part->magic == ESP_PARTITION_MAGIC)
            {
                if (part->type != PART_TYPE_APP && part->type != PART_TYPE_DATA)
                    break;
                // FIXME: label might not be nul-terminated, we should move it to a buffer and ensure it is
                if (fseek(fp, part->pos.offset, SEEK_SET) == 0 && fread(partition_buffer, part->pos.size, 1, fp))
                    update_partition(part->type, part->subtype, (char *)part->label, partition_buffer, part->pos.size);
                else
                    rg_gui_alert("File read error", (char *)part->label);
            }
        }
    }
    else
    {
        rg_gui_alert("Error", "Invalid file format!");
        goto launcher;
    }

    fclose(fp);

    rg_display_clear(C_BLACK);
    rg_gui_alert("Update complete", "All done!");

launcher:
    rg_system_switch_app(RG_APP_LAUNCHER, 0, 0, 0);
}
