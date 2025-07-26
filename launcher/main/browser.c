#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "applications.h"
#include "gui.h"

typedef struct
{
    char *name;
    uint32_t size;
    uint8_t type;
    uint8_t _unused1;
    uint8_t _unused2;
    uint8_t _unused3;
} file_t;

static const char *current_path = NULL;
static file_t *files = NULL;
static size_t files_count = 0;
static size_t files_capacity = 0;

static int scan_folder_cb(const rg_scandir_t *entry, void *arg)
{
    uint8_t type = RETRO_TYPE_INVALID;

    if (entry->is_file)
    {
        type = RETRO_TYPE_FILE;
    }
    else if (entry->is_dir)
    {
        type = RETRO_TYPE_FOLDER;
    }

    if (type == RETRO_TYPE_INVALID)
        return RG_SCANDIR_CONTINUE;

    if (files_count + 1 > files_capacity)
    {
        size_t new_capacity = (files_capacity * 1.5) + 1;
        file_t *new_buf = realloc(files, new_capacity * sizeof(file_t));
        if (!new_buf)
            return RG_SCANDIR_STOP;
        files = new_buf;
        files_capacity = new_capacity;
    }

    files[files_count++] = (file_t) {
        .name = strdup(entry->basename),
        .type = type,
    };

    return RG_SCANDIR_CONTINUE;
}

static void scan_folder(tab_t *tab, const char *path, const char *selection)
{
    size_t items_count = 0;

    for (size_t i = 0; i < files_count; ++i)
        free(files[i].name);

    current_path = rg_unique_string(path);
    files_count = 0;

    memset(&tab->status, 0, sizeof(tab->status));
    tab->navpath = current_path;

    rg_storage_scandir(current_path, scan_folder_cb, NULL, 0);

    RG_LOGI("Found %d files", (int)files_count);

    gui_resize_list(tab, files_count);
    tab->listbox.cursor = 0;

    for (size_t i = 0; i < files_count; i++)
    {
        file_t *file = &files[i];

        if (file->type == RETRO_TYPE_INVALID || !file->name)
            continue;

        if (file->type == RETRO_TYPE_FOLDER)
        {
            listbox_item_t *item = &tab->listbox.items[items_count++];
            snprintf(item->text, sizeof(item->text), "[%.40s]", file->name);
            item->group = 1;
            item->arg = file;
        }
        else if (file->type == RETRO_TYPE_FILE)
        {
            listbox_item_t *item = &tab->listbox.items[items_count++];
            snprintf(item->text, sizeof(item->text), "%s", file->name);
            item->group = 2;
            item->arg = file;
        }
    }

    gui_resize_list(tab, items_count);
    gui_sort_list(tab);

    if (selection)
    {
        for (int i = 0; i < tab->listbox.length; i++)
        {
            file_t *file = tab->listbox.items[i].arg;
            if (file && strcmp(file->name, selection) == 0)
            {
                tab->listbox.cursor = i;
                break;
            }
        }
    }

    gui_scroll_list(tab, SCROLL_SET, tab->listbox.cursor);
}

static void show_file_menu(const char *path)
{
    rg_stat_t info = rg_storage_stat(path);

    if (!info.exists)
    {
        rg_gui_alert(_("File not found"), path);
        return;
    }

    const rg_gui_option_t options[] = {
        {0, _("Open file with..."), NULL, 1, NULL},
        {0, _("Rename file"), NULL, 1, NULL},
        {0, _("Copy file"), NULL, 1, NULL},
        {0, _("Move file"), NULL, 1, NULL},
        {0, _("Delete file"), NULL, 1, NULL},
        {0, _("Close"), NULL, 1, NULL},
        RG_DIALOG_END,
    };

    rg_gui_dialog(rg_basename(path), options, -1);
}

static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    file_t *file = (file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        char *selection = file ? strdup(file->name) : NULL;
        scan_folder(tab, current_path, selection);
        free(selection);
    }
    else if (event == TAB_DEINIT)
    {
        //
    }
    else if (event == TAB_ENTER || event == TAB_LEAVE || event == TAB_SCROLL)
    {
        gui_set_status(tab, NULL, "");
        gui_set_preview(tab, NULL);
        tab->navpath = current_path;
    }
    else if (event == TAB_IDLE)
    {
        //
    }
    else if (event == TAB_ACTION)
    {
        if (file)
        {
            char pathbuf[RG_PATH_MAX + 1];
            snprintf(pathbuf, RG_PATH_MAX, "%s/%s", current_path, file->name);
            if (file->type == RETRO_TYPE_FOLDER)
                scan_folder(tab, pathbuf, NULL);
            else
                show_file_menu(pathbuf);
        }
    }
    else if (event == TAB_BACK)
    {
        if (current_path == rg_unique_string(RG_STORAGE_ROOT))
        {
            // Can't go higher than our storage root because esp-idf won't show anything
            tab->navpath = NULL;
        }
        else
        {
            char *selection = strdup(rg_basename(current_path));
            scan_folder(tab, rg_dirname(current_path), selection);
            free(selection);
        }
    }
}

void browser_init(void)
{
    current_path = rg_unique_string(RG_STORAGE_ROOT);
    gui_add_tab("browser", "File Manager", NULL, event_handler);
}
