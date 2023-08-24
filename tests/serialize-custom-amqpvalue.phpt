--TEST--
Serialize custom AMQP value
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip";
if (!getenv("PHP_AMQP_HOST")) print "skip";
?>
--FILE--
<?php
$cnn = new AMQPConnection();
$cnn->setHost(getenv('PHP_AMQP_HOST'));
$cnn->connect();

$ch = new AMQPChannel($cnn);

$ex = new AMQPExchange($ch);
$ex->setName('ex-' . bin2hex(random_bytes(32)));
$ex->setType(AMQP_EX_TYPE_FANOUT);
$ex->declare();

$q = new AMQPQueue($ch);
$q->setName('q-' . bin2hex(random_bytes(32)));
$q->declare();
$q->bind($ex->getName());

class MyAmqpValue implements AMQPValue {
    public function toAmqpValue()
    {
        return 'foo';
    }
}

class MyNestedAmqpValue implements AMQPValue {
    private $val;
    public function __construct(AMQPValue $val) {
        $this->val = $val;
    }

    public function toAmqpValue() {
        return $this->val;
    }
}

$ex->publish('msg', null, null, ['headers' => [
    'x-val' => new MyAmqpValue(),
    'x-nested' => new MyNestedAmqpValue(new MyAmqpValue()),
    'x-nested-decimal' => new MyNestedAmqpValue(new AMQPDecimal(1, 2345)),
    'x-nested-timestamp' => new MyNestedAmqpValue(new AMQPTimestamp(987))
]]);

$msg = $q->get(AMQP_AUTOACK);

var_dump($msg->getHeader('x-val'));
var_dump($msg->getHeader('x-nested'));
var_dump($msg->getHeader('x-nested-decimal')->getExponent());
var_dump($msg->getHeader('x-nested-decimal')->getSignificand());
var_dump($msg->getHeader('x-nested-timestamp')->getTimestamp());
?>
==DONE==
--EXPECT--
string(3) "foo"
string(3) "foo"
int(1)
int(2345)
float(987)
==DONE==