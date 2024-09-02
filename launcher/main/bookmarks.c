#include <rg_system.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bookmarks.h"
#include "gui.h"

static book_t books[BOOK_TYPE_COUNT];


static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_file_t *file = (retro_file_t *)(item ? item->arg : NULL);
    // book_t *book = (book_t *)tab->arg;

    if (event == TAB_INIT || event == TAB_RESCAN)
    {
        //
    }
    else if (event == TAB_REFRESH)
    {
        // tab_refresh(tab);
    }
    else if (event == TAB_ENTER || event == TAB_SCROLL)
    {
        gui_set_status(tab, NULL, "");
        gui_set_preview(tab, NULL);
    }
    else if (event == TAB_LEAVE)
    {
        //
    }
    else if (event == TAB_IDLE)
    {
        if (file && !tab->preview && gui.browse && gui.idle_counter == 1)
            gui_load_preview(tab);
    }
    else if (event == TAB_ACTION)
    {
        if (file)
            application_show_file_menu(file, false);
    }
    else if (event == TAB_BACK)
    {
        // This is now reserved for subfolder navigation (go back)
    }
}

static void tab_refresh(book_t *book)
{
    tab_t *tab = book->tab;
    size_t items_count = 0;

    if (!tab)
        return;

    memset(&tab->status, 0, sizeof(tab->status));

    if (book->count)
    {
        gui_resize_list(tab, book->count);
        for (int i = 0; i < book->capacity; i++)
        {
            retro_file_t *file = &book->items[i];
            if (file->type != RETRO_TYPE_INVALID)
            {
                listbox_item_t *listitem = &tab->listbox.items[items_count++];
                const char *type = file->app ? file->app->short_name : "n/a";
                snprintf(listitem->text, sizeof(listitem->text), "[%-3s] %.40s", type, file->name);
                listitem->arg = file;
                listitem->order = i;
            }
        }
    }

    gui_resize_list(tab, items_count);
    gui_sort_list(tab);

    if (items_count == 0)
    {
        gui_resize_list(tab, 6);
        sprintf(tab->listbox.items[0].text, "Welcome to Retro-Go!");
        sprintf(tab->listbox.items[1].text, " ");
        sprintf(tab->listbox.items[2].text, "You have no %s games", book->name);
        sprintf(tab->listbox.items[3].text, " ");
        sprintf(tab->listbox.items[4].text, "You can hide this tab in the menu");
        tab->listbox.cursor = 3;
    }
}

static void book_repack(book_t *book)
{
    // Repack the array
    book->count = 0;
    for (size_t i = 0; i < book->capacity; i++)
    {
        retro_file_t *item = &book->items[i];
        if (item->type == RETRO_TYPE_INVALID)
            continue;
        if (book->count != i)
            book->items[book->count] = *item;
        book->count++;
    }
}

static void book_append(book_t *book, const retro_file_t *new_item)
{
    // Remove the oldest item if we need the space
    while (book->count >= book->capacity)
    {
        book->items[0].type = RETRO_TYPE_INVALID;
        book_repack(book);
    }
    book->items[book->count] = *new_item;
    book->items[book->count].type = RETRO_TYPE_FILE;
    book->count++;
}

static retro_file_t *book_find(book_t *book, const retro_file_t *file)
{
    for (size_t i = 0; i < book->capacity; i++)
    {
        retro_file_t *entry = &book->items[i];
        if (entry->type == RETRO_TYPE_INVALID)
            continue;
        if (file == NULL) // return first entry
            return entry;
        if (entry->folder == file->folder && !strcmp(entry->name, file->name))
            return entry;
    }

    return NULL;
}

static void book_load(book_t *book)
{
    retro_file_t tmp_file;
    char line_buffer[169] = {0};

    // FIXME: We should leverage cJSON here instead...
    FILE *fp = fopen(book->path, "r");
    if (fp)
    {
        book->count = 0;

        while (fgets(line_buffer, sizeof(line_buffer) - 1, fp))
        {
            size_t len = strlen(line_buffer);
            while (len && (line_buffer[len - 1] == '\n' || line_buffer[len - 1] == '\r'))
            {
                line_buffer[len - 1] = 0;
                len--;
            }

            if (application_path_to_file(line_buffer, &tmp_file))
                book_append(book, &tmp_file);
            else
                RG_LOGW("Unknown path form: '%s'\n", line_buffer);
        }
        fclose(fp);
    }
}

static void book_save(book_t *book)
{
    // FIXME: We should leverage cJSON here instead...
    FILE *fp = fopen(book->path, "w");
    if (!fp && rg_storage_mkdir(rg_dirname(book->path)))
    {
        fp = fopen(book->path, "w");
    }
    if (fp)
    {
        for (size_t i = 0; i < book->capacity; i++)
        {
            retro_file_t *file = &book->items[i];
            if (file->type != RETRO_TYPE_INVALID)
                fprintf(fp, "%s/%s\n", file->folder, file->name);
        }
        fclose(fp);
    }
}

static void book_init(book_type_t book_type, const char *name, const char *desc, size_t capacity)
{
    book_t *book = &books[book_type];
    char path[RG_PATH_MAX + 1];

    sprintf(path, "%s/%s.txt", RG_BASE_PATH_CONFIG, name);

    book->name = strdup(name);
    book->path = strdup(path);
    book->capacity = capacity;
    book->count = 0;
    book->items = calloc(capacity + 1, sizeof(retro_file_t));
    book->tab = gui_add_tab(name, desc, book, event_handler);
    book->initialized = true;

    if (book_type == BOOK_TYPE_RECENT)
    {
        book->tab->listbox.sort_mode = SORT_ID_DESC;
        book->tab->listbox.cursor = 0;
    }

    book_load(book);
    tab_refresh(book);
}

retro_file_t *bookmark_find_by_app(book_type_t book_type, const retro_app_t *app)
{
    RG_ASSERT_ARG(book_type < BOOK_TYPE_COUNT && app != NULL);

    book_t *book = &books[book_type];

    // Find the last entry (most recent)
    for (int i = book->capacity - 1; i >= 0; --i)
    {
        if (book->items[i].type == RETRO_TYPE_INVALID)
            continue;

        if (app && book->items[i].app != app)
            continue;

        return &book->items[i];
    }

    return NULL;
}

bool bookmark_exists(book_type_t book_type, const retro_file_t *file)
{
    RG_ASSERT_ARG(book_type < BOOK_TYPE_COUNT && file != NULL);

    return book_find(&books[book_type], file) != NULL;
}

bool bookmark_add(book_type_t book_type, const retro_file_t *file)
{
    RG_ASSERT_ARG(book_type < BOOK_TYPE_COUNT && file != NULL);

    book_t *book = &books[book_type];

    for (retro_file_t *item; (item = book_find(book, file));)
        item->type = RETRO_TYPE_INVALID;

    book_append(book, file);
    book_save(book);
    tab_refresh(book);

    return true;
}

bool bookmark_remove(book_type_t book_type, const retro_file_t *file)
{
    RG_ASSERT_ARG(book_type < BOOK_TYPE_COUNT && file != NULL);

    book_t *book = &books[book_type];
    size_t found = 0;

    for (retro_file_t *item; (item = book_find(book, file));)
    {
        item->type = RETRO_TYPE_INVALID;
        found++;
    }

    if (found == 0)
        return false;

    book_repack(book);
    book_save(book);
    tab_refresh(book);

    return true;
}

void bookmarks_init(void)
{
    book_init(BOOK_TYPE_FAVORITE, "favorite", "Favorites", 2000);
    book_init(BOOK_TYPE_RECENT, "recent", "Recently played", 100);
}
