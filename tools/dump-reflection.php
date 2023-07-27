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

$error = false;
function error(string $message, ...$args): void
{
    global $error;
    vprintf('ERROR: ' . $message, $args);
    $error = true;
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
            error("%s contains underscore\n", $class. '::' . $method->getName());
        }

        $prefix = substr($method->getName(), 0, 3);
        if (in_array($prefix, ['set', 'has', 'get'], true) && !in_array($method->getName(), ['set', 'has', 'get'], true)) {
            $hasPlural = $r->hasMethod($method->getName() . 's');
            $isPlural = substr($method->getName(), -1) === 's';

            if ($prefix === 'get' && $method->getNumberOfParameters() !== 0 && !$hasPlural) {
                error("%s() should have no arguments\n", $class. '::' . $method->getName());
            }

            if ($prefix === 'set' && $isPlural) {
                if ($method->getParameters()[0]->getName() !== ($expectedName = lcfirst(substr($method->getName(), 3)))) {
                    error("%s(%s) must be \"%s\"\n", $class. '::' . $method->getName(), $method->getParameters()[0]->getName(), $expectedName);
                }
            }

            if ($prefix === 'get' && $hasPlural) {
                if ($method->getNumberOfRequiredParameters() !== 1 || $method->getNumberOfParameters() !== 1) {
                    error("%s() should have exactly one required parameter\n", $class. '::' . $method->getName());
                }
            }

            if ($hasPlural) {
                if ($method->getParameters()[0]->getName() !== ($expectedName = lcfirst(substr($method->getName(), 3) . 'Name'))) {
                    error("%s(%s) must be \"%s\"\n", $class. '::' . $method->getName(), $method->getParameters()[0]->getName(), $expectedName);
                }

                if ($prefix === 'set' && $method->getParameters()[1]->getName() !== ($expectedName = lcfirst(substr($method->getName(), 3) . 'Value'))) {
                    error("%s(…, %s) must be \"%s\"\n", $class. '::' . $method->getName(), $method->getParameters()[1]->getName(), $expectedName);
                }
            }
        }

        foreach ($method->getParameters() as $parameter) {
            if (strpos($parameter->getName(), '_') !== false) {
                error("%s(…%s…) contains underscore\n", $class. '::' . $method->getName(), $parameter->getName());
            }

            $default = 'unknown';
            if (version_compare(PHP_VERSION, '8.0.0', '>=')) {
                // Cannot set default values before 8.0 in native extension
                $default = $parameter->isDefaultValueAvailable() ? $parameter->getDefaultValue() : 'none';
            }

            $methodMetadata['parameters'][] = [
                'methodName' => $class. '::' . $method->getName(),
                'name' => $parameter->getName(),
                'default' => $default,
                'type' => getTypeMetadata($parameter->getType()),
                'byRef' => $parameter->isPassedByReference(),
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

if ($error) {
    exit(1);
}