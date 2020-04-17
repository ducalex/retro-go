/*****************************************/
/* SDL Graphics Engine                   */
/* Released under the GPL license        */
/*                                       */
/* Original Author:                      */
/*		Zerograd? - Hu-GO!               */
/* Redesignd for HuExpress by:           */
/*		Alexander von Gluck, kallisti5   */
/*****************************************/

#include "osd_sdl_gfx.h"
#include "utils.h"

#if 0

//! Host machine rendered screen
SDL_Renderer *sdlRenderer = NULL;
SDL_Window *sdlWindow = NULL;
SDL_GLContext sdlGLContext;

//! PC Engine rendered screen
SDL_Surface *screen = NULL;

int blit_x, blit_y;
// where must we blit the screen buffer on screen

uchar *XBuf;
// buffer for video flipping

osd_gfx_driver osd_gfx_driver_list[3] = {
	{osd_gfx_init, osd_gfx_init_normal_mode,
	 osd_gfx_put_image_normal, osd_gfx_shut_normal_mode},
	{osd_gfx_init, osd_gfx_init_normal_mode,
	 osd_gfx_put_image_normal, osd_gfx_shut_normal_mode},
	{osd_gfx_init, osd_gfx_init_normal_mode,
	 osd_gfx_put_image_normal, osd_gfx_shut_normal_mode}
};


void
osd_gfx_dummy_func(void)
{
	return;
}


static void
Slock(SDL_Surface * screen)
{
	if (SDL_MUSTLOCK(screen)) {
		if (SDL_LockSurface(screen) < 0) {
			return;
		}
	}
}


static void
Sulock(SDL_Surface * screen)
{
	if (SDL_MUSTLOCK(screen)) {
		SDL_UnlockSurface(screen);
	}
}


/*****************************************************************************

		Function: osd_gfx_put_image_normal

		Description: draw the raw computed picture to screen, without any effect
		trying to center it (I bet there is still some work on this, maybe not
														in this function)
		Parameters: none
		Return: nothing

*****************************************************************************/
void
osd_gfx_put_image_normal(void)
{
	uint32 sdlFlags = SDL_GetWindowFlags(sdlWindow);

	Slock(screen);
	dump_rgb_frame(screen->pixels);
	Sulock(screen);

	osd_gfx_blit();
}


/*****************************************************************************

		Function: osd_gfx_set_message

		Description: compute the message that will be displayed to create a sprite
			to blit on screen
		Parameters: char* mess, the message to display
		Return: nothing but set OSD_MESSAGE_SPR

*****************************************************************************/
void
osd_gfx_set_message(char *mess)
{
	
}


/*
 * osd_gfx_init:
 * One time initialization of the main output screen
 */
int
osd_gfx_init(void)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO)) {
			MESSAGE_ERROR("SDL: %s failed at %s:%d - %s\n",
				__func__, __FILE__, __LINE__, SDL_GetError());
			return 0;
		}
	}

	MESSAGE_INFO("Window Size: %d\n", option.window_size);
	MESSAGE_INFO("Fullscreen: %d\n", option.want_fullscreen);
	osd_gfx_init_normal_mode();

	if (option.want_fullscreen)
		SDL_ShowCursor(SDL_DISABLE);

	SetPalette();

	return 1;
}


/*****************************************************************************

		Function:	osd_gfx_init_normal_mode

		Description: initialize the classic 256*224 video mode for normal video_driver
		Parameters: none
		Return: 0 on error
						1 on success

*****************************************************************************/
int
osd_gfx_init_normal_mode()
{
	struct generic_rect rect;

#if ENABLE_TRACING_SDL
	printf("GFX: Mode change: %dx%d\n", io.screen_w, io.screen_h);
#endif

	if (sdlWindow == NULL) {
		// First start? Lets just set a fake screen w/h
		io.screen_w = 352;
		io.screen_h = 256;
	}

	if (io.screen_w < 160 || io.screen_w > 512) {
		MESSAGE_ERROR("Correcting out of range screen w %d\n", io.screen_w);
		io.screen_w = 256;
	}
	if (io.screen_h < 160 || io.screen_h > 256) {
		MESSAGE_ERROR("Correcting out of range screen h %d\n", io.screen_h);
		io.screen_h = 224;
	}


	struct generic_rect viewport;
	viewport.start_x = 0;
	viewport.start_y = 0;

	if (option.want_fullscreen) {
		SDL_DisplayMode current;
    	if (SDL_GetCurrentDisplayMode(0, &current)) {
			MESSAGE_ERROR("Unable to determine current DisplayMode");
		}

		calc_fullscreen_aspect(current.w, current.h, &viewport,
			io.screen_w, io.screen_h);

		MESSAGE_INFO("%dx%d , %dx%d\n", viewport.start_x, viewport.start_y,
			viewport.end_x, viewport.end_y);
	} else {
		
		viewport.end_x = io.screen_w * option.window_size;
		viewport.end_y = io.screen_h * option.window_size;
	}

	uint16 viewportWidth = viewport.end_x - viewport.start_x;
	uint16 viewportHeight = viewport.end_y - viewport.start_y;

	if (sdlWindow != NULL) {
		SDL_DestroyWindow(sdlWindow);
		sdlWindow = NULL;
	}

	uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

	if (option.want_fullscreen) 
		windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	sdlWindow = SDL_CreateWindow("HuExpress", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, viewport.end_x, viewport.end_y, windowFlags);

	if (sdlWindow == NULL) {
		MESSAGE_ERROR("SDL: %s failed at %s:%d - %s\n", __func__, __FILE__,
			__LINE__, SDL_GetError());
		return 0;
	}

	sdlGLContext = SDL_GL_CreateContext(sdlWindow);

	if (sdlGLContext == NULL) {
		MESSAGE_ERROR("SDL: %s failed at %s:%d - %s\n", __func__, __FILE__,
			__LINE__, SDL_GetError());
		return 0;
	}

	SDL_GL_MakeCurrent(sdlWindow, sdlGLContext);

	sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
	//sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_SOFTWARE);

	if (sdlRenderer == NULL) {
		MESSAGE_ERROR("SDL: %s failed at %s:%d - %s\n", __func__, __FILE__,
			__LINE__, SDL_GetError());
		return 0;
	}

	SDL_GL_SetSwapInterval(1);

	if (screen != NULL) {
		SDL_FreeSurface(screen);
		screen = NULL;
	}

	//screen = SDL_CreateRGBSurface(0, io.screen_w, io.screen_h,
	//	32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
	screen = SDL_CreateRGBSurface(0, viewportWidth, viewportHeight,
		32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);

	if (screen == NULL) {
		MESSAGE_ERROR("SDL: CreateRGBSurface failed at %s:%d - %s\n",
			__FILE__, __LINE__, SDL_GetError());
	}

	osd_gfx_glinit(&viewport);

	return (screen) ? 1 : 0;
}


//! Delete the window
void
osd_gfx_shut_normal_mode(void)
{
	SDL_FreeSurface(screen);
	screen = NULL;

	if (sdlWindow != NULL)
		SDL_DestroyWindow(sdlWindow);

	/* SDL will free physical_screen internally */
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


/*****************************************************************************

		Function: osd_gfx_savepict

		Description: save a picture in the current directory
		Parameters: none
		Return: the numeric part of the created filename, 0xFFFF meaning that no more
			names were available

*****************************************************************************/
uint16
osd_gfx_savepict()
{
	short unsigned tmp = 0;
	char filename[PATH_MAX];
	char filename_base[PATH_MAX];
	char *frame_buffer;
	FILE *output_file;
	time_t current_time;

	time(&current_time);

	if (!strftime
		(filename_base, PATH_MAX, "%%s/screenshot_%F_%R-%%d.ppm",
			localtime(&current_time)))
		return 0xFFFF;

	do {
		snprintf(filename, PATH_MAX, filename_base, video_path, tmp);
	} while (file_exists(filename) && ++tmp < 0xFFFF);

	frame_buffer =
		malloc(3 * (io.screen_w & 0xFFFE) * (io.screen_h & 0xFFFE));

	if (frame_buffer == NULL)
		return 0xFFFF;

	dump_rgb_frame(frame_buffer);

	output_file = fopen(filename, "wb");
	if (output_file != NULL) {
		char buf[100];

		snprintf(buf, sizeof(buf), "P6\n%d %d\n%d\n", io.screen_w & 0xFFFE,
			io.screen_h & 0xFFFE, 255);
		fwrite(buf, strlen(buf), 1, output_file);
		fwrite(frame_buffer, 3 * (io.screen_w & 0xFFFE)
			* (io.screen_h & 0xFFFE), 1, output_file);
		fclose(output_file);
	}

	return tmp;
}


/*****************************************************************************

		Function: osd_gfx_set_color

		Description: Change the component of the choosen color
		Parameters: uchar index : index of the color to change
					uchar r	: new red component of the color
								uchar g : new green component of the color
								uchar b : new blue component of the color
		Return:

*****************************************************************************/
void
osd_gfx_set_color(uchar index, uchar r, uchar g, uchar b)
{
	SDL_Color R;

	r <<= 2;
	g <<= 2;
	b <<= 2;

	R.r = r;
	R.g = g;
	R.b = b;

	SDL_SetPaletteColors(screen->format->palette,
		&R, 0, 1);
}


void
osd_gfx_glinit(struct generic_rect* viewport)
{
	glEnable( GL_TEXTURE_2D );

	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, option.linear_filter ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glViewport(0, 0, viewport->end_x, viewport->end_y);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
#if !defined(__APPLE__)
	glDisable(GL_TEXTURE_3D_EXT);
#endif
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/*if (conf->video_mask_overscan) {
	glOrtho(0.0, (GLdouble)screen->w,
		(GLdouble)screen->h - (OVERSCAN_BOTTOM * scalefactor),
		(GLdouble)(OVERSCAN_TOP * scalefactor), -1.0, 1.0);
	}
	else {*/
	glOrtho(0.0, viewport->end_x, viewport->end_y, 0.0, -1.0, 1.0);
	//}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void
osd_gfx_blit()
{
	Slock(screen);
	// Edit the texture object's image data	using the information SDL_Surface gives us
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		io.screen_w, io.screen_h, 0, GL_RGB, GL_UNSIGNED_BYTE,
		screen->pixels);
	Sulock(screen);

	int X = 0;
	int Y = 0;
	int windowWidth = screen->w;
	int windowHeight = screen->h;

	glBegin(GL_QUADS) ;
		glTexCoord2f(0, 0);
		glVertex3f(X, Y, 0);
		glTexCoord2f(1, 0);
		glVertex3f(X + windowWidth, Y, 0);
		glTexCoord2f(1, 1);
		glVertex3f(X + windowWidth, Y + windowHeight, 0);
		glTexCoord2f(0, 1);
		glVertex3f(X, Y + windowHeight, 0);
	glEnd();

	SDL_GL_SwapWindow(sdlWindow);
}


int
ToggleFullScreen(void)
{
	// TODO: Fix FullScreen
	return 1;

	struct generic_rect rect;
	uint32 sdlFlags = SDL_GetWindowFlags(sdlWindow);

	SDL_PauseAudio(SDL_ENABLE);

	SetPalette();

	calc_fullscreen_aspect(screen->w, screen->h, &rect,
		io.screen_w, io.screen_h);

	SDL_PauseAudio(SDL_DISABLE);

	return (sdlFlags & SDL_WINDOW_FULLSCREEN) ? 0 : 1;
}


/* drawVolume */
/* Given a string, and a value between 0 and 255, */
/* draw a 'volume' style bar */
void
drawVolume(char *name, int volume)
{
	int i;
	char volumeTemplate[] = "[---------------]";
	char result[255];

	// Load result with the bar title
	strcat(result, name);

	// 17 rounds easily into 255 and gives us 15 bars
	for (i = 1; i < 16; i++) {
		if ((i * 17) <= volume)
			volumeTemplate[i] = '|';
	}

	strcat(result, volumeTemplate);

	osd_gfx_set_message(result);
}

#else

extern void update_display_task(int width);

uint startTime;
uint stopTime;
uint totalElapsedTime;
int my_frame;
#ifdef MY_GFX_AS_TASK
extern QueueHandle_t vidQueue;
#endif
extern uint8_t* framebuffer[2];
uint8_t current_framebuffer = 0;
extern uchar* XBuf;
bool skipNextFrame = true;
extern uint16_t* my_palette;
bool scaling_enabled = false;
uint8_t frameskip = 3;

inline void update_ui_fps() {
    stopTime = xthal_get_ccount();
    int elapsedTime;
    if (stopTime > startTime)
      elapsedTime = (stopTime - startTime);
    else
      elapsedTime = ((uint64_t)stopTime + (uint64_t)0xffffffff) - (startTime);

    totalElapsedTime += elapsedTime;
    ++my_frame;

    if (my_frame == 60)
    {
      float seconds = totalElapsedTime / (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000.0f);
      float fps = my_frame / seconds;
      printf("FPS:%f\n", fps);
#ifdef MY_DEBUG_CHECKS
      if (cycles_ > 0)
      {
        printf("Further OpCodes must be inlined! %d\n", cycles_);
      }
#endif
      
      my_frame = 0;
      totalElapsedTime = 0;
#ifdef ODROID_DEBUG_PERF_USE
    odroid_debug_perf_log_one("cpu"        , ODROID_DEBUG_PERF_CPU);
    odroid_debug_perf_log_one("loop6502"   , ODROID_DEBUG_PERF_LOOP6502);
    odroid_debug_perf_log_one("INT"        , ODROID_DEBUG_PERF_INT);
    odroid_debug_perf_log_one("INT2"       , ODROID_DEBUG_PERF_INT2);
    
    odroid_debug_perf_log_one("R_S_E"      , ODROID_DEBUG_PERF_SPRITE_RefreshSpriteExact);
    odroid_debug_perf_log_one("Refr_Line"  , ODROID_DEBUG_PERF_SPRITE_RefreshLine);
    odroid_debug_perf_log_one("Refr_Scr"   , ODROID_DEBUG_PERF_SPRITE_RefreshScreen);
    odroid_debug_perf_log_one("Mem Op Acc" , ODROID_DEBUG_PERF_MEM_ACCESS1);
#endif
ODROID_DEBUG_PERF_LOG()
    }
    startTime = stopTime;
}

int
osd_gfx_init(void)
{
    printf("%s: \n", __func__);
    startTime = xthal_get_ccount();
    SetPalette();
    return true;
}

int
osd_gfx_init_normal_mode()
{
    printf("%s: (%dx%d)\n", __func__, io.screen_w, io.screen_h);
    startTime = xthal_get_ccount();
    SetPalette();
    update_display_task(io.screen_w);
    return true;
}

void
osd_gfx_put_image_normal(void)
{
   //printf("%s: %d\n", __func__, my_frame);
   /*
   if ((my_frame%15)==0)
   {
    //ili9341_write_frame_rectangleLE(0,0,300,240, osd_gfx_buffer-32);
   }
   */
    if ((my_frame%frameskip)==1)
    {
    // printf("RES: (%dx%d)\n", io.screen_w, io.screen_h);
#ifdef MY_GFX_AS_TASK
#ifndef MY_VIDEO_MODE_SCANLINES
    xQueueSend(vidQueue, &osd_gfx_buffer, portMAX_DELAY);
#endif
    current_framebuffer = current_framebuffer ? 0 : 1;
#else
    ili9341_write_frame_pcengine_mode0(osd_gfx_buffer, my_palette);
#endif
    XBuf = framebuffer[current_framebuffer];
    osd_gfx_buffer = XBuf + 32 + 64 * XBUF_WIDTH;
    skipNextFrame = true;
    } else if ((frame%frameskip)==0) {
        skipNextFrame = false;
     } else {
        skipNextFrame = true;
     }
   update_ui_fps();
}

void
osd_gfx_shut_normal_mode(void)
{
   printf("%s: \n", __func__);
}

#define COLOR_RGB(r,g,b) ( (((r)<<12)&0xf800) + (((g)<<7)&0x07e0) + (((b)<<1)&0x001f) )
void
osd_gfx_set_color(uchar index, uchar r, uchar g, uchar b)
{
   uint16_t col;
   if (index==255)
   {
      col = 0xffff;
   }
   else
   {
     r = r >> 2;
     g = g >> 2;
     b = b >> 2;
     col = COLOR_RGB( (r),(g),(b) );
     col = ((col&0x00ff) << 8) | ((col&0xff00) >> 8);
   }
   my_palette[index] = col;
}

osd_gfx_driver osd_gfx_driver_list[1] = {
    {osd_gfx_init, osd_gfx_init_normal_mode,
     osd_gfx_put_image_normal, osd_gfx_shut_normal_mode}
};
#endif
