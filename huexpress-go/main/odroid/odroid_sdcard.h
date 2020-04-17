#pragma once

#include "esp_err.h"


esp_err_t odroid_sdcard_open(const char* base_path);
esp_err_t odroid_sdcard_close();
size_t odroid_sdcard_get_filesize(const char* path);
size_t odroid_sdcard_copy_file_to_memory(const char* path, void* ptr);
char* odroid_sdcard_create_savefile_path(const char* base_path, const char* fileName);
