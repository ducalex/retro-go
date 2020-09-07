#include <odroid_system.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "lupng.h"
#include "gui.h"

#define IMAGE_LOGO_WIDTH    (47)
#define IMAGE_LOGO_HEIGHT   (51)
#define IMAGE_BANNER_WIDTH  (272)
#define IMAGE_BANNER_HEIGHT (32)

#define CRC_WIDTH    (104)
#define CRC_X_OFFSET (ODROID_SCREEN_WIDTH - CRC_WIDTH)
#define CRC_Y_OFFSET (35)

#define LIST_WIDTH       (ODROID_SCREEN_WIDTH)
#define LIST_HEIGHT      (ODROID_SCREEN_HEIGHT - LIST_Y_OFFSET)
#define LIST_LINE_COUNT  ((ODROID_SCREEN_HEIGHT - LIST_Y_OFFSET) / LIST_LINE_HEIGHT)
#define LIST_LINE_HEIGHT (odroid_overlay_get_font_size())
#define LIST_X_OFFSET    (0)
#define LIST_Y_OFFSET    (48 + LIST_LINE_HEIGHT)

#define COVER_MAX_HEIGHT (184)
#define COVER_MAX_WIDTH  (184)

theme_t gui_themes[] = {
    {0, C_GRAY, C_WHITE, C_AQUA},
    {0, C_GRAY, C_GREEN, C_AQUA},
    {0, C_WHITE, C_GREEN, C_AQUA},

    {5, C_GRAY, C_WHITE, C_AQUA},
    {5, C_GRAY, C_GREEN, C_AQUA},
    {5, C_WHITE, C_GREEN, C_AQUA},

    {11, C_GRAY, C_WHITE, C_AQUA},
    {11, C_GRAY, C_GREEN, C_AQUA},
    {11, C_WHITE, C_GREEN, C_AQUA},

    {16, C_GRAY, C_WHITE, C_AQUA},
    {16, C_GRAY, C_GREEN, C_AQUA},
    {16, C_WHITE, C_GREEN, C_AQUA},
};
int gui_themes_count = 12;

static size_t gp_buffer_size = 256 * 1024;
static void  *gp_buffer = NULL;
static char str_buffer[128];

retro_gui_t gui;

void gui_event(gui_event_t event, tab_t *tab)
{
    if (tab->event_handler)
        (*tab->event_handler)(event, tab);
}

tab_t *gui_add_tab(const char *name, const void *logo, const void *header, void *arg, void *event_handler)
{
    tab_t *tab = calloc(1, sizeof(tab_t));

    sprintf(tab->name, "%s", name);
    sprintf(tab->status, "Loading...");

    tab->event_handler = event_handler;
    tab->img_header = header;
    tab->img_logo = logo ?: (void*)tab;
    tab->initialized = false;
    tab->is_empty = false;
    tab->arg = arg;

    gui.tabs[gui.tabcount++] = tab;

    printf("gui_add_tab: Tab '%s' added at index %d\n", tab->name, gui.tabcount - 1);

    return tab;
}

void gui_init_tab(tab_t *tab)
{
    if (tab->initialized)
        return;

    tab->initialized = true;
    // tab->status[0] = 0;

    sprintf(str_buffer, "Sel.%.11s", tab->name);
    tab->listbox.cursor = odroid_settings_int32_get(str_buffer, 0);

    gui_event(TAB_INIT, tab);

    tab->listbox.cursor = MIN(tab->listbox.cursor, tab->listbox.length -1);
    tab->listbox.cursor = MAX(tab->listbox.cursor, 0);
}

tab_t *gui_get_tab(int index)
{
    return (index >= 0 && index < gui.tabcount) ? gui.tabs[index] : NULL;
}

tab_t *gui_get_current_tab()
{
    return gui_get_tab(gui.selected);
}

tab_t *gui_set_current_tab(int index)
{
    index %= gui.tabcount;

    if (index < 0)
        index += gui.tabcount;

    gui.selected = index;

    return gui_get_tab(gui.selected);
}

void gui_save_current_tab()
{
    tab_t *tab = gui_get_current_tab();

    sprintf(str_buffer, "Sel.%.11s", tab->name);
    odroid_settings_int32_set(str_buffer, tab->listbox.cursor);
    odroid_settings_int32_set("SelectedTab", gui.selected);
    odroid_settings_commit();
}

listbox_item_t *gui_get_selected_item(tab_t *tab)
{
    listbox_t *list = &tab->listbox;

    if (list->cursor >= 0 && list->cursor < list->length)
        return &list->items[list->cursor];

    return NULL;
}

static int list_comparator(const void *p, const void *q)
{
    return strcasecmp(((listbox_item_t*)p)->text, ((listbox_item_t*)q)->text);
}

void gui_sort_list(tab_t *tab, int sort_mode)
{
    if (tab->listbox.length == 0)
        return;

    qsort((void*)tab->listbox.items, tab->listbox.length, sizeof(listbox_item_t), list_comparator);
}

void gui_resize_list(tab_t *tab, int new_size)
{
    int cur_size = tab->listbox.length;

    if (new_size == cur_size)
        return;

    if (new_size == 0)
    {
        free(tab->listbox.items);
        tab->listbox.items = NULL;
    }
    else
    {
        tab->listbox.items = realloc(tab->listbox.items, new_size * sizeof(listbox_item_t));
        for (int i = cur_size; i < new_size; i++)
            memset(&tab->listbox.items[i], 0, sizeof(listbox_item_t));
    }

    tab->listbox.length = new_size;
    tab->listbox.cursor = MIN(tab->listbox.cursor, tab->listbox.length -1);
    tab->listbox.cursor = MAX(tab->listbox.cursor, 0);

    printf("gui_resize_list: Resized list '%s' from %d to %d items\n", tab->name, cur_size, new_size);
}

void gui_scroll_list(tab_t *tab, scroll_mode_t mode)
{
    listbox_t *list = &tab->listbox;

    if (list->length == 0 || list->cursor > list->length) {
        return;
    }

    int cur_cursor = list->cursor;
    int old_cursor = list->cursor;

    if (mode == LINE_UP) {
        cur_cursor--;
    }
    else if (mode == LINE_DOWN) {
        cur_cursor++;
    }
    else if (mode == PAGE_UP) {
        char st = ((char*)list->items[cur_cursor].text)[0];
        int max = LIST_LINE_COUNT - 2;
        while (--cur_cursor > 0 && max-- > 0)
        {
           if (st != ((char*)list->items[cur_cursor].text)[0]) break;
        }
    }
    else if (mode == PAGE_DOWN) {
        char st = ((char*)list->items[cur_cursor].text)[0];
        int max = LIST_LINE_COUNT - 2;
        while (++cur_cursor < list->length-1 && max-- > 0)
        {
           if (st != ((char*)list->items[cur_cursor].text)[0]) break;
        }
    }

    if (cur_cursor < 0) cur_cursor = list->length - 1;
    if (cur_cursor >= list->length) cur_cursor = 0;

    list->cursor = cur_cursor;

    if (cur_cursor != old_cursor)
    {
        gui_draw_notice(" ", C_BLACK);
        gui_draw_list(tab);
        gui_event(TAB_SCROLL, tab);
    }
}

void gui_redraw()
{
    tab_t *tab = gui_get_current_tab();
    gui_draw_header(tab);
    gui_draw_status(tab);
    gui_draw_list(tab);
    gui_event(TAB_REDRAW, tab);
}

void gui_draw_navbar()
{
    for (int i = 0; i < gui.tabcount; i++)
    {
        odroid_display_write(i * IMAGE_LOGO_WIDTH, 0, IMAGE_LOGO_WIDTH, IMAGE_LOGO_HEIGHT, gui.tabs[i]->img_logo);
    }
}

void gui_draw_header(tab_t *tab)
{
    int x_pos = IMAGE_LOGO_WIDTH;
    int y_pos = IMAGE_LOGO_HEIGHT;

    odroid_overlay_draw_fill_rect(x_pos, 0, ODROID_SCREEN_WIDTH - x_pos, LIST_Y_OFFSET, C_BLACK);
    odroid_overlay_draw_fill_rect(0, y_pos, ODROID_SCREEN_WIDTH, LIST_Y_OFFSET - y_pos, C_BLACK);

    if (tab->img_logo)
        odroid_display_write(0, 0, IMAGE_LOGO_WIDTH, IMAGE_LOGO_HEIGHT, tab->img_logo);

    if (tab->img_header)
        odroid_display_write(x_pos + 1, 0, IMAGE_BANNER_WIDTH, IMAGE_BANNER_HEIGHT, tab->img_header);
}

// void gui_draw_notice(tab_t *tab)
void gui_draw_notice(const char *text, uint16_t color)
{
    odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, text, color, C_BLACK);
}

void gui_draw_status(tab_t *tab)
{
    odroid_overlay_draw_battery(ODROID_SCREEN_WIDTH - 27, 3);
    odroid_overlay_draw_text(
        IMAGE_LOGO_WIDTH + 11,
        IMAGE_BANNER_HEIGHT + 3,
        128,
        tab->status,
        C_WHITE,
        C_BLACK
    );
}

void gui_draw_list(tab_t *tab)
{
    int columns = LIST_WIDTH / odroid_overlay_get_font_width();
    int lines = LIST_LINE_COUNT;
    theme_t *theme = &gui_themes[gui.theme % gui_themes_count];
    listbox_t *list = &tab->listbox;

    for (int i = 0; i < lines; i++) {
        int entry = list->cursor + i - (lines / 2);

        if (entry >= 0 && entry < list->length) {
            sprintf(str_buffer, "%.*s", columns, list->items[entry].text);
        } else {
            str_buffer[0] = '\0';
        }

        odroid_overlay_draw_text(
            LIST_X_OFFSET,
            LIST_Y_OFFSET + i * LIST_LINE_HEIGHT,
            LIST_WIDTH,
            str_buffer,
            (entry == list->cursor) ? theme->list_selected : theme->list_standard,
            (int)(16.f / lines * i) << theme->list_background
        );
    }
}

void gui_draw_cover(retro_emulator_file_t *file)
{
    retro_emulator_t *emu = (retro_emulator_t *)file->emulator;

    if (gp_buffer == NULL)
        gp_buffer = malloc(gp_buffer_size);

    uint16_t *cover_buffer = (uint16_t*)gp_buffer;
    const int cover_buffer_length = gp_buffer_size / 2;
    uint16_t cover_width = 0, cover_height = 0;

    if (file->checksum > 0 && file->missing_cover == 0)
    {
        char path1[128], path2[128], buf_crc[10];

        sprintf(buf_crc, "%08X", file->checksum);
        sprintf(path1, "%s/%s/%c/%s.png", ODROID_BASE_PATH_ROMART, emu->dirname, buf_crc[0], buf_crc);
        sprintf(path2, "%s/%s/%c/%s.art", ODROID_BASE_PATH_ROMART, emu->dirname, buf_crc[0], buf_crc);

        LuImage *img;
        FILE *fp;

        if ((img = luPngReadFile(path1)))
        {
            for (int p = 0, i = 0; i < img->dataSize && p < cover_buffer_length; i += 3) {
                uint8_t r = img->data[i];
                uint8_t g = img->data[i + 1];
                uint8_t b = img->data[i + 2];
                cover_buffer[p++] = ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
            }
            cover_width = img->width;
            cover_height = img->height;
            luImageRelease(img, NULL);
        }
        else if ((fp = fopen(path2, "rb")))
        {
            fread(&cover_width, 2, 1, fp);
            fread(&cover_height, 2, 1, fp);
            fread(cover_buffer, 2, cover_buffer_length, fp);
            fclose(fp);
        }

        if (cover_width > 0 && cover_height > 0)
        {
            int height = MIN(cover_height, COVER_MAX_HEIGHT);
            int width = MIN(cover_width, COVER_MAX_WIDTH);

            if (cover_height > COVER_MAX_HEIGHT || cover_width > COVER_MAX_WIDTH)
                gui_draw_notice("Art too large", C_ORANGE);

            odroid_display_write_rect(320 - width, 240 - height, width, height, cover_width, cover_buffer);
            return;
        }
    }

    gui_draw_notice(" No art found", C_RED);
    file->missing_cover = 1;
}
