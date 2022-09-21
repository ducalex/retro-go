//
//  tiles.c
//  Duke3D
//
//  Created by fabien sanglard on 12-12-22.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#include "tiles.h"
#include "engine.h"
#include "draw.h"
#include "filesystem.h"

#include "esp_attr.h"

char  artfilename[20];

tile_t *tiles;//[MAXTILES];

int32_t numTiles;

int32_t artversion;

uint8_t  *pic = NULL;

EXT_RAM_ATTR uint8_t  gotpic[(MAXTILES+7)>>3];

void setviewtotile(short tilenume, int32_t tileWidth, int32_t tileHeight)
{
    int32_t i, j;

    /* DRAWROOMS TO TILE BACKUP&SET CODE */
    tiles[tilenume].dim.width = tileWidth;
    tiles[tilenume].dim.height = tileHeight;
    bakxsiz[setviewcnt] = tileWidth;
    bakysiz[setviewcnt] = tileHeight;
    bakvidoption[setviewcnt] = vidoption;
    vidoption = 2;
    bakframeplace[setviewcnt] = frameplace;
    frameplace = tiles[tilenume].data;
    bakwindowx1[setviewcnt] = windowx1;
    bakwindowy1[setviewcnt] = windowy1;
    bakwindowx2[setviewcnt] = windowx2;
    bakwindowy2[setviewcnt] = windowy2;
    copybufbyte(&startumost[windowx1],&bakumost[windowx1],(windowx2-windowx1+1)*sizeof(bakumost[0]));
    copybufbyte(&startdmost[windowx1],&bakdmost[windowx1],(windowx2-windowx1+1)*sizeof(bakdmost[0]));
    setview(0,0,tileHeight-1,tileWidth-1);
    setaspect(65536,65536);
    j = 0;
    for(i=0; i<=tileWidth; i++) {
        ylookup[i] = j;
        j += tileWidth;
    }
    setBytesPerLine(tileHeight);
    setviewcnt++;
}




void squarerotatetile(short tilenume)
{
    int32_t i, j, k;
    uint8_t  *ptr1, *ptr2;

    dimensions_t tileDim;

    tileDim.width = tiles[tilenume].dim.width;
    tileDim.height = tiles[tilenume].dim.height;

    /* supports square tiles only for rotation part */
    if (tileDim.width == tileDim.height)
    {
        k = (tileDim.width<<1);
        for(i=tileDim.width-1; i>=0; i--)
        {
            ptr1 = tiles[tilenume].data+i*(tileDim.width+1);
            ptr2 = ptr1;
            if ((i&1) != 0) {
                ptr1--;
                ptr2 -= tileDim.width;
                swapchar(ptr1,ptr2);
            }
            for(j=(i>>1)-1; j>=0; j--)
            {
                ptr1 -= 2;
                ptr2 -= k;
                swapchar2(ptr1,ptr2,tileDim.width);
            }
        }
    }
}



//1. Lock a picture in the cache system.
//2. Mark it as used in the bitvector tracker.
IRAM_ATTR void setgotpic(int32_t tilenume)
{
    if (tiles[tilenume].lock < 200)
        tiles[tilenume].lock = 199;

    gotpic[tilenume>>3] |= pow2char[tilenume&7];
}





void loadtile(short tilenume)
{
    uint8_t  *ptr;
    int32_t i, tileFilesize;




    if ((uint32_t)tilenume >= (uint32_t)MAXTILES)
        return;

    tileFilesize = tiles[tilenume].dim.width * tiles[tilenume].dim.height;

    if (tileFilesize <= 0)
        return;

    i = tilefilenum[tilenume];
    if (i != artfilnum){
        if (artfil != -1)
            kclose(artfil);
        artfilnum = i;
        artfilplc = 0L;

        artfilename[7] = (i%10)+48;
        artfilename[6] = ((i/10)%10)+48;
        artfilename[5] = ((i/100)%10)+48;
        artfil = TCkopen4load(artfilename,0);

        if (artfil == -1){
            printf("Error, unable to load artfile:'%s'.\n",artfilename);
            getchar();
            exit(0);
        }

        faketimerhandler();
    }

    if (tiles[tilenume].data == NULL){
        tiles[tilenume].lock = 199;
        allocache(&tiles[tilenume].data,tileFilesize,(uint8_t  *) &tiles[tilenume].lock);
    }

    if (artfilplc != tilefileoffs[tilenume])
    {
        klseek(artfil,tilefileoffs[tilenume]-artfilplc,SEEK_CUR);
        faketimerhandler();
    }
    ptr = tiles[tilenume].data;

    kread(artfil,ptr,tileFilesize);
    faketimerhandler();
    artfilplc = tilefileoffs[tilenume]+tileFilesize;
}



uint8_t* allocatepermanenttile(short tilenume, int32_t width, int32_t height)
{
    int32_t j;
    uint32_t tileDataSize;

    //Check dimensions are correct.
    if ((width <= 0) || (height <= 0) || ((uint32_t)tilenume >= (uint32_t)MAXTILES))
        return(0);

    tileDataSize = width * height;

    tiles[tilenume].lock = 255;
    allocache(&tiles[tilenume].data,tileDataSize,(uint8_t  *) &tiles[tilenume].lock);

    tiles[tilenume].dim.width = width;
    tiles[tilenume].dim.height = height;
    tiles[tilenume].animFlags = 0;

    j = 15;
    while ((j > 1) && (pow2long[j] > width))
        j--;
    picsiz[tilenume] = ((uint8_t )j);

    j = 15;
    while ((j > 1) && (pow2long[j] > height))
        j--;
    picsiz[tilenume] += ((uint8_t )(j<<4));

    return(tiles[tilenume].data);
}



int loadpics(char  *filename, char * gamedir)

{
    int32_t offscount, localtilestart, localtileend, dasiz;
    short fil, i, j, k;


    strcpy(artfilename,filename);

    for(i=0; i<MAXTILES; i++)
    {
        tiles[i].dim.width = 0;
        tiles[i].dim.height = 0;
        tiles[i].animFlags = 0L;
    }

    artsize = 0L;

    numtilefiles = 0;
    do
    {
        k = numtilefiles;

        artfilename[7] = (k%10)+48;
        artfilename[6] = ((k/10)%10)+48;
        artfilename[5] = ((k/100)%10)+48;



        if ((fil = TCkopen4load(artfilename,0)) != -1)
        {
            kread32(fil,&artversion);
            if (artversion != 1) return(-1);

            kread32(fil,&numTiles);
            kread32(fil,&localtilestart);
            kread32(fil,&localtileend);

            /*kread(fil,&tilesizx[localtilestart],(localtileend-localtilestart+1)<<1);*/
            for (i = localtilestart; i <= localtileend; i++)
                kread16(fil,&tiles[i].dim.width);

            /*kread(fil,&tilesizy[localtilestart],(localtileend-localtilestart+1)<<1);*/
            for (i = localtilestart; i <= localtileend; i++)
                kread16(fil,&tiles[i].dim.height);

            /*kread(fil,&picanm[localtilestart],(localtileend-localtilestart+1)<<2);*/
            for (i = localtilestart; i <= localtileend; i++)
                kread32(fil,&tiles[i].animFlags);

            offscount = 4+4+4+4+((localtileend-localtilestart+1)<<3);
            for(i=localtilestart; i<=localtileend; i++)
            {
                tilefilenum[i] = k;
                tilefileoffs[i] = offscount;
                dasiz = tiles[i].dim.width*tiles[i].dim.height;
                offscount += dasiz;
                artsize += ((dasiz+15)&0xfffffff0);
            }
            kclose(fil);

            numtilefiles++;

        }
    }
    while (k != numtilefiles);

    printf("Art files loaded\n");

    clearbuf(gotpic,(MAXTILES+31)>>5,0L);

    cachesize = max(artsize,1048576);
    while ((pic = (uint8_t  *)kkmalloc(cachesize)) == NULL)
    {
        cachesize -= 65536L;
        if (cachesize < 65536) return(-1);
    }
    initcache(pic,cachesize);

    for(i=0; i<MAXTILES; i++)
    {
        j = 15;
        while ((j > 1) && (pow2long[j] > tiles[i].dim.width))
            j--;

        picsiz[i] = ((uint8_t )j);
        j = 15;

        while ((j > 1) && (pow2long[j] > tiles[i].dim.height))
            j--;

        picsiz[i] += ((uint8_t )(j<<4));
    }

    artfil = -1;
    artfilnum = -1;
    artfilplc = 0L;

    return(0);
}


void TILE_MakeAvailable(short picID){
    if (tiles[picID].data == NULL)
        loadtile(picID);

}

void copytilepiece(int32_t tilenume1, int32_t sx1, int32_t sy1, int32_t xsiz, int32_t ysiz,
                   int32_t tilenume2, int32_t sx2, int32_t sy2)
{
    uint8_t  *ptr1, *ptr2, dat;
    int32_t xsiz1, ysiz1, xsiz2, ysiz2, i, j, x1, y1, x2, y2;

    xsiz1 = tiles[tilenume1].dim.width;
    ysiz1 = tiles[tilenume1].dim.height;

    xsiz2 = tiles[tilenume2].dim.width;
    ysiz2 = tiles[tilenume2].dim.height;


    if ((xsiz1 > 0) && (ysiz1 > 0) && (xsiz2 > 0) && (ysiz2 > 0))
    {
        TILE_MakeAvailable(tilenume1);
        TILE_MakeAvailable(tilenume2);

        x1 = sx1;
        for(i=0; i<xsiz; i++)
        {
            y1 = sy1;
            for(j=0; j<ysiz; j++)
            {
                x2 = sx2+i;
                y2 = sy2+j;
                if ((x2 >= 0) && (y2 >= 0) && (x2 < xsiz2) && (y2 < ysiz2))
                {
                    ptr1 = tiles[tilenume1].data + x1*ysiz1 + y1;
                    ptr2 = tiles[tilenume2].data + x2*ysiz2 + y2;
                    dat = *ptr1;


                    if (dat != 255)
                        *ptr2 = *ptr1;
                }

                y1++;
                if (y1 >= ysiz1) y1 = 0;
            }
            x1++;
            if (x1 >= xsiz1) x1 = 0;
        }
    }
}



/*
 FCS:   If a texture is animated, this will return the offset to add to tilenum
 in order to retrieve the texture to display.
 */
int animateoffs(int16_t tilenum)
{
    int32_t i, k, offs;

    offs = 0;

    i = (totalclocklock>>((tiles[tilenum].animFlags>>24)&15));

    if ((tiles[tilenum].animFlags&63) > 0){
        switch(tiles[tilenum].animFlags&192)
        {
            case 64:
                k = (i%((tiles[tilenum].animFlags&63)<<1));
                if (k < (tiles[tilenum].animFlags&63))
                    offs = k;
                else
                    offs = (((tiles[tilenum].animFlags&63)<<1)-k);
                break;
            case 128:
                offs = (i%((tiles[tilenum].animFlags&63)+1));
                break;
            case 192:
                offs = -(i%((tiles[tilenum].animFlags&63)+1));
        }
    }

    return(offs);
}