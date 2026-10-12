#pragma once

#include "stdio.h"
#include "sys/stat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void fatfs_proxy_init(TaskHandle_t ownerTask);
void fatfs_proxy_deinit(void);
int fatfs_proxy_get_errno(void);

FILE* fatfs_proxy_fopen(const char * restrict name, const char * restrict type);
int fatfs_proxy_fclose(FILE *file);
int fatfs_proxy_fseek(FILE *file, long pos, int type);
long fatfs_proxy_ftell(FILE *file);
size_t fatfs_proxy_fread(void* restrict buf, size_t size, size_t n, FILE* restrict file);
size_t fatfs_proxy_fwrite(const void* restrict buf, size_t size, size_t n, FILE *file);
int fatfs_proxy_vfprintf(FILE* restrict file, const char* restrict fmt, va_list args);
int fatfs_proxy_vfscanf(FILE* restrict file, const char* restrict fmt, va_list args);
int fatfs_proxy_fgetc(FILE *file);
int fatfs_proxy_fflush(FILE *file);
int fatfs_proxy_feof(FILE *file);

int fatfs_proxy_mkdir(const char *path, mode_t mode);
