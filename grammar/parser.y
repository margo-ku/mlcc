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
    class JumpStatement;
    class WhileStatement;
    class ForStatement;
};

%define parse.trace
%define parse.error verbose

%code {
    #include <memory>

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
%token INT VOID
%token RETURN IF ELSE DO WHILE FOR BREAK CONTINUE
%token <std::string> ID
%token <int> NUMBER

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%type <std::unique_ptr<TranslationUnit>> start
%type <std::unique_ptr<TranslationUnit>> translation_unit
%type <std::unique_ptr<BaseElement>> external_declaration
%type <std::unique_ptr<FunctionDefinition>> function_definition
%type <std::unique_ptr<TypeSpecification>> declaration_specifiers
%type <std::unique_ptr<Declarator>> declarator
%type <std::unique_ptr<InitDeclarator>> init_declarator
%type <std::unique_ptr<Declaration>> declaration
%type <std::unique_ptr<CompoundStatement>> compound_statement
%type <std::unique_ptr<ItemList>> item_list
%type <std::unique_ptr<BaseElement>> item
%type <std::unique_ptr<Statement>> statement
%type <std::unique_ptr<ReturnStatement>> return_statement
%type <std::unique_ptr<ExpressionStatement>> expression_statement
%type <std::unique_ptr<SelectionStatement>> selection_statement
%type <std::unique_ptr<TypeSpecification>> type_specifier
%type <std::unique_ptr<Expression>> initializer
%type <std::unique_ptr<Expression>> expression
%type <std::unique_ptr<Expression>> primary_expression
%type <std::unique_ptr<Expression>> unary_expression
%type <std::unique_ptr<Expression>> additive_expression
%type <std::unique_ptr<Expression>> multiplicative_expression
%type <std::unique_ptr<Expression>> relation_expression
%type <std::unique_ptr<Expression>> equality_expression
%type <std::unique_ptr<Expression>> logical_and_expression
%type <std::unique_ptr<Expression>> logical_or_expression
%type <std::unique_ptr<Expression>> conditional_expression
%type <std::unique_ptr<Expression>> assignment_expression
%type <std::unique_ptr<JumpStatement>> jump_statement
%type <std::unique_ptr<Statement>> iteration_statement
%type <std::unique_ptr<BaseElement>> for_init
%type <std::unique_ptr<Expression>> expression_opt

%%
%start start;
start:
    translation_unit { driver.SetTranslationUnit(std::move($1)); };

translation_unit:
    external_declaration { $$ = std::make_unique<TranslationUnit>(); $$->AddExternalDeclaration(std::move($1)); }
    | external_declaration translation_unit { $2->AddExternalDeclaration(std::move($1)); $$ = std::move($2); };

external_declaration:
    function_definition { $$ = std::move($1); };

function_definition:
    declaration_specifiers declarator compound_statement { $$ = std::make_unique<FunctionDefinition>(std::move($1), std::move($2), std::move($3)); };

declaration_specifiers:
    type_specifier { $$ = std::move($1); };

type_specifier:
    INT { $$ = std::make_unique<TypeSpecification>("int"); };

declarator:
    ID { $$ = std::make_unique<Declarator>($1); }
    | declarator LPAREN RPAREN { $$ = std::move($1); }
    | declarator LPAREN VOID RPAREN { $$ = std::move($1); };

declaration:
    declaration_specifiers init_declarator SEMI { $$ = std::make_unique<Declaration>(std::move($1), std::move($2)); };

init_declarator:
    declarator { $$ = std::make_unique<InitDeclarator>(std::move($1)); }
    | declarator ASSIGNMENT initializer { $$ = std::make_unique<InitDeclarator>(std::move($1), std::move($3)); };

initializer:
    assignment_expression { $$ = std::move($1); };

compound_statement:
    LBRACE item_list RBRACE { $$ = std::make_unique<CompoundStatement>(std::move($2)); };

item_list:
    %empty { $$ = std::make_unique<ItemList>(); }
    | item_list item { $1->AddItem(std::move($2)); $$ = std::move($1); };

item:
    statement { $$ = std::move($1); }
    | declaration { $$ = std::move($1); };

statement:
    expression_statement { $$ = std::move($1); }
    | selection_statement { $$ = std::move($1); }
    | compound_statement { $$ = std::move($1); }
    | return_statement { $$ = std::move($1); }
    | jump_statement { $$ = std::move($1); }
    | iteration_statement { $$ = std::move($1); };

selection_statement:
    IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE { $$ = std::make_unique<SelectionStatement>(std::move($3), std::move($5)); }
    | IF LPAREN expression RPAREN statement ELSE statement { $$ = std::make_unique<SelectionStatement>(std::move($3), std::move($5), std::move($7)); };

expression_statement:
    SEMI { $$ = std::make_unique<ExpressionStatement>(); }
    | expression SEMI { $$ = std::make_unique<ExpressionStatement>(std::move($1)); };

return_statement:
    RETURN expression SEMI { $$ = std::make_unique<ReturnStatement>(std::move($2)); };

jump_statement:
    BREAK SEMI { $$ = std::make_unique<JumpStatement>(JumpStatement::JumpType::kBreak); }
    | CONTINUE SEMI { $$ = std::make_unique<JumpStatement>(JumpStatement::JumpType::kContinue); };

iteration_statement:
    WHILE LPAREN expression RPAREN statement { $$ = std::make_unique<WhileStatement>(WhileStatement::LoopType::kWhile, std::move($3), std::move($5)); }
    | DO statement WHILE LPAREN expression RPAREN SEMI { $$ = std::make_unique<WhileStatement>(WhileStatement::LoopType::kDoWhile, std::move($5), std::move($2)); }
    | FOR LPAREN for_init expression_opt SEMI expression_opt RPAREN statement { $$ = std::make_unique<ForStatement>(std::move($3), std::move($4), std::move($6), std::move($8)); };

for_init:
    declaration { $$ = std::move($1); }
    | expression_opt SEMI { $$ = std::move($1); };

expression_opt:
    %empty { $$ = std::make_unique<Expression>(); }
    | expression { $$ = std::move($1); };

expression:
    assignment_expression { $$ = std::move($1); };

assignment_expression:
    conditional_expression { $$ = std::move($1); }
    | unary_expression ASSIGNMENT assignment_expression { $$ = std::make_unique<AssignmentExpression>(std::move($1), std::move($3)); };

conditional_expression:
    logical_or_expression { $$ = std::move($1); }
    | logical_or_expression QUESTION expression COLON conditional_expression { $$ = std::make_unique<ConditionalExpression>(std::move($1), std::move($3), std::move($5)); };

logical_or_expression:
    logical_and_expression { $$ = std::move($1); }
    | logical_or_expression OR logical_and_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kOr, std::move($1), std::move($3)); };

logical_and_expression:
    equality_expression { $$ = std::move($1); }
    | logical_and_expression AND equality_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kAnd, std::move($1), std::move($3)); };

equality_expression:
    relation_expression { $$ = std::move($1); }
    | equality_expression EQ relation_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kEqual, std::move($1), std::move($3)); }
    | equality_expression NOT_EQ relation_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kNotEqual, std::move($1), std::move($3)); };

relation_expression:
    additive_expression { $$ = std::move($1); }
    | relation_expression LE additive_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kLess, std::move($1), std::move($3)); }
    | relation_expression GE additive_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kGreater, std::move($1), std::move($3)); }
    | relation_expression LEQ additive_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kLessEqual, std::move($1), std::move($3)); }
    | relation_expression GEQ additive_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kGreaterEqual, std::move($1), std::move($3)); };

additive_expression:
    multiplicative_expression { $$ = std::move($1); }
    | additive_expression PLUS multiplicative_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kPlus, std::move($1), std::move($3)); }
    | additive_expression MINUS multiplicative_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kMinus, std::move($1), std::move($3)); };

multiplicative_expression:
    unary_expression { $$ = std::move($1); }
    | multiplicative_expression STAR unary_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kMul, std::move($1), std::move($3)); }
    | multiplicative_expression SLASH unary_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kDiv, std::move($1), std::move($3)); }
    | multiplicative_expression MOD unary_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::kMod, std::move($1), std::move($3)); };

unary_expression:
    primary_expression { $$ = std::move($1); }
    | PLUS unary_expression { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryOperator::kPlus, std::move($2)); }
    | MINUS unary_expression { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryOperator::kMinus, std::move($2)); }
    | BIT_NOT unary_expression { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryOperator::kBinaryNot, std::move($2)); }
    | NOT unary_expression { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryOperator::kNot, std::move($2)); };

primary_expression:
    ID { $$ = std::make_unique<IdExpression>($1); }
    | NUMBER { $$ = std::make_unique<PrimaryExpression>($1); }
    | LPAREN expression RPAREN { $$ = std::move($2); };

%%

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << l << ": " << m << '\n';
}
