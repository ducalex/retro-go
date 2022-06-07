#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <rg_system.h>

/* Gwenesis Emulator */
#include "m68k.h"
#include "z80inst.h"
#include "ym2612.h"
#include "gwenesis_bus.h"
#include "gwenesis_io.h"
#include "gwenesis_vdp.h"

uint8_t *ROM_DATA;
size_t ROM_DATA_LENGTH;
unsigned char *VRAM;
unsigned int scan_line;
uint64_t m68k_clock;

#define AUDIO_SAMPLE_RATE (53267)
#define AUDIO_BUFFER_LENGTH (512)

static int16_t audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];

static rg_app_t *app;

#define WAIT_FOR_Z80_RUN() while (uxQueueMessagesWaiting(sound_task_run));
static QueueHandle_t sound_task_run;

static bool yfm_enabled = true;
static bool yfm_resample = true;

static const char *SETTING_YFM_EMULATION = "yfm_enable";
static const char *SETTING_YFM_RESAMPLE = "sampling";
// --- MAIN


void gwenesis_io_get_buttons()
{
}

static inline void m68k_run(uint64_t target)
{
    m68k_execute((target - m68k_clock) / M68K_FREQ_DIVISOR);
    m68k_clock = target;
}

static rg_gui_event_t yfm_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        yfm_enabled = !yfm_enabled;
        rg_settings_set_number(NS_APP, SETTING_YFM_EMULATION, yfm_enabled);
    }
    strcpy(option->value, yfm_enabled ? "On " : "Off");

    return RG_DIALOG_VOID;
}

static rg_gui_event_t sampling_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        yfm_resample = !yfm_resample;
        rg_settings_set_number(NS_APP, SETTING_YFM_RESAMPLE, yfm_resample);
        rg_audio_set_sample_rate(yfm_resample ? 26634 : 53267);
    }
    strcpy(option->value, yfm_resample ? "On " : "Off");

    return RG_DIALOG_VOID;
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_display_save_frame(filename, currentUpdate, width, height);
}

static bool save_state_handler(const char *filename)
{
    return false;
}

static bool load_state_handler(const char *filename)
{
    reset_emulation();
    return false;
}

static bool reset_handler(bool hard)
{
    reset_emulation();
    return true;
}

static void sound_task(void *arg)
{
    sound_task_run = xQueueCreate(1, sizeof(uint64_t));

    uint64_t system_clock;
    size_t audio_index = 0;

    while (true)
    {
        xQueuePeek(sound_task_run, &system_clock, portMAX_DELAY);
        z80_run(system_clock);
        xQueueReceive(sound_task_run, &system_clock, portMAX_DELAY);

        if (!yfm_enabled)
            continue;

        // This is essentially magic to get close enough. Not accurate at all.
        size_t audio_step = 3 + (scan_line & 1);
        YM2612Update(&audioBuffer[audio_index * 2], audio_step);
        audio_index += audio_step;

        // Submit at end of frame or whenever the buffer is full
        if (scan_line == 261 || audio_index >= AUDIO_BUFFER_LENGTH - 4)
        {
            int carry = 0;

            if (yfm_resample > 0)
            {
                // Resampling deals with even number of samples, carry what's left
                carry = (audio_index & 1) ? (audio_index - 1) : 0;
                for (size_t i = 0; i < audio_index - 1; ++i)
                {
                    audioBuffer[i] = (audioBuffer[i*2] + audioBuffer[(i+1)*2]) >> 1;
                }
                audio_index >>= 1;
            }

            rg_audio_submit(audioBuffer, audio_index);
            audio_index = 0;

            if (carry)
            {
                audioBuffer[0] = audioBuffer[carry*2-1];
                audioBuffer[1] = audioBuffer[carry*2-0];
                audio_index = 1;
            }
        }
    }

    vQueueDelete(sound_task_run);
    sound_task_run = NULL;

    vTaskDelete(NULL);
}

#if 0
void z80_sync(void)
{
    uint64_t target = m68k_cycles_run() * M68K_FREQ_DIVISOR + m68k_clock;
    extern uint64_t zclk;
    if (zclk >= target)
        return;
    // WAIT_FOR_Z80_RUN(); // m68k_run might call z80_sync, we don't want to conflict
    z80_run(target);
}
#endif

void app_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
    };
    const rg_gui_option_t options[] = {
        {1, "YFM emulation", "On", 1, &yfm_update_cb},
        {2, "Down sampling", "On", 1, &sampling_update_cb},
        RG_DIALOG_CHOICE_LAST
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);

    yfm_enabled = rg_settings_get_number(NS_APP, SETTING_YFM_EMULATION, 1);
    yfm_resample = rg_settings_get_number(NS_APP, SETTING_YFM_RESAMPLE, 1);

    VRAM = rg_alloc(VRAM_MAX_SIZE, MEM_FAST);

    updates[0].buffer = rg_alloc(320 * 240 * 2, MEM_FAST);
    // updates[1].buffer = rg_alloc(320 * 240 * 2, MEM_FAST);

    xTaskCreatePinnedToCore(&sound_task, "gen_sound", 2048, NULL, 7, NULL, 1);
    rg_audio_set_sample_rate(yfm_resample ? 26634 : 53267);

    RG_LOGI("Genesis start\n");

    FILE *fp = fopen(app->romPath, "rb");
    if (fp)
    {
        ROM_DATA = rg_alloc(0x300000, MEM_SLOW);
        ROM_DATA_LENGTH = fread(ROM_DATA, 1, 0x300000, fp);
        for (size_t i = 0; i < ROM_DATA_LENGTH; i += 2)
        {
            char z = ROM_DATA[i];
            ROM_DATA[i] = ROM_DATA[i + 1];
            ROM_DATA[i + 1] = z;
        }
        fclose(fp);
        RG_LOGI("ROM SIZE = %d\n", ROM_DATA_LENGTH);
    } else {
        RG_PANIC("Rom load failed");
    }

    extern unsigned char gwenesis_vdp_regs[0x20];
    extern unsigned int gwenesis_vdp_status;
    extern unsigned int screen_width, screen_height;
    extern int mode_pal;
    extern int hint_pending;

    uint32_t keymap[8] = {RG_KEY_UP, RG_KEY_DOWN, RG_KEY_LEFT, RG_KEY_RIGHT, RG_KEY_A, RG_KEY_B, RG_KEY_SELECT, RG_KEY_START};
    uint32_t joystick = 0, joystick_old;
    uint64_t system_clock = 0;
    uint32_t frames = 0;

    RG_LOGI("load_cartridge()\n");
    load_cartridge();

    RG_LOGI("power_on()\n");
    power_on();

    RG_LOGI("reset_emulation()\n");
    reset_emulation();

    RG_LOGI("rg_display_set_source_format()\n");
    rg_display_set_source_format(320, mode_pal ? 240 : 224, 0, 0, 320 * 2, RG_PIXEL_565_LE);

    RG_LOGI("emulation loop\n");
    while (true)
    {
        joystick_old = joystick;
        joystick = rg_input_read_gamepad();

        if (joystick & (RG_KEY_MENU | RG_KEY_OPTION))
        {
            if (joystick & RG_KEY_MENU)
                rg_gui_game_menu();
            else
                rg_gui_options_menu();
        }
        else if (joystick != joystick_old)
        {
            for (int i = 0; i < 8; i++)
            {
                if ((joystick & keymap[i]) == keymap[i])
                    gwenesis_io_pad_press_button(0, i);
                else
                    gwenesis_io_pad_release_button(0, i);
            }
        }

        int64_t startTime = get_elapsed_time();
        bool drawFrame = (frames++ & 3) == 3;

        int hint_counter = gwenesis_vdp_regs[10];

        screen_width = REG12_MODE_H40 ? 320 : 256;
        screen_height = 224;

        gwenesis_vdp_set_buffer(currentUpdate->buffer + (320 - screen_width)); // / 2 * 2
        gwenesis_vdp_render_config();

        for (scan_line = 0; scan_line < 262; scan_line++)
        {
            system_clock += VDP_CYCLES_PER_LINE;

            WAIT_FOR_Z80_RUN(); // m68k_run might call z80_sync, we don't want to conflict
            m68k_run(system_clock);
            xQueueSend(sound_task_run, &system_clock, portMAX_DELAY);

            if (drawFrame && scan_line < screen_height)
                gwenesis_vdp_render_line(scan_line);

            // On these lines, the line counter interrupt is reloaded
            if ((scan_line == 0) || (scan_line > screen_height))
                hint_counter = REG10_LINE_COUNTER;

            // interrupt line counter
            if (--hint_counter < 0)
            {
                if (REG0_LINE_INTERRUPT && (scan_line <= screen_height))
                {
                    hint_pending = 1;
                    if ((gwenesis_vdp_status & STATUS_VIRQPENDING) == 0)
                        m68k_set_irq(4);
                }
                hint_counter = REG10_LINE_COUNTER;
            }

            // vblank begin at the end of last rendered line
            if (scan_line == (screen_height - 1))
            {
                if (REG1_VBLANK_INTERRUPT)
                {
                    gwenesis_vdp_status |= STATUS_VIRQPENDING;
                    m68k_set_irq(6);
                }
                WAIT_FOR_Z80_RUN();
                z80_irq_line(1);
            }
            else if (scan_line == screen_height)
            {
                WAIT_FOR_Z80_RUN();
                z80_irq_line(0);
            }
        }

        if (drawFrame)
        {
            // rg_video_update_t *previousUpdate = &updates[currentUpdate == &updates[0]];
            rg_display_queue_update(currentUpdate, NULL);
            // currentUpdate = previousUpdate;
        }

        int elapsed = get_elapsed_time_since(startTime);
        rg_system_tick(elapsed);
    }
}
