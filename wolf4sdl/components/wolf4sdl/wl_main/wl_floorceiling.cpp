#include "version.h"

#ifdef USE_FLOORCEILINGTEX

#include "wl_def.h"
#include "wl_shade.h"

// Textured Floor and Ceiling by DarkOne
// With multi-textured floors and ceilings stored in lower and upper bytes of
// according tile in third mapplane, respectively.
void DrawFloorAndCeiling(byte *vbuf, unsigned vbufPitch, int min_wallheight)
{
    fixed dist;                                // distance to row projection
    fixed tex_step;                            // global step per one screen pixel
    fixed gu, gv, du, dv;                      // global texture coordinates
    int u, v;                                  // local texture coordinates
    byte *toptex, *bottex;
    unsigned lasttoptex = 0xffffffff, lastbottex = 0xffffffff;

    int halfheight = viewheight >> 1;
    int y0 = min_wallheight >> 3;              // starting y value
    if(y0 > halfheight)
        return;                                // view obscured by walls
    if(!y0) y0 = 1;                            // don't let division by zero
    unsigned bot_offset0 = vbufPitch * (halfheight + y0);
    unsigned top_offset0 = vbufPitch * (halfheight - y0 - 1);

    // draw horizontal lines
    for(int y = y0, bot_offset = bot_offset0, top_offset = top_offset0;
        y < halfheight; y++, bot_offset += vbufPitch, top_offset -= vbufPitch)
    {
        dist = (heightnumerator / (y + 1)) << 5;
        gu =  viewx + FixedMul(dist, viewcos);
        gv = -viewy + FixedMul(dist, viewsin);
        tex_step = (dist << 8) / viewwidth / 175;
        du =  FixedMul(tex_step, viewsin);
        dv = -FixedMul(tex_step, viewcos);
        gu -= (viewwidth >> 1) * du;
        gv -= (viewwidth >> 1) * dv; // starting point (leftmost)
#ifdef USE_SHADING
        byte *curshades = shadetable[GetShade(y << 3)];
#endif
        for(int x = 0, bot_add = bot_offset, top_add = top_offset;
            x < viewwidth; x++, bot_add++, top_add++)
        {
            if(wallheight[x] >> 3 <= y)
            {
                int curx = (gu >> TILESHIFT) & (MAPSIZE - 1);
                int cury = (-(gv >> TILESHIFT) - 1) & (MAPSIZE - 1);
                unsigned curtex = MAPSPOT(curx, cury, 2);
                if(curtex)
                {
                    unsigned curtoptex = curtex >> 8;
                    if (curtoptex != lasttoptex)
                    {
                        lasttoptex = curtoptex;
                        toptex = PM_GetTexture(curtoptex);
                    }
                    unsigned curbottex = curtex & 0xff;
                    if (curbottex != lastbottex)
                    {
                        lastbottex = curbottex;
                        bottex = PM_GetTexture(curbottex);
                    }
                    u = (gu >> (TILESHIFT - TEXTURESHIFT)) & (TEXTURESIZE - 1);
                    v = (gv >> (TILESHIFT - TEXTURESHIFT)) & (TEXTURESIZE - 1);
                    unsigned texoffs = (u << TEXTURESHIFT) + (TEXTURESIZE - 1) - v;
#ifdef USE_SHADING
                    if(curtoptex)
                        vbuf[top_add] = curshades[toptex[texoffs]];
                    if(curbottex)
                        vbuf[bot_add] = curshades[bottex[texoffs]];
#else
                    if(curtoptex)
                        vbuf[top_add] = toptex[texoffs];
                    if(curbottex)
                        vbuf[bot_add] = bottex[texoffs];
#endif
                }
            }
            gu += du;
            gv += dv;
        }
    }
}

#endif
