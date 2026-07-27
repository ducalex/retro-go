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
extern int g_CV_console_text_color;
extern int g_CV_num_console_lines;
extern int g_CV_classic;
extern int g_CV_TransConsole;
extern int g_CV_DebugJoystick;
extern int g_CV_DebugSound;
extern int g_CV_DebugFileAccess;
extern uint32_t sounddebugActiveSounds;
extern uint32_t sounddebugAllocateSoundCalls;
extern uint32_t sounddebugDeallocateSoundCalls;


extern int g_CV_CubicInterpolation;

#endif
