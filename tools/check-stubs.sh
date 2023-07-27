#!/usr/bin/env sh

result=0
for stub in `dirname $0`/../stubs/*.php; do
  php -l `realpath $stub`
  result=$(($result+$?))
done

test $result -gt 0 && exit 1 || exit 0