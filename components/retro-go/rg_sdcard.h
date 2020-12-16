#pragma once

#include <stdbool.h>
#include <stdint.h>

// All <int> functions return 0 on success, negative value on error.
// All <char*> functions return pointer on success, NULL on failure. Freeing required.
// All <const char*> functions return pointer on success, NULL on failure.

int rg_sdcard_mount();
int rg_sdcard_unmount();
int rg_sdcard_mkdir(const char *path);
int rg_sdcard_delete(const char* path);
int rg_sdcard_read_dir(const char* path, char **out_files, size_t *out_count);
int rg_sdcard_read_file(const char* path, void* buf, size_t buf_size);
int rg_sdcard_unzip_file(const char* path, void* buf, size_t buf_size);
int rg_sdcard_write_file(const char* path, void* buf, size_t buf_size);
int rg_sdcard_get_filesize(const char* path);

/* Utilities */
const char* path_get_filename(const char* path);
const char* path_get_extension(const char* path);
