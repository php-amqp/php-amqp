cmake_minimum_required(VERSION 3.1)
project(php_amqp C)

# NOTE: This CMake file is just for syntax highlighting & code navigation in CLion

set(CMAKE_C_STANDARD 99)

#update the following two paths accordingly
set(PHP_SRC_PATH /usr/local/include/php)
set(LIBRABBITMQ_SRC_PATH /src/rabbitmq-c)

INCLUDE_DIRECTORIES(BEFORE
        .
        /usr/local/include
        ${PHP_SRC_PATH}
        ${PHP_SRC_PATH}/main
        ${PHP_SRC_PATH}/Zend
        ${PHP_SRC_PATH}/TSRM
        ${LIBRABBITMQ_SRC_PATH}/librabbitmq
        )

file(GLOB_RECURSE SRC_LIST FOLLOW_SYMLINKS *.c)
file(GLOB_RECURSE HEADER_LIST FOLLOW_SYMLINKS *.h)

add_executable(php_amqp ${SRC_LIST} ${HEADER_LIST})

