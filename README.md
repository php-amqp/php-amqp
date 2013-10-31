# PHP AMQP bindings

A fork of the official bindings here: https://github.com/alanxz/rabbitmq-c.

This fork adds two classes

AMQPConsumer and AMQPConsumerDispatcher

which allow, when using multiple connections, a script to consume
from multiple queues at the same time.

The code uses select() for efficiency.
