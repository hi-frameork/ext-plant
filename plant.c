/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Russell                                                      |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_plant.h"
#include "SAPI.h"

/* cli sapi 模式下不处理 header 透传 */
#define  CHECK_SAPI_NAME do {                                                                   \
    if ( (strncmp(sapi_module.name, "fpm-fcgi", sizeof("fpm-fcgi") -1) != 0)                    \
            && (strncmp(sapi_module.name, "apache", sizeof("apache") -1) != 0)                  \
            && (strncmp(sapi_module.name, "cli-server", sizeof("cli-server") -1) != 0)) {       \
                PLANT_G(enable_sapi) = 0;                                                       \
                return SUCCESS;                                                                 \
    }                                                                                           \
}while(0)

PHP_FUNCTION(plant_curl_setopt);
PHP_FUNCTION(plant_curl_exec);
PHP_FUNCTION(plant_curl_setopt_array);
PHP_FUNCTION(plant_curl_reset);

static void (*ori_execute_internal)(zend_execute_data *execute_data_ptr, zend_fcall_info *fci, int return_value_used TSRMLS_DC);
ZEND_API void plant_execute_internal(zend_execute_data *execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC);

ZEND_DECLARE_MODULE_GLOBALS(plant)

/* Make sapi_module accessable */
extern sapi_module_struct sapi_module;


/* {{{ plant reload struct */
typedef struct plant_reload_def_st {
    char *orig_func;
    char *over_func;
    char *save_func;
} plant_reload_def;
/* }}} */

/* {{{ plant_reload_def */ 
static const plant_reload_def prd[] = {
    {"curl_setopt",         "plant_curl_setopt",         "origin_plant_curl_setopt"},
    {"curl_exec",           "plant_curl_exec",           "origin_plant_curl_exec"},
    {"curl_setopt_array",   "plant_curl_setopt_array",   "origin_plant_curl_setopt_array"},
    {"curl_reset",          "plant_curl_reset",          "origin_plant_curl_reset"},
    {NULL, NULL, NULL}
};
/* }}} */


/* {{{ origin_funtion_handler */
zend_function *origin_curl_setopt       = NULL;
zend_function *origin_curl_exec         = NULL;
zend_function *origin_curl_setopt_array = NULL;
zend_function *origin_curl_reset        = NULL;
/* }}} */

/* {{{ plant reload curl function for performance */
static void plant_reload_curl_function()
{
    zend_function *orig, *replace;
    const plant_reload_def *p = &(prd[0]);
    while(p->orig_func != NULL) {
        if (zend_hash_find(CG(function_table), p->save_func, strlen(p->orig_func) + 1, (void **)&orig) != SUCCESS) {
            zend_hash_find(CG(function_table), p->over_func, strlen(p->over_func) + 1, (void **)&replace);
            if (zend_hash_find(CG(function_table), p->orig_func, strlen(p->orig_func) + 1, (void **)&orig) == SUCCESS) {
                zend_hash_add(CG(function_table), p->save_func, strlen(p->save_func)+1, orig, sizeof(zend_function), NULL);
                zend_hash_update(CG(function_table), p->orig_func, strlen(p->orig_func) + 1, replace, sizeof(zend_function), NULL);
            }
        }
        p++;
    }

    // /* retrieve function from function table */
    zend_function *orig_func;
    if (zend_hash_find(CG(function_table), "origin_plant_curl_setopt", sizeof("origin_plant_curl_setopt"), (void **)&orig_func) == SUCCESS ) {
        origin_curl_setopt = orig_func;
    }
    if (zend_hash_find(CG(function_table), "origin_plant_curl_exec", sizeof("origin_plant_curl_exec"), (void **)&orig_func) == SUCCESS ) {
        origin_curl_exec = orig_func;
    }
    if (zend_hash_find(CG(function_table), "origin_plant_curl_setopt_array", sizeof("origin_plant_curl_setopt_array"), (void **)&orig_func) == SUCCESS ) {
        origin_curl_setopt_array = orig_func;
    }
    if (zend_hash_find(CG(function_table), "origin_plant_curl_reset", sizeof("origin_plant_curl_reset"), (void **)&orig_func) == SUCCESS ) {
        origin_curl_reset = orig_func;
    }
}
/* }}} */

/* {{{ clear replace function */
static void plant_clear_reload_function()
{
    const plant_reload_def *p = &(prd[0]);
    zend_function *orig;
    while (p->orig_func != NULL) {
        if (zend_hash_find(CG(function_table), p->save_func, strlen(p->save_func)+1, (void **)&orig) == SUCCESS) {
              zend_hash_update(CG(function_table), p->orig_func, strlen(p->orig_func)+1, orig, sizeof(zend_function), NULL);
              zend_hash_del(CG(function_table), p->save_func, strlen(p->save_func)+1); 
         }
        p++;
    }
}
/* }}} */

/* {{{ 创建透传流量标识 header 信息（如果存在 */
zend_bool add_header_route_label(plant_interceptor_t *pit, zval *headers)
{
    if (PLANT_G(route_label_value).len) {
        char *pass_value;
        int value_size;
        value_size = strlen(PLANT_G(route_label_key)) + sizeof(": ") - 1 + PLANT_G(route_label_value).len + 1;
        pass_value = emalloc(value_size);
        snprintf(pass_value, value_size, "%s: %s", PLANT_G(route_label_key), PLANT_G(route_label_value).c);
        pass_value[value_size - 1] = '\0';
        add_next_index_string(headers, pass_value, 1);
        efree(pass_value);
        return 1;
    }

    return 0;
}
/* }}} */


/* {{{ plant_curl_setopt */
PHP_FUNCTION(plant_curl_setopt)
{
    /* 前置操作 */
    zval *zid, **zvalue;
    long options;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlZ",  &zid, &options, &zvalue) == SUCCESS) {
        if (options == Z_LVAL(PLANT_G(pit).curl_http_header_const) && PLANT_Z_TYPE_PP(zvalue) == IS_ARRAY) {
            zval *copy_header;
            ALLOC_INIT_ZVAL(copy_header);
            ZVAL_ZVAL(copy_header, *zvalue, 1, 0);
            add_index_zval(PLANT_G(pit).curl_header_record, Z_RESVAL_P(zid), copy_header);
        }
    }
   
    /* 源方法调用 */
    if (origin_curl_setopt != NULL) {
        origin_curl_setopt->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    /* 后直操作 */
}
/* }}} */

/* {{{ plant_curl_setopt_array */
PHP_FUNCTION(plant_curl_setopt_array)
{
    /* 前置操作 */
    zval *zid, *arr;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra", &zid, &arr) == SUCCESS) {
        HashTable *ht = Z_ARRVAL_P(arr);
        zval *http_header = NULL;
        if (plant_zend_hash_index_zval_find(ht, Z_LVAL(PLANT_G(pit).curl_http_header_const), (void **)&http_header) == SUCCESS) {
            zval *copy_header;
            ALLOC_INIT_ZVAL(copy_header);
            ZVAL_ZVAL(copy_header, http_header, 1, 0);
            add_index_zval(PLANT_G(pit).curl_header_record, Z_RESVAL_P(zid), copy_header);
        }
    }
    
    /* 源方法调用 */
    if (origin_curl_setopt_array != NULL) {
        origin_curl_setopt_array->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    /* 后直操作 */
}
/* }}} */

/* {{{ plant_curl_exec */
PHP_FUNCTION(plant_curl_exec)
{
    /* 前置操作 */
    zval *res;

    int result = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res);
    if (result == SUCCESS) {

        plant_interceptor_t *pit = &PLANT_G(pit);
        zval *tmp = NULL;
        zval *option = NULL;
        int is_init = 0;

        if (plant_zend_hash_index_zval_find(Z_ARRVAL_P(pit->curl_header_record), Z_RESVAL_P(res), (void **)&tmp) == SUCCESS) {
            option = tmp;
        } else {
            ALLOC_INIT_ZVAL(option);
            array_init(option);
            is_init = 1;
        }

        // 创建透传流量标识 header 信息（如果存在）
        add_header_route_label(pit, option);

        /* 写入 curl header */
        zval func;
        zval *argv[3];
        zval ret;
        ZVAL_STRING(&func, "curl_setopt", 1);
        argv[0] = res;
        argv[1] = &pit->curl_http_header_const;
        argv[2] = option;
        call_user_function(EG(function_table), (zval **)NULL, &func, &ret, 3, argv);
        zval_dtor(&ret);

        if (is_init == 1) {
            zval_ptr_dtor(&option);
        }
        zval_dtor(&func); 
    }
    
    /* 源方法调用 */
    if (origin_curl_exec != NULL) {
        origin_curl_exec->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    /* 后直操作 */
}
/* }}} */

/* {{{ plant_curl_reset */
PHP_FUNCTION(plant_curl_reset)
{
    /* 前置操作 */

    /* 源方法调用 */
    if (origin_curl_reset != NULL) {
        origin_curl_reset->internal_function.handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    }

    /* 后直操作 */
}
/* }}} */

/* True global resources - no need for thread safety here */
// static int le_plant;

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("plant.enable",      "1",      PHP_INI_SYSTEM, OnUpdateBool, enable, zend_plant_globals, plant_globals)
    STD_PHP_INI_ENTRY("plant.route_label", "Xe-Tag", PHP_INI_SYSTEM, OnUpdateString, route_label_key, zend_plant_globals, plant_globals)
PHP_INI_END()
/* }}} */


/* {{{ 暴露获取流量标识能力 */
PHP_FUNCTION(plant_route_label)
{
    if (PLANT_G(route_label_value).len == 0) {
        RETURN_STRINGL("No HTTP_XE_TAG Header", strlen("No HTTP_XE_TAG Header"), 0);
        return;
    }
    
    RETURN_STRING(PLANT_G(route_label_value).c, 0)
}
/* }}} */


/* {{{ php_plant_init_globals
 */
static void php_plant_init_globals(zend_plant_globals *plant_globals)
{
    plant_globals->enable_sapi = 1;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(plant)
{
    ZEND_INIT_MODULE_GLOBALS(plant, php_plant_init_globals, NULL);
	REGISTER_INI_ENTRIES();

    if (!PLANT_G(enable)) {
    	return SUCCESS;
    }

    CHECK_SAPI_NAME;

    ori_execute_internal = zend_execute_internal;
    zend_execute_internal = plant_execute_internal;

    /* 为 curl 增加 hook（以此实现流量标识透传能力） */
    plant_reload_curl_function();

    plant_interceptor_t *pit = &PLANT_G(pit);
    ZVAL_LONG(&(pit->curl_http_header_const), -1);
    zend_get_constant("CURLOPT_HTTPHEADER", sizeof("CURLOPT_HTTPHEADER") - 1, &pit->curl_http_header_const);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(plant)
{
	UNREGISTER_INI_ENTRIES();

    if (!PLANT_G(enable)) {
    	return SUCCESS;
    }

    CHECK_SAPI_NAME;

    zend_execute_internal = ori_execute_internal;

    /* 重置被 hook 的 curl 函数 */
    plant_clear_reload_function();

    plant_interceptor_t *pit = &PLANT_G(pit);
    zval_dtor(&pit->curl_http_header_const);

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(plant)
{
	if (!PLANT_G(enable)) {
        return SUCCESS;
    }

    if (!PLANT_G(enable_sapi)) {
        return SUCCESS;
    }

    /* 获取请求原始流量标识 */
    extract_route_label(&PLANT_G(route_label_value));

    /* headder 记录缓存初始化 */
    plant_interceptor_t *pit = &PLANT_G(pit);
    ALLOC_INIT_ZVAL(pit->curl_header_record);
    array_init(pit->curl_header_record);

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(plant)
{
    if (!PLANT_G(enable)) {
        return SUCCESS;
    }

    if (!PLANT_G(enable_sapi)) {
        return SUCCESS;
    }

    /* 清空本次请求流量标识 */
    smart_str_free(&PLANT_G(route_label_value));

    plant_interceptor_t *pit = &PLANT_G(pit);
    zval_ptr_dtor(&pit->curl_header_record);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(plant)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "plant support", "enabled");
	php_info_print_table_row(2, "Route label", PLANT_G(route_label_key));
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ plant_functions[]
 *
 * Every user visible function must have an entry in plant_functions[].
 */
const zend_function_entry plant_functions[] = {
	PHP_FE(plant_route_label, NULL)
    PHP_FE(plant_curl_setopt, NULL)
    PHP_FE(plant_curl_setopt_array, NULL)
    PHP_FE(plant_curl_exec, NULL)
    PHP_FE(plant_curl_reset, NULL)
	PHP_FE_END	                        /* Must be the last line in plant_functions[] */
};
/* }}} */


/* {{{ plant_deps */
static const zend_module_dep plant_deps[] = {
    ZEND_MOD_REQUIRED("curl")
    ZEND_MOD_END
};
/* }}} */

/* {{{ plant_module_entry
 */
zend_module_entry plant_module_entry = {
	STANDARD_MODULE_HEADER_EX,
    NULL,
    plant_deps,
    "plant",
    plant_functions,
    PHP_MINIT(plant),
    PHP_MSHUTDOWN(plant),
    PHP_RINIT(plant),
    PHP_RSHUTDOWN(plant),
    PHP_MINFO(plant),
    PHP_PLANT_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PLANT
ZEND_GET_MODULE(plant)
#endif


ZEND_API void plant_execute_core(int internal, zend_execute_data *execute_data, zend_fcall_info *fci, int rvu TSRMLS_DC)
{
    zend_bool dobailout = 0;

    /* Call original under zend_try. baitout will be called when exit(), error
     * occurs, exception thrown and etc, so we have to catch it and free our
     * resources. */
    zend_try {
        if (internal) {
            if (ori_execute_internal) {
                ori_execute_internal(execute_data, fci, rvu TSRMLS_CC);
            } else {
                execute_internal(execute_data, fci, rvu TSRMLS_CC);
            }
        }
    } zend_catch {
        dobailout = 1;
        /* call zend_bailout() at the end of this function, we still want to
         * send message. */
    } zend_end_try();

    if (dobailout) {
        zend_bailout();
    }
}

ZEND_API void plant_execute_internal(zend_execute_data *execute_data, zend_fcall_info *fci, int return_value_used TSRMLS_DC)
{
    plant_execute_core(1, execute_data, fci, return_value_used TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
