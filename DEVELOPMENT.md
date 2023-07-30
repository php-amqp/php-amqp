# PHP AMQP extension development

PHP AMQP comes with a Docker based development environment that offers containers with multiple PHP versions as well
as a properly configured RabbitMQ instance. The development environment supports building the library against multiple
PHP versions in parallel.

## Getting started

To start the development environment, run this command:

```commandline
docker compose up
```

The command will start the development environment in the foreground. This is quite nice as it will conveniently surface
all the logs, especially the RabbitMQ logs.

## The environment

Four different development containers are provided at the moment:

- `debian-74`: PHP 7.4 on Debian 11 ("Bullseye")
- `debian-80`: PHP 8.0 on Debian 12 ("Bookworm")
- `debian-81`: PHP 8.1 on Debian 12 ("Bookworm")
- `debian-82`: PHP 8.1 on Debian 12 ("Bookworm")

To enter the container, run this command:

```
docker compose exec debian-82 bash
```

You will automatically land in `/src/build/debian-82`, which is the container specific build dir. The whole source
tree is mounted to `/src`.

### Building the extension

To build the extension, a helper tool is available:

```commandline
pamqp-build
```

Once you have made a change, run `pamqp-build` again. `pamqp-build` will try and only build what is necessary. If you
want to build from scratch, run `pamqp-build`.

🧠 The few development tools available all start with `pamqp-`, so typing `pamqp` and then hitting <kbd>Tab</kbd>
gives you a list of tools available.

### Running tests

Once you have built the extension it’s a good idea to make sure that all tests are passing before you make your change.

Run this command:

```commandline
make test
```

To inspect a test failure, you can view the result file relative from the container build directory, as there is a
symlink available to make that as simple as possible:

```commandline
cat tests/amqp_some_test.out
```

### Formatting

Discussing coding style is as much as it is futile, so PHP AMQP using clang-format to automatically format the source
code. Run this command to format:

```commandline
pamqp-format
```

### Validating stubs

When PHP AMQP’s API changes, it is very important that reflection info is correct and in sync with the stubs. For that
reason `pamqp-stubs-validate` exists to validate that the two sources of information are in sync.

```commandline
pamqp-stubs-validate
```

# Release management

Say we want to release "1.10.11" next. We first run `./infra/tools/pamqp-release-cut 1.10.11`. This will update the
version numbers and pre-populate the changelog with the latest git commits between the previous version and now. It will
prompt you to edit the changelog. Once the release is done it tells you what to do next.
Run `./infra/tools/pamqp-release-finalize 1.10.12dev` to bring latest back into development mode afterward.
