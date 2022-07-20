<?php

function truncate_tmp_log()
{
    $fp = fopen(LOG_DIR, "r+");
    ftruncate($fp, 0);
    fclose($fp);
}

function stub_nc_server($port)
{
    $host = 'localhost';
    $address =  $host . ":" . $port;

	$descriptorspec = [
		0 => STDIN,
		1 => STDOUT,
		2 => STDERR,
	];
	$doc_root = __DIR__;

    $cmd = "exec netcat -kl -p $port -c 'xargs -n1 echo >>" . NC_LOG .  "'";
	$cmd .= " 2>/dev/null";
	$handle = proc_open($cmd, $descriptorspec, $pipes, $doc_root);
    sleep(1);
	register_shutdown_function(
		function($handle) {
			proc_terminate($handle);
		},
		$handle
	);
}

function stub_cli_server_start($hostname, $port, $router)
{
	$base_dir       = realpath(__DIR__ . '/../');
    $address 		= $hostname . ":" . $port;
	$php_executable = "{$base_dir}/.vscode/php/bin/php";
	$doc_root 		= __DIR__;
	$router 		= $router ? $router : "stub_response.inc";

	$descriptorspec = [
		0 => STDIN,
		1 => STDOUT,
		2 => STDERR,
	];

	$cmd     = "exec {$php_executable} -t {$doc_root} -n -d extension_dir={$base_dir}/modules/ -d extension=plant.so -d plant.enable=1 -S " . $address;
	$cmd    .= " {$base_dir}/tests/{$router}";
	$cmd    .= " 2>/dev/null";
	// var_dump($cmd);

	$handle  = proc_open($cmd, $descriptorspec, $pipes, $doc_root);

    $i = 0;
    while (($i++ < 30) && !($fp = @fsockopen($hostname, $port))) {
        usleep(10000);
    }

    if ($fp) {
        fclose($fp);
    }

	register_shutdown_function(
		function($handle) use($router) {
			proc_terminate($handle);
		},
		$handle
	);

    return $address;
}
