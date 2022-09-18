#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef RETRO_GO
#include <rg_system.h>
#define LOG_PRINTF(level, x...) rg_system_log(RG_LOG_USER, NULL, x)
#else
#define LOG_PRINTF(level, x...) printf(x)
#define IRAM_ATTR
#endif

#define MESSAGE_ERROR(x, ...) LOG_PRINTF(1, "!! %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_WARN(x, ...)  LOG_PRINTF(2, "** %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_INFO(x, ...)  LOG_PRINTF(3, " * %s: " x, __func__, ## __VA_ARGS__)
// #define MESSAGE_DEBUG(x, ...) LOG_PRINTF(4, ">> %s: " x, __func__, ## __VA_ARGS__)
#define MESSAGE_DEBUG(x, ...) {}

#define GB_WIDTH (160)
#define GB_HEIGHT (144)

typedef uint8_t byte;

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
	GB_PALETTE_0,
	GB_PALETTE_1,
	GB_PALETTE_2,
	GB_PALETTE_3,
	GB_PALETTE_4,
	GB_PALETTE_5,
	GB_PALETTE_6,
	GB_PALETTE_7,
	GB_PALETTE_8,
	GB_PALETTE_9,
	GB_PALETTE_10,
	GB_PALETTE_11,
	GB_PALETTE_12,
	GB_PALETTE_13,
	GB_PALETTE_14,
	GB_PALETTE_15,
	GB_PALETTE_16,
	GB_PALETTE_17,
	GB_PALETTE_18,
	GB_PALETTE_19,
	GB_PALETTE_20,
	GB_PALETTE_21,
	GB_PALETTE_22,
	GB_PALETTE_23,
	GB_PALETTE_24,
	GB_PALETTE_25,
	GB_PALETTE_26,
	GB_PALETTE_27,
	GB_PALETTE_28,
	GB_PALETTE_29,
	GB_PALETTE_30,
	GB_PALETTE_31,
	GB_PALETTE_DMG,
	GB_PALETTE_MGB0,
	GB_PALETTE_MGB1,
	GB_PALETTE_CGB,
	GB_PALETTE_SGB,
	GB_PALETTE_COUNT,
} gb_palette_t;

typedef struct
{
	struct {
		bool enabled;
		int format; // gb_pixformat_t
		int colorize; // gb_palette_t
		void (*blit_func)(void);
		union
		{
			uint16_t *buffer16;
			uint8_t *buffer8;
			void *buffer;
		};
		uint16_t palette[64];
	} video;

	struct {
		bool enabled;
		bool stereo;
		long samplerate;
		int16_t *buffer;
		size_t pos, len;
	} audio;
} gb_host_t;

extern gb_host_t host;

int  gnuboy_init(int samplerate, bool stereo, int pixformat, void *blit_func);
int  gnuboy_load_bios(const char *file);
void gnuboy_free_bios(void);
int  gnuboy_load_rom(const char *file);
void gnuboy_free_rom(void);
void gnuboy_reset(bool hard);
void gnuboy_run(bool draw);
bool gnuboy_sram_dirty(void);
void gnuboy_load_bank(int);
void gnuboy_set_pad(int);

void gnuboy_get_time(int *day, int *hour, int *minute, int *second);
void gnuboy_set_time(int day, int hour, int minute, int second);
int  gnuboy_get_hwtype(void);
void gnuboy_set_hwtype(gb_hwtype_t type);
int  gnuboy_get_palette(void);
void gnuboy_set_palette(gb_palette_t pal);

int gnuboy_load_sram(const char *file);
int gnuboy_save_sram(const char *file, bool quick_save);
int gnuboy_load_state(const char *file);
int gnuboy_save_state(const char *file);
