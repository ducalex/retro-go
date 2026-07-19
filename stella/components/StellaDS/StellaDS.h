#ifndef __DS_TOOLS_H
#define __DS_TOOLS_H

#include <nds.h>

#include "Console.hxx"

#ifndef WAITVBL
#define WAITVBL
#endif

#define STELLADS_MENUINIT 0x01
#define STELLADS_MENUSHOW 0x02
#define STELLADS_PLAYINIT 0x03 
#define STELLADS_PLAYGAME 0x04 
#define STELLADS_QUITSTDS 0x05

typedef enum {
  EMUARM7_INIT_SND = 0x123C,
  EMUARM7_STOP_SND = 0x123D,
  EMUARM7_PLAY_SND = 0x123E,
} FifoMesType;

#define MAX_ROMS_PER_DIRECTORY  1500
#define MAX_FILE_NAME_LEN       199

typedef struct FICtoLoad {
  char  filename[MAX_FILE_NAME_LEN];
  uInt8 directory;
} FICA2600;

#ifdef __cplusplus
extern "C" {
#endif

extern Console* theConsole;
extern FICA2600 *vcsromlist;

extern uInt16 atari_frames;
extern uInt8  bInitialDiffSet;
extern uInt8  tv_type_requested;
extern uInt8  gSaveKeyEEWritten;
extern uInt8  gSaveKeyIsDirty;
extern uInt16 mySoundFreq;
extern uInt16 emuState;
extern uint8* sound_buffer;
extern uint8  bHaltEmulation; 
extern uint8  bScreenRefresh;
extern uInt32 gAtariFrames;
extern uInt32 gTotalAtariFrames;
extern uInt16 console_color;

extern uInt8 *BG_GFX;
extern uint16 *aptr;
extern uint16 *bptr;

extern uint8_t *videoBuf0;
extern uint8_t *videoBuf1;
extern uint32_t wave_direct_samples;
extern volatile int *stack_executionStatus;
extern rg_surface_t *screen;

extern void dsInstallSoundEmuFIFO(void);

#ifdef __cplusplus
}
#endif

#endif
