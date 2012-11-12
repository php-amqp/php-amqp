--TEST--
Compare version in package2.xml and module
--SKIPIF--
<?php if (!function_exists('simplexml_load_file')) print "skip"; ?>
--FILE--
<?php
$package = simplexml_load_file(dirname(__FILE__) . '/../package2.xml');
$packageVersion = $package->version->release;

$ext = new ReflectionExtension('amqp');
ob_start();
$ext->info();
$info = ob_get_contents();
ob_end_clean();
$matches = array();
preg_match('/Version\s*=>\s*([0-9.]+)/m', $info, $matches);
$srcVersion = $matches[1];

if (0 === strcmp($packageVersion, $srcVersion)) {
    echo "versions match\n";
} else {
    printf("src version: %s, package2.xml: %s\n", $srcVersion, $packageVersion);
}
--EXPECT--
versions match
