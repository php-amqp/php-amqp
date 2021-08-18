if (-not (Test-Path c:\build-cache)) {
    mkdir c:\build-cache
}
$bname = 'php-sdk-' + $env:BIN_SDK_VER + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "https://github.com/microsoft/php-sdk-binary-tools/archive/$bname" -OutFile "c:\build-cache\$bname"
}
$dname0 = 'php-sdk-binary-tools-php-sdk-' + $env:BIN_SDK_VER
$dname1 = 'php-sdk-' + $env:BIN_SDK_VER
if (-not (Test-Path c:\build-cache\$dname1)) {
    7z x c:\build-cache\$bname -oc:\build-cache
    move c:\build-cache\$dname0 c:\build-cache\$dname1
}
$ts_part = ''
if ('0' -eq $env:TS) { $ts_part = '-nts' }
$bname = 'php-devel-pack-' + $env:PHP_VER + $ts_part + '-Win32-' + $env:VC + '-' + $env:ARCH + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "http://windows.php.net/downloads/releases/archives/$bname" -OutFile "c:\build-cache\$bname"
    if (-not (Test-Path c:\build-cache\$bname)) {
        Invoke-WebRequest "http://windows.php.net/downloads/releases/$bname" -OutFile "c:\build-cache\$bname"
    }
}
$dname0 = 'php-' + $env:PHP_VER + '-devel-' + $env:VC + '-' + $env:ARCH
$dname1 = 'php-' + $env:PHP_VER + $ts_part + '-devel-' + $env:VC + '-' + $env:ARCH
if (-not (Test-Path c:\build-cache\$dname1)) {
    7z x c:\build-cache\$bname -oc:\build-cache
    if ($dname0 -ne $dname1) {
        move c:\build-cache\$dname0 c:\build-cache\$dname1
    }
}
$env:PATH = 'c:\build-cache\' + $dname1 + ';' + $env:PATH
$bname = $env:DEP + '-' + 'vc15' + '-' + $env:ARCH + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "http://windows.php.net/downloads/pecl/deps/$bname" -OutFile "c:\build-cache\$bname"
    7z x c:\build-cache\$bname -oc:\build-cache\deps
}
