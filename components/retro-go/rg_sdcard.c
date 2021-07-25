#include <esp_vfs_fat.h>
#include <esp_event.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "rg_system.h"

#define SETTING_DISK_ACTIVITY "DiskActivity"

static sdmmc_card_t *card = NULL;
static int diskActivity = -1;


void rg_sdcard_set_enable_activity_led(bool enable)
{
    rg_settings_set_int32(SETTING_DISK_ACTIVITY, enable);
    diskActivity = enable;
}

bool rg_sdcard_get_enable_activity_led(void)
{
    if (diskActivity == -1 && rg_settings_ready())
        diskActivity = rg_settings_get_int32(SETTING_DISK_ACTIVITY, false);
    return diskActivity;
}

static esp_err_t sdcard_do_transaction(int slot, sdmmc_command_t *cmdinfo)
{
    bool use_led = (rg_sdcard_get_enable_activity_led() && !rg_system_get_led());

    rg_spi_lock_acquire(SPI_LOCK_SDCARD);

    if (use_led)
        rg_system_set_led(1);

    esp_err_t ret = sdspi_host_do_transaction(slot, cmdinfo);

    if (use_led)
        rg_system_set_led(0);

    rg_spi_lock_release(SPI_LOCK_SDCARD);

    return ret;
}

bool rg_sdcard_mount(void)
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .allocation_unit_size = 0,
        .max_files = 5,
    };

    esp_err_t err = ESP_FAIL;

#if RG_DRIVER_SDCARD == 1

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

#elif RG_DRIVER_SDCARD == 2

    sdmmc_host_t host_config = SDMMC_HOST_DEFAULT();
    host_config.flags = SDMMC_HOST_FLAG_1BIT;
    host_config.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;

#else

    #error "No SD Card driver selected"

#endif

    if (card != NULL)
        rg_sdcard_unmount();

    err = esp_vfs_fat_sdmmc_mount(RG_BASE_PATH, &host_config, &slot_config, &mount_config, &card);

    if (err == ESP_OK)
    {
        RG_LOGI("SD Card mounted. serial=%08X\n", card->cid.serial);
        return true;
    }
    else
    {
        RG_LOGE("SD Card mounting failed. err=0x%x\n", err);
        card = NULL;
        return false;
    }
}

bool rg_sdcard_unmount(void)
{
    esp_err_t err = esp_vfs_fat_sdmmc_unmount();

    if (err == ESP_OK)
    {
        RG_LOGI("SD Card unmounted\n");
        card = NULL;
        return true;
    }

    RG_LOGE("SD Card unmounting failed. err=0x%x\n", err);
    return false;
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
