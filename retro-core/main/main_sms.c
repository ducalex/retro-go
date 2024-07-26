#include "shared.h"

#include <smsplus.h>

static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;

const rg_keyboard_map_t coleco_keyboard = {
    .columns = 3,
    .rows = 4,
    .data = {
        '1', '2', '3',
        '4', '5', '6',
        '7', '8', '9',
        '*', '0', '#',
    },
};

static const char *SETTING_PALETTE = "palette";
// --- MAIN


static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        rg_display_submit(currentUpdate, 0);
    }
}

static bool screenshot_handler(const char *filename, int width, int height)
{
	return rg_surface_save_image_file(currentUpdate, filename, width, height);
}

static bool save_state_handler(const char *filename)
{
    FILE* f = fopen(filename, "w");
    if (f)
    {
        system_save_state(f);
        fclose(f);
        return true;
    }
    return false;
}

static bool load_state_handler(const char *filename)
{
    FILE* f = fopen(filename, "r");
    if (f)
    {
        system_load_state(f);
        fclose(f);
        return true;
    }
    system_reset();
    return false;
}

static bool reset_handler(bool hard)
{
    system_reset();
    return true;
}

static rg_gui_event_t palette_update_cb(rg_gui_option_t *opt, rg_gui_event_t event)
{
    int pal = option.tms_pal;
    int max = 2;

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        if (event == RG_DIALOG_PREV)
            pal = pal > 0 ? pal - 1 : max;
        else
            pal = pal < max ? pal + 1 : 0;

        if (option.tms_pal != pal)
        {
            option.tms_pal = pal;
            for (int i = 0; i < PALETTE_SIZE; i++)
                palette_sync(i);
            if (render_copy_palette(currentUpdate->palette))
                memcpy(updates[currentUpdate == updates[0]]->palette, currentUpdate->palette, 512);
            rg_settings_set_number(NS_APP, SETTING_PALETTE, pal);
        }
        return RG_DIALOG_REDRAW;
    }

    sprintf(opt->value, "%d/%d", pal + 1, max + 1);
    return RG_DIALOG_VOID;
}

void sms_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
    };
    const rg_gui_option_t options[] = {
        {0, "Palette ", "-", RG_DIALOG_FLAG_NORMAL, &palette_update_cb},
        RG_DIALOG_END
    };

    app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, options);

    updates[0] = rg_surface_create(SMS_WIDTH, SMS_HEIGHT, RG_PIXEL_PAL565_BE, MEM_FAST);
    updates[1] = rg_surface_create(SMS_WIDTH, SMS_HEIGHT, RG_PIXEL_PAL565_BE, MEM_FAST);
    currentUpdate = updates[0];

    system_reset_config();
    option.sndrate = AUDIO_SAMPLE_RATE;
    option.overscan = 0;
    option.extra_gg = 0;
    option.tms_pal = rg_settings_get_number(NS_APP, SETTING_PALETTE, 0);

    if (rg_extension_match(app->romPath, "sg"))
        option.console = 5;
    else if (strcmp(app->configNs, "col") == 0)
        option.console = 6;
    else
        option.console = 0;

    if (rg_extension_match(app->romPath, "zip"))
    {
        void *data;
        size_t size;
        if (!rg_storage_unzip_file(app->romPath, NULL, &data, &size, RG_FILE_ALIGN_16KB))
            RG_PANIC("ROM file unzipping failed!");
        if (!load_rom(data, RG_MAX(0x4000, size), size))
            RG_PANIC("ROM file loading failed!");
    }
    else if (!load_rom_file(app->romPath))
    {
        RG_PANIC("ROM file loading failed!");
    }

    bitmap.width = SMS_WIDTH;
    bitmap.height = SMS_HEIGHT;
    bitmap.pitch = bitmap.width;
    bitmap.data = currentUpdate->data;

    system_poweron();

    app->tickRate = (sms.display == DISPLAY_NTSC) ? FPS_NTSC : FPS_PAL;

    updates[0]->offset = bitmap.viewport.x;
    updates[0]->width = bitmap.viewport.w;
    updates[0]->height = bitmap.viewport.h;
    updates[1]->offset = bitmap.viewport.x;
    updates[1]->width = bitmap.viewport.w;
    updates[1]->height = bitmap.viewport.h;

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    int skipFrames = 0;
    int colecoKey = 0;
    int colecoKeyDecay = 0;

    while (true)
    {
        uint32_t joystick = rg_input_read_gamepad();

        if (joystick & (RG_KEY_MENU|RG_KEY_OPTION))
        {
            if (joystick & RG_KEY_MENU)
                rg_gui_game_menu();
            else
                rg_gui_options_menu();
        }

        int64_t startTime = rg_system_timer();
        bool drawFrame = !skipFrames;
        bool slowFrame = false;

        input.pad[0] = 0x00;
        input.pad[1] = 0x00;
        input.system = 0x00;

        if (joystick & RG_KEY_UP)    input.pad[0] |= INPUT_UP;
        if (joystick & RG_KEY_DOWN)  input.pad[0] |= INPUT_DOWN;
        if (joystick & RG_KEY_LEFT)  input.pad[0] |= INPUT_LEFT;
        if (joystick & RG_KEY_RIGHT) input.pad[0] |= INPUT_RIGHT;
        if (joystick & RG_KEY_A)     input.pad[0] |= INPUT_BUTTON2;
        if (joystick & RG_KEY_B)     input.pad[0] |= INPUT_BUTTON1;

        if (IS_SMS)
        {
            if (joystick & RG_KEY_START)  input.system |= INPUT_PAUSE;
            if (joystick & RG_KEY_SELECT) input.system |= INPUT_START;
        }
        else if (IS_GG)
        {
            if (joystick & RG_KEY_START)  input.system |= INPUT_START;
            if (joystick & RG_KEY_SELECT) input.system |= INPUT_PAUSE;
        }
        else // Coleco
        {
            coleco.keypad[0] = 0xff;
            coleco.keypad[1] = 0xff;

            if (colecoKeyDecay > 0)
            {
                coleco.keypad[0] = colecoKey;
                colecoKeyDecay--;
            }

            if (joystick & RG_KEY_START)
            {
                rg_gui_draw_text(RG_GUI_CENTER, RG_GUI_CENTER, 0, "To start, try: 1 or * or #", C_YELLOW, C_BLACK, RG_TEXT_BIGGER);
                rg_audio_set_mute(true);
                int key = rg_input_read_keyboard(&coleco_keyboard);
                rg_audio_set_mute(false);

                if (key >= '0' && key <= '9')
                    colecoKey = key - '0';
                else if (key == '*')
                    colecoKey = 10;
                else if (key == '#')
                    colecoKey = 11;
                else
                    colecoKey = 255;
                colecoKeyDecay = 3;
            }
            else if (joystick & RG_KEY_SELECT)
            {
                rg_task_delay(100);
                system_reset();
            }
        }

        system_frame(!drawFrame);

        if (drawFrame)
        {
            if (render_copy_palette(currentUpdate->palette))
                memcpy(updates[currentUpdate == updates[0]]->palette, currentUpdate->palette, 512);
            slowFrame = !rg_display_sync(false);
            rg_display_submit(currentUpdate, 0);
            currentUpdate = updates[currentUpdate == updates[0]]; // Swap
            bitmap.data = currentUpdate->data;
        }

        // The emulator's sound buffer isn't in a very convenient format, we must remix it.
        size_t sample_count = snd.sample_count;
        rg_audio_sample_t mixbuffer[sample_count];
        for (size_t i = 0; i < sample_count; i++)
        {
            mixbuffer[i].left = snd.stream[0][i] * 2.75f;
            mixbuffer[i].right = snd.stream[1][i] * 2.75f;
        }

        // Tick before submitting audio/syncing
        rg_system_tick(rg_system_timer() - startTime);

        // Audio is used to pace emulation :)
        rg_audio_submit(mixbuffer, sample_count);

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            int frameTime = 1000000 / (app->tickRate * app->speed);
            int elapsed = rg_system_timer() - startTime;
            if (app->frameskip > 0)
                skipFrames = app->frameskip;
            else if (elapsed > frameTime + 1500) // Allow some jitter
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
