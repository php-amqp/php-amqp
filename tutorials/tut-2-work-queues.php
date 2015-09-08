<?php

if(!extension_loaded("amqp")) {
    die("AMQP module not installed");
}

class TutorialNewTask {
    private $_conn;
    private $_chan;
    private $_exch;
    private $_queue;
    public function __construct() {
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_queue = new \AMQPQueue($this->_chan);
        $this->_queue->setName("queue-task");
        $this->_queue->declareQueue();
        $this->_exch = new \AMQPExchange($this->_chan);
        $this->_exch->setName("exchange-task");
        $this->_exch->setType(AMQP_EX_TYPE_FANOUT);
        $this->_exch->declareExchange();    
        $this->_queue->bind($this->_exch->getName()); 
    }
    public function sendTask($task) {
        $this->_exch->publish($task);
    }
}

class TutorialWorker {
    private $_conn;
    private $_chan;
    private $_queue;
    public function __construct() {
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_chan->setPrefetchCount(1);
        $this->_queue = new \AMQPQueue($this->_chan);
        $this->_queue->setName("queue-task");
        $this->_queue->declareQueue();
        echo "Worker " . getmypid() . " starting up...\n";
    }
    public function consume() {
        while(true) {
            $this->_queue->consume(array($this, "onTask"));
        }
    }
    public function onTask(\AMQPEnvelope $message, \AMQPQueue $queue) {
        $queue->ack($message->getDeliveryTag());
        if($message->getBody() == "QUIT") { 
            echo "Worker " . getmypid() . " received exit.\n";
            exit(0); 
        }
        echo "Worker " . getmypid() . " got from '". $queue->getName() . 
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
}
else {
    // Another fork to produce two workers
    $pid = pcntl_fork(); 
    $worker = new TutorialWorker;
    $worker->consume();
}
