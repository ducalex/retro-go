#include <rg_system.h>

#define APP_ID 80

#define AUDIO_SAMPLE_RATE (8000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50)

// static short audioBuffer[AUDIO_BUFFER_LENGTH * 2];

static rg_video_frame_t frames[2];
static rg_video_frame_t *currentUpdate = &frames[0];

static rg_app_desc_t *app;

bool m_bIsActive = false;

// static bool netplay = false;
// --- MAIN

#include "StdAfx.h"

//#include "msounds.h"
#include "memory.h"
//#include "mainemu.h"
#include "tlcs900h.h"
#include "input.h"

#include "graphics.h"
#include "tlcs900h.h"

#include "neopopsound.h"

#ifdef DRZ80
#include "DrZ80_support.h"
#else
#if defined(CZ80)
#include "cz80_support.h"
#else
#include "z80.h"
#endif
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define PIX_TO_RGB(fmt, r, g, b) (((r >> fmt->Rloss) << fmt->Rshift) | ((g >> fmt->Gloss) << fmt->Gshift) | ((b >> fmt->Bloss) << fmt->Bshift))

typedef unsigned short word;
typedef unsigned int uint;

unsigned int m_Flag;
unsigned int interval;

unsigned long nextTick, lastTick = 0, newTick, currentTick, wait;
int FPS = 60;
int pastFPS = 0;

EMUINFO m_emuInfo;
SYSTEMINFO m_sysInfo[NR_OF_SYSTEMS];

static void initEmulator()
{
    m_emuInfo.machine = NGPC;
    m_emuInfo.drv = &m_sysInfo[m_emuInfo.machine];
    m_emuInfo.romSize = 0;
    strcpy(m_emuInfo.RomFileName, "");
    m_emuInfo.sample_rate = AUDIO_SAMPLE_RATE;
    m_emuInfo.stereo = 1;
    //m_emuInfo.fps = 60;//30;//100;  //Flavor, tweak this!

    m_sysInfo[NGP].hSize = 160;
    m_sysInfo[NGP].vSize = 152;
    m_sysInfo[NGP].Ticks = 6 * 1024 * 1024;
    /*	m_sysInfo[NGP].sound[0].sound_type = SOUND_SN76496;
	m_sysInfo[NGP].sound[0].sound_interface = new_SN76496(1, 3*1024*1024, MIXER(50,MIXER_PAN_CENTER));
	m_sysInfo[NGP].sound[1].sound_type = SOUND_DAC;
	m_sysInfo[NGP].sound[1].sound_interface = new_DAC(2,MIXER(50,MIXER_PAN_LEFT),MIXER(50,MIXER_PAN_RIGHT));
	m_sysInfo[NGP].sound[2].sound_type = 0;
	m_sysInfo[NGP].Back0 = TRUE;
	m_sysInfo[NGP].Back1 = TRUE;
	m_sysInfo[NGP].Sprites = TRUE;*/

    m_sysInfo[NGPC].hSize = 160;
    m_sysInfo[NGPC].vSize = 152;
    m_sysInfo[NGPC].Ticks = 6 * 1024 * 1024;
    /*	m_sysInfo[NGPC].sound[0].sound_type = SOUND_SN76496;
	m_sysInfo[NGPC].sound[0].sound_interface = new_SN76496(1, 3*1024*1024, 50);
	m_sysInfo[NGPC].sound[1].sound_type = SOUND_DAC;
	m_sysInfo[NGPC].sound[1].sound_interface = new_DAC(2,MIXER(50,MIXER_PAN_LEFT),MIXER(50,MIXER_PAN_RIGHT));
	m_sysInfo[NGPC].sound[2].sound_type = 0;
	m_sysInfo[NGPC].Back0 = TRUE;
	m_sysInfo[NGPC].Back1 = TRUE;
	m_sysInfo[NGPC].Sprites = TRUE;*/

	m_sysInfo[NGP].InputKeys[KEY_UP]		= m_sysInfo[NGPC].InputKeys[KEY_UP]        = 0;
	m_sysInfo[NGP].InputKeys[KEY_DOWN]		= m_sysInfo[NGPC].InputKeys[KEY_DOWN]      = 0;
	m_sysInfo[NGP].InputKeys[KEY_LEFT]		= m_sysInfo[NGPC].InputKeys[KEY_LEFT]      = 0;
	m_sysInfo[NGP].InputKeys[KEY_RIGHT]		= m_sysInfo[NGPC].InputKeys[KEY_RIGHT]     = 0;
	m_sysInfo[NGP].InputKeys[KEY_BUTTON_A]	= m_sysInfo[NGPC].InputKeys[KEY_BUTTON_A]  = 0;
	m_sysInfo[NGP].InputKeys[KEY_BUTTON_B]	= m_sysInfo[NGPC].InputKeys[KEY_BUTTON_B]  = 0;
	m_sysInfo[NGP].InputKeys[KEY_SELECT]	= m_sysInfo[NGPC].InputKeys[KEY_SELECT]    = 0;	// Option button
	m_sysInfo[NGP].InputKeys[KEY_START]		= m_sysInfo[NGPC].InputKeys[KEY_START]     = 0;	// Option button
	m_sysInfo[NGP].InputKeys[KEY_BUTTON_X]	= m_sysInfo[NGPC].InputKeys[KEY_BUTTON_X]  = 0;
	m_sysInfo[NGP].InputKeys[KEY_BUTTON_Y]	= m_sysInfo[NGPC].InputKeys[KEY_BUTTON_Y]  = 0;
	m_sysInfo[NGP].InputKeys[KEY_BUTTON_R]	= m_sysInfo[NGPC].InputKeys[KEY_BUTTON_R]  = 0;
	m_sysInfo[NGP].InputKeys[KEY_BUTTON_L]	= m_sysInfo[NGPC].InputKeys[KEY_BUTTON_L]  = 0;


    mem_init();
    tlcs_init();

#if defined(DRZ80) || defined(CZ80)
    Z80_Init();
    Z80_Reset();
#else
    z80Init();
#endif

    ngpSoundOff();

    InitInput(NULL);
}

static bool loadROM(const char *romName)
{
    FILE *romFile = rg_fopen(romName, "rb");
    if (!romFile)
    {
        fprintf(stderr, "Couldn't open %s file\n", romName);
        return false;
    }

    m_emuInfo.romSize = fread(mainrom, 1, 4 * 1024 * 1024, romFile);
    rg_fclose(romFile);

    if (memcmp(mainrom + 0x000009, " BY SNK CORPORATION", 19) != 0)
    {
        fprintf(stderr, "Not a valid or unsupported rom file. romFound==FALSE\n");
        return false;
    }

    int i = mainrom[0x000023];

    if (i != 0x10 && i != 0x00)
    {
        fprintf(stderr, "Not a valid or unsupported rom file.\n");
        return false;
    }

    // Detect System
    if (i == 0x10 || (mainrom[0x000020] == 0x34 && mainrom[0x000021] == 0x12))
    {
        m_emuInfo.machine = NGPC;
        m_emuInfo.drv = &m_sysInfo[NGPC];
    }
    else
    {
        m_emuInfo.machine = NGP;
        m_emuInfo.drv = &m_sysInfo[NGP];
    }

    strcpy(m_emuInfo.RomFileName, romName);
    setFlashSize(m_emuInfo.romSize);
    return true;
}

void graphics_paint(void)
{
    rg_display_queue_update(currentUpdate, NULL);
    printf("HAI!\n");
}

static bool save_state(char *pathName)
{
    bool ret = false;

    return ret;
}

static bool load_state(char *pathName)
{
    bool ret = false;

    return ret;
}

extern "C" void app_main(void)
{
    rg_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    rg_emu_init(&load_state, &save_state, NULL);

    app = rg_system_get_app();

    frames[0].flags = RG_PIXEL_565 | RG_PIXEL_BE;
    frames[0].width = NGPC_SIZEX;
    frames[0].height = NGPC_SIZEY;
    frames[0].stride = NGPC_SIZEX * 2;
    frames[1] = frames[0];

    frames[0].buffer = rg_alloc(NGPC_SIZEX * NGPC_SIZEY * 2, MEM_SLOW);
    frames[1].buffer = rg_alloc(NGPC_SIZEX * NGPC_SIZEY * 2, MEM_SLOW);

    drawBuffer = (unsigned short *)frames[0].buffer;

    m_bIsActive = true;

    initEmulator();

    // Load ROM
    if (!loadROM(app->romPath))
    {
        RG_PANIC("ROM Loading failed");
    }

    system_sound_chipreset();
    graphics_init();

    // if neogeo pocket color rom, act if we are a neogeo pocket color
    tlcsMemWriteB(0x6F91, tlcsMemReadB(0x00200023));

    // kludges & fixes
    switch (tlcsMemReadW(0x00200020))
    {
    case 0x0059: // Sonic
    case 0x0061: // Metal SLug 2nd
        *get_address(0x0020001F) = 0xFF;
        break;
    }

    printf("Running NGPC Emulation\n");

    ngpc_run();

    flashShutdown();
}
