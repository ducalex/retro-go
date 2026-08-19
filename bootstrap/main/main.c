#include <rg_system.h>
#include <esp_partition.h>
#include <esp_flash.h>
#include <esp_ota_ops.h>
#include <stdio.h>
#include <string.h>

#define ALIGN_BLOCK(val, alignment) ((int)(((val) + (alignment - 1)) / alignment) * alignment)

static size_t gp_buffer_size = 0x10000;
static void *gp_buffer = NULL;

void app_main(void)
{
    rg_app_t *app = rg_system_init(&(const rg_config_t){
        .storageRequired = true,
        .isLauncher = false,
    });

    char *filename = NULL;

    if (app->romPath && strlen(app->romPath) > 0)
    {
        filename = strdup(app->romPath);
    }
    else
    {
        filename = rg_gui_file_picker("Select App", RG_BASE_PATH_ROMS "/apps", NULL, true, true);
        if (!filename || !*filename)
        {
            rg_system_exit();
        }
    }

    rg_display_clear(C_BLACK);

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        rg_gui_alert("Error", "Failed to open file");
        free(filename);
        rg_system_exit();
    }

    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    const esp_partition_t *dst_part = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, "bootstrapped");
    if (!dst_part)
    {
        rg_gui_alert("Error", "Could not find 'bootstrapped' partition");
        fclose(fp);
        free(filename);
        rg_system_exit();
    }

    char *target_name = rg_settings_get_string(NS_GLOBAL, "NextBootName", NULL);
    char *target_args = rg_settings_get_string(NS_GLOBAL, "NextBootArgs", NULL);
    int target_slot = rg_settings_get_number(NS_GLOBAL, "NextBootSlot", -1);
    int target_flags = rg_settings_get_number(NS_GLOBAL, "NextBootFlags", 0);

    char *last_flashed = rg_settings_get_string(NS_GLOBAL, "LastFlashedApp", NULL);
    if (last_flashed && strcmp(filename, last_flashed) == 0)
    {
        if (target_name || target_args)
        {
            free(last_flashed);
            fclose(fp);
            free(filename);
            rg_settings_delete(NS_GLOBAL, "NextBootName");
            rg_settings_delete(NS_GLOBAL, "NextBootArgs");
            rg_settings_delete(NS_GLOBAL, "NextBootSlot");
            rg_settings_delete(NS_GLOBAL, "NextBootFlags");
            rg_settings_commit();
            rg_system_switch_app(dst_part->label, target_name, target_args, target_slot, target_flags);
            return;
        }

        const rg_gui_option_t options[] = {
            {0, "This app was flashed previously.", NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
            {0, "", NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
            {1, "Boot now", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
            {2, "Reflash", NULL, RG_DIALOG_FLAG_NORMAL, NULL},
            RG_DIALOG_END,
        };
        if (rg_gui_dialog("Already Flashed", options, -2) == 1)
        {
            free(last_flashed);
            fclose(fp);
            free(filename);
            rg_system_switch_app(dst_part->label, NULL, NULL, 0, 0);
            return;
        }
    }
    free(last_flashed);

    if (file_size > dst_part->size)
    {
        rg_gui_alert("Error", "File is larger than the 'bootstrapped' partition");
        fclose(fp);
        free(target_name);
        free(target_args);
        free(filename);
        rg_system_exit();
    }

    rg_gui_option_t lines[4] = {0};
    lines[0] = (rg_gui_option_t){0, "Flashing App...", NULL, RG_DIALOG_FLAG_NORMAL, NULL};
    lines[1] = (rg_gui_option_t)RG_DIALOG_SEPARATOR;
    lines[2] = (rg_gui_option_t){0, "Pending", NULL, RG_DIALOG_FLAG_DISABLED, NULL};
    lines[3] = (rg_gui_option_t)RG_DIALOG_END;

    rg_gui_draw_dialog("Progress", lines, 1, 2);

    lines[2].value = "Initializing OTA...";
    rg_gui_draw_dialog("Progress", lines, 1, 2);

    esp_ota_handle_t update_handle = 0;
    if (esp_ota_begin(dst_part, file_size, &update_handle) != ESP_OK)
    {
        rg_gui_alert("Error", "OTA Init failed");
        fclose(fp);
        free(target_name);
        free(target_args);
        free(filename);
        rg_system_exit();
    }

    gp_buffer = rg_alloc(gp_buffer_size, MEM_SLOW);
    int offset = 0;
    int size = file_size;

    while (size > 0)
    {
        int chunk_size = RG_MIN(size, gp_buffer_size);
        char progress[32];
        snprintf(progress, sizeof(progress), "Writing %d%%", (offset * 100) / file_size);
        lines[2].value = progress;
        rg_gui_draw_dialog("Progress", lines, 1, 2);

        if (fread(gp_buffer, 1, chunk_size, fp) != chunk_size)
        {
            rg_gui_alert("Error", "Read failed");
            break;
        }

        if (esp_ota_write(update_handle, gp_buffer, chunk_size) != ESP_OK)
        {
            rg_gui_alert("Error", "Write failed");
            break;
        }

        offset += chunk_size;
        size -= chunk_size;
        
        rg_system_tick(0);
    }

    fclose(fp);
    free(gp_buffer);

    if (size == 0)
    {
        if (esp_ota_end(update_handle) != ESP_OK)
        {
            rg_gui_alert("Error", "OTA Finalize failed");
            free(target_name);
            free(target_args);
            free(filename);
            rg_system_exit();
        }

        lines[2].value = "Complete!";
        rg_gui_draw_dialog("Progress", lines, 1, 2);
        rg_task_delay(500);

        rg_settings_set_string(NS_GLOBAL, "LastFlashedApp", filename);
        if (target_name || target_args)
        {
            rg_settings_delete(NS_GLOBAL, "NextBootName");
            rg_settings_delete(NS_GLOBAL, "NextBootArgs");
            rg_settings_delete(NS_GLOBAL, "NextBootSlot");
            rg_settings_delete(NS_GLOBAL, "NextBootFlags");
        }
        rg_settings_commit();

        free(filename);

        rg_system_switch_app(dst_part->label, target_name, target_args, target_slot, target_flags);
    }
    else
    {
        esp_ota_abort(update_handle);
        free(target_name);
        free(target_args);
        free(filename);
        rg_system_exit();
    }
}
