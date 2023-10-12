--TEST--
Compare version in package.xml and module
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!function_exists('simplexml_load_file')) print "skip SimpleXML extension is not loaded";
?>
--FILE--
<?php
$package = simplexml_load_file(dirname(__FILE__) . '/../package.xml');
$packageVersion = $package->version->release;

$ext = new ReflectionExtension('amqp');
$srcVersion = $ext->getVersion();

if (0 === strcmp($packageVersion, $srcVersion)) {
    echo "package.xml matches phpinfo() output\n";
} else {
    printf("src version: %s, package.xml: %s\n", $srcVersion, $packageVersion);
}
if (0 === strcmp($packageVersion, $ext->getVersion())) {
	echo "package.xml matches extension version\n";
} else {
	printf("ext version: %s, package.xml: %s\n", $ext->getVersion(), $packageVersion);
}
--EXPECT--
package.xml matches phpinfo() output
package.xml matches extension version
