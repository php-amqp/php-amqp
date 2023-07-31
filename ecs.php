<?php

declare(strict_types=1);

use PHP_CodeSniffer\Standards\Squiz\Sniffs\WhiteSpace\LanguageConstructSpacingSniff;
use PHP_CodeSniffer\Standards\Squiz\Sniffs\WhiteSpace\SuperfluousWhitespaceSniff;
use PhpCsFixer\Fixer\Basic\PsrAutoloadingFixer;
use PhpCsFixer\Fixer\CastNotation\CastSpacesFixer;
use PhpCsFixer\Fixer\ClassNotation\ClassAttributesSeparationFixer;
use PhpCsFixer\Fixer\ClassNotation\FinalClassFixer;
use PhpCsFixer\Fixer\ClassNotation\NoBlankLinesAfterClassOpeningFixer;
use PhpCsFixer\Fixer\ClassNotation\SingleTraitInsertPerStatementFixer;
use PhpCsFixer\Fixer\FunctionNotation\FunctionTypehintSpaceFixer;
use PhpCsFixer\Fixer\FunctionNotation\MethodArgumentSpaceFixer;
use PhpCsFixer\Fixer\FunctionNotation\ReturnTypeDeclarationFixer;
use PhpCsFixer\Fixer\FunctionNotation\StaticLambdaFixer;
use PhpCsFixer\Fixer\Import\FullyQualifiedStrictTypesFixer;
use PhpCsFixer\Fixer\Import\GlobalNamespaceImportFixer;
use PhpCsFixer\Fixer\Import\NoLeadingImportSlashFixer;
use PhpCsFixer\Fixer\Import\NoUnusedImportsFixer;
use PhpCsFixer\Fixer\Import\OrderedImportsFixer;
use PhpCsFixer\Fixer\NamespaceNotation\NoLeadingNamespaceWhitespaceFixer;
use PhpCsFixer\Fixer\Operator\BinaryOperatorSpacesFixer;
use PhpCsFixer\Fixer\Operator\ConcatSpaceFixer;
use PhpCsFixer\Fixer\Operator\TernaryOperatorSpacesFixer;
use PhpCsFixer\Fixer\Phpdoc\GeneralPhpdocAnnotationRemoveFixer;
use PhpCsFixer\Fixer\Phpdoc\PhpdocAlignFixer;
use PhpCsFixer\Fixer\Phpdoc\PhpdocIndentFixer;
use PhpCsFixer\Fixer\Phpdoc\PhpdocSingleLineVarSpacingFixer;
use PhpCsFixer\Fixer\PhpTag\BlankLineAfterOpeningTagFixer;
use PhpCsFixer\Fixer\Semicolon\NoSinglelineWhitespaceBeforeSemicolonsFixer;
use PhpCsFixer\Fixer\Semicolon\SpaceAfterSemicolonFixer;
use PhpCsFixer\Fixer\Whitespace\MethodChainingIndentationFixer;
use PhpCsFixer\Fixer\Whitespace\NoSpacesAroundOffsetFixer;
use PhpCsFixer\Fixer\Whitespace\NoWhitespaceInBlankLineFixer;
use SlevomatCodingStandard\Sniffs\Namespaces\ReferenceUsedNamesOnlySniff;
use Symplify\CodingStandard\Fixer\Spacing\StandaloneLinePromotedPropertyFixer;
use Symplify\EasyCodingStandard\Config\ECSConfig;
use Symplify\EasyCodingStandard\ValueObject\Set\SetList;

return static function (ECSConfig $ecsConfig): void {
    $ecsConfig->paths([__DIR__ . '/stubs', __DIR__ . '/infra']);

    // this way you add a single rule
    $ecsConfig->rules(
        [
            NoUnusedImportsFixer::class,
            PhpdocIndentFixer::class,
            OrderedImportsFixer::class,
            FullyQualifiedStrictTypesFixer::class,
            GlobalNamespaceImportFixer::class,
            NoLeadingImportSlashFixer::class,
            StaticLambdaFixer::class,
            ReferenceUsedNamesOnlySniff::class,
        ]
    );

    $ecsConfig->rules(
        [
            StandaloneLinePromotedPropertyFixer::class,
            BlankLineAfterOpeningTagFixer::class,
            MethodChainingIndentationFixer::class,
            CastSpacesFixer::class,
            ClassAttributesSeparationFixer::class,
            SingleTraitInsertPerStatementFixer::class,
            FunctionTypehintSpaceFixer::class,
            NoBlankLinesAfterClassOpeningFixer::class,
            NoSinglelineWhitespaceBeforeSemicolonsFixer::class,
            PhpdocSingleLineVarSpacingFixer::class,
            NoLeadingNamespaceWhitespaceFixer::class,
            NoSpacesAroundOffsetFixer::class,
            NoWhitespaceInBlankLineFixer::class,
            ReturnTypeDeclarationFixer::class,
            SpaceAfterSemicolonFixer::class,
            TernaryOperatorSpacesFixer::class,
            MethodArgumentSpaceFixer::class,
            LanguageConstructSpacingSniff::class,
        ]
    );
    $ecsConfig->ruleWithConfiguration(ClassAttributesSeparationFixer::class, [
        'elements' => [
            'const' => 'one',
            'property' => 'one',
            'method' => 'one',
        ],
    ]);
    $ecsConfig->ruleWithConfiguration(ConcatSpaceFixer::class, [
        'spacing' => 'one',
    ]);
    $ecsConfig->ruleWithConfiguration(SuperfluousWhitespaceSniff::class, [
        'ignoreBlankLines' => false,
    ]);
    $ecsConfig->ruleWithConfiguration(BinaryOperatorSpacesFixer::class, [
        'operators' => [
            '=>' => 'single_space',
            '=' => 'single_space',
        ],
    ]);

    $ecsConfig->sets([
        SetList::CLEAN_CODE,
        SetList::ARRAY,
        SetList::DOCBLOCK,
        SetList::NAMESPACES,
        SetList::COMMENTS,
        SetList::CONTROL_STRUCTURES,
        SetList::SYMPLIFY,
        SetList::PSR_12,
    ]);

    $ecsConfig->ruleWithConfiguration(GeneralPhpdocAnnotationRemoveFixer::class, [
        'annotations' => ['author', 'package', 'category'],
    ]);
    $ecsConfig->ruleWithConfiguration(PhpdocAlignFixer::class, [
        'align' => 'left',
    ]);
};