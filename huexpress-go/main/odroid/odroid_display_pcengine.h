#ifndef ODROID_DISPLAY_EMU_IMPL
void ili9341_write_frame_pcengine_mode0(uint8_t* buffer, uint16_t* pal);
void ili9341_write_frame_pcengine_mode0_w224(uint8_t* buffer, uint16_t* pal);
void ili9341_write_frame_pcengine_mode0_w256(uint8_t* buffer, uint16_t* pal);
void ili9341_write_frame_pcengine_mode0_w320(uint8_t* buffer, uint16_t* pal);
void ili9341_write_frame_pcengine_mode0_w336(uint8_t* buffer, uint16_t* pal);

void ili9341_write_frame_pcengine_mode0_scanlines(struct my_scanline* scan, uint16_t* pal);
void ili9341_write_frame_pcengine_mode0_scanlines_w224(struct my_scanline* scan, uint16_t* pal);
void ili9341_write_frame_pcengine_mode0_scanlines_w256(struct my_scanline* scan, uint16_t* pal);
void ili9341_write_frame_pcengine_mode0_scanlines_w320(struct my_scanline* scan, uint16_t* pal);
void ili9341_write_frame_pcengine_mode0_scanlines_w336(struct my_scanline* scan, uint16_t* pal);

#else

#include "../huexpress/includes/cleantypes.h"

#define PCENGINE_GAME_WIDTH  352
#define PCENGINE_GAME_HEIGHT 240
#define PCENGINE_REMOVE_X 16
#define XBUF_WIDTH  (536 + 32 + 32)

extern uchar *Pal;
extern uchar *SPM;

void ili9341_write_frame_pcengine_mode0(uint8_t* buffer, uint16_t* pal)
{
    // ili9341_write_frame_rectangleLE(0,0,300,240, buffer -32);
#if 0    
    uint8_t* framePtr = buffer + PCENGINE_REMOVE_X;
    uint8_t *sPtr = SPM;
    short x, y;
    uchar pal0 = Pal[0];
    send_reset_drawing(0, 0, 320, 240);
    for (y = 0; y < PCENGINE_GAME_HEIGHT; y += 4)
    {
      uint16_t* line_buffer = line_buffer_get();
      uint16_t* line_buffer_ptr = line_buffer; 
      for (short i = 0; i < 4; ++i) // LINE_COUNT
      {
          //int index = i * displayWidth;
          for (x = 0; x < 320; ++x)
          {
            uint8_t source=*framePtr;
            *framePtr = pal0;
            framePtr++;
            uint16_t value1 = pal[source];
            //line_buffer[index++] = value1;
            *line_buffer_ptr = value1;
            line_buffer_ptr++;
            *sPtr = 0;
            sPtr++;
          }
          framePtr+=280;
      }
      send_continue_line(line_buffer, 320, 4);
    }
    //memset(buffer, Pal[0], 240 * XBUF_WIDTH);
    //memset(SPM, 0, 240 * XBUF_WIDTH);
#endif

    uint8_t* framePtr = buffer + PCENGINE_REMOVE_X;
    uint8_t *sPtr = SPM;
    short x, y;
    uchar pal0 = Pal[0];
    send_reset_drawing(0, 0, 320, 240);
    for (y = 0; y < PCENGINE_GAME_HEIGHT; y += 4)
    {
      uint16_t* line_buffer = line_buffer_get();
      uint16_t* line_buffer_ptr = line_buffer; 
      for (short i = 0; i < 4; ++i) // LINE_COUNT
      {
          for (x = 0; x < 320; ++x)
          {
            uint8_t source=*framePtr;
            *framePtr = pal0;
            framePtr++;
            uint16_t value1 = pal[source];
            *line_buffer_ptr = value1;
            line_buffer_ptr++;
            *sPtr = 0;
            sPtr++;
          }
          framePtr+=280;
          sPtr+=280;
      }
      send_continue_line(line_buffer, 320, 4);
    }
    //memset(buffer, Pal[0], 240 * XBUF_WIDTH);
    //memset(SPM, 0, 240 * XBUF_WIDTH);
    
    //#define MISSING ( 240 * XBUF_WIDTH - PCENGINE_GAME_HEIGHT*320)
    //memset(sPtr, 0, MISSING);
}

#define ODROID_DISPLAY_FRAME_RES(FUNC_NAME, WIDTH)           \
void FUNC_NAME(uint8_t* buffer, uint16_t* pal)               \
{                                                            \
    uint8_t* framePtr = buffer ;                             \
    uint8_t *sPtr = SPM;                                     \
    short x, y;                                              \
    uchar pal0 = Pal[0];                                     \
    send_reset_drawing((320-WIDTH)/2, 0, WIDTH, 240);        \
    for (y = 0; y < PCENGINE_GAME_HEIGHT; y += 4)            \
    {                                                        \
      uint16_t* line_buffer = line_buffer_get();             \
      uint16_t* line_buffer_ptr = line_buffer;               \
      for (short i = 0; i < 4; ++i)                          \
      {                                                      \
          for (x = 0; x < WIDTH; ++x)                        \
          {                                                  \
            uint8_t source=*framePtr;                        \
            *framePtr = pal0;                                \
            framePtr++;                                      \
            uint16_t value1 = pal[source];                   \
            *line_buffer_ptr = value1;                       \
            line_buffer_ptr++;                               \
            *sPtr = 0;                                       \
            sPtr++;                                          \
          }                                                  \
          framePtr+=280 + (320-WIDTH);                       \
          sPtr+=280 + (320-WIDTH);                           \
      }                                                      \
      send_continue_line(line_buffer, WIDTH, 4);             \
    }                                                        \
}

ODROID_DISPLAY_FRAME_RES(ili9341_write_frame_pcengine_mode0_w224, 224)





void ili9341_write_frame_pcengine_mode0_w256(uint8_t* buffer, uint16_t* pal)
{
    uint8_t* framePtr = buffer ;
    uint8_t *sPtr = SPM;
    short x, y;
    uchar pal0 = Pal[0];
    send_reset_drawing(32, 0, 256, 240);
    for (y = 0; y < PCENGINE_GAME_HEIGHT; y += 4)
    {
      uint16_t* line_buffer = line_buffer_get();
      uint16_t* line_buffer_ptr = line_buffer; 
      for (short i = 0; i < 4; ++i) // LINE_COUNT
      {
          for (x = 0; x < 256; ++x)
          {
            uint8_t source=*framePtr;
            *framePtr = pal0;
            framePtr++;
            uint16_t value1 = pal[source];
            *line_buffer_ptr = value1;
            line_buffer_ptr++;
            *sPtr = 0;
            sPtr++;
          }
          framePtr+=280 + 64;
          sPtr+=280 + 64;
      }
      send_continue_line(line_buffer, 256, 4);
    }
}

void ili9341_write_frame_pcengine_mode0_w320(uint8_t* buffer, uint16_t* pal)
{
    uint8_t* framePtr = buffer ;
    uint8_t *sPtr = SPM;
    short x, y;
    uchar pal0 = Pal[0];
    send_reset_drawing(0, 0, 320, 240);
    for (y = 0; y < PCENGINE_GAME_HEIGHT; y += 4)
    {
      uint16_t* line_buffer = line_buffer_get();
      uint16_t* line_buffer_ptr = line_buffer; 
      for (short i = 0; i < 4; ++i) // LINE_COUNT
      {
          for (x = 0; x < 320; ++x)
          {
            uint8_t source=*framePtr;
            *framePtr = pal0;
            framePtr++;
            uint16_t value1 = pal[source];
            *line_buffer_ptr = value1;
            line_buffer_ptr++;
            *sPtr = 0;
            sPtr++;
          }
          framePtr+=280;
          sPtr+=280;
      }
      send_continue_line(line_buffer, 320, 4);
    }
}

void ili9341_write_frame_pcengine_mode0_w336(uint8_t* buffer, uint16_t* pal)
{
    uint8_t* framePtr = buffer + 8;
    uint8_t *sPtr = SPM;
    short x, y;
    uchar pal0 = Pal[0];
    send_reset_drawing(0, 0, 320, 240);
    for (y = 0; y < PCENGINE_GAME_HEIGHT; y += 4)
    {
      uint16_t* line_buffer = line_buffer_get();
      uint16_t* line_buffer_ptr = line_buffer; 
      for (short i = 0; i < 4; ++i) // LINE_COUNT
      {
          for (x = 0; x < 320; ++x)
          {
            uint8_t source=*framePtr;
            *framePtr = pal0;
            framePtr++;
            uint16_t value1 = pal[source];
            *line_buffer_ptr = value1;
            line_buffer_ptr++;
            *sPtr = 0;
            sPtr++;
          }
          framePtr+=280;
          sPtr+=280;
      }
      send_continue_line(line_buffer, 320, 4);
    }
}

#define ODROID_DISPLAY_FRAME_SCANLINE_RES(FUNC_NAME, WIDTH)        



#define ODROID_DISPLAY_FRAME_SCANLINE_RES2(FUNC_NAME, WIDTH)                    \
void FUNC_NAME(struct my_scanline* scan, uint16_t* pal)                         \
{                                                                                       \
    /* printf("Scanline: %3d-%3d\n", scan->YY1, scan->YY2); */                          \
    uint8_t* framePtr = scan->buffer + scan->YY1 * XBUF_WIDTH + (WIDTH-320)/2;          \
    uint8_t *sPtr = SPM + (XBUF_WIDTH) * (scan->YY1);                                   \
    short x, y;                                                                         \
    uchar pal0 = Pal[0];                                                                \
    send_reset_drawing(0, scan->YY1, 320, scan->YY2);                                   \
    for (y = scan->YY1; y < scan->YY2 - 3; y += 4)                                      \
    {                                                                                   \
      uint16_t* line_buffer = line_buffer_get();                                        \
      uint16_t* line_buffer_ptr = line_buffer;                                          \
      for (short i = 0; i < 4; ++i)                                                     \
      {                                                                                 \
          for (x = 0; x < 320; ++x)                                                     \
          {                                                                             \
            uint8_t source=*framePtr;                                                   \
            *framePtr = pal0;                                                           \
            framePtr++;                                                                 \
            uint16_t value1 = pal[source];                                              \
            *line_buffer_ptr = value1;                                                  \
            line_buffer_ptr++;                                                          \
            *sPtr = 0;                                                                  \
            sPtr++;                                                                     \
          }                                                                             \
          framePtr+=280;                                                                \
          sPtr+=280;                                                                    \
      }                                                                                 \
      send_continue_line(line_buffer, 320, 4);                                          \
    }                                                                                   \
    int last = scan->YY2 - y;                                                           \
    if (last>0)                                                                         \
    {                                                                                   \
    uint16_t* line_buffer = line_buffer_get();                                          \
      uint16_t* line_buffer_ptr = line_buffer;                                          \
      for (short i = 0; i < last; ++i)                                                  \
      {                                                                                 \
          for (x = 0; x < 320; ++x)                                                     \
          {                                                                             \
            uint8_t source=*framePtr;                                                   \
            *framePtr = pal0;                                                           \
            framePtr++;                                                                 \
            uint16_t value1 = pal[source];                                              \
            *line_buffer_ptr = value1;                                                  \
            line_buffer_ptr++;                                                          \
            *sPtr = 0;                                                                  \
            sPtr++;                                                                     \
          }                                                                             \
          framePtr+=280;                                                                \
          sPtr+=280;                                                                    \
      }                                                                                 \
      send_continue_line(line_buffer, 320, last);                                       \
    }                                                                                   \
}

ODROID_DISPLAY_FRAME_SCANLINE_RES2(ili9341_write_frame_pcengine_mode0_scanlines, 352)
ODROID_DISPLAY_FRAME_SCANLINE_RES2(ili9341_write_frame_pcengine_mode0_scanlines_w320, 320)
ODROID_DISPLAY_FRAME_SCANLINE_RES2(ili9341_write_frame_pcengine_mode0_scanlines_w336, 336)

ODROID_DISPLAY_FRAME_SCANLINE_RES2(ili9341_write_frame_pcengine_mode0_scanlines_w224, 224)
ODROID_DISPLAY_FRAME_SCANLINE_RES2(ili9341_write_frame_pcengine_mode0_scanlines_w256, 256)

#endif
