<?php

if (!extension_loaded("amqp")) {
    die("AMQP module not installed");
}

class TutorialProducer 
{
    private $connection;
    private $channel;
    private $exchange;
    
    public function __construct() 
    {
        $this->connection = new AMQPConnection(
            ['localhost', 5672, 'guest', 'guest']);
        $this->connection->connect();
        $this->channel = new AMQPChannel($this->connection);
        $this->exchange = new AMQPExchange($this->channel);
        $this->exchange->setName("exchange-topics");
        $this->exchange->setType(AMQP_EX_TYPE_TOPIC);
        $this->exchange->declareExchange();
        echo "Log producer starting up using exchange ". 
                $this->exchange->getName() . "\n";
    }
    
    public function sendInfo($log) 
    {
        $this->exchange->publish($log, 'anon.info');
    }
    
    public function sendError($log) 
    {
        $this->exchange->publish($log, 'anon.error');
    }
}

class TutorialConsumer
{
    private $connection;
    private $channel;
    private $exchange;
    private $queue;
    private $routing_key;
    
    public function __construct($routing_key = null) 
    {
        $this->routing_key = $routing_key;
        $this->connection = new AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->connection->connect();
        $this->channel = new AMQPChannel($this->connection);
        $this->channel->setPrefetchCount(1);
        $this->queue = new AMQPQueue($this->channel);
        $this->queue->declareQueue();
        $this->exchange = new AMQPExchange($this->channel);
        $this->exchange->setName("exchange-topics");
        $this->exchange->setType(AMQP_EX_TYPE_TOPIC);
        $this->exchange->declareExchange(); 
        $this->queue->bind($this->exchange->getName(), $this->routing_key); 
        echo "Log consumer '{$this->routing_key}' starting up on queue " . 
                $this->queue->getName() . "\n";
    }

    public function consume() 
    {
        $this->queue->consume(
            function (AMQPEnvelope $message, AMQPQueue $queue) {
                $queue->ack($message->getDeliveryTag());
                if ($message->getBody() == "QUIT") { 
                    $this->queue->delete();
                    echo "Log consumer '{$this->routing_key}' received exit.\n";
                    exit(0); 
                }
                echo "Log consumer '{$this->routing_key}' " . 
                    " got from queue '". $queue->getName() . 
                    "': " . $message->getBody() . "\n";
            }
        );
    }
}

// We use PCNTL to create new processes to handle
// different ends of the queuing systems.
if (!extension_loaded("pcntl")) {
    die("PCNTL module not installed");
}

$pid = pcntl_fork();
if ($pid == -1) {
    die("Fork failed\n.");
} elseif ($pid) {
    $logger = new TutorialProducer;
    
    sleep(1); // Wait for consumers to start and bind their queues.
    
    $logger->sendInfo("Info log 1");
    $logger->sendError("Error log 1");
    $logger->sendInfo("Info 1og 2");
    $logger->sendError("Error log 2");
    
    $logger->sendInfo("QUIT");
    $logger->sendError("QUIT");
    
    sleep(1);
} else {
    // Additional forks to create three consumers
    $pid = pcntl_fork(); 
    if ($pid) {
        if (pcntl_fork()) {
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
