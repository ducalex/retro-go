#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "../components/snes9x/snes9x.h"
#include "../components/snes9x/memmap.h"
#include "../components/snes9x/apu/apu.h"
#include "../components/snes9x/gfx.h"
#include "../components/snes9x/snapshot.h"
#include "../components/snes9x/controls.h"

#include "keymap.h"

#define AUDIO_SAMPLE_RATE (22050)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50)

// static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];

static rg_app_t *app;

static uint32_t frames_counter = 0;

static int keymap_id = 0;
static keymap_t keymap;

static const char *SETTING_NOTIFIED = "notified";
static const char *SETTING_KEYMAP = "keymap";
// --- MAIN


const char *S9xBasename(const char *f)
{
	const char *s = strrchr(f, '/');
	return s ? s + 1 : f;
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
	static char buffer[36 * 3];

	puts(message);
	strncpy(buffer, message, sizeof(buffer));
	buffer[sizeof(buffer) - 1] = 0;
	S9xSetInfoString(buffer);
}

bool8 S9xBlitUpdate(int width, int height)
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
		S9xMapButtonT(i, keymap.keys[i].action);
	}
}

static rg_gui_event_t change_keymap_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
	if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
	{
		if (event == RG_DIALOG_PREV && --keymap_id < 0) keymap_id = KEYMAPS_COUNT - 1;
		if (event == RG_DIALOG_NEXT && ++keymap_id > KEYMAPS_COUNT - 1) keymap_id = 0;
		update_keymap(keymap_id);
		rg_settings_set_number(NS_APP, SETTING_KEYMAP, keymap_id);
		return RG_DIALOG_CLOSE;
	}
	else if (event == RG_DIALOG_ENTER || event == RG_DIALOG_ALT)
	{
		return RG_DIALOG_DISMISS;
	}
	return RG_DIALOG_VOID;
}

static rg_gui_event_t menu_keymap_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
	{
		rg_gui_option_t options[16 + 4] = {};
		char values[16][16];
		char profile[32];
		bool dismissed = false;

		while (!dismissed)
		{
			rg_gui_option_t *option = options;

			option->label = "Profile";
			option->value = strcat(strcat(strcpy(profile, "< "), keymap.name), " >");
			option->flags = RG_DIALOG_FLAG_NORMAL;
			option->update_cb = &change_keymap_cb;
			option++;

			option->flags = RG_DIALOG_FLAG_NORMAL;
			option++;

			for (int i = 0; i < keymap.size; i++)
			{
				// keys[i].key_id contains a bitmask, convert to bit number
				int key_id = log2(keymap.keys[i].key_id);

				// For now we don't display the D-PAD because it doesn't fit on large font
				if (key_id < 4)
					continue;

				const char *key = KEYNAMES[key_id];
				const char *mod = (keymap.keys[i].mod1) ? "MENU + " : "";
				option->label = keymap.keys[i].action;
				option->value = strcat(strcpy(values[i], mod), key);
				option->flags = RG_DIALOG_FLAG_NORMAL;
				option->update_cb = &change_keymap_cb;
				option++;
			}

			option->flags = RG_DIALOG_FLAG_LAST;
			option++;

			dismissed = rg_gui_dialog("Controls", options, 0) == -1;
			rg_display_clear(C_BLACK);
		}
	}

	strcpy(option->value, keymap.name);

    return RG_DIALOG_VOID;
}

static bool screenshot_handler(const char *filename, int width, int height)
{
	return rg_display_save_frame(filename, currentUpdate, width, height);
}

static bool save_state_handler(const char *filename)
{
	return S9xFreezeGame(filename) == SUCCESS;
}

static bool load_state_handler(const char *filename)
{
	bool ret = false;

	if (access(filename, F_OK) == 0)
	{
		ret = S9xUnfreezeGame(filename) == SUCCESS;
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

extern "C" void app_main(void)
{
	const rg_handlers_t handlers = {
		.loadState = &load_state_handler,
		.saveState = &save_state_handler,
		.reset = &reset_handler,
		.screenshot = &screenshot_handler,
		.event = NULL,
		.memRead = NULL,
		.memWrite = NULL,
	};
	const rg_gui_option_t options[] = {
		{2, "Controls", (char*)"", 1, &menu_keymap_cb},
		RG_DIALOG_CHOICE_LAST
	};

	app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);

	updates[0].buffer = rg_alloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2, MEM_SLOW);
	updates[1].buffer = rg_alloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2, MEM_SLOW);

	rg_display_set_source_format(SNES_WIDTH, SNES_HEIGHT, 0, 0, SNES_WIDTH * 2, RG_PIXEL_565_LE);

	puts("\nSnes9x " VERSION " for ESP32\n");

	S9xInitSettings();

	Settings.Stereo = FALSE;
	Settings.SoundPlaybackRate = AUDIO_SAMPLE_RATE;
	Settings.SoundSync = FALSE;
	Settings.Mute = TRUE;
	Settings.Transparency = TRUE;
	Settings.SkipFrames = 0;
	Settings.Paused = FALSE;

	GFX.Screen = (uint16*)currentUpdate->buffer;

	update_keymap(rg_settings_get_number(NS_APP, SETTING_KEYMAP, 0));

	if (!S9xMemoryInit())
		RG_PANIC("Memory init failed!");

	if (!S9xSoundInit(0))
		RG_PANIC("Sound init failed!");

	if (!S9xGraphicsInit())
		RG_PANIC("Graphics init failed!");

	if (!S9xLoadROM(app->romPath))
		RG_PANIC("ROM loading failed!");

	if (app->bootFlags & RG_BOOT_RESUME)
	{
		rg_emu_load_state(app->saveSlot);
	}

	// Do this last to make sure the user sees it only the first time a game is about to start
	if (!rg_settings_get_number(NS_APP, SETTING_NOTIFIED, 0))
	{
		rg_gui_alert("Important!", "SNES support is experimental.\nIt is slow and has no sound.");
		rg_settings_set_number(NS_APP, SETTING_NOTIFIED, 1);
		rg_settings_commit();
	}

	app->refreshRate = Settings.FrameRate;

	bool menuCancelled = false;
	bool menuPressed = false;

	while (1)
	{
		uint32_t joystick = rg_input_read_gamepad();

		if (menuPressed && !(joystick & RG_KEY_MENU))
		{
			if (!menuCancelled)
			{
				usleep(50 * 1000);
				rg_gui_game_menu();
			}
			menuCancelled = false;
		}
		else if (joystick & RG_KEY_OPTION)
		{
			rg_gui_options_menu();
		}

		int64_t startTime = rg_system_timer();

		menuPressed = joystick & RG_KEY_MENU;

		if (menuPressed && (joystick & (RG_KEY_B|RG_KEY_A|RG_KEY_START|RG_KEY_SELECT)))
		{
			menuCancelled = true;
		}

		for (int i = 0; i < keymap.size; i++)
		{
			S9xReportButton(i, (joystick & (keymap.keys[i].key_id)) && keymap.keys[i].mod1 == menuPressed);
		}

		S9xMainLoop();

		long elapsed = rg_system_timer() - startTime;

		if (IPPU.RenderThisFrame)
		{
			rg_video_update_t *previousUpdate = &updates[currentUpdate == &updates[0]];
			rg_display_queue_update(currentUpdate, previousUpdate);
			currentUpdate = previousUpdate;
		}

		rg_system_tick(elapsed);

		IPPU.RenderThisFrame = (((++frames_counter) & 3) == 3);
		GFX.Screen = (uint16*)currentUpdate->buffer;
	}
}
