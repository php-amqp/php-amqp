#!/usr/bin/env sh
set -e

php tools/compare-stubs.php stubs.json
php -dextension=modules/amqp.so tools/compare-stubs.php impl.json

diff -10 -u stubs.json impl.json