<?php

if (!extension_loaded("amqp")) {
    die("AMQP module not installed");
}

class TutorialPublisher 
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
        $this->exchange->setName("exchange-logs");
        $this->exchange->setType(AMQP_EX_TYPE_FANOUT);
        $this->exchange->declareExchange();
        echo "Publisher " . getmypid() . " starting up using exchange ". 
                $this->exchange->getName() . "\n";
    }

    public function sendLog($log) 
    {
        $this->exchange->publish($log);
    }
}

class TutorialSubscriber 
{
    private $connection;
    private $channel;
    private $exchange;
    private $queue;
    public function __construct() 
    {
        $this->connection = new AMQPConnection(
            ['localhost', 5672, 'guest', 'guest']);
        $this->connection->connect();
        $this->channel = new AMQPChannel($this->connection);
        $this->channel->setPrefetchCount(1);
        $this->queue = new AMQPQueue($this->channel);
        $this->queue->declareQueue();
        $this->exchange = new AMQPExchange($this->channel);
        $this->exchange->setName("exchange-logs");
        $this->exchange->setType(AMQP_EX_TYPE_FANOUT);
        $this->exchange->declareExchange(); 
        $this->queue->bind($this->exchange->getName()); 
        echo "Subscriber " . getmypid() . " starting up on queue " . 
                $this->queue->getName() . "\n";
    }

    public function consume() 
    {
        $this->queue->consume(
             function (AMQPEnvelope $message, AMQPQueue $queue) {
                $queue->ack($message->getDeliveryTag());
                if ($message->getBody() == "QUIT") { 
                    $this->queue->delete();
                    echo "Subscriber " . getmypid() . " received exit.\n";
                    exit(0); 
                }
                echo "Logger " . getmypid() . " got from '". $queue->getName() . 
                        "': " . $message->getBody() . "\n";
            }   
        );
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
} elseif($pid) {
    $logger = new TutorialPublisher;
    
    sleep(1); // Wait for subscribers to start and bind their queues.
    
    $logger->sendLog("Log 1");
    $logger->sendLog("Log 2");
    
    // Only one required, all subscribers get the same message.
    $logger->sendLog("QUIT");
    
    sleep(1);
} else {
    // Another fork to produce two subscribers
    $pid = pcntl_fork(); 
    $logger = new TutorialSubscriber;
    $logger->consume();
}
