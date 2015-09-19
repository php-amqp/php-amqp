<?php

if (!extension_loaded("amqp")) {
    die("AMQP module not installed");
}

class TutorialConsumer 
{
    private $connection;
    private $channel;
    private $queue;
    
    public function __construct() 
    {
        $this->connection = new AMQPConnection(
            ['localhost', 5672, 'guest', 'guest']);
        $this->connection->connect();
        $this->channel = new AMQPChannel($this->connection);
        $this->channel->setPrefetchCount(1);
        $this->queue = new AMQPQueue($this->channel);
        $this->queue->setName("queue-hello-world");
        $this->queue->declareQueue();
    }
    
    public function consume() 
    {
        $this->queue->consume(
            function (AMQPEnvelope $message, AMQPQueue $queue) {
                $queue->ack($message->getDeliveryTag());
                if ($message->getBody() == "QUIT") { 
                    $this->queue->delete();
                    exit(0);                 
                }
                echo "From '". $queue->getName() . "': " . 
                    $message->getBody() . "\n";
            }
        );
    }
}

class TutorialProducer 
{
    private $connection;
    private $channel;
    private $exch;
    private $queue;
    
    public function __construct() 
    {
        $this->connection = new AMQPConnection(
            ['localhost', 5672, 'guest', 'guest']);
        $this->connection->connect();
        $this->channel = new AMQPChannel($this->connection);
        $this->queue = new AMQPQueue($this->channel);
        $this->queue->setName("queue-hello-world");
        $this->queue->declareQueue();
        $this->exch = new AMQPExchange($this->channel);
        $this->exch->setName("exchange-hello-world");
        $this->exch->setType(AMQP_EX_TYPE_FANOUT);        
        $this->exch->declareExchange();    
        $this->queue->bind($this->exch->getName()); 
        $this->exch->publish('Hello World!');
        $this->exch->publish('QUIT');
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
    $consumer = new TutorialConsumer;
    $consumer->consume();
} else { 
   $producer = new TutorialProducer;
}
