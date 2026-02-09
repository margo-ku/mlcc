%{
    #include <climits>
    
    #include "include/driver/driver.h"
    #include "parser.hh"
%}

%option noyywrap nounput noinput batch debug
%option c++
%option yyclass="Scanner"

%{
  yy::parser::symbol_type ParseIntegerLiteral(const std::string &s, const yy::parser::location_type& loc);
%}

id     [a-zA-Z][a-zA-Z_0-9]*
digit       [0-9]
int_decimal {digit}+
int_suffix  ([uU][lL]?|[lL][uU]?)?
blank  [ \t\r]

%{
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%
%{
  yy::location& loc = driver.location_;
  loc.step();
%}

^[ \t]*#[^\n]*\n
{blank}+   { /* skip */ }
\n+        { loc.lines(yyleng); loc.step(); }
{int_decimal}{int_suffix} { return ParseIntegerLiteral(yytext, loc); }
"<<"       { return yy::parser::make_BIT_LSHIFT(loc); }
">>"       { return yy::parser::make_BIT_RSHIFT(loc); }
"<="       { return yy::parser::make_LEQ(loc); }
">="       { return yy::parser::make_GEQ(loc); }
"<"        { return yy::parser::make_LE(loc); }
">"        { return yy::parser::make_GE(loc); }
"!="       { return yy::parser::make_NOT_EQ(loc); }
"=="       { return yy::parser::make_EQ(loc); }
"="        { return yy::parser::make_ASSIGNMENT(loc); }
"||"       { return yy::parser::make_OR(loc); }
"&&"       { return yy::parser::make_AND(loc); }
"-"        { return yy::parser::make_MINUS(loc); }
"+"        { return yy::parser::make_PLUS(loc); }
"*"        { return yy::parser::make_STAR(loc); }
"/"        { return yy::parser::make_SLASH(loc); }
"%"        { return yy::parser::make_MOD(loc); }
"~"        { return yy::parser::make_BIT_NOT(loc); }
"!"        { return yy::parser::make_NOT(loc); }
"if"       { return yy::parser::make_IF(loc); }
"else"     { return yy::parser::make_ELSE(loc); }
"return"   { return yy::parser::make_RETURN(loc); }
"do"       { return yy::parser::make_DO(loc); }
"while"    { return yy::parser::make_WHILE(loc); }
"for"      { return yy::parser::make_FOR(loc); }
"break"    { return yy::parser::make_BREAK(loc); }
"continue" { return yy::parser::make_CONTINUE(loc); }
"void"     { return yy::parser::make_VOID(loc); }
"int"      { return yy::parser::make_INT(loc); }
"long"     { return yy::parser::make_LONG(loc); }
"signed"   { return yy::parser::make_SIGNED(loc); }
"unsigned" { return yy::parser::make_UNSIGNED(loc); }
"static"   { return yy::parser::make_STATIC(loc); }
"extern"   { return yy::parser::make_EXTERN(loc); }
"&"        { return yy::parser::make_BIT_AND(loc); }
"|"        { return yy::parser::make_BIT_OR(loc); }
"^"        { return yy::parser::make_BIT_XOR(loc); }
"("        { return yy::parser::make_LPAREN(loc); }
")"        { return yy::parser::make_RPAREN(loc); }
"{"        { return yy::parser::make_LBRACE(loc); }
"}"        { return yy::parser::make_RBRACE(loc); }
":"        { return yy::parser::make_COLON(loc); }
";"        { return yy::parser::make_SEMI(loc); }
"?"        { return yy::parser::make_QUESTION(loc); }
","        { return yy::parser::make_COMMA(loc); }
{id}       { return yy::parser::make_ID(yytext, loc); }
<<EOF>>    { return yy::parser::make_END (loc); }
.          { throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext)); }
%%

yy::parser::symbol_type ParseIntegerLiteral(const std::string &str, 
                                              const yy::parser::location_type& loc) {
    errno = 0;
    bool has_long_suffix = false;
    bool has_unsigned_suffix = false;
    std::string num_str = str;
    while (!num_str.empty()) {
        char last = num_str.back();
        if (last == 'L' || last == 'l') {
            has_long_suffix = true;
            num_str.pop_back();
        } else if (last == 'U' || last == 'u') {
            has_unsigned_suffix = true;
            num_str.pop_back();
        } else {
            break;
        }
    }

    char* endptr;
    unsigned long long value = strtoull(num_str.c_str(), &endptr, 10);
    if (errno == ERANGE) {
        throw yy::parser::syntax_error(loc, "integer out of range: " + str);
    }

    if (has_unsigned_suffix && has_long_suffix) {
        return yy::parser::make_ULONG_NUMBER(static_cast<unsigned long>(value), loc);
    } else if (has_unsigned_suffix) {
        if (value <= UINT_MAX) {
            return yy::parser::make_UINT_NUMBER(static_cast<unsigned int>(value), loc);
        } else {
            return yy::parser::make_ULONG_NUMBER(static_cast<unsigned long>(value), loc);
        }
    } else if (has_long_suffix) {
        return yy::parser::make_LONG_NUMBER(static_cast<long>(value), loc);
    } else {
        if (value <= INT_MAX) {
            return yy::parser::make_INT_NUMBER(static_cast<int>(value), loc);
        } else if (value <= LONG_MAX) {
            return yy::parser::make_LONG_NUMBER(static_cast<long>(value), loc);
        } else {
            return yy::parser::make_ULONG_NUMBER(static_cast<unsigned long>(value), loc);
        }
    }
}