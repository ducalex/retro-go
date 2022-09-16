#include <rg_system.h>
#include <string.h>
#include <stdlib.h>
#include <nofrendo.h>

#define AUDIO_SAMPLE_RATE   (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50 + 1)

static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];
static rg_video_update_t *previousUpdate = NULL;

static rg_app_t *app;

static uint32_t joystick1;
static uint32_t *localJoystick = &joystick1;

static int fullFrame = 0;
static int overscan = true;
static int autocrop = 0;
static int palette = 0;
static int crop_h, crop_v;
static nes_t *nes;

#ifdef RG_ENABLE_NETPLAY
static uint32_t *remoteJoystick = &joystick2;
static uint32_t joystick2;

static bool netplay = false;
#endif

static const char *SETTING_AUTOCROP = "autocrop";
static const char *SETTING_OVERSCAN = "overscan";
static const char *SETTING_PALETTE = "palette";
static const char *SETTING_SPRITELIMIT = "spritelimit";
// --- MAIN


static void event_handler(int event, void *arg)
{
#ifdef RG_ENABLE_NETPLAY
    bool new_netplay;

    switch (event)
    {
    case NETPLAY_EVENT_STATUS_CHANGED:
        new_netplay = (rg_netplay_status() == NETPLAY_STATUS_CONNECTED);

        if (netplay && !new_netplay)
        {
            rg_gui_alert("Netplay", "Connection lost!");
        }
        else if (!netplay && new_netplay)
        {
            // displayScalingMode = RG_DISPLAY_SCALING_FILL;
            // displayFilterMode = RG_DISPLAY_FILTER_NONE;
            // forceVideoRefresh = true;
            input_connect(1, NES_JOYPAD);
            nes_reset(true);
        }

        netplay = new_netplay;
        break;

    default:
        break;
    }

    if (netplay && rg_netplay_mode() == NETPLAY_MODE_GUEST)
    {
        localJoystick = &joystick2;
        remoteJoystick = &joystick1;
    }
    else
    {
        localJoystick = &joystick1;
        remoteJoystick = &joystick2;
    }
#endif
}

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
        nes_reset(true);
        return false;
    }
    return true;
}

static bool reset_handler(bool hard)
{
    nes_reset(hard);
    return true;
}


static void set_display_mode(void)
{
    crop_v = (overscan) ? nes->overscan : 0;
    crop_h = (autocrop) ? 8 : 0;
    // int crop_h = (autocrop == 2) || (autocrop == 1 && nes->ppu->left_bg_counter > 210) ? 8 : 0;

    int width = NES_SCREEN_WIDTH - (crop_h * 2);
    int height = NES_SCREEN_HEIGHT - (crop_v * 2);

    rg_display_set_source_format(width, height, 0, 0, NES_SCREEN_PITCH, RG_PIXEL_PAL565_BE);
}

static void build_palette(int n)
{
    uint16_t *pal = nofrendo_buildpalette(n, 16);
    for (int i = 0; i < 256; i++)
    {
        uint16_t color = (pal[i] >> 8) | ((pal[i]) << 8);
        updates[0].palette[i] = color;
        updates[1].palette[i] = color;
    }
    free(pal);
    previousUpdate = NULL;
}

static rg_gui_event_t sprite_limit_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    bool spritelimit = ppu_getopt(PPU_LIMIT_SPRITES);

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        spritelimit = !spritelimit;
        rg_settings_set_number(NS_APP, SETTING_SPRITELIMIT, spritelimit);
        ppu_setopt(PPU_LIMIT_SPRITES, spritelimit);
    }

    strcpy(option->value, spritelimit ? "On " : "Off");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t overscan_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        overscan = !overscan;
        rg_settings_set_number(NS_APP, SETTING_OVERSCAN, overscan);
        set_display_mode();
    }

    strcpy(option->value, overscan ? "Auto" : "Off ");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t autocrop_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int val = autocrop;
    int max = 2;

    if (event == RG_DIALOG_PREV) val = val > 0 ? val - 1 : max;
    if (event == RG_DIALOG_NEXT) val = val < max ? val + 1 : 0;

    if (val != autocrop)
    {
        autocrop = val;
        rg_settings_set_number(NS_APP, SETTING_AUTOCROP, val);
        set_display_mode();
    }

    if (val == 0) strcpy(option->value, "Never ");
    if (val == 1) strcpy(option->value, "Auto  ");
    if (val == 2) strcpy(option->value, "Always");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t palette_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int pal = palette;
    int max = NES_PALETTE_COUNT - 1;

    if (event == RG_DIALOG_PREV) pal = pal > 0 ? pal - 1 : max;
    if (event == RG_DIALOG_NEXT) pal = pal < max ? pal + 1 : 0;

    if (pal != palette)
    {
        palette = pal;
        rg_settings_set_number(NS_APP, SETTING_PALETTE, pal);
        build_palette(pal);
        rg_display_queue_update(currentUpdate, NULL);
        rg_display_queue_update(currentUpdate, NULL);
        rg_task_delay(50);
    }

    if (pal == NES_PALETTE_NOFRENDO)    strcpy(option->value, "Default    ");
    if (pal == NES_PALETTE_COMPOSITE)   strcpy(option->value, "Composite  ");
    if (pal == NES_PALETTE_NESCLASSIC)  strcpy(option->value, "NES Classic");
    if (pal == NES_PALETTE_NTSC)        strcpy(option->value, "NTSC       ");
    if (pal == NES_PALETTE_PVM)         strcpy(option->value, "PVM        ");
    if (pal == NES_PALETTE_SMOOTH)      strcpy(option->value, "Smooth     ");

    return RG_DIALOG_VOID;
}


static void blit_screen(uint8 *bmp)
{
    // A rolling average should be used for autocrop == 1, it causes jitter in some games...
    // int crop_h = (autocrop == 2) || (autocrop == 1 && nes->ppu->left_bg_counter > 210) ? 8 : 0;
    currentUpdate->buffer = NES_SCREEN_GETPTR(bmp, crop_h, crop_v);
    fullFrame = rg_display_queue_update(currentUpdate, previousUpdate) == RG_UPDATE_FULL;
    previousUpdate = currentUpdate;
    currentUpdate = &updates[currentUpdate == &updates[0]];
}

static void nsf_draw_overlay(void)
{
    extern int nsf_current_song;
    char song[32];
    const nsfheader_t *header = (nsfheader_t *)nes->cart->data_ptr;
    const rg_gui_option_t options[] = {
        {0, "Name      ", (char *)header->name, 1, NULL},
        {0, "Artist    ", (char *)header->artist, 1, NULL},
        {0, "Copyright ", (char *)header->copyright, 1, NULL},
        {0, "Playing   ", (char *)song, 1, NULL},
        RG_DIALOG_CHOICE_LAST,
    };
    snprintf(song, sizeof(song), "%d / %d", nsf_current_song, header->total_songs);
    rg_gui_draw_dialog("NSF Player", options, -1);
}


void app_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .event = &event_handler,
        .screenshot = &screenshot_handler,
    };
    const rg_gui_option_t options[] = {
        {1, "Palette     ", "Default", 1, &palette_update_cb},
        {2, "Overscan    ", "Auto ", 1, &overscan_update_cb},
        {3, "Crop sides  ", "Never", 1, &autocrop_update_cb},
        {4, "Sprite limit", "On   ", 1, &sprite_limit_cb},
        RG_DIALOG_CHOICE_LAST
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);

    overscan = rg_settings_get_number(NS_APP, SETTING_OVERSCAN, 1);
    autocrop = rg_settings_get_number(NS_APP, SETTING_AUTOCROP, 0);
    palette = rg_settings_get_number(NS_APP, SETTING_PALETTE, 0);

    nes = nes_init(SYS_DETECT, AUDIO_SAMPLE_RATE, true);
    if (!nes)
    {
        RG_PANIC("Init failed.");
    }

    int ret = nes_insertcart(app->romPath, RG_BASE_PATH_SYSTEM "/fds_bios.bin");
    if (ret == -1)
        RG_PANIC("ROM load failed.");
    else if (ret == -2)
        RG_PANIC("Unsupported mapper.");
    else if (ret == -3)
        RG_PANIC("BIOS file required.");
    else if (ret < 0)
        RG_PANIC("Unsupported ROM.");

    app->refreshRate = nes->refresh_rate;
    nes->blit_func = blit_screen;

    ppu_setopt(PPU_LIMIT_SPRITES, rg_settings_get_number(NS_APP, SETTING_SPRITELIMIT, 1));

    build_palette(palette);
    set_display_mode();

    // This is necessary for successful state restoration
    // I have not yet investigated why that is...
    nes_emulate(false);
    nes_emulate(false);

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    int skipFrames = 0;
    int nsfPlayer = nes->cart->mapper_number == 31;

    while (true)
    {
        *localJoystick = rg_input_read_gamepad();

        if (*localJoystick & (RG_KEY_MENU|RG_KEY_OPTION))
        {
            if (*localJoystick & RG_KEY_MENU)
                rg_gui_game_menu();
            else
                rg_gui_options_menu();
            rg_audio_set_sample_rate(app->sampleRate * app->speed);
            if (nsfPlayer)
                rg_display_clear(C_BLACK);
        }

        int64_t startTime = rg_system_timer();
        bool drawFrame = !skipFrames && !nsfPlayer;
        int buttons = 0;

        if (joystick1 & RG_KEY_START)  buttons |= NES_PAD_START;
        if (joystick1 & RG_KEY_SELECT) buttons |= NES_PAD_SELECT;
        if (joystick1 & RG_KEY_UP)     buttons |= NES_PAD_UP;
        if (joystick1 & RG_KEY_RIGHT)  buttons |= NES_PAD_RIGHT;
        if (joystick1 & RG_KEY_DOWN)   buttons |= NES_PAD_DOWN;
        if (joystick1 & RG_KEY_LEFT)   buttons |= NES_PAD_LEFT;
        if (joystick1 & RG_KEY_A)      buttons |= NES_PAD_A;
        if (joystick1 & RG_KEY_B)      buttons |= NES_PAD_B;
        input_update(0, buttons);

    #ifdef RG_ENABLE_NETPLAY
        if (netplay)
        {
            rg_netplay_sync(localJoystick, remoteJoystick, sizeof(*localJoystick));
            uint buttons = 0;
            if (joystick2 & RG_KEY_START)  buttons |= NES_PAD_START;
            if (joystick2 & RG_KEY_SELECT) buttons |= NES_PAD_SELECT;
            if (joystick2 & RG_KEY_UP)     buttons |= NES_PAD_UP;
            if (joystick2 & RG_KEY_RIGHT)  buttons |= NES_PAD_RIGHT;
            if (joystick2 & RG_KEY_DOWN)   buttons |= NES_PAD_DOWN;
            if (joystick2 & RG_KEY_LEFT)   buttons |= NES_PAD_LEFT;
            if (joystick2 & RG_KEY_A)      buttons |= NES_PAD_A;
            if (joystick2 & RG_KEY_B)      buttons |= NES_PAD_B;
            input_update(1, buttons);
        }
    #endif

        nes_emulate(drawFrame);

        int elapsed = rg_system_timer() - startTime;

        if (skipFrames == 0)
        {
            int frameTime = 1000000 / (nes->refresh_rate * app->speed);
            if (elapsed > frameTime - 2000) // It takes about 2ms to copy the audio buffer
                skipFrames = (elapsed + frameTime / 2) / frameTime;
            else if (drawFrame && fullFrame) // This could be avoided when scaling != full
                skipFrames = 1;
            else if (nsfPlayer)
                skipFrames = 10, nsf_draw_overlay();

            if (app->speed > 1.f) // This is a hack until we account for audio speed...
                skipFrames += (int)app->speed;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        // Tick before submitting audio/syncing
        rg_system_tick(elapsed);

        // Audio is used to pace emulation :)
        rg_audio_submit((void*)nes->apu->buffer, nes->apu->samples_per_frame);
    }

    RG_PANIC("Nofrendo died!");
}
