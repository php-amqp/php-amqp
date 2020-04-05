$ErrorActionPreference = "Stop"

$ts_part = ''
if ('0' -eq $env:TS) { $ts_part = '-nts' }
$bname = 'php-devel-pack-' + $env:PHP_VER + $ts_part + '-Win32-' + $env:VC.toUpper() + '-' + $env:ARCH + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "http://windows.php.net/downloads/releases/archives/$bname" -OutFile "c:\build-cache\$bname"
    if (-not (Test-Path c:\build-cache\$bname)) {
        Invoke-WebRequest "http://windows.php.net/downloads/releases/$bname" -OutFile "c:\build-cache\$bname"
    }
}
$dname0 = 'php-' + $env:PHP_VER + '-devel-' + $env:VC.toUpper() + '-' + $env:ARCH
$dname1 = 'php-' + $env:PHP_VER + $ts_part + '-devel-' + $env:VC.toUpper() + '-' + $env:ARCH
if (-not (Test-Path c:\build-cache\$dname1)) {
    7z x c:\build-cache\$bname -oc:\build-cache
    if ($dname0 -ne $dname1) {
        move c:\build-cache\$dname0 c:\build-cache\$dname1
    }
}
$env:PATH = 'c:\build-cache\' + $dname1 + ';' + $env:PATH
$bname = 'librabbitmq-0.9.0-' + $env:VC.toUpper() + '-' + $env:ARCH + '.zip'
if (-not (Test-Path c:\build-cache\$bname)) {
    Invoke-WebRequest "http://windows.php.net/downloads/pecl/deps/$bname" -OutFile "c:\build-cache\$bname"
    7z x c:\build-cache\$bname -oc:\build-cache\deps
}
cd c:\projects\amqp
#echo "@echo off" | Out-File -Encoding "ASCII" task.bat
#echo "" | Out-File -Encoding "ASCII" -Append task.bat
echo "" | Out-File -Encoding "ASCII" task.bat
# echo "call phpsdk_deps -d c:\build-cache\deps -c" | Out-File -Encoding "ASCII" -Append task.bat
# echo "if errorlevel 7 call phpsdk_deps -d c:\build-cache\deps -un" | Out-File -Encoding "ASCII" -Append task.bat
echo "call phpize 2>&1" | Out-File -Encoding "ASCII" -Append task.bat
echo "call configure --with-php-build=c:\build-cache\deps --with-amqp --enable-debug-pack 2>&1" | Out-File -Encoding "ASCII" -Append task.bat
echo "nmake /nologo 2>&1" | Out-File -Encoding "ASCII" -Append task.bat
echo "exit %errorlevel%" | Out-File -Encoding "ASCII" -Append task.bat
$here = (Get-Item -Path "." -Verbose).FullName
$runner = 'c:\build-cache\php-sdk-' + $env:BIN_SDK_VER + '\phpsdk' + '-' + $env:VC + '-' + $env:ARCH + '.bat'
$task = $here + '\task.bat'
& $runner -t $task
