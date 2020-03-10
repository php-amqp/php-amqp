--TEST--
AMQPExchange::setArgument() test
--SKIPIF--
<?php if (!extension_loaded("amqp")) print "skip"; ?>
--FILE--
<?php
$credentials = array();

$cnn = new AMQPConnection($credentials);
$cnn->connect();

$ch = new AMQPChannel($cnn);

$heartbeat   = 10;
$e_name_ae   = 'test.exchange.ae.' . microtime(true);
$e_name = 'test.exchange.' . microtime(true);

$ex_ae = new AMQPExchange($ch);
$ex_ae->setName($e_name_ae);
$ex_ae->setFlags(AMQP_AUTODELETE);
$ex_ae->setType(AMQP_EX_TYPE_FANOUT);
$ex_ae->declareExchange();

var_dump($ex_ae);


$ex = new AMQPExchange($ch);
$ex->setName($e_name);
$ex->setFlags(AMQP_AUTODELETE);
$ex->setType(AMQP_EX_TYPE_FANOUT);
// some real keys
$ex->setArgument("x-ha-policy", "all");
$ex->setArgument("alternate-exchange", $e_name_ae);
// some custom keys to test various cases
$ex->setArgument('x-empty-string', '');
$ex->setArgument('x-alternate-exchange-one-more-time', $e_name_ae);
$ex->setArgument('x-numeric-argument', $heartbeat * 10 * 1000);
$ex->declareExchange();

var_dump($ex);
$ex->setArgument('x-alternate-exchange-one-more-time', null); // remov key
var_dump($ex);
?>
--EXPECTF--
object(AMQPExchange)#3 (9) {
  ["connection":"AMQPExchange":private]=>
  object(AMQPConnection)#1 (18) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["rpc_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
    ["cacert":"AMQPConnection":private]=>
    string(0) ""
    ["key":"AMQPConnection":private]=>
    string(0) ""
    ["cert":"AMQPConnection":private]=>
    string(0) ""
    ["verify":"AMQPConnection":private]=>
    bool(true)
    ["sasl_method":"AMQPConnection":private]=>
    int(0)
    ["connection_name":"AMQPConnection":private]=>
    NULL
  }
  ["channel":"AMQPExchange":private]=>
  object(AMQPChannel)#2 (6) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (18) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      int(5672)
      ["read_timeout":"AMQPConnection":private]=>
      float(0)
      ["write_timeout":"AMQPConnection":private]=>
      float(0)
      ["connect_timeout":"AMQPConnection":private]=>
      float(0)
      ["rpc_timeout":"AMQPConnection":private]=>
      float(0)
      ["channel_max":"AMQPConnection":private]=>
      int(256)
      ["frame_max":"AMQPConnection":private]=>
      int(131072)
      ["heartbeat":"AMQPConnection":private]=>
      int(0)
      ["cacert":"AMQPConnection":private]=>
      string(0) ""
      ["key":"AMQPConnection":private]=>
      string(0) ""
      ["cert":"AMQPConnection":private]=>
      string(0) ""
      ["verify":"AMQPConnection":private]=>
      bool(true)
      ["sasl_method":"AMQPConnection":private]=>
      int(0)
      ["connection_name":"AMQPConnection":private]=>
      NULL
    }
    ["prefetch_count":"AMQPChannel":private]=>
    int(3)
    ["prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_count":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["consumers":"AMQPChannel":private]=>
    array(0) {
    }
  }
  ["name":"AMQPExchange":private]=>
  string(%d) "test.exchange.ae.%f"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["auto_delete":"AMQPExchange":private]=>
  bool(true)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(0) {
  }
}
object(AMQPExchange)#4 (9) {
  ["connection":"AMQPExchange":private]=>
  object(AMQPConnection)#1 (18) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["rpc_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
    ["cacert":"AMQPConnection":private]=>
    string(0) ""
    ["key":"AMQPConnection":private]=>
    string(0) ""
    ["cert":"AMQPConnection":private]=>
    string(0) ""
    ["verify":"AMQPConnection":private]=>
    bool(true)
    ["sasl_method":"AMQPConnection":private]=>
    int(0)
    ["connection_name":"AMQPConnection":private]=>
    NULL
  }
  ["channel":"AMQPExchange":private]=>
  object(AMQPChannel)#2 (6) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (18) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      int(5672)
      ["read_timeout":"AMQPConnection":private]=>
      float(0)
      ["write_timeout":"AMQPConnection":private]=>
      float(0)
      ["connect_timeout":"AMQPConnection":private]=>
      float(0)
      ["rpc_timeout":"AMQPConnection":private]=>
      float(0)
      ["channel_max":"AMQPConnection":private]=>
      int(256)
      ["frame_max":"AMQPConnection":private]=>
      int(131072)
      ["heartbeat":"AMQPConnection":private]=>
      int(0)
      ["cacert":"AMQPConnection":private]=>
      string(0) ""
      ["key":"AMQPConnection":private]=>
      string(0) ""
      ["cert":"AMQPConnection":private]=>
      string(0) ""
      ["verify":"AMQPConnection":private]=>
      bool(true)
      ["sasl_method":"AMQPConnection":private]=>
      int(0)
      ["connection_name":"AMQPConnection":private]=>
      NULL
    }
    ["prefetch_count":"AMQPChannel":private]=>
    int(3)
    ["prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_count":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["consumers":"AMQPChannel":private]=>
    array(0) {
    }
  }
  ["name":"AMQPExchange":private]=>
  string(%d) "test.exchange.%f"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["auto_delete":"AMQPExchange":private]=>
  bool(true)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(5) {
    ["x-ha-policy"]=>
    string(3) "all"
    ["alternate-exchange"]=>
    string(%d) "test.exchange.ae.%f"
    ["x-empty-string"]=>
    string(0) ""
    ["x-alternate-exchange-one-more-time"]=>
    string(%d) "test.exchange.ae.%f"
    ["x-numeric-argument"]=>
    int(100000)
  }
}
object(AMQPExchange)#4 (9) {
  ["connection":"AMQPExchange":private]=>
  object(AMQPConnection)#1 (18) {
    ["login":"AMQPConnection":private]=>
    string(5) "guest"
    ["password":"AMQPConnection":private]=>
    string(5) "guest"
    ["host":"AMQPConnection":private]=>
    string(9) "localhost"
    ["vhost":"AMQPConnection":private]=>
    string(1) "/"
    ["port":"AMQPConnection":private]=>
    int(5672)
    ["read_timeout":"AMQPConnection":private]=>
    float(0)
    ["write_timeout":"AMQPConnection":private]=>
    float(0)
    ["connect_timeout":"AMQPConnection":private]=>
    float(0)
    ["rpc_timeout":"AMQPConnection":private]=>
    float(0)
    ["channel_max":"AMQPConnection":private]=>
    int(256)
    ["frame_max":"AMQPConnection":private]=>
    int(131072)
    ["heartbeat":"AMQPConnection":private]=>
    int(0)
    ["cacert":"AMQPConnection":private]=>
    string(0) ""
    ["key":"AMQPConnection":private]=>
    string(0) ""
    ["cert":"AMQPConnection":private]=>
    string(0) ""
    ["verify":"AMQPConnection":private]=>
    bool(true)
    ["sasl_method":"AMQPConnection":private]=>
    int(0)
    ["connection_name":"AMQPConnection":private]=>
    NULL
  }
  ["channel":"AMQPExchange":private]=>
  object(AMQPChannel)#2 (6) {
    ["connection":"AMQPChannel":private]=>
    object(AMQPConnection)#1 (18) {
      ["login":"AMQPConnection":private]=>
      string(5) "guest"
      ["password":"AMQPConnection":private]=>
      string(5) "guest"
      ["host":"AMQPConnection":private]=>
      string(9) "localhost"
      ["vhost":"AMQPConnection":private]=>
      string(1) "/"
      ["port":"AMQPConnection":private]=>
      int(5672)
      ["read_timeout":"AMQPConnection":private]=>
      float(0)
      ["write_timeout":"AMQPConnection":private]=>
      float(0)
      ["connect_timeout":"AMQPConnection":private]=>
      float(0)
      ["rpc_timeout":"AMQPConnection":private]=>
      float(0)
      ["channel_max":"AMQPConnection":private]=>
      int(256)
      ["frame_max":"AMQPConnection":private]=>
      int(131072)
      ["heartbeat":"AMQPConnection":private]=>
      int(0)
      ["cacert":"AMQPConnection":private]=>
      string(0) ""
      ["key":"AMQPConnection":private]=>
      string(0) ""
      ["cert":"AMQPConnection":private]=>
      string(0) ""
      ["verify":"AMQPConnection":private]=>
      bool(true)
      ["sasl_method":"AMQPConnection":private]=>
      int(0)
      ["connection_name":"AMQPConnection":private]=>
      NULL
    }
    ["prefetch_count":"AMQPChannel":private]=>
    int(3)
    ["prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_count":"AMQPChannel":private]=>
    int(0)
    ["global_prefetch_size":"AMQPChannel":private]=>
    int(0)
    ["consumers":"AMQPChannel":private]=>
    array(0) {
    }
  }
  ["name":"AMQPExchange":private]=>
  string(%d) "test.exchange.%f"
  ["type":"AMQPExchange":private]=>
  string(6) "fanout"
  ["passive":"AMQPExchange":private]=>
  bool(false)
  ["durable":"AMQPExchange":private]=>
  bool(false)
  ["auto_delete":"AMQPExchange":private]=>
  bool(true)
  ["internal":"AMQPExchange":private]=>
  bool(false)
  ["arguments":"AMQPExchange":private]=>
  array(4) {
    ["x-ha-policy"]=>
    string(3) "all"
    ["alternate-exchange"]=>
    string(%d) "test.exchange.ae.%f"
    ["x-empty-string"]=>
    string(0) ""
    ["x-numeric-argument"]=>
    int(100000)
  }
}
