# syntax=docker/dockerfile:1.4
FROM base AS common
RUN echo 'PATH=$PATH:/src/infra/tools' >> /etc/bash.bashrc
COPY --from=composer /usr/bin/composer /usr/local/bin/composer
CMD PHP_AMQP_DOCKER_ENTRYPOINT=1 php /src/infra/tools/pamqp-docker-setup && sleep infinity

# Install packages on Debian flavored distributions
FROM common AS debianish
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -yqq
RUN apt-get install -yqq software-properties-common curl
RUN . /etc/os-release && \
    curl https://apt.llvm.org/llvm-snapshot.gpg.key > /etc/apt/trusted.gpg.d/llvm-snapshot.asc  && \
    echo "deb https://apt.llvm.org/$VERSION_CODENAME llvm-toolchain-$VERSION_CODENAME-17 main" > /etc/apt/sources.list.d/llvm.list
RUN apt-get update -yqq
RUN apt-get install -yqq build-essential librabbitmq-dev
RUN apt-get install -yqq gdb valgrind clang clang-format-17
RUN apt-get install -yqq git unzip

# Install PHP on Ubuntu
FROM debianish AS ubuntu-php
RUN apt-get install -yqq software-properties-common
RUN add-apt-repository ppa:ondrej/php
RUN sed -e "s:main:main main/debug:" -i /etc/apt/sources.list.d/ondrej-*.list
RUN apt-get update -yqq
ARG php_pkg
RUN apt-get install -yqq ${php_pkg}-dev ${php_pkg}-common-dbgsym ${php_pkg}-cli-dbgsym
