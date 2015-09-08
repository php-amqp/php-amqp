<?php

if(!extension_loaded("amqp")) {
    die("AMQP module not installed");
}

class TutorialConsumer {
    private $_conn;
    private $_chan;
    private $_queue;
    public function __construct() {
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_chan->setPrefetchCount(1);
        $this->_queue = new \AMQPQueue($this->_chan);
        $this->_queue->setName("queue-hello-world");
        $this->_queue->declareQueue();
    }
    public function consume() {
        $this->_queue->consume(array($this, "onMessage"));
    }
    public function onMessage(\AMQPEnvelope $message, \AMQPQueue $queue) {
        $queue->ack($message->getDeliveryTag());
        if($message->getBody() == "QUIT") { exit(0); }
        echo "From '". $queue->getName() . "': " . $message->getBody() . "\n";
    }
}

class TutorialProducer {
    private $_conn;
    private $_chan;
    private $_exch;
    private $_queue;
    public function __construct() {
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_queue = new \AMQPQueue($this->_chan);
        $this->_queue->setName("queue-hello-world");
        $this->_queue->declareQueue();
        $this->_exch = new \AMQPExchange($this->_chan);
        $this->_exch->setName("exchange-hello-world");
        $this->_exch->setType(AMQP_EX_TYPE_FANOUT);
        $this->_exch->declareExchange();    
        $this->_queue->bind($this->_exch->getName()); 
        $this->_exch->publish('Hello World!');
        $this->_exch->publish('QUIT');
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
    $consumer = new TutorialConsumer;
    $consumer->consume();
}
else { 
   $producer = new TutorialProducer;
}
