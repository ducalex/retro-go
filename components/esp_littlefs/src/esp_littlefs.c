/**
 * @file esp_littlefs.c
 * @brief Maps LittleFS <-> ESP_VFS
 * @author Brian Pugh
 */

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#endif // LOG_LOCAL_LEVEL

#include "esp_littlefs.h"
#include "littlefs/lfs.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "littlefs_api.h"
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <time.h>
#include <unistd.h>
#include "esp_random.h"

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#error "esp_littlefs requires esp-idf >=5.0"
#endif


#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
#include <sdmmc_cmd.h>
#endif

#include "spi_flash_mmap.h"

#if CONFIG_IDF_TARGET_ESP32
#include "esp32/rom/spi_flash.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/spi_flash.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/rom/spi_flash.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/spi_flash.h"
#elif CONFIG_IDF_TARGET_ESP32H2
#include "esp32h2/rom/spi_flash.h"
#elif CONFIG_IDF_TARGET_ESP8684
#include "esp8684/rom/spi_flash.h"
#elif __has_include("esp32/rom/spi_flash.h")
#include "esp32/rom/spi_flash.h" //IDF 4
#else
#include "rom/spi_flash.h" //IDF 3
#endif

#define CONFIG_LITTLEFS_BLOCK_SIZE 4096 /* ESP32 can only operate at 4kb */

/* File Descriptor Caching Params */
#define CONFIG_LITTLEFS_FD_CACHE_REALLOC_FACTOR 2  /* Amount to resize FD cache by */
#define CONFIG_LITTLEFS_FD_CACHE_MIN_SIZE 4  /* Minimum size of FD cache */
#define CONFIG_LITTLEFS_FD_CACHE_HYST 4  /* When shrinking, leave this many trailing FD slots available */

/**
 * @brief Last Modified Time
 *
 * Use 't' for ESP_LITTLEFS_ATTR_MTIME to match example:
 *     https://github.com/ARMmbed/littlefs/issues/23#issuecomment-482293539
 * And to match other external tools such as:
 *     https://github.com/earlephilhower/mklittlefs
 */
#define ESP_LITTLEFS_ATTR_MTIME ((uint8_t) 't')

// ESP_PARTITION_SUBTYPE_DATA_LITTLEFS was introduced in later patch versions of esp-idf.
// * v5.0.7
// * v5.1.4
// * v5.2.0
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 2, 0)
#ifndef ESP_PARTITION_SUBTYPE_DATA_LITTLEFS
#define ESP_PARTITION_SUBTYPE_DATA_LITTLEFS 0x83
#endif
#endif

/**
 * @brief littlefs DIR structure
 */
typedef struct {
    DIR dir;            /*!< VFS DIR struct */
    lfs_dir_t d;        /*!< littlefs DIR struct */
    struct dirent e;    /*!< Last open dirent */
    long offset;        /*!< Offset of the current dirent */
    char *path;         /*!< Requested directory name */
} vfs_littlefs_dir_t;

static int       vfs_littlefs_open(void* ctx, const char * path, int flags, int mode);
static ssize_t   vfs_littlefs_write(void* ctx, int fd, const void * data, size_t size);
static ssize_t   vfs_littlefs_read(void* ctx, int fd, void * dst, size_t size);
static ssize_t   vfs_littlefs_pwrite(void *ctx, int fd, const void *src, size_t size, off_t offset);
static ssize_t   vfs_littlefs_pread(void *ctx, int fd, void *dst, size_t size, off_t offset);
static int       vfs_littlefs_close(void* ctx, int fd);
static off_t     vfs_littlefs_lseek(void* ctx, int fd, off_t offset, int mode);
static int       vfs_littlefs_fsync(void* ctx, int fd);
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
static esp_vfs_t vfs_littlefs_create_struct(bool writeable);
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)

#ifdef CONFIG_VFS_SUPPORT_DIR
static int     vfs_littlefs_stat(void* ctx, const char * path, struct stat * st);
static int     vfs_littlefs_unlink(void* ctx, const char *path);
static int     vfs_littlefs_rename(void* ctx, const char *src, const char *dst);
static DIR*    vfs_littlefs_opendir(void* ctx, const char* name);
static int     vfs_littlefs_closedir(void* ctx, DIR* pdir);
static struct  dirent* vfs_littlefs_readdir(void* ctx, DIR* pdir);
static int     vfs_littlefs_readdir_r(void* ctx, DIR* pdir,
                                struct dirent* entry, struct dirent** out_dirent);
static long    vfs_littlefs_telldir(void* ctx, DIR* pdir);
static void    vfs_littlefs_seekdir(void* ctx, DIR* pdir, long offset);
static int     vfs_littlefs_mkdir(void* ctx, const char* name, mode_t mode);
static int     vfs_littlefs_rmdir(void* ctx, const char* name);
static ssize_t vfs_littlefs_truncate( void *ctx, const char *path, off_t size);

#ifdef ESP_LITTLEFS_ENABLE_FTRUNCATE
static int vfs_littlefs_ftruncate(void *ctx, int fd, off_t size);
#endif // ESP_LITTLEFS_ENABLE_FTRUNCATE

static void      esp_littlefs_dir_free(vfs_littlefs_dir_t *dir);
#endif

static void      esp_littlefs_take_efs_lock(void);
static esp_err_t esp_littlefs_init_efs(esp_littlefs_t** efs, const esp_partition_t* partition, bool read_only);
static esp_err_t esp_littlefs_init(const esp_vfs_littlefs_conf_t* conf, int *index);

static esp_err_t esp_littlefs_by_label(const char* label, int * index);
static esp_err_t esp_littlefs_by_partition(const esp_partition_t* part, int*index);
static int esp_littlefs_file_sync(esp_littlefs_t *efs, vfs_littlefs_file_t *file);

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
static esp_err_t esp_littlefs_by_sdmmc_handle(sdmmc_card_t *handle, int *index);
#endif

static esp_err_t esp_littlefs_get_empty(int *index);
static void      esp_littlefs_free(esp_littlefs_t ** efs);
static int       esp_littlefs_flags_conv(int m);

#if CONFIG_LITTLEFS_USE_MTIME
static int       vfs_littlefs_utime(void *ctx, const char *path, const struct utimbuf *times);
static int       esp_littlefs_update_mtime_attr(esp_littlefs_t *efs, const char *path, time_t t);
static time_t    esp_littlefs_get_mtime_attr(esp_littlefs_t *efs, const char *path);
static time_t    esp_littlefs_get_updated_time(esp_littlefs_t *efs, vfs_littlefs_file_t *file, const char *path);
#endif

#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
/* The only way in LittleFS to get info is via a path (lfs_stat), so it cannot
 * be done if the path isn't stored. */
static int     vfs_littlefs_fstat(void* ctx, int fd, struct stat * st);
#endif

#if CONFIG_LITTLEFS_SPIFFS_COMPAT
static void mkdirs(esp_littlefs_t * efs, const char *dir);
static void rmdirs(esp_littlefs_t * efs, const char *dir);
#endif  // CONFIG_LITTLEFS_SPIFFS_COMPAT

static int vfs_littlefs_fcntl(void* ctx, int fd, int cmd, int arg);

static int sem_take(esp_littlefs_t *efs);
static int sem_give(esp_littlefs_t *efs);
static esp_err_t format_from_efs(esp_littlefs_t *efs);
static void get_total_and_used_bytes(esp_littlefs_t *efs, size_t *total_bytes, size_t *used_bytes);

static SemaphoreHandle_t _efs_lock = NULL;
static esp_littlefs_t * _efs[CONFIG_LITTLEFS_MAX_PARTITIONS] = { 0 };

/********************
 * Helper Functions *
 ********************/


#if CONFIG_LITTLEFS_HUMAN_READABLE
/**
 * @brief converts an enumerated lfs error into a string.
 * @param lfs_errno The enumerated littlefs error.
 */
static const char * esp_littlefs_errno(enum lfs_error lfs_errno);
#endif

static inline void * esp_littlefs_calloc(size_t __nmemb, size_t __size) {
    /* Used internally by this wrapper only */
#if defined(CONFIG_LITTLEFS_MALLOC_STRATEGY_INTERNAL)
    return heap_caps_calloc(__nmemb, __size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
#elif defined(CONFIG_LITTLEFS_MALLOC_STRATEGY_SPIRAM)
    return heap_caps_calloc(__nmemb, __size, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#else /* CONFIG_LITTLEFS_MALLOC_STRATEGY_DISABLE, CONFIG_LITTLEFS_MALLOC_STRATEGY_DEFAULT or not defined */
    return calloc(__nmemb, __size);
#endif
}

static void esp_littlefs_free_fds(esp_littlefs_t * efs) {
    /* Need to free all files that were opened */
    while (efs->file) {
        vfs_littlefs_file_t * next = efs->file->next;
        free(efs->file);
        efs->file = next;
    }
    free(efs->cache);
    efs->cache = 0;
    efs->cache_size = efs->fd_count = 0;
}

static int lfs_errno_remap(enum lfs_error err) {
    switch(err){
        case LFS_ERR_OK:          return 0;
        case LFS_ERR_IO:          return EIO;
        case LFS_ERR_CORRUPT:     return EBADMSG;  // This is a bit opinionated.
        case LFS_ERR_NOENT:       return ENOENT;
        case LFS_ERR_EXIST:       return EEXIST;
        case LFS_ERR_NOTDIR:      return ENOTDIR;
        case LFS_ERR_ISDIR:       return EISDIR;
        case LFS_ERR_NOTEMPTY:    return ENOTEMPTY;
        case LFS_ERR_BADF:        return EBADF;
        case LFS_ERR_FBIG:        return EFBIG;
        case LFS_ERR_INVAL:       return EINVAL;
        case LFS_ERR_NOSPC:       return ENOSPC;
        case LFS_ERR_NOMEM:       return ENOMEM;
        case LFS_ERR_NOATTR:      return ENODATA;
        case LFS_ERR_NAMETOOLONG: return ENAMETOOLONG;
    }
    return EINVAL;  // Need some default vlaue
}

esp_err_t format_from_efs(esp_littlefs_t *efs)
{
    assert( efs );
    bool was_mounted = false;

    /* Unmount if mounted */
    if(efs->cache_size > 0){
        int res;
        ESP_LOGV(ESP_LITTLEFS_TAG, "Partition was mounted. Unmounting...");
        was_mounted = true;
        res = lfs_unmount(efs->fs);
        if(res != LFS_ERR_OK){
            ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to unmount.");
            return ESP_FAIL;
        }
        esp_littlefs_free_fds(efs);
    }

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
    /* Format the SD card too */
    if (efs->sdcard) {
        esp_err_t ret = sdmmc_full_erase(efs->sdcard);
        if (ret != ESP_OK) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to format SD card: 0x%x %s", ret, esp_err_to_name(ret));
            return ret;
        }

        ESP_LOGI(ESP_LITTLEFS_TAG, "SD card formatted!");
    }
#endif

    /* Format */
    {
        esp_err_t res = ESP_OK;
        ESP_LOGV(ESP_LITTLEFS_TAG, "Formatting filesystem");

        /* Need to write explicit block_count to cfg; but skip if it's the SD card */
#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
        if (efs->sdcard) {
            res = lfs_format(efs->fs, &efs->cfg);
        } else
#endif
        {
            efs->cfg.block_count = efs->partition->size / efs->cfg.block_size;
            res = lfs_format(efs->fs, &efs->cfg);
            efs->cfg.block_count = 0;
        }

        if( res != LFS_ERR_OK ) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to format filesystem");
            return ESP_FAIL;
        }
    }

    /* Mount filesystem */
    if( was_mounted ) {
        int res;
        /* Remount the partition */
        ESP_LOGV(ESP_LITTLEFS_TAG, "Remounting formatted partition");
        res = lfs_mount(efs->fs, &efs->cfg);
        if( res != LFS_ERR_OK ) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to re-mount filesystem");
            return ESP_FAIL;
        }
        efs->cache_size = CONFIG_LITTLEFS_FD_CACHE_MIN_SIZE;  // Initial size of cache; will resize ondemand
        efs->cache = esp_littlefs_calloc(efs->cache_size, sizeof(*efs->cache));
    }
    ESP_LOGV(ESP_LITTLEFS_TAG, "Format Success!");

    return ESP_OK;
}

void get_total_and_used_bytes(esp_littlefs_t *efs, size_t *total_bytes, size_t *used_bytes) {
    sem_take(efs);
    size_t total_bytes_local = efs->cfg.block_size * efs->fs->block_count;
    if(total_bytes) *total_bytes = total_bytes_local;

    /* lfs_fs_size may return a size larger than the actual filesystem size.
     * https://github.com/littlefs-project/littlefs/blob/9c7e232086f865cff0bb96fe753deb66431d91fd/lfs.h#L658
     */
    if(used_bytes) *used_bytes = MIN(total_bytes_local, efs->cfg.block_size * lfs_fs_size(efs->fs));
    sem_give(efs);
}

/********************
 * Public Functions *
 ********************/

bool esp_littlefs_mounted(const char* partition_label) {
    int index;
    esp_err_t err;

    err = esp_littlefs_by_label(partition_label, &index);
    if(err != ESP_OK) return false;
    return _efs[index]->cache_size > 0;
}

bool esp_littlefs_partition_mounted(const esp_partition_t* partition) {
    int index;
    esp_err_t err = esp_littlefs_by_partition(partition, &index);

    if(err != ESP_OK) return false;
    return _efs[index]->cache_size > 0;
}

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
bool esp_littlefs_sdmmc_mounted(sdmmc_card_t *sdcard)
{
    int index;
    esp_err_t err = esp_littlefs_by_sdmmc_handle(sdcard, &index);

    if(err != ESP_OK) return false;
    return _efs[index]->cache_size > 0;
}
#endif

esp_err_t esp_littlefs_info(const char* partition_label, size_t *total_bytes, size_t *used_bytes){
    int index;
    esp_err_t err;

    err = esp_littlefs_by_label(partition_label, &index);
    if(err != ESP_OK) return err;
    get_total_and_used_bytes(_efs[index], total_bytes, used_bytes);

    return ESP_OK;
}

esp_err_t esp_littlefs_partition_info(const esp_partition_t* partition, size_t *total_bytes, size_t *used_bytes){
    int index;
    esp_err_t err;

    err = esp_littlefs_by_partition(partition, &index);
    if(err != ESP_OK) return err;
    get_total_and_used_bytes(_efs[index], total_bytes, used_bytes);

    return ESP_OK;
}

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
esp_err_t esp_littlefs_sdmmc_info(sdmmc_card_t *sdcard, size_t *total_bytes, size_t *used_bytes)
{
    int index;
    esp_err_t err;

    err = esp_littlefs_by_sdmmc_handle(sdcard, &index);
    if(err != ESP_OK) return err;
    get_total_and_used_bytes(_efs[index], total_bytes, used_bytes);

    return ESP_OK;
}
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)

#ifdef CONFIG_VFS_SUPPORT_DIR
static esp_vfs_dir_ops_t s_vfs_littlefs_dir = {
    .stat_p      = &vfs_littlefs_stat,
    .link_p      = NULL, /* Not Supported */
    .unlink_p    = &vfs_littlefs_unlink,
    .rename_p    = &vfs_littlefs_rename,
    .opendir_p   = &vfs_littlefs_opendir,
    .readdir_p   = &vfs_littlefs_readdir,
    .readdir_r_p = &vfs_littlefs_readdir_r,
    .telldir_p   = &vfs_littlefs_telldir,
    .seekdir_p   = &vfs_littlefs_seekdir,
    .closedir_p  = &vfs_littlefs_closedir,
    .mkdir_p     = &vfs_littlefs_mkdir,
    .rmdir_p     = &vfs_littlefs_rmdir,
    // access_p
	.truncate_p  = &vfs_littlefs_truncate,
#ifdef ESP_LITTLEFS_ENABLE_FTRUNCATE
    .ftruncate_p = &vfs_littlefs_ftruncate,
#endif // ESP_LITTLEFS_ENABLE_FTRUNCATE
#if CONFIG_LITTLEFS_USE_MTIME
    .utime_p     = &vfs_littlefs_utime,
#endif // CONFIG_LITTLEFS_USE_MTIME
};
#endif // CONFIG_VFS_SUPPORT_DIR

static esp_vfs_fs_ops_t s_vfs_littlefs = {
    .write_p     = &vfs_littlefs_write,
    .pwrite_p    = &vfs_littlefs_pwrite,
    .lseek_p     = &vfs_littlefs_lseek,
    .read_p      = &vfs_littlefs_read,
    .pread_p     = &vfs_littlefs_pread,
    .open_p      = &vfs_littlefs_open,
    .close_p     = &vfs_littlefs_close,
    .fsync_p     = &vfs_littlefs_fsync,
    .fcntl_p     = &vfs_littlefs_fcntl,
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
    .fstat_p     = &vfs_littlefs_fstat,
#endif
#ifdef CONFIG_VFS_SUPPORT_DIR
    .dir = &s_vfs_littlefs_dir,
#endif // CONFIG_VFS_SUPPORT_DIR
};

#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
esp_err_t esp_vfs_littlefs_register(const esp_vfs_littlefs_conf_t * conf)
{
    int index;
    assert(conf->base_path);
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
    const esp_vfs_t vfs = vfs_littlefs_create_struct(!conf->read_only);
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)

    esp_err_t err = esp_littlefs_init(conf, &index);
    if (err != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to initialize LittleFS");
        return err;
    }

    strlcat(_efs[index]->base_path, conf->base_path, ESP_VFS_PATH_MAX + 1);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
    int flags = ESP_VFS_FLAG_CONTEXT_PTR | ESP_VFS_FLAG_STATIC; 
    if (conf->read_only) {
        flags |= ESP_VFS_FLAG_READONLY_FS;
    }
    err = esp_vfs_register_fs(conf->base_path, &s_vfs_littlefs, flags, _efs[index]);
#else
    err = esp_vfs_register(conf->base_path, &vfs, _efs[index]);
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
    if (err != ESP_OK) {
        esp_littlefs_free(&_efs[index]);
        ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to register Littlefs to \"%s\"", conf->base_path);
        return err;
    }

    ESP_LOGV(ESP_LITTLEFS_TAG, "Successfully registered LittleFS to \"%s\"", conf->base_path);
    return ESP_OK;
}

esp_err_t esp_vfs_littlefs_unregister(const char* partition_label)
{
    int index;
    if (esp_littlefs_by_label(partition_label, &index) != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "Partition was never registered.");
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGV(ESP_LITTLEFS_TAG, "Unregistering \"%s\"", partition_label);
    esp_err_t err = esp_vfs_unregister(_efs[index]->base_path);
    if (err != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to unregister \"%s\"", partition_label);
        return err;
    }
    esp_littlefs_free(&_efs[index]);
    _efs[index] = NULL;
    return ESP_OK;
}

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
esp_err_t esp_vfs_littlefs_unregister_sdmmc(sdmmc_card_t *sdcard)
{
    assert(sdcard);
    int index;
    if (esp_littlefs_by_sdmmc_handle(sdcard, &index) != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "Partition was never registered.");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGV(ESP_LITTLEFS_TAG, "Unregistering SD card \"%p\"", sdcard);
    esp_err_t err = esp_vfs_unregister(_efs[index]->base_path);
    if (err != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to unregister SD card \"%p\"", sdcard);
        return err;
    }

    esp_littlefs_free(&_efs[index]);
    _efs[index] = NULL;
    return ESP_OK;
}
#endif

esp_err_t esp_vfs_littlefs_unregister_partition(const esp_partition_t* partition) {
    assert(partition);
    int index;
    if (esp_littlefs_by_partition(partition, &index) != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "Partition was never registered.");
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGV(ESP_LITTLEFS_TAG, "Unregistering \"0x%08"PRIX32"\"", partition->address);
    esp_err_t err = esp_vfs_unregister(_efs[index]->base_path);
    if (err != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to unregister \"0x%08"PRIX32"\"", partition->address);
        return err;
    }
    esp_littlefs_free(&_efs[index]);
    _efs[index] = NULL;
    return ESP_OK;
}

esp_err_t esp_littlefs_format(const char* partition_label) {
    bool efs_free = false;
    int index = -1;
    esp_err_t err;

    ESP_LOGV(ESP_LITTLEFS_TAG, "Formatting \"%s\"", partition_label);

    /* Get a context */
    err = esp_littlefs_by_label(partition_label, &index);

    if( err != ESP_OK ){
        /* Create a tmp context */
        ESP_LOGV(ESP_LITTLEFS_TAG, "Temporarily creating EFS context.");
        efs_free = true;
        const esp_vfs_littlefs_conf_t conf = {
                /* base_name not necessary for initializing */
                .dont_mount = true,
                .partition_label = partition_label,
        };
        err = esp_littlefs_init(&conf, &index);
        if( err != ESP_OK ) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to initialize to format.");
            goto exit;
        }
    }

    err = format_from_efs(_efs[index]);

exit:
    if(efs_free && index>=0) esp_littlefs_free(&_efs[index]);
    return err;
}

esp_err_t esp_littlefs_format_partition(const esp_partition_t* partition) {
    assert( partition );

    bool efs_free = false;
    int index = -1;
    esp_err_t err;

    ESP_LOGV(ESP_LITTLEFS_TAG, "Formatting partition at \"0x%08"PRIX32"\"", partition->address);

    /* Get a context */
    err = esp_littlefs_by_partition(partition, &index);

    if( err != ESP_OK ){
        /* Create a tmp context */
        ESP_LOGV(ESP_LITTLEFS_TAG, "Temporarily creating EFS context.");
        efs_free = true;
        const esp_vfs_littlefs_conf_t conf = {
                /* base_name not necessary for initializing */
                .dont_mount = true,
                .partition_label = NULL,
                .partition = partition,
        };
        err = esp_littlefs_init(&conf, &index);
        if( err != ESP_OK ) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to initialize to format.");
            goto exit;
        }
    }

    err = format_from_efs(_efs[index]);

exit:
    if(efs_free && index>=0) esp_littlefs_free(&_efs[index]);
    return err;
}

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
esp_err_t esp_littlefs_format_sdmmc(sdmmc_card_t *sdcard)
{
    assert(sdcard);

    bool efs_free = false;
    int index = -1;
    esp_err_t err;

    ESP_LOGV(ESP_LITTLEFS_TAG, "Formatting sdcard %p", sdcard);

    /* Get a context */
    err = esp_littlefs_by_sdmmc_handle(sdcard, &index);

    if( err != ESP_OK ){
        /* Create a tmp context */
        ESP_LOGV(ESP_LITTLEFS_TAG, "Temporarily creating EFS context.");
        efs_free = true;
        const esp_vfs_littlefs_conf_t conf = {
                /* base_name not necessary for initializing */
                .dont_mount = true,
                .partition_label = NULL,
                .partition = NULL,
                .sdcard = sdcard,
        };

        err = esp_littlefs_init(&conf, &index);
        if( err != ESP_OK ) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Failed to initialize to format.");
            goto exit;
        }
    }

    err = format_from_efs(_efs[index]);

exit:
    if(efs_free && index>=0) esp_littlefs_free(&_efs[index]);
    return err;
}
#endif

/********************
 * Static Functions *
 ********************/

/*** Helpers ***/

#if CONFIG_LITTLEFS_HUMAN_READABLE
/**
 * @brief converts an enumerated lfs error into a string.
 * @param lfs_error The littlefs error.
 */
static const char * esp_littlefs_errno(enum lfs_error lfs_errno) {
    switch(lfs_errno){
        case LFS_ERR_OK: return "LFS_ERR_OK";
        case LFS_ERR_IO: return "LFS_ERR_IO";
        case LFS_ERR_CORRUPT: return "LFS_ERR_CORRUPT";
        case LFS_ERR_NOENT: return "LFS_ERR_NOENT";
        case LFS_ERR_EXIST: return "LFS_ERR_EXIST";
        case LFS_ERR_NOTDIR: return "LFS_ERR_NOTDIR";
        case LFS_ERR_ISDIR: return "LFS_ERR_ISDIR";
        case LFS_ERR_NOTEMPTY: return "LFS_ERR_NOTEMPTY";
        case LFS_ERR_BADF: return "LFS_ERR_BADF";
        case LFS_ERR_FBIG: return "LFS_ERR_FBIG";
        case LFS_ERR_INVAL: return "LFS_ERR_INVAL";
        case LFS_ERR_NOSPC: return "LFS_ERR_NOSPC";
        case LFS_ERR_NOMEM: return "LFS_ERR_NOMEM";
        case LFS_ERR_NOATTR: return "LFS_ERR_NOATTR";
        case LFS_ERR_NAMETOOLONG: return "LFS_ERR_NAMETOOLONG";
        default: return "LFS_ERR_UNDEFINED";
    }
    return "";
}
#else
#define esp_littlefs_errno(x) ""
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
static esp_vfs_t vfs_littlefs_create_struct(bool writeable) {
    esp_vfs_t vfs = {
        .flags       = ESP_VFS_FLAG_CONTEXT_PTR,
        .write_p     = &vfs_littlefs_write,
        .pwrite_p    = &vfs_littlefs_pwrite,
        .lseek_p     = &vfs_littlefs_lseek,
        .read_p      = &vfs_littlefs_read,
        .pread_p     = &vfs_littlefs_pread,
        .open_p      = &vfs_littlefs_open,
        .close_p     = &vfs_littlefs_close,
        .fsync_p     = &vfs_littlefs_fsync,
        .fcntl_p     = &vfs_littlefs_fcntl,
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        .fstat_p     = &vfs_littlefs_fstat,
#endif
#ifdef CONFIG_VFS_SUPPORT_DIR
        .stat_p      = &vfs_littlefs_stat,
        .link_p      = NULL, /* Not Supported */
        .unlink_p    = &vfs_littlefs_unlink,
        .rename_p    = &vfs_littlefs_rename,
        .opendir_p   = &vfs_littlefs_opendir,
        .readdir_p   = &vfs_littlefs_readdir,
        .readdir_r_p = &vfs_littlefs_readdir_r,
        .telldir_p   = &vfs_littlefs_telldir,
        .seekdir_p   = &vfs_littlefs_seekdir,
        .closedir_p  = &vfs_littlefs_closedir,
        .mkdir_p     = &vfs_littlefs_mkdir,
        .rmdir_p     = &vfs_littlefs_rmdir,
        // access_p
		.truncate_p  = &vfs_littlefs_truncate,
#ifdef ESP_LITTLEFS_ENABLE_FTRUNCATE
        .ftruncate_p = &vfs_littlefs_ftruncate,
#endif // ESP_LITTLEFS_ENABLE_FTRUNCATE
#if CONFIG_LITTLEFS_USE_MTIME
        .utime_p     = &vfs_littlefs_utime,
#endif // CONFIG_LITTLEFS_USE_MTIME
#endif // CONFIG_VFS_SUPPORT_DIR
};
    if(!writeable) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
        vfs.flags |= ESP_VFS_FLAG_READONLY_FS;
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
        vfs.write_p  = NULL;
        vfs.pwrite_p = NULL;
        vfs.fsync_p  = NULL;
        vfs.link_p   = NULL;
        vfs.unlink_p = NULL;
        vfs.rename_p = NULL;
        vfs.mkdir_p  = NULL;
        vfs.rmdir_p  = NULL;
    }
    return vfs;
}
#endif // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)

/**
 * @brief Free and clear a littlefs definition structure.
 * @param efs Pointer to pointer to struct. Done this way so we can also zero
 *            out the pointer.
 */
static void esp_littlefs_free(esp_littlefs_t ** efs)
{
    esp_littlefs_t * e = *efs;
    if (e == NULL) return;
    *efs = NULL;

    if (e->fs) {
        if(e->cache_size > 0) lfs_unmount(e->fs);
        free(e->fs);
    }
    if(e->lock) vSemaphoreDelete(e->lock);

#ifdef CONFIG_LITTLEFS_MMAP_PARTITION
    esp_partition_munmap(e->mmap_handle);
#endif

    esp_littlefs_free_fds(e);
    free(e);
}

#ifdef CONFIG_VFS_SUPPORT_DIR
/**
 * @brief Free a vfs_littlefs_dir_t struct.
 */
static void esp_littlefs_dir_free(vfs_littlefs_dir_t *dir){
    if(dir == NULL) return;
    if(dir->path) free(dir->path);
    free(dir);
}
#endif

/**
 * Get a mounted littlefs filesystem by label.
 * @param[in] label
 * @param[out] index index into _efs
 * @return ESP_OK on success
 */
static esp_err_t esp_littlefs_by_partition(const esp_partition_t* part, int * index){
    int i;
    esp_littlefs_t * p;

    if(!part || !index) return ESP_ERR_INVALID_ARG;

    ESP_LOGV(ESP_LITTLEFS_TAG, "Searching for existing filesystem for partition \"0x%08"PRIX32"\"", part->address);

    for (i = 0; i < CONFIG_LITTLEFS_MAX_PARTITIONS; i++) {
        p = _efs[i];
        if (!p) continue;
        if (!p->partition) continue;
        if (part->address == p->partition->address) {
            *index = i;
            ESP_LOGV(ESP_LITTLEFS_TAG, "Found existing filesystem \"0x%08"PRIX32"\" at index %d", part->address, *index);
            return ESP_OK;
        }
    }

    ESP_LOGV(ESP_LITTLEFS_TAG, "Existing filesystem \"0x%08"PRIX32"\" not found", part->address);
    return ESP_ERR_NOT_FOUND;
}

/**
 * @brief Find index of already mounted littlefs filesystem by label.
 * @param[in] label
 * @param[out] index
 */
static esp_err_t esp_littlefs_by_label(const char* label, int * index){
    int i;
    esp_littlefs_t * p;
    const esp_partition_t *partition;

    if(!index) return ESP_ERR_INVALID_ARG;
    if(!label){
        // Search for first dat partition with subtype "littlefs"
        partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA,
                ESP_PARTITION_SUBTYPE_DATA_LITTLEFS,
                NULL
        );
        if(!partition){
            ESP_LOGE(ESP_LITTLEFS_TAG, "No data partition with subtype \"littlefs\" found");
            return ESP_ERR_NOT_FOUND;
        }
        label = partition->label;
    }

    ESP_LOGV(ESP_LITTLEFS_TAG, "Searching for existing filesystem for partition \"%s\"", label);

    for (i = 0; i < CONFIG_LITTLEFS_MAX_PARTITIONS; i++) {
        p = _efs[i];
        if (!p) continue;
        if (!p->partition) continue;
        if (strncmp(label, p->partition->label, 17) == 0) {
            *index = i;
            ESP_LOGV(ESP_LITTLEFS_TAG, "Found existing filesystem \"%s\" at index %d", label, *index);
            return ESP_OK;
        }
    }

    ESP_LOGV(ESP_LITTLEFS_TAG, "Existing filesystem \"%s\" not found", label);
    return ESP_ERR_NOT_FOUND;
}

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
static esp_err_t esp_littlefs_by_sdmmc_handle(sdmmc_card_t *handle, int *index)
{
    if(!handle || !index) return ESP_ERR_INVALID_ARG;

    ESP_LOGV(ESP_LITTLEFS_TAG, "Searching for existing filesystem for SD handle %p", handle);

    for (int i = 0; i < CONFIG_LITTLEFS_MAX_PARTITIONS; i++) {
        esp_littlefs_t *p = _efs[i];
        if (!p) continue;
        if (!p->sdcard) continue;
        if (p->sdcard == handle) {
            *index = i;
            ESP_LOGV(ESP_LITTLEFS_TAG, "Found existing filesystem %p at index %d", handle, *index);
            return ESP_OK;
        }
    }

    ESP_LOGV(ESP_LITTLEFS_TAG, "Existing filesystem %p not found", handle);
    return ESP_ERR_NOT_FOUND;
}
#endif

/**
 * @brief Get the index of an unallocated LittleFS slot.
 * @param[out] index Indexd of free LittleFS slot
 * @return ESP_OK on success
 */
static esp_err_t esp_littlefs_get_empty(int *index) {
    assert(index);
    for(uint8_t i=0; i < CONFIG_LITTLEFS_MAX_PARTITIONS; i++){
        if( _efs[i] == NULL ){
            *index = i;
            return ESP_OK;
        }
    }
    ESP_LOGE(ESP_LITTLEFS_TAG, "No more free partitions available.");
    return ESP_FAIL;
}

/**
 * @brief Convert fcntl flags to littlefs flags
 * @param m fcntl flags
 * @return lfs flags
 */
static int esp_littlefs_flags_conv(int m) {
    int lfs_flags = 0;

    // Mask out unsupported flags; can cause internal LFS issues.
    m &= (O_APPEND | O_WRONLY | O_RDWR | O_EXCL | O_CREAT | O_TRUNC);

    // O_RDONLY is 0 and not a flag, so must be explicitly checked
    if (m == O_RDONLY)  {ESP_LOGV(ESP_LITTLEFS_TAG, "O_RDONLY"); lfs_flags |= LFS_O_RDONLY;}

    if (m & O_APPEND)  {ESP_LOGV(ESP_LITTLEFS_TAG, "O_APPEND"); lfs_flags |= LFS_O_APPEND;}
    if (m & O_WRONLY)  {ESP_LOGV(ESP_LITTLEFS_TAG, "O_WRONLY"); lfs_flags |= LFS_O_WRONLY;}
    if (m & O_RDWR)    {ESP_LOGV(ESP_LITTLEFS_TAG, "O_RDWR");   lfs_flags |= LFS_O_RDWR;}
    if (m & O_EXCL)    {ESP_LOGV(ESP_LITTLEFS_TAG, "O_EXCL");   lfs_flags |= LFS_O_EXCL;}
    if (m & O_CREAT)   {ESP_LOGV(ESP_LITTLEFS_TAG, "O_CREAT");  lfs_flags |= LFS_O_CREAT;}
    if (m & O_TRUNC)   {ESP_LOGV(ESP_LITTLEFS_TAG, "O_TRUNC");  lfs_flags |= LFS_O_TRUNC;}
    return lfs_flags;
}

static void esp_littlefs_take_efs_lock(void) {
    if( _efs_lock == NULL ){
#ifdef ESP8266
        taskENTER_CRITICAL();
#else
        static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
        portENTER_CRITICAL(&mux);
#endif
        if( _efs_lock == NULL ){
            _efs_lock = xSemaphoreCreateMutex();
            assert(_efs_lock);
        }
#ifdef ESP8266
        taskEXIT_CRITICAL();
#else
        portEXIT_CRITICAL(&mux);
#endif
    }

    xSemaphoreTake(_efs_lock, portMAX_DELAY);
}


#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
static esp_err_t esp_littlefs_init_sdcard(esp_littlefs_t** efs, sdmmc_card_t* sdcard, bool read_only)
{
    /* Allocate Context */
    *efs = esp_littlefs_calloc(1, sizeof(esp_littlefs_t));
    if (*efs == NULL) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "esp_littlefs could not be malloced");
        return ESP_ERR_NO_MEM;
    }
    (*efs)->sdcard = sdcard;

    { /* LittleFS Configuration */
        (*efs)->cfg.context = *efs;
        (*efs)->read_only = read_only;

        // block device operations
        (*efs)->cfg.read  = littlefs_sdmmc_read;
        (*efs)->cfg.prog  = littlefs_sdmmc_write;
        (*efs)->cfg.erase = littlefs_sdmmc_erase;
        (*efs)->cfg.sync  = littlefs_sdmmc_sync;

        // block device configuration
        (*efs)->cfg.read_size = sdcard->csd.sector_size;
        (*efs)->cfg.prog_size = sdcard->csd.sector_size;
        (*efs)->cfg.block_size = sdcard->csd.sector_size;
        (*efs)->cfg.block_count = sdcard->csd.capacity;
        (*efs)->cfg.cache_size = MAX(CONFIG_LITTLEFS_CACHE_SIZE, sdcard->csd.sector_size); // Must not be smaller than SD sector size
        (*efs)->cfg.lookahead_size = CONFIG_LITTLEFS_LOOKAHEAD_SIZE;
        (*efs)->cfg.block_cycles = CONFIG_LITTLEFS_BLOCK_CYCLES;
#if CONFIG_LITTLEFS_MULTIVERSION
        #if CONFIG_LITTLEFS_DISK_VERSION_MOST_RECENT
        (*efs)->cfg.disk_version = 0;
#elif CONFIG_LITTLEFS_DISK_VERSION_2_1
        (*efs)->cfg.disk_version = 0x00020001;
#elif CONFIG_LITTLEFS_DISK_VERSION_2_0
        (*efs)->cfg.disk_version = 0x00020000;
#else
#error "CONFIG_LITTLEFS_MULTIVERSION enabled but no or unknown disk version selected!"
#endif
#endif
    }

    (*efs)->lock = xSemaphoreCreateRecursiveMutex();
    if ((*efs)->lock == NULL) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "mutex lock could not be created");
        return ESP_ERR_NO_MEM;
    }

    (*efs)->fs = esp_littlefs_calloc(1, sizeof(lfs_t));
    if ((*efs)->fs == NULL) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "littlefs could not be malloced");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}
#endif // CONFIG_LITTLEFS_SDMMC_SUPPORT

static esp_err_t esp_littlefs_init_efs(esp_littlefs_t** efs, const esp_partition_t* partition, bool read_only)
{
    /* Allocate Context */
    *efs = esp_littlefs_calloc(1, sizeof(esp_littlefs_t));
    if (*efs == NULL) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "esp_littlefs could not be malloced");
        return ESP_ERR_NO_MEM;
    }
    (*efs)->partition = partition;

#ifdef CONFIG_LITTLEFS_MMAP_PARTITION
    esp_err_t err = esp_partition_mmap(partition, 0, partition->size, SPI_FLASH_MMAP_DATA, &(*efs)->mmap_data, &(*efs)->mmap_handle);
    if (err != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "esp_littlefs could not map data");
        return err;
    }
#endif

    { /* LittleFS Configuration */
        (*efs)->cfg.context = *efs;
        (*efs)->read_only = read_only;

        // block device operations
#ifdef CONFIG_LITTLEFS_MMAP_PARTITION
        (*efs)->cfg.read  = littlefs_esp_part_read_mmap;
#else
        (*efs)->cfg.read  = littlefs_esp_part_read;
#endif
        (*efs)->cfg.prog  = littlefs_esp_part_write;
        (*efs)->cfg.erase = littlefs_esp_part_erase;
        (*efs)->cfg.sync  = littlefs_esp_part_sync;

        // block device configuration
        (*efs)->cfg.read_size = CONFIG_LITTLEFS_READ_SIZE;
        (*efs)->cfg.prog_size = CONFIG_LITTLEFS_WRITE_SIZE;
        (*efs)->cfg.block_size = CONFIG_LITTLEFS_BLOCK_SIZE;
        (*efs)->cfg.block_count = 0;  // Autodetect ``block_count``
        (*efs)->cfg.cache_size = CONFIG_LITTLEFS_CACHE_SIZE;
        (*efs)->cfg.lookahead_size = CONFIG_LITTLEFS_LOOKAHEAD_SIZE;
        (*efs)->cfg.block_cycles = CONFIG_LITTLEFS_BLOCK_CYCLES;
#if CONFIG_LITTLEFS_MULTIVERSION
#if CONFIG_LITTLEFS_DISK_VERSION_MOST_RECENT
        (*efs)->cfg.disk_version = 0;
#elif CONFIG_LITTLEFS_DISK_VERSION_2_1
        (*efs)->cfg.disk_version = 0x00020001;
#elif CONFIG_LITTLEFS_DISK_VERSION_2_0
        (*efs)->cfg.disk_version = 0x00020000;
#else
#error "CONFIG_LITTLEFS_MULTIVERSION enabled but no or unknown disk version selected!"
#endif
#endif
    }

    (*efs)->lock = xSemaphoreCreateRecursiveMutex();
    if ((*efs)->lock == NULL) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "mutex lock could not be created");
        return ESP_ERR_NO_MEM;
    }

    (*efs)->fs = esp_littlefs_calloc(1, sizeof(lfs_t));
    if ((*efs)->fs == NULL) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "littlefs could not be malloced");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

/**
 * @brief Initialize and mount littlefs
 * @param[in] conf Filesystem Configuration
 * @param[out] index On success, index into _efs.
 * @return ESP_OK on success
 */
static esp_err_t esp_littlefs_init(const esp_vfs_littlefs_conf_t* conf, int *index)
{
    esp_err_t err = ESP_FAIL;
    const esp_partition_t* partition = NULL;
    esp_littlefs_t * efs = NULL;
    *index = -1;

    esp_littlefs_take_efs_lock();

    if (esp_littlefs_get_empty(index) != ESP_OK) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "max mounted partitions reached");
        err = ESP_ERR_INVALID_STATE;
        goto exit;
    }

    if(conf->partition_label)
    {
        /* Input and Environment Validation */
        if (esp_littlefs_by_label(conf->partition_label, index) == ESP_OK) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Partition already used");
            err = ESP_ERR_INVALID_STATE;
            goto exit;
        }
        partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA,
                ESP_PARTITION_SUBTYPE_ANY,
                conf->partition_label);
        if (!partition) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "partition \"%s\" could not be found", conf->partition_label);
            err = ESP_ERR_NOT_FOUND;
            goto exit;
        }

    } else if(conf->partition) {
        if (esp_littlefs_by_partition(conf->partition, index) == ESP_OK) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Partition already used");
            err = ESP_ERR_INVALID_STATE;
            goto exit;
        }
        partition = conf->partition;
#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
    } else if (conf->sdcard) {
        ESP_LOGV(ESP_LITTLEFS_TAG, "Using SD card handle %p for LittleFS mount", conf->sdcard);
        err = sdmmc_get_status(conf->sdcard);
        if (err != ESP_OK) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Failed when checking SD card status: 0x%x", err);
            goto exit;
        }
#endif
    } else {
        // Find first partition with "littlefs" subtype.
        partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA,
                ESP_PARTITION_SUBTYPE_DATA_LITTLEFS,
                NULL
        );
        if (!partition) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "No data partition with subtype \"littlefs\" found");
            err = ESP_ERR_NOT_FOUND;
            goto exit;
        }
    }

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
	if (conf->sdcard) {
        err = esp_littlefs_init_sdcard(&efs, conf->sdcard, conf->read_only);
        if(err != ESP_OK) {
            goto exit;
        }
    } else
#endif
    {
        uint32_t flash_page_size = g_rom_flashchip.page_size;
        uint32_t log_page_size = CONFIG_LITTLEFS_PAGE_SIZE;
        if (log_page_size % flash_page_size != 0) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "LITTLEFS_PAGE_SIZE is not multiple of flash chip page size (%u)",
                     (unsigned int) flash_page_size);
            err = ESP_ERR_INVALID_ARG;
            goto exit;
        }

        err = esp_littlefs_init_efs(&efs, partition, conf->read_only);

        if(err != ESP_OK) {
            goto exit;
        }
    }

    // Mount and Error Check
    _efs[*index] = efs;
    if(!conf->dont_mount){
        int res;

        res = lfs_mount(efs->fs, &efs->cfg);

        if (conf->format_if_mount_failed && res != LFS_ERR_OK) {
            ESP_LOGW(ESP_LITTLEFS_TAG, "mount failed, %s (%i). formatting...", esp_littlefs_errno(res), res);
#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
            if (conf->sdcard) {
                err = esp_littlefs_format_sdmmc(conf->sdcard);
            } else
#endif
            {
                err = esp_littlefs_format_partition(efs->partition);
            }
            if(err != ESP_OK) {
                ESP_LOGE(ESP_LITTLEFS_TAG, "format failed");
                err = ESP_FAIL;
                goto exit;
            }
            res = lfs_mount(efs->fs, &efs->cfg);
        }
        if (res != LFS_ERR_OK) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "mount failed, %s (%i)", esp_littlefs_errno(res), res);
            err = ESP_FAIL;
            goto exit;
        }
        efs->cache_size = 4;
        efs->cache = esp_littlefs_calloc(efs->cache_size, sizeof(*efs->cache));

        if(conf->grow_on_mount){
#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
            if (efs->sdcard) {
                res = lfs_fs_grow(efs->fs, efs->sdcard->csd.capacity);
            } else
#endif
            {
                res = lfs_fs_grow(efs->fs, efs->partition->size / efs->cfg.block_size);
            }
            if (res != LFS_ERR_OK) {
                ESP_LOGE(ESP_LITTLEFS_TAG, "FS grow failed, %s (%i)", esp_littlefs_errno(res), res);
                err = ESP_FAIL;
                goto exit;
            }
        }
    }

    err = ESP_OK;

exit:
    if(err != ESP_OK){
        if( *index >= 0 ) {
            esp_littlefs_free(&_efs[*index]);
        }
        else{
            esp_littlefs_free(&efs);
        }
    }
    xSemaphoreGive(_efs_lock);
    return err;
}

/**
 * @brief
 * @parameter efs file system context
 */
static inline int sem_take(esp_littlefs_t *efs) {
    int res;
#if LOG_LOCAL_LEVEL >= 5
    ESP_LOGV(ESP_LITTLEFS_TAG, "------------------------ Sem Taking [%s]", pcTaskGetName(NULL));
#endif
    res = xSemaphoreTakeRecursive(efs->lock, portMAX_DELAY);
#if LOG_LOCAL_LEVEL >= 5
    ESP_LOGV(ESP_LITTLEFS_TAG, "--------------------->>> Sem Taken [%s]", pcTaskGetName(NULL));
#endif
    return res;
}

/**
 * @brief
 * @parameter efs file system context
 */
static inline int sem_give(esp_littlefs_t *efs) {
#if LOG_LOCAL_LEVEL >= 5
    ESP_LOGV(ESP_LITTLEFS_TAG, "---------------------<<< Sem Give [%s]", pcTaskGetName(NULL));
#endif
    return xSemaphoreGiveRecursive(efs->lock);
}


/* We are using a double allocation system here, which an array and a linked list.
   The array contains the pointer to the file descriptor (the index in the array is what's returned to the user).
   The linked list is used for file descriptors.
   This means that position of nodes in the list must stay consistent:
   - Allocation is obvious (append to the list from the head, and realloc the pointers array)
     There is still a O(N) search in the cache for a free position to store
   - Searching is a O(1) process (good)
   - Deallocation is more tricky. That is, for example,
     if you need to remove node 5 in a 12 nodes list, you'll have to:
       1) Mark the 5th position as freed (if it's the last position of the array realloc smaller)
       2) Walk the list until finding the pointer to the node O(N) and scrub the node so the chained list stays consistent
       3) Deallocate the node
*/

/**
 * @brief Get a file descriptor
 * @param[in,out] efs       file system context
 * @param[out]    file      pointer to a file that'll be filled with a file object
 * @param[in]     path_len  the length of the filepath in bytes (including terminating zero byte)
 * @return integer file descriptor. Returns -1 if a FD cannot be obtained.
 * @warning This must be called with lock taken
 */
static int esp_littlefs_allocate_fd(esp_littlefs_t *efs, vfs_littlefs_file_t ** file
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
  , const size_t path_len
#endif
    )
{
    int i = -1;

    assert( efs->fd_count < UINT16_MAX );
    assert( efs->cache_size < UINT16_MAX );

    /* Make sure there is enough space in the cache to store new fd */
    if (efs->fd_count + 1 > efs->cache_size) {
        uint16_t new_size = (uint16_t)MIN(UINT16_MAX, CONFIG_LITTLEFS_FD_CACHE_REALLOC_FACTOR * efs->cache_size);
        /* Resize the cache */
        vfs_littlefs_file_t ** new_cache = realloc(efs->cache, new_size * sizeof(*efs->cache));
        if (!new_cache) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Unable to allocate file cache");
            return -1; /* If it fails here, no harm is done to the filesystem, so it's safe */
        }
        /* Zero out the new portions of the cache */
        memset(&new_cache[efs->cache_size], 0, (new_size - efs->cache_size) * sizeof(*efs->cache));
        efs->cache = new_cache;
        efs->cache_size = new_size;
    }


    /* Allocate file descriptor here now */
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
    *file = esp_littlefs_calloc(1, sizeof(**file) + path_len);
#else
    *file = esp_littlefs_calloc(1, sizeof(**file));
#endif

    if (*file == NULL) {
        /* If it fails here, the file system might have a larger cache, but it's harmless, no need to reverse it */
        ESP_LOGE(ESP_LITTLEFS_TAG, "Unable to allocate FD");
        return -1;
    }

    /* Starting from here, nothing can fail anymore */

#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
    /* The trick here is to avoid dual allocation so the path pointer
        should point to the next byte after it:
        file => [ lfs_file | # | next | path | free_space ]
                                            |  /\
                                            |__/
    */
    (*file)->path = (char*)(*file) + sizeof(**file);
#endif

    /* initialize lfs_file_config */
    (*file)->lfs_file_config.buffer = (*file)->lfs_buffer;
#if ESP_LITTLEFS_ATTR_COUNT
    (*file)->lfs_file_config.attrs = (*file)->lfs_attr;
    (*file)->lfs_attr[0].type = ESP_LITTLEFS_ATTR_MTIME;
    (*file)->lfs_attr[0].buffer = &(*file)->lfs_attr_time_buffer;
    (*file)->lfs_attr[0].size = sizeof((*file)->lfs_attr_time_buffer);
#endif
    (*file)->lfs_file_config.attr_count = ESP_LITTLEFS_ATTR_COUNT;

    /* Now find a free place in cache */
    for(i=0; i < efs->cache_size; i++) {
        if (efs->cache[i] == NULL) {
            efs->cache[i] = *file;
            break;
        }
    }
    /* Save file in the list */
    (*file)->next = efs->file;
    efs->file = *file;
    efs->fd_count++;
    return i;
}

/**
 * @brief Release a file descriptor
 * @param[in,out] efs file system context
 * @param[in] fd File Descriptor to release
 * @return 0 on success. -1 if a FD cannot be obtained.
 * @warning This must be called with lock taken
 */
static int esp_littlefs_free_fd(esp_littlefs_t *efs, int fd){
    vfs_littlefs_file_t * file, * head;

    if((uint32_t)fd >= efs->cache_size) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        return -1;
    }

    /* Get the file descriptor to free it */
    file = efs->cache[fd];
    head = efs->file;
    /* Search for file in SLL to remove it */
    if (file == head) {
        /* Last file, can't fail */
        efs->file = efs->file->next;
    } else {
        while (head && head->next != file) {
            head = head->next;
        }
        if (!head) {
            ESP_LOGE(ESP_LITTLEFS_TAG, "Inconsistent list");
            return -1;
        }
        /* Transaction starts here and can't fail anymore */
        head->next = file->next;
    }
    efs->cache[fd] = NULL;
    efs->fd_count--;

    ESP_LOGV(ESP_LITTLEFS_TAG, "Clearing FD");
    free(file);

#if 0
    /* Realloc smaller if its possible
     *     * Find and realloc based on number of trailing NULL ptrs in cache
     *     * Leave some hysteris to prevent thrashing around resize points
     * This is disabled for now because it adds unnecessary complexity
     * and binary size increase that outweights its ebenfits.
     */
    if(efs->cache_size > CONFIG_LITTLEFS_FD_CACHE_MIN_SIZE) {
        uint16_t n_free;
        uint16_t new_size = efs->cache_size / CONFIG_LITTLEFS_FD_CACHE_REALLOC_FACTOR;

        if(new_size >= CONFIG_LITTLEFS_FD_CACHE_MIN_SIZE) {
            /* Count number of trailing NULL ptrs */
            for(n_free=0; n_free < efs->cache_size; n_free++) {
                if(efs->cache[efs->cache_size - n_free - 1] != NULL) {
                    break;
                }
            }

            if(n_free >= (efs->cache_size - new_size)){
                new_size += CONFIG_LITTLEFS_FD_CACHE_HYST;
                ESP_LOGV(ESP_LITTLEFS_TAG, "Reallocating cache %i -> %i", efs->cache_size, new_size);
                vfs_littlefs_file_t ** new_cache;
                new_cache = realloc(efs->cache, new_size * sizeof(*efs->cache));
                /* No harm on realloc failure, continue using the oversized cache */
                if(new_cache) {
                    efs->cache = new_cache;
                    efs->cache_size = new_size;
                }
            }
        }
    }
#endif

    return 0;
}

/**
 * @brief Compute the 32bit DJB2 hash of the given string.
 * @param[in]   path the path to hash
 * @returns the hash for this path
 */
static uint32_t compute_hash(const char * path) {
    uint32_t hash = 5381;
    char c;

    while ((c = *path++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

#ifdef CONFIG_VFS_SUPPORT_DIR
/**
 * @brief finds an open file descriptor by file name.
 * @param[in,out] efs file system context
 * @param[in] path File path to check.
 * @returns integer file descriptor. Returns -1 if not found.
 * @warning This must be called with lock taken
 * @warning if CONFIG_LITTLEFS_USE_ONLY_HASH, there is a slim chance an
 *          erroneous FD may be returned on hash collision.
 */
static int esp_littlefs_get_fd_by_name(esp_littlefs_t *efs, const char *path){
    uint32_t hash = compute_hash(path);

    for(uint16_t i=0, j=0; i < efs->cache_size && j < efs->fd_count; i++){
        if (efs->cache[i]) {
            ++j;

            if (
                efs->cache[i]->hash == hash  // Faster than strcmp
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
                && strcmp(path, efs->cache[i]->path) == 0  // May as well check incase of hash collision. Usually short-circuited.
#endif
            ) {
                ESP_LOGV(ESP_LITTLEFS_TAG, "Found \"%s\" at FD %d.", path, i);
                return i;
            }
        }
    }
    ESP_LOGV(ESP_LITTLEFS_TAG, "Unable to get a find FD for \"%s\"", path);
    return -1;
}
#endif

/*** Filesystem Hooks ***/

static int vfs_littlefs_open(void* ctx, const char * path, int flags, int mode) {
    /* Note: mode is currently unused */
    int fd=-1, lfs_flags, res;
    esp_littlefs_t *efs = (esp_littlefs_t *)ctx;
    vfs_littlefs_file_t *file = NULL;
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
    size_t path_len = strlen(path) + 1;  // include NULL terminator
#endif
#if CONFIG_LITTLEFS_OPEN_DIR
    struct lfs_info info;
#endif

    assert(path);

    ESP_LOGV(ESP_LITTLEFS_TAG, "Opening %s", path);

    /* Convert flags to lfs flags */
    lfs_flags = esp_littlefs_flags_conv(flags);
    if(efs->read_only && lfs_flags != LFS_O_RDONLY) {
        return LFS_ERR_INVAL;
    }

    /* Get a FD */
    sem_take(efs);

#if CONFIG_LITTLEFS_OPEN_DIR
    /* Check if it is a file with same path */
    if (flags & O_DIRECTORY) {
        res = lfs_stat(efs->fs, path, &info);
        if (res == LFS_ERR_OK) {
            if (info.type == LFS_TYPE_REG) {
                sem_give(efs);
                ESP_LOGV(ESP_LITTLEFS_TAG, "Open directory but it is a file");
                errno = ENOTDIR;
                return LFS_ERR_INVAL;
            }
        }
    }
#endif

    fd = esp_littlefs_allocate_fd(efs, &file
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
    , path_len
#endif
    );

    if(fd < 0) {
        errno = lfs_errno_remap(fd);
        sem_give(efs);
        ESP_LOGV(ESP_LITTLEFS_TAG, "Error obtaining FD");
        return LFS_ERR_INVAL;
    }

#if CONFIG_LITTLEFS_SPIFFS_COMPAT
    /* Create all parent directories (if necessary) */
    ESP_LOGV(ESP_LITTLEFS_TAG, "LITTLEFS_SPIFFS_COMPAT attempting to create all directories for %s", path);
    mkdirs(efs, path);
#endif  // CONFIG_LITTLEFS_SPIFFS_COMPAT

#ifndef CONFIG_LITTLEFS_MALLOC_STRATEGY_DISABLE
    /* Open File */
    res = lfs_file_opencfg(efs->fs, &file->file, path, lfs_flags, &file->lfs_file_config);
#if CONFIG_LITTLEFS_MTIME_USE_NONCE
    if(!(lfs_flags & LFS_O_RDONLY)){
        // When the READ flag is set, LittleFS will automatically populate attributes.
        // If it's not set, it will not populate attributes.
        // We want the attributes regardless so that we can properly update it.
        file->lfs_attr_time_buffer = esp_littlefs_get_mtime_attr(efs, path);
    }
#endif

#else
    #error "The use of static buffers is not currently supported by this VFS wrapper"
#endif

#if CONFIG_LITTLEFS_OPEN_DIR
    if ( flags & O_DIRECTORY && res ==  LFS_ERR_ISDIR) {
        res = LFS_ERR_OK;
        file->file.flags = flags;
    }
#endif

    if( res < 0 ) {
        errno = lfs_errno_remap(res);
        esp_littlefs_free_fd(efs, fd);
        sem_give(efs);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to open file %s. Error %s (%d)",
                path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to open file. Error %s (%d)",
                esp_littlefs_errno(res), res);
#endif
        return LFS_ERR_INVAL;
    }

    /* Sync after opening. If we are overwriting a file, this will free that
     * file's blocks in storage, prevent OOS errors.
     * See TEST_CASE:
     *     "Rewriting file frees space immediately (#7426)"
     */
#if CONFIG_LITTLEFS_OPEN_DIR
    if ( (flags & O_DIRECTORY) == 0 ) {
#endif
    if(!efs->read_only && lfs_flags != LFS_O_RDONLY)
    {
        res = esp_littlefs_file_sync(efs, file);
    }
    if(res < 0){
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to sync at opening file \"%s\". Error %s (%d)",
                file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to sync at opening file %d. Error %d", fd, res);
#endif
    }

#if CONFIG_LITTLEFS_OPEN_DIR
    }
#endif

    file->hash = compute_hash(path);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
    memcpy(file->path, path, path_len);
#endif

    sem_give(efs);
    ESP_LOGV(ESP_LITTLEFS_TAG, "Done opening %s", path);
    return fd;
}

static ssize_t vfs_littlefs_write(void* ctx, int fd, const void * data, size_t size) {
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    ssize_t res;
    vfs_littlefs_file_t *file = NULL;

    sem_take(efs);
    if((uint32_t)fd > efs->cache_size) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];
    res = lfs_file_write(efs->fs, &file->file, data, size);
#ifdef CONFIG_LITTLEFS_FLUSH_FILE_EVERY_WRITE
    if(res > 0) {
        vfs_littlefs_fsync(ctx, fd);
    }
#endif
    sem_give(efs);

    if(res < 0){
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to write FD %d; path \"%s\". Error %s (%d)",
                fd, file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to write FD %d. Error %s (%d)",
                fd, esp_littlefs_errno(res), res);
#endif
        return -1;
    }

    return res;
}

static ssize_t vfs_littlefs_read(void* ctx, int fd, void * dst, size_t size) {
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    ssize_t res;
    vfs_littlefs_file_t *file = NULL;

    sem_take(efs);
    if((uint32_t)fd > efs->cache_size) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];
    res = lfs_file_read(efs->fs, &file->file, dst, size);
    sem_give(efs);

    if(res < 0){
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to read file \"%s\". Error %s (%d)",
                file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to read FD %d. Error %s (%d)",
                fd, esp_littlefs_errno(res), res);
#endif
        return -1;
    }

    return res;
}

static ssize_t vfs_littlefs_pwrite(void *ctx, int fd, const void *src, size_t size, off_t offset)
{
    esp_littlefs_t *efs = (esp_littlefs_t *)ctx;
    ssize_t res, save_res;
    vfs_littlefs_file_t *file = NULL;

    sem_take(efs);
    if ((uint32_t)fd > efs->cache_size)
    {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];

    off_t old_offset = lfs_file_seek(efs->fs, &file->file, 0, SEEK_CUR);
    if (old_offset < (off_t)0)
    {
        res = old_offset;
        goto exit;
    }

    /* Set to wanted position.  */
    res = lfs_file_seek(efs->fs, &file->file, offset, SEEK_SET);
    if (res < (off_t)0)
        goto exit;

    /* Write out the data.  */
    res = lfs_file_write(efs->fs, &file->file, src, size);

    /* Now we have to restore the position.  If this fails we have to
     return this as an error. But if the writing also failed we
     return writing error.  */
    save_res = lfs_file_seek(efs->fs, &file->file, old_offset, SEEK_SET);
    if (res >= (ssize_t)0 && save_res < (off_t)0)
    {
        res = save_res;
    }
    sem_give(efs);

exit:
    if (res < 0)
    {
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to write FD %d; path \"%s\". Error %s (%d)",
                fd, file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to write FD %d. Error %s (%d)",
                fd, esp_littlefs_errno(res), res);
#endif
        return -1;
    }

    return res;
}

static ssize_t vfs_littlefs_pread(void *ctx, int fd, void *dst, size_t size, off_t offset)
{
    esp_littlefs_t *efs = (esp_littlefs_t *)ctx;
    ssize_t res, save_res;
    vfs_littlefs_file_t *file = NULL;

    sem_take(efs);
    if ((uint32_t)fd > efs->cache_size)
    {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];

    off_t old_offset = lfs_file_seek(efs->fs, &file->file, 0, SEEK_CUR);
    if (old_offset < (off_t)0)
    {
        res = old_offset;
        goto exit;
    }

    /* Set to wanted position.  */
    res = lfs_file_seek(efs->fs, &file->file, offset, SEEK_SET);
    if (res < (off_t)0)
        goto exit;

    /* Read the data.  */
    res = lfs_file_read(efs->fs, &file->file, dst, size);

    /* Now we have to restore the position.  If this fails we have to
     return this as an error. But if the reading also failed we
     return reading error.  */
    save_res = lfs_file_seek(efs->fs, &file->file, old_offset, SEEK_SET);
    if (res >= (ssize_t)0 && save_res < (off_t)0)
    {
        res = save_res;
    }
    sem_give(efs);

exit:
    if (res < 0)
    {
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to read file \"%s\". Error %s (%d)",
                 file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to read FD %d. Error %s (%d)",
                 fd, esp_littlefs_errno(res), res);
#endif
        return -1;
    }

    return res;
}

static int vfs_littlefs_close(void* ctx, int fd) {
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    int res;
    vfs_littlefs_file_t *file = NULL;

    sem_take(efs);
    if((uint32_t)fd > efs->cache_size) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }

    file = efs->cache[fd];

#if CONFIG_LITTLEFS_OPEN_DIR
    if ((file->file.flags & O_DIRECTORY) == 0) {
#endif
#if CONFIG_LITTLEFS_USE_MTIME
    file->lfs_attr_time_buffer = esp_littlefs_get_updated_time(efs, file, NULL);
#endif
    res = lfs_file_close(efs->fs, &file->file);
    if(res < 0){
        errno = lfs_errno_remap(res);
        sem_give(efs);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to close file \"%s\". Error %s (%d)",
                file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to close Fd %d. Error %s (%d)",
                fd, esp_littlefs_errno(res), res);
#endif
        return -1;
    }
    // TODO: update directory containing file's mtime.
#if CONFIG_LITTLEFS_OPEN_DIR
    } else {
        res = 0;
    }
#endif

    esp_littlefs_free_fd(efs, fd);
    sem_give(efs);
    return res;
}

static off_t vfs_littlefs_lseek(void* ctx, int fd, off_t offset, int mode) {
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    lfs_soff_t res;
    vfs_littlefs_file_t *file = NULL;
    int whence;

    switch (mode) {
        case SEEK_SET: whence = LFS_SEEK_SET; break;
        case SEEK_CUR: whence = LFS_SEEK_CUR; break;
        case SEEK_END: whence = LFS_SEEK_END; break;
        default:
            ESP_LOGE(ESP_LITTLEFS_TAG, "Invalid mode");
            errno = EINVAL;
            return -1;
    }

    sem_take(efs);
    if((uint32_t)fd > efs->cache_size) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];
    res = lfs_file_seek(efs->fs, &file->file, offset, whence);
    sem_give(efs);

    if(res < 0){
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to seek file \"%s\" to offset %08x. Error %s (%d)",
                file->path, (unsigned int)offset, esp_littlefs_errno(res), (int) res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to seek FD %d to offset %08x. Error (%d)",
                fd, (unsigned int)offset, (int) res);
#endif
        return -1;
    }

    return res;
}

static int vfs_littlefs_fsync(void* ctx, int fd)
{
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    ssize_t res;
    vfs_littlefs_file_t *file = NULL;


    sem_take(efs);
    if((uint32_t)fd > efs->cache_size) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];
    res = esp_littlefs_file_sync(efs, file);
    sem_give(efs);

    if(res < 0){
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to sync file \"%s\". Error %s (%d)",
                file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to sync file %d. Error %d", fd, res);
#endif
        return -1;
    }

    return res;
}

#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
static int vfs_littlefs_fstat(void* ctx, int fd, struct stat * st) {
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    struct lfs_info info;
    int res;
    vfs_littlefs_file_t *file = NULL;

    memset(st, 0, sizeof(struct stat));
    st->st_blksize = efs->cfg.block_size;

    sem_take(efs);
    if((uint32_t)fd > efs->cache_size) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD must be <%d.", efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];
    res = lfs_stat(efs->fs, file->path, &info);
    if (res < 0) {
        errno = lfs_errno_remap(res);
        sem_give(efs);
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to stat file \"%s\". Error %s (%d)",
                file->path, esp_littlefs_errno(res), res);
        return -1;
    }

#if CONFIG_LITTLEFS_USE_MTIME
    st->st_mtime = file->lfs_attr_time_buffer;
#endif

    sem_give(efs);
    if(info.type==LFS_TYPE_REG){
        // Regular File
        st->st_mode = S_IFREG;
        st->st_size = info.size;
    }
    else{
        // Directory
        st->st_mode = S_IFDIR;
        st->st_size = 0;  // info.size is only valid for REG files
    }
    return 0;
}
#endif

#ifdef CONFIG_VFS_SUPPORT_DIR
static int vfs_littlefs_stat(void* ctx, const char * path, struct stat * st) {
    assert(path);
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    struct lfs_info info;
    int res;

    memset(st, 0, sizeof(struct stat));
    st->st_blksize = efs->cfg.block_size;

    sem_take(efs);
    res = lfs_stat(efs->fs, path, &info);
    if (res < 0) {
        errno = lfs_errno_remap(res);
        sem_give(efs);
        /* Not strictly an error, since stat can be used to check
         * if a file exists */
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to stat path \"%s\". Error %s (%d)",
                path, esp_littlefs_errno(res), res);
        return -1;
    }
#if CONFIG_LITTLEFS_USE_MTIME
    st->st_mtime = esp_littlefs_get_mtime_attr(efs, path);
#endif
    sem_give(efs);
    if(info.type==LFS_TYPE_REG){
        // Regular File
        st->st_mode = S_IFREG;
        st->st_size = info.size;
    }
    else{
        // Directory
        st->st_mode = S_IFDIR;
        st->st_size = 0;  // info.size is only valid for REG files
    }
    return 0;
}

static int vfs_littlefs_unlink(void* ctx, const char *path) {
#define fail_str_1 "Failed to unlink path \"%s\"."
    assert(path);
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    struct lfs_info info;
    int res;

    sem_take(efs);
    res = lfs_stat(efs->fs, path, &info);
    if (res < 0) {
        errno = lfs_errno_remap(res);
        sem_give(efs);
        ESP_LOGV(ESP_LITTLEFS_TAG, fail_str_1 " Error %s (%d)",
                path, esp_littlefs_errno(res), res);
        return -1;
    }

    if(esp_littlefs_get_fd_by_name(efs, path) >= 0) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, fail_str_1 " Has open FD.", path);
        errno = EBUSY;
        return -1;
    }

    if (info.type == LFS_TYPE_DIR) {
        sem_give(efs);
        ESP_LOGV(ESP_LITTLEFS_TAG, "Cannot unlink a directory.");
        errno = EISDIR;
        return -1;
    }

    res = lfs_remove(efs->fs, path);
    if (res < 0) {
        errno = lfs_errno_remap(res);
        sem_give(efs);
        ESP_LOGV(ESP_LITTLEFS_TAG, fail_str_1 " Error %s (%d)",
                path, esp_littlefs_errno(res), res);
        return -1;
    }

#if CONFIG_LITTLEFS_SPIFFS_COMPAT
    /* Attempt to delete all parent directories that are empty */
    rmdirs(efs, path);
#endif  // CONFIG_LITTLEFS_SPIFFS_COMPAT

    sem_give(efs);

    return 0;
#undef fail_str_1
}

static int vfs_littlefs_rename(void* ctx, const char *src, const char *dst) {
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    int res;

    sem_take(efs);

    if(esp_littlefs_get_fd_by_name(efs, src) >= 0){
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "Cannot rename; src \"%s\" is open.", src);
        errno = EBUSY;
        return -1;
    }
    else if(esp_littlefs_get_fd_by_name(efs, dst) >= 0){
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "Cannot rename; dst \"%s\" is open.", dst);
        errno = EBUSY;
        return -1;
    }

#if CONFIG_LITTLEFS_SPIFFS_COMPAT
    /* Create all parent directories to dst (if necessary) */
    ESP_LOGV(ESP_LITTLEFS_TAG, "LITTLEFS_SPIFFS_COMPAT attempting to create all directories for %s", src);
    mkdirs(efs, dst);
#endif

    res = lfs_rename(efs->fs, src, dst);
    if (res < 0) {
        errno = lfs_errno_remap(res);
        sem_give(efs);
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to rename \"%s\" -> \"%s\". Error %s (%d)",
                src, dst, esp_littlefs_errno(res), res);
        return -1;
    }

#if CONFIG_LITTLEFS_SPIFFS_COMPAT
    /* Attempt to delete all parent directories from src that are empty */
    rmdirs(efs, src);
#endif  // CONFIG_LITTLEFS_SPIFFS_COMPAT

    sem_give(efs);

    return 0;
}

static DIR* vfs_littlefs_opendir(void* ctx, const char* name) {
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    int res;
    vfs_littlefs_dir_t *dir = NULL;

    dir = esp_littlefs_calloc(1, sizeof(vfs_littlefs_dir_t));
    if( dir == NULL ) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "dir struct could not be malloced");
        errno = ENOMEM;
        goto exit;
    }

    dir->path = strdup(name);
    if(dir->path == NULL){
        errno = ENOMEM;
        ESP_LOGE(ESP_LITTLEFS_TAG, "dir path name could not be malloced");
        goto exit;
    }

    sem_take(efs);
    res = lfs_dir_open(efs->fs, &dir->d, dir->path);
    sem_give(efs);
    if (res < 0) {
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to opendir \"%s\". Error %s (%d)",
                dir->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to opendir \"%s\". Error %d", dir->path, res);
#endif
        goto exit;
    }

    return (DIR *)dir;

exit:
    esp_littlefs_dir_free(dir);
    return NULL;
}

static int vfs_littlefs_closedir(void* ctx, DIR* pdir) {
    assert(pdir);
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    vfs_littlefs_dir_t * dir = (vfs_littlefs_dir_t *) pdir;
    int res;

    sem_take(efs);
    res = lfs_dir_close(efs->fs, &dir->d);
    sem_give(efs);
    if (res < 0) {
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to closedir \"%s\". Error %s (%d)",
                dir->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to closedir \"%s\". Error %d", dir->path, res);
#endif
        return res;
    }

    esp_littlefs_dir_free(dir);
    return 0;
}

static struct dirent* vfs_littlefs_readdir(void* ctx, DIR* pdir) {
    assert(pdir);
    vfs_littlefs_dir_t * dir = (vfs_littlefs_dir_t *) pdir;
    int res;
    struct dirent* out_dirent;

    res = vfs_littlefs_readdir_r(ctx, pdir, &dir->e, &out_dirent);
    if (res != 0) return NULL;
    return out_dirent;
}

static int vfs_littlefs_readdir_r(void* ctx, DIR* pdir,
        struct dirent* entry, struct dirent** out_dirent) {
    assert(pdir);
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    vfs_littlefs_dir_t * dir = (vfs_littlefs_dir_t *) pdir;
    int res;
    struct lfs_info info = { 0 };

    sem_take(efs);
    do{ /* Read until we get a real object name */
        res = lfs_dir_read(efs->fs, &dir->d, &info);
    }while( res>0 && (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0));
    sem_give(efs);
    if (res < 0) {
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to readdir \"%s\". Error %s (%d)",
                dir->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to readdir \"%s\". Error %d", dir->path, res);
#endif
        return -1;
    }

    if(info.type == LFS_TYPE_REG) {
        ESP_LOGV(ESP_LITTLEFS_TAG, "readdir a file of size %u named \"%s\"",
                (unsigned int) info.size, info.name);
    }
    else {
        ESP_LOGV(ESP_LITTLEFS_TAG, "readdir a dir named \"%s\"", info.name);
    }

    if(res == 0) {
        /* End of Objs */
        ESP_LOGV(ESP_LITTLEFS_TAG, "Reached the end of the directory.");
        *out_dirent = NULL;
    }
    else {
        entry->d_ino = 0;
        entry->d_type = info.type == LFS_TYPE_REG ? DT_REG : DT_DIR;
        strncpy(entry->d_name, info.name, sizeof(entry->d_name));
        *out_dirent = entry;
    }
    dir->offset++;

    return 0;
}

static long vfs_littlefs_telldir(void* ctx, DIR* pdir) {
    assert(pdir);
    vfs_littlefs_dir_t * dir = (vfs_littlefs_dir_t *) pdir;
    return dir->offset;
}

static void vfs_littlefs_seekdir(void* ctx, DIR* pdir, long offset) {
    assert(pdir);
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    vfs_littlefs_dir_t * dir = (vfs_littlefs_dir_t *) pdir;
    int res;

    if (offset < dir->offset) {
        /* close and re-open dir to rewind to beginning */
        sem_take(efs);
        res = lfs_dir_rewind(efs->fs, &dir->d);
        sem_give(efs);
        if (res < 0) {
            errno = lfs_errno_remap(res);
            ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to rewind dir \"%s\". Error %s (%d)",
                    dir->path, esp_littlefs_errno(res), res);
            return;
        }
        dir->offset = 0;
    }

    while(dir->offset < offset){
        struct dirent *out_dirent;
        res = vfs_littlefs_readdir_r(ctx, pdir, &dir->e, &out_dirent);
        if( res != 0 ){
            ESP_LOGE(ESP_LITTLEFS_TAG, "Error readdir_r");
            return;
        }
    }
}

static int vfs_littlefs_mkdir(void* ctx, const char* name, mode_t mode) {
    /* Note: mode is currently unused */
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    int res;
    ESP_LOGV(ESP_LITTLEFS_TAG, "mkdir \"%s\"", name);

    sem_take(efs);
    res = lfs_mkdir(efs->fs, name);
    sem_give(efs);
    if (res < 0) {
        errno = lfs_errno_remap(res);
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to mkdir \"%s\". Error %s (%d)",
                name, esp_littlefs_errno(res), res);
        return -1;
    }
    return 0;
}

static int vfs_littlefs_rmdir(void* ctx, const char* name) {
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    struct lfs_info info;
    int res;

    /* Error Checking */
    sem_take(efs);
    res = lfs_stat(efs->fs, name, &info);
    if (res < 0) {
        errno = lfs_errno_remap(res);
        sem_give(efs);
        ESP_LOGV(ESP_LITTLEFS_TAG, "\"%s\" doesn't exist.", name);
        return -1;
    }

    if (info.type != LFS_TYPE_DIR) {
        sem_give(efs);
        ESP_LOGV(ESP_LITTLEFS_TAG, "\"%s\" is not a directory.", name);
        errno = ENOTDIR;
        return -1;
    }

    /* Unlink the dir */
    res = lfs_remove(efs->fs, name);
    sem_give(efs);
    if ( res < 0) {
        errno = lfs_errno_remap(res);
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to unlink path \"%s\". Error %s (%d)",
                name, esp_littlefs_errno(res), res);
        return -1;
    }

    return 0;
}

static ssize_t vfs_littlefs_truncate( void *ctx, const char *path, off_t size )
{
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    ssize_t res = -1;
    vfs_littlefs_file_t *file = NULL;

    int fd = vfs_littlefs_open( ctx, path, LFS_O_RDWR, 438 );

    sem_take(efs);
    if((uint32_t)fd > efs->cache_size)
    {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];
    res = lfs_file_truncate( efs->fs, &file->file, size );
    sem_give(efs);

    if(res < 0)
    {
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to truncate file \"%s\". Error %s (%d)",
                file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to truncate FD %d. Error %s (%d)",
                fd, esp_littlefs_errno(res), res);
#endif
        res = -1;
    }
    else
    {
        ESP_LOGV( ESP_LITTLEFS_TAG, "Truncated file %s to %u bytes", path, (unsigned int) size );
    }
    vfs_littlefs_close( ctx, fd );
    return res;
}

#ifdef ESP_LITTLEFS_ENABLE_FTRUNCATE
static int vfs_littlefs_ftruncate(void *ctx, int fd, off_t size)
{
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    ssize_t res;
    vfs_littlefs_file_t *file = NULL;

    sem_take(efs);
    if((uint32_t)fd > efs->cache_size) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }
    file = efs->cache[fd];
    res = lfs_file_truncate( efs->fs, &file->file, size );
    sem_give(efs);

    if(res < 0)
    {
        errno = lfs_errno_remap(res);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to truncate file \"%s\". Error %s (%d)",
                file->path, esp_littlefs_errno(res), res);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to truncate FD %d. Error %s (%d)",
                fd, esp_littlefs_errno(res), res);
#endif
        res = -1;
    }
    else
    {
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV( ESP_LITTLEFS_TAG, "Truncated file %s to %u bytes", file->path, (unsigned int) size );
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Truncated FD %d to %u bytes", fd, (unsigned int) size );
#endif
    }
    return res;
}
#endif // ESP_LITTLEFS_ENABLE_FTRUNCATE
#endif //CONFIG_VFS_SUPPORT_DIR

/**
 * Syncs file while also updating mtime (if necessary)
 */
static int esp_littlefs_file_sync(esp_littlefs_t *efs, vfs_littlefs_file_t *file)
{
    int res;
#if CONFIG_LITTLEFS_USE_MTIME
    if((file->file.flags & 0x3) != LFS_O_RDONLY){
        file->lfs_attr_time_buffer = esp_littlefs_get_updated_time(efs, file, NULL);
    }
#endif
    res = lfs_file_sync(efs->fs, &file->file);
    return res;
}

#if CONFIG_LITTLEFS_USE_MTIME
/**
 * Sets the mtime attr to t.
 */
static int esp_littlefs_update_mtime_attr(esp_littlefs_t *efs, const char *path, time_t t)
{
    int res;
    res = lfs_setattr(efs->fs, path, ESP_LITTLEFS_ATTR_MTIME,
            &t, sizeof(t));
    if( res < 0 ) {
        errno = lfs_errno_remap(res);
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to update mtime (%d)", res);
        return -1;
    }

    return res;
}

/**
 * @brief Only to be used when calcualting what time we should write to disk.
 * @param file If non-null, use this file's attribute to get previous file's time (if use nonce).
 * @param path If non-null, use this path to read in the previous file's time (if use nonce).
 */
static time_t esp_littlefs_get_updated_time(esp_littlefs_t *efs, vfs_littlefs_file_t *file, const char *path)
{
    time_t t;
#if CONFIG_LITTLEFS_MTIME_USE_SECONDS
    // use current time
    t = time(NULL);
#elif CONFIG_LITTLEFS_MTIME_USE_NONCE
    assert( sizeof(time_t) == 8 );
    if(path){
        t = esp_littlefs_get_mtime_attr(efs, path);
    }
    else if(file){
        t = file->lfs_attr_time_buffer;
    }
    else{
        // Invalid input arguments.
        assert(0);
    }
    if( 0 == t ) t = esp_random();
    else t += 1;

    if( 0 == t ) t = 1;
#else
#error "Invalid MTIME configuration"
#endif
    return t;
}

static int vfs_littlefs_utime(void *ctx, const char *path, const struct utimbuf *times)
{
    esp_littlefs_t * efs = (esp_littlefs_t *)ctx;
    time_t t;

    assert(path);

    sem_take(efs);
    if (times) {
        t = times->modtime;
    } else {
        t = esp_littlefs_get_updated_time(efs, NULL, path);
    }

    int ret = esp_littlefs_update_mtime_attr(efs, path, t);
    sem_give(efs);
    return ret;
}

static time_t esp_littlefs_get_mtime_attr(esp_littlefs_t *efs, const char *path)
{
    time_t t;
    int size;
    size = lfs_getattr(efs->fs, path, ESP_LITTLEFS_ATTR_MTIME,
            &t, sizeof(t));
    if( size < 0 ) {
        errno = lfs_errno_remap(size);
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to get mtime attribute %s (%d)",
                esp_littlefs_errno(size), size);
#else
        ESP_LOGV(ESP_LITTLEFS_TAG, "Failed to get mtime attribute %d", size);
#endif
        return -1;
    }
    return t;
}
#endif //CONFIG_LITTLEFS_USE_MTIME

#if CONFIG_LITTLEFS_SPIFFS_COMPAT
/**
 * @brief Recursively make all parent directories for a file.
 * @param[in] dir Path of directories to make up to. The last element
 * of the path is assumed to be the file and IS NOT created.
 *   e.g.
 *       "foo/bar/baz"
 *   will create directories "foo" and "bar"
 */
static void mkdirs(esp_littlefs_t * efs, const char *dir) {
    char tmp[CONFIG_LITTLEFS_OBJ_NAME_LEN];
    char *p = NULL;

    strlcpy(tmp, dir, sizeof(tmp));
    for(p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = '\0';
            vfs_littlefs_mkdir((void*)efs, tmp, S_IRWXU);
            *p = '/';
        }
    }
}

/**
 * @brief Recursively attempt to delete all empty directories for a file.
 * @param[in] dir Path of directories to delete. The last element of the path
 * is assumed to be the file and IS NOT deleted.
 *   e.g.
 *       "foo/bar/baz"
 *   will attempt to delete directories (in order):
 *       1. "foo/bar/baz"
 *       2. "foo/bar"
 *       3. "foo"
 */

static void rmdirs(esp_littlefs_t * efs, const char *dir) {
    char tmp[CONFIG_LITTLEFS_OBJ_NAME_LEN];
    char *p = NULL;

    strlcpy(tmp, dir, sizeof(tmp));
    for(p = tmp + strlen(tmp) - 1; p != tmp; p--) {
        if(*p == '/') {
            *p = '\0';
            vfs_littlefs_rmdir((void*)efs, tmp);
            *p = '/';
        }
    }
}

#endif  // CONFIG_LITTLEFS_SPIFFS_COMPAT

static int vfs_littlefs_fcntl(void* ctx, int fd, int cmd, int arg)
{
    int result = 0;
    esp_littlefs_t *efs = (esp_littlefs_t *)ctx;
    lfs_file_t *lfs_file = NULL;
    vfs_littlefs_file_t *file = NULL;
    const uint32_t flags_mask = LFS_O_WRONLY | LFS_O_RDONLY | LFS_O_RDWR;

    sem_take(efs);
    if((uint32_t)fd > efs->cache_size) {
        sem_give(efs);
        ESP_LOGE(ESP_LITTLEFS_TAG, "FD %d must be <%d.", fd, efs->cache_size);
        errno = EBADF;
        return -1;
    }

    file = efs->cache[fd];
    if (file) {
        lfs_file = &efs->cache[fd]->file;
    } else {
        sem_give(efs);
        errno = EBADF;
        return -1;
    }

    if (cmd == F_GETFL) {
        if ((lfs_file->flags & flags_mask) == LFS_O_WRONLY) {
            result = O_WRONLY;
        } else if ((lfs_file->flags & flags_mask) == LFS_O_RDONLY) {
            result = O_RDONLY;
        } else if ((lfs_file->flags & flags_mask) == LFS_O_RDWR) {
            result = O_RDWR;
        }
    }
#ifdef CONFIG_LITTLEFS_FCNTL_GET_PATH
    else if (cmd == F_GETPATH) {
        char *buffer = (char *)(uintptr_t)arg;

        assert(buffer);

        if (snprintf(buffer, MAXPATHLEN, "%s%s", efs->base_path, file->path) > 0) {
            result = 0;
        } else {
            result = -1;
            errno = EINVAL;
        }
    }
#endif
    else {
        result = -1;
        errno = ENOSYS;
    }

    sem_give(efs);

    return result;
}
