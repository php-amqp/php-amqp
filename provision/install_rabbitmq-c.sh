#!/bin/bash

set -e

echo Installing rabbitmq-c ...


LIBRABBITMQ_VERSION=$1

cd $HOME

if [ ! -d "$HOME/rabbitmq-c" ]; then
  git clone git://github.com/alanxz/rabbitmq-c.git
else
  echo 'Using cached directory.';
  cd $HOME/rabbitmq-c
  git fetch
fi

cd $HOME/rabbitmq-c
git checkout ${LIBRABBITMQ_VERSION}

git submodule init && git submodule update
autoreconf -i && ./configure --prefix=$HOME/rabbitmq-c && make && make install
