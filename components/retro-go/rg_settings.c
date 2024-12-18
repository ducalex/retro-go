#include "rg_system.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>

static cJSON *config_root = NULL;
static bool safe_mode = false;


static cJSON *json_root(const char *name, bool set_dirty)
{
    if (!config_root)
        return NULL;

    if (name == NS_GLOBAL)
        name = "global";
    else if (name == NS_APP)
        name = rg_system_get_app()->configNs;
    else if (name == NS_FILE)
        name = rg_basename(rg_system_get_app()->romPath);
    else if (name == NS_WIFI)
        name = "wifi";
    else if (name == NS_BOOT)
        name = "boot";

    cJSON *branch = cJSON_GetObjectItem(config_root, name);
    if (!branch)
    {
        branch = cJSON_AddObjectToObject(config_root, name);
        cJSON_AddStringToObject(branch, "namespace", name);
        cJSON_AddNumberToObject(branch, "changed", 0);
        if (!safe_mode)
        {
            char *data; size_t data_len;
            char pathbuf[RG_PATH_MAX];
            snprintf(pathbuf, RG_PATH_MAX, "%s/%s.json", RG_BASE_PATH_CONFIG, name);
            if (rg_storage_read_file(pathbuf, (void **)&data, &data_len, 0))
            {
                cJSON *values = cJSON_Parse(data);
                if (!values) // Parse failure, clean the markup and try again
                    values = cJSON_Parse(rg_json_fixup(data));
                if (values)
                {
                    RG_LOGI("Config file loaded: '%s'", pathbuf);
                    cJSON_AddItemToObject(branch, "values", values);
                }
                else
                    RG_LOGE("Config file parsing failed: '%s'", pathbuf);
                free(data);
            }
        }
    }

    cJSON *root = cJSON_GetObjectItem(branch, "values");
    if (!cJSON_IsObject(root))
    {
        cJSON_DeleteItemFromObject(branch, "values");
        root = cJSON_AddObjectToObject(branch, "values");
    }

    if (set_dirty)
    {
        cJSON_SetNumberHelper(cJSON_GetObjectItem(branch, "changed"), 1);
    }

    return root;
}

static void update_value(const char *section, const char *key, cJSON *new_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section, 0), key);
    if (obj == NULL)
        cJSON_AddItemToObject(json_root(section, 1), key, new_value);
    else if (!cJSON_Compare(obj, new_value, true))
        cJSON_ReplaceItemInObject(json_root(section, 1), key, new_value);
    else
        cJSON_Delete(new_value);
}

void rg_settings_init(bool _safe_mode)
{
    config_root = cJSON_CreateObject();
    safe_mode = _safe_mode;
    json_root(NS_GLOBAL, 0);
    json_root(NS_BOOT, 0);
}

void rg_settings_commit(void)
{
    if (!config_root || safe_mode)
        return;

    for (cJSON *branch = config_root->child; branch; branch = branch->next)
    {
        char *name = cJSON_GetStringValue(cJSON_GetObjectItem(branch, "namespace"));
        int changed = cJSON_GetNumberValue(cJSON_GetObjectItem(branch, "changed"));

        if (!changed)
            continue;

        char *buffer = cJSON_Print(cJSON_GetObjectItem(branch, "values"));
        if (!buffer)
            continue;

        size_t buffer_len = strlen(buffer) + 1;

        char pathbuf[RG_PATH_MAX];
        snprintf(pathbuf, RG_PATH_MAX, "%s/%s.json", RG_BASE_PATH_CONFIG, name);
        if (rg_storage_write_file(pathbuf, buffer, buffer_len, 0) ||
            (rg_storage_mkdir(rg_dirname(pathbuf)) && rg_storage_write_file(pathbuf, buffer, buffer_len, 0)))
        {
            cJSON_SetNumberHelper(cJSON_GetObjectItem(branch, "changed"), 0);
        }

        cJSON_free(buffer);
    }

    rg_storage_commit();
}

void rg_settings_reset(void)
{
    RG_LOGI("Clearing settings...\n");
    rg_storage_delete(RG_BASE_PATH_CONFIG);
    rg_storage_mkdir(RG_BASE_PATH_CONFIG);
    if (config_root)
    {
        cJSON_Delete(config_root);
        config_root = cJSON_CreateObject();
    }
}

bool rg_settings_get_boolean(const char *section, const char *key, bool default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section, 0), key);
    if (cJSON_IsNumber(obj)) // Backwards compatible with plain numbers
        return obj->valueint != 0;
    return cJSON_IsBool(obj) ? cJSON_IsTrue(obj) : default_value;
}

void rg_settings_set_boolean(const char *section, const char *key, bool value)
{
    update_value(section, key, cJSON_CreateBool(value));
}

double rg_settings_get_number(const char *section, const char *key, double default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section, 0), key);
    return cJSON_IsNumber(obj) ? obj->valuedouble : default_value;
}

void rg_settings_set_number(const char *section, const char *key, double value)
{
    update_value(section, key, cJSON_CreateNumber(value));
}

char *rg_settings_get_string(const char *section, const char *key, const char *default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section, 0), key);
    if (cJSON_IsString(obj))
        return strdup(obj->valuestring);
    return default_value ? strdup(default_value) : NULL;
}

void rg_settings_set_string(const char *section, const char *key, const char *value)
{
    update_value(section, key, value ? cJSON_CreateString(value) : cJSON_CreateNull());
}

void rg_settings_delete(const char *section, const char *key)
{
    cJSON_DeleteItemFromObject(json_root(section, 1), key);
}

bool rg_settings_exists(const char *section, const char *key)
{
    return cJSON_GetObjectItem(json_root(section, 0), key) != NULL;
}
