#include "rg_system.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cJSON.h>

static const char *config_file_path = RG_BASE_PATH_CONFIG "/retro-go.json";
static cJSON *config_root = NULL;
static int unsaved_changes = 0;


static cJSON *json_root(const char *name)
{
    RG_ASSERT(config_root, "json_root called before settings were initialized!");

    if (name == NS_GLOBAL)
        name = "global";
    else if (name == NS_APP)
        name = rg_system_get_app()->configNs;
    else if (name == NS_FILE)
        name = rg_system_get_app()->romPath;

    cJSON *myroot;

    if (!name)
    {
        myroot = config_root;
    }
    else if (!(myroot = cJSON_GetObjectItem(config_root, name)))
    {
        myroot = cJSON_AddObjectToObject(config_root, name);
    }
    else if (!cJSON_IsObject(myroot))
    {
        myroot = cJSON_CreateObject();
        cJSON_ReplaceItemInObject(config_root, name, myroot);
    }

    return myroot;
}

void rg_settings_init(void)
{
    FILE *fp = fopen(config_file_path, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        long length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *buffer = calloc(1, length + 1);
        if (fread(buffer, 1, length, fp))
            config_root = cJSON_Parse(buffer);
        free(buffer);
        fclose(fp);
    }

    if (!config_root)
    {
        RG_LOGW("Failed to load settings from %s.\n", config_file_path);
        config_root = cJSON_CreateObject();
        return;
    }

    RG_LOGI("Settings loaded from %s.\n", config_file_path);
}

void rg_settings_commit(void)
{
    if (!unsaved_changes)
        return;

    RG_LOGI("Saving %d change(s)...\n", unsaved_changes);

    char *buffer = cJSON_Print(config_root);
    if (!buffer)
    {
        RG_LOGE("cJSON_Print() failed.\n");
        return;
    }

    FILE *fp = fopen(config_file_path, "wb");
    if (!fp)
    {
        rg_storage_delete(config_file_path);
        rg_storage_mkdir(rg_dirname(config_file_path));
        fp = fopen(config_file_path, "wb");
    }
    if (fp)
    {
        if (fputs(buffer, fp) >= 0)
            unsaved_changes = 0;
        fclose(fp);
    }

    if (unsaved_changes > 0)
        RG_LOGE("Save failed! %p\n", fp);

    cJSON_free(buffer);
}

void rg_settings_reset(void)
{
    RG_LOGI("Clearing settings...\n");
    cJSON_Delete(config_root);
    config_root = cJSON_CreateObject();
    unsaved_changes++;
    rg_storage_commit();
}

double rg_settings_get_number(const char *section, const char *key, double default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section), key);
    return obj ? obj->valuedouble : default_value;
}

void rg_settings_set_number(const char *section, const char *key, double value)
{
    cJSON *root = json_root(section);
    cJSON *obj = cJSON_GetObjectItem(root, key);

    if (!cJSON_IsNumber(obj))
    {
        cJSON_Delete(cJSON_DetachItemViaPointer(root, obj));
        cJSON_AddNumberToObject(root, key, value);
        unsaved_changes++;
    }
    else if (obj->valuedouble != value)
    {
        cJSON_SetNumberHelper(obj, value);
        unsaved_changes++;
    }
}

char *rg_settings_get_string(const char *section, const char *key, const char *default_value)
{
    cJSON *obj = cJSON_GetObjectItem(json_root(section), key);
    if (cJSON_IsString(obj))
        return strdup(obj->valuestring);
    return default_value ? strdup(default_value) : NULL;
}

void rg_settings_set_string(const char *section, const char *key, const char *value)
{
    cJSON *root = json_root(section);
    cJSON *obj = cJSON_GetObjectItem(root, key);
    cJSON *newobj = value ? cJSON_CreateString(value) : cJSON_CreateNull();

    if (obj == NULL)
    {
        cJSON_AddItemToObject(root, key, newobj);
        unsaved_changes++;
    }
    else if (!cJSON_Compare(obj, newobj, true))
    {
        cJSON_ReplaceItemInObject(root, key, newobj);
        unsaved_changes++;
    }
    else
    {
        cJSON_Delete(newobj);
    }
}

void rg_settings_delete(const char *section, const char *key)
{
    cJSON *root = json_root(section);
    if (key)
        cJSON_DeleteItemFromObject(root, key);
    else if (root != config_root)
        cJSON_Delete(cJSON_DetachItemViaPointer(config_root, root));
    unsaved_changes++;
}
