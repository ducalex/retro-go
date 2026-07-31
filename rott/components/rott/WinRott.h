#ifndef WINROTT_H
#define WINROTT_H

// central resolution globals
extern int iGLOBAL_SCREENWIDTH;
extern int iGLOBAL_SCREENHEIGHT;
extern int iGLOBAL_SCREENBWIDE;
extern int iG_SCREENWIDTH;
extern int iG_SCREENHEIGHT;

extern int iGLOBAL_HEALTH_X;
extern int iGLOBAL_HEALTH_Y;
extern int iGLOBAL_AMMO_X;
extern int iGLOBAL_AMMO_Y;

extern int iGLOBAL_FOCALWIDTH;
extern double dGLOBAL_FPFOCALWIDTH;

extern double dTopYZANGLELIMIT;
extern int iG_X_center;
extern int iG_Y_center;
extern int viewheight;
extern int viewwidth;

void SetRottScreenRes (int Width, int Height);

#endif
