#!/bin/bash

echo Provisioning...
sudo apt-get update

# Make sure these tools installed
sudo apt-get install -y git curl pkgconf

# Add PPA with fresh PHP:
sudo add-apt-repository -y ppa:ondrej/php5-5.6
sudo apt-get update

# Install available php from packages
sudo apt-get install -y php5 php5-cli php5-dev php5-fpm

# Configure php-fpm
sudo cp ~/php-amqp/provision/php/www.conf /etc/php5/fpm/pool.d/www.conf
sudo service php5-fpm restart

# Install phpbrew to manage php versions

curl -L -O -s https://github.com/phpbrew/phpbrew/raw/master/phpbrew
chmod +x phpbrew
sudo mv phpbrew /usr/bin/phpbrew
phpbrew init

cp ~/php-amqp/provision/.bashrc ~/.bashrc

sudo mkdir -p /var/www/html/
sudo chown -R vagrant:vagrant /var/www

# Requirements to build php from sources
sudo apt-get install -y libxml2-dev \
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
                        valgrind \
                        tshark

# Benchmarking...
sudo apt-get install -y apache2-utils
# For Apache-based installation
sudo apt-get install -y apache2 libapache2-mod-php5

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

#http://www.rabbitmq.com/releases/rabbitmq-server/v3.5.1/rabbitmq-server_3.5.1-1_all.deb
# Install and configure RabbitMQ
wget -qO - http://www.rabbitmq.com/rabbitmq-signing-key-public.asc | sudo apt-key add -
sudo add-apt-repository 'deb http://www.rabbitmq.com/debian/ testing main'
sudo apt-get update
#sudo apt-get install --only-upgrade -y rabbitmq-server
sudo apt-get install -y rabbitmq-server
sudo rabbitmq-plugins enable rabbitmq_management
sudo service rabbitmq-server restart

# Note: it may be good idea to checkout latest stable rabbitmq-c version, but master branch for dev reasons also good
cd ~
git clone -q git://github.com/alanxz/rabbitmq-c.git
cd rabbitmq-c && autoreconf -i && ./configure && make && sudo make install
# or install packaged version:
#sudo apt-get install -y librabbitmq1 librabbitmq-dev librabbitmq-dbg

# Do it manually when you need it,
#cd ~/php-amqp
#phpize --clean && phpize && ./configure && sudo make install
#sudo cp ~/php-amqp/provision/php/amqp.ini /etc/php5/mods-available/
#sudo php5enmod amqp
#sudo service php5-fpm restart

# For debugging segfault when amqp fails in php-fpm mode:
#sudo sh -c "echo '/home/vagrant/php-amqp/coredump-%e.%p' > /proc/sys/kernel/core_pattern"

# To test with typical dev configuration - with xdebug:
#sudo apt-get install php5-xdebug

# Cleanup unused stuff
sudo apt-get autoremove -y

# At this point it is good idea to do `phpbrew install 5.6` (or other version you want to test extension with)
# and `phpbrew ext install ~/php-amqp/`

date > /home/vagrant/vagrant_provisioned_at
