--TEST--
AMQPChannel - consumers
--SKIPIF--
<?php if (!extension_loaded("amqp")) {
    print "skip";
} ?>
--FILE--
<?php
$connection = new AMQPConnection();
$connection->connect();

$channel1 = new AMQPChannel($connection);

$q1 = new AMQPQueue($channel1);
$q1->setName('q1-' . microtime(true));
$q1->declareQueue();


$channel2 = new AMQPChannel($connection);

$q2_0 = new AMQPQueue($channel2);
$q2_0->setName('q2.0-' . microtime(true));
$q2_0->declareQueue();

$q2_1 = new AMQPQueue($channel2);
$q2_1->setName('q2.1-' . microtime(true));
$q2_1->declareQueue();


echo "Channels should have no consumers: c1: ", count($channel1->getConsumers()), ', c2: ', count($channel2->getConsumers()), PHP_EOL;

$q1->consume(null, AMQP_NOPARAM, 'test-consumer-0');

echo "Channel holds consumer: c1: ", count($channel1->getConsumers()), ', c2: ', count($channel2->getConsumers()), PHP_EOL;
$q2_0->consume(null, AMQP_NOPARAM, 'test-consumer-2-0');
$q2_1->consume(null, AMQP_NOPARAM, 'test-consumer-2-1');

echo "Channel holds consumer: c1: ", count($channel1->getConsumers()), ', c2: ', count($channel2->getConsumers()), PHP_EOL;

echo PHP_EOL;

echo "Consumers belongs to their channels:", PHP_EOL;
echo "c1:", PHP_EOL;
foreach ($channel1->getConsumers() as $tag => $queue) {
    echo '    ', $tag, ': ', $queue->getName(), PHP_EOL;
}
echo "c2:", PHP_EOL;
foreach ($channel2->getConsumers() as $tag => $queue) {
    echo '    ', $tag, ': ', $queue->getName(), PHP_EOL;
}

echo PHP_EOL;

$q1->cancel();
echo "Consumer removed after canceling: c1: ", count($channel1->getConsumers()), ', c2: ', count($channel2->getConsumers()), PHP_EOL;


$q2_0 = null;
$q2_1 = null;
echo "Consumer still stored after source variable been destroyed: c1: ", count($channel1->getConsumers()), ', c2: ', count($channel2->getConsumers()), PHP_EOL;
foreach ($channel2->getConsumers() as $tag => $queue) {
    $queue->cancel();
}
echo "Consumer removed after canceling: c1: ", count($channel1->getConsumers()), ', c2: ', count($channel2->getConsumers()), PHP_EOL;


?>
--EXPECTF--
Channels should have no consumers: c1: 0, c2: 0
Channel holds consumer: c1: 1, c2: 0
Channel holds consumer: c1: 1, c2: 2

Consumers belongs to their channels:
c1:
    test-consumer-0: q1-%f
c2:
    test-consumer-2-0: q2.0-%f
    test-consumer-2-1: q2.1-%f

Consumer removed after canceling: c1: 0, c2: 2
Consumer still stored after source variable been destroyed: c1: 0, c2: 2
Consumer removed after canceling: c1: 0, c2: 0
