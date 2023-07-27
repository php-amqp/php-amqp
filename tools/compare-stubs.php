<?php
$symbols = [
    'classes' => [],
    'constants' => [],
    'classNames' => [],
];

function sortByName(array $array): array {
    usort(
        $array,
        function (array $left, array $right) {
            return $left['name'] <=> $right['name'];
        }
    );

    return $array;
}

function getTypeMetadata(?ReflectionType $type): ?array {
    if ($type == null) {
        return null;
    }

    return [
        'name' => (string) $type,
        'nullable' => $type->allowsNull()
    ];
}

const MAGIC_METHODS = [
    '__construct',
    '__toString',
    '__wakeup',
    '__clone',
];

function getClassMetadata(string $class): array {
    $r = new ReflectionClass($class);
    $classMetadata = [
        'name' => $r->getName(),
        'constants' => [],
        'methods' => [],
    ];

    foreach ($r->getReflectionConstants() as $constant) {
        $classMetadata['constants'][] = [
            'name' => $class. '::' . $constant->getName(),
            'value' => $constant->getValue(),
            'visibility' => $constant->isPublic() ? 'public' : ($constant->isProtected() ? 'protected' : ($constant->isPrivate() ? 'private' : 'unknown')),
        ];
    }

    foreach ($r->getMethods() as $method) {
        $methodMetadata = [
            'name' => $class. '::' . $method->getName(),
            'visibility' => $method->isPublic() ? 'public' : ($method->isProtected() ? 'protected' : ($method->isPrivate() ? 'private' : 'unknown')),
            'final' => $method->isFinal(),
            'static' => $method->isStatic(),
            'parameterCount' => $method->getNumberOfParameters(),
            'requiredParameterCount' => $method->getNumberOfRequiredParameters(),
            'return' => getTypeMetadata($method->getReturnType()),
            'parameters' => [],
        ];

        if (!in_array($method->getName(), MAGIC_METHODS, true) && strpos($method->getName(), '_') !== false) {
            printf("ERROR: %s contains underscore\n", $class. '::' . $method->getName());
        }

        foreach ($method->getParameters() as $parameter) {

            if (strpos($parameter->getName(), '_') !== false) {
                printf("ERROR: %s(…%s…) contains underscore\n", $class. '::' . $method->getName(), $parameter->getName());
            }

            $methodMetadata['parameters'][] = [
                'methodName' => $class. '::' . $method->getName(),
                'name' => $parameter->getName(),
                'type' => getTypeMetadata($parameter->getType()),
            ];
        }

        $classMetadata['methods'][] = $methodMetadata;
    }

    $classMetadata['methods'] = sortByName($classMetadata['methods']);
    $classMetadata['constants'] = sortByName($classMetadata['constants']);

    return $classMetadata;
}


spl_autoload_register(function(string $class) {
    require_once __DIR__ . '/../stubs/' . $class . '.php';
});

if (!extension_loaded('amqp')) {
    require_once __DIR__ . '/../stubs/AMQP.php';
}

foreach (get_defined_constants() as $constant => $value) {
    if ($constant === 'AMQP_OS_SOCKET_TIMEOUT_ERRNO') {
        // Value differs across OS, so normalize
        $value = 0xDEADBEEF;
    }

    if (strpos($constant, 'AMQP_') === 0) {
        $symbols['constants'][] = [
            'name' => $constant,
            'value' => $value
        ];
    }
}
$symbols['constants'] = sortByName($symbols['constants']);
$symbols['classes'][] = getClassMetadata('AMQPValueException');
$symbols['classes'][] = getClassMetadata('AMQPTimestamp');
$symbols['classes'][] = getClassMetadata('AMQPQueueException');
$symbols['classes'][] = getClassMetadata('AMQPQueue');
$symbols['classes'][] = getClassMetadata('AMQPExchangeException');
$symbols['classes'][] = getClassMetadata('AMQPExchange');
$symbols['classes'][] = getClassMetadata('AMQPException');
$symbols['classes'][] = getClassMetadata('AMQPEnvelopeException');
$symbols['classes'][] = getClassMetadata('AMQPEnvelope');
$symbols['classes'][] = getClassMetadata('AMQPDecimal');
$symbols['classes'][] = getClassMetadata('AMQPConnectionException');
$symbols['classes'][] = getClassMetadata('AMQPConnection');
$symbols['classes'][] = getClassMetadata('AMQPChannelException');
$symbols['classes'][] = getClassMetadata('AMQPChannel');
$symbols['classes'][] = getClassMetadata('AMQPBasicProperties');

// Ensure we captured all AMQP classes
foreach (get_declared_classes() as $className) {
    if (strpos($className, 'AMQP') === 0) {
        $symbols['classNames'][] = $className;
    }
}
sort($symbols['classNames']);

$filename = $_SERVER['argv'][1] ?? null;
assert($filename !== null, 'Output filename must be passed');

file_put_contents($filename, json_encode($symbols, JSON_PRETTY_PRINT));