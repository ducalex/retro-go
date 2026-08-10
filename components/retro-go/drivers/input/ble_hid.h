// Bluetooth LE HID-over-GATT (HOGP) gamepad host for retro-go.
//
// Enable it by defining RG_GAMEPAD_BLE_MAP in your target's config.h and setting
// ENABLE_BLUETOOTH = True in your target's env.py (that adds the esp-idf `bt`
// component and compiles this driver).
//
// The driver connects to the first BLE HID device it sees, reads its HID Report
// Map at runtime and decodes input reports generically -- there is no per-model
// table. What a target's config.h maps is therefore HID *usages* (button numbers,
// hat switch, sticks), which are stable across controllers, rather than raw bit
// offsets, which are not.
#pragma once

#include <stdbool.h>
#include <stdint.h>

// Normalized source bits reported by the driver. Use these in RG_GAMEPAD_BLE_MAP.
// The d-pad bits are the OR of the HID hat switch and the left analog stick.
#define RG_BLE_DPAD_UP    (1U << 0)
#define RG_BLE_DPAD_RIGHT (1U << 1)
#define RG_BLE_DPAD_DOWN  (1U << 2)
#define RG_BLE_DPAD_LEFT  (1U << 3)
#define RG_BLE_L2         (1U << 4) // Analog left trigger, past half travel
#define RG_BLE_R2         (1U << 5) // Analog right trigger, past half travel

// HID Button usages 1..24 (Usage Page 0x09). Button 1 is the primary/"A" button on
// essentially every gamepad; the rest are vendor-ordered and are NOT necessarily
// contiguous -- the Stadia controller, for instance, declares 1,2,4,5,7,8,11..15,17..20
// and never uses 3, 6, 9, 10 or 16. See the mapping in your target's config.h.
#define RG_BLE_BUTTON(n)  (1U << (5 + (n))) // n = 1..24 -> bits 6..29
#define RG_BLE_BUTTON_MAX 24

void rg_ble_hid_init(void);
void rg_ble_hid_deinit(void);
// Current normalized button state, or 0 when no controller is connected.
uint32_t rg_ble_hid_read(void);
bool rg_ble_hid_connected(void);
// Human readable connection state, for the debug/about menus.
const char *rg_ble_hid_status(void);
