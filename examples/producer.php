<?php
/**
* Forks multiple producers, each sending messages to a dedicated exchange bound to one queue
**/
$aParams = array(
	'host' => 'ubuntu-G',
	'port' => 5672,
	'vhost' => '/',
	'login' => 'guest',
	'password' =>'guest'
);

$iNumConnections = 4;

$aArgv = $_SERVER['argv'];
array_shift($aArgv);

function getArg($sSwitch) {

	global $aArgv;
	
	if(!count($aArgv)) {
		error("Argument expected for switch $sSwitch");
	}
	
	return array_shift($aArgv);
}

function error($sMsg) {
	echo $sMsg."\n";
	exit;
}

function startAndRun($iChildId, $aParams, $iNumMessages) {
	
	$sExchangeBase = "ex-perf-test-";
	$sQueueBase = "q-perf-test-";

	$oConnection = new \AMQPConnection($aParams);
	$oConnection->connect();
	
	$oChannel = new \AMQPChannel($oConnection);
	
	$oExchange = new \AMQPExchange($oChannel);
	$oExchange->setName($sExchangeBase.$iChildId);
	$oExchange->setType(AMQP_EX_TYPE_FANOUT);
	$oExchange->declareExchange();
	
	$oQueue = new \AMQPQueue($oChannel);
	$oQueue->setName($sQueueBase.$iChildId);
	$oQueue->setFlags(0);
	$oQueue->declareQueue();
	$oQueue->bind($oExchange->getName(), null);
	
	for($i=0; $i<$iNumMessages; $i++) {
		$oExchange->publish("Hello world", null);
	}
	
	$oExchange->publish("STOP!!");
	
	exit;
}



$oImpl = null;
$iNumWorkers = 4;
$iNumMessages = 1000;

while(count($aArgv)) {
	
	$sArg = array_shift($aArgv);
	
	switch($sArg) {
	
		case '--n':
		case '-n': // set the number of workers
			$iNumWorkers = (int)getArg($sArg);
			break;
			
		case '--m':
		case '-m': // set the number of messages
			$iNumMessages = (int)getArg($sArg);
			break;
			
		default:
			error("Unrecognised switch $sArg");
	}
}

if($iNumWorkers == 0) {
	error("Cannot run zero workers");
}

if($iNumMessages == 0) {
	error("Cannot send zero messages");
}

echo "Running $iNumWorkers workers each sending $iNumMessages messages\n";

$aKids = array();

for($i = 0; $i < $iNumWorkers; $i++) {

	$iPid = pcntl_fork();
	
	if($iPid == -1) {
		error("Failed to fork worker");
	} else if($iPid > 0) { // I am the parent
		$aKids[$iPid] = true;
	} else {

		startAndRun($i+1, $aParams, $iNumMessages);
	}
}

while(count($aKids)) {

	$iPid = pcntl_waitpid(-1, $iStatus, WNOHANG);
	unset($aKids[$iPid]);
	usleep(100);
}

echo "Parent exiting\n";
