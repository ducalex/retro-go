#define LCD_SCREEN_BUFFER 1
#define LCD_BUFFER_LENGTH (RG_SCREEN_WIDTH * 4) // In pixels

#include <SDL2/SDL.h>

static SDL_Window *window;
static SDL_Surface *surface, *canvas;
static uint16_t *screen_buffer;
static static int screen_width = RG_SCREEN_WIDTH;
static static int screen_height = RG_SCREEN_HEIGHT;

static void lcd_init(void)
{
    window = SDL_CreateWindow("Retro-Go", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, 0);
    surface = SDL_GetWindowSurface(window);
    canvas = SDL_CreateRGBSurfaceWithFormat(0, screen_width, screen_height, 16, SDL_PIXELFORMAT_RGB565);
    screen_buffer = rg_alloc(screen_width * screen_height * 2, MEM_ANY);
}

static void lcd_deinit(void)
{
}

static void lcd_set_backlight(float percent)
{
}

static inline uint16_t *lcd_get_buffer_ptr(int left, int top)
{
    return screen_buffer + (top * display.screen.real_width) + left;
}

static void lcd_sync(void)
{
/*
    // FIXME: Copy screen_buffer to canvas
    int bpp = canvas->format->BytesPerPixel;
    int pitch = canvas->pitch;
    void *pixels = canvas->pixels;
    for (size_t i = 0; i < length; ++i) {
        int real_top = win_top + (cursor / win_width);
        int real_left = win_left + (cursor % win_width);
        if (real_top >= screen_height || real_left >= screen_width)
            return;
        uint16_t *dst = (void*)pixels + (real_top * pitch) + (real_left * bpp);
        uint16_t pixel = buffer[i];
        *dst = ((pixel & 0xFF) << 8) | ((pixel & 0xFF00) >> 8);;
        cursor++;
    }
 */
    SDL_BlitSurface(canvas, NULL, surface, NULL);
    SDL_UpdateWindowSurface(window);
}

const rg_display_driver_t rg_display_driver_sdl2 = {
    .name = "sdl2",
};
