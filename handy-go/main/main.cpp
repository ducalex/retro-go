extern "C" {
#include <freertos/FreeRTOS.h>
#include <odroid_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

#include <handy.h>

#define APP_ID 50

#define AUDIO_SAMPLE_RATE   (HANDY_AUDIO_SAMPLE_FREQ)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 40)

static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static odroid_video_frame_t update1;
static odroid_video_frame_t update2;
static odroid_video_frame_t *currentUpdate = &update1;
static odroid_video_frame_t *previousUpdate = &update2;

static CSystem *lynx = NULL;

static short dpad_mapped_up;
static short dpad_mapped_down;
static short dpad_mapped_left;
static short dpad_mapped_right;
// static bool netplay = false;
// --- MAIN

static void set_rotation()
{
    odroid_display_rotation_t rotation = odroid_display_get_rotation();

    if (rotation == ODROID_DISPLAY_ROTATION_AUTO)
    {
        rotation = ODROID_DISPLAY_ROTATION_OFF;

        switch (lynx->mCart->CRC32())
        {
            case 0x97501709: // Centipede
            case 0x0271B6E9: // Lexis
            case 0x006FD398: // NFL Football
            case 0xBCD10C3A: // Raiden
                rotation = ODROID_DISPLAY_ROTATION_LEFT;
                break;
            case 0x7F0EC7AD: // Gauntlet
            case 0xAC564BAA: // Gauntlet - The Third Encounter
            case 0xA53649F1: // Klax
                rotation = ODROID_DISPLAY_ROTATION_RIGHT;
                break;
            default:
                if (lynx->mCart->CartGetRotate() == CART_ROTATE_LEFT)
                    rotation = ODROID_DISPLAY_ROTATION_LEFT;
                if (lynx->mCart->CartGetRotate() == CART_ROTATE_RIGHT)
                    rotation = ODROID_DISPLAY_ROTATION_RIGHT;
        }
    }

    switch(rotation)
    {
        case ODROID_DISPLAY_ROTATION_LEFT:
            update1.width = update2.width = HANDY_SCREEN_HEIGHT;
            update1.height = update2.height = HANDY_SCREEN_WIDTH;
            lynx->mMikie->SetRotation(MIKIE_ROTATE_L);
            dpad_mapped_up    = BUTTON_RIGHT;
            dpad_mapped_down  = BUTTON_LEFT;
            dpad_mapped_left  = BUTTON_UP;
            dpad_mapped_right = BUTTON_DOWN;
            break;
        case ODROID_DISPLAY_ROTATION_RIGHT:
            update1.width = update2.width = HANDY_SCREEN_HEIGHT;
            update1.height = update2.height = HANDY_SCREEN_WIDTH;
            lynx->mMikie->SetRotation(MIKIE_ROTATE_R);
            dpad_mapped_up    = BUTTON_LEFT;
            dpad_mapped_down  = BUTTON_RIGHT;
            dpad_mapped_left  = BUTTON_DOWN;
            dpad_mapped_right = BUTTON_UP;
            break;
        default:
            update1.width = update2.width = HANDY_SCREEN_WIDTH;
            update1.height = update2.height = HANDY_SCREEN_HEIGHT;
            lynx->mMikie->SetRotation(MIKIE_NO_ROTATE);
            dpad_mapped_up    = BUTTON_UP;
            dpad_mapped_down  = BUTTON_DOWN;
            dpad_mapped_left  = BUTTON_LEFT;
            dpad_mapped_right = BUTTON_RIGHT;
            break;
    }
}


static bool rotation_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int rotation = (int)odroid_display_get_rotation();

    if (event == ODROID_DIALOG_PREV) {
        if (--rotation < 0) rotation = ODROID_DISPLAY_ROTATION_COUNT - 1;
        odroid_display_set_rotation((odroid_display_rotation_t)rotation);
        set_rotation();
    }
    if (event == ODROID_DIALOG_NEXT) {
        if (++rotation > ODROID_DISPLAY_ROTATION_COUNT - 1) rotation = 0;
        odroid_display_set_rotation((odroid_display_rotation_t)rotation);
        set_rotation();
    }

    strcpy(option->value, "Off  ");
    if (rotation == ODROID_DISPLAY_ROTATION_AUTO)  strcpy(option->value, "Auto ");
    if (rotation == ODROID_DISPLAY_ROTATION_LEFT)  strcpy(option->value, "Left ");
    if (rotation == ODROID_DISPLAY_ROTATION_RIGHT) strcpy(option->value, "Right");

    return event == ODROID_DIALOG_ENTER;
}

static bool save_state(char *pathName)
{
    bool ret = false;
    FILE *fp;

    if ((fp = fopen(pathName, "wb")))
    {
        ret = lynx->ContextSave(fp);
        fclose(fp);
    }

    return ret;
}


static bool load_state(char *pathName)
{
    bool ret = false;
    FILE *fp;

    if ((fp = fopen(pathName, "rb")))
    {
        ret = lynx->ContextLoad(fp);
        fclose(fp);
    }

    if (!ret) lynx->Reset();

    return ret;
}


extern "C" void app_main(void)
{
    heap_caps_malloc_extmem_enable(32 * 1024);
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&load_state, &save_state, NULL);

    update1.width = update2.width = HANDY_SCREEN_WIDTH;
    update1.height = update2.height = HANDY_SCREEN_WIDTH;
    update1.stride = update2.stride = HANDY_SCREEN_WIDTH * 2;
    update1.pixel_size = update2.pixel_size = 2;
    update1.pixel_clear = update2.pixel_clear = -1;

    update1.buffer = (void*)rg_alloc(update1.stride * update1.height, MEM_FAST);
    update2.buffer = (void*)rg_alloc(update2.stride * update2.height, MEM_FAST);

    rg_app_desc_t *app = odroid_system_get_app();

    // Init emulator
    lynx = new CSystem(app->romPath, MIKIE_PIXEL_FORMAT_16BPP_565_BE, AUDIO_SAMPLE_RATE);

    gPrimaryFrameBuffer = (UBYTE*)currentUpdate->buffer;
    gAudioBuffer = (SWORD*)&audioBuffer;
    gAudioEnabled = 1;

    if (app->startAction == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }

    set_rotation();

    odroid_gamepad_state_t joystick;

    float sampleTime = AUDIO_SAMPLE_RATE / 1000000.f;
    uint skipFrames = 0;
    bool fullFrame = 0;

    // Start emulation
    while (1)
    {
        odroid_input_read_gamepad(&joystick);

        if (joystick.values[ODROID_INPUT_MENU]) {
            odroid_overlay_game_menu();
        }
        else if (joystick.values[ODROID_INPUT_VOLUME]) {
            odroid_dialog_choice_t options[] = {
                {100, "Rotation", "Auto", 1, &rotation_cb},
                ODROID_DIALOG_CHOICE_LAST
            };
            odroid_overlay_game_settings_menu(options);
        }

        uint startTime = get_elapsed_time();
        bool drawFrame = !skipFrames;

        ULONG buttons = 0;

    	if (joystick.values[ODROID_INPUT_UP])     buttons |= dpad_mapped_up;
    	if (joystick.values[ODROID_INPUT_DOWN])   buttons |= dpad_mapped_down;
    	if (joystick.values[ODROID_INPUT_LEFT])   buttons |= dpad_mapped_left;
    	if (joystick.values[ODROID_INPUT_RIGHT])  buttons |= dpad_mapped_right;
    	if (joystick.values[ODROID_INPUT_A])      buttons |= BUTTON_A;
    	if (joystick.values[ODROID_INPUT_B])      buttons |= BUTTON_B;
    	if (joystick.values[ODROID_INPUT_START])  buttons |= BUTTON_OPT2; // BUTTON_PAUSE
    	if (joystick.values[ODROID_INPUT_SELECT]) buttons |= BUTTON_OPT1;

        lynx->SetButtonData(buttons);

        lynx->UpdateFrame(drawFrame);

        if (drawFrame)
        {
            fullFrame = odroid_display_queue_update(currentUpdate, previousUpdate) == SCREEN_UPDATE_FULL;
            previousUpdate = currentUpdate;
            currentUpdate = (currentUpdate == &update1) ? &update2 : &update1;
            gPrimaryFrameBuffer = (UBYTE*)currentUpdate->buffer;
        }

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            // The Lynx uses a variable framerate so we use the count of generated audio samples as reference instead
            if (get_elapsed_time_since(startTime) > ((gAudioBufferPointer/2) * sampleTime)) skipFrames += 1;
            if (app->speedupEnabled) skipFrames += app->speedupEnabled * 2.5;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));

        if (!app->speedupEnabled)
        {
            odroid_audio_submit(gAudioBuffer, gAudioBufferPointer >> 1);
            gAudioBufferPointer = 0;
        }
    }

    RG_PANIC("Handy died.");
}
