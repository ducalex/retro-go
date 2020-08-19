#pragma once

#include <odroid_input.h>
#include "emulators.h"
#include "stdbool.h"

typedef enum {
    KEY_PRESS_A,
    KEY_PRESS_B,
    TAB_SCROLL,
    TAB_INIT,
    TAB_SAVE,
    TAB_IDLE,
    TAB_REDRAW,
} gui_event_t;

typedef enum {
    LINE_UP,
    LINE_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    FIRST_ROW,
    LAST_ROW,
} scroll_mode_t;

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
    int arg_type;
    void *arg;
} listbox_item_t;

typedef struct {
    // listbox_item_t **items;
    listbox_item_t *items;
    int length;
    int cursor;
} listbox_t;

typedef void (*gui_event_handler_t)(gui_event_t event, void *arg);

typedef struct {
    char name[64];
    char status[64];
    const void *img_logo;
    const void *img_header;
    bool initialized;
    bool is_empty;
    void *arg;
    listbox_t listbox;
    gui_event_handler_t event_handler;
} tab_t;

typedef struct {
    tab_t *tabs[32];
    int tabcount;
    int selected;
    int theme;
    int show_empty;
    int show_cover;
    int idle_counter;
    int last_key;
    odroid_gamepad_state_t joystick;
} retro_gui_t;

extern retro_gui_t gui;
extern int gui_themes_count;

tab_t *gui_add_tab(const char *name, const void *logo, const void *header, void *arg, void *event_handler);
tab_t *gui_get_tab(int index);
tab_t *gui_get_current_tab();
tab_t *gui_set_current_tab(int index);
void gui_init_tab(tab_t *tab);
void gui_save_current_tab(void);

void gui_sort_list(tab_t *tab, int sort_mode);
void gui_scroll_list(tab_t *tab, scroll_mode_t mode);
void gui_resize_list(tab_t *tab, int new_size);
listbox_item_t *gui_get_selected_item(tab_t *tab);

void gui_event(gui_event_t event, tab_t *tab);
void gui_redraw(void);
void gui_draw_navbar(void);
void gui_draw_header(tab_t *tab);
void gui_draw_status(tab_t *tab);
void gui_draw_list(tab_t *tab);
void gui_draw_notice(const char *text, uint16_t color);
void gui_draw_cover(retro_emulator_file_t *file);
