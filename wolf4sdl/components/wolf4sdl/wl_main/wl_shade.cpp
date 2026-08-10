#include "version.h"

#ifdef USE_SHADING
#include "wl_def.h"
#include "wl_shade.h"

typedef struct {
    uint8_t destRed, destGreen, destBlue;   // values between 0 and 255
    uint8_t fogStrength;
} shadedef_t;

shadedef_t shadeDefs[] = {
    {   0,   0,   0, LSHADE_NOSHADING },
    {   0,   0,   0, LSHADE_NORMAL },
    {   0,   0,   0, LSHADE_FOG },
    {  40,  40,  40, LSHADE_NORMAL },
    {  60,  60,  60, LSHADE_FOG }
};

uint8_t shadetable[SHADE_COUNT][256];
int LSHADE_flag;

#ifdef USE_FEATUREFLAGS

// The lower 8-bit of the upper left tile of every map determine
// the used shading definition of shadeDefs.
static inline int GetShadeDefID()
{
    int shadeID = ffDataTopLeft & 0x00ff;
    assert(shadeID >= 0 && shadeID < lengthof(shadeDefs));
    return shadeID;
}

#else

static int GetShadeDefID()
{
    int shadeID;
    switch(gamestate.episode * 10 + mapon)
    {
        case  0: shadeID = 4; break;
        case  1:
        case  2:
        case  6: shadeID = 1; break;
        case  3: shadeID = 0; break;
        case  5: shadeID = 2; break;
        default: shadeID = 3; break;
    }
    assert(shadeID >= 0 && shadeID < lengthof(shadeDefs));
    return shadeID;
}

#endif


// Returns the palette index of the nearest matching color of the
// given RGB color in given palette
byte GetColor(byte red, byte green, byte blue, SDL_Color *palette)
{
    byte mincol = 0;
    double mindist = 200000.F, curdist, DRed, DGreen, DBlue;

    SDL_Color *palPtr = palette;

    for(int col = 0; col < 256; col++, palPtr++)
    {
        DRed   = (double) (red   - palPtr->r);
        DGreen = (double) (green - palPtr->g);
        DBlue  = (double) (blue  - palPtr->b);
        curdist = DRed * DRed + DGreen * DGreen + DBlue * DBlue;
        if(curdist < mindist)
        {
            mindist = curdist;
            mincol = (byte) col;
        }
    }
    return mincol;
}

// Fade all colors in 32 steps down to the destination-RGB
// (use gray for fogging, black for standard shading)
void GenerateShadeTable(byte destRed, byte destGreen, byte destBlue,
                        SDL_Color *palette, int fog)
{
    double curRed, curGreen, curBlue, redStep, greenStep, blueStep;
    SDL_Color *palPtr = palette;

    // Set the fog-flag
    LSHADE_flag=fog;

    // Color loop
    for(int i = 0; i < 256; i++, palPtr++)
    {
        // Get original palette color
        curRed   = palPtr->r;
        curGreen = palPtr->g;
        curBlue  = palPtr->b;

        // Calculate increment per step
        redStep   = ((double) destRed   - curRed)   / (SHADE_COUNT + 8);
        greenStep = ((double) destGreen - curGreen) / (SHADE_COUNT + 8);
        blueStep  = ((double) destBlue  - curBlue)  / (SHADE_COUNT + 8);

        // Calc color for each shade of the current color
        for (int shade = 0; shade < SHADE_COUNT; shade++)
        {
            shadetable[shade][i] = GetColor((byte) curRed, (byte) curGreen, (byte) curBlue, palette);

            // Inc to next shade
            curRed   += redStep;
            curGreen += greenStep;
            curBlue  += blueStep;
        }
    }
}

void NoShading()
{
    for(int shade = 0; shade < SHADE_COUNT; shade++)
        for(int i = 0; i < 256; i++)
            shadetable[shade][i] = i;
}

void InitLevelShadeTable()
{
    shadedef_t *shadeDef = &shadeDefs[GetShadeDefID()];
    if(shadeDef->fogStrength == LSHADE_NOSHADING)
        NoShading();
    else
        GenerateShadeTable(shadeDef->destRed, shadeDef->destGreen, shadeDef->destBlue, gamepal, shadeDef->fogStrength);
}

int GetShade(int scale)
{
    int shade = (scale >> 1) / (((viewwidth * 3) >> 8) + 1 + LSHADE_flag);  // TODO: reconsider this...
    if(shade > 32) shade = 32;
    else if(shade < 1) shade = 1;
    shade = 32 - shade;

    return shade;
}

#endif
