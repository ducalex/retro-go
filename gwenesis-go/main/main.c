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
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 + 1)

static int16_t audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_video_update_t updates[2];
static rg_video_update_t *currentUpdate = &updates[0];

static rg_app_t *app;

static bool yfm_enabled = true;
static bool z80_enabled = true;

static const char *SETTING_YFM_EMULATION = "yfm_enable";
static const char *SETTING_Z80_EMULATION = "z80_enable";
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

static rg_gui_event_t z80_update_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        z80_enabled = !z80_enabled;
        rg_settings_set_number(NS_APP, SETTING_Z80_EMULATION, z80_enabled);
    }
    strcpy(option->value, z80_enabled ? "On " : "Off");

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
        {2, "Z80 emulation", "On", 1, &z80_update_cb},
        RG_DIALOG_CHOICE_LAST
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);

    yfm_enabled = rg_settings_get_number(NS_APP, SETTING_YFM_EMULATION, 1);
    z80_enabled = rg_settings_get_number(NS_APP, SETTING_Z80_EMULATION, 1);

    VRAM = rg_alloc(VRAM_MAX_SIZE, MEM_FAST);

    updates[0].buffer = rg_alloc(320 * 240 * 2, MEM_FAST);
    updates[1].buffer = rg_alloc(320 * 240 * 2, MEM_FAST);

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
    extern uint64_t zclk;

    uint32_t keymap[8] = {RG_KEY_UP, RG_KEY_DOWN, RG_KEY_LEFT, RG_KEY_RIGHT, RG_KEY_A, RG_KEY_B, RG_KEY_SELECT, RG_KEY_START};
    uint32_t samples_per_frame = AUDIO_SAMPLE_RATE / 60;
    uint32_t samples_per_line = AUDIO_SAMPLE_RATE / 60 / 262;
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
        int audio_index = 0;

        screen_width = REG12_MODE_H40 ? 320 : 256;
        screen_height = 224;

        gwenesis_vdp_set_buffer(currentUpdate->buffer + (320 - screen_width)); // / 2 * 2
        gwenesis_vdp_render_config();

        if (!z80_enabled) // This will prevent z80_run/z80_sync from doing anything
            zclk = system_clock + 262 * VDP_CYCLES_PER_LINE;

        if (!yfm_enabled)
            audio_index = samples_per_frame;

        for (scan_line = 0; scan_line < 262; scan_line++)
        {
            m68k_run(system_clock + VDP_CYCLES_PER_LINE);
            z80_run(system_clock + VDP_CYCLES_PER_LINE);

            if (drawFrame && scan_line < screen_height)
                gwenesis_vdp_render_line(scan_line);

            // This is essentially magic to get close enough. Not accurate at all.
            if (audio_index < samples_per_frame)
            {
                int audio_step = (scan_line & 1) ? 4 : 3;
                YM2612Update(&audioBuffer[audio_index * 2], audio_step);
                audio_index += audio_step;
            }

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
                z80_irq_line(1);
            }
            else if (scan_line == screen_height)
            {
                z80_irq_line(0);
            }

            system_clock += VDP_CYCLES_PER_LINE;
        }

        if (drawFrame)
        {
            rg_video_update_t *previousUpdate = &updates[currentUpdate == &updates[0]];
            rg_display_queue_update(currentUpdate, NULL);
            currentUpdate = previousUpdate;
        }

        int elapsed = get_elapsed_time_since(startTime);
        rg_system_tick(elapsed);

        if (yfm_enabled)
            rg_audio_submit(audioBuffer, audio_index);
    }
}
