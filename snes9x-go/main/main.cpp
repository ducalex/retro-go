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
#include "../components/snes9x/logger.h"
#include "../components/snes9x/display.h"
#include "../components/snes9x/statemanager.h"

#define APP_ID 90

#define AUDIO_SAMPLE_RATE (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50)

static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_video_frame_t frames[2];
static rg_video_frame_t *currentUpdate = &frames[0];

static rg_app_desc_t *app;
static gamepad_state_t joystick;

static long frames_counter = 0;

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
	return "/sdcard/roms/snes";
}

const char *S9xGetFilename(const char *ex, enum s9x_getdirtype dirtype)
{
	static char s[PATH_MAX + 1];
	char drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	snprintf(s, PATH_MAX + 1, "%s%s%s%s", S9xGetDirectory(dirtype), SLASH_STR, fname, ex);

	return (s);
}

const char *S9xGetFilenameInc(const char *ex, enum s9x_getdirtype dirtype)
{
	static char s[PATH_MAX + 1];
	char drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	unsigned int i = 0;
	const char *d;

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	d = S9xGetDirectory(dirtype);

	do
		snprintf(s, PATH_MAX + 1, "%s%s%s.%03d%s", d, SLASH_STR, fname, i++, ex);
	while (rg_filesize(s) > 0 && i < 1000);

	return (s);
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
	char s[PATH_MAX + 1];
	char drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(filename, drive, dir, fname, ext);

	if (*drive || *dir == SLASH_CHAR || (strlen(dir) > 1 && *dir == '.' && *(dir + 1) == SLASH_CHAR))
	{
		strncpy(s, filename, PATH_MAX + 1);
		s[PATH_MAX] = 0;
	}
	else
		snprintf(s, PATH_MAX + 1, "%s%s%s", S9xGetDirectory(SNAPSHOT_DIR), SLASH_STR, fname);

	if (!*ext && strlen(s) <= PATH_MAX - 4)
		strcat(s, ".frz");

	if ((*file = OPEN_STREAM(s, read_only ? "rb" : "wb")))
		return (TRUE);

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
    rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];

    // rg_display_queue_update(currentUpdate, previousUpdate);
    rg_display_queue_update(currentUpdate, NULL);

    currentUpdate = previousUpdate;
	GFX.Screen = (uint16*)currentUpdate->buffer;

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

static int64_t prevtime = 0;

void S9xSyncSpeed(void)
{
	int64_t curtime = get_elapsed_time();

	rg_system_tick(IPPU.RenderThisFrame, true, curtime - prevtime);

	prevtime = curtime;

	IPPU.RenderThisFrame = !IPPU.RenderThisFrame;

	joystick = rg_input_read_gamepad();

	if (joystick.values[GAMEPAD_KEY_MENU])
	{
		rg_gui_game_menu();
	}
	else if (joystick.values[GAMEPAD_KEY_VOLUME])
	{
		rg_gui_game_settings_menu(NULL);
	}

	for (int i = 0; i < GAMEPAD_KEY_MAX; i++) {
		S9xReportButton(i, joystick.values[i]);
	}
}

bool S9xPollButton(uint32 id, bool *pressed)
{
	return false;
}

bool S9xPollAxis(uint32 id, int16 *value)
{
	return false;
}

bool S9xPollPointer(uint32 id, int16 *x, int16 *y)
{
	return false;
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
	rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
	rg_emu_init(&load_state, &save_state, NULL);

	app = rg_system_get_app();

	frames[0].width = SNES_WIDTH;
	frames[0].height = SNES_HEIGHT_EXTENDED;
	frames[0].pixel_format = PIXEL_FORMAT_565_LE;
	frames[0].pixel_clear = -1;
	frames[0].stride = SNES_WIDTH * 2;
	frames[1] = frames[0];

	frames[0].buffer = rg_alloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2, MEM_SLOW);
	frames[1].buffer = rg_alloc(SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2, MEM_SLOW);

	printf("\nSnes9x " VERSION " for ODROID-GO\n");

	S9xInitSettings();

	Settings.SixteenBitSound = FALSE;
	Settings.Stereo = FALSE;
	Settings.SoundPlaybackRate = 22050;
	Settings.SoundInputRate = 31950;
	Settings.Mute = TRUE;
	Settings.AutoDisplayMessages = TRUE;
	Settings.Transparency = TRUE;
	Settings.DumpStreamsMaxFrames = -1;
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
	S9xVerifyControllers();

	#define MAP_BUTTON(id, name) S9xMapButton((id), S9xGetCommandT((name)), false)

    MAP_BUTTON(GAMEPAD_KEY_A, "Joypad1 A");
    MAP_BUTTON(GAMEPAD_KEY_B, "Joypad1 B");
    MAP_BUTTON(GAMEPAD_KEY_START, "Joypad1 Select");
    MAP_BUTTON(GAMEPAD_KEY_SELECT, "Joypad1 Start");
    MAP_BUTTON(GAMEPAD_KEY_LEFT, "Joypad1 Left");
    MAP_BUTTON(GAMEPAD_KEY_RIGHT, "Joypad1 Right");
    MAP_BUTTON(GAMEPAD_KEY_UP, "Joypad1 Up");
    MAP_BUTTON(GAMEPAD_KEY_DOWN, "Joypad1 Down");

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

	while (1)
	{
		S9xMainLoop();
	}

	vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
	// Important to set CONFIG_ESP_TIMER_TASK_STACK_SIZE=2048
	xTaskCreatePinnedToCore(&snes9x_task, "snes9x", 1024 * 48, NULL, 5, NULL, 0);
}
