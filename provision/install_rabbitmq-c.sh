#!/bin/sh

# Setup a sane environment
set -o errexit
set -o nounset

# Parse arguments
LIBRABBITMQ_VERSION="${1:-}"
if test -z "$LIBRABBITMQ_VERSION"; then
    printf 'Usage: %s LIBRABBITMQ_VERSION\n' "$0" >&2
    exit 1
fi

# Be sure that non-master versions start with 'v' (eg. from '0.8.0' to 'v0.8.0') 
if printf '%s' "$LIBRABBITMQ_VERSION" | grep -Eq '^[0-9]+\.'; then
    LIBRABBITMQ_VERSION="v$LIBRABBITMQ_VERSION"
fi

printf 'Installing rabbitmq-c version %s...\n' "$LIBRABBITMQ_VERSION"

# Let's build in /dev/shm if available (it's a ramdisk - much faster)
if test -d /dev/shm; then
    BUILD_TMP=/dev/shm/php-amqb-build
else
    BUILD_TMP=/tmp/shm/php-amqb-build
fi

# Prepare the build directory
rm -rf $BUILD_TMP
mkdir -p $BUILD_TMP

# Download and extract the source code
curl -sSLf "https://codeload.github.com/alanxz/rabbitmq-c/tar.gz/$LIBRABBITMQ_VERSION" | tar xz --directory=$BUILD_TMP

# Prepare for building
cd $BUILD_TMP/rabbitmq-c-*
mkdir -p build
cd build
cmake ..

# Compile
cmake --build . --parallel $(nproc)

# Install
sudo cmake --build . --target install

# Cleanup
rm -rf $BUILD_TMP
