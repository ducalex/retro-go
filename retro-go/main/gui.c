#include <rom/crc.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "gui.h"
#include "odroid_display.h"
#include "odroid_overlay.h"
#include "odroid_sdcard.h"
#include "odroid_input.h"

#define IMAGE_LOGO_WIDTH    (47)
#define IMAGE_LOGO_HEIGHT   (51)
#define IMAGE_BANNER_WIDTH  (272)
#define IMAGE_BANNER_HEIGHT (32)

#define CRC_WIDTH    (96)
#define CRC_X_OFFSET (320 - CRC_WIDTH)
#define CRC_Y_OFFSET (48)

#define LIST_WIDTH       (320)
#define LIST_LINE_COUNT  ((240 - LIST_Y_OFFSET) / LIST_LINE_HEIGHT)
#define LIST_LINE_HEIGHT (ODROID_FONT_HEIGHT)
#define LIST_X_OFFSET    (0)
#define LIST_Y_OFFSET    (48 + LIST_LINE_HEIGHT)

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


const retro_emulator_file_t *gui_list_selected_file(retro_emulator_t *emu)
{
    if (emu->roms.count == 0 || emu->roms.selected > emu->roms.count) {
        return NULL;
    }
    return &emu->roms.files[emu->roms.selected];
}

bool gui_list_handle_input(retro_emulator_t *emu, odroid_gamepad_state *joystick, int *last_key)
{
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

    odroid_overlay_draw_chars(0, CRC_Y_OFFSET, 320, (char*)" ", C_WHITE, C_BLACK);
    sprintf(buffer, "Games: %d", emu->roms.count);
    odroid_overlay_draw_chars(x_pos, y_pos + ODROID_FONT_HEIGHT, 320 - x_pos, buffer, C_WHITE, C_BLACK);
    odroid_overlay_draw_chars(x_pos, y_pos, 320 - x_pos, emu->system_name, C_WHITE, C_BLACK);
    ili9341_write_frame_rectangleLE(0, 0, IMAGE_LOGO_WIDTH, IMAGE_LOGO_HEIGHT, emu->image_logo);
    ili9341_write_frame_rectangleLE(x_pos, 0, IMAGE_BANNER_WIDTH, IMAGE_BANNER_HEIGHT, emu->image_header);
}

void gui_list_draw(retro_emulator_t *emu, int theme_)
{
    int lines = LIST_LINE_COUNT;
    float gradient = 16.f / lines;
    theme_t theme = gui_themes[theme_ % gui_themes_count];

    for (int i = 0; i < lines; i++) {
        int entry = emu->roms.selected + i - (lines / 2);
        int y = LIST_Y_OFFSET + i * LIST_LINE_HEIGHT;
        char *text = (entry >= 0 && entry < emu->roms.count) ? emu->roms.files[entry].name : (char *)" ";
        uint16_t fg_color = (entry == emu->roms.selected) ? theme.list_highlight : theme.list_foreground;
        odroid_overlay_draw_chars(LIST_X_OFFSET, y, LIST_WIDTH, text, fg_color, (int)(gradient * i) << theme.list_background);
    }
}

void gui_cover_draw(retro_emulator_t *emu, odroid_gamepad_state *joystick)
{
    if (emu->roms.count == 0 || emu->roms.selected > emu->roms.count) {
        odroid_overlay_draw_chars(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)" ", C_RED, C_BLACK);
        return;
    }

    uint32_t *crc = &emu->roms.files[emu->roms.selected].checksum;
    char path[128], buf_crc[10];
    FILE *fp;

    if (*crc == 0)
    {
        const retro_emulator_file_t *file = gui_list_selected_file(emu);

        sprintf(path, CACHE_PATH "/%s/romart/%c/%s", emu->dirname, file->name[0], file->name);
        if (odroid_sdcard_copy_file_to_memory(path, crc)) {
            printf("Cache found: %s\n", path);
        }
        else if ((fp = fopen(file->path, "rb")) != NULL)
        {
            odroid_overlay_draw_chars(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)"       CRC32", C_GREEN, C_BLACK);
            fseek(fp, emu->crc_offset, SEEK_SET);
            int buf_size = 32768;
            unsigned char *buffer = (unsigned char*)malloc(buf_size);
            uint32_t crc_tmp = 0;
            bool abort = false;
            while (true)
            {
                odroid_input_gamepad_read(joystick);
                for (int i = 0; i < ODROID_INPUT_MAX; i++) {
                    abort = abort || joystick->values[i];
                }
                if (abort) break;
                int count = fread(buffer, 1, buf_size, fp);
                crc_tmp = crc32_le(crc_tmp, buffer, count);
                if (count != buf_size)
                {
                    break;
                }
            }
            free(buffer);
            fclose(fp);

            if (!abort)
            {
                *crc = crc_tmp;
                sprintf(path, CACHE_PATH "/%s/romart/%c/", emu->dirname, file->name[0]);
                odroid_sdcard_mkdir(path);
                strcat(path, file->name);
                if ((fp = fopen(path, "wb")) != NULL)
                {
                    fwrite(crc, 4, 1, fp);
                    fclose(fp);
                }
            }
        }
        else
        {
            *crc = 1;
        }
    }
    if (*crc > 1)
    {
        sprintf(buf_crc, "%X", *crc);
        sprintf(path, ROMART_PATH "/%s/%c/%s.art", emu->dirname, buf_crc[0], buf_crc);
        if ((fp = fopen(path, "rb")) != NULL)
        {
            uint16_t width, height;
            fread(&width, 2, 1, fp);
            fread(&height, 2, 1, fp);
            if (width <= 320 && height <= 176)
            {
                uint16_t *img = (uint16_t*)malloc(width * height * 2);
                fread(img, 2, width * height, fp);
                ili9341_write_frame_rectangleLE(320 - width, 240 - height, width, height, img);
                free(img);
            }
            else
            {
                *crc = 1;
            }
            fclose(fp);
        }
        else
        {
            *crc = 1;
        }
    }
    if (*crc == 1)
    {
        odroid_overlay_draw_chars(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)"No art found", C_RED, C_BLACK);
    }
    else
    {
        odroid_overlay_draw_chars(CRC_X_OFFSET, CRC_Y_OFFSET, CRC_WIDTH, (char*)" ", C_RED, C_BLACK);
    }
}
