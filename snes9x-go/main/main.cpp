#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "../components/snes9x/snes9x.h"
#include "../components/snes9x/memory.h"
#include "../components/snes9x/apu/apu.h"
#include "../components/snes9x/gfx.h"
#include "../components/snes9x/snapshot.h"
#include "../components/snes9x/controls.h"

#include "keymap.h"

#define APP_ID 90

#define AUDIO_SAMPLE_RATE (22050)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50)

#define NVS_KEY_KEYMAP "keymap"

// static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_video_frame_t frames[2];
static rg_video_frame_t *currentUpdate = &frames[0];

static rg_app_desc_t *app;

static char temp_path[PATH_MAX + 1];

static uint32_t frames_counter = 0;

static int keymap_id = 0;
static keymap_t keymap;

// static bool netplay = false;
// --- MAIN


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

void S9xTextMode(void)
{
	// This is only used by the debugger
}

void S9xGraphicsMode(void)
{
	// This is only used by the debugger
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

bool8 S9xInitUpdate(void)
{
	return (TRUE);
}

bool8 S9xDeinitUpdate(int width, int height)
{
	return (TRUE);
}

void S9xSyncSpeed(void)
{

}

void S9xExit(void)
{
	exit(0);
}

static void update_keymap(int id)
{
	keymap_id = id % KEYMAPS_COUNT;

	memcpy(&keymap, &KEYMAPS[keymap_id], sizeof(keymap));

	S9xUnmapAllControls();

	for (int i = 0; i < keymap.size; i++)
	{
		keymap.keys[i].key_id %= GAMEPAD_KEY_MAX;
		S9xMapButtonT(i, keymap.keys[i].action);
	}
}

static bool menu_keymap_cb(dialog_choice_t *option, dialog_event_t event)
{
	const int max = KEYMAPS_COUNT - 1;
	const int prev = keymap_id;

    if (event == RG_DIALOG_PREV && --keymap_id < 0) keymap_id = max;
    if (event == RG_DIALOG_NEXT && ++keymap_id > max) keymap_id = 0;

	if (keymap_id != prev)
	{
		update_keymap(keymap_id);
		rg_settings_app_int32_set(NVS_KEY_KEYMAP, keymap_id);
	}

    strcpy(option->value, keymap.name);

	if (event == RG_DIALOG_ENTER)
	{
		dialog_choice_t *options = (dialog_choice_t *)calloc(keymap.size + 2, sizeof(dialog_choice_t));
		dialog_choice_t *option = options;

		for (int i = 0; i < keymap.size; i++)
		{
			// For now we don't display the D-PAD because it doesn't fit on large font
			if (keymap.keys[i].key_id < 4)
				continue;

			const char *key = KEYNAMES[keymap.keys[i].key_id];
			const char *mod = (keymap.keys[i].mod1) ? "MENU + " : "";
			strcpy(option->value, mod);
			strcat(option->value, key);
			option->label = keymap.keys[i].action;
			option->flags = RG_DIALOG_FLAG_NORMAL;
			option++;
		}

		option->label = "Close";
		option->flags = RG_DIALOG_FLAG_NORMAL;
		option++;

		RG_DIALOG_MAKE_LAST(option);

		rg_gui_dialog("SNES  :ODROID", options, -1);
		rg_display_clear(C_BLACK);

		free(options);
	}

    return false;
}

static bool save_state_handler(char *pathName)
{
	if (S9xFreezeGame(pathName))
	{
		// lupng creates a broken image on the SNES. It seems to be compressed incorrectly
		// so for now we won't have pretty screenshot...
		char *filename = rg_emu_get_path(EMU_PATH_SCREENSHOT, 0);
		if (filename)
		{
			// rg_display_save_frame(filename, currentUpdate, 160, 0);
			rg_free(filename);
		}
		return true;
	}
	return false;
}

static bool load_state_handler(char *pathName)
{
	bool ret = false;

	if (rg_filesize(pathName) > 0)
	{
		ret = S9xUnfreezeGame(pathName);
	}
	else
	{
		S9xReset();
	}

	return ret;
}

static bool reset_handler(bool hard)
{
	if (hard)
		S9xReset();
	else
		S9xSoftReset();

    return true;
}

static void snes9x_task(void *arg)
{
	printf("\nSnes9x " VERSION " for ODROID-GO\n");

	vTaskDelay(5); // Make sure app_main() returned and freed its stack

	S9xInitSettings();

	Settings.Stereo = FALSE;
	Settings.SoundPlaybackRate = AUDIO_SAMPLE_RATE;
	Settings.SoundInputRate = 20000;
	Settings.SoundSync = FALSE;
	Settings.Mute = TRUE;
	Settings.Transparency = TRUE;
	Settings.SkipFrames = 0;
	Settings.Paused = FALSE;

	GFX.Pitch = SNES_WIDTH * 2;
	GFX.Screen = (uint16*)currentUpdate->buffer;

	update_keymap(rg_settings_app_int32_get(NVS_KEY_KEYMAP, 0));

	if (!Memory.Init())
		RG_PANIC("Memory init failed!");

	if (!S9xInitAPU() || !S9xInitSound(0))
		RG_PANIC("Sound init failed!");

	if (!S9xGraphicsInit())
		RG_PANIC("Graphics init failed!");

	if (!Memory.LoadROM(app->romPath))
		RG_PANIC("ROM loading failed!");

    if (app->startAction == EMU_START_ACTION_RESUME)
    {
        rg_emu_load_state(0);
    }

	app->refreshRate = Settings.FrameRate;

	bool menuCancelled = false;
	bool menuPressed = false;
	bool fullFrame = false;

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
			dialog_choice_t options[] = {
				{2, "Controls", "ABC", 1, &menu_keymap_cb},
				RG_DIALOG_CHOICE_LAST};
			rg_gui_game_settings_menu(options);
		}

		int64_t startTime = get_elapsed_time();

		menuPressed = joystick.values[GAMEPAD_KEY_MENU];

		if (menuPressed && (joystick.values[GAMEPAD_KEY_B] || joystick.values[GAMEPAD_KEY_A]
				|| joystick.values[GAMEPAD_KEY_START] || joystick.values[GAMEPAD_KEY_SELECT]))
		{
			menuCancelled = true;
		}

		for (int i = 0; i < keymap.size; i++)
		{
			S9xReportButton(i, joystick.values[keymap.keys[i].key_id] && keymap.keys[i].mod1 == menuPressed);
		}

		S9xMainLoop();

		long elapsed = get_elapsed_time_since(startTime);

		if (IPPU.RenderThisFrame)
		{
			rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];
			fullFrame = rg_display_queue_update(currentUpdate, previousUpdate);
			currentUpdate = previousUpdate;
		}

		rg_system_tick(IPPU.RenderThisFrame, fullFrame, elapsed);

		IPPU.RenderThisFrame = (((++frames_counter) & 3) == 3);
		GFX.Screen = (uint16*)currentUpdate->buffer;
	}

	vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
	rg_emu_proc_t handlers = {
		.loadState = &load_state_handler,
		.saveState = &save_state_handler,
		.reset = &reset_handler,
		.netplay = NULL,
	};

	rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
	rg_emu_init(&handlers);

	app = rg_system_get_app();

	frames[0].flags = RG_PIXEL_565|RG_PIXEL_LE;
	frames[0].width = SNES_WIDTH;
	frames[0].height = SNES_HEIGHT;
	frames[0].stride = SNES_WIDTH * 2;
	frames[1] = frames[0];

	frames[0].buffer = rg_alloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2, MEM_SLOW);
	frames[1].buffer = rg_alloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2, MEM_SLOW);

	snes9x_task(NULL);
}
