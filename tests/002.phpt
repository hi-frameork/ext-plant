--TEST--
检查扩展默认 route_label 配置值

--SKIPIF--
<?php if (!ini_get('plant.route_label')) print 'skip'; ?>

--FILE--
<?php
echo ini_get('plant.route_label');
?>
--EXPECT--
Xe-Tag
