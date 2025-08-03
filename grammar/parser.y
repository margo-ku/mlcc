%skeleton "lalr1.cc"
%require "3.5"

%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    class Scanner;
    class Driver;

    class BaseElement;
    class TranslationUnit;
    class ItemList;
    class FunctionDefinition;
    class Declarator;
    class InitDeclarator;
    class Declaration;
    class TypeSpecification;
    class Statement;
    class CompoundStatement;
    class ReturnStatement;
    class ExpressionStatement;
    class SelectionStatement;
    class Expression;
    class PrimaryExpression;
    class IdExpression;
    class UnaryExpression;
    class BinaryExpression;
    class AssignmentExpression;
    class ConditionalExpression;
};

%define parse.trace
%define parse.error verbose

%code {
    #include "include/driver/driver.h"
    #include "location.hh"

    #include "include/ast/translation_unit.h"
    #include "include/ast/declarations.h"
    #include "include/ast/expressions.h"
    #include "include/ast/statements.h"

    static yy::parser::symbol_type yylex(Scanner &scanner, Driver& driver) {
        return scanner.ScanToken();
    }
};

%lex-param { Scanner &scanner }
%lex-param { Driver &driver }
%parse-param { Scanner &scanner }
%parse-param { Driver &driver }

%locations

%define api.token.prefix {TOK_}

%token END 0 "end of file"
%token PLUS MINUS STAR SLASH MOD BIT_NOT
%token OR AND NOT
%token LE LEQ GE GEQ EQ NOT_EQ
%token LPAREN RPAREN LBRACE RBRACE SEMI COLON QUESTION
%token ASSIGNMENT
%token INT
%token RETURN IF ELSE
%token <std::string> ID
%token <int> NUMBER

%type <TranslationUnit*> start
%type <TranslationUnit*> translation_unit
%type <BaseElement*> external_declaration;
%type <FunctionDefinition*> function_definition;
%type <TypeSpecification*> declaration_specifiers;
%type <Declarator*> declarator;
%type <InitDeclarator*> init_declarator;
%type <Declaration*> declaration;
%type <CompoundStatement*> compound_statement;
%type <ItemList*> item_list;
%type <BaseElement*> item;
%type <Statement*> statement;
%type <ReturnStatement*> return_statement;
%type <ExpressionStatement*> expression_statement;
%type <SelectionStatement*> selection_statement;
%type <TypeSpecification*> type_specifier;
%type <Expression*> initializer;
%type <Expression*> expression;
%type <Expression*> primary_expression;
%type <Expression*> unary_expression;
%type <Expression*> additive_expression;
%type <Expression*> multiplicative_expression;
%type <Expression*> relation_expression;
%type <Expression*> equality_expression;
%type <Expression*> logical_and_expression;
%type <Expression*> logical_or_expression;
%type <Expression*> conditional_expression;
%type <Expression*> assignment_expression;

%%
%start start;
start:
    translation_unit { $$ = $1; driver.SetTranslationUnit($$); };

translation_unit:
    external_declaration { $$ = new TranslationUnit(); $$->AddExternalDeclaration($1); }
    | external_declaration translation_unit { $2->AddExternalDeclaration($1); $$ = $2; };

external_declaration:
    function_definition { $$ = $1; };

function_definition:
    declaration_specifiers declarator compound_statement { $$ = new FunctionDefinition($1, $2, $3); };

declaration_specifiers:
    type_specifier { $$ = $1; };

type_specifier:
    INT { $$ = new TypeSpecification("int"); };

declarator:
    ID { $$ = new Declarator($1); }
    | declarator LPAREN RPAREN { $$ = $1; };

declaration:
    declaration_specifiers init_declarator SEMI { $$ = new Declaration($1, $2); };

init_declarator:
    declarator { $$ = new InitDeclarator($1); }
    | declarator ASSIGNMENT initializer { $$ = new InitDeclarator($1, $3); };

initializer:
    assignment_expression { $$ = $1; };

compound_statement:
    LBRACE item_list RBRACE { $$ = new CompoundStatement($2); };

item_list:
    %empty { $$ = new ItemList(); }
    | item_list item { $1->AddItem($2); $$ = $1; };

item:
    statement { $$ = $1; }
    | declaration { $$ = $1; };

statement:
    expression_statement { $$ = $1; }
    | selection_statement { $$ = $1; }
    | compound_statement { $$ = $1; }
    | return_statement { $$ = $1; };

selection_statement:
    IF LPAREN expression RPAREN statement { $$ = new SelectionStatement($3, $5); }
    | IF LPAREN expression RPAREN statement ELSE statement { $$ = new SelectionStatement($3, $5, $7); }

expression_statement:
    SEMI { $$ = new ExpressionStatement(); }
    | expression SEMI { $$ = new ExpressionStatement($1); };

return_statement:
    RETURN expression SEMI { $$ = new ReturnStatement($2); };

expression:
    assignment_expression { $$ = $1; }

assignment_expression:
    conditional_expression { $$ = $1; }
    | unary_expression ASSIGNMENT assignment_expression { $$ = new AssignmentExpression($1, $3); };

conditional_expression:
    logical_or_expression { $$ = $1; }
    | logical_or_expression QUESTION expression COLON conditional_expression { $$ = new ConditionalExpression($1, $3, $5); };

logical_or_expression:
    logical_and_expression { $$ = $1; }
    | logical_or_expression OR logical_and_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kOr, $1, $3); };

logical_and_expression:
    equality_expression { $$ = $1; }
    | logical_and_expression AND equality_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kAnd, $1, $3); };

equality_expression:
    relation_expression { $$ = $1; }
    | equality_expression EQ relation_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kEqual, $1, $3); }
    | equality_expression NOT_EQ relation_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kNotEqual, $1, $3); };

relation_expression:
    additive_expression { $$ = $1; }
    | relation_expression LE additive_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kLess, $1, $3); }
    | relation_expression GE additive_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kGreater, $1, $3); }
    | relation_expression LEQ additive_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kLessEqual, $1, $3); }
    | relation_expression GEQ additive_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kGreaterEqual, $1, $3); };

additive_expression:
    multiplicative_expression { $$ = $1; }
    | additive_expression PLUS multiplicative_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kPlus, $1, $3); }
    | additive_expression MINUS multiplicative_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kMinus, $1, $3); };

multiplicative_expression:
    unary_expression { $$ = $1; }
    | multiplicative_expression STAR unary_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kMul, $1, $3); }
    | multiplicative_expression SLASH unary_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kDiv, $1, $3); }
    | multiplicative_expression MOD unary_expression { $$ = new BinaryExpression(BinaryExpression::BinaryOperator::kMod, $1, $3); };

unary_expression:
    primary_expression { $$ = $1; }
    | PLUS unary_expression { $$ = new UnaryExpression(UnaryExpression::UnaryOperator::kPlus, $2); }
    | MINUS unary_expression { $$ = new UnaryExpression(UnaryExpression::UnaryOperator::kMinus, $2); }
    | BIT_NOT unary_expression { $$ = new UnaryExpression(UnaryExpression::UnaryOperator::kBinaryNot, $2); }
    | NOT unary_expression { $$ = new UnaryExpression(UnaryExpression::UnaryOperator::kNot, $2); }

primary_expression:
    ID { $$ = new IdExpression($1); }
    | NUMBER { $$ = new PrimaryExpression($1); }
    | LPAREN expression RPAREN { $$ = $2; };

%%

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
