extern "C" {
#include "shared.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

#include <handy.h>

static CSystem *lynx = NULL;

static int dpad_mapped_up;
static int dpad_mapped_down;
static int dpad_mapped_left;
static int dpad_mapped_right;

static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;
// static bool netplay = false;
// --- MAIN

static void set_display_mode(void)
{
    display_rotation_t rotation = rg_display_get_rotation();
    int width, height;

    if (rotation == RG_DISPLAY_ROTATION_AUTO)
    {
        rotation = RG_DISPLAY_ROTATION_OFF;

        switch (lynx->mCart->CRC32())
        {
            case 0x97501709: // Centipede
            case 0x0271B6E9: // Lexis
            case 0x006FD398: // NFL Football
            case 0xBCD10C3A: // Raiden
                rotation = RG_DISPLAY_ROTATION_LEFT;
                break;
            case 0x7F0EC7AD: // Gauntlet
            case 0xAC564BAA: // Gauntlet - The Third Encounter
            case 0xA53649F1: // Klax
                rotation = RG_DISPLAY_ROTATION_RIGHT;
                break;
            default:
                if (lynx->mCart->CartGetRotate() == CART_ROTATE_LEFT)
                    rotation = RG_DISPLAY_ROTATION_LEFT;
                if (lynx->mCart->CartGetRotate() == CART_ROTATE_RIGHT)
                    rotation = RG_DISPLAY_ROTATION_RIGHT;
        }
    }

    switch(rotation)
    {
        case RG_DISPLAY_ROTATION_LEFT:
            width = HANDY_SCREEN_HEIGHT;
            height = HANDY_SCREEN_WIDTH;
            lynx->mMikie->SetRotation(MIKIE_ROTATE_L);
            dpad_mapped_up    = BUTTON_RIGHT;
            dpad_mapped_down  = BUTTON_LEFT;
            dpad_mapped_left  = BUTTON_UP;
            dpad_mapped_right = BUTTON_DOWN;
            break;
        case RG_DISPLAY_ROTATION_RIGHT:
            width = HANDY_SCREEN_HEIGHT;
            height = HANDY_SCREEN_WIDTH;
            lynx->mMikie->SetRotation(MIKIE_ROTATE_R);
            dpad_mapped_up    = BUTTON_LEFT;
            dpad_mapped_down  = BUTTON_RIGHT;
            dpad_mapped_left  = BUTTON_DOWN;
            dpad_mapped_right = BUTTON_UP;
            break;
        default:
            width = HANDY_SCREEN_WIDTH;
            height = HANDY_SCREEN_HEIGHT;
            lynx->mMikie->SetRotation(MIKIE_NO_ROTATE);
            dpad_mapped_up    = BUTTON_UP;
            dpad_mapped_down  = BUTTON_DOWN;
            dpad_mapped_left  = BUTTON_LEFT;
            dpad_mapped_right = BUTTON_RIGHT;
            break;
    }

    updates[0]->width = width;
    updates[0]->height = height;
    updates[1]->width = width;
    updates[1]->height = height;
}

static CSystem *new_lynx(void)
{
    if (rg_extension_match(app->romPath, "zip"))
    {
        void *data;
        size_t size;
        if (!rg_storage_unzip_file(app->romPath, NULL, &data, &size, 0))
            RG_PANIC("ROM file unzipping failed!");
        CSystem *lynx = new CSystem((UBYTE*)data, size, MIKIE_PIXEL_FORMAT_16BPP_565_BE, app->sampleRate);
        free(data);
        return lynx;
    }
    return new CSystem(app->romPath, MIKIE_PIXEL_FORMAT_16BPP_565_BE, app->sampleRate);
}


static rg_gui_event_t rotation_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int rotation = (int)rg_display_get_rotation();

    if (event == RG_DIALOG_PREV) {
        if (--rotation < 0) rotation = RG_DISPLAY_ROTATION_COUNT - 1;
        rg_display_set_rotation((display_rotation_t)rotation);
        set_display_mode();
        return RG_DIALOG_REDRAW;
    }
    if (event == RG_DIALOG_NEXT) {
        if (++rotation > RG_DISPLAY_ROTATION_COUNT - 1) rotation = 0;
        rg_display_set_rotation((display_rotation_t)rotation);
        set_display_mode();
        return RG_DIALOG_REDRAW;
    }

    strcpy(option->value, "Off  ");
    if (rotation == RG_DISPLAY_ROTATION_AUTO)  strcpy(option->value, "Auto ");
    if (rotation == RG_DISPLAY_ROTATION_LEFT)  strcpy(option->value, "Left ");
    if (rotation == RG_DISPLAY_ROTATION_RIGHT) strcpy(option->value, "Right");

    return RG_DIALOG_VOID;
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
    return rg_surface_save_image_file(currentUpdate, filename, width, height);
}

static bool save_state_handler(const char *filename)
{
    bool ret = false;
    FILE *fp;

    if ((fp = fopen(filename, "wb")))
    {
        ret = lynx->ContextSave(fp);
        fclose(fp);
    }

    return ret;
}

static bool load_state_handler(const char *filename)
{
    bool ret = false;
    FILE *fp;

    if ((fp = fopen(filename, "rb")))
    {
        ret = lynx->ContextLoad(fp);
        fclose(fp);
    }

    if (!ret) lynx->Reset();

    return ret;
}

static bool reset_handler(bool hard)
{
    // This isn't nice but lynx->Reset() crashes...
    delete lynx;
    lynx = new_lynx();
    return true;
}

extern "C" void lynx_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
        .memRead = NULL,
        .memWrite = NULL,
    };
    const rg_gui_option_t options[] = {
        {0, "Rotation", (char *)"-", RG_DIALOG_FLAG_NORMAL, &rotation_cb},
        RG_DIALOG_END
    };

    app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, options);

    // the HANDY_SCREEN_WIDTH * HANDY_SCREEN_WIDTH is deliberate because of rotation
    updates[0] = rg_surface_create(HANDY_SCREEN_WIDTH, HANDY_SCREEN_WIDTH, RG_PIXEL_565_BE, MEM_FAST);
    updates[1] = rg_surface_create(HANDY_SCREEN_WIDTH, HANDY_SCREEN_WIDTH, RG_PIXEL_565_BE, MEM_FAST);
    currentUpdate = updates[0];

    // The Lynx has a variable framerate but 60 is typical
    app->tickRate = 60;

    // Init emulator
    lynx = new_lynx();

    if (lynx->mFileType == HANDY_FILETYPE_ILLEGAL)
    {
        RG_PANIC("ROM loading failed!");
    }

    gPrimaryFrameBuffer = (UBYTE*)currentUpdate->data;
    gAudioBuffer = (SWORD*)&audioBuffer;
    gAudioEnabled = 1;

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    set_display_mode();

    float sampleTime = 1000000.f / app->sampleRate;
    long skipFrames = 0;
    bool slowFrame = false;

    // Start emulation
    while (1)
    {
        uint32_t joystick = rg_input_read_gamepad();

        if (joystick & (RG_KEY_MENU|RG_KEY_OPTION))
        {
            if (joystick & RG_KEY_MENU)
                rg_gui_game_menu();
            else
                rg_gui_options_menu();
            sampleTime = 1000000.f / (app->sampleRate * app->speed);
        }

        int64_t startTime = rg_system_timer();
        bool drawFrame = !skipFrames;
        ULONG buttons = 0;

    	if (joystick & RG_KEY_UP)     buttons |= dpad_mapped_up;
    	if (joystick & RG_KEY_DOWN)   buttons |= dpad_mapped_down;
    	if (joystick & RG_KEY_LEFT)   buttons |= dpad_mapped_left;
    	if (joystick & RG_KEY_RIGHT)  buttons |= dpad_mapped_right;
    	if (joystick & RG_KEY_A)      buttons |= BUTTON_A;
    	if (joystick & RG_KEY_B)      buttons |= BUTTON_B;
    	if (joystick & RG_KEY_START)  buttons |= BUTTON_OPT2; // BUTTON_PAUSE
    	if (joystick & RG_KEY_SELECT) buttons |= BUTTON_OPT1;

        lynx->SetButtonData(buttons);
        lynx->UpdateFrame(drawFrame);

        if (drawFrame)
        {
            slowFrame = !rg_display_sync(false);
            rg_display_submit(currentUpdate, 0);
            currentUpdate = updates[currentUpdate == updates[0]];
            gPrimaryFrameBuffer = (UBYTE*)currentUpdate->data;
        }

        app->tickRate = AUDIO_SAMPLE_RATE / (gAudioBufferPointer / 2);
        rg_system_tick(rg_system_timer() - startTime);

        rg_audio_submit(audioBuffer, gAudioBufferPointer >> 1);

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            int frameTime = ((gAudioBufferPointer / 2) * sampleTime);
            int elapsed = rg_system_timer() - startTime;
            if (app->frameskip > 0)
                skipFrames = app->frameskip;
            // The Lynx uses a variable framerate so we use the count of generated audio samples as reference instead
            else if (elapsed > frameTime + 1500)
                skipFrames = 1; // (elapsed / frameTime)
            else if (drawFrame && slowFrame)
                skipFrames = 1;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }
        gAudioBufferPointer = 0;
    }
}
