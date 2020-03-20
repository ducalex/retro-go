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

static const int frameTime = (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000 / 60);

static char *romPath = NULL;

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

bool skipFrame = false;

extern int debug_trace;
// --- MAIN


int pcm_submit()
{
    if (!speedupEnabled) {
        odroid_audio_submit(pcm.buf, pcm.pos >> 1);
    }
    pcm.pos = 0;
    return 1;
}


void run_to_vblank()
{
    uint startTime = xthal_get_ccount();

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
    // printf("%d us\n", get_elapsed_time_since(startTime) / 240);

    /* VBLANK BEGIN */
    //vid_end();
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

    skipFrame = !skipFrame && get_elapsed_time_since(startTime) > frameTime;
    fb.enabled = !skipFrame;

    pcm_submit();
}


void SaveState()
{
    odroid_input_battery_monitor_enabled_set(0);
    odroid_system_set_led(1);
    odroid_display_lock();

    char* pathName = odroid_sdcard_get_savefile_path(romPath);
    if (!pathName) abort();

    FILE* f = fopen(pathName, "w");
    if (f == NULL)
    {
        printf("%s: fopen save failed\n", __func__);
        odroid_overlay_alert("Save failed");
    }
    else
    {
        savestate(f);
        rtc_save_internal(f);
        fclose(f);

        printf("%s: savestate OK.\n", __func__);
    }

    free(pathName);

    odroid_display_unlock();
    odroid_system_set_led(0);
    odroid_input_battery_monitor_enabled_set(1);
}

void LoadState()
{
    odroid_display_lock();
    char* pathName = odroid_sdcard_get_savefile_path(romPath);
    if (!pathName) abort();

    FILE* f = fopen(pathName, "r");
    if (f == NULL)
    {
        printf("LoadState: fopen load failed\n");
        odroid_display_unlock();
    }
    else
    {
        loadstate(f);
        rtc_load_internal(f);
        fclose(f);

        odroid_display_unlock();

        vram_dirty();
        pal_dirty();
        sound_dirty();
        mem_updatemap();

        printf("LoadState: loadstate OK.\n");
    }

    free(pathName);
}

void QuitEmulator(bool save)
{
    printf("QuitEmulator: stopping tasks.\n");

    odroid_audio_terminate();

    // odroid_display_queue_update(NULL);
    odroid_display_clear(0);

    odroid_display_lock();
    odroid_display_show_hourglass();
    odroid_display_unlock();

    if (save) {
        printf("QuitEmulator: Saving state.\n");
        SaveState();
    }

    // Set menu application
    odroid_system_set_boot_app(0);

    // Reset
    esp_restart();
}

static bool palette_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event)
{
    int pal = pal_get();
    int max = 7;

    if (event == ODROID_DIALOG_PREV) {
        pal = pal > 0 ? pal - 1 : max;
    }

    if (event == ODROID_DIALOG_NEXT) {
        pal = pal < max ? pal + 1 : 0;
    }

    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        odroid_settings_Palette_set(pal);
        pal_set(pal);
        // This is less than ideal, but it works for now
        odroid_display_unlock();
        run_to_vblank();
        odroid_display_lock();
    }

    sprintf(option->value, "%d/%d", pal, max);
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
        };
        odroid_overlay_dialog("Set Clock", choices, 4, 0);
    }
    sprintf(option->value, "%02d:%02d", rtc.h, rtc.m);
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
	odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE, &romPath);

    assert(update1.buffer && update2.buffer);
    assert(audioBuffer);

    // Load ROM
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

    pal_set(odroid_settings_Palette_get());

    // Load state
    if (startAction == ODROID_START_ACTION_RESUME)
    {
        LoadState();
    }

    int delayFrames = 0;

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
                {101, "Set clock", "00:00", mbc.type == MBC_MBC3, &rtc_update_cb},
            };
            odroid_overlay_game_settings_menu(options, 2);
        }
        // if (delayFrames == 0 && joystick.values[ODROID_INPUT_START]) {
        //     delayFrames = 1;
        // }
        // if (delayFrames > 0 && ++delayFrames == 15)
        // {
        //     debug_trace = 1;
        // }

        uint startTime = xthal_get_ccount();

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

        if (speedupEnabled) {
            skipFrame = emulatedFrames % (speedupEnabled * 4);
        }

        totalElapsedTime += get_elapsed_time_since(startTime);
        ++emulatedFrames;

        if (emulatedFrames == 60)
        {
            float seconds = totalElapsedTime / (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000.0f);
            float fps = emulatedFrames / seconds;

            odroid_battery_state battery;
            odroid_input_battery_level_read(&battery);

            printf("HEAP:%d, FPS:%f, SKIP:%d, FULL:%d, BATTERY:%d [%d]\n",
                esp_get_free_heap_size() / 1024, fps, skippedFrames, fullFrames,
                battery.millivolts, battery.percentage);

            emulatedFrames = 0;
            skippedFrames = 0;
            fullFrames = 0;
            totalElapsedTime = 0;
        }
    }
}
