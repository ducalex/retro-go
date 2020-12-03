#pragma once

#include <stdint.h>

// All <int> functions return 0 on success, negative value on error.
// All <char*> functions return pointer on success, NULL on failure. Freeing required.
// All <const char*> functions return pointer on success, NULL on failure.

int odroid_sdcard_open();
int odroid_sdcard_close();
int odroid_sdcard_mkdir(const char *path);
int odroid_sdcard_unlink(const char* path);
int odroid_sdcard_list(const char* path, char **out_files, size_t *out_count);
int odroid_sdcard_read_file(const char* path, void* buf, size_t buf_size);
int odroid_sdcard_unzip_file(const char* path, void* buf, size_t buf_size);
int odroid_sdcard_write_file(const char* path, void* buf, size_t buf_size);
int odroid_sdcard_get_filesize(const char* path);
const char* odroid_sdcard_get_filename(const char* path);
const char* odroid_sdcard_get_extension(const char* path);
