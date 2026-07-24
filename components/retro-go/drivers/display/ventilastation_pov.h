#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rg_surface.h"

// Pull in the target config so RG_VS_ENABLE_POV_DISPLAY is defined before the
// guard below is evaluated. Without this, a translation unit that includes this
// header before rg_system.h/config.h (e.g. ventilastation_pov.c itself, which
// includes its own header first) would see RG_VS_ENABLE_POV_DISPLAY undefined,
// silently compiling the whole POV driver down to no-op stubs.
#include "config.h"

#if defined(ESP_PLATFORM) && defined(RG_VS_ENABLE_POV_DISPLAY) && (RG_VS_ENABLE_POV_DISPLAY)
#define RG_VENTILASTATION_POV_ENABLED 1
#else
#define RG_VENTILASTATION_POV_ENABLED 0
#endif

bool rg_vs_pov_enabled(void);

// Call once from rg_display_init(). Starts the SPI display task and hall sensor.
// Framebuffer allocation happens lazily on the first rg_vs_pov_submit_surface() call.
void rg_vs_pov_init(void);

// Submit a surface for display. Handles all rg_surface_t pixel formats:
// indexed (PAL565_LE/BE, PAL888) and direct RGB (565_LE/BE, 888).
// Updates the hardware SPI path.
void rg_vs_pov_submit_surface(const rg_surface_t *surface);

// Freeze the most recently submitted frame and black its LEDs from the outer
// edge to the centre over duration_ms.  This is used immediately before a
// native app returns to the MicroPython launcher.
void rg_vs_pov_fade_last_frame_to_black(uint32_t duration_ms);

// Legacy: explicit 32-bit palette override. Optional; rg_vs_pov_submit_surface()
// reads the palette from the surface automatically.
void rg_vs_pov_set_palette32(const uint32_t *palette, size_t count);

// Opt-in on-device render-timing profiler for the POV display task, driven by
// the same "povperf" wire commands as the MicroPython GPU task profiler (see
// hardware/rotor/modules/povdisplay/povdisplay.c and
// docs/internals/input-protocol-v2.md) so both firmwares are profiled the
// same way. Per-column timing covers both project_angle() calls (the "arms",
// one per half-turn) and any leftover wait for the SPI transfer to finish
// once rendering is done -- the transfer itself overlaps with rendering the
// next column, so this is normally near zero (see gpu_step() in
// ventilastation_pov.c). skipped_updates counts columns whose angle advanced
// by more than one step between GPU task iterations, i.e. visibly dropped
// columns.
typedef struct {
    bool enabled;
    bool calibrated;
    uint32_t samples;
    uint32_t skipped_updates;
    uint32_t deadline_misses;
    uint32_t deadline_us;
    uint32_t avg_total_us;
    uint32_t max_total_us;
    uint32_t avg_project_us;
    uint32_t max_project_us;
    uint32_t max_arm_project_us;
    uint32_t avg_spi_us;
    uint32_t max_spi_us;
    int32_t worst_slack_us;
} rg_vs_pov_performance_stats_t;

// Enabling resets the sample window first, matching the MicroPython side.
void rg_vs_pov_set_performance_profiling(bool enabled);
void rg_vs_pov_reset_performance_stats(void);
void rg_vs_pov_get_performance_stats(rg_vs_pov_performance_stats_t *out);

// Diagnostic A/B toggle between the gated hall-pulse filter (hall_filter.h)
// and the pre-filter raw passthrough, matching the MicroPython side's
// povdisplay.set_hall_filter_enabled()/get_hall_filter_enabled(). Driven by
// the "hallfilter" wire command in vs_host_bridge.c's vs_handle_command(),
// same as povcal/povperf -- this stays reachable while a native app is
// running (it's a separate in-band dispatcher from MicroPython's director.py,
// not MicroPython itself, which is not resident during that time: launching
// a native app is a full esp_ota_set_boot_partition()+esp_restart(), not
// coexistence -- see native_apps.c).
void rg_vs_pov_set_hall_filter_enabled(bool enabled);
bool rg_vs_pov_get_hall_filter_enabled(void);

typedef struct {
    bool enabled;
    int64_t period_us;
    int64_t jitter_us;
    uint32_t accepted;
    uint32_t spurious;
    uint32_t missed;
    uint32_t missed_pulses;
    uint32_t outlier;
    uint32_t stall;
    uint32_t resync;
} rg_vs_pov_hall_filter_stats_t;

void rg_vs_pov_get_hall_filter_stats(rg_vs_pov_hall_filter_stats_t *out);


// Draws a title plus a short list of option lines directly in the POV's
// native (angle, radius) address space, in a narrow wedge centred on the
// bottom of the disc -- bypassing the Cartesian vs_data/project_angle
// downsample entirely. That downsample is fine for game video but loses far
// too much resolution for text (confirmed on hardware): mirrors how the
// MicroPython ROM browser (system/launcher) keeps its own text legible by
// addressing sprites directly in polar space instead of projecting a
// Cartesian framebuffer. selected_index indexes into `lines` (not the
// caller's full list) and may be -1 for "nothing highlighted".
// Returns false if the POV driver isn't enabled.
bool rg_vs_pov_draw_native_dialog(const char *title, const char *const *lines, int line_count, int selected_index);

// Turns the native dialog overlay off, restoring the plain projected view.
void rg_vs_pov_clear_native_dialog(void);

// Register WiFi display bridge callbacks so the POV driver can forward frames
// and palette to the desktop pyglet emulator. Pass NULLs to disable.
