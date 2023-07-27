#!/usr/bin/env sh

result=0
for stub in `dirname $0`/../stubs/*.php; do
  php -l `realpath $stub`
  result=$(($result+$?))
done

[ $result -gt 0 ] && exit 1