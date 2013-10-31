<?php
/**
* Example code to use the AMQP PECL multiple-consumer extension
**/
$aParams = array(
	'host' => 'ubuntu-G',
	'port' => 5672,
	'vhost' => '/',
	'login' => 'guest',
	'password' =>'guest'
);

$sExchangeBase = "ex-perf-test-";
$sQueueBase = "q-perf-test-";
$aConsumers = array();

$aArgv = $_SERVER['argv'];
array_shift($aArgv);

$iNumConsumers = (int)array_shift($aArgv);

if(!$iNumConsumers) {
	echo "You must supply the number of consumers to run\n";
	exit;
}

echo "Starting $iNumConsumers consumers\n";


for($i=1; $i<=$iNumConsumers; $i++) {
	
	$oConnection = new \AMQPConnection($aParams);
	$oConnection->connect();
	
	$oChannel = new \AMQPChannel($oConnection);
	
	$oExchange = new \AMQPExchange($oChannel);
	$oExchange->setName($sExchangeBase.$i);
	$oExchange->setType(AMQP_EX_TYPE_FANOUT);
	$oExchange->declareExchange();
	
	$oQueue = new \AMQPQueue($oChannel);
	$oQueue->setName($sQueueBase.$i);
	$oQueue->setFlags(0);
	$oQueue->declareQueue();
	$oQueue->bind($oExchange->getName(), null);

	$fn = function(\AMQPEnvelope $oEnvelope, \AMQPQueue $oQueue) {
	
		$oQueue->ack($oEnvelope->getDeliveryTag());
	
		return $oEnvelope->getBody() != "STOP!!";
	
	};

	$oConsumer = new \AMQPConsumer($oQueue, $fn);
	$oConsumer->basicConsume();
	
	$aConsumers[] = $oConsumer;
}
	
$oDispatcher = new \AMQPConsumerDispatcher($aConsumers);

while($oDispatcher->hasConsumers()) {
	$oConsumer = $oDispatcher->select(1);

	if($oConsumer !== null) {
		if(!$oConsumer->consumeOne()) {
			$oDispatcher->removeConsumer($oConsumer);
		}
	}
}

echo "Finished\n";

