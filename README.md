# PHP AMQP bindings [![Build Status](https://secure.travis-ci.org/pdezwart/php-amqp.png)](http://travis-ci.org/pdezwart/php-amqp)

Object-oriented PHP bindings for the AMQP C library (https://github.com/alanxz/rabbitmq-c)


### Requirements:

 - [RabbitMQ C library](https://github.com/alanxz/rabbitmq-c), commonly known as librabbitmq (since php-amqp>=1.3.0  rabbitmqp >= 0.4.1 required, see [release note](http://pecl.php.net/package/amqp/1.3.0)).

### Installation
 
 Some systems has php-amqp extension in their repo or available via external repositories, so it is MAY be the preferable way to install.
 
 If you want to stay on the bleeding edge and have the latest version, install it from [PECL](http://pecl.php.net/package/amqp) or compile from sources (follow [PHP official docs instruction](http://us1.php.net/manual/en/install.pecl.phpize.php)).

### Documentation

 There official docs on PHP website [here](http://www.php.net/manual/en/book.amqp.php), but it is a bit outdated (see issue [#17](https://github.com/pdezwart/php-amqp/issues/17)) but there perfect [stub files](https://github.com/pdezwart/php-amqp/tree/master/stubs) with accurate PHPDoc which may be also used in your IDE for code completion, navigation and documentation in-place.

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

##### Keeping track of the workers
 It is a good practice to keep php processes (i.e workers/consumers) under control. Usually, system administrators write their own scripts which ask services about current status or performs some desired actions. Usually request is sent via UNIX signals.<br />
 Because amqp <i>consume</i> method is blocking, pcntl extension seems to be useless.
 
 [php-signal-handler](https://github.com/RST-com-pl/php-signal-handler) extension uses <i>signal</i> syscall, so it will work even if blocking method was executed.
 Some use cases are presented on extension's github page and examples are available [here](https://github.com/pdezwart/php-amqp/pull/89).
