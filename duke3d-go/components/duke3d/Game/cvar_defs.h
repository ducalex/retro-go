#ifndef _CVARDEFS_H_
#define _CVARDEFS_H_

#ifdef _WIN32
   #include "../../Engine/src/windows/inttypes.h"
#else
   #include <inttypes.h>
#endif

void CVARDEFS_Init();
void CVARDEFS_Render();
//
// Function declarations
//
void CVARDEFS_DefaultFunction(void* var);
void CVARDEFS_FunctionQuit(void* var);
void CVARDEFS_FunctionClear(void* var);
void CVARDEFS_FunctionLevel(void* var);
void CVARDEFS_FunctionName(void* var);
void CVARDEFS_FunctionPlayMidi(void* var);
void CVARDEFS_FunctionFOV(void* var);
void CVARDEFS_FunctionTickRate(void* var);
void CVARDEFS_FunctionTicksPerFrame(void* var);
void CVARDEFS_FunctionHelp(void* var);

//
// Variable declarations
//
int g_CV_console_text_color;
int g_CV_num_console_lines;
int g_CV_classic;
int g_CV_TransConsole;
int g_CV_DebugJoystick;
int g_CV_DebugSound;
int g_CV_DebugFileAccess;
uint32_t sounddebugActiveSounds;
uint32_t sounddebugAllocateSoundCalls;
uint32_t sounddebugDeallocateSoundCalls;


int g_CV_CubicInterpolation;

#endif