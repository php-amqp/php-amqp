<?php

function dump_methods($obj) {
    $methods = get_class_methods($obj);

    echo get_class($obj), PHP_EOL;

    foreach($methods as $m) {
        if ($m[0] == '_') {
            // ignore all _* methods
            continue;
        }

        echo '    ', $m, '():', PHP_EOL, '    ', '    ';
        var_dump($obj->$m());
    }
}

function dump_message($msg) {
    if (!$msg) {
        var_dump($msg);
        return false;
    }

    echo get_class($msg), PHP_EOL;
    echo "    getBody:", PHP_EOL, "        ";
    var_dump($msg->getBody());
    echo "    getContentType:", PHP_EOL, "        ";
    var_dump($msg->getContentType());
    echo "    getRoutingKey:", PHP_EOL, "        ";
    var_dump($msg->getRoutingKey());
    echo "    getConsumerTag:", PHP_EOL, "        ";
    var_dump($msg->getConsumerTag());
    echo "    getDeliveryTag:", PHP_EOL, "        ";
    var_dump($msg->getDeliveryTag());
    echo "    getDeliveryMode:", PHP_EOL, "        ";
    var_dump($msg->getDeliveryMode());
    echo "    getExchangeName:", PHP_EOL, "        ";
    var_dump($msg->getExchangeName());
    echo "    isRedelivery:", PHP_EOL, "        ";
    var_dump($msg->isRedelivery());
    echo "    getContentEncoding:", PHP_EOL, "        ";
    var_dump($msg->getContentEncoding());
    echo "    getType:", PHP_EOL, "        ";
    var_dump($msg->getType());
    echo "    getTimeStamp:", PHP_EOL, "        ";
    var_dump($msg->getTimeStamp());
    echo "    getPriority:", PHP_EOL, "        ";
    var_dump($msg->getPriority());
    echo "    getExpiration:", PHP_EOL, "        ";
    var_dump($msg->getExpiration());
    echo "    getUserId:", PHP_EOL, "        ";
    var_dump($msg->getUserId());
    echo "    getAppId:", PHP_EOL, "        ";
    var_dump($msg->getAppId());
    echo "    getMessageId:", PHP_EOL, "        ";
    var_dump($msg->getMessageId());
    echo "    getReplyTo:", PHP_EOL, "        ";
    var_dump($msg->getReplyTo());
    echo "    getCorrelationId:", PHP_EOL, "        ";
    var_dump($msg->getCorrelationId());
    echo "    getHeaders:", PHP_EOL, "        ";
    var_dump($msg->getHeaders());

    return false;
}

function consumeThings($message, $queue) {
    var_dump($message);
    return false;
}
