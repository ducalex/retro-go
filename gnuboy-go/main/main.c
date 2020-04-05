#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "string.h"

#include "../components/gnuboy/loader.h"
#include "../components/gnuboy/hw.h"
#include "../components/gnuboy/lcd.h"
#include "../components/gnuboy/fb.h"
#include "../components/gnuboy/cpu.h"
#include "../components/gnuboy/mem.h"
#include "../components/gnuboy/sound.h"
#include "../components/gnuboy/pcm.h"
#include "../components/gnuboy/regs.h"
#include "../components/gnuboy/rtc.h"
#include "../components/gnuboy/defs.h"

#include "odroid_system.h"

#define APP_ID 20

#define AUDIO_SAMPLE_RATE (32000)
// Note: Magic number obtained by adjusting until audio buffer overflows stop.
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 10 + 1)

#define GB_WIDTH (160)
#define GB_HEIGHT (144)

#define NVS_KEY_SAVE_SRAM "sram"

struct fb fb;
struct pcm pcm;

int16_t* audioBuffer;

static odroid_video_frame update1 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, NULL, NULL, 0};
static odroid_video_frame update2 = {GB_WIDTH, GB_HEIGHT, GB_WIDTH * 2, 2, 0xFF, NULL, NULL, 0};
static odroid_video_frame *currentUpdate = &update1;

static uint totalElapsedTime = 0;
static uint emulatedFrames = 0;
static uint skippedFrames = 0;
static uint fullFrames = 0;

static bool skipFrame = true;
static bool netplay = false;
static bool saveSRAM = false;

static int sramSaveTimer = 0;

extern int debug_trace;
// --- MAIN


void run_to_vblank()
{
    fb.enabled = !skipFrame;
    pcm.pos = 0;

    /* FIXME: djudging by the time specified this was intended
    to emulate through vblank phase which is handled at the
    end of the loop. */
    cpu_emulate(2280);

    /* FIXME: R_LY >= 0; comparsion to zero can also be removed
    altogether, R_LY is always 0 at this point */
    while (R_LY > 0 && R_LY < 144)
    {
        /* Step through visible line scanning phase */
        emu_step();
    }

    /* VBLANK BEGIN */
    if (!skipFrame)
    {
        odroid_video_frame *previousUpdate = (currentUpdate == &update1) ? &update2 : &update1;

        if (odroid_display_queue_update(currentUpdate, previousUpdate) == SCREEN_UPDATE_FULL)
        {
            ++fullFrames;
        }

        // swap buffers
        currentUpdate = previousUpdate;
        fb.ptr = currentUpdate->buffer;
    }

    rtc_tick();

    sound_mix();

    // pcm_submit();

    if (!(R_LCDC & 0x80)) {
        /* LCDC operation stopped */
        /* FIXME: djudging by the time specified, this is
        intended to emulate through visible line scanning
        phase, even though we are already at vblank here */
        cpu_emulate(32832);
    }

    while (R_LY > 0) {
        /* Step through vblank phase */
        emu_step();
    }
}


static bool SaveState(char *pathName)
{
    return state_save(pathName) == 0;
}

static bool LoadState(char *pathName)
{
    if (state_load(pathName) != 0)
    {
        emu_reset();

        if (saveSRAM) sram_load();

        return false;
    }
    return true;
}


static bool palette_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int pal = pal_get_dmg();
    int max = pal_count_dmg();

    if (event == ODROID_DIALOG_PREV) {
        pal = pal > 0 ? pal - 1 : max;
    }

    if (event == ODROID_DIALOG_NEXT) {
        pal = pal < max ? pal + 1 : 0;
    }

    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        odroid_settings_Palette_set(pal);
        pal_set_dmg(pal);
        run_to_vblank();
    }

    if (pal == 0) strcpy(option->value, "GBC");
    else sprintf(option->value, "%d/%d", pal, max);

    return event == ODROID_DIALOG_ENTER;
}

static bool save_sram_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        saveSRAM = !saveSRAM;
        odroid_settings_app_int32_set(NVS_KEY_SAVE_SRAM, saveSRAM);
    }

    strcpy(option->value, saveSRAM ? "Yes" : "No");

    return event == ODROID_DIALOG_ENTER;
}

static bool rtc_t_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (option->id == 'd') {
        if (event == ODROID_DIALOG_PREV && --rtc.d < 0) rtc.d = 364;
        if (event == ODROID_DIALOG_NEXT && ++rtc.d > 364) rtc.d = 0;
        sprintf(option->value, "%03d", rtc.d);
    }
    if (option->id == 'h') {
        if (event == ODROID_DIALOG_PREV && --rtc.h < 0) rtc.h = 23;
        if (event == ODROID_DIALOG_NEXT && ++rtc.h > 23) rtc.h = 0;
        sprintf(option->value, "%02d", rtc.h);
    }
    if (option->id == 'm') {
        if (event == ODROID_DIALOG_PREV && --rtc.m < 0) rtc.m = 59;
        if (event == ODROID_DIALOG_NEXT && ++rtc.m > 59) rtc.m = 0;
        sprintf(option->value, "%02d", rtc.m);
    }
    if (option->id == 's') {
        if (event == ODROID_DIALOG_PREV && --rtc.s < 0) rtc.s = 59;
        if (event == ODROID_DIALOG_NEXT && ++rtc.s > 59) rtc.s = 0;
        sprintf(option->value, "%02d", rtc.s);
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool rtc_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    if (event == ODROID_DIALOG_ENTER) {
        static odroid_dialog_choice_t choices[] = {
            {'d', "Day", "000", 1, &rtc_t_update_cb},
            {'h', "Hour", "00", 1, &rtc_t_update_cb},
            {'m', "Min",  "00", 1, &rtc_t_update_cb},
            {'s', "Sec",  "00", 1, &rtc_t_update_cb},
            ODROID_DIALOG_CHOICE_LAST
        };
        odroid_overlay_dialog("Set Clock", choices, 0);
    }
    sprintf(option->value, "%02d:%02d", rtc.h, rtc.m);
    return false;
}

static bool advanced_settings_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
   if (event == ODROID_DIALOG_ENTER) {
      odroid_dialog_choice_t options[] = {
        {101, "Set clock", "00:00", 1, &rtc_update_cb},
        {102, "Save SRAM", "No", 1, &save_sram_update_cb},
        ODROID_DIALOG_CHOICE_LAST
      };
      odroid_overlay_dialog("Advanced", options, 0);
   }
   return false;
}

void app_main(void)
{
    printf("gnuboy (%s-%s).\n", COMPILEDATE, GITREV);

    // Do before odroid_system_init to make sure we get the caps requested
    update1.buffer = heap_caps_calloc(GB_WIDTH * GB_HEIGHT, 2, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    update2.buffer = heap_caps_calloc(GB_WIDTH * GB_HEIGHT, 2, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);
    audioBuffer    = heap_caps_calloc(AUDIO_BUFFER_LENGTH * 2, 2, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);

    // Init all the console hardware
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    // Do the check after screen init so we can display an error
    if (!update1.buffer || !update2.buffer || !audioBuffer)
    {
        odroid_system_panic("Buffer allocation failed.");
    }

    saveSRAM = odroid_settings_app_int32_get(NVS_KEY_SAVE_SRAM, 0);

    // Load ROM
    char *romPath = odroid_system_get_path(NULL, ODROID_PATH_ROM_FILE);
    loader_init(romPath);

    // RTC
    memset(&rtc, 0, sizeof(rtc));

    // Video
    memset(&fb, 0, sizeof(fb));
    fb.w = GB_WIDTH;
  	fb.h = GB_HEIGHT;
  	fb.pelsize = 2;
  	fb.pitch = fb.w * fb.pelsize;
  	fb.indexed = 0;
  	fb.ptr = currentUpdate->buffer;
  	fb.enabled = 1;
  	fb.dirty = 0;
    fb.byteorder = 1;

    // Audio
    memset(&pcm, 0, sizeof(pcm));
    pcm.hz = AUDIO_SAMPLE_RATE;
  	pcm.stereo = 1;
  	pcm.len = AUDIO_BUFFER_LENGTH; // count of 16bit samples (x2 for stereo)
  	pcm.buf = audioBuffer;
  	pcm.pos = 0;

    emu_reset();

    lcd_begin();

    pal_set_dmg(odroid_settings_Palette_get());

    if (odroid_system_get_start_action() == ODROID_START_ACTION_RESUME)
    {
        odroid_system_emu_load_state(0);
    }
    else if (saveSRAM)
    {
        sram_load();
    }

    const int frameTime = get_frame_time(60);

    while (true)
    {
        odroid_gamepad_state joystick;
        odroid_input_gamepad_read(&joystick);

        if (joystick.values[ODROID_INPUT_MENU]) {
            odroid_overlay_game_menu();
        }
        else if (joystick.values[ODROID_INPUT_VOLUME]) {
            odroid_dialog_choice_t options[] = {
                {100, "Palette", "7/7", !hw.cgb, &palette_update_cb},
                {101, "More...", "", 1, &advanced_settings_cb},
                ODROID_DIALOG_CHOICE_LAST
            };
            odroid_overlay_game_settings_menu(options);
        }

        uint startTime = get_elapsed_time();

        pad_set(PAD_UP, joystick.values[ODROID_INPUT_UP]);
        pad_set(PAD_RIGHT, joystick.values[ODROID_INPUT_RIGHT]);
        pad_set(PAD_DOWN, joystick.values[ODROID_INPUT_DOWN]);
        pad_set(PAD_LEFT, joystick.values[ODROID_INPUT_LEFT]);
        pad_set(PAD_SELECT, joystick.values[ODROID_INPUT_SELECT]);
        pad_set(PAD_START, joystick.values[ODROID_INPUT_START]);
        pad_set(PAD_A, joystick.values[ODROID_INPUT_A]);
        pad_set(PAD_B, joystick.values[ODROID_INPUT_B]);

        if (skipFrame) {
            ++skippedFrames;
        }

        run_to_vblank();

        if (saveSRAM)
        {
            if (ram.sram_dirty)
            {
                sramSaveTimer = 90;
                ram.sram_dirty = 0;
            }

            if (sramSaveTimer > 0 && --sramSaveTimer == 0)
            {
                sram_save();
            }
        }

        skipFrame = !skipFrame && get_elapsed_time_since(startTime) > frameTime;

        if (speedupEnabled) {
            skipFrame = emulatedFrames % (speedupEnabled * 4);
        } else {
            odroid_audio_submit(pcm.buf, pcm.pos >> 1);
        }

        totalElapsedTime += get_elapsed_time_since(startTime);
        ++emulatedFrames;

        if (emulatedFrames == 60)
        {
            odroid_system_print_stats(totalElapsedTime, emulatedFrames, skippedFrames, fullFrames);

            emulatedFrames = skippedFrames = fullFrames = 0;
            totalElapsedTime = 0;
        }
    }
}
