<?php

if(!extension_loaded("amqp")) {
    die("AMQP module not installed");
}

class TutorialServer {
    private $_conn;
    private $_chan;
    private $_queue;
    
    public function __construct() {
        // Create a "well known" named queue to receive requests on.
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
        $this->_chan->setPrefetchCount(1);
        $this->_queue = new \AMQPQueue($this->_chan);
        $this->_queue->setName("queue-well-known-rpc-name");
        $this->_queue->declareQueue();
    }
    
    private function verifyRequest($req) {
        foreach(['func', 'a', 'b'] as $key) {
            if(!array_key_exists($key, $req)) {
                return false;
            }
        }
        return true;
    }

    public function consume() {
        $this->_queue->consume(
            function(\AMQPEnvelope $message, \AMQPQueue $queue) {
                $queue->ack($message->getDeliveryTag());
                if($message->getBody() == "QUIT") { 
                    $this->_queue->delete();
                    exit(0);
                }
                $req = json_decode($message->getBody(), true);
                if(!is_array($req) || !$this->verifyRequest($req)) {
                    return;
                }
                $result = 0;
                switch($req['func']) {
                    case 'add': $result = $req['a'] + $req['b']; break;
                    case 'sub': $result = $req['a'] - $req['b']; break;
                    case 'mul': $result = $req['a'] * $req['b']; break;
                    case 'div': $result = $req['a'] / $req['b']; break;
                    default: $result = 'NaN';
                }
                $reply_queue = new \AMQPQueue($this->_chan);
                $replyTo = $message->getReplyTo();
                $reply_queue->setName($message->getReplyTo());
                $reply_exch = new \AMQPExchange($this->_chan);
                $reply_exch->setName('temp-exch-' . $message->getReplyTo());
                $reply_exch->setType(AMQP_EX_TYPE_FANOUT);
                $reply_exch->declareExchange();
                $reply_queue->bind($reply_exch->getName());
                $pkt = json_encode(['result' => $result]);
                $reply_exch->publish($pkt, null, AMQP_NOPARAM, [
                    'correlation_id' => $message->getCorrelationId()
                ]);
                $reply_exch->delete();
            }
        );
    }    
}

class TutorialClient {
    private $_conn;
    private $_chan;
    private $_request;
    private $_reply_queue;
    private $_correlation_id;
    
    public function __construct() {
        $this->_conn = new \AMQPConnection(['localhost', 5672, 'guest', 'guest']);
        $this->_conn->connect();
        $this->_chan = new \AMQPChannel($this->_conn);
    }
    
    private function send($msg) {
        // Attach to well known server ETL and send message.
        $request_queue = new \AMQPQueue($this->_chan);
        $request_queue->setName("queue-well-known-rpc-name");
        $request_queue->declareQueue();
        $request_exch = new \AMQPExchange($this->_chan);
        $request_exch->setName("exchange-well-known-rpc-name");
        $request_exch->setType(AMQP_EX_TYPE_FANOUT);
        $request_exch->declareExchange();    
        $request_queue->bind($request_exch->getName());
        $request_exch->publish($msg, null, AMQP_NOPARAM, [
            'correlation_id' => $this->_correlation_id,
            'reply_to' => $this->_reply_queue->getName()
        ]);
        $request_exch->delete();
    }
    
    public function killServer() {
        $this->send("QUIT");
    }
    
    private function call($func, $a, $b) {
        // Create a reply queue the server should send response to.
        $this->_correlation_id = md5(uniqid(__METHOD__, true));
        $this->_reply_queue = new \AMQPQueue($this->_chan);
        $this->_reply_queue->setName('reply-queue-' . $this->_correlation_id);
        $this->_reply_queue->setFlags(AMQP_AUTODELETE);
        $this->_reply_queue->declareQueue();
        
        // Send request as JSON packet.
        $this->send(json_encode([
            'func' => $func,
            'a' => $a,
            'b' => $b
        ]));
        
        $this->_reply_queue->consume(
            function(\AMQPEnvelope $message, \AMQPQueue $queue) {
                $req = json_decode($message->getBody(), true);
                if($message->getCorrelationId() == $this->_correlation_id) {
                    $queue->ack($message->getDeliveryTag());
                    $this->_request = $req;
                    return false; // Break consumer loop
                }
            }
        );
        
        return $this->_request['result'];
    }
    
    public function add($a, $b) {
        return $this->call('add', $a, $b);
    }
    
    public function sub($a, $b) {
        return $this->call('sub', $a, $b);
    }
    
    public function mul($a, $b) {
        return $this->call('mul', $a, $b);
    }
    
    public function div($a, $b) {
        return $this->call('div', $a, $b);
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
    $server = new TutorialServer;
    $server->consume();
}
else { 
   $client = new TutorialClient;
   echo "1 + 2 = " . $client->add(1, 2) . "\n";
   echo "4 - 3 = " . $client->sub(4, 3) . "\n";
   echo "5 * 6 = " . $client->mul(5, 6) . "\n";
   echo "8 / 2 = " . $client->div(8, 2) . "\n";
   $client->killServer();
}
