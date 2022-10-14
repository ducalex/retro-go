#include "rg_system.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cJSON.h>

static cJSON *config_root = NULL;


static FILE *open_config_file(const char *name, const char *mode)
{
    char pathbuf[RG_PATH_MAX];
    snprintf(pathbuf, RG_PATH_MAX, "%s/%s.json", RG_BASE_PATH_CONFIG, name);
    RG_LOGI("Opening %s for %s", pathbuf, mode);
    FILE *fp = fopen(pathbuf, mode);
    if (!fp)
    {
        rg_storage_mkdir(rg_dirname(pathbuf));
        rg_storage_delete(pathbuf);
        fp = fopen(pathbuf, mode);
    }
    return fp;
}

static cJSON *json_root(const char *name, bool mode)
{
    RG_ASSERT(config_root, "json_root called before settings were initialized!");

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
        FILE *fp = open_config_file(name, "rb");
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            long length = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char *buffer = calloc(1, length + 1);
            if (fread(buffer, 1, length, fp))
                cJSON_AddItemToObject(branch, "values", cJSON_Parse(buffer));
            free(buffer);
            fclose(fp);
        }
    }

    cJSON *root = cJSON_GetObjectItem(branch, "values");
    if (!cJSON_IsObject(root))
    {
        cJSON_DeleteItemFromObject(branch, "values");
        root = cJSON_AddObjectToObject(branch, "values");
    }

    if (mode)
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

void rg_settings_init(void)
{
    config_root = cJSON_CreateObject();
    json_root(NS_GLOBAL, 0);
    json_root(NS_BOOT, 0);
}

void rg_settings_commit(void)
{
    if (!config_root)
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

        FILE *fp = open_config_file(name, "wb");
        if (fp)
        {
            if (fputs(buffer, fp) >= 0)
                cJSON_SetNumberHelper(cJSON_GetObjectItem(branch, "changed"), 0);
            fclose(fp);
        }

        cJSON_free(buffer);
    }

    rg_storage_commit();
}

void rg_settings_reset(void)
{
    RG_LOGI("Clearing settings...\n");
    rg_storage_delete(RG_BASE_PATH_CONFIG);
    cJSON_Delete(config_root);
    config_root = cJSON_CreateObject();
}

double rg_settings_get_number(const char *section, const char *key, double default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section, 0), key);
    return obj ? obj->valuedouble : default_value;
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
