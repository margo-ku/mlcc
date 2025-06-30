%{
    #include <climits>
    
    #include "include/driver/driver.h"
    #include "parser.hh"
%}

%option noyywrap nounput noinput batch debug
%option c++
%option yyclass="Scanner"

%{
  yy::parser::symbol_type make_NUMBER(const std::string &s, const yy::parser::location_type& loc);
%}

id     [a-zA-Z][a-zA-Z_0-9]*
number [0-9]+
blank  [ \t\r]

%{
  # define YY_USER_ACTION  loc.columns (yyleng);
%}

%%
%{
  yy::location& loc = driver.location_;
  loc.step();
%}

{blank}+  { /* skip */ }
\n+       { loc.lines(yyleng); loc.step(); }
{number}  { return make_NUMBER(yytext, loc); }
"<="      { return yy::parser::make_LEQ(loc); }
">="      { return yy::parser::make_GEQ(loc); }
"<"      { return yy::parser::make_LE(loc); }
">"      { return yy::parser::make_GE(loc); }
"!="      { return yy::parser::make_NOT_EQ(loc); }
"=="      { return yy::parser::make_EQ(loc); }
"="       { return yy::parser::make_ASSIGNMENT(loc); }
"||"      { return yy::parser::make_OR(loc); }
"&&"      { return yy::parser::make_AND(loc); }
"-"       { return yy::parser::make_MINUS(loc); }
"+"       { return yy::parser::make_PLUS(loc); }
"*"       { return yy::parser::make_STAR(loc); }
"/"       { return yy::parser::make_SLASH(loc); }
"%"       { return yy::parser::make_MOD(loc); }
"~"       { return yy::parser::make_BIT_NOT(loc); }
"!"       { return yy::parser::make_NOT(loc); }
"return"  { return yy::parser::make_RETURN(loc); }
"int"     { return yy::parser::make_INT(loc); }
"("       { return yy::parser::make_LPAREN(loc); }
")"       { return yy::parser::make_RPAREN(loc); }
"{"       { return yy::parser::make_LBRACE(loc); }
"}"       { return yy::parser::make_RBRACE(loc); }
";"       { return yy::parser::make_SEMI(loc); }
{id}      { return yy::parser::make_ID(yytext, loc); }
<<EOF>>   { return yy::parser::make_END (loc); }
.         { throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext)); }
%%

yy::parser::symbol_type make_NUMBER(const std::string &s, const yy::parser::location_type& loc) {
    errno = 0;
    long n = strtol(s.c_str(), NULL, 10);
    if (!(errno != ERANGE && INT_MIN <= n && n <= INT_MAX))
        throw yy::parser::syntax_error(loc, "integer is out of range: " + s);
    return yy::parser::make_NUMBER((int)n, loc);
}