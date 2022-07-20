
#ifndef PLANT_UTIL_H
#define PLANT_UTIL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#include "php.h"


#define IS_TRUE                     1
#define IS_FALSE                    -1

typedef struct {
    zval        curl_http_header_const;            /* curl add header const */
    zval        *curl_header_record;               /* record curl handler set header */
}plant_interceptor_t;

static inline int PLANT_Z_TYPE_P(zval *z)
{
    if (Z_TYPE_P(z) == IS_BOOL) {
        if ((uint8_t)Z_LVAL_P(z) == 1) {
            return IS_TRUE;     
        } else {
            return IS_FALSE;
        }
    } else {
        return Z_TYPE_P(z);
    }
}
#define PLANT_Z_TYPE_PP(z)     PLANT_Z_TYPE_P(*z)

static inline int plant_zend_hash_index_find(HashTable *ht, ulong h, void **v)
{
    zval **tmp = NULL;
    if (zend_hash_index_find(ht, h, (void **)&tmp) == SUCCESS) {
        *v = *tmp;
        return SUCCESS;
    } else {
        *v = NULL;
        return FAILURE;
    }
}

#define plant_zend_hash_index_zval_find plant_zend_hash_index_find

static inline int plant_zend_hash_find(HashTable *ht, char *k, int len, void **v)
{
    zval **tmp = NULL; 
    if (zend_hash_find(ht, k, len, (void **)&tmp) == SUCCESS) {
        *v = *tmp;
        return SUCCESS;
    } else {
        *v = NULL;
        return FAILURE;
    }
}

/* {{{ 从 $_SERVER 中获取指定字段值 */
static int find_server_var(char *key, int key_size, void **ret) 
{
    /* for jit preload server */
    if (PG(auto_globals_jit)) {
        zend_is_auto_global("_SERVER", sizeof("_SERVER") - 1);
    }

    zval **server = (zval **)&PG(http_globals)[TRACK_VARS_SERVER];
    return plant_zend_hash_find(Z_ARRVAL_P(*server), key, key_size, ret);
}
/* }}} */

/* {{{ 提取流量灰度标识 */
static inline zend_bool extract_route_label(smart_str *value)
{
    smart_str tmp = {0};
    zval *label;
    if (find_server_var("HTTP_XE_TAG", sizeof("HTTP_XE_TAG"), (void **)&label) != SUCCESS) {
        return 0;
    }

    if (label != NULL && (PLANT_Z_TYPE_P(label) == IS_STRING)) {
        smart_str_appendl(&tmp, Z_STRVAL_P(label), Z_STRLEN_P(label));
        smart_str_appendl(value, tmp.c, tmp.len);
        smart_str_0(value);
        smart_str_free(&tmp);
        return 1;
    }

    return 0;
}
/* }}} */

#endif