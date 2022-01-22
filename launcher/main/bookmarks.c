#include <rg_system.h>
#include <rg_printf.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "bookmarks.h"
#include "gui.h"

static book_t books[BOOK_TYPE_COUNT];


static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);
    // book_t *book = (book_t *)tab->arg;

    if (event == TAB_INIT)
    {
        //
    }
    else if (event == TAB_REFRESH)
    {
        // tab_refresh(tab);
    }
    else if (event == TAB_ENTER || event == TAB_SCROLL)
    {
        if (!tab->is_empty && tab->listbox.length)
            snprintf(tab->status[0].left, 24, "%d / %d", (tab->listbox.cursor + 1) % 10000, tab->listbox.length % 10000);
        else
            snprintf(tab->status[0].left, 24, "No games");
        gui_set_status(tab, NULL, "");

        rg_image_free(tab->preview);
        tab->preview = NULL;
    }
    else if (event == TAB_LEAVE)
    {
        //
    }
    else if (event == TAB_IDLE)
    {
        if (file && !tab->preview && gui.browse && gui.idle_counter == 1)
            gui_load_preview(tab);
        else if ((gui.idle_counter % 100) == 0)
            crc_cache_idle_task(tab);
    }
    else if (event == KEY_PRESS_A)
    {
        if (file)
            emulator_show_file_menu(file, false);
    }
    else if (event == KEY_PRESS_B)
    {
        // This is now reserved for subfolder navigation (go back)
    }
}

static void tab_refresh(book_type_t book_type)
{
    book_t *book = &books[book_type];
    int items_count = 0;

    if (!book->tab)
        return;

    memset(&book->tab->status, 0, sizeof(book->tab->status));

    if (book->count)
    {
        gui_resize_list(book->tab, book->count);
        for (int i = 0; i < book->count; i++)
        {
            retro_emulator_file_t *file = &book->items[i];
            if (file->is_valid)
            {
                listbox_item_t *listitem = &book->tab->listbox.items[items_count++];
                const char *type = file->emulator ? file->emulator->short_name : "n/a";
                snprintf(listitem->text, 128, "[%-3s] %.100s", type, file->name);
                listitem->arg = file;
                listitem->id = i;
            }
        }
    }
    book->tab->is_empty = items_count == 0;

    if (!book->tab->is_empty)
    {
        gui_resize_list(book->tab, items_count);
        gui_sort_list(book->tab);
    }
    else
    {
        gui_resize_list(book->tab, 6);
        sprintf(book->tab->listbox.items[0].text, "Welcome to Retro-Go!");
        sprintf(book->tab->listbox.items[2].text, "You have no %s games", book->name);
        sprintf(book->tab->listbox.items[4].text, "You can hide this tab in the menu");
        book->tab->listbox.cursor = 3;
    }
}

static void book_append(book_type_t book_type, retro_emulator_file_t *new_item)
{
    book_t *book = &books[book_type];

    if (book->capacity <= book->count + 1)
    {
        book->capacity += 10;
        book->items = realloc(book->items, book->capacity * sizeof(retro_emulator_file_t));
    }

    book->items[book->count] = *new_item;
    book->items[book->count].is_valid = true;
    book->count++;
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
                book_append(book_type, &tmp_file);
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
    if (!fp && rg_mkdir(rg_dirname(book->path)))
    {
        fp = fopen(book->path, "w");
    }
    if (fp)
    {
        int remaining = book->max_items;
        for (int i = book->count - 1; i >= 0; i--)
        {
            retro_emulator_file_t *file = &book->items[i];
            if (file->is_valid && remaining-- > 0)
                fprintf(fp, "%s\n", emulator_get_file_path(file));
        }
        fclose(fp);
    }
}

static void book_init(book_type_t book_type, const char *name, const char *desc, size_t max_items)
{
    book_t *book = &books[book_type];
    char path[RG_PATH_MAX + 1];

    sprintf(path, "%s/%s.txt", RG_BASE_PATH_CONFIG, name);

    book->name = strdup(name);
    book->path = strdup(path);
    book->tab = gui_add_tab(name, desc, book, event_handler);
    book->initialized = true;
    book->max_items = max_items;

    if (book_type == BOOK_TYPE_RECENT)
    {
        book->tab->listbox.sort_mode = SORT_ID_ASC;
        book->tab->listbox.cursor = 0;
    }

    book_load(book_type);
    tab_refresh(book_type);
}


retro_emulator_file_t *bookmark_find(book_type_t book_type, retro_emulator_file_t *file)
{
    RG_ASSERT(file, "bad param");

    book_t *book = &books[book_type];

    for (int i = 0; i < book->count; i++)
    {
        if (book->items[i].is_valid && book->items[i].emulator == file->emulator
            && strcmp(book->items[i].name, file->name) == 0)
        {
            return &book->items[i];
        }
    }

    return NULL;
}

bool bookmark_add(book_type_t book, retro_emulator_file_t *file)
{
    RG_ASSERT(file, "bad param");
    // For most book types we want unique entries. I'd prefer to keep the old one and let the calling
    // code decide what to do, but deleting the old entry is simpler for most book types who try
    // to update something... For the RECENT type we also don't want to disturb the order
    retro_emulator_file_t *bookmark;
    while ((bookmark = bookmark_find(book, file)))
        bookmark->is_valid = false;

    book_append(book, file);
    tab_refresh(book);
    book_save(book);

    return true;
}

bool bookmark_remove(book_type_t book, retro_emulator_file_t *file)
{
    RG_ASSERT(file, "bad param");

    retro_emulator_file_t *bookmark;
    int found = 0;

    while ((bookmark = bookmark_find(book, file)))
    {
        bookmark->is_valid = false;
        found++;
    }

    if (found == 0)
        return false;

    tab_refresh(book);
    book_save(book);

    return true;
}

void bookmarks_init(void)
{
    book_init(BOOK_TYPE_FAVORITE, "favorite", "Favorites", 1000);
    book_init(BOOK_TYPE_RECENT, "recent", "Recently played", 32);
}
