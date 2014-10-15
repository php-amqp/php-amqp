# PHP AMQP bindings [![Build Status](https://secure.travis-ci.org/pdezwart/php-amqp.png)](http://travis-ci.org/pdezwart/php-amqp)

Object-oriented PHP bindings for the AMQP C library (https://github.com/alanxz/rabbitmq-c)


### Requirements:

 - [RabbitMQ C library](https://github.com/alanxz/rabbitmq-c), commonly known as librabbitmq (since php-amqp>=1.3.0  rabbitmqp >= 0.4.1 required, see [release note](http://pecl.php.net/package/amqp/1.3.0)).

### Installation
 
 Some systems has php-amqp extension in their repo or available via external repositories, so it is MAY be the preferable way to install.
 
 If you want to stay on the bleeding edge and have the latest version, install it from [PECL](http://pecl.php.net/package/amqp) or compile from sources (follow [PHP official docs instruction](http://us1.php.net/manual/en/install.pecl.phpize.php)).

### Documentation

[stub files](https://github.com/pdezwart/php-amqp/tree/master/stubs) with accurate PHPDoc which may be also used in your IDE for code completion, navigation and documentation in-place.

### How to report a problem
 
 1. First, search through the closed issues and [stackoverflow.com](stackoverflow.com).
 3. Submit an issue with short and definitive title that describe your problem
 4. Provide platform info, php interpreter version, SAPI mode (cli, fpm, cgi, etc) extension is used in, php-amqp extension version, librabbitmq version, make tools version.
 5. Description should provide information how to reproduce a problem ([gist](https://gist.github.com/) is the most preferable way to include large sources) in a definitive way. When special environment [Vagrant](http://www.vagrantup.com/) may be in handy.
 6. If stack trace generated include it in full via [gist](https://gist.github.com/) or the important part (if you are definitely know what you are doing) directly in description.
 
##### Things to check before reporting a problem

 Some of them, the list is not complete.

 1. You are running on correct machine in correct environment and you platform meet your application requirement.
 2. librabbimq installed and discoverable in your environment so php-amqp extension can load it.
 3. php-amqp extension present in system (find `amqp.so` or `amqp.dll` if you are on windows), it is loaded (`php --ri amqp` produced some info), and there are no underlying abstraction that MAY emulate php-amqp work.
 5. You has correct RabbitMQ credentials.
 6. You are using the latest php-amqp, librabbitm, RabbitMQ and sometimes PHP version itself. Sometimes your problem is already solved.
 7. Other extensions disabled (especially useful when PHP interpreter crashes and you get stack trace and segmentation fault).


##### Development

 There are vagrant environment with pre-installed software and libraries necessary to build, test and run php-amqp extension.

 To start it, just type `vagrant up` and then `vagrant ssh` in php-amqp directory.

 Services available out of the box are:

 - Apache2 - on [192.168.33.10:8080](http://192.168.33.10:8080)
 - nginx - on [192.168.33.10:80](http://192.168.33.10:80)
 - RabbitMQ on [192.168.33.10:15672](http://192.168.33.10:15672/#/login/guest/guest)

 Additional tools are pre-installed to make development process as simple as possible:

 - valgrind is ready to help find memory-related problems if you `export TEST_PHP_ARGS=m` before running tests
 - [phpbrew](https://github.com/phpbrew/phpbrew) waits to help you test extension on various PHP versions.
   `phpbrew install 5.6 +debug+default+fpm` is a nice start. To switch to some version just use `phpbrew switch <version>`.

   To start php-fpm just run `phpbrew fpm start` (don't forget to run `sudo service stop php5-fpm` befor).

   This development environment out of the box ready for php-fpm and cli extension usage, if need to test it when php
   used as apache module, refer to [Apache2 support on phpbrew wiki](https://github.com/phpbrew/phpbrew/wiki/Cookbook#apache2-support).
   Keep in mind that `+apxs2` conficts with `+fpm` and it is a bit tricky to specify which libphp .so will be loaded.

##### Keeping track of the workers
 It is a good practice to keep php processes (i.e workers/consumers) under control. Usually, system administrators write their own scripts which ask services about current status or performs some desired actions. Usually request is sent via UNIX signals.<br />
 Because amqp <i>consume</i> method is blocking, pcntl extension seems to be useless.
 
 [php-signal-handler](https://github.com/RST-com-pl/php-signal-handler) extension uses <i>signal</i> syscall, so it will work even if blocking method was executed.
 Some use cases are presented on extension's github page and examples are available [here](https://github.com/pdezwart/php-amqp/pull/89).
