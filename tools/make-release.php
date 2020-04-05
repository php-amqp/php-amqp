<?php
namespace phpamqp;

use DateTimeImmutable;

require_once __DIR__ . '/functions.php';

$nextVersion = $_SERVER['argv'][1];

assert(preg_match(re(VERSION_REGEX), $nextVersion));

gitFetch();
setPackageVersion($nextVersion);
setSourceVersion($nextVersion);
setChangelog(buildChangelog(versionToTag($nextVersion), versionToTag(getPreviousVersion())));
setDate(new DateTimeImmutable('NOW'));
setStability($nextVersion);
updateFiles();
savePackageXml();
validatePackage();
printf("1) Check changelog in package.xml, edit and 'git commit --amend' as needed\n");
peclPackage(2, $nextVersion);
gitCommit(3, $nextVersion, 'releasing version');
gitTag(4, $nextVersion);
