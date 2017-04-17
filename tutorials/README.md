
# RabbitMQ C AMQP tutorials

## Introduction

This directory holds example tutorial code that implement the six tutorials 
found at: 

 - <https://www.rabbitmq.com/getstarted.html>

#### Prereqs:
- PHP5.6.x
- librabbitmq 0.5.x
- php-amqp pecl extension 1.4.0

In order to actually execute these scripts you will need a RabbitMQ server.

The easiest way (and the method used to test these as they were created) is to use
the official RabbitMQ Docker image:

 - <https://hub.docker.com/_/rabbitmq/>

####Tested against docker docker pull rabbitmq:management
- docker run -d --hostname my-rabbitmq -p 5672:5672 -p 8080:15672 rabbitmq:management

This is a work in progress.
