# PHP AMQP bindings [![Build Status](https://travis-ci.org/php-amqp/php-amqp.svg?branch=master)](https://travis-ci.org/php-amqp/php-amqp) [![Build status](https://ci.appveyor.com/api/projects/status/sv5o1id5oj63w9hu/branch/master?svg=true)](https://ci.appveyor.com/project/lstrojny/php-amqp-7lf47/branch/master)

Object-oriented PHP bindings for the AMQP C library (https://github.com/alanxz/rabbitmq-c)


### Requirements:

 - PHP >= 5.6 (PHP 7 included) with either ZTS or non-ZTS version.
 - [RabbitMQ C library](https://github.com/alanxz/rabbitmq-c), commonly known as librabbitmq
   (since php-amqp>=1.9.4  librabbitmq >= 0.7.1,
   see [release notes](https://pecl.php.net/package-changelog.php?package=amqp)).
 - to run tests [RabbitMQ server](https://www.rabbitmq.com/) >= 3.4.0 required.


### Installation

#### Linux
 Some systems has php-amqp extension in their repo or available via external repositories, so it is MAY be the preferable
 way to install.

 RPM packages are available in Fedora and EPEL (for RHEL and CentOS) official repositories,
 see [php-pecl-amqp](https://apps.fedoraproject.org/packages/php-pecl-amqp)
 
 Fresh `php-pecl-amqp` and `librabbitmq` RPMs available via [remi repo](http://rpms.remirepo.net/).

 If you want to stay on the bleeding edge and have the latest version, install php-amqp extension from
 [PECL](http://pecl.php.net/package/amqp) or compile from sources
 (follow [PHP official docs instruction](http://us1.php.net/manual/en/install.pecl.phpize.php)).
 
#### Windows
 - Before download, check if your PHP installation is thread safe or non-thread safe by entering <kbd>php -i|findstr "Thread"</kbd> in your terminal
 - Download thread safe or non-thread safe version of the extension from https://pecl.php.net/package/amqp
 - After download, copy the `rabbitmq.4.dll` and `rabbitmq.4.pdb` files to the PHP root folder and copy `php_amqp.dll` and `php_amqp.pdb` files to `PHP\ext` folder
 - Add `extension=amqp` to the `php.ini` file
 - Check if the module is properly installed with <kbd>php -m</kbd>

### Documentation

View [RabbitMQ official tutorials](http://www.rabbitmq.com/getstarted.html) 
and [php-amqp specific examples](https://github.com/rabbitmq/rabbitmq-tutorials/tree/master/php-amqp).

There are also available [stub files](https://github.com/php-amqp/php-amqp/tree/master/stubs) with accurate PHPDoc which
may be also used in your IDE for code completion, navigation and documentation in-place.

Finally, check out the [tests](https://github.com/php-amqp/php-amqp/tree/master/tests) to see typical usage and edge cases.
 
### Notes

  - Max channels per connection means how many concurrent channels per connection may be opened at the same time
    (this limit may be increased later to max AMQP protocol number - 65532 without any problem).

  - Nested header arrays may contain only string values.
  
  - You can't share none of AMQP API objects (none of `AMQPConnection`, `AMQPChannel`, `AMQPQueue`, `AMQPExchange`) between threads.
    You have to use separate connection and so on per thread.
    
### Related libraries

* [enqueue/amqp-ext](https://github.com/php-enqueue/amqp-ext) is a [amqp interop](https://github.com/queue-interop/queue-interop#amqp-interop) compatible wrapper.

#### Persistent connection

  Limitations:

  - there may be only one persistent connection per unique credentials (login+password+host+port+vhost).
    If there will be an attempt to create another persistent connection with the same credentials, an exception will be thrown.
  - channels on persistent connections are not persistent: they are destroyed between requests.
  - heartbeats are limited to blocking calls only, so if there are no any operations on a connection or no active 
    consumer set, connection may be closed by the broker as dead.

*Developers note: alternatively for built-in persistent connection support [raphf](http://pecl.php.net/package/raphf) pecl extension may be used.*

### How to report a problem
 
 1. First, search through the closed issues and [stackoverflow.com](http://stackoverflow.com).
 3. Submit an issue with short and definitive title that describe your problem
 4. Provide platform info, PHP interpreter version, SAPI mode (cli, fpm, cgi, etc) extension is used in, php-amqp extension version, librabbitmq version, make tools version.
 5. Description should provide information on how to reproduce a problem ([gist](https://gist.github.com/) is the most preferable way to include large sources) in a definitive way. Use [Vagrant](http://www.vagrantup.com/) to replicate unusual environments.
 6. If stack trace is generated, include it in full via [gist](https://gist.github.com/) or the important part (if you definitely know what you are doing) directly in the description.
 
#### Things to check before reporting a problem

 Some of them, the list is not complete.

 1. You are running on correct machine in correct environment and your platform meets your application requirement.
 2. librabbimq is installed and discoverable in your environment so php-amqp extension can load it.
 3. php-amqp extension present in system (find `amqp.so` or `amqp.dll` if you are on windows), it is loaded (`php --ri amqp` produced some info), and there are no underlying abstraction that MAY emulate php-amqp work.
 5. You hav correct RabbitMQ credentials.
 6. You are using the latest php-amqp, librabbitmq, RabbitMQ and sometimes PHP version itself. Sometimes your problem is already solved.
 7. Other extensions disabled (especially useful when PHP interpreter crashes and you get stack trace and segmentation fault).


## Development

 There is a Vagrant environment with pre-installed software and libraries necessary to build, test and run the php-amqp extension.

 To start it, just type `vagrant up` and then `vagrant ssh` in php-amqp directory.

 Services available out of the box are:

 - Apache2 - on [192.168.33.10:8080](http://192.168.33.10:8080)
 - nginx - on [192.168.33.10:80](http://192.168.33.10:80)
 - RabbitMQ on [192.168.33.10:15672](http://192.168.33.10:15672/#/login/guest/guest) with guest access (login: `guest`, password: `guest`)

Additional tools are pre-installed to make development process as simple as possible:

##### valgrind

Valgrind is ready to help find memory-related problems if you `export TEST_PHP_ARGS=-m` before running tests.

##### phpbrew

[phpbrew](https://github.com/phpbrew/phpbrew) waits to help you to test extension on various PHP versions.
`phpbrew install 5.6 +debug+default+fpm` is a nice start. To switch to some version just use `phpbrew switch <version>`.

To start php-fpm just run `phpbrew fpm start` (don't forget to run `sudo service stop php5-fpm` befor).

This development environment out of the box ready for php-fpm and cli extension usage, if need to test it when php
used as apache module, refer to [Apache2 support on phpbrew wiki](https://github.com/phpbrew/phpbrew/wiki/Cookbook#apache2-support).
Keep in mind that `+apxs2` conficts with `+fpm` and it is a bit tricky to specify which libphp .so will be loaded.

##### tshark

> [tshark](https://www.wireshark.org/docs/man-pages/tshark.html) - Dump and analyze network traffic
>
> ... TShark is able to detect, read and write the same capture files that are supported by Wireshark.
   
To use it you probably have to set network privileges for dumpcap first(see
[Platform-Specific information about capture privileges](https://wiki.wireshark.org/CaptureSetup/CapturePrivileges) Wireshark docs page
and [running wireshark “Lua: Error during loading”](http://askubuntu.com/questions/454734/running-wireshark-lua-error-during-loading) SO question):
   
   `sudo setcap 'CAP_NET_RAW+eip CAP_NET_ADMIN+eip' /usr/bin/dumpcap` 

To start capturing, run `tshark -i lo` to see output in terminal or `tshark -i lo -w capture.log` to save capture and
analyze it later (even with [AMQP](https://wiki.wireshark.org/AMQP) protocol Wireshark plugin). You may filter AMQP packages
using `-Y amqp` attribute, just give a try - `tshark -i lo -Y amqp`.
   
> NOTE: -w provides raw packet data, not text. If you want text output you need to redirect stdout (e.g. using '>'), don't use the -w option for this.


#### Configuring a RabbitMQ server

If you need to tweek RabbitMQ server params use default config
[rabbitmq.config.example](https://github.com/rabbitmq/rabbitmq-server/blob/master/docs/rabbitmq.config.example)
([raw](https://raw.githubusercontent.com/rabbitmq/rabbitmq-server/master/docs/rabbitmq.config.example))
from [official RabbitmMQ repo](https://github.com/rabbitmq/rabbitmq-server), so it may looks like 
`sudo curl https://raw.githubusercontent.com/rabbitmq/rabbitmq-server/master/docs/rabbitmq.config.example /etc/rabbitmq/rabbitmq.config`
 
To reset RabbitMQ application run in CLI (as privileged user) `rabbitmqctl stop_app && rabbitmqctl reset && rabbitmqctl start_app`.

#### Keeping track of the workers
 It is a good practice to keep php processes (i.e workers/consumers) under control. Usually, system administrators write their own scripts which ask services about current status or performs some desired actions. Usually request is sent via UNIX signals.<br />
 Because amqp <i>consume</i> method is blocking, pcntl extension seems to be useless.
 
 [php-signal-handler](https://github.com/RST-com-pl/php-signal-handler) extension uses <i>signal</i> syscall,
 so it will work even if blocking method was executed.
 Some use cases are presented on extension's github page and examples are available [here](https://github.com/php-amqp/php-amqp/pull/89).


#### Rolling a release
Say we want to release "1.1000.0" next. We first run `php tools/make-release.php 1.1000.0`. This will update the version
numbers and pre-populate the changelog with the latest git commits between the previous version and now. It will prompt
you to edit the changelog in between. Once the release is done it tells you what to do next.
Run `php tools/make-dev.php 1.1000.1` to bring master back into development mode afterwards.
