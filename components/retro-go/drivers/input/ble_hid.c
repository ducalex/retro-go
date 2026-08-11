// Bluetooth LE HID-over-GATT (HOGP) gamepad host, built on ESP-IDF's NimBLE stack.
//
// Design notes:
//  - The HID Report Map is fetched from the peer and parsed at runtime, so the driver
//    is not tied to one controller model. Targets map HID *usages* (button numbers,
//    hat switch, left stick) rather than raw report bit offsets.
//  - Everything here runs on the NimBLE host task. rg_input's polling task only ever
//    reads one word (`gamepad_state`), so there is no locking and no blocking call in
//    the input path.
//  - Bonds are persisted in NVS by NimBLE, so a controller that has been paired once
//    reconnects on its own after a power cycle.
#include "rg_system.h"
#include "ble_hid.h"

#if defined(RG_GAMEPAD_BLE_MAP) && defined(ESP_PLATFORM)

#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_store.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

// Provided by NimBLE's NVS-backed key store (declared nowhere public, same as esp-idf's examples)
void ble_store_config_init(void);

#define UUID_SVC_HID          0x1812
#define UUID_CHR_REPORT_MAP   0x2A4B
#define UUID_CHR_REPORT       0x2A4D
#define UUID_CHR_PROTO_MODE   0x2A4E
#define UUID_DSC_CCCD         0x2902
#define UUID_DSC_REPORT_REF   0x2908

#define HID_REPORT_TYPE_INPUT 1

#ifndef RG_BLE_MAX_REPORTS
#define RG_BLE_MAX_REPORTS 6
#endif
#ifndef RG_BLE_MAX_FIELDS
#define RG_BLE_MAX_FIELDS 48
#endif
#define REPORT_MAP_MAX 512
#define USAGE_STACK_MAX 24

// ----------------------------------------------------------------------------
// HID report descriptor parsing
// ----------------------------------------------------------------------------

enum
{
    FIELD_BUTTON,
    FIELD_HAT,
    FIELD_AXIS_X,
    FIELD_AXIS_Y,
    FIELD_TRIGGER_L,
    FIELD_TRIGGER_R,
};

typedef struct
{
    uint8_t report_id;
    uint8_t kind;
    uint8_t bit_size;
    uint16_t bit_offset;
    uint16_t index; // Button number, for FIELD_BUTTON
    int32_t lmin, lmax;
} hid_field_t;

static hid_field_t hid_fields[RG_BLE_MAX_FIELDS];
static int hid_field_count = 0;

static const uint32_t hat_to_dpad[8] = {
    RG_BLE_DPAD_UP,
    RG_BLE_DPAD_UP | RG_BLE_DPAD_RIGHT,
    RG_BLE_DPAD_RIGHT,
    RG_BLE_DPAD_RIGHT | RG_BLE_DPAD_DOWN,
    RG_BLE_DPAD_DOWN,
    RG_BLE_DPAD_DOWN | RG_BLE_DPAD_LEFT,
    RG_BLE_DPAD_LEFT,
    RG_BLE_DPAD_LEFT | RG_BLE_DPAD_UP,
};

static void hid_add_field(uint8_t report_id, uint16_t page, uint32_t usage, uint16_t offset, uint8_t size,
                          int32_t lmin, int32_t lmax)
{
    int kind;

    if (page == 0x09 && usage >= 1 && usage <= RG_BLE_BUTTON_MAX)
        kind = FIELD_BUTTON;
    else if (page == 0x01 && usage == 0x39)
        kind = FIELD_HAT;
    else if (page == 0x01 && usage == 0x30)
        kind = FIELD_AXIS_X;
    else if (page == 0x01 && usage == 0x31)
        kind = FIELD_AXIS_Y;
    else if (page == 0x02 && usage == 0xC5) // Brake
        kind = FIELD_TRIGGER_L;
    else if (page == 0x02 && usage == 0xC4) // Accelerator
        kind = FIELD_TRIGGER_R;
    else
        return; // Not something we can map (right stick, vendor pages, padding, ...)

    if (hid_field_count >= RG_BLE_MAX_FIELDS)
    {
        RG_LOGW("Report map has more fields than we can store, ignoring the rest\n");
        return;
    }

    hid_fields[hid_field_count++] = (hid_field_t){
        .report_id = report_id,
        .kind = (uint8_t)kind,
        .bit_size = size,
        .bit_offset = offset,
        .index = (uint16_t)usage,
        .lmin = lmin,
        .lmax = lmax,
    };
}

// Walks a HID report descriptor and records every field we know how to map.
// Bit offsets are counted per report ID and exclude the report ID byte itself,
// which is what HOGP notifications give us (the ID lives in the Report Reference).
static void hid_parse_report_map(const uint8_t *desc, size_t len)
{
    static uint16_t bit_cursor[256];

    uint32_t usages[USAGE_STACK_MAX];
    int usage_count = 0;
    int32_t usage_min = -1, usage_max = -1;
    int32_t lmin = 0, lmax = 0;
    uint32_t report_size = 0, report_count = 0;
    uint16_t usage_page = 0;
    uint8_t report_id = 0;
    size_t i = 0;

    memset(bit_cursor, 0, sizeof(bit_cursor));
    hid_field_count = 0;

    while (i < len)
    {
        uint8_t head = desc[i++];

        if (head == 0xFE) // Long item: [tag][size][data...] -- nothing we care about
        {
            if (i + 2 > len)
                break;
            size_t skip = desc[i];
            i += 2 + skip;
            continue;
        }

        uint8_t size = head & 0x03;
        uint8_t type = (head >> 2) & 0x03;
        uint8_t tag = (head >> 4) & 0x0F;

        if (size == 3)
            size = 4;
        if (i + size > len)
            break;

        uint32_t uval = 0;
        for (uint8_t k = 0; k < size; ++k)
            uval |= (uint32_t)desc[i + k] << (8 * k);
        i += size;

        int32_t sval = (int32_t)uval;
        if (size == 1 && (uval & 0x80))
            sval = (int32_t)(uval | 0xFFFFFF00u);
        else if (size == 2 && (uval & 0x8000))
            sval = (int32_t)(uval | 0xFFFF0000u);

        if (type == 1) // Global
        {
            switch (tag)
            {
            case 0x00: usage_page = (uint16_t)uval; break;
            case 0x01: lmin = sval; break;
            case 0x02: lmax = (lmin < 0) ? sval : (int32_t)uval; break;
            case 0x07: report_size = uval; break;
            case 0x08: report_id = (uint8_t)uval; break;
            case 0x09: report_count = uval; break;
            default: break;
            }
        }
        else if (type == 2) // Local
        {
            switch (tag)
            {
            case 0x00:
                if (usage_count < USAGE_STACK_MAX)
                    usages[usage_count++] = uval;
                break;
            case 0x01: usage_min = (int32_t)uval; break;
            case 0x02: usage_max = (int32_t)uval; break;
            default: break;
            }
        }
        else if (type == 0) // Main
        {
            if (tag == 0x08) // Input
            {
                bool is_const = (uval & 0x01) != 0;
                bool is_var = (uval & 0x02) != 0;
                uint16_t *cursor = &bit_cursor[report_id];

                for (uint32_t n = 0; n < report_count; ++n)
                {
                    if (!is_const && is_var)
                    {
                        uint32_t usage = 0;
                        uint16_t page = usage_page;

                        if (usage_min >= 0)
                        {
                            uint32_t candidate = (uint32_t)usage_min + n;
                            if (usage_max < 0 || candidate <= (uint32_t)usage_max)
                                usage = candidate;
                        }
                        else if (usage_count > 0)
                        {
                            usage = usages[n < (uint32_t)usage_count ? n : (uint32_t)usage_count - 1];
                        }

                        if (usage > 0xFFFF) // Extended (32-bit) usage carries its own page
                        {
                            page = (uint16_t)(usage >> 16);
                            usage &= 0xFFFF;
                        }

                        if (usage)
                            hid_add_field(report_id, page, usage, *cursor, (uint8_t)report_size, lmin, lmax);
                    }
                    *cursor += (uint16_t)report_size;
                }
            }
            // Every main item clears the local state (usages, min/max)
            usage_count = 0;
            usage_min = usage_max = -1;
        }
    }

    RG_LOGI("Report map parsed: %d bytes, %d mappable fields\n", (int)len, hid_field_count);
}

static uint32_t extract_bits(const uint8_t *data, size_t len, uint16_t offset, uint8_t size)
{
    uint32_t out = 0;
    for (uint8_t k = 0; k < size && k < 32; ++k)
    {
        uint16_t bit = offset + k;
        if ((size_t)(bit >> 3) >= len)
            break;
        if (data[bit >> 3] & (1 << (bit & 7)))
            out |= (1U << k);
    }
    return out;
}

static int32_t sign_extend(uint32_t value, uint8_t bits)
{
    if (bits == 0 || bits >= 32)
        return (int32_t)value;
    uint32_t sign = 1U << (bits - 1);
    return (int32_t)((value ^ sign) - sign);
}

static uint32_t hid_decode_report(uint8_t report_id, const uint8_t *data, size_t len)
{
    uint32_t out = 0;

    for (int i = 0; i < hid_field_count; ++i)
    {
        const hid_field_t *f = &hid_fields[i];
        if (f->report_id != report_id)
            continue;

        uint32_t raw = extract_bits(data, len, f->bit_offset, f->bit_size);
        int32_t span = f->lmax - f->lmin;
        int32_t value = (f->lmin < 0) ? sign_extend(raw, f->bit_size) : (int32_t)raw;

        switch (f->kind)
        {
        case FIELD_BUTTON:
            if (raw)
                out |= RG_BLE_BUTTON(f->index);
            break;

        case FIELD_HAT:
        {
            int32_t dir = value - f->lmin;
            if (dir >= 0 && dir < 8 && span >= 7) // span < 7 means it isn't really an 8-way hat
                out |= hat_to_dpad[dir];
            break;
        }

        case FIELD_AXIS_X:
        case FIELD_AXIS_Y:
        {
            if (span <= 0)
                break;
            int32_t norm = ((value - f->lmin) * 255) / span; // 0..255, centre 128
            if (f->kind == FIELD_AXIS_X)
                out |= (norm < 64) ? RG_BLE_DPAD_LEFT : (norm > 191) ? RG_BLE_DPAD_RIGHT : 0;
            else
                out |= (norm < 64) ? RG_BLE_DPAD_UP : (norm > 191) ? RG_BLE_DPAD_DOWN : 0;
            break;
        }

        case FIELD_TRIGGER_L:
        case FIELD_TRIGGER_R:
            if (span > 0 && (value - f->lmin) * 2 > span)
                out |= (f->kind == FIELD_TRIGGER_L) ? RG_BLE_L2 : RG_BLE_R2;
            break;
        }
    }

    return out;
}

// ----------------------------------------------------------------------------
// GATT client / connection state machine
// ----------------------------------------------------------------------------

enum
{
    STEP_SVC = 0,
    STEP_CHRS,
    STEP_DSCS,
    STEP_REPORT_MAP,
    STEP_REPORT_REFS,
    STEP_PROTO_MODE,
    STEP_SUBSCRIBE,
    STEP_DONE,
};

typedef struct
{
    uint16_t def_handle;
    uint16_t val_handle;
    uint16_t end_handle; // Last attribute belonging to this characteristic
    uint16_t cccd_handle;
    uint16_t ref_handle;
    uint8_t report_id;
    uint8_t report_type;
} report_chr_t;

static struct
{
    volatile uint32_t buttons;
    volatile bool connected;
    const char *status;
    uint8_t own_addr_type;
    uint16_t conn_handle;
    uint16_t svc_start, svc_end;
    uint16_t report_map_handle;
    uint16_t proto_mode_handle;
    report_chr_t reports[RG_BLE_MAX_REPORTS];
    int report_count;
    int step;
    int iter;
    uint8_t report_map[REPORT_MAP_MAX];
    size_t report_map_len;
    bool initialized;
} ble = {
    .conn_handle = BLE_HS_CONN_HANDLE_NONE,
    .status = "off",
};

static int gap_event(struct ble_gap_event *event, void *arg);
static void disc_run_step(void);

// 16-bit value of a UUID, or 0 for the 32/128-bit ones (which we never care about here)
static inline uint16_t uuid16(const ble_uuid_t *uuid)
{
    return (uuid->type == BLE_UUID_TYPE_16) ? ble_uuid_u16(uuid) : 0;
}

static void start_scan(void)
{
    struct ble_gap_disc_params params = {
        .itvl = 0,
        .window = 0,
        .filter_policy = 0,
        .limited = 0,
        .passive = 0, // Active scan: some controllers only advertise 0x1812 in the scan response
        // No duplicate filtering: with an infinite scan, one sighting that arrives without
        // its scan response (frequent under Wi-Fi coexistence) would fail the name filter
        // and then be suppressed forever — pairing would only ever work right after boot.
        .filter_duplicates = 0,
    };

    ble.status = "scanning";

    int rc = ble_gap_disc(ble.own_addr_type, BLE_HS_FOREVER, &params, gap_event, NULL);
    if (rc == BLE_HS_EBUSY || rc == BLE_HS_EPREEMPTED)
    {
        // A connect attempt or a dying scan still owns the radio; clear it and retry once
        ble_gap_conn_cancel();
        ble_gap_disc_cancel();
        rc = ble_gap_disc(ble.own_addr_type, BLE_HS_FOREVER, &params, gap_event, NULL);
    }
    if (rc != 0 && rc != BLE_HS_EALREADY)
    {
        ble.status = "scan failed";
        RG_LOGE("ble_gap_disc failed: %d\n", rc);
    }
    else
    {
        RG_LOGI("BLE scanning for HID devices\n");
    }
}

static void reset_connection_state(void)
{
    ble.conn_handle = BLE_HS_CONN_HANDLE_NONE;
    ble.connected = false;
    ble.buttons = 0;
    ble.report_count = 0;
    ble.report_map_len = 0;
    ble.report_map_handle = 0;
    ble.proto_mode_handle = 0;
    ble.step = STEP_SVC;
    ble.iter = 0;
    hid_field_count = 0;
}

// True if this advertisement looks like a HID gamepad we should try to own.
static bool adv_is_hid(const struct ble_hs_adv_fields *fields)
{
    for (int i = 0; i < fields->num_uuids16; ++i)
        if (ble_uuid_u16(&fields->uuids16[i].u) == UUID_SVC_HID)
            return true;

    // Appearance 0x03Cx is the HID category (0x03C4 = Gamepad, 0x03C0 = generic HID)
    if (fields->appearance_is_present && (fields->appearance & 0xFFC0) == 0x03C0)
        return true;

#ifdef RG_BLE_NAME_FILTER
    static const char *name_filter[] = RG_BLE_NAME_FILTER;
    if (fields->name && fields->name_len)
    {
        char name[32];
        size_t len = RG_MIN(fields->name_len, sizeof(name) - 1);
        memcpy(name, fields->name, len);
        name[len] = 0;
        for (size_t i = 0; i < RG_COUNT(name_filter); ++i)
            if (strstr(name, name_filter[i]))
                return true;
    }
#endif

    return false;
}

static void disc_next(void)
{
    ble.step++;
    ble.iter = 0;
    disc_run_step();
}

static void disc_fail(const char *what, int rc)
{
    RG_LOGE("BLE discovery failed at %s (rc=%d), dropping connection\n", what, rc);
    ble.status = "error";
    if (ble.conn_handle != BLE_HS_CONN_HANDLE_NONE)
        ble_gap_terminate(ble.conn_handle, BLE_ERR_REM_USER_CONN_TERM);
}

static int on_disc_svc(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_svc *svc,
                       void *arg)
{
    if (error->status == 0 && svc)
    {
        ble.svc_start = svc->start_handle;
        ble.svc_end = svc->end_handle;
    }
    else if (error->status == BLE_HS_EDONE)
    {
        if (ble.svc_start == 0)
            disc_fail("no HID service", 0);
        else
            disc_next();
    }
    else
    {
        disc_fail("service discovery", error->status);
    }
    return 0;
}

static int on_disc_chr(uint16_t conn_handle, const struct ble_gatt_error *error, const struct ble_gatt_chr *chr,
                       void *arg)
{
    if (error->status == 0 && chr)
    {
        uint16_t uuid = uuid16(&chr->uuid.u);

        // Any new characteristic closes the previous report's attribute range, so a CCCD
        // belonging to something else (a boot report, say) can't be misattributed to it.
        if (ble.report_count > 0 && ble.reports[ble.report_count - 1].end_handle == 0)
            ble.reports[ble.report_count - 1].end_handle = chr->def_handle - 1;

        if (uuid == UUID_CHR_REPORT_MAP)
        {
            ble.report_map_handle = chr->val_handle;
        }
        else if (uuid == UUID_CHR_PROTO_MODE)
        {
            ble.proto_mode_handle = chr->val_handle;
        }
        else if (uuid == UUID_CHR_REPORT && ble.report_count < RG_BLE_MAX_REPORTS)
        {
            ble.reports[ble.report_count++] = (report_chr_t){
                .def_handle = chr->def_handle,
                .val_handle = chr->val_handle,
                .report_type = 0xFF, // Unknown until we read the Report Reference descriptor
            };
        }
    }
    else if (error->status == BLE_HS_EDONE)
    {
        // The final report characteristic runs to the end of the service
        if (ble.report_count > 0 && ble.reports[ble.report_count - 1].end_handle == 0)
            ble.reports[ble.report_count - 1].end_handle = ble.svc_end;
        RG_LOGI("HID service: %d report characteristic(s), report map handle %d\n", ble.report_count,
                ble.report_map_handle);
        disc_next();
    }
    else
    {
        disc_fail("characteristic discovery", error->status);
    }
    return 0;
}

static int on_disc_dsc(uint16_t conn_handle, const struct ble_gatt_error *error, uint16_t chr_val_handle,
                       const struct ble_gatt_dsc *dsc, void *arg)
{
    if (error->status == 0 && dsc)
    {
        uint16_t uuid = uuid16(&dsc->uuid.u);

        // We discover descriptors across the whole service in one pass, so match each one
        // to the report characteristic whose attribute range actually contains it.
        report_chr_t *owner = NULL;
        for (int i = 0; i < ble.report_count; ++i)
            if (dsc->handle > ble.reports[i].val_handle && dsc->handle <= ble.reports[i].end_handle)
                owner = &ble.reports[i];

        if (owner)
        {
            if (uuid == UUID_DSC_CCCD)
                owner->cccd_handle = dsc->handle;
            else if (uuid == UUID_DSC_REPORT_REF)
                owner->ref_handle = dsc->handle;
        }
    }
    else if (error->status == BLE_HS_EDONE)
    {
        disc_next();
    }
    else
    {
        disc_fail("descriptor discovery", error->status);
    }
    return 0;
}

static int on_read_report_map(uint16_t conn_handle, const struct ble_gatt_error *error,
                              struct ble_gatt_attr *attr, void *arg)
{
    if (error->status == 0 && attr && attr->om)
    {
        uint16_t chunk = OS_MBUF_PKTLEN(attr->om);
        if (ble.report_map_len + chunk <= REPORT_MAP_MAX)
        {
            uint16_t copied = 0;
            ble_hs_mbuf_to_flat(attr->om, ble.report_map + ble.report_map_len, chunk, &copied);
            ble.report_map_len += copied;
        }
        else
        {
            RG_LOGW("Report map larger than %d bytes, truncating\n", REPORT_MAP_MAX);
        }
    }
    else if (error->status == BLE_HS_EDONE)
    {
        hid_parse_report_map(ble.report_map, ble.report_map_len);
        disc_next();
    }
    else
    {
        disc_fail("report map read", error->status);
    }
    return 0;
}

static int on_read_report_ref(uint16_t conn_handle, const struct ble_gatt_error *error,
                              struct ble_gatt_attr *attr, void *arg)
{
    int index = (int)(intptr_t)arg;

    if (error->status == 0 && attr && attr->om && index < ble.report_count)
    {
        uint8_t buf[2] = {0};
        uint16_t copied = 0;
        ble_hs_mbuf_to_flat(attr->om, buf, sizeof(buf), &copied);
        if (copied >= 2)
        {
            ble.reports[index].report_id = buf[0];
            ble.reports[index].report_type = buf[1];
            RG_LOGI("Report characteristic %d: id=%d type=%d\n", index, buf[0], buf[1]);
        }
    }

    // Move on regardless: a report we can't identify is simply one we won't subscribe to.
    ble.iter++;
    disc_run_step();
    return 0;
}

static int on_write_done(uint16_t conn_handle, const struct ble_gatt_error *error, struct ble_gatt_attr *attr,
                         void *arg)
{
    if (ble.step == STEP_PROTO_MODE)
    {
        disc_next();
    }
    else
    {
        ble.iter++;
        disc_run_step();
    }
    return 0;
}

static void disc_run_step(void)
{
    int rc = 0;

    switch (ble.step)
    {
    case STEP_SVC:
        ble.svc_start = ble.svc_end = 0;
        rc = ble_gattc_disc_svc_by_uuid(ble.conn_handle, BLE_UUID16_DECLARE(UUID_SVC_HID), on_disc_svc, NULL);
        break;

    case STEP_CHRS:
        rc = ble_gattc_disc_all_chrs(ble.conn_handle, ble.svc_start, ble.svc_end, on_disc_chr, NULL);
        break;

    case STEP_DSCS:
        if (ble.report_count == 0)
        {
            disc_fail("no report characteristics", 0);
            return;
        }
        rc = ble_gattc_disc_all_dscs(ble.conn_handle, ble.svc_start, ble.svc_end, on_disc_dsc, NULL);
        break;

    case STEP_REPORT_MAP:
        if (!ble.report_map_handle)
        {
            disc_fail("no report map", 0);
            return;
        }
        ble.report_map_len = 0;
        rc = ble_gattc_read_long(ble.conn_handle, ble.report_map_handle, 0, on_read_report_map, NULL);
        break;

    case STEP_REPORT_REFS:
        while (ble.iter < ble.report_count && !ble.reports[ble.iter].ref_handle)
            ble.iter++;
        if (ble.iter >= ble.report_count)
        {
            disc_next();
            return;
        }
        rc = ble_gattc_read(ble.conn_handle, ble.reports[ble.iter].ref_handle, on_read_report_ref,
                            (void *)(intptr_t)ble.iter);
        break;

    case STEP_PROTO_MODE:
        if (!ble.proto_mode_handle)
        {
            disc_next();
            return;
        }
        // 0x01 = Report Protocol. Write-without-response per the HOGP spec.
        {
            static const uint8_t report_protocol = 0x01;
            rc = ble_gattc_write_no_rsp_flat(ble.conn_handle, ble.proto_mode_handle, &report_protocol, 1);
            if (rc == 0)
            {
                disc_next();
                return;
            }
        }
        break;

    case STEP_SUBSCRIBE:
        while (ble.iter < ble.report_count &&
               (ble.reports[ble.iter].report_type != HID_REPORT_TYPE_INPUT || !ble.reports[ble.iter].cccd_handle))
            ble.iter++;
        if (ble.iter >= ble.report_count)
        {
            disc_next();
            return;
        }
        {
            static const uint8_t notify_on[2] = {0x01, 0x00};
            rc = ble_gattc_write_flat(ble.conn_handle, ble.reports[ble.iter].cccd_handle, notify_on,
                                      sizeof(notify_on), on_write_done, NULL);
        }
        break;

    case STEP_DONE:
        ble.connected = true;
        ble.status = "connected";
        RG_LOGI("BLE gamepad ready (%d field(s) mapped)\n", hid_field_count);
        return;

    default:
        return;
    }

    if (rc != 0)
        disc_fail("gatt request", rc);
}

static int gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_hs_adv_fields fields;
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC:
        if (ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data) != 0)
            return 0;
        if (!adv_is_hid(&fields))
            return 0;

        RG_LOGI("Found BLE HID device %02x:%02x:%02x:%02x:%02x:%02x, connecting...\n", event->disc.addr.val[5],
                event->disc.addr.val[4], event->disc.addr.val[3], event->disc.addr.val[2],
                event->disc.addr.val[1], event->disc.addr.val[0]);

        ble_gap_disc_cancel();
        ble.status = "connecting";
        rc = ble_gap_connect(ble.own_addr_type, &event->disc.addr, 10000, NULL, gap_event, NULL);
        if (rc != 0)
        {
            RG_LOGE("ble_gap_connect failed: %d\n", rc);
            start_scan();
        }
        return 0;

    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status != 0)
        {
            RG_LOGW("BLE connection failed (%d), scanning again\n", event->connect.status);
            reset_connection_state();
            start_scan();
            return 0;
        }
        ble.conn_handle = event->connect.conn_handle;
        ble.status = "pairing";
        ble_gattc_exchange_mtu(ble.conn_handle, NULL, NULL);
        // HOGP requires an encrypted link. If the peer is already bonded this just
        // restarts encryption with the stored key.
        rc = ble_gap_security_initiate(ble.conn_handle);
        if (rc != 0)
        {
            RG_LOGW("Security handshake could not be started (%d), trying unencrypted\n", rc);
            ble.step = STEP_SVC;
            ble.iter = 0;
            disc_run_step();
        }
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        // Proceed either way -- a few controllers serve their reports without encryption.
        if (event->enc_change.status != 0)
            RG_LOGW("Encryption failed (%d), trying to read reports anyway\n", event->enc_change.status);
        ble.step = STEP_SVC;
        ble.iter = 0;
        disc_run_step();
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        RG_LOGW("BLE gamepad disconnected (reason %d)\n", event->disconnect.reason);
        reset_connection_state();
        start_scan();
        return 0;

    case BLE_GAP_EVENT_NOTIFY_RX:
    {
        if (!event->notify_rx.om)
            return 0;
        uint8_t data[32];
        uint16_t len = 0;
        if (ble_hs_mbuf_to_flat(event->notify_rx.om, data, sizeof(data), &len) != 0)
            return 0;
        for (int i = 0; i < ble.report_count; ++i)
        {
            if (ble.reports[i].val_handle == event->notify_rx.attr_handle)
            {
                uint32_t buttons = hid_decode_report(ble.reports[i].report_id, data, len);
                if (buttons != ble.buttons)
                {
                    // Logged on change so an unknown controller can be mapped from the serial
                    // monitor without a rebuild: press a button, read the number it reports,
                    // then fix RG_GAMEPAD_BLE_MAP in your target's config.h.
                    char pressed[4 * RG_BLE_BUTTON_MAX], *w = pressed;
                    for (int n = 1; n <= RG_BLE_BUTTON_MAX; ++n)
                        if (buttons & RG_BLE_BUTTON(n))
                            w += sprintf(w, " %d", n);
                    *w = 0;
                    RG_LOGI("BLE gamepad: dpad=%c%c%c%c triggers=%c%c buttons:%s\n",
                            (buttons & RG_BLE_DPAD_UP) ? 'U' : '-', (buttons & RG_BLE_DPAD_DOWN) ? 'D' : '-',
                            (buttons & RG_BLE_DPAD_LEFT) ? 'L' : '-', (buttons & RG_BLE_DPAD_RIGHT) ? 'R' : '-',
                            (buttons & RG_BLE_L2) ? 'L' : '-', (buttons & RG_BLE_R2) ? 'R' : '-', pressed);
                    ble.buttons = buttons;
                }
                break;
            }
        }
        return 0;
    }

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        // The peer wants to pair again but we already have a bond: drop the stale one.
        if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc) == 0)
            ble_store_util_delete_peer(&desc.peer_id_addr);
        return BLE_GAP_REPEAT_PAIRING_RETRY;

    case BLE_GAP_EVENT_MTU:
        RG_LOGI("BLE MTU is now %d\n", event->mtu.value);
        return 0;

    default:
        return 0;
    }
}

static void on_sync(void)
{
    if (ble_hs_util_ensure_addr(0) != 0)
        RG_LOGE("No BLE identity address available!\n");
    if (ble_hs_id_infer_auto(0, &ble.own_addr_type) != 0)
        RG_LOGE("Could not determine BLE address type!\n");
    start_scan();
}

static void on_reset(int reason)
{
    RG_LOGE("BLE host reset, reason %d\n", reason);
    reset_connection_state();
    ble.status = "reset";
}

static void host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

// ----------------------------------------------------------------------------
// Public interface
// ----------------------------------------------------------------------------

void rg_ble_hid_init(void)
{
    if (ble.initialized)
        return;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    err = nimble_port_init();
    if (err != ESP_OK)
    {
        RG_LOGE("nimble_port_init failed: 0x%x -- BLE gamepad disabled\n", err);
        ble.status = "failed";
        return;
    }

    reset_connection_state();

    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT; // "Just Works" pairing
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

    ble_svc_gap_init();
    ble_svc_gap_device_name_set(RG_PROJECT_NAME);
    ble_store_config_init();

    // Status must be set before the host task starts: its sync callback can fire
    // (and set "scanning") before we return, and must not be overwritten here.
    ble.initialized = true;
    ble.status = "starting";

    nimble_port_freertos_init(host_task);

    RG_LOGI("BLE HID gamepad host started\n");
}

void rg_ble_hid_deinit(void)
{
    if (!ble.initialized)
        return;
    if (ble.conn_handle != BLE_HS_CONN_HANDLE_NONE)
        ble_gap_terminate(ble.conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    nimble_port_stop();
    nimble_port_deinit();
    reset_connection_state();
    ble.initialized = false;
    ble.status = "off";
}

uint32_t rg_ble_hid_read(void)
{
    return ble.connected ? ble.buttons : 0;
}

bool rg_ble_hid_connected(void)
{
    return ble.connected;
}

const char *rg_ble_hid_status(void)
{
    return ble.status;
}

void rg_ble_hid_forget_bonds(void)
{
    if (!ble.initialized)
        return;
    // Drop all stored bonds so the next connection pairs from scratch. Cures the
    // stale-key loop where the controller re-entered pairing mode (wiping its own
    // key) while we still encrypt with the old one: "Encryption failed (520)".
    ble_store_clear();
    RG_LOGI("BLE bonds cleared, re-pairing\n");
    ble_gap_conn_cancel(); // abort any in-flight connect attempt (harmless if none)
    if (ble.conn_handle != BLE_HS_CONN_HANDLE_NONE)
        ble_gap_terminate(ble.conn_handle, BLE_ERR_REM_USER_CONN_TERM); // rescan happens on disconnect
    else
        start_scan();
}

#else

// Keep the translation unit non-empty for targets that don't enable BLE input.
void rg_ble_hid_init(void) {}
void rg_ble_hid_deinit(void) {}
uint32_t rg_ble_hid_read(void) { return 0; }
bool rg_ble_hid_connected(void) { return false; }
const char *rg_ble_hid_status(void) { return "disabled"; }
void rg_ble_hid_forget_bonds(void) {}

#endif
