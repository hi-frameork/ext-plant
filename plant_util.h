
#ifndef PLANT_UTIL_H
#define PLANT_UTIL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

#define ALLOC_INIT_ZVAL(p)          do{p = emalloc(sizeof(zval)); bzero(p, sizeof(zval));}while(0)
#define PLANT_Z_TYPE_P              Z_TYPE_P
#define PLANT_Z_TYPE_PP(z)          Z_TYPE_P(*z)
#define PLANT_PHP_MAX_PARAMS_NUM    20

#if PHP_VERSION_ID >= 70300
#define HASH_FLAG_PERSISTENT       (1<<0)
#endif

typedef struct {
    zval        curl_http_header_const;            /* curl add header const */
    zval        *curl_header_record;               /* record curl handler set header */
}plant_interceptor_t;


static inline int plant_call_user_function(HashTable *ht, zval **obj, zval *function_name, zval *retval_ptr, uint32_t param_count, zval **params) 
{
    zval pass_params[PLANT_PHP_MAX_PARAMS_NUM];
    int i = 0;
    for(;i < param_count; i++){
        pass_params[i] = *params[i];
    }
    zval *pass_obj = obj ? *obj : NULL;
    return call_user_function(ht, pass_obj, function_name, retval_ptr, param_count, pass_params);
}

static inline int plant_zend_get_constant(char *key, int len, zval *z)
{
    zend_string *key_str = zend_string_init(key, len, 0);
    zval *c = zend_get_constant(key_str); 
    zend_string_free(key_str);
    if (c != NULL) {
        ZVAL_COPY(z,c);
        return 1;
    } else {
        return 0;
    }
}

static inline int plant_zend_hash_index_zval_find(HashTable *ht, ulong h, void **v)
{
    zval *value = zend_hash_index_find(ht, h);
    if (value == NULL) {
        return FAILURE;
    } else {
        *v = value;
        return SUCCESS;
    }
}

static inline int plant_zend_hash_find(HashTable *ht, char *k, int len, void **v)
{
    zval *value = zend_hash_str_find(ht, k, len - 1);
    if (value == NULL) {
        return FAILURE;
    } else {
        *v = value;
        return SUCCESS;
    }
}

/* {{{ 从 $_SERVER 中获取指定字段值 */
static int find_server_var(char *key, int key_size, void **ret) 
{
    /* for jit preload server */
    if (PG(auto_globals_jit)) {
        zend_is_auto_global_str("_SERVER", sizeof("_SERVER") - 1);
    }

    zval *server = &PG(http_globals)[TRACK_VARS_SERVER];
    return plant_zend_hash_find(Z_ARRVAL_P(server), key, key_size, ret);
}
/* }}} */

/* {{{ 提取流量灰度标识 */
static inline zend_bool extract_route_label(smart_string *value)
{
    smart_string tmp = {0};
    zval *label;
    if (find_server_var("HTTP_XE_TAG", sizeof("HTTP_XE_TAG"), (void **)&label) != SUCCESS) {
        return 0;
    }

    if (label != NULL && (PLANT_Z_TYPE_P(label) == IS_STRING)) {
        smart_string_appendl(&tmp, Z_STRVAL_P(label), Z_STRLEN_P(label));
        smart_string_appendl(value, tmp.c, tmp.len);
        smart_string_0(value);
        smart_string_free(&tmp);
        return 1;
    }

    return 0;
}
/* }}} */


#endif