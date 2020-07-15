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

typedef struct  {
    uint16_t list_background;
    uint16_t list_foreground;
    uint16_t list_highlight;
} theme_t;

theme_t gui_themes[] = {
    {0, C_GRAY, C_WHITE},
    {0, C_GRAY, C_GREEN},
    {0, C_WHITE, C_GREEN},

    {5, C_GRAY, C_WHITE},
    {5, C_GRAY, C_GREEN},
    {5, C_WHITE, C_GREEN},

    {11, C_GRAY, C_WHITE},
    {11, C_GRAY, C_GREEN},
    {11, C_WHITE, C_GREEN},

    {16, C_GRAY, C_WHITE},
    {16, C_GRAY, C_GREEN},
    {16, C_WHITE, C_GREEN},
};
int gui_themes_count = 12;


static uint16_t *cover_buffer;


retro_emulator_file_t *gui_list_selected_file(retro_emulator_t *emu)
{
    if (emu->roms.count == 0 || emu->roms.selected > emu->roms.count) {
        return NULL;
    }
    return &emu->roms.files[emu->roms.selected];
}

bool gui_list_handle_input(retro_emulator_t *emu, odroid_gamepad_state *joystick, int *last_key)
{
    if (emu->roms.count == 0 || emu->roms.selected > emu->roms.count) {
        return false;
    }

    int selected = emu->roms.selected;
    if (joystick->values[ODROID_INPUT_UP]) {
        *last_key = ODROID_INPUT_UP;
        selected--;
    } else if (joystick->values[ODROID_INPUT_DOWN]) {
        *last_key = ODROID_INPUT_DOWN;
        selected++;
    } else if (joystick->values[ODROID_INPUT_LEFT]) {
        *last_key = ODROID_INPUT_LEFT;
        char st = ((char*)emu->roms.files[selected].name)[0];
        int max = LIST_LINE_COUNT - 2;
        while (--selected > 0 && max-- > 0)
        {
           if (st != ((char*)emu->roms.files[selected].name)[0]) break;
        }
    } else if (joystick->values[ODROID_INPUT_RIGHT]) {
        *last_key = ODROID_INPUT_RIGHT;
        char st = ((char*)emu->roms.files[selected].name)[0];
        int max = LIST_LINE_COUNT - 2;
        while (++selected < emu->roms.count-1 && max-- > 0)
        {
           if (st != ((char*)emu->roms.files[selected].name)[0]) break;
        }
    }

    if (selected < 0) selected = emu->roms.count - 1;
    if (selected >= emu->roms.count) selected = 0;

    bool changed = emu->roms.selected != selected;
    emu->roms.selected = selected;
    return changed;
}

void gui_header_draw(retro_emulator_t *emu)
{
    int x_pos = IMAGE_LOGO_WIDTH + 1;
    int y_pos = IMAGE_BANNER_HEIGHT;
    char buffer[40];

    for (int i = y_pos; i < LIST_Y_OFFSET - 1; i += ODROID_FONT_HEIGHT) {
        odroid_overlay_draw_text(0, i, 320, (char*)" ", C_WHITE, C_BLACK);
    }
    sprintf(buffer, "Games: %d", emu->roms.count);
    odroid_overlay_draw_text(x_pos + 10, y_pos + 3, 0, buffer, C_WHITE, C_BLACK);
    odroid_display_write(0, 0, IMAGE_LOGO_WIDTH, IMAGE_LOGO_HEIGHT, emu->image_logo);
    odroid_display_write(x_pos, 0, IMAGE_BANNER_WIDTH, IMAGE_BANNER_HEIGHT, emu->image_header);
}

void gui_list_draw(retro_emulator_t *emu, int theme_)
{
    int lines = LIST_LINE_COUNT;
    float gradient = 16.f / lines;
    theme_t theme = gui_themes[theme_ % gui_themes_count];

    odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)" ", C_RED, C_BLACK);

    int color, entry, y;
    char text[64];

    for (int i = 0; i < lines; i++) {
        entry = emu->roms.selected + i - (lines / 2);
        color = (entry == emu->roms.selected) ? theme.list_highlight : theme.list_foreground;
        y = LIST_Y_OFFSET + i * LIST_LINE_HEIGHT;

        text[0] = '\0';

        if (entry >= 0 && entry < emu->roms.count) {
            snprintf(text, 64, emu->roms.files[entry].name);
        }
        else if (emu->roms.count == 0) {
            if (i == 1) sprintf(text, "No roms found!");
            else if (i == 4) sprintf(text, "Place roms in folder: /roms/%s", emu->dirname);
            else if (i == 6) sprintf(text, "With file extension: .%s", emu->ext);
        }

        odroid_overlay_draw_text(LIST_X_OFFSET, y, LIST_WIDTH, text, color, (int)(gradient * i) << theme.list_background);
    }
}

// This should be in a FreeRTOS task
// But the problem is that the main task can't draw while this task accesses the SD Card or draws
// So it's kind of pointless. The solution would be preloading I think.
void gui_cover_draw(retro_emulator_t *emu, odroid_gamepad_state *joystick)
{
    if (emu->roms.count == 0 || emu->roms.selected > emu->roms.count) {
        return;
    }

    retro_emulator_file_t *file = gui_list_selected_file(emu);
    char path[128], path2[128], path3[128], buf_crc[10];
    FILE *fp;

    char *cache_path = odroid_system_get_path(ODROID_PATH_CRC_CACHE, file->path);

    if (file->checksum == 0)
    {
        file->missing_cover = 0;

        if ((fp = fopen(cache_path, "rb")) != NULL)
        {
            fread(&file->checksum, 4, 1, fp);
            fclose(fp);
        }
        else if ((fp = fopen(file->path, "rb")) != NULL)
        {
            odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)"        CRC32", C_GREEN, C_BLACK);

            fseek(fp, emu->crc_offset, SEEK_SET);
            int buf_size = 32768;
            uint32_t crc_tmp = 0;
            bool abort = false;

            while (true)
            {
                odroid_input_gamepad_read(joystick);
                abort = joystick->bitmask > 0;
                if (abort) break;
                int count = fread(cover_buffer, 1, buf_size, fp);
                crc_tmp = crc32_le(crc_tmp, (const uint8_t*)cover_buffer, count);
                if (count != buf_size)
                {
                    break;
                }
            }
            fclose(fp);

            if (!abort)
            {
                file->checksum = crc_tmp;
                if ((fp = fopen(cache_path, "wb")) != NULL)
                {
                    fwrite(&file->checksum, 4, 1, fp);
                    fclose(fp);
                }
            }
        }
        else
        {
            file->checksum = 1;
        }
        odroid_overlay_draw_text(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)" ", C_RED, C_BLACK);
    }

    free(cache_path);

    if (file->checksum > 1 && file->missing_cover == 0)
    {
        const int cover_buffer_length = 1024 * 1024 / 2;
        uint16_t cover_width = 0, cover_height = 0;

        if (!cover_buffer) {
            cover_buffer = (uint16_t*)malloc(cover_buffer_length);
        }

        sprintf(buf_crc, "%08X", file->checksum);

        // /sd/romart/gbc/0/08932754.png
        // /sd/romart/gbc/Super Mario.png
        sprintf(path, "%s/%s/%c/%s.png", ODROID_BASE_PATH_ROMART, emu->dirname, buf_crc[0], buf_crc);
        sprintf(path2, "%s/%s/%s.png", ODROID_BASE_PATH_ROMART, emu->dirname, file->name);

        // /sd/romart/gbc/0/08932754.art
        sprintf(path3, "%s/%s/%c/%s.art", ODROID_BASE_PATH_ROMART, emu->dirname, buf_crc[0], buf_crc);

        LuImage *img;
        if ((img = luPngReadFile(path)) || (img = luPngReadFile(path2)))
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
