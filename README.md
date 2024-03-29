# PHP AMQP bindings [![Test](https://github.com/php-amqp/php-amqp/actions/workflows/test.yaml/badge.svg)](https://github.com/php-amqp/php-amqp/actions/workflows/test.yaml)

Object-oriented PHP bindings for the AMQP C library (https://github.com/alanxz/rabbitmq-c)

### Requirements:

- PHP >= 7.4 with either ZTS or non-ZTS version.
- Starting with php-amqp 2.0.0 at least version 0.8.0 of [librabbitmq](https://github.com/alanxz/rabbitmq-c) is
  required (at least 0.10.0 is recommended)
- to run tests [RabbitMQ server](https://www.rabbitmq.com/) >= 3.4.0 required.

### Installation

#### Linux

Some systems have php-amqp extension available in their package repositories or available via an external repository so it is MAY be the preferable
way to install.

Fresh `php-pecl-amqp` and `librabbitmq` RPMs available via [remi repo](http://rpms.remirepo.net/).

If you want to stay on the bleeding edge and have the latest version, install php-amqp extension from
[PECL](http://pecl.php.net/package/amqp) or compile from sources
(follow [PHP official docs instruction](http://us1.php.net/manual/en/install.pecl.phpize.php)).

#### Windows

- Before download, check if your PHP installation is thread safe or non-thread safe by entering <kbd>php -i|findstr "
  Thread"</kbd> in your terminal
- Download thread safe or non-thread safe version of the extension for your PHP version
  from https://pecl.php.net/package/amqp. Look for the "DLL" link next to each release in the list of available releases
- After download, copy the `rabbitmq.4.dll` and `rabbitmq.4.pdb` files to the PHP root folder and copy `php_amqp.dll`
  and `php_amqp.pdb` files to `PHP\ext` folder
- Add `extension=amqp` to the `php.ini` file
- Check if the module is properly installed with <kbd>php -m</kbd>

### Documentation

Check out the official [RabbitMQ tutorials](http://www.rabbitmq.com/getstarted.html)
as well as the [php-amqp specific examples](https://github.com/rabbitmq/rabbitmq-tutorials/tree/main/php-amqp).

There are also [stub files](https://github.com/php-amqp/php-amqp/tree/latest/stubs) available that document the API of
PHP AMQP. These stubs can also be used in your IDE for code completion, navigation and documentation.

Check out the [upgrading guide](https://github.com/php-amqp/php-amqp/tree/latest/UPGRADING.md) to check
breaking changes between versions, e.g. from 1.x to 2.x.

Finally, check out the [tests](https://github.com/php-amqp/php-amqp/tree/latest/tests) to see usage examples and edge
cases.

### Notes & limitations

- You can't share any of AMQP API objects (`AMQPConnection`, `AMQPChannel`, `AMQPQueue`, `AMQPExchange`)
  between threads. Use a separate connection per thread.
- There may only be one persistent connection
  per [connection information](https://github.com/search?q=repo%3Aphp-amqp%2Fphp-amqp+amqp_conn_res_h&type=code).
  If there will be an attempt to create another persistent connection with the same credentials, an exception will be
  thrown.
- Channels on persistent connections are not persistent: they are destroyed between requests.
- Heartbeats are limited to blocking calls only, so if there are no any operations on a connection or no active
  consumer set, connection may be closed by the broker as dead.

### Related libraries

- [enqueue/amqp-ext](https://github.com/php-enqueue/amqp-ext) is
  an [amqp interop](https://github.com/queue-interop/queue-interop#amqp-interop) compatible wrapper
- [symfony/amqp-messenger](https://symfony.com/components/AMQP%20Messenger) provides AMQP integration
  for [symfony/messenger](https://symfony.com/doc/current/messenger.html), the popular messaging component by Symfony.

### How to report a problem

1. First, search through the closed issues and [stackoverflow.com](http://stackoverflow.com).
2. Submit an issue with short and definitive title that describe your problem
3. Provide platform info, PHP interpreter version, SAPI mode (cli, fpm, cgi, etc) the extension is used in, php-amqp
   extension version, librabbitmq version, make tools version.
4. Description should provide information on how to reproduce a problem ([gist](https://gist.github.com/) is the most
   preferable way to include large sources) in a definitive way. Use [Vagrant](http://www.vagrantup.com/) to replicate
   unusual environments.
5. If stack trace is generated, include it in full via [gist](https://gist.github.com/) or the important part (if you
   definitely know what you are doing) directly in the description.

#### Things to check before reporting a problem

Some of them, the list is not complete.

1. You are running on correct machine in correct environment and your platform meets your application requirement.
2. librabbitmq is installed and discoverable in your environment so php-amqp extension can load it.
3. php-amqp extension present in system (find `amqp.so` or `amqp.dll` if you are on windows), it is
   loaded (`php --ri amqp` produced some info), and there are no underlying abstraction that MAY emulate php-amqp work.
4. You hav correct RabbitMQ credentials.
5. You are using the latest php-amqp, librabbitmq, RabbitMQ and sometimes PHP version itself. Sometimes your problem is
   already solved.
6. Other extensions disabled (especially useful when PHP interpreter crashes, and you get a stack trace and segmentation
   fault).

## Development

See [DEVELOPMENT.md](DEVELOPMENT.md) for details.
