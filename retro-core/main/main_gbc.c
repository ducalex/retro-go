#include "shared.h"

#include <sys/time.h>
#include <gnuboy.h>

static int skipFrames = 20; // The 20 is to hide startup flicker in some games
static bool slowFrame = false;

static int video_time;
static int audio_time;

static const char *sramFile;
static int autoSaveSRAM = 0;
static int autoSaveSRAM_Timer = 0;
static int useSystemTime = true;

static const char *SETTING_SAVESRAM = "SaveSRAM";
static const char *SETTING_PALETTE  = "Palette";
static const char *SETTING_SYSTIME = "SysTime";
// --- MAIN


static void update_rtc_time(void)
{
    if (!useSystemTime)
        return;
    time_t timer = time(NULL);
    struct tm *info = localtime(&timer);
    gnuboy_set_time(info->tm_yday, info->tm_hour, info->tm_min, info->tm_sec);
}

static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        rg_display_submit(currentUpdate, 0);
    }
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_display_save_frame(filename, currentUpdate, width, height);
}

static bool save_state_handler(const char *filename)
{
    return gnuboy_save_state(filename) == 0;
}

static bool load_state_handler(const char *filename)
{
    if (gnuboy_load_state(filename) != 0)
    {
        // If a state fails to load then we should behave as we do on boot
        // which is a hard reset and load sram if present
        gnuboy_reset(true);
        gnuboy_load_sram(sramFile);
        update_rtc_time();

        return false;
    }

    update_rtc_time();

    skipFrames = 0;
    autoSaveSRAM_Timer = 0;

    // TO DO: Call rtc_sync() if a physical RTC is present
    return true;
}

static bool reset_handler(bool hard)
{
    gnuboy_reset(hard);
    update_rtc_time();

    skipFrames = 20;
    autoSaveSRAM_Timer = 0;

    return true;
}

static rg_gui_event_t palette_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int pal = gnuboy_get_palette();
    int max = GB_PALETTE_COUNT - 1;

    if (event == RG_DIALOG_PREV)
        pal = pal > 0 ? pal - 1 : max;

    if (event == RG_DIALOG_NEXT)
        pal = pal < max ? pal + 1 : 0;

    if (pal != gnuboy_get_palette())
    {
        rg_settings_set_number(NS_APP, SETTING_PALETTE, pal);
        gnuboy_set_palette(pal);
        gnuboy_run(true);
        return RG_DIALOG_REDRAW;
    }

    if (pal == GB_PALETTE_DMG)
        strcpy(option->value, "DMG   ");
    else if (pal == GB_PALETTE_MGB0)
        strcpy(option->value, "Pocket");
    else if (pal == GB_PALETTE_MGB1)
        strcpy(option->value, "Light ");
    else if (pal == GB_PALETTE_CGB)
        strcpy(option->value, "GBC   ");
    else if (pal == GB_PALETTE_SGB)
        strcpy(option->value, "SGB   ");
    else
        sprintf(option->value, "%d/%d   ", pal + 1, max - 1);

    return RG_DIALOG_VOID;
}

static rg_gui_event_t sram_autosave_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV) autoSaveSRAM--;
    if (event == RG_DIALOG_NEXT) autoSaveSRAM++;

    autoSaveSRAM = RG_MIN(RG_MAX(0, autoSaveSRAM), 999);

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        rg_settings_set_number(NS_APP, SETTING_SAVESRAM, autoSaveSRAM);
    }

    if (autoSaveSRAM == 0) strcpy(option->value, "Off ");
    else sprintf(option->value, "%3ds", autoSaveSRAM);

    return RG_DIALOG_VOID;
}

static rg_gui_event_t rtc_t_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int d, h, m, s;

    gnuboy_get_time(&d, &h, &m, &s);

    if (option->arg == 'd') {
        if (event == RG_DIALOG_PREV && --d < 0) d = 364;
        if (event == RG_DIALOG_NEXT && ++d > 364) d = 0;
        sprintf(option->value, "%02d", d);
    }
    if (option->arg == 'h') {
        if (event == RG_DIALOG_PREV && --h < 0) h = 23;
        if (event == RG_DIALOG_NEXT && ++h > 23) h = 0;
        sprintf(option->value, "%02d", h);
    }
    if (option->arg == 'm') {
        if (event == RG_DIALOG_PREV && --m < 0) m = 59;
        if (event == RG_DIALOG_NEXT && ++m > 59) m = 0;
        sprintf(option->value, "%02d", m);
    }
    if (option->arg == 's') {
        if (event == RG_DIALOG_PREV && --s < 0) s = 59;
        if (event == RG_DIALOG_NEXT && ++s > 59) s = 0;
        sprintf(option->value, "%02d", s);
    }
    if (option->arg == 'x') {
        if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
            useSystemTime = !useSystemTime;
            rg_settings_set_number(NS_APP, SETTING_SYSTIME, useSystemTime);
        }
        strcpy(option->value, useSystemTime ? "Yes" : "No ");
    }

    gnuboy_set_time(d, h, m, s);

    // TO DO: Update system clock

    return RG_DIALOG_VOID;
}

static rg_gui_event_t rtc_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER) {
        rg_gui_option_t choices[] = {
            {'d', "Day", "000", 1, &rtc_t_update_cb},
            {'h', "Hour", "00", 1, &rtc_t_update_cb},
            {'m', "Min",  "00", 1, &rtc_t_update_cb},
            {'s', "Sec",  "00", 1, &rtc_t_update_cb},
            {'x', "Sync",  "Yes", 1, &rtc_t_update_cb},
            RG_DIALOG_END
        };
        rg_gui_dialog("RTC config", choices, 0);
    }
    int h, m;
    gnuboy_get_time(NULL, &h, &m, NULL);
    sprintf(option->value, "%02d:%02d", h, m);
    return RG_DIALOG_VOID;
}

static void video_callback(void *buffer)
{
    int64_t startTime = rg_system_timer();
    slowFrame = !rg_display_sync(false);
    rg_display_submit(currentUpdate, 0);
    video_time += rg_system_timer() - startTime;
}


static void audio_callback(void *buffer, size_t length)
{
    int64_t startTime = rg_system_timer();
    rg_audio_submit(buffer, length >> 1);
    audio_time += rg_system_timer() - startTime;
}

void gbc_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
    };
    const rg_gui_option_t options[] = {
        {0, "Palette", "7/7", 1, &palette_update_cb},
        {0, "RTC config", "00:00", 1, &rtc_update_cb},
        {0, "SRAM autosave", "Off", 1, &sram_autosave_cb},
        RG_DIALOG_END
    };

    app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, options);

    updates[0].buffer = rg_alloc(GB_WIDTH * GB_HEIGHT * 2, MEM_ANY);
    updates[1].buffer = rg_alloc(GB_WIDTH * GB_HEIGHT * 2, MEM_ANY);

    rg_display_set_source_format(GB_WIDTH, GB_HEIGHT, 0, 0, GB_WIDTH * 2, RG_PIXEL_565_BE);

    useSystemTime = (int)rg_settings_get_number(NS_APP, SETTING_SYSTIME, 1);
    autoSaveSRAM = (int)rg_settings_get_number(NS_APP, SETTING_SAVESRAM, 0);
    sramFile = rg_emu_get_path(RG_PATH_SAVE_SRAM, app->romPath);

    if (!rg_storage_mkdir(rg_dirname(sramFile)))
        RG_LOGE("Unable to create SRAM folder...");

    // Initialize the emulator
    if (gnuboy_init(app->sampleRate, GB_AUDIO_STEREO_S16, GB_PIXEL_565_BE, &video_callback, &audio_callback) < 0)
        RG_PANIC("Emulator init failed!");

    gnuboy_set_framebuffer(currentUpdate->buffer);
    gnuboy_set_soundbuffer((void *)audioBuffer, sizeof(audioBuffer) / 2);

    // Load ROM
    if (gnuboy_load_rom(app->romPath) < 0)
        RG_PANIC("ROM Loading failed!");

    // Load BIOS
    if (gnuboy_get_hwtype() == GB_HW_CGB)
        gnuboy_load_bios(RG_BASE_PATH_BIOS "/gbc_bios.bin");
    else
        gnuboy_load_bios(RG_BASE_PATH_BIOS "/gb_bios.bin");

    gnuboy_set_palette(rg_settings_get_number(NS_APP, SETTING_PALETTE, GB_PALETTE_DMG));

    // Hard reset to have a clean slate
    gnuboy_reset(true);

    // Load saved state or SRAM
    if (app->bootFlags & RG_BOOT_RESUME)
        rg_emu_load_state(app->saveSlot);
    else
        gnuboy_load_sram(sramFile);

    update_rtc_time();

    // Don't show palette option for GBC
    if (gnuboy_get_hwtype() == GB_HW_CGB)
        app->options++;

    // Ready!

    uint32_t joystick_old = -1;
    uint32_t joystick = 0;

    while (true)
    {
        joystick = rg_input_read_gamepad();

        if (joystick & (RG_KEY_MENU|RG_KEY_OPTION))
        {
            if (joystick & RG_KEY_MENU)
            {
                if (gnuboy_sram_dirty()) // save in case the user quits
                    gnuboy_save_sram(sramFile, false);
                rg_gui_game_menu();
            }
            else
                rg_gui_options_menu();
            rg_audio_set_sample_rate(app->sampleRate * app->speed);
        }
        else if (joystick != joystick_old)
        {
            int pad = 0;
            if (joystick & RG_KEY_UP) pad |= GB_PAD_UP;
            if (joystick & RG_KEY_RIGHT) pad |= GB_PAD_RIGHT;
            if (joystick & RG_KEY_DOWN) pad |= GB_PAD_DOWN;
            if (joystick & RG_KEY_LEFT) pad |= GB_PAD_LEFT;
            if (joystick & RG_KEY_SELECT) pad |= GB_PAD_SELECT;
            if (joystick & RG_KEY_START) pad |= GB_PAD_START;
            if (joystick & RG_KEY_A) pad |= GB_PAD_A;
            if (joystick & RG_KEY_B) pad |= GB_PAD_B;
            gnuboy_set_pad(pad); // That call is somewhat costly, that's why we try to avoid it
            joystick_old = joystick;
        }

        int64_t startTime = rg_system_timer();
        bool drawFrame = !skipFrames;

        video_time = audio_time = 0;

        if (drawFrame)
        {
            currentUpdate = &updates[currentUpdate == &updates[0]];
            gnuboy_set_framebuffer(currentUpdate->buffer);
        }
        gnuboy_run(drawFrame);

        if (autoSaveSRAM > 0)
        {
            if (autoSaveSRAM_Timer <= 0)
            {
                if (gnuboy_sram_dirty())
                {
                    autoSaveSRAM_Timer = autoSaveSRAM * 60;
                }
            }
            else if (--autoSaveSRAM_Timer == 0)
            {
                gnuboy_save_sram(sramFile, true);
            }
        }

        // Tick before submitting audio/syncing
        rg_system_tick(rg_system_timer() - startTime - audio_time);

        if (skipFrames == 0)
        {
            int frameTime = 1000000 / (app->tickRate * app->speed);
            int elapsed = rg_system_timer() - startTime;
            if (app->frameskip > 0)
                skipFrames = app->frameskip;
            else if (elapsed > frameTime + 1500) // Allow some jitter
                skipFrames = (elapsed + frameTime / 2) / frameTime;
            else if (drawFrame && slowFrame)
                skipFrames = 1;
            if (app->speed > 1.f)
                skipFrames += 2;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }
    }
}
