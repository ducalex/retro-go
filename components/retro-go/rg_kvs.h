#include <stdbool.h>
#include <stddef.h>

// typedef struct rg_kvs_t rg_kvs_t;
typedef void rg_kvs_t;

rg_kvs_t *rg_kvs_open(const char *filename);
bool rg_kvs_close(rg_kvs_t *kvs);
bool rg_kvs_get(rg_kvs_t *kvs, const char *key, void *buffer, size_t length);
bool rg_kvs_put(rg_kvs_t *kvs, const char *key, const void *buffer, size_t length);
bool rg_kvs_delete(rg_kvs_t *kvs, const char *key);
bool rg_kvs_vacuum(rg_kvs_t *kvs);
