#!/bin/bash

echo Provisioning...
sudo apt-get update
sudo apt-get -y autoremove

# Make sure these tools installed
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y git htop curl tshark pkgconf
# Add PPA with fresh PHP 5:
sudo add-apt-repository -u -y ppa:ondrej/php

# Install available php from packages
sudo apt-get install -y php7.0 php7.0-cli php7.0-dev php7.0-fpm

sudo cp ~/php-amqp/provision/php/www.conf /etc/php/7.0/fpm/pool.d/www.conf
sudo service php7.0-fpm restart

# Install phpbrew to manage php versions

curl -L -O -s https://github.com/phpbrew/phpbrew/raw/master/phpbrew
chmod +x phpbrew
sudo mv phpbrew /usr/bin/phpbrew
phpbrew init

cp ~/php-amqp/provision/.bashrc ~/.bashrc

sudo mkdir -p /var/www/html/
sudo chown -R vagrant:vagrant /var/www

# Requirements to build php from sources
sudo apt-get install -y \
    libxml2-dev \
    libcurl4-openssl-dev \
    libjpeg-dev \
    libpng-dev \
    libxpm-dev \
    libmcrypt-dev \
    libmysqlclient-dev \
    libpq-dev \
    libicu-dev \
    libfreetype6-dev \
    libldap2-dev \
    libxslt-dev \
    libbz2-dev \
    libreadline-dev \
    autoconf \
    libtool \
    pkg-config \
    valgrind

# Benchmarking...
sudo apt-get install -y apache2-utils
# For Apache-based installation
sudo apt-get install -y apache2 libapache2-mod-php7.0

# Move Apache to port 8080
sudo cp ~/php-amqp/provision/apache/000-default.conf /etc/apache2/sites-available/000-default.conf
sudo cp ~/php-amqp/provision/apache/ports.conf /etc/apache2/ports.conf
sudo service apache2 restart

sudo cp -f /var/www/html/index.html /var/www/html/index-apache.html

sudo bash -c "echo '<?php phpinfo();' > /var/www/html/index.php"

# For Nginx-based installation
sudo apt-get install -y nginx
sudo cp ~/php-amqp/provision/nginx/default /etc/nginx/sites-available/default
sudo service nginx restart
sudo cp -f /usr/share/nginx/html/index.html /var/www/html/index-nginx.html

# Install and configure RabbitMQ
wget -qO - https://www.rabbitmq.com/rabbitmq-release-signing-key.asc | sudo apt-key add -
sudo add-apt-repository 'deb http://www.rabbitmq.com/debian/ testing main'
sudo apt-get update
#sudo apt-get install --only-upgrade -y rabbitmq-server
sudo apt-get install -y rabbitmq-server
sudo rabbitmq-plugins enable rabbitmq_management
sudo cp ~/php-amqp/provision/rabbitmq.config /etc/rabbitmq/
sudo service rabbitmq-server restart

# Note: it may be good idea to checkout latest stable rabbitmq-c version, but master branch for dev reasons also good
cd ~
git clone -q git://github.com/alanxz/rabbitmq-c.git
cd rabbitmq-c
sudo apt-get install -y cmake
mkdir build && cd build
cmake ..
sudo cmake --build . --target install
# or install packaged version:
#sudo apt-get install -y librabbitmq1 librabbitmq-dev librabbitmq-dbg

# Do it manually when you need it,
#cd ~/php-amqp
#phpize --clean && phpize && ./configure && sudo make install
#sudo cp ~/php-amqp/provision/php/amqp.ini /etc/php/7.0/mods-available/
#sudo phpenmod amqp
#sudo service php7.0-fpm restart

# For debugging segfault when amqp fails in php-fpm mode:
#sudo sh -c "echo '/home/vagrant/php-amqp/coredump-%e.%p' > /proc/sys/kernel/core_pattern"

# To test with typical dev configuration - with xdebug:
#sudo apt-get install -y php5-xdebug

# Cleanup unused stuff
sudo apt-get autoremove -y

# At this point it is good idea to do `phpbrew install 5.6` (or other version you want to test extension with)
# and `phpbrew ext install ~/php-amqp/`

date > /home/vagrant/vagrant_provisioned_at
