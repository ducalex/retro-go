#pragma once

#include <rg_system.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define GB_WIDTH (160)
#define GB_HEIGHT (144)

#define LOG_PRINTF(level, x...) rg_system_log(RG_LOG_USER, NULL, x)
// #define LOG_PRINTF(level, x...) printf(x)

#define MESSAGE_ERROR(x, ...) LOG_PRINTF(1, "!! %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_WARN(x, ...)  LOG_PRINTF(2, "** %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_INFO(x, ...)  LOG_PRINTF(3, " * %s: " x, __func__, ## __VA_ARGS__)
// #define MESSAGE_DEBUG(x, ...) LOG_PRINTF(4, ">> %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_DEBUG(x, ...) {}

typedef uint8_t byte;
typedef uint8_t un8;
typedef uint16_t un16;
typedef uint32_t un32;
typedef int8_t n8;
typedef int16_t n16;
typedef int32_t n32;

typedef enum
{
	GB_HW_DMG,
	GB_HW_CGB,
	GB_HW_SGB,
} gb_hwtype_t;

typedef enum
{
	GB_PAD_RIGHT = 0x01,
	GB_PAD_LEFT = 0x02,
	GB_PAD_UP = 0x04,
	GB_PAD_DOWN = 0x08,
	GB_PAD_A = 0x10,
	GB_PAD_B = 0x20,
	GB_PAD_SELECT = 0x40,
	GB_PAD_START = 0x80,
} gb_padbtn_t;

typedef enum
{
	GB_PIXEL_PALETTED,
	GB_PIXEL_565_LE,
	GB_PIXEL_565_BE,
} gb_pixformat_t;

typedef enum
{
	GB_PALETTE_DEFAULT,
	GB_PALETTE_2BGRAYS,
	GB_PALETTE_LINKSAW,
	GB_PALETTE_NSUPRGB,
	GB_PALETTE_NGBARNE,
	GB_PALETTE_GRAPEFR,
	GB_PALETTE_MEGAMAN,
	GB_PALETTE_POKEMON,
	GB_PALETTE_DMGREEN,
	GB_PALETTE_GBC,
	GB_PALETTE_SGB,
	GB_PALETTE_COUNT,
} gb_palette_t;

int  gnuboy_init(int samplerate, bool stereo, int pixformat, void *vblank_func);
int  gnuboy_load_bios(const char *file);
void gnuboy_free_bios(void);
int  gnuboy_load_rom(const char *file);
void gnuboy_free_rom(void);
void gnuboy_reset(bool hard);
void gnuboy_run(bool draw);
bool gnuboy_sram_dirty(void);
void gnuboy_load_bank(int);
void gnuboy_set_pad(uint);

void gnuboy_get_time(int *day, int *hour, int *minute, int *second);
void gnuboy_set_time(int day, int hour, int minute, int second);
int  gnuboy_get_hwtype(void);
void gnuboy_set_hwtype(gb_hwtype_t type);
int  gnuboy_get_palette(void);
void gnuboy_set_palette(gb_palette_t pal);