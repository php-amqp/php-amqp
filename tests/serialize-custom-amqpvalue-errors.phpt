--TEST--
Serialize custom AMQP value - errors
--SKIPIF--
<?php
if (!extension_loaded("amqp")) print "skip AMQP extension is not loaded";
elseif (!getenv("PHP_AMQP_HOST")) print "skip PHP_AMQP_HOST environment variable is not set";
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

class RecursiveValue implements AMQPValue {
    public function toAmqpValue()
    {
        return $this;
    }
}

class NestedValue implements AMQPValue {
    private $v;

    public function __construct($v)
    {
        $this->v = $v;
    }

    public function toAmqpValue()
    {
        return $this->v;
    }
}

try {
    $ex->publish('msg', null, null, ['headers' => ['x-val' => new RecursiveValue()]]);
} catch (AMQPException $e) {
    var_dump($e->getMessage());
}
$ex->publish('msg', null, null, ['headers' => ['x-val' => new NestedValue(new stdClass())]]);

$ex->publish('msg', null, null, ['headers' => ['x-val' => new NestedValue(new NestedValue(new stdClass()))]]);

?>
==DONE==
--EXPECTF--
string(%d) "Maximum serialization depth of 128 reached while serializing value"

Warning: AMQPExchange::publish(): Ignoring field 'x-val' due to unsupported value type (object) in %s on line %d

Warning: AMQPExchange::publish(): Ignoring field 'x-val' due to unsupported value type (object) in %s on line %d
==DONE==