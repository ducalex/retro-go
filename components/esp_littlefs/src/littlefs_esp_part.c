/**
 * @file littlefs_api.c
 * @brief Maps the HAL of esp_partition <-> littlefs
 * @author Brian Pugh
 */

//#define ESP_LOCAL_LOG_LEVEL ESP_LOG_INFO

#include "esp_log.h"
#include "esp_partition.h"
#include "esp_vfs.h"
#include "littlefs/lfs.h"
#include "esp_littlefs.h"
#include "littlefs_api.h"

#ifdef CONFIG_LITTLEFS_WDT_RESET
#include "esp_task_wdt.h"
#endif

#ifdef CONFIG_LITTLEFS_MMAP_PARTITION
int littlefs_esp_part_read_mmap(const struct lfs_config *c, lfs_block_t block,
                           lfs_off_t off, void *buffer, lfs_size_t size) {
    esp_littlefs_t * efs = c->context;
    size_t part_off = (block * c->block_size) + off;
    if (part_off > efs->partition->size || part_off + size > efs->partition->size) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "attempt to read out bounds of mmaped region %08x-%08x", (unsigned int)part_off, (unsigned int)(part_off + size));
        return LFS_ERR_IO;
    }
    memcpy(buffer, efs->mmap_data + part_off, size);
    return 0;
}
#endif

int littlefs_esp_part_read(const struct lfs_config *c, lfs_block_t block,
                           lfs_off_t off, void *buffer, lfs_size_t size) {
    esp_littlefs_t * efs = c->context;
    size_t part_off = (block * c->block_size) + off;
    
#ifdef CONFIG_LITTLEFS_WDT_RESET
    esp_task_wdt_reset();
#endif
    
    esp_err_t err = esp_partition_read(efs->partition, part_off, buffer, size);
    if (err) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "failed to read addr %08x, size %08x, err %d", (unsigned int) part_off, (unsigned int) size, err);
        return LFS_ERR_IO;
    }
    return 0;
}

int littlefs_esp_part_write(const struct lfs_config *c, lfs_block_t block,
                            lfs_off_t off, const void *buffer, lfs_size_t size) {
    esp_littlefs_t * efs = c->context;
    size_t part_off = (block * c->block_size) + off;
    
#ifdef CONFIG_LITTLEFS_WDT_RESET
    esp_task_wdt_reset();
#endif
    
    esp_err_t err = esp_partition_write(efs->partition, part_off, buffer, size);
    if (err) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "failed to write addr %08x, size %08x, err %d", (unsigned int) part_off, (unsigned int) size, err);
        return LFS_ERR_IO;
    }
    return 0;
}

int littlefs_esp_part_erase(const struct lfs_config *c, lfs_block_t block) {
    esp_littlefs_t * efs = c->context;
    size_t part_off = block * c->block_size;
    
#ifdef CONFIG_LITTLEFS_WDT_RESET
    esp_task_wdt_reset();
#endif
    
    esp_err_t err = esp_partition_erase_range(efs->partition, part_off, c->block_size);
    if (err) {
        ESP_LOGE(ESP_LITTLEFS_TAG, "failed to erase addr %08x, size %08x, err %d", (unsigned int) part_off, (unsigned int) c->block_size, err);
        return LFS_ERR_IO;
    }
    return 0;

}

int littlefs_esp_part_sync(const struct lfs_config *c) {
    /* Unnecessary for esp-idf */
    return 0;
}

