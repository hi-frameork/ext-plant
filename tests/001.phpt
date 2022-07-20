--TEST--
检查 Plant 扩展是否存在

--SKIPIF--
<?php if (!extension_loaded("plant")) print "skip"; ?>

--FILE--
<?php 
echo "plant extension is available";
?>
--EXPECT--
plant extension is available
