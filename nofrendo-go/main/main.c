#include <rg_system.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <nofrendo.h>
#include <nes/nes.h>
#include <nes/input.h>
#include <nes/state.h>

#define APP_ID 10

#define AUDIO_SAMPLE_RATE   (32000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50 + 1)

#define SETTING_AUTOCROP "autocrop"

static uint8_t *romData;
static uint32_t romSize;

static uint16_t myPalette[64];
static rg_video_frame_t frames[2];
static rg_video_frame_t *currentUpdate = &frames[0];

static gamepad_state_t joystick1;
static gamepad_state_t *localJoystick = &joystick1;

static bool overscan = true;
static long autocrop = false;

static bool fullFrame = 0;
static long frameTime = 0;
static nes_t *nes;

static rg_app_desc_t *app;

#ifdef ENABLE_NETPLAY
static gamepad_state_t *remoteJoystick = &joystick2;
static gamepad_state_t joystick2;

static bool netplay = false;
#endif
// --- MAIN


static void netplay_handler(netplay_event_t event, void *arg)
{
#ifdef ENABLE_NETPLAY
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
            nes_reset(ZERO_RESET);
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

static bool save_state_handler(char *pathName)
{
    if (state_save(pathName) >= 0)
    {
        char *filename = rg_emu_get_path(EMU_PATH_SCREENSHOT, 0);
        if (filename)
        {
            rg_display_save_frame(filename, currentUpdate, 160, 0);
            rg_free(filename);
        }
        return true;
    }
    return false;
}

static bool load_state_handler(char *pathName)
{
    if (state_load(pathName) < 0)
    {
        nes_reset(HARD_RESET);
        return false;
    }
    return true;
}

static bool reset_handler(bool hard)
{
    nes_reset(hard ? HARD_RESET : SOFT_RESET);
    return true;
}


static dialog_return_t sprite_limit_cb(dialog_option_t *option, dialog_event_t event)
{
    int val = rg_settings_SpriteLimit_get();

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        val = val ? 0 : 1;
        rg_settings_SpriteLimit_set(val);
        ppu_setopt(PPU_LIMIT_SPRITES, val);
    }

    strcpy(option->value, val ? "On " : "Off");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t overscan_update_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        overscan = !overscan;
        rg_settings_DisplayOverscan_set(overscan);
    }

    strcpy(option->value, overscan ? "Auto" : "Off ");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t autocrop_update_cb(dialog_option_t *option, dialog_event_t event)
{
    int val = autocrop;
    int max = 2;

    if (event == RG_DIALOG_PREV) val = val > 0 ? val - 1 : max;
    if (event == RG_DIALOG_NEXT) val = val < max ? val + 1 : 0;

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        autocrop = val;
        rg_settings_app_int32_set(SETTING_AUTOCROP, autocrop);
    }

    if (val == 0) strcpy(option->value, "Never ");
    if (val == 1) strcpy(option->value, "Auto  ");
    if (val == 2) strcpy(option->value, "Always");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t region_update_cb(dialog_option_t *option, dialog_event_t event)
{
    int val = rg_settings_Region_get();
    int max = 2;

    if (event == RG_DIALOG_PREV) val = val > 0 ? val - 1 : max;
    if (event == RG_DIALOG_NEXT) val = val < max ? val + 1 : 0;

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        rg_settings_Region_set(val);
    }

    if (val == EMU_REGION_AUTO) strcpy(option->value, "Auto");
    if (val == EMU_REGION_NTSC) strcpy(option->value, "NTSC");
    if (val == EMU_REGION_PAL)  strcpy(option->value, "PAL ");

    return RG_DIALOG_IGNORE;
}

static dialog_return_t advanced_settings_cb(dialog_option_t *option, dialog_event_t event)
{
    if (event == RG_DIALOG_ENTER) {
        dialog_option_t options[] = {
            {1, "Region      ", "Auto ", 1, &region_update_cb},
            {2, "Overscan    ", "Auto ", 1, &overscan_update_cb},
            {3, "Crop sides  ", "Never", 1, &autocrop_update_cb},
            {4, "Sprite limit", "On   ", 1, &sprite_limit_cb},
            RG_DIALOG_CHOICE_LAST
        };
        rg_gui_dialog("Advanced", options, 0);
    }
    return RG_DIALOG_IGNORE;
}

static dialog_return_t palette_update_cb(dialog_option_t *option, dialog_event_t event)
{
    int pal = ppu_getopt(PPU_PALETTE_RGB);
    int max = PPU_PAL_COUNT - 1;

    if (event == RG_DIALOG_PREV) pal = pal > 0 ? pal - 1 : max;
    if (event == RG_DIALOG_NEXT) pal = pal < max ? pal + 1 : 0;

    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT) {
        rg_settings_Palette_set(pal);
        ppu_setopt(PPU_PALETTE_RGB, pal);
        rg_display_queue_update(currentUpdate, NULL);
        rg_display_queue_update(currentUpdate, NULL);
    }

    sprintf(option->value, "%.7s", ppu_getpalette(pal)->name);
    return RG_DIALOG_IGNORE;
}


uint8_t *osd_getromdata(void)
{
    return romData;
}

uint32_t osd_getromsize(void)
{
    return romSize;
}

void osd_loadstate()
{
    if (app->startAction == EMU_START_ACTION_RESUME)
    {
        rg_emu_load_state(0);
    }

    ppu_setopt(PPU_LIMIT_SPRITES, rg_settings_SpriteLimit_get());
    ppu_setopt(PPU_PALETTE_RGB, rg_settings_Palette_get());
    overscan = rg_settings_DisplayOverscan_get();
    autocrop = rg_settings_app_int32_get(SETTING_AUTOCROP, 0);

    nes = nes_getptr();
    frameTime = get_frame_time(nes->refresh_rate);

    app->refreshRate = nes->refresh_rate;
}

void osd_log(int type, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

int osd_init()
{
    input_connect(INP_JOYPAD0);
    input_connect(INP_JOYPAD1);
    return 0;
}

void osd_shutdown()
{
    //
}

// We've reached vsync. We need to process audio and sleep if we ran too fast
void osd_vsync()
{
    static int32_t skipFrames = 0;
    static int64_t lastSyncTime = 0;

    long elapsed = get_elapsed_time_since(lastSyncTime);

    if (skipFrames == 0)
    {
        if (app->speedupEnabled)
            skipFrames = app->speedupEnabled * 2;
        else if (elapsed >= frameTime) // Frame took too long
            skipFrames = 1;
        else if (nes->drawframe && fullFrame) // This could be avoided when scaling != full
            skipFrames = 1;
    }
    else if (skipFrames > 0)
    {
        skipFrames--;
    }

    // Tick before submitting audio/syncing
    rg_system_tick(!nes->drawframe, fullFrame, elapsed);

    nes->drawframe = (skipFrames == 0);

    // Use audio to throttle emulation
    if (!app->speedupEnabled)
    {
        rg_audio_submit(nes->apu->buffer, nes->apu->samples_per_frame);
    }

    lastSyncTime = get_elapsed_time();
}

/*
** Video
*/
void osd_setpalette(rgb_t *pal)
{
    for (int i = 0; i < 64; i++)
    {
        uint16_t c = (pal[i].b >> 3) + ((pal[i].g >> 2) << 5) + ((pal[i].r >> 3) << 11);
        myPalette[i] = (c >> 8) | ((c & 0xff) << 8);
    }
    rg_display_set_config_param(changed, 1);
}

void osd_blitscreen(uint8 *bmp)
{
    int crop_v = (overscan) ? nes->overscan : 0;
    int crop_h = (autocrop == 2) || (autocrop == 1 && nes->ppu->left_bg_counter > 210) ? 8 : 0;

    // A rolling average should be used for autocrop == 1, it causes jitter in some games...

    currentUpdate->buffer = NES_SCREEN_GETPTR(bmp, crop_h, crop_v);
    currentUpdate->stride = NES_SCREEN_PITCH;
    currentUpdate->width = NES_SCREEN_WIDTH - (crop_h * 2);
    currentUpdate->height = NES_SCREEN_HEIGHT - (crop_v * 2);

    rg_video_frame_t *previousUpdate = &frames[currentUpdate == &frames[0]];

    fullFrame = rg_display_queue_update(currentUpdate, previousUpdate) == RG_SCREEN_UPDATE_FULL;

    currentUpdate = previousUpdate;
}

void osd_getinput(void)
{
    uint16 input = 0;

    *localJoystick = rg_input_read_gamepad();

    if (localJoystick->values[GAMEPAD_KEY_MENU])
    {
        rg_gui_game_menu();
    }
    else if (localJoystick->values[GAMEPAD_KEY_VOLUME])
    {
        dialog_option_t options[] = {
            {100, "Palette", "Default", 1, &palette_update_cb},
            {101, "More...", NULL, 1, &advanced_settings_cb},
            RG_DIALOG_CHOICE_LAST};
        rg_gui_game_settings_menu(options);
    }

#ifdef ENABLE_NETPLAY
    if (netplay)
    {
        rg_netplay_sync(localJoystick, remoteJoystick, sizeof(gamepad_state_t));
        if (joystick2.values[GAMEPAD_KEY_START])  input |= INP_PAD_START;
        if (joystick2.values[GAMEPAD_KEY_SELECT]) input |= INP_PAD_SELECT;
        if (joystick2.values[GAMEPAD_KEY_UP])     input |= INP_PAD_UP;
        if (joystick2.values[GAMEPAD_KEY_RIGHT])  input |= INP_PAD_RIGHT;
        if (joystick2.values[GAMEPAD_KEY_DOWN])   input |= INP_PAD_DOWN;
        if (joystick2.values[GAMEPAD_KEY_LEFT])   input |= INP_PAD_LEFT;
        if (joystick2.values[GAMEPAD_KEY_A])      input |= INP_PAD_A;
        if (joystick2.values[GAMEPAD_KEY_B])      input |= INP_PAD_B;
        input_update(INP_JOYPAD1, input);
        input = 0;
    }
#endif

    if (joystick1.values[GAMEPAD_KEY_START])  input |= INP_PAD_START;
    if (joystick1.values[GAMEPAD_KEY_SELECT]) input |= INP_PAD_SELECT;
    if (joystick1.values[GAMEPAD_KEY_UP])     input |= INP_PAD_UP;
    if (joystick1.values[GAMEPAD_KEY_RIGHT])  input |= INP_PAD_RIGHT;
    if (joystick1.values[GAMEPAD_KEY_DOWN])   input |= INP_PAD_DOWN;
    if (joystick1.values[GAMEPAD_KEY_LEFT])   input |= INP_PAD_LEFT;
    if (joystick1.values[GAMEPAD_KEY_A])      input |= INP_PAD_A;
    if (joystick1.values[GAMEPAD_KEY_B])      input |= INP_PAD_B;

    input_update(INP_JOYPAD0, input);
}


void app_main(void)
{
    rg_emu_proc_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .netplay = &netplay_handler,
    };

    rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    rg_emu_init(&handlers);

    app = rg_system_get_app();

    frames[0].flags = RG_PIXEL_PAL|RG_PIXEL_565|RG_PIXEL_BE;
    frames[0].pixel_mask = 0x3F;
    frames[0].palette = myPalette;
    frames[1] = frames[0];

    // Load ROM
    MESSAGE_INFO("Loading rom file: '%s'\n", app->romPath);

    FILE *fp;
    if ((fp = fopen(app->romPath, "rb")))
    {
        fseek(fp, 0, SEEK_END);
        romSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        romData = rg_alloc(romSize, MEM_SLOW);
        romSize *= fread(romData, romSize, 1, fp);
        fclose(fp);
    }

    if (romSize < 16)
    {
        RG_PANIC("ROM file loading failed!");
    }

    MESSAGE_INFO("Loading complete! romSize=%d\n", romSize);

    int region, ret;

    switch (rg_settings_Region_get())
    {
        case EMU_REGION_AUTO: region = NES_AUTO; break;
        case EMU_REGION_NTSC: region = NES_NTSC; break;
        case EMU_REGION_PAL:  region = NES_PAL;  break;
        default: region = NES_NTSC; break;
    }

    MESSAGE_INFO("Nofrendo start!\n");

    ret = nofrendo_start(app->romPath, region, AUDIO_SAMPLE_RATE, true);

    switch (ret)
    {
        case -1: RG_PANIC("Init failed.");
        case -2: RG_PANIC("Unsupported ROM.");
        case -3: RG_PANIC("ROM Loading failed.");
        default: RG_PANIC("Nofrendo died!");
    }
}
