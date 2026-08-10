#include "version.h"

#ifdef USE_DIR3DSPR
#include "wl_def.h"
#include "wl_shade.h"

// Define directional 3d sprites in wl_act1.cpp (there are two examples)
// Make sure you have according entries in ScanInfoPlane in wl_game.cpp.


void Scale3DShaper(int x1, int x2, int shapenum, uint32_t flags, fixed ny1, fixed ny2,
                   fixed nx1, fixed nx2, byte *vbuf, unsigned vbufPitch)
{
    t_compshape *shape;
    unsigned scale1,starty,endy;
    word *cmdptr;
    byte *line;
    byte *vmem;
    int dx,len,i,newstart,ycnt,pixheight,screndy,upperedge,scrstarty;
    unsigned j;
    fixed height,dheight,height1,height2;
    int xpos[TEXTURESIZE+1];
    int slinex;
    fixed dxx=(ny2-ny1)<<8,dzz=(nx2-nx1)<<8;
    fixed dxa=0,dza=0;
    byte col;

    shape = (t_compshape *) PM_GetSprite(shapenum);

    len=shape->rightpix-shape->leftpix+1;
    if(!len) return;

    ny1+=dxx>>9;
    nx1+=dzz>>9;

    dxa=-(dxx>>1),dza=-(dzz>>1);
    dxx>>=TEXTURESHIFT,dzz>>=TEXTURESHIFT;
    dxa+=shape->leftpix*dxx,dza+=shape->leftpix*dzz;

    xpos[0]=(int)((ny1+(dxa>>8))*scale/(nx1+(dza>>8))+centerx);
    height1 = heightnumerator/((nx1+(dza>>8))>>8);
    height=(((fixed)height1)<<12)+2048;

    for(i=1;i<=len;i++)
    {
        dxa+=dxx,dza+=dzz;
        xpos[i]=(int)((ny1+(dxa>>8))*scale/(nx1+(dza>>8))+centerx);
        if(xpos[i-1]>viewwidth) break;
    }
    len=i-1;
    dx = xpos[len] - xpos[0];
    if(!dx) return;

    height2 = heightnumerator/((nx1+(dza>>8))>>8);
    dheight=(((fixed)height2-(fixed)height1)<<12)/(fixed)dx;

    cmdptr = (word *) shape->dataofs;

    i=0;
    if(x2>viewwidth) x2=viewwidth;

    for(i=0;i<len;i++)
    {
        for(slinex=xpos[i];slinex<xpos[i+1] && slinex<x2;slinex++)
        {
            height+=dheight;
            if(slinex<0) continue;

            scale1=(unsigned)(height>>15);

            if(wallheight[slinex]<(height>>12) && scale1 /*&& scale1<=maxscale*/)
            {
#ifdef USE_SHADING
                byte *curshades;
                if(flags & FL_FULLBRIGHT)
                    curshades = shadetable[0];
                else
                    curshades = shadetable[GetShade(scale1<<3)];
#endif

                pixheight=scale1*SPRITESCALEFACTOR;
                upperedge=viewheight/2-scale1;

                line=(byte *)shape + cmdptr[i];

                while((endy = READWORD(line)) != 0)
                {
                    endy >>= 1;
                    newstart = READWORD(line);
                    starty = READWORD(line) >> 1;
                    j=starty;
                    ycnt=j*pixheight;
                    screndy=(ycnt>>6)+upperedge;
                    if(screndy<0) vmem=vbuf+slinex;
                    else vmem=vbuf+screndy*vbufPitch+slinex;
                    for(;j<endy;j++)
                    {
                        scrstarty=screndy;
                        ycnt+=pixheight;
                        screndy=(ycnt>>6)+upperedge;
                        if(scrstarty!=screndy && screndy>0)
                        {
#ifdef USE_SHADING
                            col=curshades[((byte *)shape)[newstart+j]];
#else
                            col=((byte *)shape)[newstart+j];
#endif
                            if(scrstarty<0) scrstarty=0;
                            if(screndy>viewheight) screndy=viewheight,j=endy;

                            while(scrstarty<screndy)
                            {
                                *vmem=col;
                                vmem+=vbufPitch;
                                scrstarty++;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Scale3DShape(byte *vbuf, unsigned vbufPitch, statobj_t *ob)
{
    fixed nx1,nx2,ny1,ny2;
    int viewx1,viewx2;
    fixed diradd;
    fixed playx = viewx;
    fixed playy = viewy;

    //
    // the following values for "diradd" aren't optimized yet
    // if you have problems with sprites being visible through wall edges
    // where they shouldn't, you can try to adjust these values and SIZEADD
    //

#define SIZEADD 1024

    switch(ob->flags & FL_DIR_POS_MASK)
    {
        case FL_DIR_POS_FW: diradd=0x7ff0+0x8000; break;
        case FL_DIR_POS_BW: diradd=-0x7ff0+0x8000; break;
        case FL_DIR_POS_MID: diradd=0x8000; break;
        default:
            Quit("Unknown directional 3d sprite position (shapenum = %i)", ob->shapenum);
    }

    if(ob->flags & FL_DIR_VERT_FLAG)     // vertical dir 3d sprite
    {
        fixed gy1,gy2,gx,gyt1,gyt2,gxt;
        //
        // translate point to view centered coordinates
        //
        gy1 = (((long)ob->tiley) << TILESHIFT)+0x8000-playy-0x8000L-SIZEADD;
        gy2 = gy1+0x10000L+2*SIZEADD;
        gx = (((long)ob->tilex) << TILESHIFT)+diradd-playx;

        //
        // calculate newx
        //
        gxt = FixedMul(gx,viewcos);
        gyt1 = FixedMul(gy1,viewsin);
        gyt2 = FixedMul(gy2,viewsin);
        nx1 = gxt-gyt1;
        nx2 = gxt-gyt2;

        //
        // calculate newy
        //
        gxt = FixedMul(gx,viewsin);
        gyt1 = FixedMul(gy1,viewcos);
        gyt2 = FixedMul(gy2,viewcos);
        ny1 = gyt1+gxt;
        ny2 = gyt2+gxt;
    }
    else                                    // horizontal dir 3d sprite
    {
        fixed gx1,gx2,gy,gxt1,gxt2,gyt;
        //
        // translate point to view centered coordinates
        //
        gx1 = (((long)ob->tilex) << TILESHIFT)+0x8000-playx-0x8000L-SIZEADD;
        gx2 = gx1+0x10000L+2*SIZEADD;
        gy = (((long)ob->tiley) << TILESHIFT)+diradd-playy;

        //
        // calculate newx
        //
        gxt1 = FixedMul(gx1,viewcos);
        gxt2 = FixedMul(gx2,viewcos);
        gyt = FixedMul(gy,viewsin);
        nx1 = gxt1-gyt;
        nx2 = gxt2-gyt;

        //
        // calculate newy
        //
        gxt1 = FixedMul(gx1,viewsin);
        gxt2 = FixedMul(gx2,viewsin);
        gyt = FixedMul(gy,viewcos);
        ny1 = gyt+gxt1;
        ny2 = gyt+gxt2;
    }

    if(nx1 < 0 || nx2 < 0) return;      // TODO: Clip on viewplane

    //
    // calculate perspective ratio
    //
    if(nx1>=0 && nx1<=1792) nx1=1792;
    if(nx1<0 && nx1>=-1792) nx1=-1792;
    if(nx2>=0 && nx2<=1792) nx2=1792;
    if(nx2<0 && nx2>=-1792) nx2=-1792;

    viewx1=(int)(centerx+ny1*scale/nx1);
    viewx2=(int)(centerx+ny2*scale/nx2);

    if(viewx2 < viewx1)
    {
        Scale3DShaper(viewx2,viewx1,ob->shapenum,ob->flags,ny2,ny1,nx2,nx1,vbuf,vbufPitch);
    }
    else
    {
        Scale3DShaper(viewx1,viewx2,ob->shapenum,ob->flags,ny1,ny2,nx1,nx2,vbuf,vbufPitch);
    }
}

#endif
