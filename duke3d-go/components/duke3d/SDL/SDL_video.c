#include "SDL_video.h"

static rg_video_update_t update;

SDL_Surface* primary_surface;

int SDL_LockSurface(SDL_Surface *surface)
{
    return 0;
}

void SDL_UnlockSurface(SDL_Surface* surface)
{

}

void SDL_UpdateRect(SDL_Surface *screen, Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
    SDL_Flip(screen);
}

SDL_VideoInfo *SDL_GetVideoInfo(void)
{
    SDL_VideoInfo *info = malloc(sizeof(SDL_VideoInfo));
    return info;
}

char *SDL_VideoDriverName(char *namebuf, int maxlen)
{
    return "RETRO-GO";
}


SDL_Rect **SDL_ListModes(SDL_PixelFormat *format, Uint32 flags)
{
    SDL_Rect mode[1] = {{0,0,320,200}};
    return &mode;
}

void SDL_WM_SetCaption(const char *title, const char *icon)
{

}

char *SDL_GetKeyName(SDLKey key)
{
    return (char *)"";
}

SDL_Keymod SDL_GetModState(void)
{
    return (SDL_Keymod)0;
}

IRAM_ATTR Uint32 SDL_GetTicks(void)
{
    return rg_system_timer() / 1000;
}

Uint32 SDL_WasInit(Uint32 flags)
{
	return 0;
}

int SDL_InitSubSystem(Uint32 flags)
{
    if(flags == SDL_INIT_VIDEO)
    {
        SDL_CreateRGBSurface(0, 320, 200, 8, 0,0,0,0);
    }
    if( flags &= SDL_INIT_AUDIO)
    {

    }
    return 0; // 0 = OK, -1 = Error
}

SDL_Surface *SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    SDL_Surface *surface = (SDL_Surface *)calloc(1, sizeof(SDL_Surface));
    SDL_Rect rect = { .x=0, .y=0, .w=width, .h=height};
    SDL_Color col = {.r=0, .g=0, .b=0, .unused=0};
    SDL_Palette pal =  {.ncolors=1, .colors=&col};
    SDL_PixelFormat* pf = (SDL_PixelFormat*)calloc(1, sizeof(SDL_PixelFormat));
	pf->palette = &pal;
	pf->BitsPerPixel = 8;
	pf->BytesPerPixel = 1;
	pf->Rloss = 0; pf->Gloss = 0; pf->Bloss = 0; pf->Aloss = 0,
	pf->Rshift = 0; pf->Gshift = 0; pf->Bshift = 0; pf->Ashift = 0;
	pf->Rmask = 0; pf->Gmask = 0; pf->Bmask = 0; pf->Amask = 0;
	pf->colorkey = 0;
	pf->alpha = 0;

    surface->flags = flags;
    surface->format = pf;
    surface->w = width;
    surface->h = height;
    surface->pitch = width*(depth/8);
    surface->clip_rect = rect;
    surface->refcount = 1;
    surface->pixels = malloc(width*height*1);
    if(primary_surface == NULL)
    	primary_surface = surface;
    return surface;
}

int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
    if(dst)//|| dst->sprite == NULL)
    {
    	if(dstrect != NULL)
    	{
			for(int y = dstrect->y; y < dstrect->y + dstrect->h;y++)
				memset((unsigned char *)dst->pixels + y*320 + dstrect->x, (unsigned char)color, dstrect->w);
    	} else {
    		memset(dst->pixels, (unsigned char)color, dst->pitch*dst->h);
    	}
    }
    return 0;
}

SDL_Surface *SDL_GetVideoSurface(void)
{
    return primary_surface;
}

Uint32 SDL_MapRGB(SDL_PixelFormat *fmt, Uint8 r, Uint8 g, Uint8 b)
{
    if(fmt->BitsPerPixel == 16)
    {
        uint16_t bb = (b >> 3) & 0x1f;
        uint16_t gg = ((g >> 2) & 0x3f) << 5;
        uint16_t rr = ((r >> 3) & 0x1f) << 11;
        return (Uint32) (rr | gg | bb);
    }
    return (Uint32)0;
}

int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors)
{
	for(int i = firstcolor; i < firstcolor+ncolors; i++)
	{
		int v=((colors[i].r>>3)<<11)+((colors[i].g>>2)<<5)+(colors[i].b>>3);
		update.palette[i]=(v>>8)+(v<<8);
	}
	return 1;
}

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
	return SDL_GetVideoSurface();
}

void SDL_FreeSurface(SDL_Surface *surface)
{
    free(surface->pixels);
    free(surface->format);
    surface->refcount = 0;
}

void SDL_QuitSubSystem(Uint32 flags)
{

}

int SDL_Flip(SDL_Surface *screen)
{
    update.buffer = screen->pixels;
    rg_display_queue_update(&update, NULL);
    rg_system_tick(0);
	return 0;
}

int SDL_VideoModeOK(int width, int height, int bpp, Uint32 flags)
{
	if(bpp == 8)
		return 1;
	return 0;
}

SemaphoreHandle_t display_mutex = NULL;

void SDL_LockDisplay()
{
    // if (display_mutex == NULL)
    // {
    //     printf("Creating display mutex.\n");
    //     display_mutex = xSemaphoreCreateMutex();
    //     if (!display_mutex)
    //         abort();
    //     //xSemaphoreGive(display_mutex);
    // }

    // if (!xSemaphoreTake(display_mutex, 60000 / portTICK_RATE_MS))
    // {
    //     printf("Timeout waiting for display lock.\n");
    //     abort();
    // }
    //printf("L");
    //taskYIELD();
}

void SDL_UnlockDisplay()
{
    // if (!display_mutex)
    //     abort();
    // if (!xSemaphoreGive(display_mutex))
    //     abort();

    //printf("U ");
    //taskYIELD();
}
