#include <odroid_system.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "favorites.h"
#include "bitmaps.h"
#include "gui.h"

#define KEY_FAVORITES "Favorites"

static favorite_t *favorites;
static int favorites_count = 0;
static tab_t *fav_tab;

static void favorites_load();

static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        // I know this happens twice, this is needed for now...
        favorites_load();
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
        if (file->checksum == 0)
            emulator_crc32_file(file);

        if (gui.show_cover && gui.idle_counter == (gui.show_cover == 1 ? 8 : 1))
            gui_draw_cover(file);
    }
    else if (event == TAB_REDRAW)
    {
        if (gui.show_cover)
            gui_draw_cover(file);
    }
}

static void favorites_load()
{
    char *favorites_str = odroid_settings_string_get(KEY_FAVORITES, "");
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

        if (emulator_build_file_object(token, &favorite->file))
        {
            strcpy(favorite->path, token);
            sprintf(favorite->name, "[%-3s] %s", favorite->file.ext, favorite->file.name);
            strcpy(fav_tab->listbox.items[pos].text, favorite->name);
            fav_tab->listbox.items[pos].arg = &favorite->file;
            pos++;
        }
        else
        {
            printf("Unknown favorite: '%s'\n", token);
            favorites_count--;
        }

        token = strtok(NULL, "\n");
    }

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

static void favorites_save()
{
    char *buffer = calloc(favorites_count, 128);

    for (int i = 0; i < favorites_count; i++)
    {
        if (!favorites[i].removed) {
            strcat(buffer, favorites[i].path);
            strcat(buffer, "\n");
        }
    }

    odroid_settings_string_set(KEY_FAVORITES, buffer);
    odroid_settings_commit();
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
    fav_tab = gui_add_tab("favorites", &logo_fav, NULL, NULL, event_handler);

    // We must load favorites now because other tabs depend on it for menu items
    favorites_load();
}
