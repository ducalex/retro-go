extern "C" {
#include <freertos/FreeRTOS.h>
#include <odroid_system.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
}

#include <handy.h>

#define APP_ID 50

#define AUDIO_SAMPLE_RATE 32000
#define AUDIO_BUFFER_LENGTH ((AUDIO_SAMPLE_RATE / 60 + 25) * 2)

#define PIXEL_MASK 0xFF
#define PAL_SHIFT_MASK 0x00

static DMA_ATTR short audioBuffer[AUDIO_BUFFER_LENGTH];

static odroid_video_frame update1;
static odroid_video_frame update2;
static odroid_video_frame *currentUpdate = &update1;
static odroid_video_frame *previousUpdate = &update2;

static CSystem *lynx = NULL;
// static bool netplay = false;
// --- MAIN


static bool save_state(char *pathName)
{
    return true;
}


static bool load_state(char *pathName)
{
    lynx->Reset();
    return true;
}


extern "C" void app_main(void)
{
    printf("\n========================================\n\n");
    printf("Handy-go (%s).\n", PROJECT_VER);

    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&load_state, &save_state, NULL);

    update1.width = update2.width = HANDY_SCREEN_WIDTH;
    update1.height = update2.height = HANDY_SCREEN_HEIGHT + 2;
    update1.stride = update2.stride = HANDY_SCREEN_WIDTH * 2;
    update1.pixel_size = update2.pixel_size = 2;
    update1.pixel_clear = update2.pixel_clear = -1;

    update1.buffer = (void*)rg_alloc(update1.stride * update1.height, MEM_FAST);
    update2.buffer = (void*)rg_alloc(update2.stride * update2.height, MEM_FAST);

    const char *romFile = odroid_system_get_rom_path();

    // Init emulator
    lynx = new CSystem(romFile);
    lynx->DisplaySetAttributes(MIKIE_NO_ROTATE, MIKIE_PIXEL_FORMAT_16BPP_565_BE, update1.stride);

    gPrimaryFrameBuffer = (UBYTE*)currentUpdate->buffer + currentUpdate->stride;
    gAudioBuffer = (UBYTE*)&audioBuffer;
    gAudioEnabled = 1;

    if (odroid_system_get_start_action() == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }

    odroid_gamepad_state joystick;

    uint frameTime = get_frame_time(60);
    uint skipFrames = 0;
    bool fullFrame = 0;

    // Start emulation
    while (1)
    {
        odroid_input_gamepad_read(&joystick);

        if (joystick.values[ODROID_INPUT_MENU]) {
            odroid_overlay_game_menu();
        }
        else if (joystick.values[ODROID_INPUT_VOLUME]) {
            odroid_dialog_choice_t options[] = {
                {100, "Rotation", "Off  ", 0, NULL},
                ODROID_DIALOG_CHOICE_LAST
            };
            odroid_overlay_game_settings_menu(options);
        }

        uint startTime = get_elapsed_time();
        bool drawFrame = !skipFrames;

        ULONG buttons = 0;

    	if (joystick.values[ODROID_INPUT_UP])     buttons |= BUTTON_UP;
    	if (joystick.values[ODROID_INPUT_DOWN])   buttons |= BUTTON_DOWN;
    	if (joystick.values[ODROID_INPUT_LEFT])   buttons |= BUTTON_LEFT;
    	if (joystick.values[ODROID_INPUT_RIGHT])  buttons |= BUTTON_RIGHT;
    	if (joystick.values[ODROID_INPUT_A])      buttons |= BUTTON_A;
    	if (joystick.values[ODROID_INPUT_B])      buttons |= BUTTON_B;
    	if (joystick.values[ODROID_INPUT_START])  buttons |= BUTTON_PAUSE; // BUTTON_OPT1
    	if (joystick.values[ODROID_INPUT_SELECT]) buttons |= BUTTON_OPT2;

        lynx->SetButtonData(buttons);

        lynx->UpdateFrame(drawFrame);

        if (drawFrame)
        {
            fullFrame = odroid_display_queue_update(currentUpdate, previousUpdate) == SCREEN_UPDATE_FULL;
            previousUpdate = currentUpdate;
            currentUpdate = (currentUpdate == &update1) ? &update2 : &update1;
            gPrimaryFrameBuffer = (UBYTE*)currentUpdate->buffer + currentUpdate->stride;
        }

        // See if we need to skip a frame to keep up
        if (skipFrames == 0)
        {
            if (get_elapsed_time_since(startTime) > frameTime) skipFrames += 1;
            if (speedupEnabled) skipFrames += speedupEnabled * 2.5;
        }
        else if (skipFrames > 0)
        {
            skipFrames--;
        }

        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));

        if (!speedupEnabled && gAudioBufferPointer > 0)
        {
            odroid_audio_submit(audioBuffer, gAudioBufferPointer / 4);
            gAudioBufferPointer = 0;
        }
    }

    printf("Handy died.\n");
    abort();
}
