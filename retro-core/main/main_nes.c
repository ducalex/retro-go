#include "shared.h"

#include <nofrendo.h>
#include <nes/nes.h>

static int overscan = true;
static int autocrop = 0;
static int palette = 0;
static bool slowFrame = false;
static bool nsfPlayer = false;
static nes_t *nes;

static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;

static const char *SETTING_AUTOCROP = "autocrop";
static const char *SETTING_OVERSCAN = "overscan";
static const char *SETTING_PALETTE = "palette";
static const char *SETTING_SPRITELIMIT = "spritelimit";
// --- MAIN


static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        if (nsfPlayer)
            rg_display_clear(C_BLACK);
        else if (nes)
            (nes->blit_func)(NULL);
    }
}

static bool screenshot_handler(const char *filename, int width, int height)
{
	return rg_surface_save_image_file(currentUpdate, filename, width, height);
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

static void build_palette(int n)
{
    uint16_t *pal = nofrendo_buildpalette(n, 16);
    for (int i = 0; i < 256; i++)
    {
        uint16_t color = (pal[i] >> 8) | ((pal[i]) << 8);
        updates[0]->palette[i] = color;
        updates[1]->palette[i] = color;
    }
    free(pal);
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
        return RG_DIALOG_REDRAW;
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
        return RG_DIALOG_REDRAW;
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
        return RG_DIALOG_REDRAW;
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
    slowFrame = bmp && !rg_display_sync(false);
    // A rolling average should be used for autocrop == 1, it causes jitter in some games...
    // int crop_h = (autocrop == 2) || (autocrop == 1 && nes->ppu->left_bg_counter > 210) ? 8 : 0;
    int crop_v = (overscan) ? nes->overscan : 0;
    int crop_h = (autocrop) ? 8 : 0;
    // crop_h = (autocrop == 2) || (autocrop == 1 && nes->ppu->left_bg_counter > 210) ? 8 : 0;
    currentUpdate->width = NES_SCREEN_WIDTH - crop_h * 2;
    currentUpdate->height = NES_SCREEN_HEIGHT - crop_v * 2;
    currentUpdate->offset = crop_v * currentUpdate->stride + crop_h + 8;
    rg_display_submit(currentUpdate, 0);
}

static void nsf_draw_overlay(void)
{
    extern int nsf_current_song;
    char song[32];
    const nsfheader_t *header = (nsfheader_t *)nes->cart->data_ptr;
    const rg_gui_option_t options[] = {
        {0, "Name      ", (char*)header->name,      RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Artist    ", (char*)header->artist,    RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Copyright ", (char*)header->copyright, RG_DIALOG_FLAG_NORMAL, NULL},
        {0, "Playing   ", (char*)song,              RG_DIALOG_FLAG_NORMAL, NULL},
        RG_DIALOG_END,
    };
    snprintf(song, sizeof(song), "%d / %d", nsf_current_song, header->total_songs);
    rg_gui_draw_dialog("NSF Player", options, -1);
}


void nes_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .event = &event_handler,
        .screenshot = &screenshot_handler,
    };
    const rg_gui_option_t options[] = {
        {0, "Palette     ", "-", RG_DIALOG_FLAG_NORMAL, &palette_update_cb},
        {0, "Overscan    ", "-", RG_DIALOG_FLAG_NORMAL, &overscan_update_cb},
        {0, "Crop sides  ", "-", RG_DIALOG_FLAG_NORMAL, &autocrop_update_cb},
        {0, "Sprite limit", "-", RG_DIALOG_FLAG_NORMAL, &sprite_limit_cb},
        RG_DIALOG_END
    };

    app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, options);

    overscan = rg_settings_get_number(NS_APP, SETTING_OVERSCAN, 1);
    autocrop = rg_settings_get_number(NS_APP, SETTING_AUTOCROP, 0);
    palette = rg_settings_get_number(NS_APP, SETTING_PALETTE, 0);

    updates[0] = rg_surface_create(NES_SCREEN_PITCH, NES_SCREEN_HEIGHT, RG_PIXEL_PAL565_BE, MEM_FAST);
    updates[1] = rg_surface_create(NES_SCREEN_PITCH, NES_SCREEN_HEIGHT, RG_PIXEL_PAL565_BE, MEM_FAST);
    currentUpdate = updates[0];

    nes = nes_init(SYS_DETECT, app->sampleRate, true, RG_BASE_PATH_BIOS "/fds_bios.bin");
    if (!nes)
        RG_PANIC("Init failed.");

    int ret = -1;

    if (rg_extension_match(app->romPath, "zip"))
    {
        void *data;
        size_t size;
        if (!rg_storage_unzip_file(app->romPath, NULL, &data, &size, RG_FILE_ALIGN_8KB))
            RG_PANIC("ROM file unzipping failed!");
        ret = nes_insertcart(rom_loadmem(data, size));
    }
    else
    {
        ret = nes_loadfile(app->romPath);
    }

    if (ret == -1)
        RG_PANIC("ROM load failed.");
    else if (ret == -2)
        RG_PANIC("Unsupported mapper.");
    else if (ret == -3)
        RG_PANIC("BIOS file required.");
    else if (ret < 0)
        RG_PANIC("Unsupported ROM.");

    app->tickRate = nes->refresh_rate;
    nes->blit_func = blit_screen;

    nsfPlayer = nes->cart->type == ROM_TYPE_NSF;

    ppu_setopt(PPU_LIMIT_SPRITES, rg_settings_get_number(NS_APP, SETTING_SPRITELIMIT, 1));

    build_palette(palette);

    // This is necessary for successful state restoration
    // I have not yet investigated why that is...
    nes_emulate(false);
    nes_emulate(false);

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    int skipFrames = 0;

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
        bool drawFrame = !skipFrames && !nsfPlayer;
        int buttons = 0;

        if (joystick & RG_KEY_START)  buttons |= NES_PAD_START;
        if (joystick & RG_KEY_SELECT) buttons |= NES_PAD_SELECT;
        if (joystick & RG_KEY_UP)     buttons |= NES_PAD_UP;
        if (joystick & RG_KEY_RIGHT)  buttons |= NES_PAD_RIGHT;
        if (joystick & RG_KEY_DOWN)   buttons |= NES_PAD_DOWN;
        if (joystick & RG_KEY_LEFT)   buttons |= NES_PAD_LEFT;
        if (joystick & RG_KEY_A)      buttons |= NES_PAD_A;
        if (joystick & RG_KEY_B)      buttons |= NES_PAD_B;

        if (drawFrame)
        {
            currentUpdate = updates[currentUpdate == updates[0]];
            nes_setvidbuf(currentUpdate->data);
        }

        input_update(0, buttons);
        nes_emulate(drawFrame);

        // Tick before submitting audio/syncing
        rg_system_tick(rg_system_timer() - startTime);

        // Audio is used to pace emulation :)
        rg_audio_submit((void*)nes->apu->buffer, nes->apu->samples_per_frame);

        if (skipFrames == 0)
        {
            int frameTime = 1000000 / (nes->refresh_rate * app->speed);
            int elapsed = rg_system_timer() - startTime;
            if (nsfPlayer)
                skipFrames = 10, nsf_draw_overlay();
            else if (app->frameskip > 0)
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

    RG_PANIC("Nofrendo died!");
}
