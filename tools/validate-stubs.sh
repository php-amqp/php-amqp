#!/usr/bin/env sh
set -e

php -n tools/compare-stubs.php stubs.json
php -n -dextension=modules/amqp.so tools/compare-stubs.php impl.json

diff -10 -u stubs.json impl.json