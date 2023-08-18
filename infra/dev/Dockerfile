# syntax=docker/dockerfile:1.4
FROM base AS common
RUN echo 'PATH=$PATH:/src/infra/tools' | tee -a /etc/bashrc /etc/bash.bashrc
COPY --from=composer /usr/bin/composer /usr/local/bin/composer
CMD PHP_AMQP_DOCKER_ENTRYPOINT=1 php /src/infra/tools/pamqp-docker-setup && sleep infinity

FROM common AS redhatish
RUN dnf install -y 'dnf-command(config-manager)'
RUN dnf config-manager --set-enabled crb
RUN dnf install -y gcc clang libtool pkg-config autoconf gdb valgrind git unzip librabbitmq-devel
# FIXME: clang-format 17.x not available yet
RUN dnf install -y https://rpms.remirepo.net/enterprise/remi-release-9.rpm
ARG php_version
RUN dnf module install -y php:remi-${php_version}
RUN dnf install -y php-devel

# Install packages on Debian flavored distributions
FROM common AS debianish
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -yqq
RUN apt-get install -yqq software-properties-common curl
RUN . /etc/os-release && \
    curl https://apt.llvm.org/llvm-snapshot.gpg.key > /etc/apt/trusted.gpg.d/llvm-snapshot.asc  && \
    echo "deb https://apt.llvm.org/$VERSION_CODENAME llvm-toolchain-$VERSION_CODENAME-17 main" > /etc/apt/sources.list.d/llvm.list
RUN apt-get update -yqq
RUN apt-get install -yqq gcc clang libtool pkg-config autoconf gdb valgrind librabbitmq-dev git unzip
RUN apt-get install -yqq clang-format-17

# Install PHP on Ubuntu
FROM debianish AS ubuntu-php
RUN apt-get install -yqq software-properties-common
RUN add-apt-repository ppa:ondrej/php
RUN sed -e "s:main:main main/debug:" -i /etc/apt/sources.list.d/ondrej-*.list
RUN apt-get update -yqq
ARG php_version
RUN apt-get install -yqq php${php_version}-dev php${php_version}-common-dbgsym php${php_version}-cli-dbgsym
