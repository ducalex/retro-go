#include "vs_host_bridge.h"

#include "rg_system.h"

#if defined(ESP_PLATFORM) && defined(RG_VS_ENABLE_HOST_BRIDGE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nvs.h>
#include "color_pipeline.h"
#include "vs_board_config.h"
#include "drivers/display/ventilastation_pov.h"

// ---- Input protocol v2 byte-stream parser ----
// Feeds the UART transport below. Mirrors
// apps/micropython/ventilastation/input_parser.py: joystick frames
// ('*' + 3 data bytes) and command lines (alnum-prefixed, '\n'-terminated)
// share the same byte stream (see docs/internals/input-protocol-v2.md in vsdk).
typedef enum
{
    VS_SCAN,
    VS_JOY,
    VS_CMD,
} vs_parse_state_t;

#define VS_CMD_MAX 256

static vs_parse_state_t vs_state = VS_SCAN;
static uint8_t vs_joy_buf[3];
static int vs_joy_pos = 0;
static char vs_cmd_buf[VS_CMD_MAX + 1];
static int vs_cmd_len = 0;

static uint8_t vs_joy1 = 0;
static uint8_t vs_joy2 = 0;
static uint8_t vs_extra = 0;

// RESYNC / device identification (see
// docs/internals/input-protocol-v2.md#resync--device-identification in vsdk).
// Mirrors input_parser.py's _RESYNC_SEQUENCE byte-for-byte, including the
// "track a match in parallel with normal parsing, don't suppress it" design:
// only the leading 'R' has its high bit set, so this is always safe to
// recognize regardless of vs_state.
static const uint8_t VS_RESYNC_SEQUENCE[] = { '\n', '\n', 0xD2, 'E', 'S', 'Y', 'N', 'C', '\n' };
#define VS_RESYNC_LEN (sizeof(VS_RESYNC_SEQUENCE) / sizeof(VS_RESYNC_SEQUENCE[0]))
static int vs_resync_match = 0;

#define VS_POVCAL_NAMESPACE "voom_pov"
#define VS_POVCAL_KEY "color_v1"
#define VS_POVCAL_HEADER_BYTES 12
#define VS_POVCAL_CONTROLS_BYTES 15
#define VS_POVCAL_MATRIX_BYTES 18
#define VS_POVCAL_LED_TRIMS (VS_POVCAL_HEADER_BYTES + VS_POVCAL_CONTROLS_BYTES + VS_POVCAL_MATRIX_BYTES)
#define VS_POVCAL_SOURCE_EOTF VS_POVCAL_HEADER_BYTES
#define VS_POVCAL_SOURCE_GAMMA (VS_POVCAL_SOURCE_EOTF + 1)
#define VS_POVCAL_MASTER (VS_POVCAL_SOURCE_GAMMA + 2)
#define VS_POVCAL_WHITE (VS_POVCAL_MASTER + 2)
#define VS_POVCAL_RADIAL (VS_POVCAL_WHITE + 6)
#define VS_POVCAL_GB_FLOOR (VS_POVCAL_RADIAL + 2)
#define VS_POVCAL_GB_CEILING (VS_POVCAL_GB_FLOOR + 1)

static uint8_t vs_povcal_profile[COLOR_PIPELINE_PROFILE_BYTES];
static bool vs_povcal_loaded = false;

static uint16_t vs_povcal_u16(const uint8_t *data, int offset)
{
    return (uint16_t)data[offset] | ((uint16_t)data[offset + 1] << 8);
}

static uint32_t vs_povcal_u32(const uint8_t *data, int offset)
{
    return (uint32_t)data[offset] | ((uint32_t)data[offset + 1] << 8)
        | ((uint32_t)data[offset + 2] << 16) | ((uint32_t)data[offset + 3] << 24);
}

static void vs_povcal_put_u16(uint8_t *data, int offset, uint16_t value)
{
    data[offset] = value & 0xff;
    data[offset + 1] = value >> 8;
}

static void vs_povcal_put_u32(uint8_t *data, int offset, uint32_t value)
{
    for (int index = 0; index < 4; index++)
        data[offset + index] = (value >> (index * 8)) & 0xff;
}

static bool vs_povcal_load(void)
{
    nvs_handle_t nvs;
    size_t length = sizeof(vs_povcal_profile);
    if (nvs_open(VS_POVCAL_NAMESPACE, NVS_READONLY, &nvs) != ESP_OK)
        return false;
    esp_err_t result = nvs_get_blob(nvs, VS_POVCAL_KEY, vs_povcal_profile, &length);
    nvs_close(nvs);
    if (result != ESP_OK || !color_pipeline_apply(vs_povcal_profile, length))
        return false;
    vs_povcal_loaded = true;
    return true;
}

static bool vs_povcal_save(void)
{
    nvs_handle_t nvs;
    if (nvs_open(VS_POVCAL_NAMESPACE, NVS_READWRITE, &nvs) != ESP_OK)
        return false;
    esp_err_t result = nvs_set_blob(nvs, VS_POVCAL_KEY, vs_povcal_profile, sizeof(vs_povcal_profile));
    if (result == ESP_OK)
        result = nvs_commit(nvs);
    nvs_close(nvs);
    return result == ESP_OK;
}

static void vs_povcal_send_error(const char *code)
{
    char line[64];
    uint32_t generation = vs_povcal_loaded ? vs_povcal_u32(vs_povcal_profile, 8) : 0;
    snprintf(line, sizeof(line), "povcal_error %lu %s", (unsigned long)generation, code);
    vs_host_bridge_send(line, NULL, 0);
}

static void vs_povcal_send_state(void)
{
    if (!vs_povcal_loaded && !vs_povcal_load()) {
        vs_povcal_send_error("profile_unavailable");
        return;
    }
    char line[80];
    snprintf(line, sizeof(line), "povcal_state %d %lu %u", 1,
             (unsigned long)vs_povcal_u32(vs_povcal_profile, 8),
             (unsigned)sizeof(vs_povcal_profile));
    vs_host_bridge_send(line, vs_povcal_profile, sizeof(vs_povcal_profile));
}

static bool vs_povcal_number(const char *value, int minimum, int maximum, int *out)
{
    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (end == value || *end != '\0' || parsed < minimum || parsed > maximum)
        return false;
    *out = (int)parsed;
    return true;
}

static bool vs_povcal_apply_set(const char *command)
{
    if (!vs_povcal_loaded && !vs_povcal_load())
        return false;
    uint8_t candidate[COLOR_PIPELINE_PROFILE_BYTES];
    memcpy(candidate, vs_povcal_profile, sizeof(candidate));
    int a, b, c;
    const char *value;

    if (strcmp(command, "source_eotf srgb") == 0) {
        candidate[VS_POVCAL_SOURCE_EOTF] = 0;
    } else if (strncmp(command, "source_eotf power ", 18) == 0) {
        value = command + 18;
        if (!vs_povcal_number(value, 1000, 4000, &a)) return false;
        candidate[VS_POVCAL_SOURCE_EOTF] = 1;
        vs_povcal_put_u16(candidate, VS_POVCAL_SOURCE_GAMMA, a);
    } else if (strncmp(command, "master ", 7) == 0) {
        if (!vs_povcal_number(command + 7, 0, 4000, &a)) return false;
        vs_povcal_put_u16(candidate, VS_POVCAL_MASTER, a);
    } else if (sscanf(command, "white %d %d %d", &a, &b, &c) == 3) {
        if (a < 0 || a > 4000 || b < 0 || b > 4000 || c < 0 || c > 4000) return false;
        vs_povcal_put_u16(candidate, VS_POVCAL_WHITE, a);
        vs_povcal_put_u16(candidate, VS_POVCAL_WHITE + 2, b);
        vs_povcal_put_u16(candidate, VS_POVCAL_WHITE + 4, c);
    } else if (strncmp(command, "radial_exponent ", 16) == 0) {
        if (!vs_povcal_number(command + 16, 0, 4000, &a)) return false;
        vs_povcal_put_u16(candidate, VS_POVCAL_RADIAL, a);
    } else if (sscanf(command, "led_gain %d %d", &a, &b) == 2) {
        if (a < 0 || a >= COLOR_PIPELINE_LEDS || b < 0 || b > 4096) return false;
        vs_povcal_put_u16(candidate, VS_POVCAL_LED_TRIMS + a * 2, b);
    } else if (strncmp(command, "gb_floor ", 9) == 0) {
        if (!vs_povcal_number(command + 9, 0, 31, &a) || a > candidate[VS_POVCAL_GB_CEILING]) return false;
        candidate[VS_POVCAL_GB_FLOOR] = a;
    } else if (strncmp(command, "gb_ceiling ", 11) == 0) {
        if (!vs_povcal_number(command + 11, 0, 31, &a) || a < candidate[VS_POVCAL_GB_FLOOR]) return false;
        candidate[VS_POVCAL_GB_CEILING] = a;
    } else {
        return false;
    }

    vs_povcal_put_u32(candidate, 8, vs_povcal_u32(candidate, 8) + 1);
    if (!color_pipeline_apply(candidate, sizeof(candidate)))
        return false;
    memcpy(vs_povcal_profile, candidate, sizeof(candidate));
    return true;
}

static bool vs_povcal_apply_test(const char *command)
{
    int level = 255;
    const char *level_at = strchr(command, ' ');
    char name[16];
    size_t name_len = level_at ? (size_t)(level_at - command) : strlen(command);
    if (name_len == 0 || name_len >= sizeof(name)) return false;
    memcpy(name, command, name_len);
    name[name_len] = '\0';
    if (level_at && !vs_povcal_number(level_at + 1, 0, 255, &level)) return false;
    int pattern;
    if (strcmp(name, "off") == 0) pattern = COLOR_TEST_OFF;
    else if (strcmp(name, "gray") == 0) pattern = COLOR_TEST_GRAY;
    else if (strcmp(name, "red") == 0) pattern = COLOR_TEST_RED;
    else if (strcmp(name, "green") == 0) pattern = COLOR_TEST_GREEN;
    else if (strcmp(name, "blue") == 0) pattern = COLOR_TEST_BLUE;
    else if (strcmp(name, "white") == 0) pattern = COLOR_TEST_WHITE;
    else if (strcmp(name, "radial") == 0) pattern = COLOR_TEST_RADIAL;
    else return false;
    return color_pipeline_set_test_pattern(pattern, level);
}

// On-device POV render-timing profiler (see
// docs/internals/input-protocol-v2.md and ventilastation_pov.h). Reuses the
// exact same "povperf" wire commands as the MicroPython GPU task profiler so
// one host-side tool profiles both firmwares.
static void vs_povperf_send_error(const char *code)
{
    char line[48];
    snprintf(line, sizeof(line), "povperf_error %s", code);
    vs_host_bridge_send(line, NULL, 0);
}

static void vs_povperf_send_stats(void)
{
    rg_vs_pov_performance_stats_t stats;
    rg_vs_pov_get_performance_stats(&stats);
    int complete = stats.samples && !stats.skipped_updates && !stats.deadline_misses;

    // Worst case for the timing line below: 175 literal chars + 11 %lu (10
    // digits each) + 1 %ld (11 digits incl. sign) + NUL = 297 bytes.
    char line[320];
    snprintf(line, sizeof(line),
        "povperf_state enabled=%d encoder=%s scene=voom complete=%d",
        stats.enabled ? 1 : 0, stats.calibrated ? "calibrated" : "legacy", complete);
    vs_host_bridge_send(line, NULL, 0);

    snprintf(line, sizeof(line),
        "povperf_timing samples=%lu deadline_us=%lu skipped=%lu overruns=%lu "
        "avg_total_us=%lu max_total_us=%lu avg_project_us=%lu max_project_us=%lu "
        "max_arm_project_us=%lu avg_spi_us=%lu max_spi_us=%lu worst_slack_us=%ld",
        (unsigned long)stats.samples, (unsigned long)stats.deadline_us,
        (unsigned long)stats.skipped_updates, (unsigned long)stats.deadline_misses,
        (unsigned long)stats.avg_total_us, (unsigned long)stats.max_total_us,
        (unsigned long)stats.avg_project_us, (unsigned long)stats.max_project_us,
        (unsigned long)stats.max_arm_project_us, (unsigned long)stats.avg_spi_us,
        (unsigned long)stats.max_spi_us, (long)stats.worst_slack_us);
    vs_host_bridge_send(line, NULL, 0);
}

// Diagnostic A/B toggle between the gated hall-pulse filter and the
// pre-filter raw passthrough (hall_filter.h / ventilastation_pov.c), reusing
// the same "hallfilter" wire command as the MicroPython side
// (hall_filter_control.py) so one host-side control (the emulator's F5 key)
// reaches whichever firmware is actually running.
static void vs_hallfilter_send_state(void)
{
    rg_vs_pov_hall_filter_stats_t stats;
    rg_vs_pov_get_hall_filter_stats(&stats);
    char line[224];
    snprintf(line, sizeof(line),
        "hallfilter_state enabled=%d period_us=%lld jitter_us=%lld "
        "accepted=%lu spurious=%lu missed=%lu missed_pulses=%lu outlier=%lu stall=%lu resync=%lu",
        stats.enabled ? 1 : 0, (long long)stats.period_us, (long long)stats.jitter_us,
        (unsigned long)stats.accepted, (unsigned long)stats.spurious,
        (unsigned long)stats.missed, (unsigned long)stats.missed_pulses,
        (unsigned long)stats.outlier, (unsigned long)stats.stall, (unsigned long)stats.resync);
    vs_host_bridge_send(line, NULL, 0);
}

// Shared by the "reset"/"exit" text commands and RESYNC (see
// docs/internals/input-protocol-v2.md#resync--device-identification):
// freeze+fade the current frame so every shared retro-core/fMSX/prboom game
// stops cleanly before its partition restarts.
static void vs_reset_and_restart(void)
{
    rg_audio_set_mute(true);
    rg_display_fade_last_frame_to_black(500);
    rg_system_restart();
}

static void vs_handle_command(const char *cmd)
{
    if (strcmp(cmd, "povcal get") == 0) {
        vs_povcal_send_state();
        return;
    }

    if (strncmp(cmd, "povcal set ", 11) == 0) {
        if (vs_povcal_apply_set(cmd + 11))
            vs_povcal_send_state();
        else
            vs_povcal_send_error("invalid_value");
        return;
    }

    if (strncmp(cmd, "povcal test ", 12) == 0) {
        if (vs_povcal_apply_test(cmd + 12))
            vs_povcal_send_state();
        else
            vs_povcal_send_error("invalid_test");
        return;
    }

    if (strcmp(cmd, "povcal commit") == 0) {
        if (vs_povcal_loaded && vs_povcal_save())
            vs_povcal_send_state();
        else
            vs_povcal_send_error("nvs_write_failed");
        return;
    }

    if (strcmp(cmd, "povcal revert") == 0) {
        if (vs_povcal_load())
            vs_povcal_send_state();
        else
            vs_povcal_send_error("profile_unavailable");
        return;
    }

    if (strcmp(cmd, "povcal factory") == 0) {
        uint32_t generation = vs_povcal_loaded ? vs_povcal_u32(vs_povcal_profile, 8) + 1 : 0;
        if (color_pipeline_build_default(vs_povcal_profile, sizeof(vs_povcal_profile), generation)
            && color_pipeline_apply(vs_povcal_profile, sizeof(vs_povcal_profile))) {
            vs_povcal_loaded = true;
            vs_povcal_send_state();
        } else {
            vs_povcal_send_error("factory_failed");
        }
        return;
    }

    if (strcmp(cmd, "povperf status") == 0) {
        vs_povperf_send_stats();
        return;
    }

    if (strcmp(cmd, "povperf start") == 0) {
        rg_vs_pov_reset_performance_stats();
        rg_vs_pov_set_performance_profiling(true);
        vs_povperf_send_stats();
        return;
    }

    if (strcmp(cmd, "povperf stop") == 0) {
        rg_vs_pov_set_performance_profiling(false);
        vs_povperf_send_stats();
        return;
    }

    if (strcmp(cmd, "povperf reset") == 0) {
        rg_vs_pov_reset_performance_stats();
        vs_povperf_send_stats();
        return;
    }

    if (strcmp(cmd, "povperf mode legacy") == 0 || strcmp(cmd, "povperf mode calibrated") == 0) {
        if (!color_pipeline_set_enabled(strcmp(cmd, "povperf mode calibrated") == 0)) {
            vs_povperf_send_error("profile_unavailable");
            return;
        }
        rg_vs_pov_reset_performance_stats();
        vs_povperf_send_stats();
        return;
    }

    if (strcmp(cmd, "hallfilter status") == 0) {
        vs_hallfilter_send_state();
        return;
    }

    if (strcmp(cmd, "hallfilter on") == 0) {
        rg_vs_pov_set_hall_filter_enabled(true);
        vs_hallfilter_send_state();
        return;
    }

    if (strcmp(cmd, "hallfilter off") == 0) {
        rg_vs_pov_set_hall_filter_enabled(false);
        vs_hallfilter_send_state();
        return;
    }

    if (strcmp(cmd, "hallfilter toggle") == 0) {
        rg_vs_pov_set_hall_filter_enabled(!rg_vs_pov_get_hall_filter_enabled());
        vs_hallfilter_send_state();
        return;
    }

    // EXIT is the emulator's Home/Guide action.  Both it and RESET return to
    // MicroPython, but first freeze the current game frame and sweep it black
    // from the outer LEDs to the centre.  Blocking here stops every shared
    // retro-core, fMSX, and prboom game before its partition restarts.
    if (strcmp(cmd, "reset") == 0 || strcmp(cmd, "exit") == 0)
    {
        vs_reset_and_restart();
        return;
    }

    if (strncmp(cmd, "ota_start ", 10) == 0)
    {
        const char *url = cmd + 10;
        FILE *f = fopen(RG_STORAGE_ROOT "/ota_request", "w");
        if (f)
        {
            fputs(url, f);
            fclose(f);
        }
        rg_system_restart();
        return;
    }

    // Every other command (eg. wifi_config) is silently ignored -- native
    // apps only care about joystick state, reset, OTA, and the
    // povcal/povperf/hallfilter calibration, profiling, and diagnostic
    // commands handled above.
}

static void vs_feed_bytes(const uint8_t *data, int n)
{
    for (int i = 0; i < n; ++i)
    {
        uint8_t b = data[i];

        // Track a possible RESYNC match in parallel with normal parsing, not
        // instead of it -- see input_parser.py's feed() for the full
        // rationale (0x0A is both the marker's first byte and a legitimate
        // command terminator/raw joystick payload byte, so a partial or
        // failed match must not swallow it). Only the byte that completes
        // the match is consumed here instead of falling through below.
        if (b == VS_RESYNC_SEQUENCE[vs_resync_match])
        {
            vs_resync_match++;
            if (vs_resync_match == (int)VS_RESYNC_LEN)
            {
                // Deliberately not resetting vs_state/vs_cmd_len here: unlike
                // input_parser.py (which sets a flag its caller acts on
                // later), vs_reset_and_restart() reboots immediately and
                // never returns, which already wipes every static in this
                // file. The explicit reset below and the `return` are just
                // defensive in case that assumption ever changes.
                vs_resync_match = 0;
                vs_reset_and_restart();
                return;
            }
            // Not yet complete: fall through to the switch below, exactly
            // like input_parser.py -- this byte is still processed normally
            // (e.g. the marker's leading '\n' terminates whatever command
            // was in flight, same as a real newline would).
        }
        else
        {
            vs_resync_match = (b == VS_RESYNC_SEQUENCE[0]) ? 1 : 0;
        }

        switch (vs_state)
        {
        case VS_SCAN:
            if (b == 0x2A)
            {
                vs_state = VS_JOY;
                vs_joy_pos = 0;
            }
            else if ((b >= '0' && b <= '9') || (b >= 'A' && b <= 'Z') || (b >= 'a' && b <= 'z'))
            {
                vs_state = VS_CMD;
                vs_cmd_len = 0;
                vs_cmd_buf[vs_cmd_len++] = (char)b;
            }
            break;

        case VS_JOY:
            vs_joy_buf[vs_joy_pos++] = b;
            if (vs_joy_pos == 3)
            {
                vs_joy1 = vs_joy_buf[0] & 0x7F;
                vs_joy2 = vs_joy_buf[1] & 0x7F;
                vs_extra = vs_joy_buf[2] & 0x7F;
                vs_state = VS_SCAN;
            }
            break;

        case VS_CMD:
            if (b == 0x0A)
            {
                vs_cmd_buf[vs_cmd_len] = '\0';
                if (vs_cmd_len > 0)
                    vs_handle_command(vs_cmd_buf);
                vs_state = VS_SCAN;
            }
            else if (vs_cmd_len >= VS_CMD_MAX)
            {
                vs_state = VS_SCAN; // drop an over-long command, resync on next '*'/alnum
            }
            else
            {
                vs_cmd_buf[vs_cmd_len++] = (char)b;
            }
            break;
        }
    }
}

uint8_t vs_host_bridge_get_joy2(void)
{
    return vs_joy2;
}

uint8_t vs_host_bridge_get_extra(void)
{
    return vs_extra;
}


#include <driver/gpio.h>
#include <driver/uart.h>

#define VS_HOST_RX_BUF 256
#define VS_HOST_TX_RINGBUF 8192

// VS_ROTOR_GIT_HASH is defined by components/retro-go/CMakeLists.txt from
// ESP-IDF's own PROJECT_VER (git-describe-based; see that file), so this
// falls back to "unknown" only for a build that doesn't define it.
#ifndef VS_ROTOR_GIT_HASH
#define VS_ROTOR_GIT_HASH "unknown"
#endif
#define VS_ROTOR_VERSION "v1.0"

static bool bridge_initialized = false;
static vs_board_config_t board_config;

void vs_host_bridge_init(void)
{
    if (bridge_initialized)
        return;
    if (!vs_board_config_load(&board_config)) {
        RG_LOGE("vs_host: board configuration missing; run make configure-board\n");
        return;
    }
    bridge_initialized = true;

    const uart_config_t cfg = {
        .baud_rate = board_config.serial_baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    if (!uart_is_driver_installed(board_config.serial_uart))
    {
        ESP_ERROR_CHECK(uart_driver_install(board_config.serial_uart, VS_HOST_RX_BUF, VS_HOST_TX_RINGBUF, 0, NULL, 0));
        ESP_ERROR_CHECK(uart_param_config(board_config.serial_uart, &cfg));
        ESP_ERROR_CHECK(uart_set_pin(board_config.serial_uart, board_config.serial_tx, board_config.serial_rx,
                                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    }

    // RESYNC identification banner (see
    // docs/internals/input-protocol-v2.md#resync--device-identification):
    // the first thing this bridge puts on the wire, since this is the
    // earliest point any native app touches the host UART. Raw
    // uart_write_bytes, not a logging macro -- RG_LOGI's own prefix/newline
    // handling would make the line unrecognizable to a RESYNC prober.
    static const char banner[] = "VENTILASTATION ROTOR " VS_ROTOR_VERSION " " VS_ROTOR_GIT_HASH "\n";
    uart_write_bytes(board_config.serial_uart, banner, sizeof(banner) - 1);

    RG_LOGI("vs_host: UART%d host link up (tx=%d rx=%d, %d baud)\n",
            (int)board_config.serial_uart, (int)board_config.serial_tx,
            (int)board_config.serial_rx, (int)board_config.serial_baud);
}

void vs_host_bridge_send(const char *line, const uint8_t *data, size_t len)
{
    vs_host_bridge_init();
    if (!bridge_initialized)
        return;
    uart_write_bytes(board_config.serial_uart, line, strlen(line));
    uart_write_bytes(board_config.serial_uart, "\n", 1);
    if (data && len)
        uart_write_bytes(board_config.serial_uart, (const char *)data, len);
}

int vs_host_bridge_recv_input(void)
{
    vs_host_bridge_init();
    if (!bridge_initialized)
        return -1;

    uint8_t buf[64];
    int n;
    while ((n = uart_read_bytes(board_config.serial_uart, buf, sizeof(buf), 0)) > 0)
        vs_feed_bytes(buf, n);

    return (int)vs_joy1;
}

bool vs_host_bridge_connected(void)
{
    vs_host_bridge_init();
    return bridge_initialized;
}


#else

void vs_host_bridge_init(void) {}

void vs_host_bridge_send(const char *line, const uint8_t *data, size_t len)
{
    (void)line;
    (void)data;
    (void)len;
}

int vs_host_bridge_recv_input(void)
{
    return -1;
}

bool vs_host_bridge_connected(void)
{
    return false;
}

uint8_t vs_host_bridge_get_joy2(void)
{
    return 0;
}

uint8_t vs_host_bridge_get_extra(void)
{
    return 0;
}

#endif
