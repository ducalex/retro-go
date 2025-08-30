#include "shared.h"

#include <snes9x.h>
#include <math.h>

// #define FRAME_DOUBLE_BUFFERING
// #define AUDIO_DOUBLE_BUFFERING
#define USE_AUDIO_TASK

typedef struct
{
	char name[16];
	struct {
		uint16_t snes9x_mask;
		uint16_t local_mask;
		uint16_t mod_mask;
	} keys[16];
} keymap_t;

static const keymap_t KEYMAPS[] = {
	{"Type A", {
		{SNES_A_MASK, RG_KEY_A, 0},
		{SNES_B_MASK, RG_KEY_B, 0},
		{SNES_X_MASK, RG_KEY_START, 0},
		{SNES_Y_MASK, RG_KEY_SELECT, 0},
		{SNES_TL_MASK, RG_KEY_B, RG_KEY_MENU},
		{SNES_TR_MASK, RG_KEY_A, RG_KEY_MENU},
		{SNES_START_MASK, RG_KEY_START, RG_KEY_MENU},
		{SNES_SELECT_MASK, RG_KEY_SELECT, RG_KEY_MENU},
		{SNES_UP_MASK, RG_KEY_UP, 0},
		{SNES_DOWN_MASK, RG_KEY_DOWN, 0},
		{SNES_LEFT_MASK, RG_KEY_LEFT, 0},
		{SNES_RIGHT_MASK, RG_KEY_RIGHT, 0},
	}},
	{"Type B", {
		{SNES_A_MASK, RG_KEY_START, 0},
		{SNES_B_MASK, RG_KEY_A, 0},
		{SNES_X_MASK, RG_KEY_SELECT, 0},
		{SNES_Y_MASK, RG_KEY_B, 0},
		{SNES_TL_MASK, RG_KEY_B, RG_KEY_MENU},
		{SNES_TR_MASK, RG_KEY_A, RG_KEY_MENU},
		{SNES_START_MASK, RG_KEY_START, RG_KEY_MENU},
		{SNES_SELECT_MASK, RG_KEY_SELECT, RG_KEY_MENU},
		{SNES_UP_MASK, RG_KEY_UP, 0},
		{SNES_DOWN_MASK, RG_KEY_DOWN, 0},
		{SNES_LEFT_MASK, RG_KEY_LEFT, 0},
		{SNES_RIGHT_MASK, RG_KEY_RIGHT, 0},
	}},
	{"Type C", {
		{SNES_A_MASK, RG_KEY_A, 0},
		{SNES_B_MASK, RG_KEY_B, 0},
		{SNES_X_MASK, 0, 0},
		{SNES_Y_MASK, 0, 0},
		{SNES_TL_MASK, 0, 0},
		{SNES_TR_MASK, 0, 0},
		{SNES_START_MASK, RG_KEY_START, 0},
		{SNES_SELECT_MASK, RG_KEY_SELECT, 0},
		{SNES_UP_MASK, RG_KEY_UP, 0},
		{SNES_DOWN_MASK, RG_KEY_DOWN, 0},
		{SNES_LEFT_MASK, RG_KEY_LEFT, 0},
		{SNES_RIGHT_MASK, RG_KEY_RIGHT, 0},
	}},
};

static const size_t KEYMAPS_COUNT = (sizeof(KEYMAPS) / sizeof(keymap_t));

static const char *SNES_BUTTONS[] = {
	"None", "None", "None", "None", "R", "L", "X", "A", "Right", "Left", "Down", "Up", "Start", "Select", "Y", "B"
};

#define AUDIO_LOW_PASS_RANGE ((60 * 65536) / 100)

static rg_app_t *app;
static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;
static rg_audio_sample_t *audioBuffers[2];
static rg_audio_sample_t *currentAudioBuffer;

#ifdef USE_AUDIO_TASK
static rg_task_t *audio_task_handle;
#endif

static bool sound_enabled = true;
static bool lowpass_filter = false;

static int keymap_id = 0;
static keymap_t keymap;

static const char *SETTING_KEYMAP = "keymap";
static const char *SETTING_SOUND_EMULATION = "apu";
static const char *SETTING_SOUND_FILTER = "filter";
// --- MAIN

static void update_keymap(int id)
{
    keymap_id = id % KEYMAPS_COUNT;
    keymap = KEYMAPS[keymap_id];
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_surface_save_image_file(currentUpdate, filename, width, height);
}

static bool save_state_handler(const char *filename)
{
    return S9xSaveState(filename);
}

static bool load_state_handler(const char *filename)
{
    return S9xLoadState(filename);
}

static bool reset_handler(bool hard)
{
    S9xReset();
    memset(audioBuffers[0], 0, AUDIO_BUFFER_LENGTH * 4);
    memset(audioBuffers[1], 0, AUDIO_BUFFER_LENGTH * 4);
    return true;
}

static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        rg_display_submit(currentUpdate, 0);
    }
}

static rg_gui_event_t apu_toggle_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        sound_enabled = !sound_enabled;
        rg_settings_set_number(NS_APP, SETTING_SOUND_EMULATION, sound_enabled);
    }

    strcpy(option->value, sound_enabled ? _("On") : _("Off"));
    return RG_DIALOG_VOID;
}

static rg_gui_event_t lowpass_filter_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        lowpass_filter = !lowpass_filter;
        rg_settings_set_number(NS_APP, SETTING_SOUND_FILTER, lowpass_filter);
    }

    strcpy(option->value, lowpass_filter ? _("On") : _("Off"));
    return RG_DIALOG_VOID;
}

static rg_gui_event_t change_keymap_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        if (event == RG_DIALOG_PREV && --keymap_id < 0)
            keymap_id = KEYMAPS_COUNT - 1;
        if (event == RG_DIALOG_NEXT && ++keymap_id > KEYMAPS_COUNT - 1)
            keymap_id = 0;
        update_keymap(keymap_id);
        rg_settings_set_number(NS_APP, SETTING_KEYMAP, keymap_id);
        return RG_DIALOG_REDRAW;
    }

    if (event == RG_DIALOG_ENTER)
    {
        return RG_DIALOG_CANCEL;
    }

    if (option->arg == -1)
    {
        strcat(strcat(strcpy(option->value, "< "), keymap.name), " >");
    }
    else if (option->arg >= 0)
    {
        int local_button = keymap.keys[option->arg].local_mask;
        int mod_button = keymap.keys[option->arg].mod_mask;
        int snes9x_button = log2(keymap.keys[option->arg].snes9x_mask); // convert bitmask to bit number

        if (snes9x_button < 4 || (local_button & (RG_KEY_UP|RG_KEY_DOWN|RG_KEY_LEFT|RG_KEY_RIGHT)))
        {
            option->flags = RG_DIALOG_FLAG_HIDDEN;
            return RG_DIALOG_VOID;
        }

        if (keymap.keys[option->arg].mod_mask)
            sprintf(option->value, "%s + %s", rg_input_get_key_name(mod_button), rg_input_get_key_name(local_button));
        else
            sprintf(option->value, "%s", rg_input_get_key_name(local_button));

        option->label = SNES_BUTTONS[snes9x_button];
        option->flags = RG_DIALOG_FLAG_NORMAL;
    }

    return RG_DIALOG_VOID;
}

static rg_gui_event_t menu_keymap_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        const rg_gui_option_t options[] = {
            {-1, _("Profile"), "-", RG_DIALOG_FLAG_NORMAL, &change_keymap_cb},
            {-2, "", NULL, RG_DIALOG_FLAG_MESSAGE, NULL},
            {-3, "snes9x  ", "handheld", RG_DIALOG_FLAG_MESSAGE, NULL},
            {0, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {1, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {2, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {3, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {4, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {5, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {6, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {7, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {8, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {9, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {10, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {11, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {12, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {13, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {14, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            {15, "-", "-", RG_DIALOG_FLAG_HIDDEN, &change_keymap_cb},
            RG_DIALOG_END,
        };
        rg_gui_dialog(option->label, options, 0);
        return RG_DIALOG_REDRAW;
    }

    strcpy(option->value, keymap.name);
    return RG_DIALOG_VOID;
}

bool S9xInitDisplay(void)
{
    GFX.Pitch = SNES_WIDTH * 2;
    GFX.ZPitch = SNES_WIDTH;
    GFX.Screen = currentUpdate->data;
    GFX.SubScreen = malloc(GFX.Pitch * SNES_HEIGHT_EXTENDED);
    GFX.ZBuffer = malloc(GFX.ZPitch * SNES_HEIGHT_EXTENDED);
    GFX.SubZBuffer = malloc(GFX.ZPitch * SNES_HEIGHT_EXTENDED);
    return GFX.Screen && GFX.SubScreen && GFX.ZBuffer && GFX.SubZBuffer;
}

void S9xDeinitDisplay(void)
{
}

uint32_t S9xReadJoypad(int32_t port)
{
    if (port != 0)
        return 0;

    uint32_t joystick = rg_input_read_gamepad();
    uint32_t joypad = 0;

    for (int i = 0; i < RG_COUNT(keymap.keys); ++i)
    {
        uint32_t bitmask = keymap.keys[i].local_mask | keymap.keys[i].mod_mask;
        if (bitmask && bitmask == (joystick & bitmask))
        {
            joypad |= keymap.keys[i].snes9x_mask;
        }
    }

    return joypad;
}

bool S9xReadMousePosition(int32_t which1, int32_t *x, int32_t *y, uint32_t *buttons)
{
    return false;
}

bool S9xReadSuperScopePosition(int32_t *x, int32_t *y, uint32_t *buttons)
{
    return false;
}

bool JustifierOffscreen(void)
{
    return true;
}

void JustifierButtons(uint32_t *justifiers)
{
    (void)justifiers;
}

static inline void mix_samples(int32_t count)
{
    currentAudioBuffer = audioBuffers[currentAudioBuffer == audioBuffers[0]];
    if (lowpass_filter)
        S9xMixSamplesLowPass((int16_t *)currentAudioBuffer, count, AUDIO_LOW_PASS_RANGE);
    else
        S9xMixSamples((int16_t *)currentAudioBuffer, count);
}

#ifdef USE_AUDIO_TASK
static void audio_task(void *arg)
{
    rg_task_msg_t msg;
    while (rg_task_receive(&msg))
    {
        if (msg.type == RG_TASK_MSG_STOP)
            break;
        if (msg.type != 0)
            mix_samples(AUDIO_BUFFER_LENGTH << 1);
        rg_audio_submit(currentAudioBuffer, AUDIO_BUFFER_LENGTH);
    }
}
#endif

static void options_handler(rg_gui_option_t *dest)
{
    *dest++ = (rg_gui_option_t){0, _("Audio enable"), "-", RG_DIALOG_FLAG_NORMAL, &apu_toggle_cb};
    *dest++ = (rg_gui_option_t){0, _("Audio filter"), "-", RG_DIALOG_FLAG_NORMAL, &lowpass_filter_cb};
    *dest++ = (rg_gui_option_t){0, _("Controls"),     "-", RG_DIALOG_FLAG_NORMAL, &menu_keymap_cb};
    *dest++ = (rg_gui_option_t)RG_DIALOG_END;
}

void snes_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
        .options = &options_handler,
    };
    app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, NULL);

    // Load settings
    sound_enabled = rg_settings_get_number(NS_APP, SETTING_SOUND_EMULATION, 1);
    lowpass_filter = rg_settings_get_number(NS_APP, SETTING_SOUND_FILTER, 0);
    update_keymap(rg_settings_get_number(NS_APP, SETTING_KEYMAP, 0));

    // Allocate surfaces and audio buffers
    updates[0] = rg_surface_create(SNES_WIDTH, SNES_HEIGHT_EXTENDED, RG_PIXEL_565_LE, 0);
    updates[0]->height = SNES_HEIGHT;
#ifdef FRAME_DOUBLE_BUFFERING
    updates[1] = rg_surface_create(SNES_WIDTH, SNES_HEIGHT_EXTENDED, RG_PIXEL_565_LE, 0);
    updates[1]->height = SNES_HEIGHT;
#else
    updates[1] = updates[0];
#endif
    currentUpdate = updates[0];

#ifdef AUDIO_DOUBLE_BUFFERING
    audioBuffers[0] = (rg_audio_sample_t *)calloc(AUDIO_BUFFER_LENGTH, 4);
    audioBuffers[1] = (rg_audio_sample_t *)calloc(AUDIO_BUFFER_LENGTH, 4);
#else
    audioBuffers[0] = (rg_audio_sample_t *)calloc(AUDIO_BUFFER_LENGTH, 4);
    audioBuffers[1] = audioBuffers[0];
#endif
    currentAudioBuffer = audioBuffers[0];

    if (!updates[0] || !updates[1] || !audioBuffers[0] || !audioBuffers[1])
        RG_PANIC("Failed to allocate buffers!");

#ifdef USE_AUDIO_TASK
    // Set up multicore audio
    audio_task_handle = rg_task_create("snes_audio", &audio_task, NULL, 2048, RG_TASK_PRIORITY_6, 1);
    RG_ASSERT(audio_task_handle, "Failed to create audio task!");
#endif

    Settings.CyclesPercentage = 100;
    Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
    Settings.FrameTimePAL = 20000;
    Settings.FrameTimeNTSC = 16667;
    Settings.ControllerOption = SNES_JOYPAD;
    Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
    Settings.SoundPlaybackRate = AUDIO_SAMPLE_RATE;
    Settings.SoundInputRate = AUDIO_SAMPLE_RATE;
    Settings.DisableSoundEcho = false;
    Settings.InterpolatedSound = true;

    if (!S9xInitDisplay())
        RG_PANIC("Display init failed!");

    if (!S9xInitMemory())
        RG_PANIC("Memory init failed!");

    if (!S9xInitAPU())
        RG_PANIC("APU init failed!");

    if (!S9xInitSound(0, 0))
        RG_PANIC("Sound init failed!");

    if (!S9xInitGFX())
        RG_PANIC("Graphics init failed!");

    S9xSetPlaybackRate(Settings.SoundPlaybackRate);

    const char *filename = app->romPath;

    if (rg_extension_match(filename, "zip"))
    {
        if (!rg_storage_unzip_file(filename, NULL, (void **)&Memory.ROM, &Memory.ROM_AllocSize, RG_FILE_USER_BUFFER))
            RG_PANIC("ROM file unzipping failed!");
        filename = NULL;
    }

    if (!LoadROM(filename))
        RG_PANIC("ROM loading failed!");

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    rg_system_set_tick_rate(Memory.ROMFramesPerSecond);
    app->frameskip = 3;

    bool menuCancelled = false;
    bool menuPressed = false;
    int skipFrames = 0;

    while (1)
    {
        uint32_t joystick = rg_input_read_gamepad();

        if (menuPressed && !(joystick & RG_KEY_MENU))
        {
            if (!menuCancelled)
            {
                rg_task_delay(50);
                rg_gui_game_menu();
            }
            menuCancelled = false;
        }
        else if (joystick & RG_KEY_OPTION)
        {
            rg_gui_options_menu();
            memset(audioBuffers[0], 0, AUDIO_BUFFER_LENGTH * 4);
            memset(audioBuffers[1], 0, AUDIO_BUFFER_LENGTH * 4);
        }

        menuPressed = joystick & RG_KEY_MENU;

        if (menuPressed && joystick & ~RG_KEY_MENU)
        {
            menuCancelled = true;
        }

        int64_t startTime = rg_system_timer();
        bool drawFrame = (skipFrames == 0);
        bool slowFrame = false;

        IPPU.RenderThisFrame = drawFrame;
        GFX.Screen = currentUpdate->data;

        S9xMainLoop();

    #ifdef USE_AUDIO_TASK
        rg_task_msg_t msg = {.type = (int)sound_enabled};
        rg_task_send(audio_task_handle, &msg);
    #endif

        if (drawFrame)
        {
            slowFrame = !rg_display_sync(false);
            rg_display_submit(currentUpdate, 0);
            currentUpdate = updates[currentUpdate == updates[0]];
        }

    #ifndef USE_AUDIO_TASK
        if (sound_enabled)
            mix_samples(AUDIO_BUFFER_LENGTH << 1);
        rg_system_tick(rg_system_timer() - startTime);
        rg_audio_submit(currentAudioBuffer, AUDIO_BUFFER_LENGTH);
    #else
        rg_system_tick(rg_system_timer() - startTime);
    #endif

        if (skipFrames == 0)
        {
            int elapsed = rg_system_timer() - startTime;
            if (app->frameskip > 0)
                skipFrames = app->frameskip;
            else if (elapsed > app->frameTime + 1500) // Allow some jitter
                skipFrames = 1; // (elapsed / frameTime)
            else if (drawFrame && slowFrame)
                skipFrames = 1;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }
    }
}
