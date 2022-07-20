--TEST--
流量标识未传入默认策略测试（不处理）

--FILE--
<?php

require __DIR__ . '/stub_server.inc';

$hostname = 'localhost';
$port     = '5678';

stub_cli_server_start($hostname, $port,  'stub_agian_curl.inc');
stub_cli_server_start($hostname, '5679', 'stub_response.inc');

$url      = 'http://' . $hostname . ':' . $port;

$ch = curl_init($url);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
curl_exec($ch);
curl_close($ch);
?>
--EXPECT--
No Xe-Tag header!