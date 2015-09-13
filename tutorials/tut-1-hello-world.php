<?php

if(!extension_loaded("amqp")) 
{
    die("AMQP module not installed");
}

class TutorialConsumer 
{
    private $conn;
    private $chan;
    private $queue;
    
    public function __construct() 
    {
        $this->conn = new AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->conn->connect();
        $this->chan = new AMQPChannel($this->conn);
        $this->chan->setPrefetchCount(1);
        $this->queue = new AMQPQueue($this->chan);
        $this->queue->setName("queue-hello-world");
        $this->queue->declareQueue();
    }
    
    public function consume() 
    {
        $this->queue->consume(
            function(AMQPEnvelope $message, AMQPQueue $queue) {
                $queue->ack($message->getDeliveryTag());
                if($message->getBody() == "QUIT") { 
                    $this->queue->delete();
                    exit(0);                 
                }
                echo "From '". $queue->getName() . "': " . $message->getBody() . "\n";
            }
        );
    }
}

class TutorialProducer 
{
    private $conn;
    private $chan;
    private $exch;
    private $queue;
    
    public function __construct() 
    {
        $this->conn = new AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->conn->connect();
        $this->chan = new AMQPChannel($this->conn);
        $this->queue = new AMQPQueue($this->chan);
        $this->queue->setName("queue-hello-world");
        $this->queue->declareQueue();
        $this->exch = new AMQPExchange($this->chan);
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
if(!extension_loaded("pcntl")) 
{
    die("PCNTL module not installed");
}

$pid = pcntl_fork();
if($pid == -1) 
{
    die("Fork failed\n.");
} 
else if($pid) 
{
    $consumer = new TutorialConsumer;
    $consumer->consume();
}
else 
{ 
   $producer = new TutorialProducer;
}
