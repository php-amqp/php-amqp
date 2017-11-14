#!/bin/bash

set -e

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 LIBRABBITMQ_VERSION" >&2
  exit 1
fi

LIBRABBITMQ_VERSION=$1

echo "Installing rabbitmq-c ..."

if [ ! -d "$HOME/rabbitmq-c" ]; then
  cd $HOME
  git clone git://github.com/alanxz/rabbitmq-c.git
  cd $HOME/rabbitmq-c
else
  echo 'Using cached directory.';
  cd $HOME/rabbitmq-c
  git fetch
fi

git checkout ${LIBRABBITMQ_VERSION}

mkdir build && cd build
cmake ..
cmake --build . --target install
