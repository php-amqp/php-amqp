<?php
namespace phpamqp;

use DateTimeImmutable;

require_once __DIR__ . '/functions.php';

$nextVersion = $_SERVER['argv'][1];

assert(preg_match(re(VERSION_REGEX), $nextVersion));

gitFetch();
setPackageVersion($nextVersion);
setSourceVersion($nextVersion);
setChangelog(buildChangelog($nextVersion, getPreviousVersion()));
setDate(new DateTimeImmutable('NOW'));
updateFiles();
savePackageXml();
validatePackage();
peclPackage(1, $nextVersion);
gitCommit(2, $nextVersion);
gitTag(3, $nextVersion);
