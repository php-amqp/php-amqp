# syntax=docker/dockerfile:1.4
FROM php-core AS dev

RUN set -xe; \
    apt-get update -yqq && \
    pecl channel-update pecl.php.net

RUN apt-get install -yqq build-essential
RUN apt-get install -yqq librabbitmq-dev
RUN apt-get install -yqq socat
RUN apt-get install -yqq gdb
RUN apt-get install -yqq valgrind
RUN apt-get install -yqq git
RUN apt-get install -yqq unzip

RUN echo 'PATH=$PATH:/src/infra/tools' >> /etc/bash.bashrc
RUN echo 'CFLAGS="-D_FORTIFY_SOURCE=2 -fstack-protector-strong -Wall -Werror"' >> /etc/bash.bashrc

COPY --from=composer /usr/bin/composer /usr/local/bin/composer

CMD PHP_AMQP_DOCKER_ENTRYPOINT=1 php /src/infra/tools/pamqp-docker-setup && \
    # Forward localhost:5672 to rabbitmq:5672 (Plain TCP) so that the testsuite can access RabbitMQ on localhost
    socat tcp-listen:5672,reuseaddr,fork tcp:rabbitmq:5672

FROM dev AS dev-all
RUN apt-get install -yqq gnupg2
RUN echo "deb https://apt.llvm.org/bookworm/ llvm-toolchain-bookworm main" > /etc/apt/sources.list.d/llvm.list
RUN curl https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN apt-get update -yqq
RUN apt-get install -yqq clang-format-17