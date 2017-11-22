--TEST--
AMQPQueue - nested consumers
--SKIPIF--
<?php if (!extension_loaded("amqp")) {
    print "skip";
} ?>
--FILE--
<?php

function test(AMQPChannel $channel1)
{
    $ex1 = new AMQPExchange($channel1);
    $ex1->setName('ex1-' . microtime(true));
    $ex1->setType(AMQP_EX_TYPE_FANOUT);
    $ex1->declareExchange();

    $q1 = new AMQPQueue($channel1);
    $q1->setName('q1-' . microtime(true));
    $q1->declareQueue();
    $q1->bind($ex1->getName());

    $cnt1 = 4;
    $cnt2 = 4;
    $nested_publish = true;

    for($i=0; $i < $cnt1; $i++) {
        $ex1->publish("message 1 - {$i}");
    }

    $q1->consume(function (\AMQPEnvelope $message, \AMQPQueue $queue)  use (&$cnt1, &$cnt2, &$nested_publish) {

        $queue->ack($message->getDeliveryTag());

        printf("1: %s [%s] %s - %s (%s): %s queue\n", $message->getExchangeName(), $message->getBody(), $message->getConsumerTag(), $queue->getConsumerTag(), $queue->getName(), $message->getConsumerTag() == $queue->getConsumerTag() ? 'valid' : 'not valid');

        $channel2 = new \AMQPChannel($queue->getConnection());

        $ex2 = new AMQPExchange($channel2);
        $ex2->setName('ex2-' . microtime(true));
        $ex2->setType(AMQP_EX_TYPE_FANOUT);
        $ex2->declareExchange();

        $q2 = new AMQPQueue($channel2);
        $q2->setName('q2-' . microtime(true));
        $q2->declareQueue();
        $q2->bind($ex2->getName());

        if ($nested_publish) {
            for($i=0; $i < $cnt2; $i++) {
                $ex2->publish("message 2 - {$i}");
            }
            $nested_publish = false;
        }

        $q2->consume(function (AMQPEnvelope $message, AMQPQueue $queue) use (&$cnt2) {
            printf("2: %s [%s] %s - %s (%s): %s queue\n", $message->getExchangeName(), $message->getBody(), $message->getConsumerTag(), $queue->getConsumerTag(), $queue->getName(), $message->getConsumerTag() == $queue->getConsumerTag() ? 'valid' : 'not valid');
            $queue->ack($message->getDeliveryTag());

            return --$cnt2 > 1;
        });

        return --$cnt1 > 1;
    });
}

$connection1 = new AMQPConnection();
$connection1->connect();
$channel1 = new AMQPChannel($connection1);
echo 'With default prefetch = 3', PHP_EOL;
test($channel1);

$channel1->close();
$channel1 = null;
$connection1->disconnect();
$connection1 = null;

// var_dump($channel1);
$connection2 = new AMQPConnection();
$connection2->connect();

$channel2 = new AMQPChannel($connection2);
$channel2->setPrefetchCount(1);
echo 'With prefetch = 1', PHP_EOL;
test($channel2);



?>
--EXPECTF--
With default prefetch = 3
1: ex1-%f [message 1 - 0] amq.ctag-%s - amq.ctag-%s (q1-%f): valid queue
2: ex1-%f [message 1 - 1] amq.ctag-%s - amq.ctag-%s (q1-%f): valid queue
2: ex1-%f [message 1 - 2] amq.ctag-%s - amq.ctag-%s (q1-%f): valid queue
2: ex1-%f [message 1 - 3] amq.ctag-%s - amq.ctag-%s (q1-%f): valid queue
1: ex2-%f [message 2 - 0] amq.ctag-%s - amq.ctag-%s (q2-%f): valid queue
2: ex2-%f [message 2 - 1] amq.ctag-%s - amq.ctag-%s (q2-%f): valid queue
1: ex2-%f [message 2 - 2] amq.ctag-%s - amq.ctag-%s (q2-%f): valid queue
2: ex2-%f [message 2 - 3] amq.ctag-%s - amq.ctag-%s (q2-%f): valid queue
With prefetch = 1
1: ex1-%f [message 1 - 0] amq.ctag-%s - amq.ctag-%s (q1-%f): valid queue
2: ex1-%f [message 1 - 1] amq.ctag-%s - amq.ctag-%s (q1-%f): valid queue
2: ex2-%f [message 2 - 0] amq.ctag-%s - amq.ctag-%s (q2-%f): valid queue
2: ex2-%f [message 2 - 1] amq.ctag-%s - amq.ctag-%s (q2-%f): valid queue
1: ex2-%f [message 2 - 2] amq.ctag-%s - amq.ctag-%s (q2-%f): valid queue
2: ex%d-%f [message %d - %d] amq.ctag-%s - amq.ctag-%s (q%d-%f): valid queue
1: ex%d-%f [message %d - %d] amq.ctag-%s - amq.ctag-%s (q%d-%f): valid queue
2: ex1-%f [message 1 - 3] amq.ctag-%s - amq.ctag-%s (q1-%f): valid queue
