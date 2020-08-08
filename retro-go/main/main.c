#include <odroid_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "emulators.h"
#include "favorites.h"
#include "gui.h"

static bool show_empty = true;
static int  show_cover = 1;

static bool font_size_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int font_size = odroid_settings_FontSize_get();
    if (event == ODROID_DIALOG_PREV) {
        if (--font_size < 1) font_size = 2;
        odroid_overlay_set_font_size(font_size);
        gui_redraw();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++font_size > 2) font_size = 1;
        odroid_overlay_set_font_size(font_size);
        gui_redraw();
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
        if (--gui.theme < 0) gui.theme = max;
        odroid_settings_int32_set("Theme", gui.theme);
        gui_redraw();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++gui.theme > max) gui.theme = 0;
        odroid_settings_int32_set("Theme", gui.theme);
        gui_redraw();
    }
    sprintf(option->value, "%d/%d", gui.theme + 1, max + 1);
    return event == ODROID_DIALOG_ENTER;
}

void retro_loop()
{
    tab_t *tab = gui_get_current_tab();
    int debounce = 0;
    int last_key = -1;
    int selected_tab_last = -1;
    int idle_counter = 0;

    show_empty   = odroid_settings_int32_get("ShowEmpty", 1);
    show_cover   = odroid_settings_int32_get("ShowCover", 1);
    gui.theme    = odroid_settings_int32_get("Theme", 0);
    gui.selected = odroid_settings_int32_get("SelectedTab", 0);

    while (true)
    {
        if (gui.selected != selected_tab_last)
        {
            int dir = gui.selected - selected_tab_last;

            if (gui.selected >= gui.tabcount) gui.selected = 0;
            if (gui.selected < 0) gui.selected = gui.tabcount - 1;

            tab = gui_get_current_tab();

            gui_draw_header(tab);

            if (!tab->initialized)
            {
                gui_init_tab(tab);
            }

            if (!show_empty && tab->listbox.length == 0)
            {
                gui.selected += dir;
                continue;
            }

            gui_draw_subtext(tab);
            gui_draw_list(tab);
            // gui_save_tab(tab);

            selected_tab_last = gui.selected;
        }

        if (idle_counter % 100 == 0)
        {
            odroid_overlay_draw_battery(320 - 26, 3);
        }

        odroid_gamepad_state joystick;
        odroid_input_gamepad_read(&joystick);

        if (show_cover && idle_counter == (show_cover == 1 ? 8 : 1))
        {
            gui_draw_cover(emulator_get_file(tab->arg, tab->listbox.cursor), &joystick);
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

                retro_emulator_file_t *file = emulator_get_file(tab->arg, tab->listbox.cursor);
                if (file)
                {
                    char *file_path = emulator_get_file_path(file);
                    char *save_path = odroid_system_get_path(ODROID_PATH_SAVE_STATE, file_path);
                    bool has_save = access(save_path, F_OK) != -1;

                    odroid_dialog_choice_t choices[] = {
                        {0, "Resume game", "", has_save, NULL},
                        {1, "New game", "", 1, NULL},
                        {2, "Delete save", "", has_save, NULL},
                        ODROID_DIALOG_CHOICE_LAST
                    };
                    int sel = odroid_overlay_dialog(NULL, choices, has_save ? 0 : 1);

                    if (sel == 0 || sel == 1) {
                        gui_save_tab(tab);
                        emulator_start(file, sel == 0);
                    }
                    else if (sel == 2) {
                        if (odroid_overlay_confirm("Delete savestate?", false) == 1) {
                            unlink(save_path);
                        }
                    }
                    free(file_path);
                    free(save_path);

                    gui_redraw();
                }
            }
            else if (joystick.values[ODROID_INPUT_B]) {
                last_key = ODROID_INPUT_B;

                retro_emulator_file_t *file = emulator_get_file(tab->arg, tab->listbox.cursor);
                if (file)
                {
                    char *file_path = emulator_get_file_path(file);

                    odroid_dialog_choice_t choices[] = {
                        {0, "File", "...", 1, NULL},
                        {0, "Type", "N/A", 1, NULL},
                        {0, "Folder", "...", 1, NULL},
                        {0, "Size", "0", 1, NULL},
                        {0, "CRC32", "N/A", 1, NULL},
                        {0, "---", "", -1, NULL},
                        {1, "Close", "", 1, NULL},
                        ODROID_DIALOG_CHOICE_LAST
                    };

                    sprintf(choices[0].value, "%.127s", file->name);
                    sprintf(choices[1].value, "%s", file->ext);
                    sprintf(choices[2].value, "%s", file->folder);
                    sprintf(choices[3].value, "%d KB", odroid_sdcard_get_filesize(file_path) / 1024);
                    if (file->checksum > 1) {
                        sprintf(choices[4].value, "%08X", file->checksum);
                    }

                    odroid_overlay_dialog("Properties", choices, -1);
                    free(file_path);

                    gui_redraw();
                }
            }
            else if (joystick.values[ODROID_INPUT_SELECT]) {
                last_key = ODROID_INPUT_SELECT;
                debounce = -10;
                gui.selected--;
            }
            else if (joystick.values[ODROID_INPUT_START]) {
                last_key = ODROID_INPUT_START;
                debounce = -10;
                gui.selected++;
            }
            else if (joystick.values[ODROID_INPUT_MENU]) {
                last_key = ODROID_INPUT_MENU;
                odroid_dialog_choice_t choices[] = {
                    {0, "Date", COMPILEDATE, 1, NULL},
                    {0, "Commit", GITREV, 1, NULL},
                    {0, "", "", -1, NULL},
                    {1, "Reboot to firmware", "", 1, NULL},
                    {2, "Reset settings", "", 1, NULL},
                    {0, "Close", "", 1, NULL},
                    ODROID_DIALOG_CHOICE_LAST
                };
                int sel = odroid_overlay_dialog("Retro-Go", choices, -1);
                if (sel == 1) {
                    odroid_system_switch_app(-16);
                }
                else if (sel == 2) {
                    if (odroid_overlay_confirm("Reset all settings?", false) == 1) {
                        odroid_settings_reset();
                        esp_restart();
                    }
                }
                gui_redraw();
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
                gui_redraw();
            }
            else if (gui_handle_input(tab, &joystick, &last_key)) {
                gui_draw_list(tab);
            }
        }

        if (joystick.bitmask > 0) {
            idle_counter = 0;
        } else {
            idle_counter++;
        }

        usleep(15 * 1000UL);
    }
}

void app_main(void)
{
    odroid_system_init(0, 32000);
    odroid_display_clear(0);

    emulators_init();
    favorites_init();

    retro_loop();
}
