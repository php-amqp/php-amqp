<?php

if(!extension_loaded("amqp")) {
    die("AMQP module not installed");
}

class TutorialPublisher {
    private $_conn;
    private $_chan;
    private $_exch;
    public function __construct() {
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_exch = new \AMQPExchange($this->_chan);
        $this->_exch->setName("exchange-logs");
        $this->_exch->setType(AMQP_EX_TYPE_FANOUT);
        $this->_exch->declareExchange();
        echo "Publisher " . getmypid() . " starting up using exchange ". 
                $this->_exch->getName() . "\n";
    }
    public function sendLog($log, $level = 0) {
        $this->_exch->publish($log);
    }
}

class TutorialSubscriber {
    private $_conn;
    private $_chan;
    private $_exch;
    private $_queue;
    public function __construct() {
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_chan->setPrefetchCount(1);
        $this->_queue = new \AMQPQueue($this->_chan);
        $this->_queue->declareQueue();
        $this->_exch = new \AMQPExchange($this->_chan);
        $this->_exch->setName("exchange-logs");
        $this->_exch->setType(AMQP_EX_TYPE_FANOUT);
        $this->_exch->declareExchange(); 
        $this->_queue->bind($this->_exch->getName()); 
        echo "Subscriber " . getmypid() . " starting up on queue " . 
                $this->_queue->getName() . "\n";
    }
    public function consume() {
        while(true) {
            $this->_queue->consume(array($this, "onLog"));
        }
    }
    public function onLog(\AMQPEnvelope $message, \AMQPQueue $queue) {
        $queue->ack($message->getDeliveryTag());
        if($message->getBody() == "QUIT") { 
            echo "Subscriber " . getmypid() . " received exit.\n";
            exit(0); 
        }
        echo "Logger " . getmypid() . " got from '". $queue->getName() . 
                "': " . $message->getBody() . "\n";
    }
}

// We use PCNTL to create new processes to handle
// different ends of the queuing systems.
if(!extension_loaded("pcntl")) {
    die("PCNTL module not installed");
}

$pid = pcntl_fork();
if($pid == -1) {
    die("Fork failed\n.");
}
else if($pid) {
    $logger = new TutorialPublisher;
    
    sleep(1); // Wait for subscribers to start and bind their queues.
    
    $logger->sendLog("Log 1");
    $logger->sendLog("Log 2");
    
    // Only one required, all subscribers get the same message.
    $logger->sendLog("QUIT");
}
else {
    // Another fork to produce two subscribers
    $pid = pcntl_fork(); 
    $logger = new TutorialSubscriber;
    $logger->consume();
}
