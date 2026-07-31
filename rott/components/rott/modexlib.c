#include "rt_def.h"
#include "watcom.h"
#include <stdio.h>
#include <string.h>
#include "modexlib.h"
#include <rg_system.h>
#include "rg_display.h"
#include "SDL.h"

int    linewidth;
byte  *page1start = NULL;
byte  *page2start = NULL;
byte  *page3start = NULL;
int    screensize;
byte  *bufferofs;
byte  *displayofs;
boolean graphicsmode=false;
boolean StretchScreen=false;

void GraphicsMode ( void )
{
    if (graphicsmode) return;
    graphicsmode = true;
}

void SetTextMode ( void )
{
    graphicsmode = false;
}

void TextMode ( void )
{
    graphicsmode = false;
}

void WaitVBL( void ) { SDL_Delay (16); }

void VL_SetVGAPlaneMode ( void )
{
    linewidth = 320;
    // Standard linear ylookup for logical 320x200
    for (int i=0; i<600; i++) ylookup[i] = i * linewidth;
    
    // Engine now renders native 200 lines
    screensize = 320 * 200;

    // Allocate 320x200 buffer
    if (!page1start) {
        page1start = rg_alloc(screensize, MEM_SLOW);
        memset(page1start, 0, screensize);
    }
    page2start = page1start;
    page3start = page1start;

    displayofs = page1start;
    bufferofs = page1start;

    // Hard-lock SDL surface to 320x200 to allow Retro-Go to scale it
    SDL_SetVideoMode(320, 200, 8, 0);

    printf("ROTT: Video reset to Native 320x200 Mode\n");
}

void VL_CopyPlanarPage ( byte * src, byte * dest ) { if (src != dest) memcpy(dest,src,64000); }
void VL_CopyPlanarPageToMemory ( byte * src, byte * dest ) { if (src != dest) memcpy(dest,src,64000); }
void VL_CopyDisplayToHidden ( void ) { /* No-op */ }

void VL_ClearBuffer (byte *buf, byte color) { 
    if (page1start) memset(page1start, color, 64000); 
}

void VL_ClearVideo (byte color) {
    SDL_Surface *surface = SDL_GetVideoSurface();
    if (surface && surface->pixels) {
        while (rg_display_is_busy())
            rg_task_yield();
        memset (surface->pixels, color, 64000); 
    }
}

void VH_UpdateScreen (void)
{ 
    SDL_Surface *surface = SDL_GetVideoSurface();
    if (surface && surface->pixels && page1start) {
        // Keep the single internal staging surface stable while Retro-Go owns it.
        while (rg_display_is_busy())
            rg_task_yield();

        // Direct copy to the 320x200 SDL surface.
        // Retro-Go will then scale this to fill the physical screen.
        memcpy(surface->pixels, page1start, 64000);
    }
    SDL_UpdateRect (surface, 0, 0, 0, 0);
}

void XFlipPage ( void ) { 
    VH_UpdateScreen(); 
    // ZERO DRIFT: Always restart drawing at the base of memory
    bufferofs = page1start;
    displayofs = page1start;
}

void EnableScreenStretch(void) { StretchScreen = true; }
void DisableScreenStretch(void) { StretchScreen = false; }

static void StretchMemPicture () { }
void DrawCenterAim () { }
void TurnOffTextCursor ( void ) { }
