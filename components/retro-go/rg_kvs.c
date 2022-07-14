#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "rg_system.h"

#define RG_KVS_MAGIC 0x564B4752 // RGKV

typedef struct
{
    uint32_t magic;
    uint32_t checksum;
    uint32_t reserved[6];
} kvs_header_t;

typedef struct
{
    char key[24];
    uint32_t datasize;
    uint32_t blocksize;
} kvs_entry_t;

// struct rg_kvs_t
// {
//
// }

static bool kvs_find(rg_kvs_t *kvs, const char *key, kvs_entry_t *out)
{
    size_t initial_pos = ftell(kvs);
    bool from_start = false;
    kvs_entry_t entry;

    while (!from_start || ftell(kvs) < initial_pos)
    {
        if (!fread(&entry, sizeof(kvs_entry_t), 1, kvs))
        {
            if (!from_start)
            {
                fseek(kvs, sizeof(kvs_header_t), SEEK_SET);
                from_start = true;
                continue;
            }
            break;
        }
        // RG_LOGI("key '%s'\n", var.key);

        if (strncmp(entry.key, key, sizeof(entry.key)) == 0)
        {
            if (out)
                memcpy(out, &entry, sizeof(kvs_entry_t));
            return true;
        }
        fseek(kvs, entry.blocksize, SEEK_CUR);
    }
    return false;
}

rg_kvs_t *rg_kvs_open(const char *filename)
{
    kvs_header_t header;
    FILE *fp;

    if ((fp = fopen(filename, "r+")))
    {
        fread(&header, sizeof(kvs_header_t), 1, fp);
        if (header.magic == RG_KVS_MAGIC)
            return fp;
        fclose(fp);
        RG_LOGW("KVS '%s' is corrupted and will be overwritten!\n", filename);
    }

    if ((fp = fopen(filename, "w+")))
    {
        header.magic = RG_KVS_MAGIC;
        fwrite(&header, sizeof(kvs_header_t), 1, fp);
        return fp;
    }

    fclose(fp);
    return NULL;
}

bool rg_kvs_close(rg_kvs_t *kvs)
{
    fclose(kvs);
    return true;
}

bool rg_kvs_get(rg_kvs_t *kvs, const char *key, void *buffer, size_t size)
{
    kvs_entry_t entry;
    if (kvs_find(kvs, key, &entry))
    {
        if (fread(buffer, RG_MIN(entry.datasize, size), 1, kvs))
        {
            if (entry.datasize > entry.blocksize)
                fseek(kvs, entry.blocksize - entry.datasize, SEEK_CUR);
            return true;
        }
        fseek(kvs, 0, SEEK_SET);
    }
    return false;
}

bool rg_kvs_put(rg_kvs_t *kvs, const char *key, const void *buffer, size_t size)
{
    kvs_entry_t entry = {0};

    if (kvs_find(kvs, key, &entry))
    {
        fseek(kvs, -sizeof(kvs_entry_t), SEEK_CUR);
        if (size > entry.blocksize)
        {
            // This is dumb but we don't support growing for now...
            strcpy(entry.key, "????????");
            entry.datasize = 0;
            fwrite(&entry, sizeof(kvs_entry_t), 1, kvs);
            fseek(kvs, 0, SEEK_END);
        }
    }
    else
    {
        fseek(kvs, 0, SEEK_END);
    }

    strncpy(entry.key, key, sizeof(entry.key) - 1);
    entry.datasize = size;
    entry.blocksize = RG_MAX(size, entry.blocksize);
    entry.blocksize = (((entry.blocksize - 1) & ~3) | 3) + 1; // 4bytes alignment
    fwrite(&entry, sizeof(kvs_entry_t), 1, kvs);
    fwrite(buffer, size, 1, kvs);
    for (size_t i = entry.datasize; i < entry.blocksize; ++i)
    {
        fputc(0, kvs);
    }

    return true;
}

bool rg_kvs_delete(rg_kvs_t *kvs, const char *key)
{
    return false;
}

bool rg_kvs_vacuum(rg_kvs_t *kvs)
{
    return false;
}
