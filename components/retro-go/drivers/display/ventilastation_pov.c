#include "ventilastation_pov.h"

#if RG_VENTILASTATION_POV_ENABLED

#include <math.h>
#include <string.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_attr.h>
#include <esp_err.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include "color_pipeline.h"
#include "hall_filter.h"
#include "intensidades.h"
#include "rg_system.h"
#include "vs_board_config.h"
#include "tinyfont_data.h"

#define RG_VS_COLUMNS 256
#define RG_VS_PIXELS 54
#define RG_VS_FASTEST_CREDIBLE_TURN_US 10000
#define RG_VS_TAU 6.28318530717958647692
#define RG_VS_APA102_BLACK 0x000000e0

// Base rotation offset (columns, 256 = one full turn): compensates for the
// quarter-turn difference between the board's angle-0 ray and the screen mapping.
// The runtime value vs_angle_offset adds the user-calibrated pov_column_offset
// (written to NVS by MicroPython before launching this app) on top of this.
#define RG_VS_ANGLE_OFFSET 64

static int vs_angle_offset = RG_VS_ANGLE_OFFSET;
static vs_board_config_t vs_board;

// Indexed (palette) framebuffer — 1 byte per pixel, updated by rg_vs_pov_submit_surface().
static uint8_t  *vs_data     = NULL;
// Direct-RGB framebuffer — 0x00RRGGBB per pixel, used when surface format is not indexed.
static uint32_t *vs_data_rgb = NULL;
// Current surface pixel format (rg_pixel_format_t). Determines which buffer project_angle() reads.
static int       vs_format   = 0;

static uint32_t *vs_palette = NULL;
static uint16_t *vs_projection_table = NULL;
static spi_device_handle_t spi_handle;
static spi_transaction_t vs_spi_trans;
static bool vs_spi_ongoing = false;
static char *spi_buf = NULL;
static uint32_t *pixels0 = NULL;
static uint32_t *pixels1 = NULL;
static int buf_size = 0;

// Polar framebuffer of finished per-column APA102 words. The projection
// (project_angle over the whole frame) runs off the per-column critical path
// on the render task; the serve task only copies the current column's two arms
// out of the published front buffer and clocks the SPI, so its cost is
// constant regardless of the emulated scene. Double-buffered in internal SRAM
// (uncached, hence coherent across cores); falls back to a single shared
// buffer if the internal heap can't spare the second one (POV tolerates the
// rare per-column tear that then becomes possible).
static uint32_t *fb_a = NULL;
static uint32_t *fb_b = NULL;
static uint32_t *volatile fb_front = NULL;
static uint32_t *volatile fb_back = NULL;
static int screen_width = 0;
static int screen_height = 0;
static int last_column = 0;

// --- Native (angle, radius) dialog/menu overlay ---------------------------
// project_angle()'s normal path nearest-neighbour-samples a 320x240 Cartesian
// surface down to this disc's 256x54 native resolution, which is fine for
// game video but far too lossy for text (confirmed illegible on hardware).
// The overlay below is addressed directly in (angle, radius) space -- one
// cell per physical LED -- with no resampling, the same way the MicroPython
// ROM browser (system/launcher) keeps its text legible: sprites positioned
// directly in polar coordinates instead of projected from a framebuffer.
//
// Only a narrow wedge of columns is overridden (matching the ROM browser's
// ~21-character-wide band "held at the readable bottom of the display"), so
// the rest of the disc keeps showing whatever game frame is underneath.
// Hardware-confirmed 2026-07-19: widened to 3/4 of the full rotation (was a
// ~1/3-width wedge matching the ROM browser's 21-char band).
#define VS_GUI_WEDGE_HALF_WIDTH 96 // angular columns each side of centre (3/4 of 256)
// Column considered the bottom of the disc after vs_angle_offset is applied
// (see RG_VS_ANGLE_OFFSET above). Best guess from the offset math; nudge
// this if the overlay shows up somewhere other than the bottom on hardware.
#define VS_GUI_WEDGE_CENTER 0
#define VS_GUI_CHAR_STEP (TINYFONT_GLYPH_WIDTH + 1) // 1-column gap between glyphs
#define VS_GUI_ROW_STEP (TINYFONT_GLYPH_HEIGHT + 3) // 3-LED gap between rows
#define VS_GUI_MAX_CHARS (VS_GUI_WEDGE_HALF_WIDTH * 2 / VS_GUI_CHAR_STEP)
#define VS_GUI_MAX_ROWS (RG_VS_PIXELS / VS_GUI_ROW_STEP)

enum { VS_GUI_OFF = 0, VS_GUI_WHITE, VS_GUI_YELLOW };
static const uint32_t vs_gui_palette[] = {
    [VS_GUI_OFF] = 0,
    [VS_GUI_WHITE] = 0x00FFFFFF,
    [VS_GUI_YELLOW] = 0x00FFE000,
};

static volatile bool vs_gui_overlay_active = false;
static uint8_t vs_gui_overlay[RG_VS_COLUMNS][RG_VS_PIXELS];

static void vs_ensure_buffers(int width, int height); // defined below, used by rg_vs_pov_draw_native_dialog()

static inline bool vs_gui_column_in_wedge(int column)
{
    int rel = (column - VS_GUI_WEDGE_CENTER) % RG_VS_COLUMNS;
    if (rel < 0)
        rel += RG_VS_COLUMNS;
    if (rel > RG_VS_COLUMNS / 2)
        rel -= RG_VS_COLUMNS;
    return rel >= -VS_GUI_WEDGE_HALF_WIDTH && rel <= VS_GUI_WEDGE_HALF_WIDTH;
}

// row_index 0 is the outermost (rim-most) row; higher indices move toward
// the centre. Hardware-confirmed 2026-07-19: legibility degrades sharply
// near the centre/axle (rows there read as tiny/unreadable regardless of
// character count), so row 0 (the title, read first) belongs at the rim
// where it's most legible, and later/less-critical rows (eg. a trailing
// "OK") move inward -- confirmed readable even that close to centre because
// it's short.
static void vs_gui_draw_glyph(int angle_start, int row_index, int c, uint8_t color)
{
    if (c < 0 || c >= TINYFONT_NUM_GLYPHS)
        c = ' ';
    int row_base_led = RG_VS_PIXELS - (row_index + 1) * VS_GUI_ROW_STEP;
    const uint8_t *rows = tinyfont_glyphs[c];
    for (int ry = 0; ry < TINYFONT_GLYPH_HEIGHT; ry++)
    {
        uint8_t bits = rows[ry];
        if (!bits)
            continue;
        // Hardware-confirmed 2026-07-19: the disc shows the naive mapping
        // rotated 180 degrees, so both axes are mirrored here relative to the
        // "obvious" (rx, ry) -> (col, led) mapping.
        int led = row_base_led + ry;
        if (led < 0 || led >= RG_VS_PIXELS)
            continue;
        for (int rx = 0; rx < TINYFONT_GLYPH_WIDTH; rx++)
        {
            if (!(bits & (1 << (TINYFONT_GLYPH_WIDTH - 1 - rx))))
                continue;
            int col = (angle_start - rx) % RG_VS_COLUMNS;
            if (col < 0)
                col += RG_VS_COLUMNS;
            vs_gui_overlay[col][led] = color;
        }
    }
}

static void vs_gui_draw_line(int row_index, const char *text, uint8_t color)
{
    if (!text || !*text || row_index >= VS_GUI_MAX_ROWS)
        return;
    int len = (int)strlen(text);
    if (len > VS_GUI_MAX_CHARS)
        len = VS_GUI_MAX_CHARS;
    int angle_start = VS_GUI_WEDGE_CENTER - (len * VS_GUI_CHAR_STEP) / 2;
    // Character order is reversed too, to match the 180-degree mirroring in
    // vs_gui_draw_glyph() (otherwise each glyph would be correctly rotated
    // but the string would read back-to-front).
    for (int i = 0; i < len; i++)
        vs_gui_draw_glyph(angle_start + (len - 1 - i) * VS_GUI_CHAR_STEP, row_index, text[i], color);
}

bool rg_vs_pov_draw_native_dialog(const char *title, const char *const *lines, int line_count, int selected_index)
{
    if (!rg_vs_pov_enabled())
        return false;

    // A dialog can be the very first thing ever drawn (eg. a startup alert,
    // before any game frame was submitted) -- make sure gpu_step() has
    // buffers to scan at all, even if they're blank.
    vs_ensure_buffers(RG_SCREEN_WIDTH, RG_SCREEN_HEIGHT);

    if (line_count > VS_GUI_MAX_ROWS - 1)
        line_count = VS_GUI_MAX_ROWS - 1; // caller should already have windowed to this

    memset(vs_gui_overlay, VS_GUI_OFF, sizeof(vs_gui_overlay));

    vs_gui_draw_line(0, title, VS_GUI_WHITE);
    for (int i = 0; i < line_count; i++)
        vs_gui_draw_line(i + 1, lines[i], i == selected_index ? VS_GUI_YELLOW : VS_GUI_WHITE);

    __atomic_store_n(&vs_gui_overlay_active, true, __ATOMIC_RELEASE);
    return true;
}

void rg_vs_pov_clear_native_dialog(void)
{
    __atomic_store_n(&vs_gui_overlay_active, false, __ATOMIC_RELEASE);
}

// The exit transition runs on the game task while the LED task keeps scanning
// the last copied surface on core 1.  The atomic flag rejects any later
// surface submission, so the visible image remains frozen during the sweep.
static bool vs_exit_fade_active = false;
static uint8_t vs_exit_black_outer_leds = 0;
static uint32_t vs_exit_fade_generation = 0;
static uint32_t vs_exit_presented_generation = 0;

// True while framebuffers are being (re)allocated or the projection table is being rebuilt.
// The render and serve tasks skip work while this is set. Initially true so they wait for
// the first rg_vs_pov_submit_surface() call before touching vs_data.
static volatile bool rebuilding = true;

// Filtered equivalents of the raw hall edge -- gpu_serve() reads these two
// globals, same as before the hall filter existed; only the writer changed
// (see hall_filter_drain() below). Mirrors hardware/rotor/modules/povdisplay/povdisplay.c.
static volatile int64_t last_turn = 0;
static volatile int64_t last_turn_duration = 102400;

// hall_neg_sensed() (IRAM_ATTR, must stay division-free) hands raw edges that
// pass the coarse debounce floor to this single-slot mailbox; hall_filter_drain()
// -- called from vs_display_task()'s task-context loop, never from the ISR --
// does the real classification, which needs 64-bit division that isn't
// guaranteed IRAM-safe on this toolchain. See hall_filter.h.
static portMUX_TYPE hall_lock = portMUX_INITIALIZER_UNLOCKED;
static volatile int64_t hall_pending_edge = 0;
static volatile bool hall_pending = false;
static int64_t hall_raw_last_edge = 0;  // ISR-only: previous edge that passed the coarse floor
static hall_filter_t hall_filter_state;

// Diagnostic bypass: when false, hall_filter_drain() reproduces the
// pre-filter behavior exactly (raw carry-forward, no classification). Mirrors
// hardware/rotor/modules/povdisplay/povdisplay.c's hall_filter_enabled.
static volatile bool hall_filter_enabled = true;

// This profiler is deliberately opt-in, mirroring the MicroPython GPU task
// profiler (hardware/rotor/modules/povdisplay/povdisplay.c) so the same
// "povperf" wire commands (docs/internals/input-protocol-v2.md) work here.
typedef struct {
    uint32_t samples;
    uint32_t skipped_updates;
    uint32_t deadline_misses;
    uint32_t deadline_us;
    uint64_t total_us;
    uint64_t project_us;
    uint64_t spi_us;
    uint32_t max_total_us;
    uint32_t max_project_us;
    uint32_t max_arm_project_us;
    uint32_t max_spi_us;
    int32_t worst_slack_us;
    bool have_column;
    uint8_t last_perf_column;
} vs_pov_performance_t;

static portMUX_TYPE vs_performance_lock = portMUX_INITIALIZER_UNLOCKED;
static volatile bool vs_performance_enabled = false;
static vs_pov_performance_t vs_performance;

static uint32_t vs_elapsed_us(int64_t start, int64_t end) {
    return end <= start ? 0 : (uint32_t)(end - start);
}

static bool vs_performance_is_enabled(void) {
    return __atomic_load_n(&vs_performance_enabled, __ATOMIC_ACQUIRE);
}

static void vs_performance_reset(void) {
    portENTER_CRITICAL(&vs_performance_lock);
    memset(&vs_performance, 0, sizeof(vs_performance));
    vs_performance.worst_slack_us = INT32_MAX;
    portEXIT_CRITICAL(&vs_performance_lock);
}

static void vs_performance_record(uint8_t column, uint32_t deadline_us,
        uint32_t total_us, uint32_t project_us, uint32_t arm0_us,
        uint32_t arm1_us, uint32_t spi_us) {
    if (!vs_performance_is_enabled()) {
        return;
    }
    int32_t slack_us = deadline_us > (uint32_t)INT32_MAX
        ? INT32_MAX : (int32_t)deadline_us - (int32_t)total_us;
    portENTER_CRITICAL(&vs_performance_lock);
    if (vs_performance.have_column) {
        uint8_t delta = (column - vs_performance.last_perf_column) & 0xff;
        if (delta > 1) {
            vs_performance.skipped_updates += delta - 1;
        }
    } else {
        vs_performance.have_column = true;
    }
    vs_performance.last_perf_column = column;
    vs_performance.samples++;
    vs_performance.deadline_us = deadline_us;
    vs_performance.total_us += total_us;
    vs_performance.project_us += project_us;
    vs_performance.spi_us += spi_us;
    if (total_us > vs_performance.max_total_us) vs_performance.max_total_us = total_us;
    if (project_us > vs_performance.max_project_us) vs_performance.max_project_us = project_us;
    if (arm0_us > vs_performance.max_arm_project_us) vs_performance.max_arm_project_us = arm0_us;
    if (arm1_us > vs_performance.max_arm_project_us) vs_performance.max_arm_project_us = arm1_us;
    if (spi_us > vs_performance.max_spi_us) vs_performance.max_spi_us = spi_us;
    if (total_us > deadline_us) vs_performance.deadline_misses++;
    if (slack_us < vs_performance.worst_slack_us) vs_performance.worst_slack_us = slack_us;
    portEXIT_CRITICAL(&vs_performance_lock);
}

static void vs_load_color_profile(void) {
    uint8_t profile[COLOR_PIPELINE_PROFILE_BYTES];
    size_t length = sizeof(profile);
    nvs_handle_t nvs;
    if (nvs_open("voom_pov", NVS_READONLY, &nvs) != ESP_OK) {
        RG_LOGW("vs_pov: no POV colour profile; using legacy transfer tables\n");
        return;
    }
    esp_err_t result = nvs_get_blob(nvs, "color_v1", profile, &length);
    nvs_close(nvs);
    if (result != ESP_OK || !color_pipeline_apply(profile, length)) {
        RG_LOGW("vs_pov: invalid POV colour profile; using legacy transfer tables\n");
        return;
    }
    RG_LOGI("vs_pov: loaded calibrated POV colour profile\n");
}

static void vs_setup_projection_table(void) {
    int center_x = screen_width / 2;
    int center_y = screen_height / 2;
    int radius = RG_MIN(center_x, center_y) - 2;

    for (int angle = 0; angle < RG_VS_COLUMNS; angle++) {
        double a = (angle + vs_angle_offset) * RG_VS_TAU / RG_VS_COLUMNS;
        for (int led = 0; led < RG_VS_PIXELS; led++) {
            int x = 128 + (int)(radius * (led + 1) / RG_VS_PIXELS * cos(a));
            int y = 128 + (int)(radius * (led + 1) / RG_VS_PIXELS * sin(a));
            vs_projection_table[angle * RG_VS_PIXELS + led] = (x << 8) + y;
        }
    }
}

static void project_angle(int angle, uint32_t row[RG_VS_PIXELS]) {
    uint8_t black_outer_leds = __atomic_load_n(
        &vs_exit_black_outer_leds, __ATOMIC_ACQUIRE);
    bool overlay_here = __atomic_load_n(&vs_gui_overlay_active, __ATOMIC_ACQUIRE)
        && vs_gui_column_in_wedge(angle);
    for (int led = 0; led < RG_VS_PIXELS; led++) {
        // LED zero is the centre of the projected frame, so blackening the
        // highest indexes first visibly moves from the outside toward it.
        if (led >= RG_VS_PIXELS - black_outer_leds) {
            row[led] = RG_VS_APA102_BLACK;
            continue;
        }

        uint32_t doom_color;
        uint8_t overlay_color = overlay_here ? vs_gui_overlay[angle][led] : VS_GUI_OFF;
        if (overlay_color != VS_GUI_OFF) {
            doom_color = vs_gui_palette[overlay_color];
        } else {
            uint16_t pos = vs_projection_table[angle * RG_VS_PIXELS + led];
            int x = ((pos >> 8) & 0xff) - 128 + screen_width / 2;
            int y = (pos & 0xff) - 128 + screen_height / 2;
            if (vs_format & RG_PIXEL_PALETTE) {
                doom_color = vs_palette[vs_data[y * screen_width + x]];
            } else {
                doom_color = vs_data_rgb[y * screen_width + x];
            }
        }

        if (color_pipeline_is_active()) {
            row[led] = color_pipeline_encode_rgb(
                led,
                (doom_color & 0xff0000) >> 16,
                (doom_color & 0x00ff00) >> 8,
                doom_color & 0x0000ff
            );
        } else {
            int level = intensidades_por_led[led];
            row[led] = (brillos[led] & 0x1f) | 0xe0 |
                intensidades[level][(doom_color & 0xff0000) >> 16] << 24 |
                intensidades[level][(doom_color & 0x00ff00) >> 8] << 16 |
                intensidades[level][doom_color & 0x0000ff] << 8;
        }
    }
}

static void spi_start_buses(void) {
    esp_err_t ret;
    const spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = vs_board.led_mosi,
        .sclk_io_num = vs_board.led_clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ret = spi_bus_initialize((spi_host_device_t)vs_board.led_spi_host, &buscfg, SPI_DMA_CH_AUTO);
    RG_ASSERT(ret == ESP_OK || ret == ESP_ERR_INVALID_STATE, "spi_bus_initialize failed.");

    // The configured CS frames each LED burst for the hardware workbench SPI
    // slave. APA102 strips ignore it, so -1 is also valid for a board without CS.
    const spi_device_interface_config_t devcfg = {
        // Honor the NVS-provisioned LED clock (defaults to 30 MHz on current
        // boards) rather than a hardcoded rate; a shorter transfer is what
        // gives the decoupled serve enough per-column headroom.
        .clock_speed_hz = vs_board.led_freq > 0 ? vs_board.led_freq : SPI_MASTER_FREQ_20M,
        .mode = 0,
        .spics_io_num = vs_board.led_cs,
        .queue_size = 2,
    };
    ret = spi_bus_add_device((spi_host_device_t)vs_board.led_spi_host, &devcfg, &spi_handle);
    RG_ASSERT(ret == ESP_OK, "spi_bus_add_device failed.");
}

static void vsspi_init(void) {
    buf_size = 4 + RG_VS_PIXELS * 4 * 2 + 8;
    spi_buf = heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
    RG_ASSERT(spi_buf, "ventilastation SPI buffer alloc failed");
    memset(spi_buf, 0xff, buf_size);
    ((uint32_t *)spi_buf)[0] = 0;
    pixels0 = (uint32_t *)(spi_buf + 4);
    pixels1 = (uint32_t *)(spi_buf + RG_VS_PIXELS * 4);
    for (int n = 0; n < RG_VS_PIXELS; n++) {
        pixels0[n] = 0x010000ff;
        pixels1[n] = 0x000100ff;
    }

    // Framebuffers must be internal (uncached) so the render task's writes on
    // core 1 are seen by the serve without cache maintenance.
    size_t fb_bytes = (size_t)RG_VS_COLUMNS * RG_VS_PIXELS * sizeof(uint32_t);
    fb_a = heap_caps_malloc(fb_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);
    fb_b = heap_caps_malloc(fb_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);
    RG_ASSERT(fb_a, "ventilastation framebuffer alloc failed");
    if (!fb_b) {
        fb_b = fb_a; // single-buffer fallback (tear-tolerant)
    }
    for (size_t i = 0; i < (size_t)RG_VS_COLUMNS * RG_VS_PIXELS; i++) {
        fb_a[i] = RG_VS_APA102_BLACK;
        fb_b[i] = RG_VS_APA102_BLACK;
    }
    fb_front = fb_a;
    fb_back = fb_b;
}

// Queues data_in for DMA transmission and returns immediately, so the caller
// can render the next column while the transfer is still in flight. Mirrors
// spiWriteNL() in hardware/rotor/modules/povdisplay/minispi.c. The caller
// must not touch data_in again until vsspi_wait_complete() returns.
static void vsspi_write_queue(const void *data_in, size_t len) {
    vs_spi_trans.length = len * 8;
    vs_spi_trans.tx_buffer = data_in;
    vs_spi_trans.flags = SPI_TRANS_DMA_BUFFER_ALIGN_MANUAL;
    esp_err_t ret = spi_device_queue_trans(spi_handle, &vs_spi_trans, pdMS_TO_TICKS(10));
    ESP_ERROR_CHECK(ret);
    vs_spi_ongoing = true;
}

// Blocks until the transfer queued by vsspi_write_queue() has completed.
static void vsspi_wait_complete(void) {
    if (!vs_spi_ongoing) {
        return;
    }
    spi_transaction_t *completed_trans = NULL;
    esp_err_t ret = spi_device_get_trans_result(spi_handle, &completed_trans, pdMS_TO_TICKS(100));
    ESP_ERROR_CHECK(ret);
    vs_spi_ongoing = false;
}

static void IRAM_ATTR hall_neg_sensed(void *arg) {
    int64_t this_turn = rg_system_timer();
    int64_t this_turn_duration = this_turn - hall_raw_last_edge;
    if (this_turn_duration > RG_VS_FASTEST_CREDIBLE_TURN_US) {
        hall_raw_last_edge = this_turn;
        portENTER_CRITICAL_ISR(&hall_lock);
        hall_pending_edge = this_turn;
        hall_pending = true;
        portEXIT_CRITICAL_ISR(&hall_lock);
    }
}

static void hall_init(void) {
    hall_filter_init(&hall_filter_state, last_turn_duration);
    gpio_set_direction(vs_board.hall_gpio, GPIO_MODE_INPUT);
    gpio_set_intr_type(vs_board.hall_gpio, GPIO_INTR_NEGEDGE);
    esp_err_t ret = gpio_install_isr_service(0);
    RG_ASSERT(ret == ESP_OK || ret == ESP_ERR_INVALID_STATE, "gpio_install_isr_service failed.");
    ret = gpio_isr_handler_add(vs_board.hall_gpio, hall_neg_sensed, (void *)vs_board.hall_gpio);
    RG_ASSERT(ret == ESP_OK, "gpio_isr_handler_add failed.");
}

// Task-context only (never call from an ISR: hall_filter_submit() does 64-bit
// division). Drains at most one pending edge per call -- vs_display_task()'s
// loop iterates far faster than the hall fires, so this always runs well
// within one column period of the real edge.
static void hall_filter_drain(void) {
    int64_t edge;
    bool has_edge;
    portENTER_CRITICAL(&hall_lock);
    has_edge = hall_pending;
    edge = hall_pending_edge;
    hall_pending = false;
    portEXIT_CRITICAL(&hall_lock);
    if (!has_edge) {
        return;
    }
    if (!__atomic_load_n(&hall_filter_enabled, __ATOMIC_ACQUIRE)) {
        last_turn_duration = edge - last_turn;
        last_turn = edge;
        return;
    }
    if (hall_filter_submit(&hall_filter_state, edge)) {
        last_turn = hall_filter_state.last_turn_us;
        last_turn_duration = hall_filter_state.period_us;
    }
}

void rg_vs_pov_set_hall_filter_enabled(bool enabled) {
    bool was_enabled = __atomic_load_n(&hall_filter_enabled, __ATOMIC_ACQUIRE);
    if (enabled && !was_enabled) {
        hall_filter_init(&hall_filter_state, last_turn_duration);
    }
    __atomic_store_n(&hall_filter_enabled, enabled, __ATOMIC_RELEASE);
}

bool rg_vs_pov_get_hall_filter_enabled(void) {
    return __atomic_load_n(&hall_filter_enabled, __ATOMIC_ACQUIRE);
}

void rg_vs_pov_get_hall_filter_stats(rg_vs_pov_hall_filter_stats_t *out) {
    out->enabled = __atomic_load_n(&hall_filter_enabled, __ATOMIC_ACQUIRE);
    out->period_us = hall_filter_state.period_us;
    out->jitter_us = hall_filter_state.jitter_us;
    out->accepted = hall_filter_state.accepted_count;
    out->spurious = hall_filter_state.spurious_count;
    out->missed = hall_filter_state.missed_count;
    out->missed_pulses = hall_filter_state.missed_pulses_total;
    out->outlier = hall_filter_state.outlier_count;
    out->stall = hall_filter_state.stall_count;
    out->resync = hall_filter_state.resync_count;
}

// Serve the column currently under the LEDs, if the fan has advanced to a new
// one: copy that column's two arms out of the published framebuffer and start
// the DMA. Constant, cheap cost -- no projection here -- so it always meets the
// per-column deadline regardless of scene complexity.
static void gpu_serve(void) {
    int64_t now = rg_system_timer();
    uint32_t column = ((now - last_turn) * RG_VS_COLUMNS / last_turn_duration) % RG_VS_COLUMNS;
    if (column != last_column) {
        bool measuring = vs_performance_is_enabled();
        int64_t measurement_start = measuring ? rg_system_timer() : 0;
        uint32_t column_deadline_us = last_turn_duration > 0
            ? (uint32_t)(last_turn_duration / RG_VS_COLUMNS) : 0;

        // The previous DMA overlapped the projection work below (it was queued
        // last iteration and ran while we projected a column), so this wait is
        // normally already satisfied.
        vsspi_wait_complete();
        int64_t wait_end = measuring ? rg_system_timer() : 0;

        uint32_t *fb = __atomic_load_n(&fb_front, __ATOMIC_ACQUIRE);
        uint32_t *arm0 = fb + (size_t)((column + RG_VS_COLUMNS / 2) % RG_VS_COLUMNS) * RG_VS_PIXELS;
        uint32_t *arm1 = fb + (size_t)column * RG_VS_PIXELS;
        for (int n = 0; n < RG_VS_PIXELS; n++) {
            pixels0[n] = arm0[RG_VS_PIXELS - 1 - n];
            pixels1[n] = arm1[n];
        }
        vsspi_write_queue(spi_buf, buf_size);
        int64_t queue_end = measuring ? rg_system_timer() : 0;
        last_column = column;

        if (measuring) {
            vs_performance_record(column, column_deadline_us,
                vs_elapsed_us(measurement_start, queue_end),  // total
                0, 0, 0,                                       // projection off-path
                vs_elapsed_us(measurement_start, wait_end));   // spi wait
        }
    }
}

// Project one framebuffer column per call, publishing a fresh frame every time
// the whole ring has been covered. Interleaved with gpu_serve() on the same
// task: because each projected column (~one SPI transfer's worth of work)
// overlaps the DMA of the column just served, projection is off the serve's
// critical path -- a served column always reads an already-projected fb entry,
// and the projection only ever delays the next serve by well under a column
// period. This keeps the panel refreshing at ~30 fps with zero skips while the
// serve stays constant-cost.
static int render_col = 0;
static void project_next_column(void) {
    project_angle(render_col, fb_back + (size_t)render_col * RG_VS_PIXELS);
    if (++render_col >= RG_VS_COLUMNS) {
        render_col = 0;
        uint32_t generation = __atomic_load_n(&vs_exit_fade_generation, __ATOMIC_ACQUIRE);
        uint32_t *published = fb_back;
        __atomic_store_n(&fb_front, published, __ATOMIC_RELEASE);
        if (fb_a != fb_b)
            fb_back = (published == fb_a) ? fb_b : fb_a;
        // A frame projected at this generation is now published and will be
        // served within a rotation -- what rg_vs_pov_fade_last_frame_to_black()
        // waits on.
        __atomic_store_n(&vs_exit_presented_generation, generation, __ATOMIC_RELEASE);
    }
}

static void IRAM_ATTR vs_display_task(void *arg) {
    hall_init();
    // Acquire the bus once and keep driving the LED strip.
    spi_start_buses();
    ESP_ERROR_CHECK(spi_device_acquire_bus(spi_handle, portMAX_DELAY));
    while (true) {
        hall_filter_drain();
        if (rebuilding) {
            rg_task_delay(2); // no surface/framebuffer yet
            continue;
        }
        gpu_serve();
        project_next_column();
    }
}

bool rg_vs_pov_enabled(void) {
    return true;
}

void rg_vs_pov_init(void) {
    if (!vs_board_config_load(&vs_board)) {
        RG_LOGE("vs_pov: board configuration missing; run make configure-board\n");
        return;
    }
    // POV calibration is independent of the board wiring and is adjusted from
    // the MicroPython settings scene.
    nvs_handle_t nvs;
    if (nvs_open("voom_pov", NVS_READONLY, &nvs) == ESP_OK) {
        int32_t offset;
        if (nvs_get_i32(nvs, "col_offset", &offset) == ESP_OK)
            vs_angle_offset = ((int)RG_VS_ANGLE_OFFSET + (int)offset + RG_VS_COLUMNS) % RG_VS_COLUMNS;
        nvs_close(nvs);
    }
    vs_load_color_profile();

    vs_palette          = rg_alloc(256 * sizeof(uint32_t), MEM_FAST);
    vs_projection_table = rg_alloc(RG_VS_COLUMNS * RG_VS_PIXELS * sizeof(uint16_t), MEM_FAST);
    RG_ASSERT(vs_palette && vs_projection_table, "ventilastation POV alloc failed");

    memset(vs_palette, 0, 256 * sizeof(uint32_t));
    memset(vs_projection_table, 0, RG_VS_COLUMNS * RG_VS_PIXELS * sizeof(uint16_t));

    // vs_data / vs_data_rgb allocated lazily in rg_vs_pov_submit_surface().
    // rebuilding stays true until then so the projection doesn't touch NULL
    // pointers.

    vsspi_init();
    // One task on core 1 both serves columns and projects the framebuffer,
    // interleaved (see vs_display_task); core 0 stays entirely with the
    // emulator.
    rg_task_create("vs_display", &vs_display_task, NULL, 3 * 1024, RG_TASK_PRIORITY_6, 1);
}

void rg_vs_pov_set_palette32(const uint32_t *palette, size_t count)
{
    if (!vs_palette || !palette)
        return;
    if (count > 256)
        count = 256;
    memcpy(vs_palette, palette, count * sizeof(uint32_t));
}

void rg_vs_pov_set_performance_profiling(bool enabled)
{
    if (enabled) {
        vs_performance_reset();
    }
    __atomic_store_n(&vs_performance_enabled, enabled, __ATOMIC_RELEASE);
}

void rg_vs_pov_reset_performance_stats(void)
{
    vs_performance_reset();
}

void rg_vs_pov_get_performance_stats(rg_vs_pov_performance_stats_t *out)
{
    vs_pov_performance_t snapshot;
    portENTER_CRITICAL(&vs_performance_lock);
    snapshot = vs_performance;
    portEXIT_CRITICAL(&vs_performance_lock);

    uint32_t samples = snapshot.samples;
    out->enabled = vs_performance_is_enabled();
    out->calibrated = color_pipeline_is_active();
    out->samples = samples;
    out->skipped_updates = snapshot.skipped_updates;
    out->deadline_misses = snapshot.deadline_misses;
    out->deadline_us = snapshot.deadline_us;
    out->avg_total_us = samples ? (uint32_t)(snapshot.total_us / samples) : 0;
    out->max_total_us = snapshot.max_total_us;
    out->avg_project_us = samples ? (uint32_t)(snapshot.project_us / samples) : 0;
    out->max_project_us = snapshot.max_project_us;
    out->max_arm_project_us = snapshot.max_arm_project_us;
    out->avg_spi_us = samples ? (uint32_t)(snapshot.spi_us / samples) : 0;
    out->max_spi_us = snapshot.max_spi_us;
    out->worst_slack_us = samples ? snapshot.worst_slack_us : 0;
}

// (Re)allocate framebuffers and rebuild the projection table for the given
// dimensions, if not already done. Shared by rg_vs_pov_submit_surface() and
// rg_vs_pov_draw_native_dialog(): the latter can be the very first thing
// drawn (eg. a "BIOS file missing" alert before any game frame exists), and
// gpu_step() won't scan anything -- overlay included -- until this has run
// at least once (see `rebuilding` below).
static void vs_ensure_buffers(int width, int height)
{
    if (screen_width == width && screen_height == height && vs_data && vs_data_rgb)
        return;

    rebuilding = true;
    free(vs_data);     vs_data     = NULL;
    free(vs_data_rgb); vs_data_rgb = NULL;

    screen_width  = width;
    screen_height = height;
    size_t n = (size_t)screen_width * screen_height;

    vs_data     = rg_alloc(n,                    MEM_FAST);
    vs_data_rgb = rg_alloc(n * sizeof(uint32_t), MEM_FAST);
    RG_ASSERT(vs_data && vs_data_rgb, "ventilastation POV surface alloc failed");

    memset(vs_data,     0, n);
    memset(vs_data_rgb, 0, n * sizeof(uint32_t));
    vs_setup_projection_table();
    rebuilding = false;
}

void rg_vs_pov_submit_surface(const rg_surface_t *surface)
{
    if (__atomic_load_n(&vs_exit_fade_active, __ATOMIC_ACQUIRE))
        return;
    if (!surface || !surface->data)
        return;
    vs_ensure_buffers(surface->width, surface->height);

    vs_format = surface->format;

    if (surface->format & RG_PIXEL_PALETTE)
    {
        // Indexed surface: 1 byte per pixel.
        const uint8_t *src = (const uint8_t *)surface->data;
        int src_stride = surface->stride ? surface->stride : surface->width;
        if (src_stride == surface->width)
        {
            memcpy(vs_data, src, (size_t)surface->width * surface->height);
        }
        else
        {
            for (int y = 0; y < surface->height; y++)
                memcpy(vs_data + y * surface->width, src + y * src_stride, surface->width);
        }

        // Derive vs_palette from the surface's RGB565 palette (big-endian for PAL565_BE).
        if (surface->palette)
        {
            bool be = (surface->format & ~RG_PIXEL_PALETTE) == RG_PIXEL_565_BE;
            for (int i = 0; i < 256; i++)
            {
                uint16_t p = surface->palette[i];
                if (be) p = __builtin_bswap16(p);
                uint32_t r = ((p >> 11) & 0x1F) * 255 / 31;
                uint32_t g = ((p >> 5)  & 0x3F) * 255 / 63;
                uint32_t b = ( p        & 0x1F) * 255 / 31;
                vs_palette[i] = (r << 16) | (g << 8) | b;
            }
        }
    }
    else
    {
        // Direct RGB surface: convert to 0x00RRGGBB and store in vs_data_rgb.
        int w = surface->width, h = surface->height;

        if (surface->format == RG_PIXEL_565_LE || surface->format == RG_PIXEL_565_BE)
        {
            const uint16_t *src = (const uint16_t *)surface->data;
            int src_stride_px = surface->stride ? (surface->stride / 2) : w;
            bool be = (surface->format == RG_PIXEL_565_BE);
            for (int y = 0; y < h; y++)
            {
                const uint16_t *row = src + y * src_stride_px;
                uint32_t *dst = vs_data_rgb + y * w;
                for (int x = 0; x < w; x++)
                {
                    uint16_t p = row[x];
                    if (be) p = __builtin_bswap16(p);
                    uint32_t r = ((p >> 11) & 0x1F) * 255 / 31;
                    uint32_t g = ((p >> 5)  & 0x3F) * 255 / 63;
                    uint32_t b = ( p        & 0x1F) * 255 / 31;
                    dst[x] = (r << 16) | (g << 8) | b;
                }
            }
        }
        else if (surface->format == RG_PIXEL_888)
        {
            const uint8_t *src = (const uint8_t *)surface->data;
            int src_stride_b = surface->stride ? surface->stride : (w * 3);
            for (int y = 0; y < h; y++)
            {
                const uint8_t *row = src + y * src_stride_b;
                uint32_t *dst = vs_data_rgb + y * w;
                for (int x = 0; x < w; x++)
                    dst[x] = ((uint32_t)row[x*3] << 16) | ((uint32_t)row[x*3+1] << 8) | row[x*3+2];
            }
        }
    }
}

void rg_vs_pov_fade_last_frame_to_black(uint32_t duration_ms)
{
    if (rebuilding || (!vs_data && !vs_data_rgb))
        return;

    __atomic_store_n(&vs_exit_fade_active, true, __ATOMIC_RELEASE);
    __atomic_store_n(&vs_exit_black_outer_leds, 0, __ATOMIC_RELEASE);

    int64_t started = rg_system_timer();
    int64_t duration_us = (int64_t)duration_ms * 1000;
    while (duration_us > 0) {
        int64_t elapsed = rg_system_timer() - started;
        if (elapsed >= duration_us)
            break;
        uint32_t black_leds = (uint32_t)(elapsed * RG_VS_PIXELS / duration_us);
        __atomic_store_n(&vs_exit_black_outer_leds, black_leds, __ATOMIC_RELEASE);
        // Yield to the core-1 SPI task while keeping the game loop stopped.
        rg_task_delay(5);
    }

    __atomic_store_n(&vs_exit_black_outer_leds, RG_VS_PIXELS, __ATOMIC_RELEASE);
    uint32_t generation = __atomic_add_fetch(
        &vs_exit_fade_generation, 1, __ATOMIC_ACQ_REL);
    // Do not reset until the display task has sent the black centre LED at
    // least once.
    while (__atomic_load_n(&vs_exit_presented_generation, __ATOMIC_ACQUIRE) != generation) {
        rg_task_delay(1);
    }
}

#else

bool rg_vs_pov_enabled(void) {
    return false;
}

void rg_vs_pov_init(void) {
}

void rg_vs_pov_set_palette32(const uint32_t *palette, size_t count) {
    (void)palette;
    (void)count;
}

void rg_vs_pov_submit_surface(const rg_surface_t *surface) {
    (void)surface;
}

void rg_vs_pov_fade_last_frame_to_black(uint32_t duration_ms) {
    (void)duration_ms;
}

void rg_vs_pov_set_performance_profiling(bool enabled) {
    (void)enabled;
}

void rg_vs_pov_reset_performance_stats(void) {
}

void rg_vs_pov_get_performance_stats(rg_vs_pov_performance_stats_t *out) {
    *out = (rg_vs_pov_performance_stats_t){ 0 };
bool rg_vs_pov_draw_native_dialog(const char *title, const char *const *lines, int line_count, int selected_index) {
    (void)title;
    (void)lines;
    (void)line_count;
    (void)selected_index;
    return false;
}

void rg_vs_pov_clear_native_dialog(void) {
}

#endif
