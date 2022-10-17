#include "rg_system.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if RG_STORAGE_DRIVER == 1 || RG_STORAGE_DRIVER == 2
#include <driver/sdmmc_host.h>
#include <esp_vfs_fat.h>
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#define mkdir(A, B) mkdir(A)
#endif

static bool disk_mounted = false;
static bool disk_led = true;

static const char *SETTING_DISK_ACTIVITY = "DiskActivity";


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
    // spi_device_acquire_bus(spi_handle, portMAX_DELAY);
    esp_err_t ret = sdspi_host_do_transaction(slot, cmdinfo);
    // spi_device_release_bus(spi_handle);
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
    if (disk_mounted)
        rg_storage_deinit();

    int error_code = -1;

#if RG_STORAGE_DRIVER == 0 // Host (stdlib)

    error_code = 0;

#elif RG_STORAGE_DRIVER == 1 // SDSPI

    sdmmc_host_t host_config = SDSPI_HOST_DEFAULT();
    host_config.flags = SDMMC_HOST_FLAG_SPI;
    host_config.do_transaction = &sdcard_do_transaction;
    // These are for esp-idf 4.2 compatibility
    host_config.init = &sdspi_host_init;
    host_config.deinit = &sdspi_host_deinit;
#ifdef RG_STORAGE_HOST
    host_config.slot = RG_STORAGE_HOST;
#endif
#ifdef RG_STORAGE_SPEED
    host_config.max_freq_khz = RG_STORAGE_SPEED;
#endif

    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = RG_GPIO_SDSPI_MISO;
    slot_config.gpio_mosi = RG_GPIO_SDSPI_MOSI;
    slot_config.gpio_sck = RG_GPIO_SDSPI_CLK;
    slot_config.gpio_cs = RG_GPIO_SDSPI_CS;
    slot_config.dma_channel = SPI_DMA_CH_AUTO;

    esp_vfs_fat_mount_config_t mount_config = {.max_files = 8};

    esp_err_t err = esp_vfs_fat_sdmmc_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
    if (err == ESP_ERR_TIMEOUT || err == ESP_ERR_INVALID_RESPONSE || err == ESP_ERR_INVALID_CRC)
    {
        RG_LOGW("SD Card mounting failed (0x%x), retrying at lower speed...\n", err);
        host_config.max_freq_khz = SDMMC_FREQ_PROBING;
        err = esp_vfs_fat_sdmmc_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
    }
    error_code = err;

#elif RG_STORAGE_DRIVER == 2 // SDMMC

    sdmmc_host_t host_config = SDMMC_HOST_DEFAULT();
    host_config.flags = SDMMC_HOST_FLAG_1BIT;
    host_config.do_transaction = &sdcard_do_transaction;
#ifdef RG_STORAGE_HOST
    host_config.slot = RG_STORAGE_HOST;
#endif
#ifdef RG_STORAGE_SPEED
    host_config.max_freq_khz = RG_STORAGE_SPEED;
#endif

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
#if SOC_SDMMC_USE_GPIO_MATRIX
    slot_config.clk = RG_GPIO_SDSPI_CLK;
    slot_config.cmd = RG_GPIO_SDSPI_CMD;
    slot_config.d0 = RG_GPIO_SDSPI_D0;
    // d1 and d3 normally not used in width=1 but sdmmc_host_init_slot saves them, so just in case
    slot_config.d1 = slot_config.d3 = -1;
#endif

    esp_vfs_fat_mount_config_t mount_config = {.max_files = 8};

    esp_err_t err = esp_vfs_fat_sdmmc_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
    if (err == ESP_ERR_TIMEOUT || err == ESP_ERR_INVALID_RESPONSE || err == ESP_ERR_INVALID_CRC)
    {
        RG_LOGW("SD Card mounting failed (0x%x), retrying at lower speed...\n", err);
        host_config.max_freq_khz = SDMMC_FREQ_PROBING;
        err = esp_vfs_fat_sdmmc_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
    }
    error_code = err;

#elif RG_STORAGE_DRIVER == 3 // USB OTG

    #warning "USB OTG isn't available on your SOC"
    error_code = -1;

#elif RG_STORAGE_DRIVER == 4 // SPI Flash

    wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
    esp_err_t err = esp_vfs_fat_spiflash_mount(RG_STORAGE_ROOT, "storage", &s_wl_handle);
    error_code = err;

#else

    #error "No supported storage driver selected!"

#endif

    if (!error_code)
        RG_LOGI("Storage mounted at %s. driver=%d\n", RG_STORAGE_ROOT, RG_STORAGE_DRIVER);
    else
        RG_LOGE("Storage mounting failed. driver=%d, err=0x%x\n", RG_STORAGE_DRIVER, error_code);

    rg_settings_init();

    disk_mounted = !error_code;
    disk_led = rg_settings_get_number(NS_GLOBAL, SETTING_DISK_ACTIVITY, 1);
}

void rg_storage_deinit(void)
{
    rg_storage_commit();

    int error_code = 0;

#if RG_STORAGE_DRIVER == 1 || RG_STORAGE_DRIVER == 2
    esp_err_t err = esp_vfs_fat_sdmmc_unmount();
    error_code = err;
#endif

    if (!error_code)
        RG_LOGI("Storage unmounted.\n");
    else
        RG_LOGE("Storage unmounting failed. err=0x%x\n", error_code);

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

        for (char *p = temp + strlen(RG_STORAGE_ROOT) + 1; *p; p++)
        {
            if (*p == '/')
            {
                *p = 0;
                if (strlen(temp) > 0)
                {
                    RG_LOGI("Creating %s\n", temp);
                    mkdir(temp, 0777);
                }
                *p = '/';
                while (*(p + 1) == '/')
                    p++;
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
        struct dirent *ent;
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

static int scandir_natural_sort(const void *a, const void *b)
{
    // FIXME: Do something...
    return 0;
}

rg_scandir_t *rg_storage_scandir(const char *path, bool (*validator)(const char *path), uint32_t flags)
{
    DIR *dir = opendir(path);
    if (!dir)
        return NULL;

    rg_scandir_t *results = calloc(1, sizeof(rg_scandir_t));
    size_t capacity = 0;
    size_t count = 0;
    struct dirent *ent;
    struct stat statbuf;

    char fullpath[RG_PATH_MAX + 1] = {0};
    char *basename = fullpath + sprintf(fullpath, "%s/", path);
    size_t basename_len = RG_PATH_MAX - (basename - fullpath);

    while ((ent = readdir(dir)))
    {
        strncpy(basename, ent->d_name, basename_len);

        if (basename[0] == '.') // For backwards compat we'll ignore all hidden files...
            continue;

        if (validator && !validator(fullpath))
            continue;

        if (count + 1 >= capacity)
        {
            capacity += 100;
            void *temp = realloc(results, (capacity + 1) * sizeof(rg_scandir_t));
            if (!temp)
            {
                RG_LOGW("Not enough memory to finish scan!\n");
                break;
            }
            results = temp;
        }

        rg_scandir_t *result = &results[count++];

        strncpy(result->name, basename, sizeof(result->name) - 1);
        result->is_valid = 1;
        #if defined(DT_REG) && defined(DT_DIR)
            result->is_file = ent->d_type == DT_REG;
            result->is_dir = ent->d_type == DT_DIR;
        #else
            flags |= RG_SCANDIR_STAT;
        #endif

        if ((flags & RG_SCANDIR_STAT) && stat(fullpath, &statbuf) == 0)
        {
            result->is_file = S_ISREG(statbuf.st_mode);
            result->is_dir = S_ISDIR(statbuf.st_mode);
            result->size = statbuf.st_size;
        }
    }
    memset(&results[count], 0, sizeof(rg_scandir_t));

    if (flags & RG_SCANDIR_SORT)
    {
        qsort(results, count, sizeof(rg_scandir_t), scandir_natural_sort);
    }

    return results;
}
