#include <rg_system.h>
#include <string.h>

#define AUDIO_SAMPLE_RATE (22050)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60 + 1)

static rg_surface_t *updates[2];
static rg_surface_t *currentUpdate;
static rg_app_t *app;

static int JoyState, LastKey, InMenu, InKeyboard;
static int KeyboardCol, KeyboardRow, KeyboardKey;
static int64_t KeyboardDebounce = 0;
static int FrameStartTime;
static int KeyboardEmulation, CropPicture;
static int PrevScanLines212 = -1;
static char *PendingLoadSTA = NULL;

#define BPS16
#define BPP16
#define UNIX
#define GenericSetVideo SetVideo
#define LSB_FIRST
#define NARROW
#define WIDTH 256
#define HEIGHT 228
#define XKEYS 12
#define YKEYS 6

void PutImage(void);

static uint16_t BPal[256];
static uint16_t XPal[80];
static uint16_t XPal0;
static uint16_t *XBuf;

#include "MSX.h"
#include "Console.h"
#include "EMULib.h"
#include "Sound.h"
#include "Record.h"
#include "Touch.h"
#include "CommonMux.h"
#include "msxfix.h"

static Image NormScreen;
const char *Title = "fMSX 6.0";
const char *Disks[2][MAXDISKS + 1];

static const unsigned char KBDKeys[YKEYS][XKEYS] = {
    {0x1B, CON_F1, CON_F2, CON_F3, CON_F4, CON_F5, CON_F6, CON_F7, CON_F8, CON_INSERT, CON_DELETE, CON_STOP},
    {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '='},
    {CON_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', CON_BS},
    {'^', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', CON_ENTER},
    {'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, 0},
    {'[', ']', ' ', ' ', ' ', ' ', ' ', '\\', '\'', 0, 0, 0}};

int ProcessEvents(int Wait)
{
    for (int i = 0; i < 16; ++i)
        KeyState[i] = 0xFF;
    JoyState = 0;

    uint32_t joystick = rg_input_read_gamepad();

    if (joystick & (RG_KEY_MENU | RG_KEY_OPTION))
    {
        if (joystick & RG_KEY_MENU)
            rg_gui_game_menu();
        else
            rg_gui_options_menu();
        return 0;
    }

    if (!InMenu && joystick == RG_KEY_SELECT)
    {
        InMenu = true;
        rg_audio_set_mute(true);
        MenuMSX();
        rg_audio_set_mute(false);
        rg_input_wait_for_key(RG_KEY_ANY, false, 500);
        InMenu = false;
        return 0;
    }

    if (joystick == RG_KEY_START)
    {
        InKeyboard = !InKeyboard;
        rg_input_wait_for_key(RG_KEY_ANY, false, 500);
    }

    if (InMenu)
    {
        if (joystick == RG_KEY_LEFT)
            LastKey = CON_LEFT;
        if (joystick == RG_KEY_RIGHT)
            LastKey = CON_RIGHT;
        if (joystick == RG_KEY_UP)
            LastKey = CON_UP;
        if (joystick == RG_KEY_DOWN)
            LastKey = CON_DOWN;
        if (joystick == RG_KEY_A)
            LastKey = CON_OK;
        if (joystick == RG_KEY_B)
            LastKey = CON_EXIT;
    }
    else if (InKeyboard)
    {
        if (joystick & (RG_KEY_LEFT | RG_KEY_RIGHT | RG_KEY_UP | RG_KEY_DOWN))
        {
            if (rg_system_timer() > KeyboardDebounce)
            {
                if (joystick == RG_KEY_LEFT)
                    KeyboardCol--;
                if (joystick == RG_KEY_RIGHT)
                    KeyboardCol++;
                if (joystick == RG_KEY_UP)
                    KeyboardRow--;
                if (joystick == RG_KEY_DOWN)
                    KeyboardRow++;

                KeyboardCol = RG_MIN(RG_MAX(KeyboardCol, 0), XKEYS - 1);
                KeyboardRow = RG_MIN(RG_MAX(KeyboardRow, 0), YKEYS - 1);
                PutImage();
                KeyboardDebounce = rg_system_timer() + 250000;
            }
        }
        else if (joystick == RG_KEY_A)
        {
            KeyboardKey = KBDKeys[KeyboardRow][KeyboardCol];
            KBD_SET(KeyboardKey);
        }
        else if (joystick == RG_KEY_B)
        {
            rg_input_wait_for_key(RG_KEY_ANY, false, 500);
            InKeyboard = false;
        }
    }
    else if (KeyboardEmulation)
    {
        if (joystick & RG_KEY_LEFT)
            KBD_SET(KBD_LEFT);
        if (joystick & RG_KEY_RIGHT)
            KBD_SET(KBD_RIGHT);
        if (joystick & RG_KEY_UP)
            KBD_SET(KBD_UP);
        if (joystick & RG_KEY_DOWN)
            KBD_SET(KBD_DOWN);
        if (joystick & RG_KEY_A)
            KBD_SET(KBD_SPACE);
        if (joystick & RG_KEY_B)
            KBD_SET(KBD_ENTER);
    }
    else
    {
        if (joystick & RG_KEY_LEFT)
            JoyState |= JST_LEFT;
        if (joystick & RG_KEY_RIGHT)
            JoyState |= JST_RIGHT;
        if (joystick & RG_KEY_UP)
            JoyState |= JST_UP;
        if (joystick & RG_KEY_DOWN)
            JoyState |= JST_DOWN;
        if (joystick & RG_KEY_A)
            JoyState |= JST_FIREA;
        if (joystick & RG_KEY_B)
            JoyState |= JST_FIREB;
    }

    return 0;
}

int InitMachine(void)
{
    NormScreen = (Image){
        .Data = currentUpdate->data,
        .W = WIDTH,
        .H = HEIGHT,
        .L = WIDTH,
        .D = 16,
    };

    XBuf = NormScreen.Data;
    SetScreenDepth(NormScreen.D);
    SetVideo(&NormScreen, 0, 0, WIDTH, HEIGHT);

    for (int J = 0; J < 80; J++)
        SetColor(J, 0, 0, 0);

    for (int J = 0; J < 256; J++)
    {
        uint16_t color = C_RGB(((J >> 2) & 0x07) * 255 / 7, ((J >> 5) & 0x07) * 255 / 7, (J & 0x03) * 255 / 3);
        BPal[J] = ((color >> 8) | (color << 8)) & 0xFFFF;
    }

    InitSound(AUDIO_SAMPLE_RATE, 150);
    SetChannels(64, 0xFFFFFFFF);

    RPLInit(SaveState, LoadState, MAX_STASIZE);
    RPLRecord(RPL_RESET);
    return 1;
}

void TrashMachine(void)
{
    RPLTrash();
    TrashSound();
}

void SetColor(byte N, byte R, byte G, byte B)
{
    uint16_t color = C_RGB(R, G, B);
    color = (color >> 8) | (color << 8);
    if (N)
        XPal[N] = color;
    else
        XPal0 = color;
}

void UpdateViewport(void)
{
    int crop_v = CropPicture ? (ScanLines212 ? 8 : 18) : 0;
    rg_display_set_source_viewport(WIDTH, HEIGHT, 0, crop_v);
}

void PutImage(void)
{
    if (CropPicture && ScanLines212 != PrevScanLines212)
    {
        UpdateViewport();
        PrevScanLines212 = ScanLines212;
    }

    if (InKeyboard)
        DrawKeyboard(&NormScreen, KBDKeys[KeyboardRow][KeyboardCol]);
    rg_display_submit(currentUpdate, 0);
    currentUpdate = updates[currentUpdate == updates[0]];
    NormScreen.Data = currentUpdate->data;
    XBuf = currentUpdate->data;
}

unsigned int Joystick(void)
{
    ProcessEvents(0);
    return JoyState;
}

void Keyboard(void)
{
    // Keyboard() is a convenient place to do our vsync stuff :)
    rg_system_tick(rg_system_timer() - FrameStartTime);
    FrameStartTime = rg_system_timer();

    if (PendingLoadSTA)
    {
        LoadSTA(PendingLoadSTA);
        free(PendingLoadSTA);
        PendingLoadSTA = NULL;
    }
}

unsigned int Mouse(byte N)
{
    return 0;
}

int ShowVideo(void)
{
    rg_display_submit(currentUpdate, 0);
    rg_system_tick(0);
    return 1;
}

unsigned int GetJoystick(void)
{
    ProcessEvents(0);
    return 0;
}

unsigned int GetMouse(void)
{
    return 0;
}

unsigned int GetKey(void)
{
    unsigned int J;
    ProcessEvents(0);
    J = LastKey;
    LastKey = 0;
    return J;
}

unsigned int WaitKey(void)
{
    GetKey();
    rg_input_wait_for_key(RG_KEY_ANY, false, 200);
    while (!rg_input_wait_for_key(RG_KEY_ANY, true, 100))
        continue;
    return GetKey();
}

unsigned int WaitKeyOrMouse(void)
{
    LastKey = WaitKey();
    return 0;
}

unsigned int InitAudio(unsigned int Rate, unsigned int Latency)
{
    return AUDIO_SAMPLE_RATE;
}

void TrashAudio(void)
{
    //
}

unsigned int GetFreeAudio(void)
{
    return AUDIO_BUFFER_LENGTH * 2;
}

void PlayAllSound(int uSec)
{
    RenderAndPlayAudio(2 * uSec * AUDIO_SAMPLE_RATE / 1000000);
}

unsigned int WriteAudio(sample *Data, unsigned int Length)
{
    int64_t start = rg_system_timer();
    rg_audio_submit((void *)Data, Length >> 1);
    FrameStartTime += rg_system_timer() - start;
    return Length;
}

static bool save_state_handler(const char *filename)
{
    return SaveSTA(filename);
}

static bool load_state_handler(const char *filename)
{
    PendingLoadSTA = strdup(filename);
    return true;
}

static bool reset_handler(bool hard)
{
    ResetMSX(Mode,RAMPages,VRAMPages);
    return true;
}

static bool screenshot_handler(const char *filename, int width, int height)
{
    return rg_display_save_frame(filename, currentUpdate, width, height);
}

static void event_handler(int event, void *arg)
{
    if (event == RG_EVENT_REDRAW)
    {
        rg_display_submit(currentUpdate, 0);
    }
}

static rg_gui_event_t crop_select_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        CropPicture = !CropPicture;
        UpdateViewport();
        rg_settings_set_number(NS_APP, "Crop", CropPicture);
        return RG_DIALOG_REDRAW;
    }
    strcpy(option->value, CropPicture ? "On" : "Off");
    return RG_DIALOG_VOID;
}

static rg_gui_event_t input_select_cb(rg_gui_option_t *option, rg_gui_event_t event)
{
    if (event == RG_DIALOG_PREV || event == RG_DIALOG_NEXT)
    {
        KeyboardEmulation = !KeyboardEmulation;
        rg_settings_set_number(NS_APP, "Input", KeyboardEmulation);
    }
    strcpy(option->value, KeyboardEmulation ? "Keyboard" : "Joystick");
    return RG_DIALOG_VOID;
}

void app_main(void)
{
    const rg_handlers_t handlers = {
        .loadState = &load_state_handler,
        .saveState = &save_state_handler,
        .reset = &reset_handler,
        .screenshot = &screenshot_handler,
        .event = &event_handler,
    };
    const rg_gui_option_t options[] = {
        {0, "Input", "-", RG_DIALOG_FLAG_NORMAL, &input_select_cb},
        {0, "Crop ", "-", RG_DIALOG_FLAG_NORMAL, &crop_select_cb},
        RG_DIALOG_END,
    };

    app = rg_system_init(AUDIO_SAMPLE_RATE, &handlers, options);
    // This is probably not right, but the emulator outputs 440 samples per frame??
    app->tickRate = 55;

    updates[0] = rg_surface_create(WIDTH, HEIGHT, RG_PIXEL_565_BE, MEM_FAST);
    updates[1] = rg_surface_create(WIDTH, HEIGHT, RG_PIXEL_565_BE, MEM_FAST);
    currentUpdate = updates[0];

    KeyboardEmulation = rg_settings_get_number(NS_APP, "Input", 1);
    CropPicture = rg_settings_get_number(NS_APP, "Crop", 0);

    if (app->bootFlags & RG_BOOT_RESUME)
    {
        PendingLoadSTA = rg_emu_get_path(RG_PATH_SAVE_STATE + app->saveSlot, app->romPath);
    }

    char *argv[] = {"fmsx", "-skip", "50", "-home", RG_BASE_PATH_BIOS, "-joy", "1", NULL, NULL, NULL,};
    int argc = RG_COUNT(argv) - 3;

    if (strcasecmp(rg_extension(app->romPath), "dsk") == 0)
    {
        argv[argc++] = "-diska";
    }
    argv[argc++] = app->romPath;

    RG_LOGI("fMSX start");
    fmsx_main(argc, argv);

    RG_LOGI("fMSX ended");
    rg_system_exit();
}
