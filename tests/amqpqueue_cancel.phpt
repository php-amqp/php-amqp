--TEST--
AMQPQueue cancel
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
function create_connection() {
	$conn = new AMQPConnection();
	$conn->connect();
	return $conn;
}

function create_channel($connection) {
	$channel = new AMQPChannel($connection);
	$channel->setPrefetchCount(1);
	return $channel;
}

function create_exchange($channel) {
	$exchange = new AMQPExchange($channel);
	$exchange->setName('test_cancel_exchange');
	$exchange->setType(AMQP_EX_TYPE_DIRECT);
	$exchange->declareExchange();
	return $exchange;
}

function create_queue($channel) {
	$queue = new AMQPQueue($channel);
	$queue->setName('test_cancel_queue');
	$queue->setFlags(AMQP_NOPARAM);
	$queue->declareQueue();
	$queue->bind('test_cancel_exchange', 'test_cancel_routing_key');
	return $queue;
}

class TrivialAcceptor {
	private $count = 0;
	private $maxCount;

	public function __construct($maxCount) {
		$this->maxCount = $maxCount;
	}

	public function accept($envelope, $queue) {
		var_dump(
			$envelope->getBody(),
			$envelope->getDeliveryTag(),
			$envelope->isRedelivery()
		);
		echo "\n";
		$queue->ack($envelope->getDeliveryTag());
		$this->count++;
		return $this->count < $this->maxCount;
	}
}

function get_acceptor($count) {
	$acceptorObject = new TrivialAcceptor($count);
	return array($acceptorObject, 'accept');
}

function send_message($exchange, $message) {
	$exchange->publish($message, 'test_cancel_routing_key');
}

function wait_for_messages($queue, $consumer_tag, $message_count, $cancel_afterwards) {
	$consumeMethod = new ReflectionMethod('AMQPQueue', 'consume');
	switch ($consumeMethod->getNumberOfParameters())
	{
		case 3:
			$queue->consume(get_acceptor($message_count), AMQP_NOPARAM, $consumer_tag);
			if ($cancel_afterwards)
				$queue->cancel($consumer_tag);
			break;

		case 2:
			$queue->consume(get_acceptor($message_count), AMQP_NOPARAM);
			if ($cancel_afterwards)
				$queue->cancel();
			break;

		default:
			echo "AMQP::consume() takes neither 2 nor 3 parameters";
			exit(1);
	}
}

$consumer_tag_prefix = uniqid();

$send_conn = create_connection();
$send_chan = create_channel($send_conn);
$exchange = create_exchange($send_chan);

$recv_conn_1 = create_connection();
$recv_chan_1 = create_channel($recv_conn_1);
$queue_1 = create_queue($recv_chan_1);

send_message($exchange, 'message 0');
wait_for_messages($queue_1, $consumer_tag_prefix.'_1', 1, true);

send_message($exchange, 'message 1');
send_message($exchange, 'message 2');

$recv_chan_2 = create_channel($recv_conn_1);
$queue_2 = create_queue($recv_chan_2);

wait_for_messages($queue_2, $consumer_tag_prefix.'_2', 2, false);

$recv_conn_1->disconnect();
sleep(1);

$recv_conn_2 = create_connection();
$recv_chan_3 = create_channel($recv_conn_2);
$queue_3 = create_queue($recv_chan_3);

send_message($exchange, 'message 3');
send_message($exchange, 'message 4');

wait_for_messages($queue_3, $consumer_tag_prefix.'_3', 2, false);

$queue_3->delete(AMQP_NOPARAM);
$exchange->delete();
?>
--EXPECT--
string(9) "message 0"
int(1)
bool(false)

string(9) "message 1"
int(1)
bool(false)

string(9) "message 2"
int(2)
bool(false)

string(9) "message 3"
int(1)
bool(false)

string(9) "message 4"
int(2)
bool(false)

