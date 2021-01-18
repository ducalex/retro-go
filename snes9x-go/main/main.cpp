extern "C"
{
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
}

#include "../components/snes9x/snes9x.h"
#include "../components/snes9x/memmap.h"
#include "../components/snes9x/apu/apu.h"
#include "../components/snes9x/gfx.h"
#include "../components/snes9x/snapshot.h"
#include "../components/snes9x/controls.h"
#include "../components/snes9x/display.h"

#define APP_ID 90

#define AUDIO_SAMPLE_RATE (22050)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50)

// static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_video_frame_t frames[2];
static rg_video_frame_t *currentUpdate = &frames[0];

static rg_app_desc_t *app;

static char temp_path[PATH_MAX + 1];

static uint32_t frames_counter = 0;

// static bool netplay = false;
// --- MAIN

void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	*drive = 0;

	const char *slash = strrchr(path, SLASH_CHAR),
			   *dot = strrchr(path, '.');

	if (dot && slash && dot < slash)
		dot = NULL;

	if (!slash)
	{
		*dir = 0;

		strcpy(fname, path);

		if (dot)
		{
			fname[dot - path] = 0;
			strcpy(ext, dot + 1);
		}
		else
			*ext = 0;
	}
	else
	{
		strcpy(dir, path);
		dir[slash - path] = 0;

		strcpy(fname, slash + 1);

		if (dot)
		{
			fname[dot - slash - 1] = 0;
			strcpy(ext, dot + 1);
		}
		else
			*ext = 0;
	}
}

void _makepath(char *path, const char *, const char *dir, const char *fname, const char *ext)
{
	if (dir && *dir)
	{
		strcpy(path, dir);
		strcat(path, SLASH_STR);
	}
	else
		*path = 0;

	strcat(path, fname);

	if (ext && *ext)
	{
		strcat(path, ".");
		strcat(path, ext);
	}
}

const char *S9xGetDirectory(enum s9x_getdirtype dirtype)
{
	S9xGetFilename("none", dirtype);

	char *end = strrchr(temp_path, '/');

	if (end)
		*end = 0;

	return temp_path;
}

const char *S9xGetFilename(const char *ex, enum s9x_getdirtype dirtype)
{
	char *path;

	switch (dirtype)
	{
		case HOME_DIR:
		case ROMFILENAME_DIR:
		case ROM_DIR:
			path = rg_emu_get_path(EMU_PATH_ROM_FILE, 0);
			strcpy(temp_path, path);
			break;

		case SRAM_DIR:
			path = rg_emu_get_path(EMU_PATH_SAVE_SRAM, 0);
			strcpy(temp_path, path);
			break;

		case SNAPSHOT_DIR:
			path = rg_emu_get_path(EMU_PATH_SAVE_STATE, 0);
			strcpy(temp_path, path);
			break;

		default:
			path = rg_emu_get_path(EMU_PATH_TEMP_FILE, 0);
			strcpy(temp_path, path);
			strcpy(temp_path, ex);
			break;
	}

	free(path);

	return temp_path;
}

const char *S9xBasename(const char *f)
{
	return rg_get_filename(f);
}

const char *S9xChooseFilename(bool8 read_only)
{
	// Return a saved state
	return NULL;
}

bool8 S9xOpenSnapshotFile(const char *filename, bool8 read_only, STREAM *file)
{
	return (FALSE);
}

void S9xCloseSnapshotFile(STREAM file)
{
	CLOSE_STREAM(file);
}

void S9xTextMode(void)
{
}

void S9xGraphicsMode(void)
{
}

void S9xSetTitle(const char *string)
{
}

void S9xMessage(int type, int number, const char *message)
{
	const int max = 36 * 3;
	static char buffer[max + 1];

	fprintf(stdout, "%s\n", message);
	strncpy(buffer, message, max + 1);
	buffer[max] = 0;
	S9xSetInfoString(buffer);
}

void S9xInitDisplay(int argc, char **argv)
{
	GFX.Pitch = SNES_WIDTH * 2;
	GFX.Screen = (uint16*)currentUpdate->buffer;

	if (!S9xGraphicsInit())
	{
		fprintf(stderr, "Snes9x: Graphics Memory allocation failure - not enough RAM/virtual memory available.\nExiting...\n");
		Memory.Deinit();
		S9xDeinitAPU();
		exit(1);
	}
}

bool8 S9xInitUpdate(void)
{
	return (TRUE);
}

bool8 S9xDeinitUpdate(int width, int height)
{
	return (TRUE);
}

bool8 S9xContinueUpdate(int width, int height)
{
	return (TRUE);
}

void S9xToggleSoundChannel(int c)
{
	static uint8 sound_switch = 255;

	if (c == 8)
		sound_switch = 255;
	else
		sound_switch ^= 1 << c;

	S9xSetSoundControl(sound_switch);
}

void S9xSetPalette(void)
{
	return;
}

void S9xAutoSaveSRAM(void)
{
	// Memory.SaveSRAM(S9xGetFilename(".srm", SRAM_DIR));
}

void S9xSyncSpeed(void)
{

}

void S9xHandlePortCommand(s9xcommand_t cmd, int16 data1, int16 data2)
{

}

bool8 S9xOpenSoundDevice(void)
{
	return (FALSE);
}

void S9xExit(void)
{
}

static bool save_state(char *pathName)
{
	bool ret = false;

	return ret;
}

static bool load_state(char *pathName)
{
	bool ret = false;

	return ret;
}

static void snes9x_task(void *arg)
{
	printf("\nSnes9x " VERSION " for ODROID-GO\n");

	vTaskDelay(5); // Make sure app_main() returned and freed its stack

	S9xInitSettings();

	Settings.SixteenBitSound = FALSE;
	Settings.Stereo = FALSE;
	Settings.SoundPlaybackRate = AUDIO_SAMPLE_RATE;
	Settings.SoundInputRate = 20000;
	Settings.SoundSync = FALSE;
	Settings.Mute = TRUE;
	Settings.AutoDisplayMessages = TRUE;
	Settings.Transparency = TRUE;
	Settings.SkipFrames = 0;

	if (!Memory.Init() || !S9xInitAPU())
	{
		fprintf(stderr, "Snes9x: Memory allocation failure - not enough RAM/virtual memory available.\nExiting...\n");
		Memory.Deinit();
		S9xDeinitAPU();
		exit(1);
	}

	S9xInitDisplay(0, NULL);

	S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
	S9xSetController(1, CTL_NONE, 1, 0, 0, 0);

	S9xMapButtonT(SNES_A_MASK, "Joypad1 A");
    S9xMapButtonT(SNES_B_MASK, "Joypad1 B");
    S9xMapButtonT(SNES_X_MASK, "Joypad1 X");
    S9xMapButtonT(SNES_Y_MASK, "Joypad1 Y");
    S9xMapButtonT(SNES_TL_MASK, "Joypad1 L");
    S9xMapButtonT(SNES_TR_MASK, "Joypad1 R");
    S9xMapButtonT(SNES_START_MASK, "Joypad1 Start");
    S9xMapButtonT(SNES_SELECT_MASK, "Joypad1 X");
    S9xMapButtonT(SNES_LEFT_MASK, "Joypad1 Left");
    S9xMapButtonT(SNES_RIGHT_MASK, "Joypad1 Right");
    S9xMapButtonT(SNES_UP_MASK, "Joypad1 Up");
    S9xMapButtonT(SNES_DOWN_MASK, "Joypad1 Down");

	S9xInitSound(0);
	S9xSetSoundMute(TRUE);

	uint32 saved_flags = CPU.Flags;

	if (!Memory.LoadROM(app->romPath))
	{
		fprintf(stderr, "Error opening the ROM file.\n");
		exit(1);
	}

	// Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR));

	CPU.Flags = saved_flags;
	Settings.StopEmulation = FALSE;

	bool menuCancelled = false;
	bool menuPressed = false;

	while (1)
	{
		gamepad_state_t joystick = rg_input_read_gamepad();

		if (menuPressed && !joystick.values[GAMEPAD_KEY_MENU])
		{
			if (!menuCancelled)
			{
				vTaskDelay(5);
				rg_gui_game_menu();
			}
			menuCancelled = false;
		}
		else if (joystick.values[GAMEPAD_KEY_VOLUME])
		{
			rg_gui_game_settings_menu(NULL);
		}

		int64_t startTime = get_elapsed_time();

		menuPressed = joystick.values[GAMEPAD_KEY_MENU];

		if (menuPressed && (joystick.values[GAMEPAD_KEY_B] || joystick.values[GAMEPAD_KEY_A]
				|| joystick.values[GAMEPAD_KEY_START] || joystick.values[GAMEPAD_KEY_SELECT]))
		{
			menuCancelled = true;
		}

		S9xReportButton(SNES_TL_MASK, menuPressed && joystick.values[GAMEPAD_KEY_B]);
		S9xReportButton(SNES_TR_MASK, menuPressed && joystick.values[GAMEPAD_KEY_A]);
		S9xReportButton(SNES_START_MASK, menuPressed && joystick.values[GAMEPAD_KEY_START]);
		S9xReportButton(SNES_SELECT_MASK, menuPressed && joystick.values[GAMEPAD_KEY_SELECT]);

		S9xReportButton(SNES_A_MASK, !menuPressed && joystick.values[GAMEPAD_KEY_A]);
		S9xReportButton(SNES_B_MASK, !menuPressed && joystick.values[GAMEPAD_KEY_B]);
		S9xReportButton(SNES_X_MASK, !menuPressed && joystick.values[GAMEPAD_KEY_START]);
		S9xReportButton(SNES_Y_MASK, !menuPressed && joystick.values[GAMEPAD_KEY_SELECT]);

		S9xReportButton(SNES_UP_MASK, joystick.values[GAMEPAD_KEY_UP]);
		S9xReportButton(SNES_DOWN_MASK, joystick.values[GAMEPAD_KEY_DOWN]);
		S9xReportButton(SNES_LEFT_MASK, joystick.values[GAMEPAD_KEY_LEFT]);
		S9xReportButton(SNES_RIGHT_MASK, joystick.values[GAMEPAD_KEY_RIGHT]);

		S9xMainLoop();

		long elapsed = get_elapsed_time_since(startTime);

		if (IPPU.RenderThisFrame)
		{
			rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];

			bool full = rg_display_queue_update(currentUpdate, previousUpdate) == RG_SCREEN_UPDATE_FULL;
			// rg_display_queue_update(currentUpdate, NULL);

			currentUpdate = previousUpdate;
			GFX.Screen = (uint16*)currentUpdate->buffer;

			rg_system_tick(true, full, elapsed);
		}
		else
		{
			rg_system_tick(false, false, elapsed);
		}

		IPPU.RenderThisFrame = (((++frames_counter) & 3) == 3);
	}

	vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
	rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
	rg_emu_init(&load_state, &save_state, NULL);

	app = rg_system_get_app();

	frames[0].flags = RG_PIXEL_565|RG_PIXEL_LE;
	frames[0].width = SNES_WIDTH;
	frames[0].height = SNES_HEIGHT;
	frames[0].stride = SNES_WIDTH * 2;
	frames[1] = frames[0];

	frames[0].buffer = rg_alloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2, MEM_SLOW);
	frames[1].buffer = rg_alloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2, MEM_SLOW);

	heap_caps_malloc_extmem_enable(64 * 1024);

	// Important to set CONFIG_ESP_TIMER_TASK_STACK_SIZE=2048
	xTaskCreatePinnedToCore(&snes9x_task, "snes9x", 1024 * 32, NULL, 5, NULL, 0);
}
