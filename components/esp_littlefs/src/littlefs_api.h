#ifndef ESP_LITTLEFS_API_H__
#define ESP_LITTLEFS_API_H__

#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_vfs.h"
#include "esp_partition.h"
#include "littlefs/lfs.h"
#include "sdkconfig.h"

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
#include <sdmmc_cmd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_LITTLEFS_USE_MTIME
    #define ESP_LITTLEFS_ATTR_COUNT 1
#else
    #define ESP_LITTLEFS_ATTR_COUNT 0
#endif

/**
 * @brief a file descriptor
 * That's also a singly linked list used for keeping tracks of all opened file descriptor 
 *
 * Shortcomings/potential issues of 32-bit hash (when CONFIG_LITTLEFS_USE_ONLY_HASH) listed here:
 *     * unlink - If a different file is open that generates a hash collision, it will report an
 *                error that it cannot unlink an open file.
 *     * rename - If a different file is open that generates a hash collision with
 *                src or dst, it will report an error that it cannot rename an open file.
 * Potential consequences:
 *    1. A file cannot be deleted while a collision-geneating file is open.
 *       Worst-case, if the other file is always open during the lifecycle
 *       of your app, it's collision file cannot be deleted, which in the 
 *       worst-case could cause storage-capacity issues.
 *    2. Same as (1), but for renames
 */
typedef struct _vfs_littlefs_file_t {
    lfs_file_t file;

    /* Allocate all other necessary buffers */
    struct lfs_file_config lfs_file_config;
    uint8_t lfs_buffer[CONFIG_LITTLEFS_CACHE_SIZE];
#if ESP_LITTLEFS_ATTR_COUNT
    struct lfs_attr lfs_attr[ESP_LITTLEFS_ATTR_COUNT];
    time_t lfs_attr_time_buffer;
#endif

    uint32_t hash;
    struct _vfs_littlefs_file_t * next;       /*!< Pointer to next file in Singly Linked List */
#ifndef CONFIG_LITTLEFS_USE_ONLY_HASH
    char     * path;
#endif
} vfs_littlefs_file_t;

/**
 * @brief littlefs definition structure
 */
typedef struct {
    lfs_t *fs;                                /*!< Handle to the underlying littlefs */
    SemaphoreHandle_t lock;                   /*!< FS lock */

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT
    sdmmc_card_t *sdcard;                     /*!< The SD card driver handle on which littlefs is located */
#endif

    const esp_partition_t* partition;         /*!< The partition on which littlefs is located */

#ifdef CONFIG_LITTLEFS_MMAP_PARTITION
    const void *mmap_data;                    /*!< Buffer of mmapped partition */
    esp_partition_mmap_handle_t mmap_handle;  /*!< Handle to mmapped partition */
#endif

    char base_path[ESP_VFS_PATH_MAX+1];       /*!< Mount point */

    struct lfs_config cfg;                    /*!< littlefs Mount configuration */

    vfs_littlefs_file_t *file;                /*!< Singly Linked List of files */

    vfs_littlefs_file_t **cache;              /*!< A cache of pointers to the opened files */
    uint16_t             cache_size;          /*!< The cache allocated size (in pointers) */
    uint16_t             fd_count;            /*!< The count of opened file descriptor used to speed up computation */
    bool                 read_only;           /*!< Filesystem is read-only */
} esp_littlefs_t;

#ifdef CONFIG_LITTLEFS_MMAP_PARTITION
/**
 * @brief Read a region in a block, only for use with an mmapped partition.
 *
 * Negative error codes are propogated to the user.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_esp_part_read_mmap(const struct lfs_config *c, lfs_block_t block,
                           lfs_off_t off, void *buffer, lfs_size_t size);
#endif

/**
 * @brief Read a region in a block.
 *
 * Negative error codes are propogated to the user.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_esp_part_read(const struct lfs_config *c, lfs_block_t block,
                           lfs_off_t off, void *buffer, lfs_size_t size);

/**
 * @brief Program a region in a block.
 *
 * The block must have previously been erased. 
 * Negative error codes are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_esp_part_write(const struct lfs_config *c, lfs_block_t block,
                            lfs_off_t off, const void *buffer, lfs_size_t size);

/**
 * @brief Erase a block.
 *
 * A block must be erased before being programmed.
 * The state of an erased block is undefined.
 * Negative error codes are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 * @return errorcode. 0 on success.
 */
int littlefs_esp_part_erase(const struct lfs_config *c, lfs_block_t block);

/**
 * @brief Sync the state of the underlying block device.
 *
 * Negative error codes are propogated to the user.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_esp_part_sync(const struct lfs_config *c);

#ifdef CONFIG_LITTLEFS_SDMMC_SUPPORT

/**
 * @brief Read a region in a block on SD card
 *
 * Negative error codes are propogated to the user.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_sdmmc_read(const struct lfs_config *c, lfs_block_t block,
                           lfs_off_t off, void *buffer, lfs_size_t size);

/**
 * @brief Program a region in a block on SD card.
 *
 * The block must have previously been erased.
 * Negative error codes are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_sdmmc_write(const struct lfs_config *c, lfs_block_t block,
                            lfs_off_t off, const void *buffer, lfs_size_t size);

/**
 * @brief Erase a block on SD card.
 *
 * A block must be erased before being programmed.
 * The state of an erased block is undefined.
 * Negative error codes are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 * @return errorcode. 0 on success.
 */
int littlefs_sdmmc_erase(const struct lfs_config *c, lfs_block_t block);

/**
 * @brief Sync the state of the underlying SD card.
 *
 * Negative error codes are propogated to the user.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_sdmmc_sync(const struct lfs_config *c);

#endif // CONFIG_LITTLEFS_SDMMC_SUPPORT

#ifdef __cplusplus
}
#endif

#endif
