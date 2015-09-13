<?php

if(!extension_loaded("amqp")) 
{
    die("AMQP module not installed");
}

class TutorialPublisher 
{
    private $conn;
    private $chan;
    private $exch;

    public function __construct() 
    {
        $this->conn = new AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->conn->connect();
        $this->chan = new AMQPChannel($this->conn);
        $this->exch = new AMQPExchange($this->chan);
        $this->exch->setName("exchange-logs");
        $this->exch->setType(AMQP_EX_TYPE_FANOUT);
        $this->exch->declareExchange();
        echo "Publisher " . getmypid() . " starting up using exchange ". 
                $this->exch->getName() . "\n";
    }

    public function sendLog($log) 
    {
        $this->exch->publish($log);
    }
}

class TutorialSubscriber 
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
        $this->chan->setPrefetchCount(1);
        $this->queue = new AMQPQueue($this->chan);
        $this->queue->declareQueue();
        $this->exch = new AMQPExchange($this->chan);
        $this->exch->setName("exchange-logs");
        $this->exch->setType(AMQP_EX_TYPE_FANOUT);
        $this->exch->declareExchange(); 
        $this->queue->bind($this->exch->getName()); 
        echo "Subscriber " . getmypid() . " starting up on queue " . 
                $this->queue->getName() . "\n";
    }

    public function consume() 
    {
        $this->queue->consume(
             function(AMQPEnvelope $message, AMQPQueue $queue) {
                $queue->ack($message->getDeliveryTag());
                if($message->getBody() == "QUIT") { 
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
    $logger = new TutorialPublisher;
    
    sleep(1); // Wait for subscribers to start and bind their queues.
    
    $logger->sendLog("Log 1");
    $logger->sendLog("Log 2");
    
    // Only one required, all subscribers get the same message.
    $logger->sendLog("QUIT");
    
    sleep(1);
}
else 
{
    // Another fork to produce two subscribers
    $pid = pcntl_fork(); 
    $logger = new TutorialSubscriber;
    $logger->consume();
}
