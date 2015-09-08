<?php

if(!extension_loaded("amqp")) {
    die("AMQP module not installed");
}

class TutorialProducer {
    private $_conn;
    private $_chan;
    private $_exch;
    
    public function __construct() {
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_exch = new \AMQPExchange($this->_chan);
        $this->_exch->setName("exchange-topics");
        $this->_exch->setType(AMQP_EX_TYPE_TOPIC);
        $this->_exch->declareExchange();
        echo "Log producer starting up using exchange ". 
                $this->_exch->getName() . "\n";
    }
    
    public function sendInfo($log) {
        $this->_exch->publish($log, 'anon.info');
    }
    
    public function sendError($log) {
        $this->_exch->publish($log, 'anon.error');
    }
}

class TutorialConsumer {
    private $_conn;
    private $_chan;
    private $_exch;
    private $_queue;
    private $_routing_key;
    
    public function __construct($routing_key = null) {
        $this->_routing_key = $routing_key;
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_chan->setPrefetchCount(1);
        $this->_queue = new \AMQPQueue($this->_chan);
        $this->_queue->declareQueue();
        $this->_exch = new \AMQPExchange($this->_chan);
        $this->_exch->setName("exchange-topics");
        $this->_exch->setType(AMQP_EX_TYPE_TOPIC);
        $this->_exch->declareExchange(); 
        $this->_queue->bind($this->_exch->getName(), $this->_routing_key); 
        echo "Log consumer '{$this->_routing_key}' starting up on queue " . 
                $this->_queue->getName() . "\n";
    }
    
    public function consume() {
        $this->_queue->consume(
            function(\AMQPEnvelope $message, \AMQPQueue $queue) {
                $queue->ack($message->getDeliveryTag());
                if($message->getBody() == "QUIT") { 
                    echo "Log consumer '{$this->_routing_key}'  received exit.\n";
                    exit(0); 
                }
                echo "Log consumer '{$this->_routing_key}' " . 
                    " got from queue '". $queue->getName() . 
                    "': " . $message->getBody() . "\n";
            }
        );
    }
    
    public function onLog(\AMQPEnvelope $message, \AMQPQueue $queue) {
        $queue->ack($message->getDeliveryTag());
        if($message->getBody() == "QUIT") { 
            echo "Log consumer '{$this->_routing_key}'  received exit.\n";
            exit(0); 
        }
        echo "Log consumer '{$this->_routing_key}' " . 
                " got from queue '". $queue->getName() . 
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
    $logger = new TutorialProducer;
    
    sleep(1); // Wait for consumers to start and bind their queues.
    
    $logger->sendInfo("Info log 1");
    $logger->sendError("Error log 1");
    $logger->sendInfo("Info 1og 2");
    $logger->sendError("Error log 2");
    
    $logger->sendInfo("QUIT");
    $logger->sendError("QUIT");
    
    sleep(1);
}
else {
    // Additional forks to create three consumers
    $pid = pcntl_fork(); 
    if($pid) {
        if(pcntl_fork()) {
            $logger = new TutorialConsumer("anon.info");
            $logger->consume();
        } else {
            $logger = new TutorialConsumer("anon.error");
            $logger->consume();
        }
    } else {
        // This comsumer gets all logs
        $logger = new TutorialConsumer("anon.*");
        $logger->consume();
    }
}
