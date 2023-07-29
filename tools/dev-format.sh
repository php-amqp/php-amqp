#!/usr/bin/env sh
set -e
base_dir=`dirname $0`/../
real_base_dir=`realpath $base_dir`
clang-format-17 -i $real_base_dir/*.c $real_base_dir/*.h