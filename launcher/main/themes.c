#include <rg_system.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include "themes.h"
#include "gui.h"


static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);

    if (event == TAB_INIT)
    {
        const char *themes_path = RG_BASE_PATH_THEMES;
        size_t items_count = 1;
        struct dirent* ent;
        DIR* dir;

        memset(&tab->status, 0, sizeof(tab->status));
        gui_resize_list(tab, 1);

        tab->listbox.items[0] = (listbox_item_t){
            .text = "[Built-in theme]",
            .enabled = true,
            .id = -1,
            .arg = NULL,
        };

        if ((dir = opendir(themes_path)))
        {
            RG_LOGI("Scanning directory %s\n", themes_path);

            while ((ent = readdir(dir)))
            {
                if (ent->d_name[0] == '.')
                    continue;

                if (ent->d_type == DT_DIR)
                {
                    gui_resize_list(tab, items_count + 1);
                    listbox_item_t *item = &tab->listbox.items[items_count++];
                    strcpy(item->text, ent->d_name);
                    item->arg = strdup(item->text); // This could be relocated
                    item->enabled = true;
                }
            }
            closedir(dir);
        }

        gui_resize_list(tab, items_count);
        gui_sort_list(tab);

        if (items_count < 2)
        {
            gui_resize_list(tab, 6);
            sprintf(tab->listbox.items[0].text, "Welcome to Retro-Go!");
            sprintf(tab->listbox.items[2].text, "Place themes in folder: %s", themes_path + strlen(RG_ROOT_PATH));
            sprintf(tab->listbox.items[4].text, "You can hide this tab in the menu");
            tab->listbox.cursor = 3;
        }
    }
    else if (event == TAB_REFRESH)
    {
        // tab_refresh(tab);
    }
    else if (event == TAB_ENTER || event == TAB_SCROLL)
    {
        if (item->arg)
            snprintf(tab->status[0].left, 24, "%d / %d", (tab->listbox.cursor + 1) % 10000, tab->listbox.length % 10000);
        else
            strcpy(tab->status[0].left, "No themes");
        gui_set_status(tab, NULL, "");
        gui_set_preview(tab, NULL);
    }
    else if (event == TAB_LEAVE)
    {
        //
    }
    else if (event == TAB_IDLE)
    {
        if (item && item->enabled && !tab->preview && gui.browse && gui.idle_counter == 1)
        {
            char preview_path[RG_PATH_MAX];
            sprintf(preview_path, "%s/%s/preview.png", RG_BASE_PATH_THEMES, item->text);
            gui_set_preview(tab, rg_image_load_from_file(preview_path, 0));
        }
        else if ((gui.idle_counter % 100) == 0)
            crc_cache_idle_task(tab);
    }
    else if (event == TAB_ACTION)
    {
        if (item && item->enabled)
        {
            gui_set_theme(item->arg);
            rg_gui_alert("Success!", "Theme activated.");
        }
    }
    else if (event == TAB_BACK)
    {
        // This is now reserved for subfolder navigation (go back)
    }
}

bool theme_install(const char *filename)
{
    RG_ASSERT(filename, "bad param");

    return true;
}

void themes_init(void)
{
    tab_t *tab = gui_add_tab("themes", "Themes", NULL, event_handler);
    // tab->enabled = false; // Hide by default, will only show when called by settings
    tab->enabled = true;
}
