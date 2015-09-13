<?php

if(!extension_loaded("amqp")) 
{
    die("AMQP module not installed");
}

class TutorialNewTask {
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
        $this->queue->setName("queue-task");
        $this->queue->declareQueue();
        $this->exch = new AMQPExchange($this->chan);
        $this->exch->setName("exchange-task");
        $this->exch->setType(AMQP_EX_TYPE_FANOUT);
        $this->exch->declareExchange();    
        $this->queue->bind($this->exch->getName()); 
    }
    
    public function sendTask($task)
    {
        $this->exch->publish($task);
    }
}

class TutorialWorker 
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
        $this->queue->setName("queue-task");
        $this->queue->declareQueue();
        echo "Worker " . getmypid() . " starting up...\n";
    }
    
    public function consume() 
    {
        $this->queue->consume(
            function(AMQPEnvelope $message, AMQPQueue $queue) {
                $queue->ack($message->getDeliveryTag());
                if($message->getBody() == "QUIT") { 
                    $this->queue->delete();
                    echo "Worker " . getmypid() . " received exit.\n";
                    exit(0); 
                }
                echo "Worker " . getmypid() . " got from '". $queue->getName() . 
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
    $task = new TutorialNewTask;
    
    sleep(1); // Wait for workers to start up and attach to queue.
    
    $task->sendTask("task 1");
    $task->sendTask("task 2");
    $task->sendTask("task 3");
    $task->sendTask("task 4");
    
    // We start two workers so send two quits to kill them.
    // Normally you wouldn't know how many workers to kill so
    // You would use a seperate PUB/SUB (described in tut 3)
    // to send a single QUIT that they all receive on another 
    // queue.
    $task->sendTask("QUIT");
    $task->sendTask("QUIT");
    
    sleep(1);
}
else 
{
    // Another fork to produce two workers
    $pid = pcntl_fork(); 
    $worker = new TutorialWorker;
    $worker->consume();
}
