<?php

require __DIR__ . '/../tests/stub_server.inc';

$hostname = 'localhost';
$port     = '5678';

// stub_cli_server_start($hostname, $port,  'stub_route_label.inc');
stub_cli_server_start($hostname, $port,  'stub_agian_curl.inc');
// stub_cli_server_start($hostname, '5679', 'stub_response.php');

$url      = 'http://' . $hostname . ':' . $port;
$trace_id = 'xe-tag-001';

$headers = [
    "Xe-Tag: $trace_id",
];

$ch = curl_init($url);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);
curl_close($ch);

// var_dump($data);