#pragma once

#include <rg_gui.h>
#include <stdbool.h>

typedef enum {
    START_SCREEN_AUTO,
    START_SCREEN_CAROUSEL,
    START_SCREEN_BROWSER,
    START_SCREEN_COUNT,
} start_screen_t;

typedef enum {
    SCROLL_MODE_CENTER,
    SCROLL_MODE_PAGING,
    // SCROLL_MODE_EDGE,
    SCROLL_MODE_COUNT,
} scroll_mode_t;

typedef enum {
    TAB_ACTION,
    TAB_BACK,
    TAB_SCROLL,
    TAB_INIT,
    TAB_ENTER,
    TAB_LEAVE,
    TAB_IDLE,
    TAB_REFRESH,
    TAB_RESCAN,
} gui_event_t;

typedef enum {
    SCROLL_RESET,
    SCROLL_SET,
    SCROLL_END,
    SCROLL_LINE,
    SCROLL_PAGE,
} scroll_whence_t;

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
} preview_mode_t;

typedef struct {
    struct {
        uint16_t standard_bg;
        uint16_t standard_fg;
        uint16_t selected_bg;
        uint16_t selected_fg;
    } list;
} theme_t;

typedef struct {
    char text[92];
    int16_t order;
    uint8_t group;
    uint8_t unused; // icon, enabled
    void *arg;
} listbox_item_t;

typedef struct {
    // listbox_item_t **items;
    listbox_item_t *items;
    int capacity;
    int length;
    int cursor;
    int sort_mode;
} listbox_t;

typedef struct {
    const char *name;
    size_t size;
    uint8_t data[];
} binfile_t;

typedef void (*gui_event_handler_t)(gui_event_t event, void *arg);

// typedef struct {
//     navpath_t *parent;
//     char label[32];
//     void *arg;
// } navpath_t;

typedef struct tab_s {
    char name[16];
    char desc[64];
    struct {
        char left[24];
        char right[24];
    } status[2];
    bool initialized;
    bool enabled;
    void *arg;
    const char *navpath;
    listbox_t listbox;
    rg_image_t *background;
    rg_image_t *banner;
    rg_image_t *logo;
    rg_image_t *preview;
    int background_shade;
    gui_event_handler_t event_handler;
} tab_t;

typedef struct {
    tab_t *tabs[32];
    size_t tabs_count;
    int selected_tab;
    int startup_mode;
    int browse;
    char *hidden_tabs;
    const char *theme;
    int color_theme;
    int start_screen;
    int show_preview;
    int scroll_mode;
    int width;
    int height;
    theme_t themes[4];
    uint32_t idle_counter;
    uint32_t joystick;
    bool http_lock; // FIXME: should be a mutex...
    rg_surface_t *surface;
} retro_gui_t;

extern retro_gui_t gui;
extern const binfile_t *builtin_images[];

tab_t *gui_add_tab(const char *name, const char *desc, void *arg, void *event_handler);
tab_t *gui_get_tab(int index);
tab_t *gui_get_current_tab(void);
tab_t *gui_set_current_tab(int index);
void gui_set_status(tab_t *tab, const char *left, const char *right);
void gui_init_tab(tab_t *tab);

rg_image_t *gui_get_image(const char *type, const char *subtype);

void gui_sort_list(tab_t *tab);
void gui_scroll_list(tab_t *tab, scroll_whence_t mode, int arg);
void gui_resize_list(tab_t *tab, int new_size);
listbox_item_t *gui_get_selected_item(tab_t *tab);

void gui_init(bool cold_boot);
void gui_invalidate(void);
void gui_update_theme(void);
void gui_save_config(void);
void gui_event(gui_event_t event, tab_t *tab);
void gui_redraw(void);
void gui_set_preview(tab_t *tab, rg_image_t *preview);
void gui_load_preview(tab_t *tab);
void gui_draw_background(tab_t *tab, int shade);
void gui_draw_header(tab_t *tab, int offset);
void gui_draw_status(tab_t *tab);
void gui_draw_list(tab_t *tab);
void gui_draw_tab_indicator(void);
