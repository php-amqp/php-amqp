$ErrorActionPreference = "Stop"

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
