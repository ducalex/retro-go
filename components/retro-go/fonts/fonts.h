#include <rg_gui.h>

/**
 * This file can be edited to add fonts to retro-go.
 * There is a tool to convert ttf to prop fonts there:
 * https://github.com/loboris/ESP32_TFT_library/tree/master/tools
 * Note: The header must be removed (the first 4 bytes of data)
 */

extern const rg_font_t font_basic8x8;
extern const rg_font_t font_DejaVu12;
extern const rg_font_t font_DejaVu15;
extern const rg_font_t font_VeraBold12;
extern const rg_font_t font_VeraBold15;

static const rg_font_t *fonts[] = {
    &font_basic8x8,
    &font_basic8x8,
    &font_basic8x8,
    &font_DejaVu12,
    &font_DejaVu15,
    &font_VeraBold12,
    &font_VeraBold15,
};
static const size_t fonts_count = sizeof(fonts) / sizeof(rg_font_t*);
