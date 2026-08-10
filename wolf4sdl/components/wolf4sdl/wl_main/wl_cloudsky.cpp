#include "version.h"

#ifdef USE_CLOUDSKY

#include "wl_def.h"
#include "wl_cloudsky.h"

// Each colormap defines a number of colors which should be mapped from
// the skytable. The according colormapentry_t array defines how these colors should
// be mapped to the wolfenstein palette. The first int of each entry defines
// how many colors are grouped to this entry and the absolute value of the
// second int sets the starting palette index for this pair. If this value is
// negative the index will be decremented for every color, if it's positive
// it will be incremented.
//
// Example colormap:
//   colormapentry_t colmapents_1[] = { { 6, -10 }, { 2, 40 } };
//   colormap_t colorMaps[] = {
//      { 8, colmapents_1 }
//   };
//
//   The colormap 0 consists of 8 colors. The first color group consists of 6
//   colors and starts descending at palette index 10: 10, 9, 8, 7, 6, 5
//   The second color group consists of 2 colors and starts ascending at
//   index 40: 40, 41
//   There's no other color group because all colors of this colormap are
//   already used (6+2=8)
//
// Warning: Always make sure that the sum of the amount of the colors in all
//          color groups is the number of colors used for your colormap!

colormapentry_t colmapents_1[] = { { 16, -31 }, { 16, 136 } };
colormapentry_t colmapents_2[] = { { 16, -31 } };

colormap_t colorMaps[] = {
    { 32, colmapents_1 },
    { 16, colmapents_2 }
};

const int numColorMaps = lengthof(colorMaps);

// The sky definitions which can be selected as defined by GetCloudSkyDefID() in wl_def.h
// You can use <TAB>+Z in debug mode to find out suitable values for seed and colorMapIndex
// Each entry consists of seed, speed, angle and colorMapIndex
cloudsky_t cloudSkys[] = {
    { 626,   800,  20,  0 },
    { 1234,  650,  60,  1 },
    { 0,     700,  120, 0 },
    { 0,     0,    0,   0 },
    { 11243, 750,  310, 0 },
    { 32141, 750,  87,  0 },
    { 12124, 750,  64,  0 },
    { 55543, 500,  240, 0 },
    { 65535, 200,  54,  1 },
    { 4,     1200, 290, 0 },
};

byte skyc[65536L];

long cloudx = 0, cloudy = 0;
cloudsky_t *curSky = NULL;

#ifdef USE_FEATUREFLAGS

// The lower left tile of every map determines the used cloud sky definition from cloudSkys.
static int GetCloudSkyDefID()
{
    int skyID = ffDataBottomLeft;
    assert(skyID >= 0 && skyID < lengthof(cloudSkys));
    return skyID;
}

#else

static int GetCloudSkyDefID()
{
    int skyID;
    switch(gamestate.episode * 10 + mapon)
    {
        case  0: skyID =  0; break;
        case  1: skyID =  1; break;
        case  2: skyID =  2; break;
        case  3: skyID =  3; break;
        case  4: skyID =  4; break;
        case  5: skyID =  5; break;
        case  6: skyID =  6; break;
        case  7: skyID =  7; break;
        case  8: skyID =  8; break;
        case  9: skyID =  9; break;
        default: skyID =  9; break;
    }
    assert(skyID >= 0 && skyID < lengthof(cloudSkys));
    return skyID;
}

#endif

void SplitS(unsigned size,unsigned x1,unsigned y1,unsigned x2,unsigned y2)
{
   if(size==1) return;
   if(!skyc[((x1+size/2)*256+y1)])
   {
      skyc[((x1+size/2)*256+y1)]=(byte)(((int)skyc[(x1*256+y1)]
            +(int)skyc[((x2&0xff)*256+y1)])/2)+rand()%(size*2)-size;
      if(!skyc[((x1+size/2)*256+y1)]) skyc[((x1+size/2)*256+y1)]=1;
   }
   if(!skyc[((x1+size/2)*256+(y2&0xff))])
   {
      skyc[((x1+size/2)*256+(y2&0xff))]=(byte)(((int)skyc[(x1*256+(y2&0xff))]
            +(int)skyc[((x2&0xff)*256+(y2&0xff))])/2)+rand()%(size*2)-size;
      if(!skyc[((x1+size/2)*256+(y2&0xff))])
         skyc[((x1+size/2)*256+(y2&0xff))]=1;
   }
   if(!skyc[(x1*256+y1+size/2)])
   {
      skyc[(x1*256+y1+size/2)]=(byte)(((int)skyc[(x1*256+y1)]
            +(int)skyc[(x1*256+(y2&0xff))])/2)+rand()%(size*2)-size;
      if(!skyc[(x1*256+y1+size/2)]) skyc[(x1*256+y1+size/2)]=1;
   }
   if(!skyc[((x2&0xff)*256+y1+size/2)])
   {
      skyc[((x2&0xff)*256+y1+size/2)]=(byte)(((int)skyc[((x2&0xff)*256+y1)]
            +(int)skyc[((x2&0xff)*256+(y2&0xff))])/2)+rand()%(size*2)-size;
      if(!skyc[((x2&0xff)*256+y1+size/2)]) skyc[((x2&0xff)*256+y1+size/2)]=1;
   }

   skyc[((x1+size/2)*256+y1+size/2)]=(byte)(((int)skyc[(x1*256+y1)]
         +(int)skyc[((x2&0xff)*256+y1)]+(int)skyc[(x1*256+(y2&0xff))]
         +(int)skyc[((x2&0xff)*256+(y2&0xff))])/4)+rand()%(size*2)-size;

   SplitS(size/2,x1,y1+size/2,x1+size/2,y2);
   SplitS(size/2,x1+size/2,y1,x2,y1+size/2);
   SplitS(size/2,x1+size/2,y1+size/2,x2,y2);
   SplitS(size/2,x1,y1,x1+size/2,y1+size/2);
}

void InitSky()
{
    unsigned cloudskyid = GetCloudSkyDefID();
    if(cloudskyid >= lengthof(cloudSkys))
        Quit("Illegal cloud sky id: %u", cloudskyid);
    curSky = &cloudSkys[cloudskyid];

    memset(skyc, 0, sizeof(skyc));
    // funny water texture if used instead of memset ;D
    // for(int i = 0; i < 65536; i++)
    //     skyc[i] = rand() % 32 * 8;

    srand(curSky->seed);
    skyc[0] = rand() % 256;
    SplitS(256, 0, 0, 256, 256);

    // Smooth the clouds a bit
    for(int k = 0; k < 2; k++)
    {
        for(int i = 0; i < 256; i++)
        {
            for(int j = 0; j < 256; j++)
            {
                int32_t val = -skyc[j * 256 + i];
                for(int m = 0; m < 3; m++)
                {
                    for(int n = 0; n < 3; n++)
                    {
                        val += skyc[((j + n - 1) & 0xff) * 256 + ((i + m - 1) & 0xff)];
                    }
                }
                skyc[j * 256 + i] = (byte)(val >> 3);
            }
        }
    }

    // the following commented line could be useful, if you're trying to
    // create a new color map. This will display your current color map
    // in one (of course repeating) stripe of the sky

    // for(int i = 0; i < 256; i++)
    //     skyc[i] = skyc[i + 256] = skyc[i + 512] = i;

    if(curSky->colorMapIndex >= lengthof(colorMaps))
        Quit("Illegal colorMapIndex for cloud sky def %u: %u", cloudskyid, curSky->colorMapIndex);

    colormap_t *curMap = &colorMaps[curSky->colorMapIndex];
    int numColors = curMap->numColors;
    byte colormap[256];
    colormapentry_t *curEntry = curMap->entries;
    for(int calcedCols = 0; calcedCols < numColors; curEntry++)
    {
        if(curEntry->startAndDir < 0)
        {
            for(int i = 0, ind = -curEntry->startAndDir; i < curEntry->length; i++, ind--)
                colormap[calcedCols++] = ind;
        }
        else
        {
            for(int i = 0, ind = curEntry->startAndDir; i < curEntry->length; i++, ind++)
                colormap[calcedCols++] = ind;
        }
    }

    for(int i = 0; i < 256; i++)
    {
        for(int j = 0; j < 256; j++)
        {
            skyc[i * 256 + j] = colormap[skyc[i * 256 + j] * numColors / 256];
        }
    }
}

// Based on Textured Floor and Ceiling by DarkOne
void DrawClouds(byte *vbuf, unsigned vbufPitch, int min_wallheight)
{
    // Move clouds
    fixed moveDist = tics * curSky->speed;
    cloudx += FixedMul(moveDist,sintable[curSky->angle]);
    cloudy -= FixedMul(moveDist,costable[curSky->angle]);

    // Draw them
    int y0, halfheight;
    unsigned top_offset0;
    fixed dist;                                // distance to row projection
    fixed tex_step;                            // global step per one screen pixel
    fixed gu, gv, du, dv;                      // global texture coordinates
    int u, v;                                  // local texture coordinates

    // ------ * prepare * --------
    halfheight = viewheight >> 1;
    y0 = min_wallheight >> 3;                  // starting y value
    if(y0 > halfheight)
        return;                                // view obscured by walls
    if(!y0) y0 = 1;                            // don't let division by zero
    top_offset0 = vbufPitch * (halfheight - y0 - 1);

    // draw horizontal lines
    for(int y = y0, top_offset = top_offset0; y < halfheight; y++, top_offset -= vbufPitch)
    {
        dist = (heightnumerator / y) << 8;
        gu =  viewx + FixedMul(dist, viewcos) + cloudx;
        gv = -viewy + FixedMul(dist, viewsin) + cloudy;
        tex_step = (dist << 8) / viewwidth / 175;
        du =  FixedMul(tex_step, viewsin);
        dv = -FixedMul(tex_step, viewcos);
        gu -= (viewwidth >> 1)*du;
        gv -= (viewwidth >> 1)*dv;          // starting point (leftmost)
        for(int x = 0, top_add = top_offset; x < viewwidth; x++, top_add++)
        {
            if(wallheight[x] >> 3 <= y)
            {
                u = (gu >> 13) & 255;
                v = (gv >> 13) & 255;
                vbuf[top_add] = skyc[((255 - u) << 8) + 255 - v];
            }
            gu += du;
            gv += dv;
        }
    }
}

#endif
