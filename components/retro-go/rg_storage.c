#include <driver/sdmmc_host.h>
#include <esp_vfs_fat.h>
#include <esp_event.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "rg_system.h"

static bool disk_mounted = false;
static bool disk_led = true;

#define SETTING_DISK_ACTIVITY "DiskActivity"


void rg_storage_set_activity_led(bool enable)
{
    rg_settings_set_number(NS_GLOBAL, SETTING_DISK_ACTIVITY, enable);
    disk_led = enable;
}

bool rg_storage_get_activity_led(void)
{
    return disk_led;
}

#if RG_STORAGE_DRIVER == 1 || RG_STORAGE_DRIVER == 2
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
#endif

void rg_storage_init(void)
{
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .allocation_unit_size = 0,
        .max_files = 8,
    };

    if (disk_mounted)
        rg_storage_deinit();

#if RG_STORAGE_DRIVER == 1

    sdmmc_host_t host_config = SDSPI_HOST_DEFAULT();
    host_config.flags = SDMMC_HOST_FLAG_SPI;
    host_config.slot = RG_GPIO_SDSPI_HOST;
    host_config.do_transaction = &sdcard_do_transaction;
#if RG_STORAGE_HIGHSPEED
    host_config.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
#endif
    // These are for esp-idf 4.2 compatibility
    host_config.init = &sdspi_host_init;
    host_config.deinit = &sdspi_host_deinit;

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = RG_GPIO_SDSPI_MISO;
    slot_config.gpio_mosi = RG_GPIO_SDSPI_MOSI;
    slot_config.gpio_sck  = RG_GPIO_SDSPI_CLK;
    slot_config.gpio_cs = RG_GPIO_SDSPI_CS;
    slot_config.dma_channel = SPI_DMA_CH_AUTO;

    esp_err_t err = esp_vfs_fat_sdmmc_mount(RG_ROOT_PATH, &host_config, &slot_config, &mount_config, NULL);
    if (err == ESP_ERR_TIMEOUT || err == ESP_ERR_INVALID_RESPONSE || err == ESP_ERR_INVALID_CRC)
    {
        RG_LOGW("SD Card mounting failed (0x%x), retrying at lower speed...\n", err);
        host_config.max_freq_khz = SDMMC_FREQ_PROBING;
        err = esp_vfs_fat_sdmmc_mount(RG_ROOT_PATH, &host_config, &slot_config, &mount_config, NULL);
    }

#elif RG_STORAGE_DRIVER == 2

    sdmmc_host_t host_config = SDMMC_HOST_DEFAULT();
    host_config.flags = SDMMC_HOST_FLAG_1BIT;
    host_config.do_transaction = &sdcard_do_transaction;
#if RG_STORAGE_HIGHSPEED
    host_config.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
#endif

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
#if SOC_SDMMC_USE_GPIO_MATRIX
    slot_config.clk = RG_GPIO_SDSPI_CLK,
    slot_config.cmd = RG_GPIO_SDSPI_CMD,
    slot_config.d0 = RG_GPIO_SDSPI_D0,
    // d1 and d3 normally not used in width=1 but sdmmc_host_init_slot saves them, so just in case
    slot_config.d1 = slot_config.d3 = -1;
#endif

    esp_err_t err = esp_vfs_fat_sdmmc_mount(RG_ROOT_PATH, &host_config, &slot_config, &mount_config, NULL);
    if (err == ESP_ERR_TIMEOUT || err == ESP_ERR_INVALID_RESPONSE || err == ESP_ERR_INVALID_CRC)
    {
        RG_LOGW("SD Card mounting failed (0x%x), retrying at lower speed...\n", err);
        host_config.max_freq_khz = SDMMC_FREQ_PROBING;
        err = esp_vfs_fat_sdmmc_mount(RG_ROOT_PATH, &host_config, &slot_config, &mount_config, NULL);
    }

#elif RG_STORAGE_DRIVER == 4

    wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
    esp_err_t err = esp_vfs_fat_spiflash_mount(RG_ROOT_PATH, "storage", &s_wl_handle)

#else

    #warning "No supported storage driver selected!"
    esp_err_t err = ESP_ERR_NOT_SUPPORTED;

#endif

    if (err == ESP_OK)
        RG_LOGI("Storage mounted at %s. driver=%d\n", RG_ROOT_PATH, RG_STORAGE_DRIVER);
    else
        RG_LOGE("Storage mounting failed. driver=%d, err=0x%x\n", RG_STORAGE_DRIVER, err);

    rg_settings_init();

    disk_mounted = err == ESP_OK;
    disk_led = rg_settings_get_number(NS_GLOBAL, SETTING_DISK_ACTIVITY, 1);
}

void rg_storage_deinit(void)
{
    rg_storage_commit();

#if RG_STORAGE_DRIVER == 1 || RG_STORAGE_DRIVER == 2
    esp_err_t err = esp_vfs_fat_sdmmc_unmount();
#else
    esp_err_t err = ESP_ERR_NOT_SUPPORTED;
#endif

    if (err == ESP_OK)
        RG_LOGI("Storage unmounted.\n");
    else
        RG_LOGE("Storage unmounting failed. err=0x%x\n", err);

    disk_mounted = false;
}

bool rg_storage_ready(void)
{
    return disk_mounted;
}

void rg_storage_commit(void)
{
    if (!disk_mounted)
        return;

    rg_settings_commit();
    // flush buffers();
}

bool rg_storage_mkdir(const char *dir)
{
    RG_ASSERT(dir, "Bad param");

    char temp[RG_PATH_MAX + 1];
    int ret = mkdir(dir, 0777);

    if (ret == -1)
    {
        if (errno == EEXIST)
            return true;

        strncpy(temp, dir, RG_PATH_MAX);

        for (char *p = temp + strlen(RG_ROOT_PATH) + 1; *p; p++)
        {
            if (*p == '/')
            {
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

bool rg_storage_delete(const char *path)
{
    RG_ASSERT(path, "Bad param");
    DIR *dir;

    if (unlink(path) == 0)
    {
        RG_LOGI("Deleted file %s\n", path);
        return true;
    }
    else if (errno == ENOENT)
    {
        // The path already doesn't exist!
        return true;
    }
    else if (rmdir(path) == 0)
    {
        RG_LOGI("Deleted empty folder %s\n", path);
        return true;
    }
    else if ((dir = opendir(path)))
    {
        char pathbuf[128]; // Smaller than RG_PATH_MAX to prevent issues due to lazy recursion...
        struct dirent* ent;
        while ((ent = readdir(dir)))
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;
            snprintf(pathbuf, sizeof(pathbuf), "%s/%s", path, ent->d_name);
            rg_storage_delete(pathbuf);
        }
        closedir(dir);
        if (rmdir(path) == 0)
        {
            RG_LOGI("Deleted folder %s\n", path);
            return true;
        }
    }

    return false;
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
