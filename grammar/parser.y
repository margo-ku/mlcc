%skeleton "lalr1.cc"
%require "3.5"

%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include "include/ast/declarations.h"

    class Scanner;
    class Driver;

    class BaseElement;
    class TranslationUnit;
    class ItemList;
    class FunctionDefinition;
    class Declarator;
    class Declaration;
    class DeclarationSpecifiers;
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
    class ArgumentExpressionList;
    class ParameterList;
    class ParameterDeclaration;
    class IdentifierDeclarator;
    class FunctionDeclarator;
    class FunctionCallExpression;
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
%token BIT_AND BIT_OR BIT_XOR BIT_LSHIFT BIT_RSHIFT
%token LE LEQ GE GEQ EQ NOT_EQ
%token LPAREN RPAREN LBRACE RBRACE SEMI COLON QUESTION COMMA
%token ASSIGNMENT
%token INT LONG VOID
%token RETURN IF ELSE DO WHILE FOR BREAK CONTINUE
%token <std::string> ID
%token <int> INT_NUMBER
%token <long> LONG_NUMBER
%token <unsigned int> UINT_NUMBER
%token <unsigned long> ULONG_NUMBER
%token SIGNED UNSIGNED
%token STATIC EXTERN

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%type <std::unique_ptr<TranslationUnit>> start
%type <std::unique_ptr<TranslationUnit>> translation_unit
%type <std::unique_ptr<BaseElement>> external_declaration
%type <std::unique_ptr<FunctionDefinition>> function_definition
%type <DeclarationSpecifierSet> declaration_specifier_set
%type <DeclarationSpecifierSet> declaration_specifier_set_opt
%type <std::unique_ptr<DeclarationSpecifiers>> declaration_specifiers
%type <TypeSpecifierSet> type_specifier_list
%type <TypeSpecifierSet::Specifier> type_specifier
%type <StorageClassSpecifierSet::Specifier> storage_class_specifier
%type <std::unique_ptr<Declarator>> declarator
%type <std::unique_ptr<Declarator>> init_declarator
%type <std::unique_ptr<Declaration>> declaration
%type <std::unique_ptr<CompoundStatement>> compound_statement
%type <std::unique_ptr<ItemList>> item_list
%type <std::unique_ptr<BaseElement>> item
%type <std::unique_ptr<Statement>> statement
%type <std::unique_ptr<ReturnStatement>> return_statement
%type <std::unique_ptr<ExpressionStatement>> expression_statement
%type <std::unique_ptr<SelectionStatement>> selection_statement
%type <std::unique_ptr<Expression>> initializer
%type <std::unique_ptr<Expression>> expression
%type <std::unique_ptr<Expression>> primary_expression
%type <std::unique_ptr<Expression>> postfix_expression
%type <std::unique_ptr<Expression>> unary_expression
%type <std::unique_ptr<Expression>> cast_expression
%type <std::unique_ptr<Expression>> additive_expression
%type <std::unique_ptr<Expression>> shift_expression
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
%type <std::unique_ptr<Expression>> and_expression
%type <std::unique_ptr<Expression>> exclusive_or_expression
%type <std::unique_ptr<Expression>> inclusive_or_expression
%type <std::unique_ptr<ArgumentExpressionList>> argument_expression_list
%type <std::unique_ptr<ParameterList>> parameter_list
%type <std::unique_ptr<ParameterDeclaration>> parameter_declaration

%%
%start start;
start:
    translation_unit END { driver.SetTranslationUnit(std::move($1)); };

translation_unit:
    external_declaration { $$ = std::make_unique<TranslationUnit>(); $$->AddExternalDeclaration(std::move($1)); }
    | translation_unit external_declaration { $1->AddExternalDeclaration(std::move($2)); $$ = std::move($1); };

external_declaration:
    function_definition { $$ = std::move($1); }
    | declaration { $$ = std::move($1); };

function_definition:
    declaration_specifiers declarator compound_statement { $$ = std::make_unique<FunctionDefinition>(std::move($1), std::move($2), std::move($3)); };

declaration_specifiers:
    declaration_specifier_set { $$ = std::make_unique<DeclarationSpecifiers>($1); };

declaration_specifier_set:
    storage_class_specifier declaration_specifier_set_opt { $$ = $2; $$.Add($1); }
    | type_specifier declaration_specifier_set_opt { $$ = $2; $$.Add($1); };

declaration_specifier_set_opt:
    %empty { $$ = DeclarationSpecifierSet{}; }
    | declaration_specifier_set { $$ = $1; };

type_specifier_list:
    type_specifier { $$ = TypeSpecifierSet{}; $$.Add($1); }
    | type_specifier_list type_specifier { $$ = $1; $$.Add($2); };

storage_class_specifier:
    STATIC { $$ = StorageClassSpecifierSet::Specifier::Static; }
    | EXTERN { $$ = StorageClassSpecifierSet::Specifier::Extern; };

type_specifier:
    INT { $$ = TypeSpecifierSet::Specifier::Int; }
    | LONG { $$ = TypeSpecifierSet::Specifier::Long; }
    | SIGNED { $$ = TypeSpecifierSet::Specifier::Signed; }
    | UNSIGNED { $$ = TypeSpecifierSet::Specifier::Unsigned; }

declarator:
    ID { $$ = std::make_unique<IdentifierDeclarator>($1); }
    | declarator LPAREN RPAREN { $$ = std::make_unique<FunctionDeclarator>(std::move($1)); }
    | declarator LPAREN VOID RPAREN { $$ = std::make_unique<FunctionDeclarator>(std::move($1)); }
    | declarator LPAREN parameter_list RPAREN { $$ = std::make_unique<FunctionDeclarator>(std::move($1), std::move($3)); };

declaration:
    declaration_specifiers init_declarator SEMI { $$ = std::make_unique<Declaration>(std::move($1), std::move($2)); };

init_declarator:
    declarator { $$ = std::move($1); }
    | declarator ASSIGNMENT initializer { $1->SetInitializer(std::move($3)); $$ = std::move($1); };

initializer:
    assignment_expression { $$ = std::move($1); };

parameter_list:
    parameter_declaration { $$ = std::make_unique<ParameterList>(); $$->AddParameter(std::move($1)); }
    | parameter_list COMMA parameter_declaration { $1->AddParameter(std::move($3)); $$ = std::move($1); };

parameter_declaration:
    declaration_specifiers declarator { $$ = std::make_unique<ParameterDeclaration>(std::move($1), std::move($2)); };

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
    BREAK SEMI { $$ = std::make_unique<JumpStatement>(JumpStatement::JumpType::Break); }
    | CONTINUE SEMI { $$ = std::make_unique<JumpStatement>(JumpStatement::JumpType::Continue); };

iteration_statement:
    WHILE LPAREN expression RPAREN statement { $$ = std::make_unique<WhileStatement>(WhileStatement::LoopType::While, std::move($3), std::move($5)); }
    | DO statement WHILE LPAREN expression RPAREN SEMI { $$ = std::make_unique<WhileStatement>(WhileStatement::LoopType::DoWhile, std::move($5), std::move($2)); }
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
    | logical_or_expression OR logical_and_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Or, std::move($1), std::move($3)); };

logical_and_expression:
    inclusive_or_expression { $$ = std::move($1); }
    | logical_and_expression AND inclusive_or_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::And, std::move($1), std::move($3)); };

inclusive_or_expression:
    exclusive_or_expression { $$ = std::move($1); }
    | inclusive_or_expression BIT_OR exclusive_or_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::BitwiseOr, std::move($1), std::move($3)); };

exclusive_or_expression:
    and_expression { $$ = std::move($1); } 
    | exclusive_or_expression BIT_XOR and_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::BitwiseXor, std::move($1), std::move($3)); };

and_expression:
    equality_expression { $$ = std::move($1); }
    | and_expression BIT_AND equality_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::BitwiseAnd, std::move($1), std::move($3)); };

equality_expression:
    relation_expression { $$ = std::move($1); }
    | equality_expression EQ relation_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Equal, std::move($1), std::move($3)); }
    | equality_expression NOT_EQ relation_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::NotEqual, std::move($1), std::move($3)); };

relation_expression:
    shift_expression { $$ = std::move($1); }
    | relation_expression LE shift_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Less, std::move($1), std::move($3)); }
    | relation_expression GE shift_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Greater, std::move($1), std::move($3)); }
    | relation_expression LEQ shift_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::LessEqual, std::move($1), std::move($3)); }
    | relation_expression GEQ shift_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::GreaterEqual, std::move($1), std::move($3)); };

shift_expression:
    additive_expression { $$ = std::move($1); }
    | shift_expression BIT_LSHIFT additive_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::LeftShift, std::move($1), std::move($3)); }
    | shift_expression BIT_RSHIFT additive_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::RightShift, std::move($1), std::move($3)); };

additive_expression:
    multiplicative_expression { $$ = std::move($1); }
    | additive_expression PLUS multiplicative_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Plus, std::move($1), std::move($3)); }
    | additive_expression MINUS multiplicative_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Minus, std::move($1), std::move($3)); };

multiplicative_expression:
    cast_expression { $$ = std::move($1); }
    | multiplicative_expression STAR cast_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Mul, std::move($1), std::move($3)); }
    | multiplicative_expression SLASH cast_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Div, std::move($1), std::move($3)); }
    | multiplicative_expression MOD cast_expression { $$ = std::make_unique<BinaryExpression>(BinaryExpression::BinaryOperator::Mod, std::move($1), std::move($3)); };

cast_expression:
    unary_expression { $$ = std::move($1); }
    | LPAREN type_specifier_list RPAREN cast_expression { $$ = std::make_unique<CastExpression>(std::make_unique<TypeSpecification>($2), std::move($4)); };

unary_expression:
    postfix_expression { $$ = std::move($1); }
    | PLUS unary_expression { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryOperator::Plus, std::move($2)); }
    | MINUS unary_expression { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryOperator::Minus, std::move($2)); }
    | BIT_NOT unary_expression { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryOperator::BinaryNot, std::move($2)); }
    | NOT unary_expression { $$ = std::make_unique<UnaryExpression>(UnaryExpression::UnaryOperator::Not, std::move($2)); };

postfix_expression:
    primary_expression { $$ = std::move($1); }
    | postfix_expression LPAREN RPAREN { $$ = std::make_unique<FunctionCallExpression>(std::move($1)); }
    | postfix_expression LPAREN argument_expression_list RPAREN { $$ = std::make_unique<FunctionCallExpression>(std::move($1), std::move($3)); };

primary_expression:
    ID { $$ = std::make_unique<IdExpression>($1); }
    | INT_NUMBER { $$ = std::make_unique<PrimaryExpression>($1); }
    | LONG_NUMBER { $$ = std::make_unique<PrimaryExpression>($1); }
    | UINT_NUMBER { $$ = std::make_unique<PrimaryExpression>($1); }
    | ULONG_NUMBER { $$ = std::make_unique<PrimaryExpression>($1); }
    | LPAREN expression RPAREN { $$ = std::move($2); };

argument_expression_list:
    assignment_expression { $$ = std::make_unique<ArgumentExpressionList>(); $$->AddArgument(std::move($1)); }
    | argument_expression_list COMMA assignment_expression { $1->AddArgument(std::move($3)); $$ = std::move($1); };

%%

void yy::parser::error(const location_type& l, const std::string& m) {
  std::cerr << "Parsing error at line " << l.begin.line << ", column " << l.begin.column << ": " << m << std::endl;
}
