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
preg_match('/Version\s*=>\s*([0-9.a-z-]+)/m', $info, $matches);
$srcVersion = $matches[1];

if (0 === strcmp($packageVersion, $srcVersion)) {
    echo "package.xml matches phpinfo() output\n";
} else {
    printf("src version: %s, package2.xml: %s\n", $srcVersion, $packageVersion);
}
if (0 === strcmp($packageVersion, $ext->getVersion())) {
	echo "package.xml matches extension version\n";
} else {
	printf("ext version: %s, package2.xml %s\n", $ext->getVersion(), $packageVersion);
}
--EXPECT--
package.xml matches phpinfo() output
package.xml matches extension version
