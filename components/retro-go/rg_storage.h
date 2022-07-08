#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define RG_ROOT_PATH            "/sd"
#define RG_BASE_PATH            RG_ROOT_PATH "/retro-go"
#define RG_BASE_PATH_CACHE      RG_BASE_PATH "/cache"
#define RG_BASE_PATH_CONFIG     RG_BASE_PATH "/config"
#define RG_BASE_PATH_COVERS     RG_ROOT_PATH "/romart"
#define RG_BASE_PATH_ROMS       RG_ROOT_PATH "/roms"
#define RG_BASE_PATH_SAVES      RG_BASE_PATH "/saves"
#define RG_BASE_PATH_SYSTEM     RG_BASE_PATH "/system"
#define RG_BASE_PATH_THEMES     RG_BASE_PATH "/themes"

// TO DO: This should be an enum, not strings and even less a function call...
#define NS_GLOBAL  "global"
#define NS_APP     rg_system_get_app()->configNs
#define NS_FILE    rg_system_get_app()->romPath

void rg_storage_init(void);
void rg_storage_deinit(void);
bool rg_storage_format(void);
bool rg_storage_ready(void);
void rg_storage_commit(void);
void rg_storage_set_activity_led(bool enable);
bool rg_storage_get_activity_led(void);

bool rg_mkdir(const char *dir);
bool rg_delete(const char *path);
const char *rg_dirname(const char *path);
const char *rg_basename(const char *path);
const char *rg_extension(const char *path);
