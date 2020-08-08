#include <odroid_system.h>
#include <rom/crc.h>
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
#define LIST_LINE_COUNT  ((ODROID_SCREEN_HEIGHT - LIST_Y_OFFSET) / LIST_LINE_HEIGHT)
#define LIST_LINE_HEIGHT (ODROID_FONT_HEIGHT)
#define LIST_X_OFFSET    (0)
#define LIST_Y_OFFSET    (48 + LIST_LINE_HEIGHT)

#define COVER_MAX_HEIGHT (200)
#define COVER_MAX_WIDTH  (200)

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


tab_t *gui_add_tab(const char *name, const void *logo, const void *header, void *arg, void *event_handler)
{
    tab_t *tab = calloc(1, sizeof(tab_t));

    sprintf(tab->name, "%s", name);
    sprintf(tab->subtext, "Loading...");

    tab->event_handler = event_handler;
    tab->img_header = header;
    tab->img_logo = logo ?: (void*)tab;
    tab->arg = arg;

    gui.tabs[gui.tabcount++] = tab;

    return tab;
}

void gui_init_tab(tab_t *tab)
{
    if (tab->initialized)
        return;

    tab->initialized = true;
    tab->subtext[0] = 0;

    sprintf(str_buffer, "Sel.%.11s", tab->name);
    tab->listbox.cursor = odroid_settings_int32_get(str_buffer, 0);

    if (tab->event_handler)
        (*tab->event_handler)(TAB_INIT, tab);

    if (tab->arg) {
        retro_emulator_t *emu = (retro_emulator_t *)tab->arg;
        emulator_init(emu);
        sprintf(tab->subtext, "Games: %d", emu->roms.count);
        tab->listbox.length = emu->roms.count;

        // if (emu->roms.count == 0) {
        //     if (i == 1) sprintf(text, "No roms found!");
        //     else if (i == 4) sprintf(text, "Place roms in folder: /roms/%s", emu->dirname);
        //     else if (i == 6) sprintf(text, "With file extension: .%s", emu->ext);
        // }
    }

    tab->listbox.cursor = MIN(tab->listbox.cursor, tab->listbox.length -1);
    tab->listbox.cursor = MAX(tab->listbox.cursor, 0);
}

void gui_save_tab(tab_t *tab)
{
    sprintf(str_buffer, "Sel.%.11s", tab->name);
    odroid_settings_int32_set(str_buffer, tab->listbox.cursor);
    odroid_settings_int32_set("SelectedTab", gui.selected);
}

tab_t *gui_get_current_tab()
{
    return gui_get_tab(gui.selected);
}

tab_t *gui_get_tab(int index)
{
    if (index < 0)
        index = gui.tabcount + index;

    if (index >= 0 && index < gui.tabcount)
        return gui.tabs[index];

    return NULL;
}

listbox_item_t *gui_get_selected_item(tab_t *tab)
{
    return NULL;
}

bool gui_handle_input(tab_t *tab, odroid_gamepad_state *joystick, int *last_key)
{
    retro_emulator_t *emu = (retro_emulator_t *)tab->arg;
    listbox_t *list = &tab->listbox;

    if (emu == NULL || list->length == 0 || list->cursor > list->length) {
        return false;
    }

    int selected = list->cursor;
    if (joystick->values[ODROID_INPUT_UP]) {
        *last_key = ODROID_INPUT_UP;
        selected--;
    }
    else if (joystick->values[ODROID_INPUT_DOWN]) {
        *last_key = ODROID_INPUT_DOWN;
        selected++;
    }
    else if (joystick->values[ODROID_INPUT_LEFT]) {
        *last_key = ODROID_INPUT_LEFT;
        char st = ((char*)emu->roms.files[selected].name)[0];
        int max = LIST_LINE_COUNT - 2;
        while (--selected > 0 && max-- > 0)
        {
           if (st != ((char*)emu->roms.files[selected].name)[0]) break;
        }
    }
    else if (joystick->values[ODROID_INPUT_RIGHT]) {
        *last_key = ODROID_INPUT_RIGHT;
        char st = ((char*)emu->roms.files[selected].name)[0];
        int max = LIST_LINE_COUNT - 2;
        while (++selected < list->length-1 && max-- > 0)
        {
           if (st != ((char*)emu->roms.files[selected].name)[0]) break;
        }
    }

    if (selected < 0) selected = list->length - 1;
    if (selected >= list->length) selected = 0;

    bool changed = list->cursor != selected;
    list->cursor = selected;
    return changed;
}

void gui_redraw()
{
    tab_t *tab = gui_get_current_tab();
    gui_draw_header(tab);
    gui_draw_list(tab);
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
    int x_pos = IMAGE_LOGO_WIDTH + 1;
    int y_pos = IMAGE_BANNER_HEIGHT;

    odroid_overlay_draw_fill_rect(x_pos, 0, ODROID_SCREEN_WIDTH - y_pos, LIST_Y_OFFSET, C_BLACK);

    if (tab->img_logo)
        odroid_display_write(0, 0, IMAGE_LOGO_WIDTH, IMAGE_LOGO_HEIGHT, tab->img_logo);

    if (tab->img_header)
        odroid_display_write(x_pos, 0, IMAGE_BANNER_WIDTH, IMAGE_BANNER_HEIGHT, tab->img_header);

    odroid_overlay_draw_battery(ODROID_SCREEN_WIDTH - 26, 3);
    gui_draw_subtext(tab);
}

void gui_draw_subtext(tab_t *tab)
{
    odroid_overlay_draw_text(
        IMAGE_LOGO_WIDTH + 11,
        IMAGE_BANNER_HEIGHT + 3,
        ODROID_SCREEN_WIDTH,
        tab->subtext,
        C_WHITE,
        C_BLACK
    );
}

void gui_draw_list(tab_t *tab)
{
    int lines = LIST_LINE_COUNT;
    int columns = LIST_WIDTH / ODROID_FONT_WIDTH;
    theme_t *theme = &gui_themes[gui.theme % gui_themes_count];

    listbox_t *list = &tab->listbox;
    retro_emulator_t *emu = (retro_emulator_t *)tab->arg;

    odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)" ", C_RED, C_BLACK);

    for (int i = 0; i < lines; i++) {
        int entry = list->cursor + i - (lines / 2);

        if (entry >= 0 && entry < list->length) {
            sprintf(str_buffer, "%.*s", columns, emu->roms.files[entry].name);
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

void gui_draw_cover(retro_emulator_file_t *file, odroid_gamepad_state *joystick)
{
    retro_emulator_t *emu = (retro_emulator_t *)file->emulator;

    char path1[128], path2[128], path3[128], buf_crc[10];
    FILE *fp, *fp2;

    if (!gp_buffer) {
        gp_buffer = malloc(gp_buffer_size);
    }

    if (file->checksum == 0)
    {
        char *file_path = emulator_get_file_path(file);
        char *cache_path = odroid_system_get_path(ODROID_PATH_CRC_CACHE, file_path);

        file->missing_cover = 0;

        if ((fp = fopen(cache_path, "rb")) != NULL)
        {
            fread(&file->checksum, 4, 1, fp);
            fclose(fp);
        }
        else if ((fp = fopen(file_path, "rb")) != NULL)
        {
            odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)"        CRC32", C_GREEN, C_BLACK);

            uint32_t chunk_size = 32768;
            uint32_t crc_tmp = 0;
            uint32_t count = 0;

            fseek(fp, emu->crc_offset, SEEK_SET);
            while (true)
            {
                odroid_input_gamepad_read(joystick);
                if (joystick->bitmask > 0) break;

                count = fread(gp_buffer, 1, chunk_size, fp);
                if (count == 0) break;

                crc_tmp = crc32_le(crc_tmp, gp_buffer, count);
                if (count < chunk_size) break;
            }

            if (feof(fp))
            {
                file->checksum = crc_tmp;
                if ((fp2 = fopen(cache_path, "wb")) != NULL)
                {
                    fwrite(&file->checksum, 4, 1, fp2);
                    fclose(fp2);
                }
            }
            fclose(fp);
        }
        else
        {
            file->checksum = 1;
        }
        free(cache_path);
        free(file_path);
    }

    odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)" ", C_RED, C_BLACK);

    if (file->checksum > 1 && file->missing_cover == 0)
    {
        uint16_t *cover_buffer = (uint16_t*)gp_buffer;
        const int cover_buffer_length = gp_buffer_size / 2;
        uint16_t cover_width = 0, cover_height = 0;

        sprintf(buf_crc, "%08X", file->checksum);
        sprintf(path1, "%s/%s/%c/%s.png", ODROID_BASE_PATH_ROMART, emu->dirname, buf_crc[0], buf_crc);
        sprintf(path2, "%s/%s/%s.png", ODROID_BASE_PATH_ROMART, emu->dirname, file->name);
        sprintf(path3, "%s/%s/%c/%s.art", ODROID_BASE_PATH_ROMART, emu->dirname, buf_crc[0], buf_crc);

        LuImage *img;
        if ((img = luPngReadFile(path1)) || (img = luPngReadFile(path2)))
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
        else if ((fp = fopen(path3, "rb")) != NULL)
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
            {
                odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)"Art too large", C_ORANGE, C_BLACK);
            }

            odroid_display_write_rect(320 - width, 240 - height, width, height, cover_width, cover_buffer);
            file->missing_cover = false;
            return;
        }
    }

    odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)" No art found", C_RED, C_BLACK);
    file->missing_cover = true;
}
