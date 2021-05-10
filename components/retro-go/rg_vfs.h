#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef enum
{
    RG_SDCARD,
    RG_FLASH,
    RG_USBHDD,
} rg_blkdev_t;

typedef enum
{
    RG_SKIP_HIDDEN = (1 << 0),
    RG_RECURSIVE   = (1 << 1),
    RG_FILES_ONLY  = (1 << 2),
} rg_vfs_flags_t;

// FIX ME: This needs to be moved to a more appropriate place...
typedef struct
{
    size_t count;
    size_t length;
    char buffer[];
} rg_strings_t;

bool rg_vfs_init(void);
bool rg_vfs_deinit(void);

bool rg_vfs_mount(rg_blkdev_t dev);
bool rg_vfs_unmount(rg_blkdev_t dev);

void rg_vfs_set_enable_disk_led(bool enable);
bool rg_vfs_get_enable_disk_led(void);

rg_strings_t *rg_vfs_readdir(const char* path, int flags);
bool rg_vfs_mkdir(const char *dir);
bool rg_vfs_delete(const char *path);
long rg_vfs_filesize(const char *path);
bool rg_vfs_isdir(const char *path);

char *rg_vfs_dirname(const char *path);
const char *rg_vfs_basename(const char *path);
const char *rg_vfs_extension(const char *path);

FILE *rg_vfs_fopen(const char *path, const char *mode);
int rg_vfs_fclose(FILE *file);
