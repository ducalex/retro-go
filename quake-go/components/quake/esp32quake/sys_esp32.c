/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"
#include "errno.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rg_system.h"
#include "fatfs_proxy.h"

#define MAX_HANDLES 32

typedef struct 
{
    bool isOpen;
    FILE *file;
    uint32_t mmapOffset;
} file_handle_t;

static file_handle_t sys_handles[MAX_HANDLES];

static volatile bool sys_quit = false;
static bool host_is_up = false;
static uint32_t excluded_pause_us = 0;
static quakeparms_t parms;

static const char *mmap_pak_path;
static uint32_t mmap_pak_size;
static const void *mmap_pak;

qboolean isDedicated = false;
extern qboolean r_fov_greater_than_90;

// quake stdio wrappers - had to do these to allow proxying the calls to fatfs_proxy 
//- separate 'thread' (task) that does work in cache-disable context
//as opposed to quake main task which has stack on PSRAM - spiflash operations are not allowed in this case

struct QUAKE_FILE
{
    FILE *file;
    bool isMemoryFile;
};

QUAKE_FILE* quake_fopen(const char* restrict name, const char* restrict type)
{
    FILE *file;
    bool isMemoryFile;

    if (mmap_pak && mmap_pak_path && strcmp(name, mmap_pak_path) == 0)
    {
        if (strcmp(type, "rb") != 0)
            Sys_Error("quake_fopen: invalid mode %s for mmapped %s\n", type, name);

        isMemoryFile = true;
        file = fmemopen((void*)mmap_pak, mmap_pak_size, "rb");
    }
    else
    {
        isMemoryFile = false;
        file = fatfs_proxy_fopen(name, type);
    }

    if (file == NULL)
        return NULL;

    QUAKE_FILE *qfile = malloc(sizeof(QUAKE_FILE));

    if (qfile == NULL)
        Sys_Error("quake_fopen: malloc for QUAKE_FILE failed");

    qfile->file = file;
    qfile->isMemoryFile = isMemoryFile;

    return qfile;
}

int	quake_fclose(QUAKE_FILE *qfile)
{
    int ret;

    if (qfile->isMemoryFile)
        ret = fclose(qfile->file);
    else   
        ret = fatfs_proxy_fclose(qfile->file);

    free(qfile);
    return ret;
}

int quake_fseek(QUAKE_FILE *qfile, long pos, int type)
{
    int ret;

    if (qfile->isMemoryFile)
        ret = fseek(qfile->file, pos, type);
    else   
        ret = fatfs_proxy_fseek(qfile->file, pos, type);

    return ret;
}

size_t quake_fread(void* restrict buf, size_t size, size_t n, QUAKE_FILE* restrict qfile)
{
    size_t ret;

    if (qfile->isMemoryFile)
        ret = fread(buf, size, n, qfile->file);
    else   
        ret = fatfs_proxy_fread(buf, size, n, qfile->file);

    return ret;
}

size_t quake_fwrite(const void* restrict buf, size_t size, size_t n, QUAKE_FILE *qfile)
{
    size_t ret;
    
    if (qfile->isMemoryFile)
        ret = fwrite(buf, size, n, qfile->file);
    else   
        ret = fatfs_proxy_fwrite(buf, size, n, qfile->file);

    return ret;
}

int quake_fprintf(QUAKE_FILE* restrict qfile, const char* restrict fmt, ...)
{
    int ret;
    va_list args;

    va_start(args, fmt);

    if (qfile->isMemoryFile)
        ret = vfprintf(qfile->file, fmt, args);
    else   
        ret = fatfs_proxy_vfprintf(qfile->file, fmt, args);

    va_end(args);

    return ret;
}

int quake_fscanf(QUAKE_FILE* restrict qfile, const char* restrict fmt, ...)
{
    int ret;
    va_list args;

    va_start(args, fmt);

    if (qfile->isMemoryFile)
        ret = vfscanf(qfile->file, fmt, args);
    else   
        ret = fatfs_proxy_vfscanf(qfile->file, fmt, args);

    va_end(args);

    return ret;
}

int quake_fgetc(QUAKE_FILE *qfile)
{
    int ret;

    if (qfile->isMemoryFile)
        ret = fgetc(qfile->file);
    else
        ret = fatfs_proxy_fgetc(qfile->file);

    return ret;
}

int	quake_fflush(QUAKE_FILE *qfile)
{
    int ret;

    if (qfile->isMemoryFile)
        ret = fflush(qfile->file);
    else
        ret = fatfs_proxy_fflush(qfile->file);

    return ret;
}

int	quake_feof(QUAKE_FILE *qfile)
{
    int ret;

    if (qfile->isMemoryFile)
        ret = feof(qfile->file);
    else
        ret = fatfs_proxy_feof(qfile->file);

    return ret;
}

/*
===============================================================================

FILE IO

===============================================================================
*/

static int findhandle(void)
{
    int i;
    
    for (i = 1; i < MAX_HANDLES; ++i)
        if (!sys_handles[i].isOpen)
            return i;

    Sys_Error("out of handles");
    return -1;
}

static int filelength(FILE *f)
{
    long pos;
    long end;

    pos = fatfs_proxy_ftell(f);
    fatfs_proxy_fseek(f, 0, SEEK_END);
    end = fatfs_proxy_ftell(f);
    fatfs_proxy_fseek(f, pos, SEEK_SET);

    return end;
}

int Sys_FileOpenRead(char *path, int *hndl)
{
    FILE *f;
    int i;
    
    i = findhandle();
    
    //mmap hook
    if (mmap_pak && mmap_pak_path && !strcmp(path, mmap_pak_path))
    {
        sys_handles[i].isOpen = true;
        sys_handles[i].mmapOffset = 0;
        
        *hndl = i;
        return mmap_pak_size;
    }

    f = fatfs_proxy_fopen(path, "rb");
    if (!f)
    {
        RG_LOGW("Sys_FileOpenRead: failed to open %s", path);
        *hndl = -1;
        return -1;
    }

    sys_handles[i].isOpen = true;
    sys_handles[i].file = f;

    *hndl = i;
    
    return filelength(f);
}

int Sys_FileOpenWrite(char *path)
{
    FILE *f;
    int i;
    
    if (mmap_pak && mmap_pak_path && !strcmp(path, mmap_pak_path))
        Sys_Error("attempt to write to mmaped pak file");

    i = findhandle();

    f = fatfs_proxy_fopen(path, "wb");

    if (!f)
        Sys_Error ("Error opening %s: %s", path,strerror(errno));

    sys_handles[i].isOpen = true;
    sys_handles[i].file = f;
    
    return i;
}

void Sys_FileClose(int handle)
{
    if (sys_handles[handle].file != NULL)
    {
        fatfs_proxy_fclose(sys_handles[handle].file);
        sys_handles[handle].file = NULL;
    }

    sys_handles[handle].isOpen = false;
}

void Sys_FileSeek(int handle, int position)
{
    assert(sys_handles[handle].isOpen);

    if (sys_handles[handle].file == NULL)
    {
        sys_handles[handle].mmapOffset = position;
        return;
    }

    fatfs_proxy_fseek(sys_handles[handle].file, position, SEEK_SET);
}

int Sys_FileRead(int handle, void *dest, int count)
{
    assert(sys_handles[handle].isOpen);
    
    rg_task_yield();

    if (sys_handles[handle].file == NULL)
    {
        if (sys_handles[handle].mmapOffset >= mmap_pak_size)
            return 0;

        if ((sys_handles[handle].mmapOffset + count) > mmap_pak_size)
            count = mmap_pak_size - sys_handles[handle].mmapOffset;

        memcpy(dest, (const uint8_t*)mmap_pak + sys_handles[handle].mmapOffset, count);
        sys_handles[handle].mmapOffset += count;
        return count;
    }

    int ret = fatfs_proxy_fread(dest, 1, count, sys_handles[handle].file);

    if (ret != count)
    {
        RG_LOGE("Sys_FileRead: expected %d, got %d. errno: %d\n", count, ret, fatfs_proxy_get_errno());
    }
    return ret;
}

int Sys_FileWrite(int handle, void *data, int count)
{
    assert(sys_handles[handle].isOpen && sys_handles[handle].file != NULL);

    return fatfs_proxy_fwrite(data, 1, count, sys_handles[handle].file);
}

int Sys_FileTime(char *path)
{
    FILE *f;
    
    if (mmap_pak && mmap_pak_path && !strcmp(path, mmap_pak_path))
        return 1;

    f = fatfs_proxy_fopen(path, "rb");
    if (f)
    {
        fatfs_proxy_fclose(f);
        return 1;
    }
    
    return -1;
}

void Sys_mkdir(char *path)
{    
    if (fatfs_proxy_mkdir(path, 0755) != 0)
    {
        int err = fatfs_proxy_get_errno();
        if (err != EEXIST)
            RG_LOGE("Sys_mkdir: unable to create %s: %s\n", path, strerror(err));
    }
}

const void* Sys_FileGetMmapBase(int handle)
{
    assert(sys_handles[handle].isOpen);

    return sys_handles[handle].file == NULL ? mmap_pak : NULL;
}

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_MakeCodeWriteable(unsigned long startaddr, unsigned long length)
{
}

void Sys_Error(char *error, ...)
{
    va_list         argptr;

    va_start (argptr,error);
    rg_system_vlog(RG_LOG_ERROR, "Quake", error, argptr);
    va_end (argptr);

    abort();
}

void Sys_Printf(char *fmt, ...)
{
    va_list argptr;
    
    va_start(argptr,fmt);
    vprintf(fmt,argptr);
    va_end(argptr);
}

void Sys_Quit(void)
{	
    sys_quit = true;
}

void Sys_Shutdown(void)
{
    sys_quit = true;
    if (host_is_up)
    {
        host_is_up = false;
        Host_Shutdown();
    }

    // Pack files remain open for the engine lifetime. Close every native file
    // handle before the proxy exits and Retro-Go unmounts storage.
    for (int i = 0; i < MAX_HANDLES; ++i)
    {
        if (sys_handles[i].isOpen)
            Sys_FileClose(i);
    }

    if (parms.membase)
    {
        free(parms.membase);
        parms.membase = NULL;
    }
}

void Sys_ExcludePauseTime(uint32_t paused_us)
{
    excluded_pause_us += paused_us;
}

double Sys_FloatTime(void)
{
    return ((uint64_t)esp_timer_get_time()) / 1000000.0;
}

char *Sys_ConsoleInput(void)
{
    return NULL;
}

void Sys_Sleep(void)
{
}

void Sys_SendKeyEvents(void)
{
    // This is also called by SCR_ModalMessage while the main host frame is
    // blocked, so controller input must be pumped here rather than only from
    // Host_Frame.
    IN_Commands();
}

void Sys_HighFPPrecision(void)
{
}

void Sys_LowFPPrecision(void)
{
}

//=============================================================================

extern void S_PrecacheAmbients(void);

void esp32_quake_main(int argc, char **argv, const char *basedir,
                      const char *pak0Path, const char *pak1Path,
                      uint32_t pakSize, const void *pakMmap)
{
    mmap_pak_path = pak0Path;
    mmap_pak_size = pakSize;
    mmap_pak = pakMmap;
    COM_SetSelectedPaks(pak0Path, pak1Path);

    strcpy(com_savedir, RG_BASE_PATH_SAVES "/quake");
    strcpy(com_configdir, RG_BASE_PATH_CONFIG "/quake");

#if defined(CONFIG_IDF_TARGET_ESP32P4)
    // All supported P4 handhelds have at least 16 MiB PSRAM. Leave ample room
    // for Retro-Go/display allocations while exceeding WinQuake's historical
    // minimum and accommodating the official mission packs.
    parms.memsize = 10240*1024; // 10MB Hunk for P4
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    parms.memsize = 6400*1024; // 6.25MB Hunk for S3
#else
    parms.memsize = 3481*1024; // 3.4MB Hunk for original ESP32 (4MB limit)
#endif
    parms.membase = heap_caps_malloc(parms.memsize, MALLOC_CAP_SPIRAM);
    parms.basedir = (char*)basedir;

    if (parms.membase == NULL)
        Sys_Error("membase allocation failed\n");

    COM_InitArgv(argc, argv);

    parms.argc = com_argc;
    parms.argv = com_argv;

    RG_LOGI("Host_Init\n");
    Host_Init(&parms);
    host_is_up = true;
    
    // Call our deferred precaching after Host_Init
    S_PrecacheAmbients();

    // Force some cvars
    Cvar_SetValue("r_drawviewmodel", 1);
    Cvar_SetValue("viewsize", 100);
    Cvar_SetValue("crosshair", 1);
    Cvar_SetValue("fov", 90);
    Cvar_SetValue("nosound", 0);

    // START and optional X both emit this held key. Apply it for shareware,
    // registered data and mods so the hardware mapping never depends on a
    // particular default.cfg.
    Cbuf_AddText("bind c +movedown\n");

    double oldtime = Sys_FloatTime();
    int force_viewmodel = 100;
    int skip_frames = 0;

    while (!sys_quit)
    {
        double newtime = Sys_FloatTime();
        double time = (newtime - oldtime) * rg_system_get_app()->speed;
        int frame_before = host_framecount;
        int64_t work_start = rg_system_timer();

        VID_SetSkipFrame(skip_frames > 0);

        if (force_viewmodel > 0) {
            Cvar_SetValue("r_drawviewmodel", 1);
            r_fov_greater_than_90 = false;
            force_viewmodel--;
        }

        Host_Frame (time);

        uint32_t paused_us = excluded_pause_us;
        excluded_pause_us = 0;
        oldtime = newtime + paused_us / 1000000.0;

        // Host_FilterTime rejects iterations above Quake's 72 Hz ceiling.
        // Count only an accepted simulation frame as a Retro-Go tick.
        if (host_framecount != frame_before)
        {
            int64_t busy_us = rg_system_timer() - work_start - paused_us
                              - VID_ConsumeDisplayTime();
            rg_system_tick((int)(busy_us > 0 ? busy_us : 0));

            if (skip_frames > 0)
                skip_frames--;
            else if (VID_ConsumeDisplayLate())
                skip_frames = 1;
        }

        // Yield after accounting so time spent running display/audio tasks is
        // not reported as Quake CPU work.
        rg_task_yield();
    }

    Sys_Shutdown();
}
