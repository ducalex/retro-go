#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
#include <esp_vfs_fat.h>
#include <esp_err.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

#include "odroid_system.h"
#include "odroid_sdcard.h"
#include "../miniz/miniz.h"

static bool sdcardOpen = false;

#define SDCARD_ACCESS_BEGIN() { \
    /* if (!sdcardOpen) RG_PANIC("SD Card not initialized"); */ \
    if (!sdcardOpen) { printf("SD Card not initialized\n"); return -1; } \
    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD); \
}
#define SDCARD_ACCESS_END() odroid_system_spi_lock_release(SPI_LOCK_SDCARD)

int odroid_sdcard_open()
{
    if (sdcardOpen)
    {
        printf("odroid_sdcard_open: already open.\n");
        return -1;
    }

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = HSPI_HOST; // HSPI_HOST;
    //host.max_freq_khz = SDMMC_FREQ_HIGHSPEED; //10000000;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = ODROID_PIN_SD_MISO;
    slot_config.gpio_mosi = ODROID_PIN_SD_MOSI;
    slot_config.gpio_sck  = ODROID_PIN_SD_CLK;
    slot_config.gpio_cs = ODROID_PIN_SD_CS;
    //slot_config.dma_channel = 2;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config;
    memset(&mount_config, 0, sizeof(mount_config));

    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;


    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(ODROID_BASE_PATH, &host, &slot_config, &mount_config, NULL);
    sdcardOpen = (ret == ESP_OK);

    if (ret != ESP_OK)
    {
        printf("odroid_sdcard_open: esp_vfs_fat_sdmmc_mount failed (%d)\n", ret);
        return -1;
    }

    return 0;
}

int odroid_sdcard_close()
{
    esp_err_t ret = esp_vfs_fat_sdmmc_unmount();
    if (ret != ESP_OK)
    {
        printf("odroid_sdcard_close: esp_vfs_fat_sdmmc_unmount failed (%d)\n", ret);
    }
    sdcardOpen = false;
    return (ret == ESP_OK) ? 0 : -1;
}

int odroid_sdcard_read_file(const char* path, void* buf, size_t buf_size)
{
    SDCARD_ACCESS_BEGIN();

    int count = -1;
    FILE* f;

    if ((f = fopen(path, "rb")))
    {
        count = fread(buf, 1, buf_size, f);
        fclose(f);
    }
    else
        printf("%s: fopen failed. path='%s'\n", __func__, path);

    SDCARD_ACCESS_END();

    return count;
}

int odroid_sdcard_write_file(const char* path, void* buf, size_t buf_size)
{
    SDCARD_ACCESS_BEGIN();

    int count = -1;
    FILE* f;

    if ((f = fopen(path, "wb")))
    {
        count = fwrite(buf, 1, buf_size, f);
        fclose(f);
    }
    else
        printf("%s: fopen failed. path='%s'\n", __func__, path);

    SDCARD_ACCESS_END();

    return count;
}

int odroid_sdcard_unzip_file(const char* path, void* buf, size_t buf_size)
{
    SDCARD_ACCESS_BEGIN();

    int count = -1;
    mz_zip_archive zip_archive;

    memset(&zip_archive, 0, sizeof(zip_archive));

    if (mz_zip_reader_init_file(&zip_archive, path, 0))
    {
        printf("%s: Opened archive %s\n", __func__, path);
        mz_zip_archive_file_stat file_stat;
        if (mz_zip_reader_file_stat(&zip_archive, 0, &file_stat))
        {
            printf("%s: Extracting file %s\n", __func__, file_stat.m_filename);
            if (mz_zip_reader_extract_to_mem(&zip_archive, 0, buf, buf_size, 0))
            {
                count = file_stat.m_uncomp_size;
            }
        }
        mz_zip_reader_end(&zip_archive);
    }

    SDCARD_ACCESS_END();

    return count;
}

int odroid_sdcard_list(const char* path, char **out_files, size_t *out_count)
{
    SDCARD_ACCESS_BEGIN();

    // Maybe we should replace out_files by a struct that contains the packed list + an index to allow
    // n-addressing. We'd still maintain the benefits of no custom free needed and reduced alloc overhead.

    int ret = -1;

    DIR* dir = opendir(path);
    if (dir)
    {
        char *buffer = NULL;
        size_t bufsize = 0;
        size_t bufpos = 0;
        size_t count = 0;
        struct dirent* file;

        while ((file = readdir(dir)))
        {
            size_t len = strlen(file->d_name) + 1;

            if (len < 2) continue;

            if ((bufsize - bufpos) < len)
            {
                bufsize += 1024;
                buffer = realloc(buffer, bufsize);
            }

            memcpy(buffer + bufpos, &file->d_name, len);
            bufpos += len;
            count++;
        }

        buffer = realloc(buffer, bufpos + 1);
        buffer[bufpos] = 0;

        closedir(dir);
        ret = 0;

        if (out_files) *out_files = buffer;
        if (out_count) *out_count = count;
    }

    SDCARD_ACCESS_END();

    return ret;
}

int odroid_sdcard_unlink(const char* path)
{
    SDCARD_ACCESS_BEGIN();

    int ret = unlink(path);

    SDCARD_ACCESS_END();

    return ret;
}

int odroid_sdcard_mkdir(const char *dir)
{
    SDCARD_ACCESS_BEGIN();

    int ret = mkdir(dir, 0777);

    if (ret == -1)
    {
        if (errno == EEXIST)
        {
            printf("odroid_sdcard_mkdir: Folder exists %s\n", dir);
            return 0;
        }

        char temp[255];
        strncpy(temp, dir, sizeof(temp) - 1);

        for (char *p = temp + strlen(ODROID_BASE_PATH) + 1; *p; p++) {
            if (*p == '/') {
                *p = 0;
                if (strlen(temp) > 0) {
                    printf("odroid_sdcard_mkdir: Creating %s\n", temp);
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
        printf("odroid_sdcard_mkdir: Folder created %s\n", dir);
    }

    SDCARD_ACCESS_END();

    return ret;
}

int odroid_sdcard_get_filesize(const char* path)
{
    SDCARD_ACCESS_BEGIN();

    int ret = -1;
    FILE* f;

    if ((f = fopen(path, "rb")))
    {
        fseek(f, 0, SEEK_END);
        ret = ftell(f);
        fclose(f);
    }
    else
        printf("odroid_sdcard_get_filesize: fopen failed.\n");

    SDCARD_ACCESS_END();

    return ret;
}

const char* odroid_sdcard_get_filename(const char* path)
{
    const char *name = strrchr(path, '/');
    return name ? name + 1 : NULL;
}

const char* odroid_sdcard_get_extension(const char* path)
{
    const char *ext = strrchr(path, '.');
    return ext ? ext + 1 : NULL;
}
