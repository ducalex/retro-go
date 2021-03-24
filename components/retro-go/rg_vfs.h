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

bool rg_vfs_init(void);
bool rg_vfs_deinit(void);

bool rg_vfs_mount(rg_blkdev_t dev);
bool rg_vfs_unmount(rg_blkdev_t dev);

void rg_vfs_set_enable_disk_led(bool enable);
bool rg_vfs_get_enable_disk_led(void);

bool rg_vfs_mkdir(const char *dir);
bool rg_vfs_readdir(const char* path, char **out_files, size_t *out_count, bool skip_hidden);
bool rg_vfs_delete(const char *path);
long rg_vfs_filesize(const char *path);

const char *rg_vfs_basename(const char *path);
const char *rg_vfs_extension(const char *path);
