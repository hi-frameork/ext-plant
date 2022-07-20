/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_PLANT_H
#define PHP_PLANT_H

extern zend_module_entry plant_module_entry;
#define phpext_plant_ptr &plant_module_entry

#define PHP_PLANT_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_PLANT_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PLANT_API __attribute__ ((visibility("default")))
#else
#	define PHP_PLANT_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "ext/standard/php_smart_string.h"
#include "plant_util.h"

PHP_MINIT_FUNCTION(plant);
PHP_MSHUTDOWN_FUNCTION(plant);
PHP_RINIT_FUNCTION(plant);
PHP_RSHUTDOWN_FUNCTION(plant);
PHP_MINFO_FUNCTION(plant);


ZEND_BEGIN_MODULE_GLOBALS(plant)
    zend_bool               enable;                    /* 配置(ini): plant.enable */
    char                    *route_label_key;          /* 配置(ini): plant.route_label */

    plant_interceptor_t     pit;                       /* 拦截头模块 */

    smart_string            route_label_value;         /* 请求原始流量标识值（来自 route_label_key） */
    zend_bool               enable_sapi;               /* enable_sapi */
ZEND_END_MODULE_GLOBALS(plant)

/* Always refer to the globals in your function as PLANT_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define PLANT_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(plant, v)

#if defined(ZTS) && defined(COMPILE_DL_PLANT)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif	/* PHP_PLANT_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
