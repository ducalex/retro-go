#include "version.h"

#ifdef USE_PARALLAX

#include "wl_def.h"

#ifdef USE_FEATUREFLAGS

// The lower left tile of every map determines the start texture of the parallax sky.
static int GetParallaxStartTexture()
{
    int startTex = ffDataBottomLeft;
    assert(startTex >= 0 && startTex < PMSpriteStart);
    return startTex;
}

#else

static int GetParallaxStartTexture()
{
    int startTex;
    switch(gamestate.episode * 10 + mapon)
    {
        case  0: startTex = 20; break;
        default: startTex =  0; break;
    }
    assert(startTex >= 0 && startTex < PMSpriteStart);
    return startTex;
}

#endif

void DrawParallax(byte *vbuf, unsigned vbufPitch)
{
    int startpage = GetParallaxStartTexture();
    int midangle = player->angle * (FINEANGLES / ANGLES);
    int skyheight = viewheight >> 1;
    int curtex = -1;
    byte *skytex;

    startpage += USE_PARALLAX - 1;

    for(int x = 0; x < viewwidth; x++)
    {
        int curang = pixelangle[x] + midangle;
        if(curang < 0) curang += FINEANGLES;
        else if(curang >= FINEANGLES) curang -= FINEANGLES;
        int xtex = curang * USE_PARALLAX * TEXTURESIZE / FINEANGLES;
        int newtex = xtex >> TEXTURESHIFT;
        if(newtex != curtex)
        {
            curtex = newtex;
            skytex = PM_GetTexture(startpage - curtex);
        }
        int texoffs = TEXTUREMASK - ((xtex & (TEXTURESIZE - 1)) << TEXTURESHIFT);
        int yend = skyheight - (wallheight[x] >> 3);
        if(yend <= 0) continue;

        for(int y = 0, offs = x; y < yend; y++, offs += vbufPitch)
            vbuf[offs] = skytex[texoffs + (y * TEXTURESIZE) / skyheight];
    }
}

#endif
