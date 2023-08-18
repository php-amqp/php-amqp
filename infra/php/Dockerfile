# syntax=docker/dockerfile:1.4
FROM php-core AS dev
RUN . /etc/os-release && \
    echo "deb https://apt.llvm.org/$VERSION_CODENAME llvm-toolchain-$VERSION_CODENAME-17 main" > /etc/apt/sources.list.d/llvm.list && \
    curl https://apt.llvm.org/llvm-snapshot.gpg.key > /etc/apt/trusted.gpg.d/llvm-snapshot.asc
RUN apt-get update -yqq
RUN apt-get install -yqq build-essential librabbitmq-dev
RUN apt-get install -yqq gdb valgrind clang clang-format-17
RUN apt-get install -yqq git unzip
RUN echo 'PATH=$PATH:/src/infra/tools' >> /etc/bash.bashrc
COPY --from=composer /usr/bin/composer /usr/local/bin/composer
CMD PHP_AMQP_DOCKER_ENTRYPOINT=1 php /src/infra/tools/pamqp-docker-setup && sleep infinity
