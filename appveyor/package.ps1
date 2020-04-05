$ts_part = ''
if ('0' -eq $env:TS) { $ts_part = '-nts' }
$arch_part = ''
if ('x64' -eq $env:ARCH) { $arch_part = '-x86_64' }
if ($env:APPVEYOR_REPO_TAG -eq "true") {
    $bname = 'php_amqp-' + $env:APPVEYOR_REPO_TAG_NAME + '-' + $env:PHP_VER.substring(0, 3) + '-' + $env:VC + $ts_part + $arch_part
} else {
    #$bname = 'php_amqp-' + $env:APPVEYOR_REPO_COMMIT.substring(0, 8) + '-' + $env:PHP_VER.substring(0, 3) + '-' + $env:VC + $ts_part + $arch_part
    $bname = 'php_amqp-' + $env:PHP_VER.substring(0, 3) + '-' + $env:VC + $ts_part + $arch_part
}
$zip_bname = $bname + '.zip'
$dll_bname = $bname + '.dll'
$dir = 'c:\projects\amqp\';
if ('x64' -eq $env:ARCH) { $dir = $dir + 'x64\' }
$dir = $dir + 'Release'
if ('1' -eq $env:TS) { $dir = $dir + '_TS' }
cp $dir\php_amqp.dll $dir\$dll_bname
& 7z a c:\$zip_bname $dir\$dll_bname $dir\php_amqp.pdb c:\projects\amqp\LICENSE
Push-AppveyorArtifact c:\$zip_bname
