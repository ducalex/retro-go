#include <rg_system.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "../components/gnuboy/save.h"
#include "../components/gnuboy/lcd.h"
#include "../components/gnuboy/sound.h"
#include "../components/gnuboy/gnuboy.h"

#define AUDIO_SAMPLE_RATE   (32000)

static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];

static rg_app_t *app;

static bool fullFrame = false;
static long skipFrames = 20; // The 20 is to hide startup flicker in some games

static const char *sramFile;
static long autoSaveSRAM = 0;
static long autoSaveSRAM_Timer = 0;

static const char *SETTING_SAVESRAM = "SaveSRAM";
static const char *SETTING_PALETTE  = "Palette";
// --- MAIN


static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_display_save_frame(filename, currentUpdate, width, height);
}

static bool save_state_handler(const char *filename)
{
    return state_save(filename) == 0;
}

static bool load_state_handler(const char *filename)
{
    if (state_load(filename) != 0)
    {
        // If a state fails to load then we should behave as we do on boot
        // which is a hard reset and load sram if present
        gnuboy_reset(true);
        sram_load(sramFile);

        return false;
    }

    skipFrames = 0;
    autoSaveSRAM_Timer = 0;

    // TO DO: Call rtc_sync() if a physical RTC is present
    return true;
}

static bool reset_handler(bool hard)
{
    gnuboy_reset(hard);

    fullFrame = false;
    skipFrames = 20;
    autoSaveSRAM_Timer = 0;

    if (hard)
    {
        time_t timer = time(NULL);
        struct tm *info = localtime(&timer);
        gnuboy_set_time(info->tm_yday, info->tm_hour, info->tm_min, info->tm_sec);
    }

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
        usleep(50000);
    }

    if (pal == GB_PALETTE_GBC)
        strcpy(option->value, "GBC");
    else if (pal == GB_PALETTE_SGB)
        strcpy(option->value, "SGB");
    else
        sprintf(option->value, "%d/%d", pal + 1, max - 1);

    return RG_DIALOG_VOID;
}

static rg_gui_event_t sram_save_now_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_ENTER)
    {
        rg_system_set_led(1);

        int ret = sram_save(sramFile, false);

        if (ret == -1)
            rg_gui_alert("Nothing to save", "Cart has no Battery or SRAM!");
        else if (ret < 0)
            rg_gui_alert("Save write failed!", sramFile);

        rg_system_set_led(0);

        return RG_DIALOG_CLOSE;
    }

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
    else sprintf(option->value, "%lds", autoSaveSRAM);

    return RG_DIALOG_VOID;
}

static rg_gui_event_t rtc_t_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int d, h, m, s;

    gnuboy_get_time(&d, &h, &m, &s);

    if (option->id == 'd') {
        if (event == RG_DIALOG_PREV && --d < 0) d = 364;
        if (event == RG_DIALOG_NEXT && ++d > 364) d = 0;
        sprintf(option->value, "%02d", d);
    }
    if (option->id == 'h') {
        if (event == RG_DIALOG_PREV && --h < 0) h = 23;
        if (event == RG_DIALOG_NEXT && ++h > 23) h = 0;
        sprintf(option->value, "%02d", h);
    }
    if (option->id == 'm') {
        if (event == RG_DIALOG_PREV && --m < 0) m = 59;
        if (event == RG_DIALOG_NEXT && ++m > 59) m = 0;
        sprintf(option->value, "%02d", m);
    }
    if (option->id == 's') {
        if (event == RG_DIALOG_PREV && --s < 0) s = 59;
        if (event == RG_DIALOG_NEXT && ++s > 59) s = 0;
        sprintf(option->value, "%02d", s);
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
            RG_DIALOG_CHOICE_LAST
        };
        rg_gui_dialog("Set Clock", choices, 0);
    }
    int h, m;
    gnuboy_get_time(NULL, &h, &m, NULL);
    sprintf(option->value, "%02d:%02d", h, m);
    return RG_DIALOG_VOID;
}

static void vblank_callback(void)
{
    rg_video_update_t *previousUpdate = &updates[currentUpdate == &updates[0]];

    fullFrame = rg_display_queue_update(currentUpdate, previousUpdate) == RG_UPDATE_FULL;

    // swap buffers
    currentUpdate = previousUpdate;
    lcd.out.buffer = currentUpdate->buffer;
}

static void auto_sram_update(void)
{
    if (autoSaveSRAM > 0 && gnuboy_sram_dirty())
    {
        rg_system_set_led(1);
        if (sram_save(sramFile, true) != 0)
        {
            MESSAGE_ERROR("sram_save() failed...\n");
        }
        rg_system_set_led(0);
    }
}

void app_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .netplay = NULL,
        .screenshot = &screenshot_handler,
    };
    const rg_gui_option_t options[] = {
        {100, "Palette", "7/7", 1, &palette_update_cb},
        {101, "Set clock", "00:00", 1, &rtc_update_cb},
        RG_DIALOG_SEPARATOR,
        {111, "Auto save SRAM", "Off", 1, &sram_autosave_cb},
        {112, "Save SRAM now ", NULL, 1, &sram_save_now_cb},
        RG_DIALOG_CHOICE_LAST
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);

    updates[0].buffer = rg_alloc(GB_WIDTH * GB_HEIGHT * 2, MEM_ANY);
    updates[1].buffer = rg_alloc(GB_WIDTH * GB_HEIGHT * 2, MEM_ANY);

    rg_display_set_source_format(GB_WIDTH, GB_HEIGHT, 0, 0, GB_WIDTH * 2, RG_PIXEL_565_BE);

    autoSaveSRAM = rg_settings_get_number(NS_APP, SETTING_SAVESRAM, 0);
    sramFile = rg_emu_get_path(RG_PATH_SAVE_SRAM, 0);

    if (!rg_mkdir(rg_dirname(sramFile)))
        RG_LOGW("Unable to create SRAM folder...");

    // Initialize the emulator
    gnuboy_init(AUDIO_SAMPLE_RATE, true, GB_PIXEL_565_BE, vblank_callback);

    // Load ROM
    if (gnuboy_load_rom(app->romPath) < 0)
        RG_PANIC("ROM Loading failed!");

    // Load BIOS
    if (gnuboy_get_hwtype() == GB_HW_CGB)
        gnuboy_load_bios(RG_BASE_PATH_SYSTEM "/gbc_bios.bin");
    else
        gnuboy_load_bios(RG_BASE_PATH_SYSTEM "/gb_bios.bin");

    gnuboy_set_palette(rg_settings_get_number(NS_APP, SETTING_PALETTE, GB_PALETTE_GBC));

    // Hard reset to have a clean slate
    gnuboy_reset(true);

    // Load saved state or SRAM
    if (app->bootFlags & RG_BOOT_RESUME)
        rg_emu_load_state(0);
    else
        sram_load(sramFile);

    // Don't show palette option for GBC
    if (gnuboy_get_hwtype() == GB_HW_CGB)
        app->options++;

    // Ready!

    uint32_t joystick_old = -1;
    uint32_t joystick = 0;

    while (true)
    {
        joystick = rg_input_read_gamepad();

        if (joystick & RG_KEY_MENU)
        {
            auto_sram_update();
            rg_gui_game_menu();
        }
        else if (joystick & RG_KEY_OPTION)
        {
            auto_sram_update();
            rg_gui_game_settings_menu();
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
            gnuboy_set_pad(pad);
            joystick_old = joystick;
        }

        int64_t startTime = get_elapsed_time();
        bool drawFrame = !skipFrames;

        gnuboy_run(drawFrame);

        // if (rtc.dirty)
        // {
            // At this point we know the time probably changed, we could update our
            // internal or external clock.
        //     printf("Time changed in game!\n");
        //     rtc.dirty = 0;
        // }

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
                auto_sram_update();

                #ifdef RG_TARGET_ODROID_GO
                // This is only necessary on the odroid-go
                skipFrames += 5;
                #endif
            }
        }

        long elapsed = get_elapsed_time_since(startTime);

        if (skipFrames == 0)
        {
            if (app->speedupEnabled)
                skipFrames = app->speedupEnabled * 2;
            else if (elapsed >= get_frame_time(60)) // Frame took too long
                skipFrames = 1;
            else if (drawFrame && fullFrame) // This could be avoided when scaling != full
                skipFrames = 1;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        // Tick before submitting audio/syncing
        rg_system_tick(elapsed);

        if (!app->speedupEnabled)
        {
            rg_audio_submit(snd.output.buf, snd.output.pos >> 1);
        }
    }
}
