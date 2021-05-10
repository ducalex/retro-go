#include <esp_vfs_fat.h>
#include <esp_event.h>
#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "rg_system.h"

#define SETTING_DISK_ACTIVITY "DiskActivity"

static int initialized = false;
static int diskActivity = -1;


bool rg_vfs_init(void)
{
    initialized =  true;
    diskActivity = -1;
    return true;
}

bool rg_vfs_deinit(void)
{
    initialized = false;
    return true;
}

void rg_vfs_set_enable_disk_led(bool enable)
{
    rg_settings_set_int32(SETTING_DISK_ACTIVITY, enable);
    diskActivity = enable;
}

bool rg_vfs_get_enable_disk_led(void)
{
    if (diskActivity == -1 && rg_settings_ready())
        diskActivity = rg_settings_get_int32(SETTING_DISK_ACTIVITY, false);
    return diskActivity;
}

static esp_err_t sdcard_do_transaction(int slot, sdmmc_command_t *cmdinfo)
{
    bool use_led = (rg_vfs_get_enable_disk_led() && !rg_system_get_led());

    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    if (use_led)
    {
        rg_system_set_led(1);
    }

    esp_err_t ret = sdspi_host_do_transaction(slot, cmdinfo);

    if (use_led)
    {
        rg_system_set_led(0);
    }

    rg_spi_lock_release(SPI_LOCK_SDCARD);

    return ret;
}

bool rg_vfs_mount(rg_blkdev_t dev)
{
    if (dev != RG_SDCARD)
        return false;

    sdmmc_host_t host_config = SDSPI_HOST_DEFAULT();
    host_config.slot = HSPI_HOST;
    host_config.max_freq_khz = SDMMC_FREQ_DEFAULT; // SDMMC_FREQ_26M;
    host_config.do_transaction = &sdcard_do_transaction;

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = RG_GPIO_SD_MISO;
    slot_config.gpio_mosi = RG_GPIO_SD_MOSI;
    slot_config.gpio_sck  = RG_GPIO_SD_CLK;
    slot_config.gpio_cs = RG_GPIO_SD_CS;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .allocation_unit_size = 0,
        .max_files = 5,
    };

    // esp_vfs_fat_sdmmc_unmount();

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(
            RG_BASE_PATH, &host_config, &slot_config, &mount_config, NULL);

    if (ret == ESP_OK)
    {
        RG_LOGI("SD Card mounted, freq=%d\n", 0);
        return true;
    }

    RG_LOGE("SD Card mounting failed (%d)\n", ret);
    return false;
}

bool rg_vfs_unmount(rg_blkdev_t dev)
{
    if (dev != RG_SDCARD)
        return false;

    esp_err_t ret = esp_vfs_fat_sdmmc_unmount();
    if (ret == ESP_OK)
    {
        RG_LOGI("SD Card unmounted\n");
        return true;
    }
    RG_LOGE("SD Card unmounting failed (%d)\n", ret);
    return false;
}

bool rg_vfs_mkdir(const char *dir)
{
    int ret = mkdir(dir, 0777);

    if (ret == -1)
    {
        if (errno == EEXIST)
        {
            return true;
        }

        char temp[PATH_MAX + 1];
        strncpy(temp, dir, sizeof(temp) - 1);

        for (char *p = temp + strlen(RG_BASE_PATH) + 1; *p; p++) {
            if (*p == '/') {
                *p = 0;
                if (strlen(temp) > 0) {
                    RG_LOGI("Creating %s\n", temp);
                    mkdir(temp, 0777);
                }
                *p = '/';
                while (*(p+1) == '/') p++;
            }
        }

        ret = mkdir(temp, 0777);
    }

    if (ret == 0)
    {
        RG_LOGI("Folder created %s\n", dir);
    }

    return (ret == 0);
}

rg_strings_t *rg_vfs_readdir(const char* path, int flags)
{
    DIR* dir = opendir(path);
    if (!dir)
        return NULL;

    size_t buffer_size = 512;
    rg_strings_t *files = calloc(1, sizeof(rg_strings_t) + buffer_size);
    char pathbuf[PATH_MAX + 1];
    struct dirent* file;

    while ((file = readdir(dir)))
    {
        const char *name = file->d_name;
        size_t name_len = strlen(name) + 1;
        bool is_directory = (file->d_type == DT_DIR);

        if (name[0] == '.')
        {
            if (flags & RG_SKIP_HIDDEN)
                continue;
            else if (name_len <= 3)
                continue;
        }

        if (!is_directory || !(flags & RG_FILES_ONLY))
        {
            if ((files->length + name_len) >= buffer_size)
            {
                buffer_size += 1024;
                files = realloc(files, sizeof(rg_strings_t) + buffer_size);
            }

            memcpy(&files->buffer[files->length], name, name_len);
            files->length += name_len;
            files->count++;
        }

        // FIX ME: This won't work correctly on more than one nesting level
        // That is because we don't carry the base path around so we'll store an
        // incomplete relative path beyond the first level...
        if (is_directory && (flags & RG_RECURSIVE))
        {
            sprintf(pathbuf, "%.120s/%.120s", path, name);
            rg_strings_t *sub = rg_vfs_readdir(pathbuf, flags);
            if (sub && sub->count > 0)
            {
                buffer_size = files->length + sub->length + (name_len * sub->count) + 128;
                files = realloc(files, sizeof(rg_strings_t) + buffer_size);

                char *ptr = sub->buffer;
                for (size_t i = 0; i < sub->count; i++)
                {
                    files->length += sprintf(&files->buffer[files->length], "%s/%s", name, ptr) + 1;
                    files->count++;
                    ptr += strlen(sub->buffer) + 1;
                }
            }
            free(sub);
        }
    }

    files->buffer[files->length] = 0;

    closedir(dir);

    return files;
}

bool rg_vfs_delete(const char *path)
{
    return (unlink(path) == 0 || rmdir(path) == 0);
}

long rg_vfs_filesize(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0) ? st.st_size : -1;
}

bool rg_vfs_isdir(const char *path)
{
    struct stat st;

    if (stat(path, &st) == 0)
        return S_ISDIR(st.st_mode);

    return false;
}

char *rg_vfs_dirname(const char *path)
{
    char *dirname = strdup(path);
    char *basename = strrchr(dirname, '/');
    if (basename) {
        *basename = 0;
    }
    return dirname;
}

const char *rg_vfs_basename(const char *path)
{
    const char *name = strrchr(path, '/');
    return name ? name + 1 : NULL;
}

const char* rg_vfs_extension(const char *path)
{
    const char *ext = strrchr(path, '.');
    return ext ? ext + 1 : NULL;
}

FILE *rg_vfs_fopen(const char *path, const char *mode)
{
    FILE *file = fopen(path, mode);
    if (!file && !strchr(mode, 'r'))
    {
        // Try to create the directory and retry open
        char *dirname = rg_vfs_dirname(path);
        rg_vfs_mkdir(dirname);
        free(dirname);
        file = fopen(path, mode);
    }
    return file;
}

int rg_vfs_fclose(FILE *file)
{
    return fclose(file);
}
