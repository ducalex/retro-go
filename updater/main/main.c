#include <rg_system.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <esp_flash_partitions.h>
#include <esp_partition.h>
#include <esp_flash.h>
#include <esp_ota_ops.h>

#define MAX_PARTITIONS (24) // ESP_PARTITION_TABLE_MAX_ENTRIES

#define RETRO_GO_IMG_MAGIC "RG_IMG_0"
typedef struct
{
    uint8_t magic[8];
    char name[28];
    char version[28];
    char target[28];
    uint32_t timestamp;
    uint32_t crc32;
    char reserved[156];
} img_info_t;

typedef struct
{
    char name[17];
    struct {int offset, size;} src;
    struct {int offset, size;} dst;
} flash_task_t;

static size_t gp_buffer_size = 0x10000;
static void *gp_buffer = NULL;
static rg_app_t *app;

#define FORMAT(message...) ({snprintf(message_buffer, sizeof(message_buffer), message); message_buffer; })
#define TRY(cond, error_message...)                      \
    RG_LOGD("Line: %s", #cond);                          \
    if (!(cond))                                         \
    {                                                    \
        rg_gui_alert(_("Error"), FORMAT(error_message)); \
        goto fail;                                       \
    }
#define ALIGN_BLOCK(val, alignment) ((int)(((val) + (alignment - 1)) / alignment) * alignment)
#define CHECK_OVERLAP(start1, end1, start2, end2) ((end1) > (start2) && (end2) > (start1))

static bool fread_at(void *output, int offset, int length, FILE *fp)
{
    if (offset < 0 && !(fseek(fp, 0, SEEK_END) == 0 && fseek(fp, offset, SEEK_CUR) == 0))
        return false;
    else if (offset >= 0 && !(fseek(fp, offset, SEEK_SET) == 0))
        return false;
    return fread(output, length, 1, fp) == 1;
}

static bool parse_file(esp_partition_info_t *partition_table, size_t *num_partitions, FILE *fp)
{
    char message_buffer[256];
    img_info_t img_info;
    int _num_partitions;

    TRY(fread_at(&img_info, -sizeof(img_info_t), sizeof(img_info_t), fp), "File read failed");
    img_info.name[sizeof(img_info.name) - 1] = 0;       // Just in case
    img_info.version[sizeof(img_info.version) - 1] = 0; // Just in case
    img_info.target[sizeof(img_info.target) - 1] = 0;   // Just in case

    if (memcmp(img_info.magic, RETRO_GO_IMG_MAGIC, 8) == 0) // Valid retro-go image
    {
        if (strcasecmp(img_info.target, RG_TARGET_NAME) != 0) // Wrong target
        {
            FORMAT("The file appears to be for a different device.\n"
                   "Current device: %s\n"
                   "Image device: %s\n\n"
                   "Do you want to continue anyway?",
                   RG_TARGET_NAME,
                   img_info.target);
            if (!rg_gui_confirm("Warning", message_buffer, false))
                goto fail;
        }
        // TODO: Validate checksum
        FORMAT("Current version: %s\nNew version: %s\n\nContinue?", app->version, img_info.version);
        if (!rg_gui_confirm("Warning", message_buffer, false))
            goto fail;
    }
    else if (!rg_gui_confirm("Warning", "File is not a valid Retro-Go image.\nContinue anyway?", false))
    {
        goto fail;
    }
    TRY(fread_at(gp_buffer, ESP_PARTITION_TABLE_OFFSET, ESP_PARTITION_TABLE_MAX_LEN, fp), "File read failed");
    TRY(esp_partition_table_verify((const esp_partition_info_t *)gp_buffer, true, &_num_partitions) == ESP_OK, "File is not a valid ESP32 image.");
    memcpy(partition_table, gp_buffer, sizeof(esp_partition_info_t) * _num_partitions);
    *num_partitions = _num_partitions;
    return true;
fail:
    // TODO: Also support images that truncate the first 0x1000, just in case
    // TODO: Also support .fw, which should be trivial to parse albeit lacking some meta data
    return false;
}

static bool process_queue(flash_task_t *queue, size_t queue_count, FILE *fp)
{
    char message_buffer[256];
    rg_gui_option_t lines[queue_count + 4];
    // size_t lines_count = 0;

    for (size_t i = 0; i < queue_count; ++i)
        lines[i] = (rg_gui_option_t){0, queue[i].name, NULL, RG_DIALOG_FLAG_NORMAL, NULL};
    lines[queue_count + 0] = (rg_gui_option_t)RG_DIALOG_SEPARATOR;
    lines[queue_count + 1] = (rg_gui_option_t){5, "Proceed!", NULL, RG_DIALOG_FLAG_NORMAL, NULL};
    lines[queue_count + 2] = (rg_gui_option_t)RG_DIALOG_END;

    if (rg_gui_dialog("Partitions to be updated:", lines, -1) != 5)
        goto fail;

#define TRY_F(msg, cond, err)                                  \
    RG_LOGD("Line: %s", #cond);                                \
    rg_display_clear(C_BLACK);                                 \
    lines[i].flags = RG_DIALOG_FLAG_NORMAL;                    \
    lines[i].value = msg;                                      \
    rg_gui_draw_dialog("Progress", lines, queue_count, i);     \
    if (!(cond))                                               \
    {                                                          \
        lines[i].value = err;                                  \
        rg_gui_draw_dialog("Progress", lines, queue_count, i); \
        errors++;                                              \
        break;                                                 \
    }

    int errors = 0;

    for (size_t i = 0; i < queue_count; ++i)
        lines[i].value = "Pending", lines[i].flags = RG_DIALOG_FLAG_DISABLED;

    for (size_t i = 0; i < queue_count; ++i)
    {
        const flash_task_t *t = &queue[i];
        TRY_F("Erasing", esp_flash_erase_region(NULL, t->dst.offset, ALIGN_BLOCK(t->dst.size, 0x1000)) == ESP_OK || true, "Erase err");
        int offset = 0, size = t->src.size;
        while (size > 0)
        {
            int chunk_size = RG_MIN(size, gp_buffer_size);
            TRY_F("Reading", fread_at(gp_buffer, t->src.offset + offset, chunk_size, fp), "Read err");
            TRY_F("Writing", esp_flash_write(NULL, gp_buffer, t->dst.offset + offset, ALIGN_BLOCK(chunk_size, 0x1000)) == ESP_OK, "Write err");
            offset += chunk_size;
            size -= chunk_size;
        }
        TRY_F("Complete", size == 0, "Failed");
    }

    lines[queue_count + 0] = (rg_gui_option_t){0, FORMAT("  - %d errors -  ", errors), NULL, RG_DIALOG_FLAG_NORMAL, NULL};
    lines[queue_count + 1] = (rg_gui_option_t){0, "Complete!", NULL, RG_DIALOG_FLAG_NORMAL, NULL};
    lines[queue_count + 2] = (rg_gui_option_t)RG_DIALOG_END;
    rg_gui_dialog("Complete", lines, -1);
    return true;
fail:
    return false;
}

static bool do_update(const char *filename)
{
    esp_partition_info_t partition_table[MAX_PARTITIONS] = {0};
    size_t num_partitions = 0;
    flash_task_t queue[MAX_PARTITIONS] = {0};
    size_t queue_count = 0;
    char message_buffer[256];
    FILE *fp = NULL;

    RG_LOGI("Filename: %s", filename);
    rg_display_clear(C_BLACK);

    TRY(fp = fopen(filename, "rb"), "File open failed");
    if (!parse_file(partition_table, &num_partitions, fp))
        goto fail;

    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    const esp_partition_t *factory_partition = esp_partition_find_first(0x00, 0x00, NULL);
    bool dangerous = false;

    RG_LOGW("Checking if we can perform a standard update...");
    size_t updatable_partitions = 0;
    for (size_t i = 0; i < num_partitions; ++i)
    {
        const esp_partition_info_t *src = &partition_table[i];
        const esp_partition_t *dst = esp_partition_find_first(src->type, ESP_PARTITION_SUBTYPE_ANY, (char *)src->label);

        if (src->type != PART_TYPE_APP || (src->subtype & 0xF0) != PART_SUBTYPE_OTA_FLAG)
            RG_LOGW("Skipping partition %.16s: Unsupported type.", (char *)src->label);
        else if (src->pos.offset < 0x10000)
            RG_LOGW("Skipping partition %.16s: Forbidden region.", (char *)src->label);
        else if (dst == running_partition)
            RG_LOGW("Skipping partition %.16s: Currently running.", (char *)src->label);
        else
        {
            updatable_partitions++;
            if (!dst)
                RG_LOGW("Skipping partition %.16s: No match found.", (char *)src->label);
            else if (dst->size < src->pos.size)
                RG_LOGW("Skipping partition %.16s: New partition is bigger.", (char *)src->label);
            else
            {
                RG_LOGI("Partition %.16s can be updated!", (char *)src->label);
                flash_task_t *task = memset(&queue[queue_count++], 0, sizeof(flash_task_t));
                memcpy(task->name, src->label, 16);
                task->src.offset = src->pos.offset;
                task->src.size = src->pos.size;
                task->dst.offset = dst->address;
                task->dst.size = dst->size;
            }
        }
    }

    if (updatable_partitions != queue_count)
    {
        // Some partitions can't be updated with this method, we have to attempt a dangerous update instead!
        RG_LOGW("Some partitions cannot be updated in this mode. updatable_partitions=%d, queue_count=%d",
            updatable_partitions, queue_count);
    #if CONFIG_SPI_FLASH_DANGEROUS_WRITE_ALLOWED
        // The presence of a factory app that is NOT the current app indicates that we're controlled by
        // odroid-go-firmware or odroid-go-multifirmware and it isn't safe to do arbitrary flash writes
        if (factory_partition && factory_partition != running_partition)
            rg_gui_alert(_("Error"), "Probably running under odroid-go-multifirmware, cannot do full update!");
        else
            dangerous = true;
    #else
        rg_gui_alert(_("Error"), "CONFIG_SPI_FLASH_DANGEROUS_WRITE_ALLOWED is not set, cannot do full update!\n");
    #endif
    }

    if (dangerous)
    {
        RG_LOGW("Performing a dangerous update!");
        queue_count = 0;
        for (size_t i = 0; i < num_partitions; ++i)
        {
            const esp_partition_info_t *src = &partition_table[i];
            if (src->type != PART_TYPE_APP || (src->subtype & 0xF0) != PART_SUBTYPE_OTA_FLAG)
                RG_LOGW("Skipping partition %.16s: Unsupported type.", (char *)src->label);
            else if (src->pos.offset < 0x10000)
                RG_LOGW("Skipping partition %.16s: Forbidden region.", (char *)src->label);
            // FIXME: Instead of skipping an overlaping partition, we should attempt to relocate it.
            else if (CHECK_OVERLAP(src->pos.offset, src->pos.offset + src->pos.size,
                                running_partition->address, running_partition->address + running_partition->size))
                RG_LOGW("Skipping partition %.16s: Overlap with self.", (char *)src->label);
            else
            {
                RG_LOGI("Partition %.16s can be updated!", (char *)src->label);
                flash_task_t *task = memset(&queue[queue_count++], 0, sizeof(flash_task_t));
                memcpy(task->name, src->label, 16);
                task->src.offset = src->pos.offset;
                task->src.size = src->pos.size;
                task->dst.offset = src->pos.offset;
                task->dst.size = src->pos.size;
            }
        }

        queue[queue_count++] = (flash_task_t){
            .name = "partition_table",
            .src = {ESP_PARTITION_TABLE_OFFSET, ESP_PARTITION_TABLE_MAX_LEN},
            .dst = {ESP_PARTITION_TABLE_OFFSET, ESP_PARTITION_TABLE_MAX_LEN},
        };
    }

    rg_display_clear(C_BLACK);

    if (!process_queue(queue, queue_count, fp))
        goto fail;

    fclose(fp);
    return true;
fail:
    if (fp)
        fclose(fp);
    return false;
}

void app_main(void)
{
    app = rg_system_init(&(const rg_config_t){
        .storageRequired = true,
        // This is a hack to hide the status bar with bogus speed info. A better solution will be
        // needed as we still want the battery icon!
        .isLauncher = true,
    });

    gp_buffer = rg_alloc(gp_buffer_size, MEM_FAST);
    // const char *filename = app->romPath;

    while (true)
    {
        char *filename = rg_gui_file_picker("Select update", RG_BASE_PATH_UPDATES, NULL, true, true);
        rg_display_clear(C_BLACK);
        if (!filename || !*filename)
            break;
        if (do_update(filename))
            break;
        free(filename);
    }

    free(gp_buffer);
    rg_system_exit();
}
