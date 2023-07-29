#!/usr/bin/env sh
set -e

SHARED_JSON=`php -n -r 'echo !extension_loaded("json") ? "true" : "false";'`

args="-n"
if [ "${SHARED_JSON}" = "true" ]; then
  args="${args} -d extension=json.so"
fi

base_dir=`dirname $0`/../
real_base_dir=`realpath $base_dir`

php $args $real_base_dir/tools/dump-reflection.php stubs.json
php $args -d extension=modules/amqp.so $real_base_dir/tools/dump-reflection.php impl.json

diff -10 -u stubs.json impl.json