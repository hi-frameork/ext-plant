--TEST--
检查自定义 route_label 配置值

--SKIPIF--
<?php if (!ini_get('plant.route_label')) print 'skip'; ?>

--INI--
plant.route_label = Xe-Custom-Header
--FILE--
<?php
echo ini_get('plant.route_label');
?>
--EXPECT--
Xe-Custom-Header