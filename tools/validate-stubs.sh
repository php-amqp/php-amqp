#!/usr/bin/env sh
set -e

SHARED_JSON=`php -n -r 'echo !extension_loaded("json") ? "true" : "false";'`

args="-n"
if [ "${SHARED_JSON}" = "true" ]; then
  args="${args} -d extension=json.so"
fi

php $args tools/dump-reflection.php stubs.json
php $args -d extension=modules/amqp.so tools/dump-reflection.php impl.json

diff -10 -u stubs.json impl.json