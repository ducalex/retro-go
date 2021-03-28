#include <rg_system.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "favorites.h"
#include "images.h"
#include "gui.h"

static favorite_t *favorites;
static int favorites_count = 0;
static int recent_count = 0;
static tab_t *recent_tab;
static tab_t *fav_tab;

static const char *SETTING_FAVORITES = "Favorites";
static const char *SETTING_RECENT = "Recent";


static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        // I know this happens twice, this is needed for now...
        // Update 2021: I don't remember WHY it is needed, is it still the case..?
        // favorites_load();

        if (tab == fav_tab && favorites_count == 0)
        {
            fav_tab->listbox.cursor = 3;
        }
    }

    if (file == NULL)
        return;

    if (event == KEY_PRESS_A)
    {
        emulator_show_file_menu(file);
        gui_redraw();
    }
    else if (event == KEY_PRESS_B)
    {
        emulator_show_file_info(file);
        gui_redraw();
    }
    else if (event == TAB_IDLE)
    {
        if (gui.show_preview && gui.idle_counter == (gui.show_preview_fast ? 1 : 8))
            gui_draw_preview(file);

        if (gui.idle_counter == 1000)
            crc_cache_populate_idle();
    }
    else if (event == TAB_REDRAW)
    {
        // if (gui.show_preview)
        //     gui_draw_preview(file);
    }
}

void favorites_load()
{
    char *favorites_str = rg_settings_get_string(SETTING_FAVORITES, "");
    char *temp_ptr = favorites_str;

    favorites_count = 0;
    free(favorites);

    while (*temp_ptr)
        if (*temp_ptr++ == '\n')
            favorites_count++;

    favorites = calloc(favorites_count + 1, sizeof(favorite_t));
    gui_resize_list(fav_tab, favorites_count);

    char *token = strtok(favorites_str, "\n");
    int pos = 0;

    while (token != NULL)
    {
        favorite_t *favorite = &favorites[pos];
        listbox_item_t *listitem = &fav_tab->listbox.items[pos];

        if (emulator_build_file_object(token, &favorite->file))
        {
            snprintf(favorite->path, 168, "%s", token);
            snprintf(listitem->text, 128, "[%-3s] %s", favorite->file.ext, favorite->file.name);
            listitem->arg = &favorite->file;
            pos++;
        }
        else
        {
            RG_LOGW("Unknown favorite: '%s'\n", token);
            favorites_count--;
        }

        token = strtok(NULL, "\n");
    }

    free(favorites_str);

    if (favorites_count > 0)
    {
        sprintf(fav_tab->status, "Favorites: %d", favorites_count);
        gui_resize_list(fav_tab, favorites_count);
        gui_sort_list(fav_tab, 0);
        fav_tab->is_empty = false;
    }
    else
    {
        sprintf(fav_tab->status, "No favorites");
        gui_resize_list(fav_tab, 6);
        sprintf(fav_tab->listbox.items[0].text, "Welcome to Retro-Go!");
        sprintf(fav_tab->listbox.items[2].text, "You have no favorites.");
        sprintf(fav_tab->listbox.items[4].text, "Use SELECT and START to navigate.");
        fav_tab->listbox.cursor = 3;
        fav_tab->is_empty = true;
    }
}

void favorites_save()
{
    char *buffer = calloc(favorites_count, 128);

    for (int i = 0; i < favorites_count; i++)
    {
        if (!favorites[i].removed) {
            strcat(buffer, favorites[i].path);
            strcat(buffer, "\n");
        }
    }

    rg_settings_set_string(SETTING_FAVORITES, buffer);
    rg_settings_save();
    free(buffer);
}

favorite_t *favorite_find(retro_emulator_file_t *file)
{
    for (int i = 0; i < favorites_count; i++)
    {
        if (favorites[i].file.emulator == file->emulator
            && strcmp(favorites[i].file.name, file->name) == 0)
        {
            return &favorites[i];
        }
    }

    return NULL;
}

bool favorite_add(retro_emulator_file_t *file)
{
    // There's always one free slot at the end
    favorite_t *favorite = &favorites[favorites_count++];

    sprintf(favorite->path, "%s/%s.%s", file->folder, file->name, file->ext);
    favorite->removed = false;

    favorites_save();
    favorites_load();

    return true;
}

bool favorite_remove(retro_emulator_file_t *file)
{
    favorite_t *favorite = favorite_find(file);

    if (favorite == NULL)
        return false;

    favorite->removed = true;

    // Lazy lazy lazy
    favorites_save();
    favorites_load();

    return true;
}

void favorites_init()
{
    fav_tab = gui_add_tab(
        "favorites",
        rg_gui_load_image(logo_fav.data, logo_fav.size),
        rg_gui_load_image(header_fav.data, header_fav.size),
        NULL,
        event_handler);

    // recent_tab = gui_add_tab(
    //     "recent",
    //     rg_gui_load_image(logo_recent.data, logo_recent.size),
    //     rg_gui_load_image(header_recent.data, header_recent.size),
    //     NULL,
    //     event_handler);

    // We must load favorites now because other tabs depend on it for menu items
    favorites_load();
}
