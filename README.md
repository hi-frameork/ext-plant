# 自定义 header 透传扩展

通过本扩展可以对特定配置名称的头进行无业务入侵透传，**开发动机是用于流量灰度标识透传**，可以在业务侧不改动代码前提下传递特定 header 头信息。

## 使用

各个 PHP 使用的版本分别位于对应的分支中，说明：

* 5.x 用于 PHP5.6 版本。
* 7.x 用于 PHP7.0 至 PHP7.4 版本。

**扩展 ini 配置：**

```ini
extension=plant.so
plant.enable = On
plant.route_label = Xe-Tag
```

**安装：**

```bash
# 对于 PHP5.6 版本
git clone -b 5.x git@talkcheap.xiaoeknow.com:BasedService/php-header-ext.git
cd php-header-ext
phpize
./configure
make
# install 可能需要 root 身份
make install

# 对于 PHP7.0 - PHP7.4
git clone -b 7.x git@talkcheap.xiaoeknow.com:BasedService/php-header-ext.git
cd php-header-ext
phpize
./configure
make
# install 可能需要 root 身份
make install
```