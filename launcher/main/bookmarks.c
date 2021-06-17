#include <rg_system.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "bookmarks.h"
#include "images.h"
#include "gui.h"

static book_t books[BOOK_TYPE_COUNT];


static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);
    book_t *book = (book_t *)tab->arg;

    if (event == TAB_INIT)
    {
        if (tab->is_empty)
        {
            tab->listbox.cursor = 3;
        }
    }
    else if (event == TAB_ENTER)
    {
        //
    }
    else if (event == TAB_LEAVE)
    {
        //
    }
    else if (event == TAB_SCROLL)
    {
        if (!tab->is_empty && tab->listbox.length)
            snprintf(tab->status[0].left, 24, "%d / %d", (tab->listbox.cursor + 1) % 10000, tab->listbox.length % 10000);
        else
            snprintf(tab->status[0].left, 24, "No %.20s", book ? book->name : "bookmark");
        gui_set_status(tab, NULL, "");
    }
    else if (event == TAB_REDRAW)
    {
        // gui_draw_preview(tab, file);
    }
    else if (event == TAB_IDLE)
    {
        if (file && gui.show_preview && gui.idle_counter == (gui.show_preview_fast ? 1 : 8))
            gui_draw_preview(tab, file);
        else if ((gui.idle_counter % 100) == 0)
            crc_cache_idle_task(tab);
    }
    else if (event == KEY_PRESS_A)
    {
        if (file)
            emulator_show_file_menu(file);
        gui_redraw();
    }
    else if (event == KEY_PRESS_B)
    {
        if (file)
            emulator_show_file_info(file);
        gui_redraw();
    }
}

static void tab_refresh(book_type_t book_type)
{
    book_t *book = &books[book_type];
    int list_index = 0;

    if (book->tab)
    {
        memset(&book->tab->status, 0, sizeof(book->tab->status));
        book->tab->is_empty = book->count <= 0;

        if (book->count > 0)
        {
            gui_resize_list(book->tab, book->count);
            for (int i = 0; i < book->count; i++)
            {
                retro_emulator_file_t *file = &book->items[i];
                if (file->name[0])
                {
                    listbox_item_t *listitem = &book->tab->listbox.items[list_index++];
                    snprintf(listitem->text, 128, "[%-3s] %.100s", file->ext, file->name);
                    listitem->arg = file;
                }
            }
            gui_resize_list(book->tab, list_index);
            gui_sort_list(book->tab, book->sort_mode);
        }
        else
        {
            gui_resize_list(book->tab, 6);
            sprintf(book->tab->listbox.items[0].text, "Welcome to Retro-Go!");
            sprintf(book->tab->listbox.items[2].text, "You have no %s games.", book->name);
            sprintf(book->tab->listbox.items[4].text, "Use SELECT and START to navigate.");
            book->tab->listbox.cursor = 3;
        }
    }
}

static retro_emulator_file_t *book_get_free_slot(book_type_t book_type)
{
    book_t *book = &books[book_type];

    // for (int i = 0; i < book->count; i++)
    //     if (book->items[i].removed)

    if (book->capacity <= book->count + 1)
    {
        book->capacity += 10;
        book->items = realloc(book->items, book->capacity * sizeof(retro_emulator_file_t));
    }

    return &book->items[book->count++];
}

static void book_load(book_type_t book_type)
{
    book_t *book = &books[book_type];
    retro_emulator_file_t tmp_file;
    char line_buffer[169] = {0};

    FILE *fp = fopen(book->path, "r");
    if (fp)
    {
        book->count = 0;

        while (fgets(line_buffer, 168, fp))
        {
            size_t len = strlen(line_buffer);
            if (line_buffer[len - 1] == '\n')
                line_buffer[len - 1] = 0;

            if (emulator_build_file_object(line_buffer, &tmp_file))
            {
                *book_get_free_slot(book_type) = tmp_file;
            }
            else
                RG_LOGW("Unknown path form: '%s'\n", line_buffer);
        }
        fclose(fp);
    }
}

static void book_save(book_type_t book_type)
{
    book_t *book = &books[book_type];

    FILE *fp = fopen(book->path, "w");
    if (fp)
    {
        for (int i = 0; i < book->count; i++)
        {
            retro_emulator_file_t *file = &book->items[i];
            if (file->folder[0] && file->name[0])
                fprintf(fp, "%s/%s.%s\n", file->folder, file->name, file->ext);
        }
        fclose(fp);
    }
}

static void book_init(book_type_t book_type, const char *name, int sort_mode, const binfile_t *logo, const binfile_t *header)
{
    rg_image_t *logo_img = logo ? rg_image_load_from_memory(logo->data, logo->size, 0) : NULL;
    rg_image_t *header_img = header ? rg_image_load_from_memory(header->data, header->size, 0) : NULL;

    book_t *book = &books[book_type];

    strcpy(book->name, name);
    sprintf(book->path, "%s/%s.txt", RG_BASE_PATH_SYS, book->name);
    book->tab = gui_add_tab(name, logo_img, header_img, book, event_handler);
    book->sort_mode = sort_mode;
    book->initialized = true;

    book_load(book_type);
    tab_refresh(book_type);
}


retro_emulator_file_t *bookmark_find(book_type_t book_type, retro_emulator_file_t *file)
{
    book_t *book = &books[book_type];

    RG_ASSERT(file, "bad param");

    for (int i = 0; i < book->count; i++)
    {
        if (book->items[i].emulator == file->emulator
            && strcmp(book->items[i].name, file->name) == 0)
        {
            return &book->items[i];
        }
    }

    return NULL;
}

bool bookmark_add(book_type_t book, retro_emulator_file_t *file)
{
    // For most book types we want unique entries. I'd prefer to keep the old one and let the calling
    // code decide what to do, but deleting the old entry is simpler for most book types who try
    // to update something... For the RECENT type we also don't want to disturb the order
    retro_emulator_file_t *bookmark;
    while ((bookmark = bookmark_find(book, file)))
        *bookmark = (retro_emulator_file_t){0};
    // if (bookmark_find(book, file))
    //     return false;

    *book_get_free_slot(book) = *file;
    tab_refresh(book);
    book_save(book);

    return true;
}

bool bookmark_remove(book_type_t book, retro_emulator_file_t *file)
{
    RG_ASSERT(file, "bad param");

    retro_emulator_file_t *bookmark = bookmark_find(book, file);
    if (bookmark)
    {
        *bookmark = (retro_emulator_file_t){0};
        tab_refresh(book);
        book_save(book);
        return true;
    }
    return false;
}

void bookmarks_init()
{
    // In older retro-go <= 1.25 the favorites were stored in retro-go.json
    // Try to import and then clear them...
    char *old_favorites = rg_settings_get_string("Favorites", NULL);
    if (old_favorites && old_favorites[0])
    {
        RG_LOGI("Importing old favorites....\n");
        FILE *fp = fopen(RG_BASE_PATH_SYS "/favorite.txt", "a");
        if (fp)
        {
            fputs(old_favorites, fp);
            fclose(fp);
            rg_settings_set_string("Favorites", "");
        }
        free(old_favorites);
    }

    book_init(BOOK_TYPE_FAVORITE, "favorite", 0, &logo_fav, &header_fav);
    book_init(BOOK_TYPE_RECENT, "recent", -1, &logo_recent, &header_recent);
}
