#include "../rg_gui.h"

/**
 * This file can be edited to add fonts to retro-go.
 * There is a tool to convert ttf to prop fonts there:
 * https://github.com/loboris/ESP32_TFT_library/tree/master/tools
 * Note: The header must be removed (the first 4 bytes of data)
 * 
 * https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md
 * 
 */

extern const rg_font_t font_Vera_Bold12;

enum {
    RG_FONT_VERABOLD_12,

    RG_FONT_MAX,
};

static const rg_font_t *fonts[RG_FONT_MAX] = {
    &font_Vera_Bold12,
};
