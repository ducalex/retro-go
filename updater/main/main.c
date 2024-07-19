#include <rg_system.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_partition.h>

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

    // In the final version we'll read in chunk but since we have 4MB of PSRAM let's use it for this POC
    void *buffer = malloc(2 * 1024 * 1024);
    fw_t *fw = calloc(1, sizeof(fw_t));
    if (!fw || !buffer)
    {
        rg_gui_alert("Error", "Out of memory");
        goto launcher;
    }

    // No need to check this one, the header check will tell us if all is good
    fread(fw, sizeof(fw_t), 1, fp);

    if (memcmp(fw->header, ODROID_HEADER, 24) == 0)
    {
        // All good
    }
    else if (memcmp(fw->header, ESPLAY_HEADER, 22) == 0)
    {
        // ESPLAY header is 2 bytes shorter, just shift the data and everything else is good
        memmove((void *)fw + 2, fw, sizeof(fw_t) - 2);
        fseek(fp, sizeof(fw_t) - 2, SEEK_SET);
    }
    else
    {
        rg_gui_alert("Error", "Invalid file format!");
        goto launcher;
    }

    if (!rg_gui_confirm("Flash update?", fw->description, false))
    {
        goto launcher;
    }

    // Here we should check the checksum blah blah blah


    // Copy the firmware
    for (fw_partition_t fw_part; fread(&fw_part, sizeof(fw_part), 1, fp);)
    {
        const esp_partition_t *part = esp_partition_find_first(fw_part.type, fw_part.subtype, fw_part.label);
        size_t nextEntry = ftell(fp) + fw_part.dataLength;

        rg_display_clear(C_BLACK);
        rg_gui_draw_text(0, 32, RG_SCREEN_WIDTH, part->label, C_WHITE, C_BLACK, RG_TEXT_BIGGER|RG_TEXT_ALIGN_CENTER);

        if (!part)
        {
            // We don't seem to have this partition and we can't rewrite the partition table to add it...
            rg_gui_alert("Unknown partition", fw_part.label);
        }
        else if (strncmp(part->label, "updater", 8) == 0)
        {
            // Can't update self
            rg_gui_alert("Skipping self", fw_part.label);
        }
        else if (fw_part.dataLength > part->size)
        {
            rg_gui_alert("Size mismatch, can't flash", fw_part.label);
        }
        else if (fw_part.dataLength > 0)
        {
            if (fw_part.length != part->size)
            {
                rg_gui_draw_text(0, 96, RG_SCREEN_WIDTH, "Warning: Size mismatch", C_ORANGE, C_BLACK, RG_TEXT_BIGGER|RG_TEXT_ALIGN_CENTER);
            }

            if (fread(buffer, fw_part.dataLength, 1, fp))
            {
                rg_gui_draw_text(0, 64, RG_SCREEN_WIDTH, "Erasing....", C_WHITE, C_BLACK, RG_TEXT_BIGGER|RG_TEXT_ALIGN_CENTER);
                esp_partition_erase_range(part, 0, part->size);
                rg_gui_draw_text(0, 64, RG_SCREEN_WIDTH, "Writing....", C_WHITE, C_BLACK, RG_TEXT_BIGGER|RG_TEXT_ALIGN_CENTER);
                esp_partition_write(part, 0, buffer, fw_part.dataLength);
            }
            else
            {
                rg_gui_alert("File read error", fw_part.label);
            }
        }
        else
        {
            // Do nothing, could be a data partition
        }

        fseek(fp, nextEntry, SEEK_SET);
    }

    fclose(fp);

    rg_display_clear(C_BLACK);
    rg_gui_alert("Update complete", "All done!");

launcher:
    rg_system_switch_app(RG_APP_LAUNCHER, 0, 0, 0);
}
