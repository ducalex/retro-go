#include <rg_system.h>
#include <string.h>
#include <stdlib.h>

#include "applications.h"
#include "gui.h"

#define HEADER_HEIGHT       (50)
#define LOGO_WIDTH          (46)
#define PREVIEW_HEIGHT      ((int)(gui.height * 0.70f))
#define PREVIEW_WIDTH       ((int)(gui.width * 0.50f))

retro_gui_t gui;

#define SETTING_SELECTED_TAB    "SelectedTab"
#define SETTING_START_SCREEN    "StartScreen"
#define SETTING_STARTUP_MODE    "StartupMode"
#define SETTING_THEME           "Theme"
#define SETTING_COLOR_THEME     "ColorTheme"
#define SETTING_SHOW_PREVIEW    "ShowPreview"
#define SETTING_SCROLL_MODE     "ScrollMode"
#define SETTING_HIDDEN_TABS     "HiddenTabs"
#define SETTING_HIDE_TAB(name)  strcat((char[99]){"HideTab."}, (name))

static int max_visible_lines(const tab_t *tab, int *_line_height)
{
    int line_height = TEXT_RECT("ABC123", 0).height;
    if (_line_height) *_line_height = line_height;
    return (gui.height - (HEADER_HEIGHT + 6) - (tab->navpath ? line_height : 0)) / line_height;
}

void gui_init(bool cold_boot)
{
    gui = (retro_gui_t){
        .selected_tab = rg_settings_get_number(NS_APP, SETTING_SELECTED_TAB, 0),
        .startup_mode = rg_settings_get_number(NS_APP, SETTING_STARTUP_MODE, 0),
        .hidden_tabs  = rg_settings_get_string(NS_APP, SETTING_HIDDEN_TABS, ""),
        .color_theme  = rg_settings_get_number(NS_APP, SETTING_COLOR_THEME, 0),
        .start_screen = rg_settings_get_number(NS_APP, SETTING_START_SCREEN, START_SCREEN_AUTO),
        .show_preview = rg_settings_get_number(NS_APP, SETTING_SHOW_PREVIEW, PREVIEW_MODE_SAVE_COVER),
        .scroll_mode  = rg_settings_get_number(NS_APP, SETTING_SCROLL_MODE, SCROLL_MODE_CENTER),
        .width        = rg_display_get_info()->screen.width,
        .height       = rg_display_get_info()->screen.height,
    };
    // Auto: Show carousel on cold boot, browser on warm boot (after cleanly exiting an emulator)
    gui.browse = gui.start_screen == START_SCREEN_BROWSER || (gui.start_screen == START_SCREEN_AUTO && !cold_boot);
    gui.surface = rg_surface_create(gui.width, gui.height, RG_PIXEL_565_LE, MEM_SLOW);
    gui_update_theme();
}

void gui_event(gui_event_t event, tab_t *tab)
{
    if (tab && tab->event_handler)
        (*tab->event_handler)(event, tab);
}

tab_t *gui_add_tab(const char *name, const char *desc, void *arg, void *event_handler)
{
    RG_ASSERT_ARG(name && desc);

    tab_t *tab = calloc(1, sizeof(tab_t));

    snprintf(tab->name, sizeof(tab->name), "%s", name);
    snprintf(tab->desc, sizeof(tab->desc), "%s", desc);
    sprintf(tab->status[1].left, "Loading...");

    tab->event_handler = event_handler;
    tab->initialized = false;
    tab->enabled = !rg_settings_get_number(NS_APP, SETTING_HIDE_TAB(name), 0);
    tab->arg = arg;
    tab->listbox = (listbox_t){
        .items = calloc(10, sizeof(listbox_item_t)),
        .capacity = 10,
        .length = 0,
        .cursor = 0,
        .sort_mode = SORT_TEXT_ASC,
    };

    gui.tabs[gui.tabs_count++] = tab;

    RG_LOGI("Tab '%s' added at index %d\n", tab->name, gui.tabs_count - 1);

    return tab;
}

void gui_init_tab(tab_t *tab)
{
    if (tab->initialized)
        return;

    tab->initialized = true;
    // tab->status[0] = 0;

    gui_event(TAB_INIT, tab);
    gui_scroll_list(tab, SCROLL_SET, tab->listbox.cursor);
}

tab_t *gui_get_tab(int index)
{
    return (index >= 0 && index < gui.tabs_count) ? gui.tabs[index] : NULL;
}

void gui_invalidate(void)
{
    for (size_t i = 0; i < gui.tabs_count; ++i)
    {
        if (gui.tabs[i]->initialized)
            gui_event(TAB_RESCAN, gui.tabs[i]);
    }
}

rg_image_t *gui_get_image(const char *type, const char *subtype)
{
    char name[64];

    if (subtype && *subtype)
        snprintf(name, sizeof(name), "%s_%s.png", type, subtype);
    else
        snprintf(name, sizeof(name), "%s.png", type);

    // Try to get image from theme
    rg_image_t *img = rg_gui_get_theme_image(name);
    if (img)
        return img;

    // Then fallback to built-in images
    for (const binfile_t **img = builtin_images; *img; img++)
    {
        if (strcmp((*img)->name, name) == 0)
            return rg_surface_load_image((*img)->data, (*img)->size, 0);
    }

    return NULL;
}

tab_t *gui_get_current_tab(void)
{
    tab_t *tab = gui_get_tab(gui.selected_tab);
    if (!tab)
        RG_LOGE("current tab is NULL!");
    return tab;
}

tab_t *gui_set_current_tab(int index)
{
    tab_t *prev_tab = gui_get_tab(gui.selected_tab);
    tab_t *curr_tab;

    index %= (int)gui.tabs_count;

    if (index < 0)
        index += gui.tabs_count;

    gui.selected_tab = index;

    curr_tab = gui_get_tab(gui.selected_tab);

    if (prev_tab && prev_tab != curr_tab)
    {
        // FIXME: We should recompress the images rather than fully free them, because if a custom theme is
        //        used then it means that we're constantly reloading from SD Card which is very slow...
        rg_surface_free(prev_tab->background), prev_tab->background = NULL;
        // rg_surface_free(prev_tab->banner), prev_tab->banner = NULL;
        // rg_surface_free(prev_tab->logo), prev_tab->logo = NULL;
    }

    return curr_tab;
}

void gui_set_status(tab_t *tab, const char *left, const char *right)
{
    if (!tab)
        tab = gui_get_current_tab();
    if (tab && left)
        strcpy(tab->status[1].left, left);
    if (tab && right)
        strcpy(tab->status[1].right, right);
}

void gui_update_theme(void)
{
    // Load our four color schemes from gui theme
    gui.themes[0].list.standard_bg = rg_gui_get_theme_color("launcher_1", "list_standard_bg", C_TRANSPARENT);
    gui.themes[0].list.standard_fg = rg_gui_get_theme_color("launcher_1", "list_standard_fg", C_GRAY);
    gui.themes[0].list.selected_bg = rg_gui_get_theme_color("launcher_1", "list_selected_bg", C_TRANSPARENT);
    gui.themes[0].list.selected_fg = rg_gui_get_theme_color("launcher_1", "list_selected_bg", C_WHITE);

    gui.themes[1].list.standard_bg = rg_gui_get_theme_color("launcher_2", "list_standard_bg", C_TRANSPARENT);
    gui.themes[1].list.standard_fg = rg_gui_get_theme_color("launcher_2", "list_standard_fg", C_GRAY);
    gui.themes[1].list.selected_bg = rg_gui_get_theme_color("launcher_2", "list_selected_bg", C_TRANSPARENT);
    gui.themes[1].list.selected_fg = rg_gui_get_theme_color("launcher_2", "list_selected_bg", C_GREEN);

    gui.themes[2].list.standard_bg = rg_gui_get_theme_color("launcher_3", "list_standard_bg", C_TRANSPARENT);
    gui.themes[2].list.standard_fg = rg_gui_get_theme_color("launcher_3", "list_standard_fg", C_GRAY);
    gui.themes[2].list.selected_bg = rg_gui_get_theme_color("launcher_3", "list_selected_bg", C_WHITE);
    gui.themes[2].list.selected_fg = rg_gui_get_theme_color("launcher_3", "list_selected_bg", C_BLACK);

    gui.themes[3].list.standard_bg = rg_gui_get_theme_color("launcher_4", "list_standard_bg", C_TRANSPARENT);
    gui.themes[3].list.standard_fg = rg_gui_get_theme_color("launcher_4", "list_standard_fg", C_DARK_GRAY);
    gui.themes[3].list.selected_bg = rg_gui_get_theme_color("launcher_4", "list_selected_bg", C_WHITE);
    gui.themes[3].list.selected_fg = rg_gui_get_theme_color("launcher_4", "list_selected_bg", C_BLACK);

    // Flush our image cache to make sure the new images are loaded next time
    for (size_t i = 0; i < gui.tabs_count; ++i)
    {
        tab_t *tab = gui.tabs[i];
        rg_surface_free(tab->background), tab->background = NULL;
        rg_surface_free(tab->banner), tab->banner = NULL;
        rg_surface_free(tab->logo), tab->logo = NULL;
    }
}

void gui_save_config(void)
{
    rg_settings_set_number(NS_APP, SETTING_SELECTED_TAB, gui.selected_tab);
    rg_settings_set_number(NS_APP, SETTING_START_SCREEN, gui.start_screen);
    rg_settings_set_number(NS_APP, SETTING_SHOW_PREVIEW, gui.show_preview);
    rg_settings_set_number(NS_APP, SETTING_SCROLL_MODE, gui.scroll_mode);
    rg_settings_set_number(NS_APP, SETTING_COLOR_THEME, gui.color_theme);
    rg_settings_set_number(NS_APP, SETTING_STARTUP_MODE, gui.startup_mode);
    rg_settings_set_string(NS_APP, SETTING_HIDDEN_TABS, gui.hidden_tabs);
    for (int i = 0; i < gui.tabs_count; i++)
        rg_settings_set_number(NS_APP, SETTING_HIDE_TAB(gui.tabs[i]->name), !gui.tabs[i]->enabled);
    rg_settings_commit();
}

listbox_item_t *gui_get_selected_item(tab_t *tab)
{
    if (tab)
    {
        listbox_t *list = &tab->listbox;
        if (list->cursor >= 0 && list->cursor < list->length)
            return &list->items[list->cursor];
    }
    return NULL;
}

static int list_comp_text_asc(const listbox_item_t *a, const listbox_item_t *b)
{
    return a->group == b->group ? strcasecmp(a->text, b->text) : ((int)a->group - b->group);
}

static int list_comp_text_desc(const listbox_item_t *a, const listbox_item_t *b)
{
    return a->group == b->group ? strcasecmp(b->text, a->text) : ((int)a->group - b->group);
}

static int list_comp_id_asc(const listbox_item_t *a, const listbox_item_t *b)
{
    return a->group == b->group ? ((int)a->order - b->order) : ((int)a->group - b->group);
}

static int list_comp_id_desc(const listbox_item_t *a, const listbox_item_t *b)
{
    return a->group == b->group ? ((int)b->order - a->order) : ((int)a->group - b->group);
}

void gui_sort_list(tab_t *tab)
{
    void *comp[] = {&list_comp_id_asc, &list_comp_id_desc, &list_comp_text_asc, &list_comp_text_desc};
    size_t sort_mode = tab->listbox.sort_mode - 1;

    if (!tab->listbox.length || sort_mode > RG_COUNT(comp) - 1)
        return;

    qsort((void*)tab->listbox.items, tab->listbox.length, sizeof(listbox_item_t), comp[sort_mode]);
}

void gui_resize_list(tab_t *tab, int new_size)
{
    listbox_t *list = &tab->listbox;

    if (new_size == list->length)
        return;

    // Always grow but only shrink past a certain threshold
    if (new_size >= list->capacity || list->capacity - new_size >= 20)
    {
        list->capacity = new_size + 10;
        list->items = realloc(list->items, list->capacity * sizeof(listbox_item_t));
        RG_LOGI("Resized list '%s' from %d to %d items (new capacity: %d)\n",
            tab->name, list->length, new_size, list->capacity);
    }

    for (int i = list->length; i < list->capacity; i++)
        memset(&list->items[i], 0, sizeof(listbox_item_t));

    list->length = new_size;

    if (list->cursor >= new_size)
        list->cursor = new_size ? new_size - 1 : 0;
}

void gui_scroll_list(tab_t *tab, scroll_whence_t mode, int arg)
{
    listbox_t *list = &tab->listbox;

    int cur_cursor = RG_MAX(RG_MIN(list->cursor, list->length - 1), 0);
    int old_cursor = list->cursor;

    if (list->length == 0)
    {
        // cur_cursor = -1;
        cur_cursor = 0;
    }
    else if (mode == SCROLL_SET)
    {
        cur_cursor = arg;
    }
    else if (mode == SCROLL_LINE)
    {
        cur_cursor += arg;
    }
    else if (mode == SCROLL_PAGE)
    {
        // int start = list->items[cur_cursor].text[0];
        int direction = arg > 0 ? 1 : -1;
        for (int max = max_visible_lines(tab, NULL); max > 0; --max)
        {
            cur_cursor += direction;
            if (cur_cursor < 0 || cur_cursor >= list->length)
                break;
            // if (start != list->items[cur_cursor].text[0])
            //     break;
        }
    }

    if (cur_cursor < 0) cur_cursor = list->length - 1;
    if (cur_cursor >= list->length) cur_cursor = 0;

    list->cursor = cur_cursor;

    if (list->length && list->items[list->cursor].arg)
        sprintf(tab->status[0].left, "%d / %d", (list->cursor + 1) % 10000, list->length % 10000);
    else
        strcpy(tab->status[0].left, "List empty");

    gui_event(TAB_SCROLL, tab);

    if (cur_cursor != old_cursor)
    {
        gui_redraw();
    }
}

void gui_redraw(void)
{
    rg_display_sync(true);
    rg_gui_set_surface(gui.surface);

    tab_t *tab = gui_get_current_tab();
    if (!tab)
    {
        RG_LOGW("No tab to redraw...");
    }
    else if (gui.browse)
    {
        gui_draw_background(tab, 4);
        gui_draw_header(tab, 0);
        gui_draw_status(tab);
        gui_draw_list(tab);
        if (tab->preview)
        {
            int height = RG_MIN(tab->preview->height, PREVIEW_HEIGHT);
            int width = RG_MIN(tab->preview->width, PREVIEW_WIDTH);
            rg_gui_draw_image(-width, -height, width, height, true, tab->preview);
        }
    }
    else
    {
        gui_draw_background(tab, 0);
        gui_draw_header(tab, (gui.height - HEADER_HEIGHT) / 2);
        // gui_draw_tab_indicator();
    }

    rg_gui_set_surface(NULL);
    rg_display_submit(gui.surface, 0);
}

void gui_draw_background(tab_t *tab, int shade)
{
    // We can't losslessly change shade, must reload!
    if (tab->background && tab->background_shade > 0 && tab->background_shade != shade)
    {
        rg_surface_free(tab->background);
        tab->background = NULL;
    }

    if (!tab->background)
    {
        tab->background = gui_get_image("background", tab->name);
        tab->background_shade = 0;
        if (tab->background && (tab->background->width != gui.width || tab->background->height != gui.height))
        {
            rg_image_t *temp = rg_surface_resize(tab->background, gui.width, gui.height);
            if (temp)
            {
                rg_surface_free(tab->background);
                tab->background = temp;
            }
        }
    }

    if (tab->background && tab->background_shade != shade && shade > 0)
    {
        rg_image_t *img = tab->background;
        for (int y = 0; y < img->height; ++y)
        {
            uint16_t *line = img->data + y * img->stride;
            for (int x = 0; x < img->width; ++x)
            {
                int pixel = line[x];
                int r = ((pixel >> 11) & 0x1F) / shade;
                int g = ((pixel >> 5) & 0x3F) / shade;
                int b = ((pixel) & 0x1F) / shade;
                line[x] = ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | ((b & 0x1F) << 0);
            }
        }
        tab->background_shade = shade;
    }

    rg_gui_draw_image(0, 0, gui.width, gui.height, false, tab->background);
}

void gui_draw_header(tab_t *tab, int offset)
{
    if (!tab->banner)
        tab->banner = gui_get_image("banner", tab->name);
    if (!tab->logo)
        tab->logo = gui_get_image("logo", tab->name);

    rg_gui_draw_image(0, offset, LOGO_WIDTH, HEADER_HEIGHT, false, tab->logo);
    if (tab->banner)
        rg_gui_draw_image(LOGO_WIDTH + 1, offset + 8, 0, HEADER_HEIGHT - 8, false, tab->banner);
    else
        rg_gui_draw_text(LOGO_WIDTH + 8, offset + 8, 0, tab->desc, C_SNOW, C_BLACK, RG_TEXT_BIGGER);
}

void gui_draw_tab_indicator(void)
{
    char buffer[64] = {0};
    memset(buffer, '-', gui.tabs_count);
    rg_gui_draw_text(RG_GUI_CENTER, RG_GUI_BOTTOM, 0, buffer, C_DIM_GRAY, C_TRANSPARENT, RG_TEXT_BIGGER|RG_TEXT_MONOSPACE);
    memset(buffer, ' ', gui.tabs_count);
    buffer[gui.selected_tab] = '-';
    rg_gui_draw_text(RG_GUI_CENTER, RG_GUI_BOTTOM, 0, buffer, C_SNOW, C_TRANSPARENT, RG_TEXT_BIGGER|RG_TEXT_MONOSPACE);
}

void gui_draw_status(tab_t *tab)
{
    const int status_x = LOGO_WIDTH + 12;
    const int status_y = HEADER_HEIGHT - 16;
    char *txt_left = tab->status[tab->status[1].left[0] ? 1 : 0].left;
    char *txt_right = tab->status[tab->status[1].right[0] ? 1 : 0].right;

    rg_gui_draw_text(status_x, status_y, gui.width - status_x, txt_right, C_SNOW, C_TRANSPARENT, RG_TEXT_ALIGN_LEFT);
    rg_gui_draw_text(status_x, status_y, 0, txt_left, C_WHITE, C_TRANSPARENT, RG_TEXT_ALIGN_RIGHT);
    rg_gui_draw_icons();
}

void gui_draw_list(tab_t *tab)
{
    const theme_t *theme = &gui.themes[gui.color_theme % RG_COUNT(gui.themes)];
    rg_color_t fg[2] = {theme->list.standard_fg, theme->list.selected_fg};
    rg_color_t bg[2] = {theme->list.standard_bg, theme->list.selected_bg};

    const listbox_t *list = &tab->listbox;
    int line_height, top = HEADER_HEIGHT + 6;
    int lines = max_visible_lines(tab, &line_height);
    int line_offset = 0;

    if (tab->navpath)
    {
        char buffer[64];
        snprintf(buffer, 63, "[%s]",  tab->navpath);
        top += rg_gui_draw_text(0, top, gui.width, buffer, fg[0], bg[0], 0).height;
    }

    top += ((gui.height - top) - (lines * line_height)) / 2;

    if (gui.scroll_mode == SCROLL_MODE_PAGING)
    {
        line_offset = (list->cursor / lines) * lines;
    }
    else // (gui.scroll_mode == SCROLL_MODE_CENTER)
    {
        line_offset = list->cursor - (lines / 2);
    }

    for (int i = 0; i < lines; i++)
    {
        int idx = line_offset + i;
        int selected = idx == list->cursor;
        char *label = (idx >= 0 && idx < list->length) ? list->items[idx].text : "";
        top += rg_gui_draw_text(0, top, gui.width, label, fg[selected], bg[selected], 0).height;
    }
}

void gui_set_preview(tab_t *tab, rg_image_t *preview)
{
    if (!tab)
        return;

    if (tab->preview)
        rg_surface_free(tab->preview);

    tab->preview = preview;
}

void gui_load_preview(tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    bool show_missing_cover = false;
    uint32_t order;

    gui_set_preview(tab, NULL);

    if (!item || !item->arg)
        return;

    switch (gui.show_preview)
    {
        case PREVIEW_MODE_COVER_SAVE:
            show_missing_cover = true;
            order = 0x4123;
            break;
        case PREVIEW_MODE_SAVE_COVER:
            show_missing_cover = true;
            order = 0x1234;
            break;
        case PREVIEW_MODE_COVER_ONLY:
            show_missing_cover = true;
            order = 0x0123;
            break;
        case PREVIEW_MODE_SAVE_ONLY:
            show_missing_cover = false;
            order = 0x0004;
            break;
        default:
            show_missing_cover = false;
            order = 0x0000;
    }

    retro_file_t *file = item->arg;
    retro_app_t *app = file->app;
    uint32_t errors = 0;

    while (order && !tab->preview)
    {
        char path[RG_PATH_MAX + 1];
        size_t path_len = 0;
        int type = order & 0xF;

        order >>= 4;

        // Give up on any button press to improve responsiveness
        if ((gui.joystick |= rg_input_read_gamepad()))
            break;

        if (file->missing_cover & (1 << type))
            continue;

        if (type == 0x1 && app->use_crc_covers && application_get_file_crc32(file)) // Game cover (old format)
            path_len = snprintf(path, RG_PATH_MAX, "%s/%X/%08X.art", app->paths.covers, (int)(file->checksum >> 28), (int)file->checksum);
        else if (type == 0x2 && app->use_crc_covers && application_get_file_crc32(file)) // Game cover (png)
            path_len = snprintf(path, RG_PATH_MAX, "%s/%X/%08X.png", app->paths.covers, (int)(file->checksum >> 28), (int)file->checksum);
        else if (type == 0x3) // Game cover (based on filename)
        {
            path_len = snprintf(path, RG_PATH_MAX, "%s/%s", app->paths.covers, file->name);
            if (path_len < RG_PATH_MAX - 3) // Don't bother if we already have an overflow
                strcpy(path + path_len - strlen(rg_extension(file->name) ?: ""), "png");
        }
        else if (type == 0x4 && file->saves > 0) // Save state screenshot (png)
        {
            snprintf(path, RG_PATH_MAX, "%s/%s", file->folder, file->name);
            uint8_t last_used_slot = rg_emu_get_last_used_slot(path);
            if (last_used_slot != 0xFF)
            {
                char *preview = rg_emu_get_path(RG_PATH_SCREENSHOT + last_used_slot, path);
                path_len = snprintf(path, RG_PATH_MAX, "%s", preview);
                free(preview);
            }
        }

        if (path_len > 0 && path_len < RG_PATH_MAX)
        {
            RG_LOGD("Looking for %s", path);
            gui_set_preview(tab, rg_surface_load_image_file(path, 0));
            // if (!tab->preview && rg_storage_exists(path))
            //     errors++;
        }

        file->missing_cover |= (tab->preview ? 0 : 1) << type;
    }

    if (!tab->preview && file->checksum && (show_missing_cover || errors))
    {
        RG_LOGI("No image found for '%s'\n", file->name);
        gui_set_status(tab, NULL, errors ? "Bad cover" : "No cover");
        // gui_draw_status(tab);
        // tab->preview = gui_get_image("cover", file->app);
    }
}
