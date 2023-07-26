#!/usr/bin/env sh
set -e
phpize
CFLAGS="-D_FORTIFY_SOURCE=2 -fstack-protector-strong -Wall -Werror" ./configure
make clean all