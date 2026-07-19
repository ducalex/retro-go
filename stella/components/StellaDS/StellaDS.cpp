#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StellaDS.h"
#include "Console.hxx"
#include "Cart.hxx"
#include "Event.hxx"
#include "TIA.hxx"
#include "stella_config.h"

// These are centralized globals used by the whole emulator core
uint8_t *BG_GFX = NULL;
uInt8 *myDSFramePointer = NULL;

uInt16 atari_frames = 0;
uInt8 bInitialDiffSet = 0;
uInt8 tv_type_requested = NTSC;
uInt8 gSaveKeyEEWritten = false;
uInt8 gSaveKeyIsDirty = false;
uInt16 mySoundFreq = 44100;

Int32 debug[40] = {0};
char DEBUG_DUMP = 0;
char my_filename[MAX_FILE_NAME_LEN + 1] = {0};

// Massive structures moved to pointers for PSRAM allocation
AllConfig_t *allConfigs_ptr = NULL;
FICA2600 *vcsromlist = NULL;
uInt16 countvcs = 0, ucFicAct = 0;

Console *theConsole = NULL;
TIA theTIA;

int bg0, bg0b, bg1b;
uInt16 emuState;
uint8  bHaltEmulation = 0;
uint8  bScreenRefresh = 0;
uInt16 console_color = 0;

uint8_t *sound_buffer = NULL;

extern "C" {
uint16_t *aptr = NULL;
uint16_t *bptr = NULL;
uInt8 *videoBuf0 = NULL;
uInt8 *videoBuf1 = NULL;
uInt32 gAtariFrames = 0, gTotalAtariFrames = 0;
volatile int *stack_executionStatus = NULL;
uInt32 wave_direct_samples = 0;
rg_surface_t *screen = NULL;
}

// NDS Stub functions
void irqVCount(void) {}
void vblankIntr() {}
void dsShowScreenEmu() {}
void dsShowScreenPaddles() {}
void dsShowScreenKeypad() {}
void dsDisplayButton(unsigned char button) {}
void dsShowScreenMain(bool bFull) {}
void dsFreeEmu() {}
bool dsWaitOnQuit() { return true; }
void SaveFavorites() {}
void dsPrintValue(int x, int y, unsigned int isSelect, char *pchStr) {}
void dsPrintFPS() {}
void dsMainLoop(void) {}
unsigned int dsWaitForRom() { return 0; }
void dsDisplayFiles(unsigned int findex, uint32_t cursor) {}
void dsInitPalette(void) {}
void vcsFindFiles(void) {}
int a26Filescmp(const void *c1, const void *c2) { return 0; }
void dsShowScreenInstructions() {}
void highscore_display() {}
void DumpDebugData() {}
void ShowStatusLine() {}
void ShowConfig() {}
void dsWarnIncompatibileCart() {}
void dsPrintCartType(char *md5, int type) {}

void LoadConfig(void) {
    if (allConfigs_ptr) {
        // Initialize with safe defaults
        memset(allConfigs_ptr, 0, sizeof(AllConfig_t));
        allConfigs_ptr->config_ver = CONFIG_VER;
        allConfigs_ptr->global.sound = SOUND_WAVE;
        for (int slot=0; slot<MAX_CONFIGS; slot++) {
            strcpy(allConfigs_ptr->cart[slot].md5, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            allConfigs_ptr->cart[slot].soundQuality = SOUND_WAVE;
        }
        myGlobalCartInfo = allConfigs_ptr->global;
    }
}
void SaveConfig(bool bShow) {}
void LoadFavorites(void) {}

void dsInstallSoundEmuFIFO(void) {
    if (sound_buffer == NULL) {
        sound_buffer = (uint8_t *)rg_alloc(SOUND_SIZE, MEM_SLOW);
    }
    aptr = (uint16_t *)&sound_buffer[0];
    bptr = (uint16_t *)&sound_buffer[2];
}

void _putchar(char character) {}
