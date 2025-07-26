#include "../rg_gui.h"

/**
 * This file can be edited to add fonts to retro-go.
 * To create new fonts you can use font_converter.py located in the tools folder.
 */

extern const rg_font_t font_basic8x8;
extern const rg_font_t font_DejaVu12;
extern const rg_font_t font_DejaVu15;
extern const rg_font_t font_VeraBold11;
extern const rg_font_t font_VeraBold14;

enum {
    RG_FONT_BASIC_8,
    RG_FONT_BASIC_12,
    RG_FONT_BASIC_16,
    RG_FONT_DEJAVU_12,
    RG_FONT_DEJAVU_15,
    RG_FONT_VERA_11,
    RG_FONT_VERA_14,
    RG_FONT_MAX,
};

static const rg_font_t *fonts[RG_FONT_MAX] = {
    &font_basic8x8,
    &font_basic8x8,
    &font_basic8x8,
    &font_DejaVu12,
    &font_DejaVu15,
    &font_VeraBold11,
    &font_VeraBold14,
};
