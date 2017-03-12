<?php
$xml = simplexml_load_file('package.xml');

$sourceExpression = '*.[ch]';
$testsExpression = '*.phpt';
$stubExpression = 'stubs/*.php';

function addFiles(SimpleXMLElement $dir, $expression, $role) {
    foreach (glob($expression) as $file) {
        if ($file === 'config.h') {
            continue;
        }
        $child = $dir->addChild('file');
        $child->addAttribute('name', $file);
        $child->addAttribute('role', $role);
    }
}

function removeNodes(SimpleXMLElement $el, $expression) {
    $nodesToDelete = [];

    foreach ($el->children() as $file) {
        if (fnmatch($expression, (string) $file['name'])) {
            $nodesToDelete[] = $file;
        }
    }

    foreach ($nodesToDelete as $node) {
        $dom = dom_import_simplexml($node);
        $dom->parentNode->removeChild($dom);
    }
}

removeNodes($xml->contents->dir, $sourceExpression);
removeNodes($xml->contents->dir, $testsExpression);
removeNodes($xml->contents->dir, $stubExpression);
addFiles($xml->contents->dir, $sourceExpression, 'src');
addFiles($xml->contents->dir, 'tests/' . $testsExpression, 'test');
addFiles($xml->contents->dir, $stubExpression, 'doc');

$xml->saveXML('package.xml');

`xmlformat  --in-place package.xml`;
`xmlstarlet format --indent-spaces 4 package.xml > package.xml.new`;
rename('package.xml.new', 'package.xml');
