#pragma once

#include <rg_input.h>
#include <stdbool.h>
#include "emulators.h"

typedef enum {
    KEY_PRESS_A,
    KEY_PRESS_B,
    TAB_SCROLL,
    TAB_INIT,
    TAB_ENTER,
    TAB_LEAVE,
    TAB_SAVE,
    TAB_IDLE,
    TAB_REDRAW,
} gui_event_t;

typedef enum {
    SCROLL_UPDATE,
    SCROLL_ABSOLUTE,
    SCROLL_LINE_UP,
    SCROLL_LINE_DOWN,
    SCROLL_PAGE_UP,
    SCROLL_PAGE_DOWN,
    SCROLL_FIRST_ROW,
    SCROLL_LAST_ROW,
} scroll_mode_t;

typedef enum {
    SORT_NONE,
    SORT_ID_ASC,
    SORT_ID_DESC,
    SORT_TEXT_ASC,
    SORT_TEXT_DESC,
} sort_mode_t;

typedef enum {
    PREVIEW_MODE_NONE = 0,
    PREVIEW_MODE_COVER_SAVE,
    PREVIEW_MODE_SAVE_COVER,
    PREVIEW_MODE_COVER_ONLY,
    PREVIEW_MODE_SAVE_ONLY,
    PREVIEW_MODE_COUNT
} preview_modes_t;

typedef struct {
    const char *name;
    uint16_t order;
} preview_mode_t;

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
    int sort_mode;
} listbox_t;

typedef void (*gui_event_handler_t)(gui_event_t event, void *arg);

typedef struct tab_s {
    char name[64];
    struct {
        char left[24];
        char right[24];
    } status[2];
    const rg_image_t *img_logo;
    const rg_image_t *img_header;
    bool initialized;
    bool is_empty;
    bool enabled;
    void *arg;
    listbox_t listbox;
    gui_event_handler_t event_handler;
} tab_t;

typedef struct {
    tab_t *tabs[32];
    int tabcount;
    int selected;
    int theme;
    int show_preview;
    int show_preview_fast;
    int idle_counter;
    int last_key;
    int width;
    int height;
    gamepad_state_t joystick;
} retro_gui_t;

extern retro_gui_t gui;
extern const int gui_themes_count;

tab_t *gui_add_tab(const char *name, const rg_image_t *logo, const rg_image_t *header, void *arg, void *event_handler);
tab_t *gui_get_tab(int index);
tab_t *gui_get_current_tab();
tab_t *gui_set_current_tab(int index);
void gui_set_status(tab_t *tab, const char *left, const char *right);
void gui_init_tab(tab_t *tab);

void gui_sort_list(tab_t *tab);
void gui_scroll_list(tab_t *tab, scroll_mode_t mode, int arg);
void gui_resize_list(tab_t *tab, int new_size);
listbox_item_t *gui_get_selected_item(tab_t *tab);

void gui_init(void);
void gui_save_position(bool commit);
void gui_save_config(bool commit);
void gui_event(gui_event_t event, tab_t *tab);
void gui_redraw(void);
void gui_draw_navbar(void);
void gui_draw_header(tab_t *tab);
void gui_draw_status(tab_t *tab);
void gui_draw_list(tab_t *tab);
void gui_draw_preview(tab_t *tab, retro_emulator_file_t *file);
