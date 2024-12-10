#include "../rg_gui.h"

/**
 * This file can be edited to add fonts to retro-go.
 * There is a tool to convert ttf to prop fonts there:
 * https://github.com/loboris/ESP32_TFT_library/tree/master/tools
 * Note: The header must be removed (the first 4 bytes of data)
 */

extern const rg_font_t font_verabold12;

enum {
    RG_FONT_VERABOLD_12,

    RG_FONT_MAX,
};

static const rg_font_t *fonts[RG_FONT_MAX] = {
    &font_verabold12,
};
