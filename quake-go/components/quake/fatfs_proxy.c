#include "fatfs_proxy.h"
#include <errno.h>
#include "assert.h"
#include "esp_attr.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define FATFS_PROXY_TASK_NAME       ("fatfs_task")
#define FATFS_PROXY_TASK_STACK_SIZE (4096)
#define FATFS_PROXY_TASK_PRIORITY   (5) // Owner blocks until each request completes
#define FATFS_PROXY_TASK_CORE       (0) // Internal stack remains usable during FATFS cache operations

typedef enum 
{
    FATFS_PROXY_OPERATION_INVALID = 0,
    FATFS_PROXY_OPERATION_FOPEN,
    FATFS_PROXY_OPERATION_FCLOSE,
    FATFS_PROXY_OPERATION_FSEEK,
    FATFS_PROXY_OPERATION_FTELL,
    FATFS_PROXY_OPERATION_FREAD,
    FATFS_PROXY_OPERATION_FWRITE,
    FATFS_PROXY_OPERATION_VFPRINTF,
    FATFS_PROXY_OPERATION_VFSCANF,
    FATFS_PROXY_OPERATION_FGETC,
    FATFS_PROXY_OPERATION_FFLUSH,
    FATFS_PROXY_OPERATION_FEOF,
    FATFS_PROXY_OPERATION_MKDIR,
    FATFS_PROXY_OPERATION_QUIT
} fatfs_proxy_operation_t;

static DRAM_ATTR TaskHandle_t owner_task_handle = NULL;
static DRAM_ATTR TaskHandle_t fatfs_task_handle = NULL;

static DRAM_ATTR volatile fatfs_proxy_operation_t proxy_operation;

static DRAM_ATTR int proxy_return_value;
static DRAM_ATTR int proxy_errno;
static DRAM_ATTR const char *proxy_path;
static DRAM_ATTR const char *proxy_mode;
static DRAM_ATTR FILE *proxy_file;
static DRAM_ATTR void *proxy_buf;
static DRAM_ATTR mode_t proxy_mkdir_mode;
static DRAM_ATTR va_list* proxy_args;

static DRAM_ATTR long proxy_long;
static DRAM_ATTR int proxy_int;
static DRAM_ATTR size_t proxy_size;

static IRAM_ATTR void fatfs_task_entry(void *arg)
{
    (void)arg; // Unused
    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        switch (proxy_operation)
        {
            case FATFS_PROXY_OPERATION_FOPEN:
                proxy_file = fopen(proxy_path, proxy_mode);
                break;
            case FATFS_PROXY_OPERATION_FCLOSE:
                proxy_return_value = fclose(proxy_file);
                break;
            case FATFS_PROXY_OPERATION_FSEEK:
                proxy_return_value = fseek(proxy_file, proxy_long, proxy_int);
                break;
            case FATFS_PROXY_OPERATION_FTELL:
                proxy_long = ftell(proxy_file);
                break;
            case FATFS_PROXY_OPERATION_FREAD:
                proxy_size = fread(proxy_buf, proxy_size, proxy_long, proxy_file);
                break;
            case FATFS_PROXY_OPERATION_FWRITE:
                proxy_size = fwrite(proxy_buf, proxy_size, proxy_long, proxy_file);
                break;
            case FATFS_PROXY_OPERATION_VFPRINTF:
                proxy_return_value = vfprintf(proxy_file, proxy_path, *proxy_args);
                break;
            case FATFS_PROXY_OPERATION_VFSCANF:
                proxy_return_value = vfscanf(proxy_file, proxy_path, *proxy_args);
                break;
            case FATFS_PROXY_OPERATION_FGETC:
                proxy_return_value = fgetc(proxy_file);
                break;
            case FATFS_PROXY_OPERATION_FFLUSH:
                proxy_return_value = fflush(proxy_file);
                break;
            case FATFS_PROXY_OPERATION_FEOF:
                proxy_return_value = feof(proxy_file);
                break;
            case FATFS_PROXY_OPERATION_MKDIR:
                proxy_return_value = mkdir(proxy_path, proxy_mkdir_mode);
                break;
            case FATFS_PROXY_OPERATION_QUIT:
                fatfs_task_handle = NULL;
                if (owner_task_handle)
                    xTaskNotifyGive(owner_task_handle);
                vTaskDelete(NULL);
                return;
            default:
                break; 
        }

        proxy_errno = errno;
        proxy_operation = FATFS_PROXY_OPERATION_INVALID;

        if (owner_task_handle)
            xTaskNotifyGive(owner_task_handle);
    }
}

static void fatfs_do()
{
    assert(xTaskGetCurrentTaskHandle() == owner_task_handle);

    if (fatfs_task_handle) {
        xTaskNotifyGive(fatfs_task_handle);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    } else {
        esp_system_abort("fatfs_task_handle is NULL");
    }
}

int fatfs_proxy_get_errno(void)
{
    return proxy_errno;
}

void fatfs_proxy_init(TaskHandle_t ownerTask)
{
    if (fatfs_task_handle != NULL) return;

    if (xTaskCreatePinnedToCore(fatfs_task_entry, 
                                FATFS_PROXY_TASK_NAME, 
                                FATFS_PROXY_TASK_STACK_SIZE, 
                                NULL, 
                                FATFS_PROXY_TASK_PRIORITY, 
                                &fatfs_task_handle, 
                                FATFS_PROXY_TASK_CORE) != pdPASS)
        esp_system_abort("unable to create fatfs task");

    owner_task_handle = ownerTask;
}

void fatfs_proxy_deinit(void)
{
    if (fatfs_task_handle == NULL) return;

    proxy_operation = FATFS_PROXY_OPERATION_QUIT;
    fatfs_do();
}

FILE* fatfs_proxy_fopen(const char * restrict name, const char * restrict type)
{
    proxy_path = name;
    proxy_mode = type;
    proxy_operation = FATFS_PROXY_OPERATION_FOPEN;

    fatfs_do();

    return proxy_file;
}

int	fatfs_proxy_fclose(FILE *file)
{
    proxy_file = file;
    proxy_operation = FATFS_PROXY_OPERATION_FCLOSE;

    fatfs_do();

    return proxy_return_value;
}

int	fatfs_proxy_fseek(FILE *file, long pos, int type)
{
    proxy_file = file;
    proxy_long = pos;
    proxy_int = type;
    proxy_operation = FATFS_PROXY_OPERATION_FSEEK;

    fatfs_do();

    return proxy_return_value;
}

long fatfs_proxy_ftell(FILE *file)
{
    proxy_file = file;
    proxy_operation = FATFS_PROXY_OPERATION_FTELL;

    fatfs_do();

    return proxy_long;
}

size_t fatfs_proxy_fread(void * restrict buf, size_t size, size_t n, FILE * restrict file)
{
    proxy_buf = buf;
    proxy_size = size;
    proxy_long = n;
    proxy_file = file;
    proxy_operation = FATFS_PROXY_OPERATION_FREAD;

    fatfs_do();

    return proxy_size;
}

size_t fatfs_proxy_fwrite(const void * restrict buf, size_t size, size_t n, FILE *file)
{
    proxy_buf = (void*)buf;
    proxy_size = size;
    proxy_long = n;
    proxy_file = file;
    proxy_operation = FATFS_PROXY_OPERATION_FWRITE;

    fatfs_do();

    return proxy_size;
}

int	fatfs_proxy_vfprintf(FILE* restrict file, const char* restrict fmt, va_list args)
{
    proxy_file = file;
    proxy_path = fmt;
    proxy_args = &args;
    proxy_operation = FATFS_PROXY_OPERATION_VFPRINTF;

    fatfs_do();

    return proxy_return_value;
}

int	fatfs_proxy_vfscanf(FILE* restrict file, const char* restrict fmt, va_list args)
{
    proxy_file = file;
    proxy_path = fmt;
    proxy_args = &args;
    proxy_operation = FATFS_PROXY_OPERATION_VFSCANF;

    fatfs_do();
    
    return proxy_return_value;
}

int	fatfs_proxy_fgetc(FILE *file)
{
    proxy_file = file;
    proxy_operation = FATFS_PROXY_OPERATION_FGETC;

    fatfs_do();

    return proxy_return_value;
}

int	fatfs_proxy_fflush(FILE *file)
{
    proxy_file = file;
    proxy_operation = FATFS_PROXY_OPERATION_FFLUSH;

    fatfs_do();

    return proxy_return_value;
}

int	fatfs_proxy_feof(FILE *file)
{
    proxy_file = file;
    proxy_operation = FATFS_PROXY_OPERATION_FEOF;

    fatfs_do();

    return proxy_return_value;
}

int	fatfs_proxy_mkdir(const char *path, mode_t mode)
{
    proxy_path = path;
    proxy_mkdir_mode = mode;
    proxy_operation = FATFS_PROXY_OPERATION_MKDIR;

    fatfs_do();

    return proxy_return_value;
}
