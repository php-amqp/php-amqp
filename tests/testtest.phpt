--TEST--
Test the tests
--FILE--
<?php
foreach (glob(__DIR__ . '/*.phpt') as $test) {
    if ($test === __FILE__ . 't') {
        continue;
    }

    $content = file_get_contents($test);

    if (!$content) {
        printf("%s could not be read\n", basename($test));
        continue;
    }

    if (strpos($content, "\n--SKIPIF--\n") == false) {
        printf("%s does not contain SKIPIF section\n", basename($test));
        continue;
    }

    if (!preg_match('/--FILE--(?P<testCode>.*?)--[A-Z]+--/s', $content, $matches)) {
        printf("%s TEST section cannot be parsed\n", basename($test));
        continue;
    }
    ['testCode' => $testCode] = $matches;

    if (!preg_match('/--SKIPIF--(?P<skipCode>.*?)--[A-Z]+--/s', $content, $matches)) {
        printf("%s SKIPIF section cannot be parsed\n", basename($test));
        continue;
    }

    ['skipCode' => $skipCode] = $matches;

    if (!preg_match('/if\s*\(!extension_loaded\("amqp"\)\)\s*\{?\s*print "skip";/', $skipCode)) {
        printf("%s --SKIP-- does not check for the extension being present\n", basename($test));
        continue;
    }

    $hostVars = ['PHP_AMQP_HOST', 'PHP_AMQP_SSL_HOST'];
    foreach ($hostVars as $hostVar) {
        if (strpos($testCode, $hostVar) !== false && !preg_match('/!getenv\(["\']' . $hostVar . '/', $skipCode)) {
            printf("%s --TEST-- contains reference to %s but --SKIP-- does not check for it\n", basename($test), $hostVar);
            continue 2;
        }

        if (strpos($testCode, $hostVar) === false && strpos($skipCode, $hostVar) !== false) {
            printf("%s --TEST-- contains no reference to %s but --SKIP-- checks for reference\n", basename($test), $hostVar);
            continue 2;
        }
    }
}
?>
==DONE==
--EXPECT--

==DONE==