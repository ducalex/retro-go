#include "rt_def.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "modexlib.h"

// Setting resolution globals to 320x240 for Retro-Go port
int iGLOBAL_SCREENWIDTH  = 320;
int iGLOBAL_SCREENHEIGHT = 200;
int iGLOBAL_SCREENBWIDE  = 80;
int iG_SCREENWIDTH       = 80;
int iG_SCREENHEIGHT      = 200;

int iGLOBAL_HEALTH_X = 20;
int iGLOBAL_HEALTH_Y = 185; 
int iGLOBAL_AMMO_X   = 300;
int iGLOBAL_AMMO_Y   = 184; 

int iGLOBAL_FOCALWIDTH = 160;
double dGLOBAL_FPFOCALWIDTH = 160.0;

double dTopYZANGLELIMIT;

int iG_X_center;
int iG_Y_center;

int iG_playerTilt = 0;

boolean iG_aimCross = 0;

extern int  viewheight;
extern int  viewwidth;

//----------------------------------------------------------------------
#define FINEANGLES                        2048
void SetRottScreenRes (int Width, int Height) 
{
    // Force native resolution for Retro-Go hardware
    printf("ROTT: Setting resolution to 320x200\n");
    iGLOBAL_SCREENWIDTH = 320;
    iGLOBAL_SCREENHEIGHT = 200;
    iGLOBAL_SCREENBWIDE = 80;
      iG_SCREENWIDTH = 80;

      iGLOBAL_FOCALWIDTH = 160;
      dGLOBAL_FPFOCALWIDTH = 160.0;
      iGLOBAL_HEALTH_X = 20;
      iGLOBAL_HEALTH_Y = 185;
      iGLOBAL_AMMO_Y = 184;

      dTopYZANGLELIMIT = (44.0*FINEANGLES/360.0);
}
// RESTORE SCREEN SHAKE ROUTINES
extern int viewsize;
void MoveScreenUpLeft()
{
    int startX,startY,startoffset;
    byte *Ycnt,*b;
    b=(byte *)bufferofs;
    b += (((iGLOBAL_SCREENHEIGHT-viewheight)/2)*iGLOBAL_SCREENWIDTH)+((iGLOBAL_SCREENWIDTH-viewwidth)/2);
    if (viewsize == 8) {b += 8*iGLOBAL_SCREENWIDTH;}
    startX = 3;
    startY = 3;
    startoffset = (startY*iGLOBAL_SCREENWIDTH)+startX;
    for (Ycnt=b;Ycnt<b+((viewheight-startY)*iGLOBAL_SCREENWIDTH);Ycnt+=iGLOBAL_SCREENWIDTH){
        memcpy(Ycnt,Ycnt+startoffset, viewwidth-startX);
    }
}

void MoveScreenDownLeft()
{
    int startX,startY,startoffset;
    byte *Ycnt,*b;
    b=(byte *)bufferofs;
    b += (((iGLOBAL_SCREENHEIGHT-viewheight)/2)*iGLOBAL_SCREENWIDTH)+((iGLOBAL_SCREENWIDTH-viewwidth)/2);
    if (viewsize == 8) {b += 8*iGLOBAL_SCREENWIDTH;}
    startX = 3;
    startY = 3;
    startoffset = (startY*iGLOBAL_SCREENWIDTH);
    for (Ycnt=b+((viewheight-startY-1)*iGLOBAL_SCREENWIDTH);Ycnt>b;Ycnt-=iGLOBAL_SCREENWIDTH){
        memcpy(Ycnt+startoffset,Ycnt+startX,viewwidth-startX);
    }
}

void MoveScreenUpRight()
{
    int startX,startY,startoffset;
    byte *Ycnt,*b;
    b=(byte *)bufferofs;
    b += (((iGLOBAL_SCREENHEIGHT-viewheight)/2)*iGLOBAL_SCREENWIDTH)+((iGLOBAL_SCREENWIDTH-viewwidth)/2);
    if (viewsize == 8) {b += 8*iGLOBAL_SCREENWIDTH;}
    startX = 3;
    startY = 3;
    startoffset = (startY*iGLOBAL_SCREENWIDTH);
    for (Ycnt=b;Ycnt<b+((viewheight-startY)*iGLOBAL_SCREENWIDTH);Ycnt+=iGLOBAL_SCREENWIDTH){
        memcpy(Ycnt+startX,Ycnt+startoffset, viewwidth-startX);
    }
}

void MoveScreenDownRight()
{
    int startX,startY,startoffset;
    byte *Ycnt,*b;
    b=(byte *)bufferofs;
    b += (((iGLOBAL_SCREENHEIGHT-viewheight)/2)*iGLOBAL_SCREENWIDTH)+((iGLOBAL_SCREENWIDTH-viewwidth)/2);
    if (viewsize == 8) {b += 8*iGLOBAL_SCREENWIDTH;}
    startX = 3;
    startY = 3;
    startoffset = (startY*iGLOBAL_SCREENWIDTH)+startX;
    for (Ycnt=b+((viewheight-startY-1)*iGLOBAL_SCREENWIDTH);Ycnt>b;Ycnt-=iGLOBAL_SCREENWIDTH){
        memcpy(Ycnt+startoffset,Ycnt,viewwidth-startX);
    }
}
