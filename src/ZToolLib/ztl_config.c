#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ztl_palloc.h"
#include "ztl_utils.h"
#include "ztl_config.h"

#define ZTL_CONFIG_DEFAULT_ITEMS    512

typedef struct  
{
    char*   desc;       // like "1", "true", "false", "on", "off"
    uint8_t val;        // 0 or 1
}ztl_bool_value_t;


static ztl_bool_value_t zbooltable[] = {
    { "1",      1 },
    { "0",      0 },
    { "YES",    1 },
    { "NO",     0 },
    { "Yes",    1 },
    { "No",     0 },
    { "yes",    1 },
    { "no",     0 },
    { "ON",     1 },
    { "OFF",    0 },
    { "on",     1 },
    { "off",    0 },
    { "TRUE",   1 },
    { "FALSE",  0 },
    { "True",   1 },
    { "False",  0 },
    { "true",   1 },
    { "false",  0 },
    { NULL,     0}
};

struct ztl_config_s
{
    char                filename[512];
    char                comment;
    char                delimiter;
    uint32_t            count;
    uint32_t            nalloc;
    ztl_pair_value_t*   items;
    ztl_pool_t*         pool;
};

/* do read & parse file content */
static int ztl_read_file_content(ztl_config_t* zconf);


ztl_config_t* ztl_config_open(const char* filename, char comment, char delimiter)
{
    ztl_config_t*   zconf;
    ztl_pool_t*     pool;

    if (comment == 0)
        comment = '#';
    if (delimiter == 0)
        delimiter = ' ';

    pool = ztl_create_pool(ZTL_DEFAULT_POOL_SIZE);
    if (!pool) 
    {
        return NULL;
    }

    zconf = (ztl_config_t*)ztl_pcalloc(pool, sizeof(ztl_config_t));
    strncpy(zconf->filename, filename, sizeof(zconf->filename) - 1);
    zconf->comment   = comment;
    zconf->delimiter = delimiter;
    zconf->count     = 0;
    zconf->nalloc    = ZTL_CONFIG_DEFAULT_ITEMS;
    zconf->items     = ztl_pcalloc(pool, zconf->nalloc * sizeof(ztl_pair_value_t));
    zconf->pool      = pool;

    // open the file and read content
    if (ztl_read_file_content(zconf) != 0)
    {
        ztl_config_close(zconf);
        return NULL;
    }

    return zconf;
}

void ztl_config_close(ztl_config_t* zconf)
{
    if (!zconf) {
        return;
    }

    ztl_pool_t* pool = zconf->pool;
    if (pool) {
        ztl_destroy_pool(pool);
    }
}

bool ztl_config_set_item(ztl_config_t* zconf, const char* key, const char* val, bool overwrite)
{
    ztl_pair_value_t* lpitem;
    uint32_t keylen = (uint32_t)strlen(key);
    uint32_t vallen = (uint32_t)strlen(val);

    if (keylen == 0 || vallen == 0) {
        return false;
    }

    for (uint32_t i = 0; i < zconf->count; ++i)
    {
        if (strcmp(zconf->items[i].key, key) == 0)
        {
            if (overwrite)
            {
                char* lpcopy = (char*)ztl_pcalloc(zconf->pool, strlen(val) + 1);
                memcpy(lpcopy, val, strlen(val));
                zconf->items[i].value = lpcopy;
                return true;
            }
            return false;
        }
    }

    if (zconf->count >= zconf->nalloc) {
        zconf->nalloc *= 2;
        zconf->items = realloc(zconf->items, zconf->nalloc * sizeof(ztl_pair_value_t));
    }
    lpitem = &zconf->items[zconf->count++];

    lpitem->key = (char*)ztl_palloc(zconf->pool, keylen + 1);
    lpitem->value = (char*)ztl_palloc(zconf->pool, vallen + 1);
    memcpy(lpitem->key, key, keylen);
    memcpy(lpitem->value, val, vallen);
    lpitem->key[keylen] = 0;
    lpitem->value[vallen] = 0;

    return true;
}

int ztl_config_all_items(ztl_config_t* zconf, ztl_pair_value_t* pair_values[], uint32_t size)
{
    uint32_t n;
    for (n = 0; n < zconf->count; ++n)
    {
        if (n >= size)
            break;
        pair_values[n] = &zconf->items[n];
    }
    return n;
}

bool ztl_config_read_str(ztl_config_t* zconf, const char* key, char** poutval, int* outlen)
{
    char* lpv;
    lpv = ztl_config_have(zconf, key, 0);
    if (lpv) {
        if (poutval)
            *poutval = lpv;
        if (outlen)
            *outlen = (int)strlen(lpv);
        return true;
    }
    return false;
}

bool ztl_config_read_strval(ztl_config_t* zconf, const char* key, char* outval, int len)
{
    char* pv;
    int   vallen;
    if (ztl_config_read_str(zconf, key, &pv, &vallen)) {
        strncpy(outval, pv, len);
        return true;
    }
    return false;
}

bool ztl_config_read_int16(ztl_config_t* zconf, const char* key, void* outi16)
{
    char* lpv;
    lpv = ztl_config_have(zconf, key, 0);
    if (lpv) {
        *((int16_t*)outi16) = atoi(lpv);
        return true;
    }
    return false;
}

bool ztl_config_read_int32(ztl_config_t* zconf, const char* key, void* outi32)
{
    char* lpv;
    lpv = ztl_config_have(zconf, key, 0);
    if (lpv) {
        *((int32_t*)outi32) = atoi(lpv);
        return true;
    }
    return false;
}

bool ztl_config_read_int64(ztl_config_t* zconf, const char* key, void* outi64)
{
    char* lpv;
    lpv = ztl_config_have(zconf, key, 0);
    if (lpv) {
        *((int64_t*)outi64) = atoll(lpv);
        return true;
    }
    return false;
}

bool ztl_config_read_double(ztl_config_t* zconf, const char* key, void* outdbl)
{
    char* lpv;
    lpv = ztl_config_have(zconf, key, 0);
    if (lpv) {
        *((double*)outdbl) = atof(lpv);
        return true;
    }
    return false;
}

bool ztl_config_read_bool(ztl_config_t* zconf, const char* key, bool* outbool)
{
    char* lpv;
    lpv = ztl_config_have(zconf, key, 0);
    if (lpv) {
        *outbool = ztl_boolvalue_lookup(lpv);
        return true;
    }
    return false;
}


bool ztl_boolvalue_lookup(const char* desc)
{
    if (!desc || !desc[0]) {
        return false;
    }

    for (int i = 0; zbooltable[i].desc; ++i)
    {
        if (ztl_stricmp(zbooltable[i].desc, desc) == 0) {
            return zbooltable[i].val;
        }
    }

    return false;
}

char* ztl_config_have(ztl_config_t* zconf, const char* key, int strictly)
{
    ztl_pair_value_t* pv;
    for (uint32_t i = 0; i < zconf->count; ++i)
    {
        pv = &zconf->items[i];
        if ((strictly && strcmp(pv->key, key) == 0) || (ztl_stricmp(pv->key, key) == 0)) {
            return pv->value;
        }
    }

    return NULL;
}


static int ztl_read_file_content(ztl_config_t* zconf)
{
    char        buffer[1000];
    FILE*       fp;
    char        *lpkey, *lpval;

    fp = fopen(zconf->filename, "r");
    if (fp == NULL) {
        return -1;
    }

    // parse the file content
    while (!feof(fp))
    {
        memset(buffer, 0, sizeof(buffer));
        (void)fgets(buffer, sizeof(buffer) - 1, fp);

        lefttrim(buffer);
        replace_char(buffer, '\t', ' ');
        if (buffer[0] == zconf->comment) {
            continue;
        }

        lpval = strchr(buffer, zconf->delimiter);
        if (lpval == NULL) {
            continue;
        }
        *lpval++ = '\0';

        // key
        lpkey = buffer;
        righttrim(lpkey);

        // value
        lefttrim(lpval);
        righttrim(lpval);

        ztl_config_set_item(zconf, lpkey, lpval, true);
    }
    fclose(fp);

    return 0;
}
