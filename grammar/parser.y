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
    class TypeSpecification;
    class Statement;
    class CompoundStatement;
    class ReturnStatement;
    class Expression;
    class PrimaryExpression;
    class UnaryExpression;
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
%token PLUS MINUS BIT_NOT NOT
%token LPAREN RPAREN LBRACE RBRACE SEMI
%token INT
%token RETURN
%token <std::string> ID
%token <int> NUMBER

%type <TranslationUnit*> start
%type <TranslationUnit*> translation_unit
%type <BaseElement*> external_declaration;
%type <FunctionDefinition*> function_definition;
%type <TypeSpecification*> declaration_specifiers;
%type <Declarator*> declarator;
%type <CompoundStatement*> compound_statement;
%type <ItemList*> item_list;
%type <BaseElement*> item;
%type <Statement*> statement;
%type <ReturnStatement*> return_statement;
%type <TypeSpecification*> type_specifier;
%type <Expression*> expression;
%type <PrimaryExpression*> primary_expression;
%type <UnaryExpression*> unary_expression;
%type <char> unary_operator;

%%
%start start;
start:
    translation_unit { $$ = $1; driver.translation_unit = $$; };

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
    ID LPAREN RPAREN { $$ = new Declarator($1); };

compound_statement:
    LBRACE item_list RBRACE { $$ = new CompoundStatement($2); };

item_list:
    %empty { $$ = new ItemList(); }
    | item_list item { $1->AddItem($2); $$ = $1; };

item:
    statement { $$ = $1; };

statement:
    return_statement { $$ = $1; };

return_statement:
    RETURN expression SEMI { $$ = new ReturnStatement($2); };

expression:
    primary_expression { $$ = $1; }
    | unary_expression { $$ = $1; };

unary_expression:
    unary_operator expression { $$ = new UnaryExpression($1, $2); };

unary_operator:
    PLUS { $$ = '+'; }
    | MINUS { $$ = '-'; }
    | BIT_NOT { $$ = '~'; }
    | NOT { $$ = '!'; };

primary_expression:
    NUMBER { $$ = new PrimaryExpression($1); };

%%

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
