#pragma once

#include <odroid_input.h>
#include "emulators.h"

typedef struct {
    uint16_t list_background;
    uint16_t list_standard;
    uint16_t list_selected;
    uint16_t list_disabled;
} theme_t;

typedef struct {
    char text[128];
    int enabled;
    int id;
    void *arg;
} listbox_item_t;

typedef struct {
    listbox_item_t **items;
    int length;
    int cursor;
} listbox_t;

typedef enum {
    KEY_PRESS_A,
    KEY_PRESS_B,
    SEL_CHANGED,
    TAB_INIT,
    TAB_SHOW,
    TAB_HIDE,
    TAB_SAVE,
} tab_event_t;

typedef void (*tab_event_handler_t)(tab_event_t event, void *tab);

typedef struct {
    char name[64];
    char subtext[64];
    const void *img_logo;
    const void *img_header;
    int position;
    int initialized;
    int enabled;
    void *arg;
    listbox_t listbox;
    tab_event_handler_t event_handler;
} tab_t;

typedef struct {
    tab_t *tabs[32];
    int tabcount;
    int selected;
    int theme;
} retro_gui_t;

extern retro_gui_t gui;
extern int gui_themes_count;

tab_t *gui_add_tab(const char *name, const void *logo, const void *header, void *arg, void *event_handler);
void   gui_init_tab(tab_t *tab);
void   gui_save_tab(tab_t *tab);
tab_t *gui_get_current_tab();
tab_t *gui_get_tab(int index);
listbox_item_t *gui_get_selected_item(tab_t *tab);

void gui_redraw(void);
void gui_draw_navbar(void);
void gui_draw_header(tab_t *tab);
void gui_draw_subtext(tab_t *tab);
void gui_draw_list(tab_t *tab);
bool gui_handle_input(tab_t *tab, odroid_gamepad_state *joystick, int *last_key);
void gui_draw_cover(retro_emulator_file_t *file, odroid_gamepad_state *joystick);
