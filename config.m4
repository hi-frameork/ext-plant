dnl $Id$
dnl config.m4 for extension plant

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(plant, for plant support,
dnl Make sure that the comment is aligned:
dnl [  --with-plant             Include plant support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(plant, whether to enable plant support,
[  --enable-plant           Enable plant support])


# PHP enable debug模型下开启扩展debug支持
if test -z "$PHP_DEBUG"; then
  AC_ARG_ENABLE(debug,
    [--enable-debug  compile with debugging system],
    [PHP_DEBUG=$enableval], [PHP_DEBUG=no]
  )
fi

if test "$PHP_PLANT" != "no"; then

  dnl check ZTS support
  if test "$PHP_THREAD_SAFETY" == "yes"; then
    AC_MSG_ERROR([molten does not support ZTS])
  fi

  dnl check mmap functions
  AC_CHECK_FUNCS(mmap)
  AC_CHECK_FUNCS(munmap)

  dnl check for curl
  AC_MSG_CHECKING([for curl-config])
  CURL_CONFIG="curl-config"
  CURL_CONFIG_PATH=`$php_shtool path $CURL_CONFIG`

  dnl for curl-config
  if test -f "$CURL_CONFIG_PATH" && test -x "$CURL_CONFIG_PATH" && $CURL_CONFIG_PATH --version > /dev/null 2>&1; then
      AC_MSG_RESULT([$CURL_CONFIG_PATH])
      CURL_LIB_NAME=curl
      dnl CURL_INCLUDE=`$CURL_CONFIG_PATH --prefix`/include/curl
      PHP_CHECK_LIBRARY($CURL_LIB_NAME, curl_easy_init, [
        dnl add curl include dir,  the lib dir in general path
        dnl PHP_EVAL_INCLINE($CURL_INCLUDE)
        PHP_ADD_LIBRARY($CURL_LIB_NAME,1,CURL_SHARED_LIBADD)
        PHP_SUBST(CURL_SHARED_LIBADD)
        AC_DEFINE(HAS_CURL, 1, [we have curl to execute curl])
      ] ,[
        AC_MSG_ERROR([libcurl not found])
     ])
  else
      AC_MSG_ERROR([The libcurl-devel were not found. Please install it.
                    On Debian: sudo apt-get install libcurl4-openssl-dev 
                    On Redhat: sudo yum install libcurl-devel])
  fi

  PHP_NEW_EXTENSION(plant, plant.c, $ext_shared)
fi
