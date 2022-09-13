extern "C" {
#include <rg_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

#include <handy.h>

#define AUDIO_SAMPLE_RATE   (HANDY_AUDIO_SAMPLE_FREQ)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 40)

static rg_audio_sample_t audioBuffer[AUDIO_BUFFER_LENGTH];

static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];
static rg_app_t *app = NULL;
static CSystem *lynx = NULL;

static int dpad_mapped_up;
static int dpad_mapped_down;
static int dpad_mapped_left;
static int dpad_mapped_right;
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

    rg_display_set_source_format(width, height, 0, 0, HANDY_SCREEN_WIDTH * 2, RG_PIXEL_565_BE);
}


static rg_gui_event_t rotation_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    int rotation = (int)rg_display_get_rotation();

    if (event == RG_DIALOG_PREV) {
        if (--rotation < 0) rotation = RG_DISPLAY_ROTATION_COUNT - 1;
        rg_display_set_rotation((display_rotation_t)rotation);
        set_display_mode();
    }
    if (event == RG_DIALOG_NEXT) {
        if (++rotation > RG_DISPLAY_ROTATION_COUNT - 1) rotation = 0;
        rg_display_set_rotation((display_rotation_t)rotation);
        set_display_mode();
    }

    strcpy(option->value, "Off  ");
    if (rotation == RG_DISPLAY_ROTATION_AUTO)  strcpy(option->value, "Auto ");
    if (rotation == RG_DISPLAY_ROTATION_LEFT)  strcpy(option->value, "Left ");
    if (rotation == RG_DISPLAY_ROTATION_RIGHT) strcpy(option->value, "Right");

    return RG_DIALOG_VOID;
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_display_save_frame(filename, currentUpdate, width, height);
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
    lynx = new CSystem(app->romPath, MIKIE_PIXEL_FORMAT_16BPP_565_BE, AUDIO_SAMPLE_RATE);
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
        {100, "Rotation", NULL, 1, &rotation_cb},
        RG_DIALOG_CHOICE_LAST
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);

    // the HANDY_SCREEN_WIDTH * HANDY_SCREEN_WIDTH is deliberate because of rotation
    updates[0].buffer = (void*)rg_alloc(HANDY_SCREEN_WIDTH * HANDY_SCREEN_WIDTH * 2, MEM_FAST);
    updates[1].buffer = (void*)rg_alloc(HANDY_SCREEN_WIDTH * HANDY_SCREEN_WIDTH * 2, MEM_FAST);

    // The Lynx has a variable framerate but 60 is typical
    app->refreshRate = 60;

    // Init emulator
    lynx = new CSystem(app->romPath, MIKIE_PIXEL_FORMAT_16BPP_565_BE, AUDIO_SAMPLE_RATE);

    if (lynx->mFileType == HANDY_FILETYPE_ILLEGAL)
    {
        RG_PANIC("ROM loading failed!");
    }

    gPrimaryFrameBuffer = (UBYTE*)currentUpdate->buffer;
    gAudioBuffer = (SWORD*)&audioBuffer;
    gAudioEnabled = 1;

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        rg_emu_load_state(app->saveSlot);
    }

    set_display_mode();

    float sampleTime = AUDIO_SAMPLE_RATE / 1000000.f;
    long skipFrames = 0;
    bool fullFrame = 0;

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
            sampleTime = AUDIO_SAMPLE_RATE / 1000000.f * app->speed;
            rg_audio_set_sample_rate(app->sampleRate * app->speed);
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
            rg_video_update_t *previousUpdate = &updates[currentUpdate == &updates[0]];

            fullFrame = rg_display_queue_update(currentUpdate, previousUpdate) == RG_UPDATE_FULL;

            currentUpdate = previousUpdate;
            gPrimaryFrameBuffer = (UBYTE*)currentUpdate->buffer;
        }

        int elapsed = rg_system_timer() - startTime;

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            if (app->speed > 1.f)
                skipFrames += (int)app->speed * 2;
            // The Lynx uses a variable framerate so we use the count of generated audio samples as reference instead
            else if (elapsed > ((gAudioBufferPointer/2) * sampleTime))
                skipFrames += 1;
            else if (drawFrame && fullFrame) // This could be avoided when scaling != full
                skipFrames += 1;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        rg_system_tick(elapsed);

        rg_audio_submit(audioBuffer, gAudioBufferPointer >> 1);
        gAudioBufferPointer = 0;
    }
}
