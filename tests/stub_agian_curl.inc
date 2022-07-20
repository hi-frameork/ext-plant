<?php

$hostname = 'localhost';
$port     = '5679';
$url      = 'http://' . $hostname . ':' . $port;

$ch = curl_init($url);
// 如果下游过来的请求存在 Xe-Tag 将由扩展自动透传
curl_exec($ch);
curl_close($ch);