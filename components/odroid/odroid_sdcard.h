#pragma once

#include <stddef.h>

bool odroid_sdcard_open();
bool odroid_sdcard_close();
int odroid_sdcard_mkdir(char *dir);
size_t odroid_sdcard_read_file(const char* path, void* buf, size_t buf_size);
size_t odroid_sdcard_unzip_file(const char* path, void* buf, size_t buf_size);
size_t odroid_sdcard_write_file(const char* path, void* buf, size_t buf_size);

size_t odroid_sdcard_get_filesize(const char* path);
const char* odroid_sdcard_get_filename(const char* path);
const char* odroid_sdcard_get_extension(const char* path);