    save_gfx_context(1);
    load_gfx_context(0);
    
    if (!skipNextFrame)
    if (!UCount) { /* Either we're in frameskip = 0 or we're in the frame to draw */
        if (SpriteON && SPONSwitch)
        {
#ifdef MY_INLINE_SPRITE
#if MY_INLINE_SPRITE==1
            int Y1 = last_display_counter;
            int Y2 = display_counter;
            uchar bg = 0;
            #include "sprite_RefreshSpriteExact.h"
#endif
#if MY_INLINE_SPRITE==2
            int Y1 = last_display_counter;
            int Y2 = display_counter;
            uchar bg = 0;
            #include "sprite_RefreshSpriteExact_V2.h"
#endif
#else
            RefreshSpriteExact(last_display_counter, display_counter - 1, 0);
#endif
        }
#ifdef MY_INLINE_SPRITE
        {
        int Y1 = last_display_counter;
        int Y2 = display_counter - 1;
        #include "sprite_RefreshLine.h"
        }
#else
        RefreshLine(last_display_counter, display_counter - 1);
#endif
        if (SpriteON && SPONSwitch)
        {
#ifdef MY_INLINE_SPRITE
#if MY_INLINE_SPRITE==1
            int Y1 = last_display_counter;
            int Y2 = display_counter;
            uchar bg = 1;
            #include "sprite_RefreshSpriteExact.h"
#endif
#if MY_INLINE_SPRITE==2
            int Y1 = last_display_counter;
            int Y2 = display_counter;
            uchar bg = 1;
            #include "sprite_RefreshSpriteExact_V2.h"
#endif
#else
            RefreshSpriteExact(last_display_counter, display_counter - 1, 1);
#endif
        }
#ifdef MY_VIDEO_MODE_SCANLINES
    struct my_scanline send;
    send.YY1 = last_display_counter;
    send.YY2 = display_counter;
    send.buffer = osd_gfx_buffer;
    xQueueSend(vidQueue, &send, portMAX_DELAY);
#endif
    }
    load_gfx_context(1);
    gfx_need_redraw = 0;
