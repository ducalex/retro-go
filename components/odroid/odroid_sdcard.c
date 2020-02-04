#include "odroid_sdcard.h"

//#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_heap_caps.h"
#include "esp_spiffs.h"

#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>



#define SD_PIN_NUM_MISO 19
#define SD_PIN_NUM_MOSI 23
#define SD_PIN_NUM_CLK  18
#define SD_PIN_NUM_CS 22


static bool isOpen = false;
static char SD_BASE_PATH[32];

esp_err_t odroid_sdcard_open(const char* base_path)
{
    esp_err_t ret;

    if (isOpen)
    {
        printf("odroid_sdcard_open: alread open.\n");
        ret = ESP_FAIL;
    }
    else
    {
        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    	host.slot = HSPI_HOST; // HSPI_HOST;
    	//host.max_freq_khz = SDMMC_FREQ_HIGHSPEED; //10000000;
        host.max_freq_khz = SDMMC_FREQ_DEFAULT;

    	sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    	slot_config.gpio_miso = (gpio_num_t)SD_PIN_NUM_MISO;
    	slot_config.gpio_mosi = (gpio_num_t)SD_PIN_NUM_MOSI;
    	slot_config.gpio_sck  = (gpio_num_t)SD_PIN_NUM_CLK;
    	slot_config.gpio_cs = (gpio_num_t)SD_PIN_NUM_CS;
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
    	ret = esp_vfs_fat_sdmmc_mount(base_path, &host, &slot_config, &mount_config, &card);

    	if (ret == ESP_OK)
        {
            strcpy(SD_BASE_PATH, base_path);
            isOpen = true;
        }
        else
        {
            printf("odroid_sdcard_open: esp_vfs_fat_sdmmc_mount failed (%d)\n", ret);
        }
    }

	return ret;
}


esp_err_t odroid_sdcard_close()
{
    if (!isOpen)
    {
        printf("odroid_sdcard_close: not open.\n");
        return ESP_FAIL;
    }

    esp_err_t ret = esp_vfs_fat_sdmmc_unmount();
    if (ret != ESP_OK)
    {
        printf("odroid_sdcard_close: esp_vfs_fat_sdmmc_unmount failed (%d)\n", ret);
    }

    isOpen = false;

    return ret;
}


size_t odroid_sdcard_get_filesize(const char* path)
{
    size_t ret = 0;

    if (!isOpen)
    {
        printf("odroid_sdcard_get_filesize: not open.\n");
        return 0;
    }

    FILE* f = fopen(path, "rb");
    if (f == NULL)
    {
        printf("odroid_sdcard_get_filesize: fopen failed.\n");
        return 0;
    }

    fseek(f, 0, SEEK_END);
    ret = ftell(f);
    fseek(f, 0, SEEK_SET);

    return ret;
}

size_t odroid_sdcard_copy_file_to_memory(const char* path, void* ptr)
{
    size_t ret = 0;

    if (!isOpen)
    {
        printf("odroid_sdcard_copy_file_to_memory: not open.\n");
        return 0;
    }

    if (!ptr)
    {
        printf("odroid_sdcard_copy_file_to_memory: ptr is null.\n");
        return 0;
    }

    FILE* f = fopen(path, "rb");
    if (f == NULL)
    {
        printf("odroid_sdcard_copy_file_to_memory: fopen failed.\n");
        return 0;
    }

    const size_t BLOCK_SIZE = 512;
    while(true)
    {
        size_t count = fread((uint8_t*)ptr + ret, 1, BLOCK_SIZE, f);
        ret += count;
        if (count < BLOCK_SIZE) break;
    }

    fclose(f);

    return ret;
}

size_t odroid_sdcard_unzip_file_to_memory(const char* path, void* ptr)
{
    size_t ret = 0;

    if (!isOpen)
    {
        printf("odroid_sdcard_unzip_file_to_memory: not open.\n");
        return 0;
    }

    if (!ptr)
    {
        printf("odroid_sdcard_unzip_file_to_memory: ptr is null.\n");
        return 0;
    }

    return ret;
}

char* odroid_sdcard_get_savefile_path(const char* romPath)
{
    char* result = NULL;

    char* fileName = odroid_sdcard_get_filename(romPath);

    //printf("%s: base_path='%s', fileName='%s'\n", __func__, base_path, fileName);

    // Determine folder
    char* extension = fileName + strlen(fileName); // place at NULL terminator
    while (extension != fileName)
    {
        if (*extension == '.')
        {
            ++extension;
            break;
        }
        --extension;
    }

    if (extension == fileName)
    {
        printf("%s: File extention not found.\n", __func__);
        abort();
    }

    //printf("%s: extension='%s'\n", __func__, extension);

    const char* DATA_PATH = "/odroid/data/";
    const char* SAVE_EXTENSION = ".sav";

    size_t savePathLength = strlen(SD_BASE_PATH) + strlen(DATA_PATH) + strlen(extension) + 1 + strlen(fileName) + strlen(SAVE_EXTENSION) + 1;
    char* savePath = malloc(savePathLength);
    if (savePath)
    {
        strcpy(savePath, SD_BASE_PATH);
        strcat(savePath, DATA_PATH);
        strcat(savePath, extension);
        strcat(savePath, "/");
        strcat(savePath, fileName);
        strcat(savePath, SAVE_EXTENSION);

        printf("%s: savefile_path='%s'\n", __func__, savePath);

        result = savePath;
    }

    free(fileName);

    return result;
}


char* odroid_sdcard_get_filename(const char* path)
{
	int length = strlen(path);
	int fileNameStart = length;

	if (fileNameStart < 1) abort();

	while (fileNameStart > 0)
	{
		if (path[fileNameStart] == '/')
		{
			++fileNameStart;
			break;
		}

		--fileNameStart;
	}

	int size = length - fileNameStart + 1;

	char* result = malloc(size);
	if (!result) abort();

	result[size - 1] = 0;
	for (int i = 0; i < size - 1; ++i)
	{
		result[i] = path[fileNameStart + i];
	}

	//printf("GetFileName: result='%s'\n", result);

	return result;
}

char* odroid_sdcard_get_extension(const char* path)
{
	// Note: includes '.'
	int length = strlen(path);
	int extensionStart = length;

	if (extensionStart < 1) abort();

	while (extensionStart > 0)
	{
		if (path[extensionStart] == '.')
		{
			break;
		}

		--extensionStart;
	}

	int size = length - extensionStart + 1;

	char* result = malloc(size);
	if (!result) abort();

	result[size - 1] = 0;
	for (int i = 0; i < size - 1; ++i)
	{
		result[i] = path[extensionStart + i];
	}

	//printf("GetFileExtenstion: result='%s'\n", result);

	return result;
}

char* odroid_sdcard_get_filename_without_extension(const char* path)
{
	char* fileName = odroid_sdcard_get_filename(path);

	int length = strlen(fileName);
	int extensionStart = length;

	if (extensionStart < 1) abort();

	while (extensionStart > 0)
	{
		if (fileName[extensionStart] == '.')
		{
			break;
		}

		--extensionStart;
	}

	int size = extensionStart + 1;

	char* result = malloc(size);
	if (!result) abort();

	result[size - 1] = 0;
	for (int i = 0; i < size - 1; ++i)
	{
		result[i] = fileName[i];
	}

	free(fileName);

	//printf("GetFileNameWithoutExtension: result='%s'\n", result);

	return result;
}

int odroid_sdcard_mkdir(char *dir)
{
    if (!isOpen)
    {
        printf("odroid_sdcard_mkdir: not open.\n");
        return 0;
    }

    char tmp[128];
    size_t len = strlen(tmp);

    strcpy(tmp, dir);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    for (char *p = tmp + strlen(SD_BASE_PATH) + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            printf("odroid_sdcard_mkdir: Creating %s\n", tmp);
            mkdir(tmp, 0777);
            *p = '/';
        }
    }

    return mkdir(tmp, 0777);
}
