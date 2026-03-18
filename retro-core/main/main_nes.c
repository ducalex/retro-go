#include "cheat.h"
#include "fceu.h"
#include "nes_palettes.h"
#include "rom_manager.h"
#include "shared.h"
#include <fceumm.h>
#include <stdio.h>
#include <stdlib.h>
#include <streams/memory_stream.h>
#include <string.h>

static const char *SETTING_PALETTE = "palette";

#define NES_WIDTH 256
#define NES_HEIGHT 240

// --- GLOBALS
static rg_app_t *app;
static rg_surface_t *updates[3];
static rg_surface_t *currentUpdate;
static int currentBufferIndex = 0;
static bool slowFrame = false;

#ifndef RG_ATTR_EXT_RAM
#define RG_ATTR_EXT_RAM __attribute__((section(".ext_ram.bss")))
#endif

static uint8_t *nes_framebuffer = NULL;
static uint16_t palette565[256];
static uint32_t fceu_joystick;

// Linkage stubs for FCEUMM
unsigned int swapDuty = 0;
void GetKeyboard(void) {}

// Forward declarations for memstream (from libretro-common)
extern void memstream_set_buffer(uint8_t *buffer, uint64_t size);
extern uint64_t memstream_get_last_size(void);

// --- G&W COMPATIBILITY
rom_manager_t rom_mgr;

// Implementation of FCEUSS_Save_Fs and FCEUSS_Load_Fs using memstream
bool FCEUSS_Save_Fs(const char *path) {
  size_t size = 1024 * 512; // 512KB is usually enough for NES
  uint8_t *buffer = malloc(size);
  if (!buffer)
    return false;

  memstream_set_buffer(buffer, size);
  FCEUSS_Save_Mem();
  size_t used = (size_t)memstream_get_last_size();

  bool success = rg_storage_write_file(path, buffer, used, 0);
  free(buffer);
  return success;
}

bool FCEUSS_Load_Fs(const char *path) {
  void *buffer = NULL;
  size_t size = 0;
  if (!rg_storage_read_file(path, &buffer, &size, 0))
    return false;

  memstream_set_buffer((uint8_t *)buffer, size);
  FCEUSS_Load_Mem();
  free(buffer);
  return true;
}

CartInfo *get_cart_info(void) {
  if (iNESCart.mapper >= 0 || iNESCart.PRGRomSize > 0)
    return &iNESCart;
  if (UNIFCart.PRGRomSize > 0)
    return &UNIFCart;
  return NULL;
}

static bool load_sram(void) {
  char *path = rg_emu_get_path(RG_PATH_SAVE_SRAM, app->romPath);
  if (!path)
    return false;

  void *buffer = NULL;
  size_t size = 0;
  if (!rg_storage_read_file(path, &buffer, &size, 0)) {
    free(path);
    return false;
  }

  CartInfo *cart = get_cart_info();
  if (!cart) {
    free(path);
    free(buffer);
    return false;
  }

  bool loaded = false;
  for (int i = 0; i < 4; i++) {
    if (cart->SaveGame[i] && cart->SaveGameLen[i] > 0) {
      size_t len = cart->SaveGameLen[i];
      if (size >= len) {
        memcpy(cart->SaveGame[i], buffer, len);
        loaded = true;
        break; // Only support first slot for now
      }
    }
  }

  free(path);
  free(buffer);
  return loaded;
}

static bool save_sram(void) {
  char *path = rg_emu_get_path(RG_PATH_SAVE_SRAM, app->romPath);
  CartInfo *cart = get_cart_info();
  if (!path)
    return false;
  if (!cart) {
    free(path);
    return false;
  }

  for (int i = 0; i < 4; i++) {
    if (cart->SaveGame[i] && cart->SaveGameLen[i] > 0) {
      bool ret = rg_storage_write_file(path, cart->SaveGame[i],
                                       cart->SaveGameLen[i], 0);
      free(path);
      return ret;
    }
  }
  free(path);
  return false;
}

static void update_audio(int32_t *samples, size_t count) {
  if (count <= 0 || !samples)
    return;

  static rg_audio_frame_t audio_buf[2048];
  if (count > 2048)
    count = 2048;

  for (size_t i = 0; i < count; i++) {
    int32_t s = samples[i] * 4; // Increased to 4x boost
    if (s > 32767)
      s = 32767;
    else if (s < -32768)
      s = -32768;
    audio_buf[i].left = (int16_t)s;
    audio_buf[i].right = (int16_t)s;
  }

  rg_audio_submit(audio_buf, count);
}

// --- FCEU CALLBACKS
void FCEUD_PrintError(char *c) { RG_LOGE("%s\n", c); }
void FCEUD_Message(char *s) { RG_LOGI("%s\n", s); }
void FCEUD_DispMessage(enum retro_log_level level, unsigned duration,
                       const char *str) {
  RG_LOGI("%s\n", str);
}

void FCEUD_SetPalette(uint8 index, uint8 r, uint8 g, uint8 b) {
  // Store as 565 Big-Endian for hardware optimized blitting
  uint16_t color = (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
  palette565[index] = (color >> 8) | (color << 8);
}

// Game Genie Stubs
void FCEU_OpenGenie(void) {}
void FCEU_CloseGenie(void) {}
void FCEU_GeniePower(void) {}

// --- HANDLERS
static void event_handler(int event, void *arg) {
  if (event == RG_EVENT_REDRAW) {
    rg_display_clear(C_BLACK);
  } else if (event == RG_EVENT_SHUTDOWN || event == RG_EVENT_SLEEP) {
    save_sram();
  }
}

static bool screenshot_handler(const char *filename, int width, int height) {
  return rg_surface_save_image_file(currentUpdate, filename, width, height);
}

static bool save_state_handler(const char *filename) {
  save_sram(); // Good time to save SRAM too
  return FCEUSS_Save_Fs(filename);
}

static bool load_state_handler(const char *filename) {
  return FCEUSS_Load_Fs(filename);
}

static bool reset_handler(bool hard) {
  if (hard)
    FCEUI_PowerNES();
  else
    FCEUI_ResetNES();
  return true;
}

// --- CHEATS
static void apply_cheat_code(const char *code, const char *name, int status) {
  if (!code || strlen(code) < 6)
    return;

  uint16 a_16;
  uint8 v;
  int comp;

  if (!FCEUI_DecodeGG(code, &a_16, &v, &comp)) {
    int type = 0;
    if (!FCEUI_DecodePAR(code, &a_16, &v, &comp, &type)) {
      RG_LOGE("Invalid cheat code: %s\n", code);
      return;
    }
  }

  uint32 a = a_16;

  // Use description format: "NAME|CODE"
  char full_desc[128];
  snprintf(full_desc, sizeof(full_desc), "%s|%s", name ? name : "Cheat", code);
  FCEUI_AddCheat(full_desc, a, v, comp,
                 1); // Always use type 1 (Sub) for GG/PAR

  // Set initial status
  // We identify the cheat by its index, which is the last one added.
  int total = 0;
  while (FCEUI_GetCheat(total, NULL, NULL, NULL, NULL, NULL, NULL))
    total++;

  if (total > 0) {
    FCEUI_SetCheat(total - 1, NULL, -1, -1, -1, status, 1);
  }
}

static void load_cheats(void) {
  char *path = rg_emu_get_path(RG_PATH_SAVE_SRAM, app->romPath);
  if (!path)
    return;

  char *saves_str = strstr(path, "saves");
  if (saves_str) {
    memcpy(saves_str, "cheat", 5);
  }

  char *ext = strrchr(path, '.');
  if (ext)
    strcpy(ext, ".cht");

  void *buffer = NULL;
  size_t size = 0;
  if (!rg_storage_read_file(path, &buffer, &size, 0)) {
    free(path);
    return;
  }

  FCEU_ResetCheats();

  char *line = strtok((char *)buffer, "\r\n");
  while (line) {
    char *sep1 = strchr(line, '|');
    if (sep1) {
      *sep1 = 0;
      char *name = line;
      char *code_part = sep1 + 1;
      int status = 1; // Default to ON

      char *sep2 = strchr(code_part, '|');
      if (sep2) {
        *sep2 = 0;
        char *status_str = sep2 + 1;
        if (strcmp(status_str, "OFF") == 0)
          status = 0;
      }
      apply_cheat_code(code_part, name, status);
    }
    line = strtok(NULL, "\r\n");
  }

  free(buffer);
  free(path);
}

static void save_cheats(void) {
  char *path = rg_emu_get_path(RG_PATH_SAVE_SRAM, app->romPath);
  if (!path)
    return;

  char *saves_str = strstr(path, "saves");
  if (saves_str) {
    memcpy(saves_str, "cheat", 5);
  }
  
  rg_storage_mkdir(rg_dirname(path));

  char *ext = strrchr(path, '.');
  if (ext)
    strcpy(ext, ".cht");

  char *buffer = malloc(8192);
  if (!buffer) {
    free(path);
    return;
  }
  buffer[0] = 0;

  for (int i = 0; i < 64; i++) {
    uint32 a;
    uint8 v;
    int s, t, comp;
    char *full_name = NULL;
    if (!FCEUI_GetCheat(i, &full_name, &a, &v, &comp, &s, &t))
      break;

    if (full_name) {
      strcat(buffer, full_name);
      strcat(buffer, s ? "|ON" : "|OFF");
      strcat(buffer, "\n");
    }
  }

  if (strlen(buffer) > 0) {
    rg_storage_write_file(path, buffer, strlen(buffer), 0);
  } else {
    rg_storage_delete(path);
  }

  free(buffer);
  free(path);
}

static int last_cheat_sel = 0;
static rg_gui_event_t cheat_toggle_cb(rg_gui_option_t *opt,
                                      rg_gui_event_t event) {
  if (!opt)
    return RG_DIALOG_VOID;

  int index = (int)opt->arg;
  uint32 a;
  uint8 v;
  int s, t, comp;
  char *name = NULL;

  if (event == RG_DIALOG_INIT || event == RG_DIALOG_UPDATE) {
    if (opt->value && FCEUI_GetCheat(index, &name, &a, &v, &comp, &s, &t)) {
      strcpy(opt->value, s ? "ON" : "OFF");
    }
    return RG_DIALOG_VOID;
  }

  if (event != RG_DIALOG_ENTER && event != RG_DIALOG_SELECT)
    return RG_DIALOG_VOID;

  if (FCEUI_GetCheat(index, &name, &a, &v, &comp, &s, &t)) {
    FCEUI_SetCheat(index, NULL, -1, -1, -1, !s, t);
    return RG_DIALOG_UPDATE;
  }
  return RG_DIALOG_VOID;
}

static void handle_cheat_menu(void) {
  static rg_gui_option_t choices[32];
  static char choices_names[32][64];

  while (true) {
    int count = 0;
    for (int i = 0; i < 30; i++) {
      uint32 a;
      uint8 v;
      int s, t, comp;
      char *full_name = NULL;
      if (!FCEUI_GetCheat(i, &full_name, &a, &v, &comp, &s, &t))
        break;

      char *sep = strchr(full_name, '|');
      if (sep) {
        size_t len = sep - full_name;
        if (len > 60)
          len = 60;
        strncpy(choices_names[count], full_name, len);
        choices_names[count][len] = 0;
      } else {
        strncpy(choices_names[count], full_name, 63);
        choices_names[count][63] = 0;
      }
      char *display_name = choices_names[count];

      choices[count].flags = RG_DIALOG_FLAG_NORMAL;
      choices[count].label = display_name;
      choices[count].value = s ? "ON" : "OFF";
      choices[count].update_cb = cheat_toggle_cb;
      choices[count].arg = (intptr_t)i;
      count++;
    }

    if (count == 0) {
      rg_gui_alert("Cheats",
                   "No cheats active. Use 'Load Cheats' or 'Add Cheats'.");
      break;
    }

    choices[count++] = (rg_gui_option_t)RG_DIALOG_END;

    intptr_t sel_arg = rg_gui_dialog("Cheats Menu", choices, last_cheat_sel);

    if (sel_arg == RG_DIALOG_CANCELLED)
      break;
  }
}

static void handle_add_cheat_menu(void) {
  char *code = rg_gui_input_str("Add Cheat", "Enter Code (GG/PAR)", "");
  if (code) {
    char *name = rg_gui_input_str("Add Cheat", "Enter Description", "");
    if (name) {
      apply_cheat_code(code, name, 1);
      rg_gui_alert("Add Cheat", "Cheat added successfully.");
      free(name);
    }
    free(code);
  }
}

static void handle_delete_cheat_menu(void) {
  static rg_gui_option_t choices[32];
  static char choices_names[32][64];

  while (true) {
    int count = 0;
    for (int i = 0; i < 30; i++) {
      uint32 a;
      uint8 v;
      int s, t, comp;
      char *full_name = NULL;
      if (!FCEUI_GetCheat(i, &full_name, &a, &v, &comp, &s, &t))
        break;

      char *sep = strchr(full_name, '|');
      if (sep) {
        size_t len = sep - full_name;
        if (len > 60)
          len = 60;
        strncpy(choices_names[count], full_name, len);
        choices_names[count][len] = 0;
      } else {
        strncpy(choices_names[count], full_name, 63);
        choices_names[count][63] = 0;
      }
      char *display_name = choices_names[count];

      choices[count].flags = RG_DIALOG_FLAG_NORMAL;
      choices[count].label = display_name;
      choices[count].value = NULL;
      choices[count].arg = (intptr_t)i;
      count++;
    }

    if (count == 0) {
      rg_gui_alert("Delete Cheats", "No cheats to delete.");
      break;
    }

    choices[count++] = (rg_gui_option_t)RG_DIALOG_END;

    intptr_t sel_arg = rg_gui_dialog("Select Cheat to Delete", choices, 0);

    if (sel_arg == RG_DIALOG_CANCELLED)
      break;

    if (sel_arg >= 0 && sel_arg < 30) {
      FCEUI_DelCheat((uint32)sel_arg);
      // Stay in the menu to delete more or show updated list
    }
  }
}

static rg_gui_event_t handle_load_cheats_cb(rg_gui_option_t *opt,
                                            rg_gui_event_t event) {
  if (event == RG_DIALOG_ENTER) {
    load_cheats();
    rg_gui_alert("Load Cheats", "Cheats loaded from SD Card.");
    return RG_DIALOG_VOID;
  }
  return RG_DIALOG_VOID;
}

static rg_gui_event_t handle_save_cheats_cb(rg_gui_option_t *opt,
                                            rg_gui_event_t event) {
  if (event == RG_DIALOG_ENTER) {
    save_cheats();
    rg_gui_alert("Save Cheats", "Cheats saved to SD Card.");
    return RG_DIALOG_VOID;
  }
  return RG_DIALOG_VOID;
}

static rg_gui_event_t handle_cheat_menu_cb(rg_gui_option_t *opt,
                                           rg_gui_event_t event) {
  if (event == RG_DIALOG_ENTER) {
    handle_cheat_menu();
    return RG_DIALOG_VOID;
  }
  return RG_DIALOG_VOID;
}

static rg_gui_event_t handle_add_cheat_menu_cb(rg_gui_option_t *opt,
                                               rg_gui_event_t event) {
  if (event == RG_DIALOG_ENTER) {
    handle_add_cheat_menu();
    return RG_DIALOG_VOID;
  }
  return RG_DIALOG_VOID;
}

static rg_gui_event_t handle_delete_cheat_menu_cb(rg_gui_option_t *opt,
                                                  rg_gui_event_t event) {
  if (event == RG_DIALOG_ENTER) {
    handle_delete_cheat_menu();
    return RG_DIALOG_VOID;
  }
  return RG_DIALOG_VOID;
}

static void update_palette(nespal_t type) {
  if (type < 0 || type >= NES_PALETTE_COUNT)
    type = 0;

  // FCEUMM expects 64 RGB triplets (192 bytes)
  FCEUI_SetPaletteArray((uint8_t *)nes_palettes[type]);
}

static rg_gui_event_t palette_selection_cb(rg_gui_option_t *option,
                                           rg_gui_event_t event) {
  const char *names[] = {"Nofrendo", "Composite", "Classic",
                         "NTSC",     "PVM",       "Smooth"};
  const char *config_ns = app ? app->configNs : "fceumm";
  int palette = rg_settings_get_number(config_ns, SETTING_PALETTE, 0);

  if (event == RG_DIALOG_INIT) {
    //
  } else if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT ||
             event == RG_DIALOG_ENTER || event == RG_DIALOG_SELECT) {
    if (event == RG_DIALOG_PREV)
      palette = (palette + NES_PALETTE_COUNT - 1) % NES_PALETTE_COUNT;
    else
      palette = (palette + 1) % NES_PALETTE_COUNT;

    rg_settings_set_number(config_ns, SETTING_PALETTE, palette);
    rg_settings_commit();
    update_palette((nespal_t)palette);
    return RG_DIALOG_REDRAW;
  }

  if (option && option->value) {
    strncpy(option->value, names[palette], 15);
    option->value[15] = 0;
  }

  return RG_DIALOG_VOID;
}

// --- GUI CALLBACKS
static void options_handler(rg_gui_option_t *dest) {
  *dest++ = (rg_gui_option_t){.label = "Load Cheats",
                              .flags = RG_DIALOG_FLAG_NORMAL,
                              .update_cb = handle_load_cheats_cb};
  *dest++ = (rg_gui_option_t){.label = "Save Cheats",
                              .flags = RG_DIALOG_FLAG_NORMAL,
                              .update_cb = handle_save_cheats_cb};
  *dest++ = (rg_gui_option_t){.label = "Cheats Menu",
                              .flags = RG_DIALOG_FLAG_NORMAL,
                              .update_cb = handle_cheat_menu_cb};
  *dest++ = (rg_gui_option_t){.label = "Add Cheats",
                              .flags = RG_DIALOG_FLAG_NORMAL,
                              .update_cb = handle_add_cheat_menu_cb};
  *dest++ = (rg_gui_option_t){.label = "Delete Cheats",
                              .flags = RG_DIALOG_FLAG_NORMAL,
                              .update_cb = handle_delete_cheat_menu_cb};
  *dest++ = (rg_gui_option_t){.label = "Color Palette",
                              .value = "-",
                              .flags = RG_DIALOG_FLAG_NORMAL,
                              .update_cb = palette_selection_cb};
  *dest++ = (rg_gui_option_t)RG_DIALOG_END;
}

// FDS Functions from fds.h
extern bool FCEU_FDSIsDiskInserted();
extern void FCEU_FDSInsert(int oride);
extern void FCEU_FDSEject(void);
extern void FCEU_FDSSelect(void);
extern void FCEU_FDSSelect_previous(void);

// --- FCEUMM MAIN
void fceumm_main(void) {
  const rg_handlers_t handlers = {
      .loadState = &load_state_handler,
      .saveState = &save_state_handler,
      .reset = &reset_handler,
      .event = &event_handler,
      .screenshot = &screenshot_handler,
      .options = &options_handler,
  };

  app = rg_system_reinit(AUDIO_SAMPLE_RATE, &handlers, NULL);
  rg_system_set_overclock(1);

  if (!nes_framebuffer) {
    // NES_WIDTH * 312 = 79,872 bytes.
    // Move to PSRAM (MEM_SLOW) to free up Internal RAM.
    nes_framebuffer = rg_alloc(NES_WIDTH * 312, MEM_SLOW);
    if (!nes_framebuffer) {
      RG_PANIC("Failed to allocate NES framebuffer in MEM_SLOW");
    }
  }
  XBuf = nes_framebuffer;

  if (!FCEUI_Initialize()) {
    RG_PANIC("FCEUI_Initialize failed");
  }

  // Load ROM into PSRAM to avoid Internal RAM pressure (especially for large
  // ROMs)
  void *rom_data = NULL;
  size_t rom_size = 0;

  // First, get the file size
  rg_stat_t st = rg_storage_stat(app->romPath);
  if (st.size > 0) {
    rom_size = st.size;
    rom_data = rg_alloc(rom_size, MEM_SLOW); // Allocate in PSRAM
    if (rom_data) {
      if (!rg_storage_read_file(app->romPath, &rom_data, &rom_size,
                                RG_FILE_USER_BUFFER)) {
        free(rom_data);
        rom_data = NULL;
      }
    }
  }

  if (!rom_data) {
    RG_PANIC("Failed to load ROM into PSRAM");
  }

  if (!FCEUI_LoadGame(app->romPath, rom_data, rom_size, NULL)) {
    RG_PANIC("FCEUI_LoadGame failed");
  }

  load_sram();
  load_cheats();

  char *sram_path = rg_emu_get_path(RG_PATH_SAVE_SRAM, app->romPath);
  if (sram_path) {
    rg_storage_mkdir(rg_dirname(sram_path));
    free(sram_path);
  }

  // Region detection
  int slstart, slend;
  int is_pal = FCEUI_GetCurrentVidSystem(&slstart, &slend);
  rg_system_set_tick_rate(60);
  app->frameskip = 0;
  RG_LOGI("Detected %s region (%d lines)\n", is_pal ? "PAL" : "NTSC",
          is_pal ? 312 : 262);

  // Use normal_scanlines (updated by LoadGame) for surface height
  extern unsigned normal_scanlines;
  int surface_height = (normal_scanlines > 0 && normal_scanlines <= 312)
                           ? normal_scanlines
                           : NES_HEIGHT;

  // Use 256 lines to avoid out-of-bounds writes from FCEUMM's PPU (which can
  // write to line 240) and to accommodate PAL games safely. NES_WIDTH (256) is
  // standard for this core.
  updates[0] = rg_surface_create(NES_WIDTH, 256, RG_PIXEL_PAL565_BE, MEM_SLOW);
  updates[1] = rg_surface_create(NES_WIDTH, 256, RG_PIXEL_PAL565_BE, MEM_SLOW);
  updates[2] = rg_surface_create(NES_WIDTH, 256, RG_PIXEL_PAL565_BE, MEM_SLOW);
  currentBufferIndex = 0;
  currentUpdate = updates[currentBufferIndex];

  // Initialize external options
  extern void FCEUI_DisableSpriteLimitation(int a);
  FCEUI_DisableSpriteLimitation(1); // Always disable by default

  FSettings.soundq =
      0; // Use LQ sound path for TARGET_GNW compatibility (FlushEmulateSound)
  FCEUI_Sound(app->sampleRate);
  FCEUI_SetInput(0, SI_GAMEPAD, &fceu_joystick, 0);

  update_palette(
      (nespal_t)rg_settings_get_number(app->configNs, SETTING_PALETTE, 0));

  if (app->bootFlags & RG_BOOT_RESUME) {
    rg_emu_load_state(app->saveSlot);
  }

  app->frameskip = 0;

  static uint32_t joystick_old = 0;
  static bool menu_cancelled = false;
  static bool menu_pressed = false;
  static bool turbo_a_toggled = false;
  static bool turbo_b_toggled = false;
  static int turbo_counter = 0;

  while (!rg_system_exit_called()) {

    uint32_t joystick = rg_input_read_gamepad();
    uint32_t joystick_down = joystick & ~joystick_old;
    uint32_t input_buf = 0;
    turbo_counter++;

    // MENU button modifier logic
    if (joystick & RG_KEY_MENU) {
      // Toggle Turbo A
      if (joystick_down & RG_KEY_A) {
        turbo_a_toggled = !turbo_a_toggled;
        RG_LOGI("Turbo A: %s\n", turbo_a_toggled ? "ON" : "OFF");
      }
      // Toggle Turbo B
      if (joystick_down & RG_KEY_B) {
        turbo_b_toggled = !turbo_b_toggled;
        RG_LOGI("Turbo B: %s\n", turbo_b_toggled ? "ON" : "OFF");
      }
      // FDS: Manual Insert
      if (joystick_down & RG_KEY_UP) {
        FCEUI_FDSInsert(0);
        RG_LOGI("FDS: Disk Inserted\n");
      }
      // FDS: Manual Eject
      if (joystick_down & RG_KEY_DOWN) {
        FCEUI_FDSEject();
        RG_LOGI("FDS: Disk Ejected\n");
      }
      // FDS: Next Side
      if (joystick_down & RG_KEY_SELECT) {
        FCEUI_FDSSelect();
        RG_LOGI("FDS: Switched Side\n");
      }

      // If any other button is pressed while MENU is held, cancel the menu
      // trigger
      if (joystick & ~RG_KEY_MENU) {
        menu_cancelled = true;
      }
      menu_pressed = true;
    } else {
      // Handle Menu release
      if (joystick_old & RG_KEY_MENU) {
        if (!menu_cancelled) {
          save_sram();
          rg_gui_game_menu();
        }
        menu_cancelled = false;
      }
      menu_pressed = false;
    }

    // Process Basic Inputs
    if (!menu_pressed) {
      if (joystick & RG_KEY_UP)
        input_buf |= JOY_UP;
      if (joystick & RG_KEY_DOWN)
        input_buf |= JOY_DOWN;
      if (joystick & RG_KEY_LEFT)
        input_buf |= JOY_LEFT;
      if (joystick & RG_KEY_RIGHT)
        input_buf |= JOY_RIGHT;
      if (joystick & RG_KEY_A) {
        if (!turbo_a_toggled || (turbo_counter & 4))
          input_buf |= JOY_A;
      }
      if (joystick & RG_KEY_B) {
        if (!turbo_b_toggled || (turbo_counter & 4))
          input_buf |= JOY_B;
      }
      if (joystick & RG_KEY_START)
        input_buf |= JOY_START;
      if (joystick & RG_KEY_SELECT)
        input_buf |= JOY_SELECT;
    }

    if (joystick & RG_KEY_OPTION) {
      save_sram();
      rg_gui_options_menu();
    }

    joystick_old = joystick;
    fceu_joystick = input_buf;

    int64_t startTime = rg_system_timer();

    static int skipFrames = 0;
    bool drawFrame = !skipFrames;

    // Switch buffers and set target for next frame
    if (drawFrame) {
      currentBufferIndex = (currentBufferIndex + 1) % 3;
      currentUpdate = updates[currentBufferIndex];
      // Copy palette to avoid race conditions with display task
      memcpy(currentUpdate->palette, palette565, 512);

      currentUpdate->width = NES_WIDTH;
      currentUpdate->height = surface_height;
      currentUpdate->offset = 0;

      XBuf = (uint8_t *)currentUpdate->data;
    }

    uint8_t *gfx = NULL;
    int32_t *sound = NULL;
    int32_t sound_samples = 0;

    // FCEUMM handles internal skipping if we pass the flag
    FCEUI_Emulate(&gfx, &sound, &sound_samples, drawFrame ? 0 : 1);

    if (drawFrame && gfx) {
      slowFrame = !rg_display_sync(false);
      rg_display_submit(currentUpdate, RG_DISPLAY_WRITE_NOSYNC);
    }

    // Audio submission provides the pacing (sync)
    update_audio(sound, sound_samples);

    // Dynamic frame skipping logic
    if (skipFrames == 0) {
      int64_t elapsed = rg_system_timer() - startTime;
      if (elapsed > app->frameTime + 1000) { // Slight jitter allowed
        skipFrames = 1;
      } else if (drawFrame && slowFrame) {
        skipFrames = 1;
      }
    } else {
      skipFrames--;
    }

    rg_system_tick(rg_system_timer() - startTime);
  }

  save_sram();
  FCEUI_CloseGame();
}

extern void nofrendo_main(void);

void nes_main(void) {
  app = rg_system_get_app();

  // FDS always uses FCEUMM
  if (strcmp(app->configNs, "fds") == 0) {
    app->name = "fceumm";
    fceumm_main();
    return;
  }

  // Check boot-specific core (from launcher's choose core)
  int core = (int)rg_settings_get_number(NS_BOOT, "Core", -1);
  if (core == -1) {
    // Check preferred core for NES
    core = (int)rg_settings_get_number(NS_APP, "Core",
                                       0); // 0 = FCEUMM, 1 = Nofrendo
  }

  if (core == 1) {
    app->name = "nofrendo";
    nofrendo_main();
  } else {
    app->name = "fceumm";
    fceumm_main();
  }
}
