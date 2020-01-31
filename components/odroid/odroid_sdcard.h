#pragma once

#include "esp_err.h"


esp_err_t odroid_sdcard_open(const char* base_path);
esp_err_t odroid_sdcard_close();
size_t odroid_sdcard_get_filesize(const char* path);
size_t odroid_sdcard_copy_file_to_memory(const char* path, void* ptr);
int odroid_sdcard_mkdir(char *dir);

char* odroid_sdcard_get_savefile_path(const char* romPath);
char* odroid_sdcard_get_filename(const char* path);
char* odroid_sdcard_get_extension(const char* path);
char* odroid_sdcard_get_filename_without_extension(const char* path);
