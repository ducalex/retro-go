#include <driver/sdmmc_host.h>
#include <esp_vfs_fat.h>
#include <esp_event.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <cJSON.h>

#include "rg_system.h"

#define CONFIG_FILE_PATH RG_BASE_PATH_CONFIG "/retro-go.json"
#define SETTING_DISK_ACTIVITY "DiskActivity"

static esp_err_t sdcard_mount = ESP_FAIL;
static bool disk_led = true;
static cJSON *root = NULL;
static int unsaved_changes = 0;


void rg_storage_set_activity_led(bool enable)
{
    rg_settings_set_number(NS_GLOBAL, SETTING_DISK_ACTIVITY, enable);
    disk_led = enable;
}

bool rg_storage_get_activity_led(void)
{
    return disk_led;
}

static esp_err_t sdcard_do_transaction(int slot, sdmmc_command_t *cmdinfo)
{
    bool use_led = (disk_led && !rg_system_get_led());

    if (use_led)
        rg_system_set_led(1);

#if RG_STORAGE_DRIVER == 1
    //spi_device_acquire_bus(spi_handle, portMAX_DELAY);
    esp_err_t ret = sdspi_host_do_transaction(slot, cmdinfo);
    //spi_device_release_bus(spi_handle);
#else
    esp_err_t ret = sdmmc_host_do_transaction(slot, cmdinfo);
#endif

    if (use_led)
        rg_system_set_led(0);

    return ret;
}

void rg_storage_init(void)
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .allocation_unit_size = 0,
        .max_files = 8,
    };

    if (sdcard_mount == ESP_OK)
        rg_storage_deinit();

#if RG_STORAGE_DRIVER == 1

    sdmmc_host_t host_config = SDSPI_HOST_DEFAULT();
    host_config.slot = HSPI_HOST;
    host_config.max_freq_khz = SDMMC_FREQ_DEFAULT; // SDMMC_FREQ_26M;
    host_config.do_transaction = &sdcard_do_transaction;

    // These are for esp-idf 4.2 compatibility
    host_config.flags = SDMMC_HOST_FLAG_SPI;
    host_config.init = &sdspi_host_init;
    host_config.deinit = &sdspi_host_deinit;

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = RG_GPIO_SD_MISO;
    slot_config.gpio_mosi = RG_GPIO_SD_MOSI;
    slot_config.gpio_sck  = RG_GPIO_SD_CLK;
    slot_config.gpio_cs = RG_GPIO_SD_CS;

#elif RG_STORAGE_DRIVER == 2

    sdmmc_host_t host_config = SDMMC_HOST_DEFAULT();
    host_config.flags = SDMMC_HOST_FLAG_1BIT;
    host_config.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
    host_config.do_transaction = &sdcard_do_transaction;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;

#else

    #error "No SD Card driver selected"

#endif

    sdcard_mount = esp_vfs_fat_sdmmc_mount(RG_ROOT_PATH, &host_config, &slot_config, &mount_config, NULL);
    if (sdcard_mount == ESP_OK)
        RG_LOGI("SD Card mounted at %s. driver=%d\n", RG_ROOT_PATH, RG_STORAGE_DRIVER);
    else
        RG_LOGE("SD Card mounting failed. driver=%d, err=0x%x\n", RG_STORAGE_DRIVER, sdcard_mount);

    // Now we can load settings from SD Card
    FILE *fp = fopen(CONFIG_FILE_PATH, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        long length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *buffer = calloc(1, length + 1);
        if (fread(buffer, 1, length, fp))
            root = cJSON_Parse(buffer);
        free(buffer);
        fclose(fp);
    }

    if (root)
    {
        RG_LOGI("Settings loaded from %s.\n", CONFIG_FILE_PATH);
        disk_led = rg_settings_get_number(NS_GLOBAL, SETTING_DISK_ACTIVITY, 1);
    }
    else
    {
        RG_LOGW("Failed to load settings from %s.\n", CONFIG_FILE_PATH);
        root = cJSON_CreateObject();
    }
}

void rg_storage_deinit(void)
{
    rg_storage_commit();

    esp_err_t err = esp_vfs_fat_sdmmc_unmount();
    if (err != ESP_OK)
    {
        RG_LOGE("SD Card unmounting failed. err=0x%x\n", err);
        return;
    }

    RG_LOGI("SD Card unmounted.\n");
    sdcard_mount = ESP_FAIL;
}

bool rg_storage_ready(void)
{
    return sdcard_mount == ESP_OK;
}

void rg_storage_commit(void)
{
    if (unsaved_changes > 0)
    {
        char *buffer = cJSON_Print(root);
        if (buffer)
        {
            FILE *fp = fopen(CONFIG_FILE_PATH, "wb");
            if (!fp)
            {
                if (unlink(CONFIG_FILE_PATH) == -1)
                    rg_mkdir(rg_dirname(CONFIG_FILE_PATH));
                fp = fopen(CONFIG_FILE_PATH, "wb");
            }
            if (fp)
            {
                if (fputs(buffer, fp) > 0)
                    unsaved_changes = 0;
                fclose(fp);
            }
            cJSON_free(buffer);
        }
        else
        {
            RG_LOGE("cJSON_Print() failed.\n");
        }
    }

    // flush buffers();
}


static cJSON *json_root(const char *name)
{
    RG_ASSERT(root, "json_root called before settings were initialized!");

    cJSON *myroot;

    if (!name)
    {
        myroot = root;
    }
    else if (!(myroot = cJSON_GetObjectItem(root, name)))
    {
        myroot = cJSON_AddObjectToObject(root, name);
    }
    else if (!cJSON_IsObject(myroot))
    {
        myroot = cJSON_CreateObject();
        cJSON_ReplaceItemInObject(root, name, myroot);
    }

    return myroot;
}

void rg_settings_reset(void)
{
    cJSON_Delete(root);
    root = cJSON_CreateObject();
    unsaved_changes++;
    rg_storage_commit();
}

double rg_settings_get_number(const char *section, const char *key, double default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section), key);
    return obj ? obj->valuedouble : default_value;
}

void rg_settings_set_number(const char *section, const char *key, double value)
{
    cJSON *root = json_root(section);
    cJSON *obj = cJSON_GetObjectItem(root, key);

    if (!cJSON_IsNumber(obj))
    {
        cJSON_Delete(cJSON_DetachItemViaPointer(root, obj));
        cJSON_AddNumberToObject(root, key, value);
        unsaved_changes++;
    }
    else if (obj->valuedouble != value)
    {
        cJSON_SetNumberHelper(obj, value);
        unsaved_changes++;
    }
}

char *rg_settings_get_string(const char *section, const char *key, const char *default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section), key);
    if (cJSON_IsString(obj))
        return strdup(obj->valuestring);
    return default_value ? strdup(default_value) : NULL;
}

void rg_settings_set_string(const char *section, const char *key, const char *value)
{
    cJSON *root = json_root(section);
    cJSON *obj = cJSON_GetObjectItem(root, key);

    if (!value || !cJSON_IsString(obj))
    {
        cJSON_Delete(cJSON_DetachItemViaPointer(root, obj));
        cJSON_AddStringToObject(root, key, value);
        unsaved_changes++;
    }
    else if (strcmp(obj->valuestring, value) != 0)
    {
        cJSON_ReplaceItemInObject(root, key, cJSON_CreateString(value));
        unsaved_changes++;
    }
}

void rg_settings_delete(const char *section, const char *key)
{
    if (key)
        cJSON_DeleteItemFromObject(json_root(section), key);
    else
        cJSON_DeleteItemFromObject(root, section);
    unsaved_changes++;
}


bool rg_mkdir(const char *dir)
{
    RG_ASSERT(dir, "Bad param");

    int ret = mkdir(dir, 0777);

    if (ret == -1)
    {
        if (errno == EEXIST)
        {
            return true;
        }

        char temp[PATH_MAX + 1];
        strncpy(temp, dir, sizeof(temp) - 1);

        for (char *p = temp + strlen(RG_ROOT_PATH) + 1; *p; p++) {
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

const char *rg_dirname(const char *path)
{
    static char buffer[100];
    const char *basename = strrchr(path, '/');
    ptrdiff_t length = basename - path;

    if (!path || !basename)
        return ".";

    if (path[0] == '/' && path[1] == 0)
        return "/";

    RG_ASSERT(length < 100, "to do: use heap");

    strncpy(buffer, path, length);
    buffer[length] = 0;

    return buffer;
}

const char *rg_basename(const char *path)
{
    const char *name = strrchr(path, '/');
    return name ? name + 1 : path;
}

const char *rg_extension(const char *path)
{
    const char *basename = rg_basename(path);
    const char *ext = strrchr(basename, '.');
    return ext ? ext + 1 : NULL;
}
