//
//  global.h
//  Duke3D
//
//  Created by fabien sanglard on 12-12-17.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#ifndef Duke3D_global_h
#define Duke3D_global_h
#include "esp32_compat.h"
void FixFilePath(char  *filename);
int FindDistance3D(int ix, int iy, int iz);
void Shutdown(void);

#ifndef LITTLE_ENDIAN
    #ifdef __APPLE__
    #else
        #define LITTLE_ENDIAN 1234
    #endif
#endif

#ifndef BIG_ENDIAN
     #ifdef __APPLE__
     #else
        #define BIG_ENDIAN 4321
    #endif
#endif

#if PLATFORM_WIN32
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

#ifdef __APPLE__
#if __powerpc__
#define BYTE_ORDER BIG_ENDIAN
#else
// Defined in endian.h
//#define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

#ifndef BYTE_ORDER
#error Please define your platform.
#endif

#if (BYTE_ORDER == LITTLE_ENDIAN)
#define KeepShort IntelShort
#define SwapShort MotoShort
#define Keepint32_t IntelLong
#define Swapint32_t MotoLong
#else
#define KeepShort MotoShort
#define SwapShort IntelShort
#define Keepint32_t MotoLong
#define Swapint32_t IntelLong
#endif

int32_t MotoLong (int32_t l);
int32_t IntelLong (int32_t l);

void Error (int errorType, char  *error, ...);

#endif
