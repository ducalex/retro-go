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

bool odroid_sdcard_open()
{
    esp_err_t ret;

    if (sdcardOpen)
    {
        printf("odroid_sdcard_open: already open.\n");
        ret = ESP_FAIL;
    }
    else
    {
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
    	sdmmc_card_t* card;
    	ret = esp_vfs_fat_sdmmc_mount(ODROID_BASE_PATH, &host, &slot_config, &mount_config, &card);

    	if (ret == ESP_OK)
        {
            sdcardOpen = true;
        }
        else
        {
            printf("odroid_sdcard_open: esp_vfs_fat_sdmmc_mount failed (%d)\n", ret);
        }
    }

	return ret == ESP_OK;
}

bool odroid_sdcard_close()
{
    esp_err_t ret = esp_vfs_fat_sdmmc_unmount();
    if (ret != ESP_OK)
    {
        printf("odroid_sdcard_close: esp_vfs_fat_sdmmc_unmount failed (%d)\n", ret);
    }
    sdcardOpen = false;
    return ret == ESP_OK;
}

size_t odroid_sdcard_get_filesize(const char* path)
{
    assert(sdcardOpen == true);

    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

    size_t ret = 0;
    FILE* f;

    if ((f = fopen(path, "rb")))
    {
        fseek(f, 0, SEEK_END);
        ret = ftell(f);
        fclose(f);
    }
    else
        printf("odroid_sdcard_get_filesize: fopen failed.\n");

    odroid_system_spi_lock_release(SPI_LOCK_SDCARD);

    return ret;
}

size_t odroid_sdcard_copy_file_to_memory(const char* path, void* buf, size_t buf_size)
{
    assert(sdcardOpen == true);

    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

    size_t block_size = MIN(4096, buf_size);
    size_t ret = 0, count = 0;
    FILE* f;

    if ((f = fopen(path, "rb")))
    {
        do {
            count = fread(buf + ret, 1, block_size, f);
            ret += count;
        } while (count == block_size && ret < buf_size);

        fclose(f);
    }
    else
        printf("%s: fopen failed. path='%s'\n", __func__, path);

    odroid_system_spi_lock_release(SPI_LOCK_SDCARD);

    return ret;
}

size_t odroid_sdcard_unzip_file_to_memory(const char* path, void* buf, size_t buf_size)
{
    assert(sdcardOpen == true);

    odroid_system_spi_lock_acquire(SPI_LOCK_SDCARD);

    size_t ret = 0;
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
                ret = file_stat.m_uncomp_size;
            }
        }
        mz_zip_reader_end(&zip_archive);
    }

    odroid_system_spi_lock_release(SPI_LOCK_SDCARD);

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

int odroid_sdcard_mkdir(char *dir)
{
    assert(sdcardOpen == true);

    int ret = mkdir(dir, 0777);

    if (ret == -1)
    {
        if (errno == EEXIST)
        {
            printf("odroid_sdcard_mkdir: Folder exists %s\n", dir);
            return 0;
        }

        char tmp[128];
        size_t len = strlen(tmp);

        strcpy(tmp, dir);
        if (tmp[len - 1] == '/') {
            tmp[len - 1] = 0;
        }

        for (char *p = tmp + strlen(ODROID_BASE_PATH) + 1; *p; p++) {
            if (*p == '/') {
                *p = 0;
                printf("odroid_sdcard_mkdir: Creating %s\n", tmp);
                mkdir(tmp, 0777);
                *p = '/';
            }
        }

        ret = mkdir(tmp, 0777);
    }

    if (ret == 0)
    {
        printf("odroid_sdcard_mkdir: Folder created %s\n", dir);
    }

    return ret;
}
