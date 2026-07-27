//
//  macos_compat.h
//  Duke3D
//
//  Created by fabien sanglard on 12-12-12.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#ifndef Duke3D_macos_compat_h
#define Duke3D_macos_compat_h

#define PLATFORM_SUPPORTS_SDL

#include <stdlib.h>

#define kmalloc(x) malloc(x)
#define kkmalloc(x) malloc(x)
#define kfree(x) free(x)
#define kkfree(x) free(x)

#ifdef FP_OFF
#undef FP_OFF
#endif

// Horrible horrible macro: Watcom allowed memory pointer to be cast
// to a 32bits integer. The code is unfortunately stuffed with this :( !
#define FP_OFF(x) ((int32_t) (x))

#ifndef max
#define max(x, y)  (((x) > (y)) ? (x) : (y))
#endif

#ifndef min
#define min(x, y)  (((x) < (y)) ? (x) : (y))
#endif

#include <inttypes.h>
#define __int64 int64_t

#define O_BINARY 0

#define UDP_NETWORKING 1

#define PLATFORM_MACOSX 1

#define SOL_IP SOL_SOCKET
#define IP_RECVERR  SO_BROADCAST

#define stricmp strcasecmp
#define strcmpi strcasecmp

#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define USER_DUMMY_NETWORK 1

#endif
