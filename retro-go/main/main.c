#include <odroid_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "emulators.h"
#include "gui.h"

extern int gui_themes_count;

static retro_emulator_t *emu = NULL;

static bool show_empty = true;
static int  show_cover = 1;
 int  scroll_mode = 0;
static int  selected_emu = 0;
static int  theme = 0;

static void redraw_screen()
{
    gui_header_draw(emu);
    gui_list_draw(emu, theme);
}

static bool font_size_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int font_size = odroid_settings_FontSize_get();
    if (event == ODROID_DIALOG_PREV) {
        if (--font_size < 1) font_size = 2;
        odroid_overlay_set_font_size(font_size);
        redraw_screen();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++font_size > 2) font_size = 1;
        odroid_overlay_set_font_size(font_size);
        redraw_screen();
    }
    strcpy(option->value, font_size > 1 ? "Large" : "Small");
    return event == ODROID_DIALOG_ENTER;
}

static bool hide_empty_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        show_empty = !show_empty;
        odroid_settings_int32_set("ShowEmpty", show_empty);
    }
    strcpy(option->value, show_empty ? "Yes" : "No");
    return event == ODROID_DIALOG_ENTER;
}

static bool startup_app_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int startup_app = odroid_settings_StartupApp_get();
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        startup_app = startup_app ? 0 : 1;
        odroid_settings_StartupApp_set(startup_app);
    }
    strcpy(option->value, startup_app == 0 ? "Launcher" : "Previous");
    return event == ODROID_DIALOG_ENTER;
}

static bool show_cover_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV) {
        if (--show_cover < 0) show_cover = 2;
        odroid_settings_int32_set("ShowCover", show_cover);
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++show_cover > 2) show_cover = 0;
        odroid_settings_int32_set("ShowCover", show_cover);
    }
    if (show_cover == 0) strcpy(option->value, "No");
    if (show_cover == 1) strcpy(option->value, "Slow");
    if (show_cover == 2) strcpy(option->value, "Fast");
    return event == ODROID_DIALOG_ENTER;
}

static bool color_shift_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int max = gui_themes_count - 1;
    if (event == ODROID_DIALOG_PREV) {
        if (--theme < 0) theme = max;
        odroid_settings_int32_set("Theme", theme);
        redraw_screen();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++theme > max) theme = 0;
        odroid_settings_int32_set("Theme", theme);
        redraw_screen();
    }
    sprintf(option->value, "%d/%d", theme + 1, max + 1);
    return event == ODROID_DIALOG_ENTER;
}

static bool scroll_mode_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV && --scroll_mode < 0) scroll_mode = 2;
    if (event == ODROID_DIALOG_NEXT && ++scroll_mode > 2) scroll_mode = 0;

    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        odroid_settings_int32_set("ScrollMode", scroll_mode);
    }

    if (scroll_mode == 0) sprintf(option->value, "Letter   ");
    if (scroll_mode == 1) sprintf(option->value, "Half Page");
    if (scroll_mode == 2) sprintf(option->value, "Full Page");

    return event == ODROID_DIALOG_ENTER;
}

void retro_loop()
{
    int debounce = 0;
    int last_key = -1;
    int selected_emu_last = -1;
    int idle_counter = 0;
    bool redraw = true;

    show_empty   = odroid_settings_int32_get("ShowEmpty", 1);
    show_cover   = odroid_settings_int32_get("ShowCover", 1);
    scroll_mode  = odroid_settings_int32_get("ScrollMode", 0);
    theme        = odroid_settings_int32_get("Theme", 0);

    emulators_init();

    while (true)
    {
        if (emulators->selected != selected_emu_last)
        {
            int dir = emulators->selected - selected_emu_last;

            if (emulators->selected >= emulators->count) emulators->selected = 0;
            if (emulators->selected < 0) emulators->selected = emulators->count - 1;

            emu = &emulators->entries[emulators->selected];

            if (!emu->initialized)
            {
                odroid_overlay_draw_text(58, 35, 320 - 58, (char*)"Loading directory...", C_WHITE, C_BLACK);
                emulators_init_emu(emu);
            }

            if (!show_empty && emu->roms.count == 0)
            {
                emulators->selected += dir;
                continue;
            }

            gui_header_draw(emu);

            selected_emu_last = emulators->selected;
            redraw = true;
        }

        if (redraw || idle_counter % 100 == 0)
        {
            odroid_overlay_draw_battery(320 - 26, 3);
        }

        if (redraw)
        {
            gui_list_draw(emu, theme);
            redraw = false;
        }

        odroid_gamepad_state joystick;
        odroid_input_gamepad_read(&joystick);

        if (show_cover && idle_counter == (show_cover == 1 ? 8 : 1))
        {
            gui_cover_draw(emu, &joystick);
        }

        if (last_key >= 0) {
            if (!joystick.values[last_key]) {
                last_key = -1;
                debounce = 0;
            } else if (debounce++ > 12) {
                debounce = 12;
                last_key = -1;
            }
        } else {
            if (joystick.values[ODROID_INPUT_A]) {
                last_key = ODROID_INPUT_A;
                redraw = true;

                retro_emulator_file_t *file = emu_get_selected_file(emu);
                if (file)
                {
                    char *save_path = odroid_system_get_path(ODROID_PATH_SAVE_STATE, file->path);
                    bool has_save = access(save_path, F_OK) != -1;

                    odroid_dialog_choice_t choices[] = {
                        {0, "Resume game", "", has_save, NULL},
                        {1, "New game", "", 1, NULL},
                        {2, "Delete save", "", has_save, NULL},
                        ODROID_DIALOG_CHOICE_LAST
                    };
                    int sel = odroid_overlay_dialog(NULL, choices, has_save ? 0 : 1);

                    if (sel == 0) {
                        odroid_settings_StartAction_set(ODROID_START_ACTION_RESUME);
                        break;
                    }
                    else if (sel == 1) {
                        odroid_settings_StartAction_set(ODROID_START_ACTION_NEWGAME);
                        break;
                    }
                    else if (sel == 2) {
                        if (odroid_overlay_confirm("Delete savestate?", false) == 1) {
                            unlink(save_path);
                        }
                    }
                    else if (sel == 3) {
                        odroid_settings_StartAction_set(ODROID_START_ACTION_NETPLAY);
                        break;
                    }
                    free(save_path);
                    // continue;
                }
            }
            else if (joystick.values[ODROID_INPUT_B]) {
                last_key = ODROID_INPUT_B;

                retro_emulator_file_t *file = emu_get_selected_file(emu);
                if (file && file->checksum != 0)
                {
                    odroid_dialog_choice_t choices[] = {
                        {0, "File", "...", 1, NULL},
                        {0, "Type", "N/A", 1, NULL},
                        {0, "Folder", "...", 1, NULL},
                        {0, "Size", "0", 1, NULL},
                        {0, "CRC32", "00000000", 1, NULL},
                        {0, "---", "", -1, NULL},
                        {1, "Close", "", 1, NULL},
                        ODROID_DIALOG_CHOICE_LAST
                    };

                    sprintf(choices[0].value, "%.127s", file->name);
                    sprintf(choices[1].value, "%s", file->ext);
                    sprintf(choices[2].value, "%.*s", strlen(file->path)-strlen(file->name)-strlen(file->ext)-2, file->path);
                    sprintf(choices[3].value, "%d KB", odroid_sdcard_get_filesize(file->path) / 1024);
                    sprintf(choices[4].value, "%08X", file->checksum);

                    odroid_overlay_dialog("Properties", choices, -1);
                }
                selected_emu_last = -1;
                redraw = true;
            }
            else if (joystick.values[ODROID_INPUT_SELECT]) {
                last_key = ODROID_INPUT_SELECT;
                debounce = -10;
                emulators->selected--;
            }
            else if (joystick.values[ODROID_INPUT_START]) {
                last_key = ODROID_INPUT_START;
                debounce = -10;
                emulators->selected++;
            }
            else if (joystick.values[ODROID_INPUT_MENU]) {
                last_key = ODROID_INPUT_MENU;
                odroid_dialog_choice_t choices[] = {
                    {0, "Date", COMPILEDATE, 1, NULL},
                    {0, "Commit", GITREV, 1, NULL},
                    {0, "", "", -1, NULL},
                    {1, "Reboot to firmware", "", 1, NULL},
                    {0, "Close", "", 1, NULL},
                    ODROID_DIALOG_CHOICE_LAST
                };
                int sel = odroid_overlay_dialog("Retro-Go", choices, 4);
                if (sel == 1) {
                    odroid_system_switch_app(-16);
                }
                selected_emu_last = -1;
                redraw = true;
            }
            else if (joystick.values[ODROID_INPUT_VOLUME]) {
                last_key = ODROID_INPUT_VOLUME;
                odroid_dialog_choice_t choices[] = {
                    {0, "---", "", -1, NULL},
                    {0, "Color theme", "1/10", 1, &color_shift_cb},
                    {0, "Font size", "Small", 1, &font_size_cb},
                    {0, "Show cover", "Yes", 1, &show_cover_cb},
                    {0, "Show empty", "Yes", 1, &hide_empty_cb},
                    {0, "---", "", -1, NULL},
                    {0, "Startup app", "Last", 1, &startup_app_cb},
                    ODROID_DIALOG_CHOICE_LAST
                };
                odroid_overlay_settings_menu(choices);
                selected_emu_last = -1;
                redraw = true;
            }
            else if (gui_list_handle_input(emu, &joystick, &last_key)) {
                redraw = true;
            }
        }

        if (joystick.bitmask > 0) {
            idle_counter = 0;
        } else {
            idle_counter++;
        }

        usleep(15 * 1000UL);
    }

    emulators_start_emu(emu);
}


void app_main(void)
{
    odroid_system_init(0, 32000);
    odroid_display_clear(0);
    retro_loop();
}
