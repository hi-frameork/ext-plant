#!/bin/env bash

#----------------------------------------------------------------
#
# 此脚本文件为生成当前扩展开发环境而编写
# 通过该脚本为 vsocde 创建开发环境
#
# 包括：
#   根据当前分支下载 PHP 源码
#   编译 PHP 源码（Compilation）
#
# created_at: 2022-03-29
#
#----------------------------------------------------------------

BASE_DIR=`pwd`
PHP_PKG_TAR="${BASE_DIR}/php-pkg.tar.xz"  # PHP 压缩包保存地址
PHP_SRC_DIR="${BASE_DIR}/php-src"         # PHP 压缩包解压路径
PHP_BUILD_DIR="${BASE_DIR}/.vscode/php"   # PHP 编译安装目录

# PHP 版本与对应的下载地址
declare -A versions
versions['7.0']='https://www.php.net/distributions/php-7.0.33.tar.xz'
versions['5.6']='https://www.php.net/distributions/php-5.6.40.tar.xz'
versions['7.1']='https://www.php.net/distributions/php-7.1.33.tar.xz'
versions['7.2']='https://www.php.net/distributions/php-7.2.34.tar.xz'
versions['7.3']='https://www.php.net/distributions/php-7.3.33.tar.xz'
versions['7.4']='https://www.php.net/distributions/php-7.4.27.tar.xz'


echo '请输入 PHP 版本(例如7.0)：'
read -p '> ' php_version
echo '你输入内容为：'${php_version}

download_url=${versions[$php_version]}
if [ -z ${download_url} ]; then
  exit 0
fi

if [ "$(command -v curl)" ]; then
  curl -L ${download_url} -o ${PHP_PKG_TAR}
elif [[ condition ]]; then
  wget ${download_url} -O ${PHP_PKG_TAR}
fi

if [[ ! -f ${PHP_PKG_TAR} ]]; then
  echo "文件${PHP_PKG_TAR}未找到，请检查 download_url"
  exit 0
fi

# 删除 PHP 源码目录，重新解压
rm -rf ${PHP_BUILD_DIR}
rm -rf ${PHP_SRC_DIR}
mkdir ${PHP_SRC_DIR}
tar -xf ${PHP_PKG_TAR} --strip-components 1 -C ${PHP_SRC_DIR}

# 最小依赖构建(当前需求仅需要 hook curl 扩展即可)
cd ${PHP_SRC_DIR}
./configure --prefix=${PHP_BUILD_DIR} --disable-all --enable-debug --with-curl
make -j$(nproc) && make install
