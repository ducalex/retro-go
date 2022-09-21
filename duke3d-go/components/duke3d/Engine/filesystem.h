//
//  filesystem.h
//  Duke3D
//
//  Created by fabien sanglard on 12-12-19.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#ifndef Duke3D_filesystem_h
#define Duke3D_filesystem_h

#include "stdio.h"
#include "platform.h"



#define MAXGROUPFILES 4     /* Warning: Fix groupfil if this is changed */


extern int32_t groupefil_crc32[MAXGROUPFILES];

int32_t  initgroupfile(const char  *filename);
void     uninitgroupfile(void);
uint16_t crc16(uint8_t  *data_p, uint16_t length);
uint32_t crc32_update(uint8_t  *buf, uint32_t length, uint32_t crc_to_update);
int32_t  kopen4load(const char  *filename, int openOnlyFromGRP);
int32_t  kread(int32_t handle, void *buffer, int32_t leng);
int32_t  kread8(int32_t handle, uint8_t  *buffer);
int32_t  kread16(int32_t handle, int16_t *buffer);
int32_t  kread32(int32_t handle, int32_t *buffer);
int32_t  klseek(int32_t handle, int32_t offset, int whence);
int32_t  kfilelength(int32_t handle);
void     kclose(int32_t handle);
void     kdfread(void *buffer, size_t dasizeof, size_t count, int32_t fil);
void     dfread(void *buffer, size_t dasizeof, size_t count, FILE *fil);
void     dfwrite(void *buffer, size_t dasizeof, size_t count, FILE *fil);

int      getGRPcrc32(int grpID);

char*    getGameDir(void);
void     setGameDir(char* gameDir);

int32_t  TCkopen4load(const char  *filename, int32_t readfromGRP);

//#if defined(__APPLE__) || defined(__linux__)
int32_t  filelength(int32_t fd);
//#endif

#endif
