%{
  #include <iostream>
  #include <string>
  #include "actions_parser.h"
  #include "actions_scanner.h"
  #include "../rule.h"

  typedef ACT::ACT_Parser::token token;

  #define YY_NO_UNISTD_H
  int param_count = 0;
  int pattern_count = 0;
  int lineNum = 1;
%}

%option prefix="act"
%option outfile="actions_lexer.cpp" header-file="actions_lexer.h"
%option debug
%option noyywrap
%option c++
%option nodefault
%option yyclass="ACT::ACT_Scanner"

%x PARAM
%x PATTERN
%x REMAINING

%%

[ \r\t]*				;
\(              { param_count = 1; BEGIN(PARAM); return '(';}
<PARAM>\(       { param_count++; yylval->sval = strdup(yytext); return token::STRING; }
<PARAM>\)       { param_count--;
                  if (param_count == 0) {
                    BEGIN(INITIAL); return ')';
                  } else {
                    yylval->sval = strdup(yytext);
                    return token::STRING;
                  }
                }
<PARAM>[^\(\)0-9\.]*     { yylval->sval = strdup(yytext); return token::STRING;}
<PARAM>[0-9]+\.?[0-9]*   { yylval->dval = atof(yytext);   return token::DOUBLE;}
extrude         { return token::EXTRUDE; }
split           { return token::SPLIT; }
\{              { pattern_count = 1; BEGIN(PATTERN); yylval->sval = strdup(yytext); return token::BEG_PTRN;}
<PATTERN>\{     { pattern_count++; yylval->sval = strdup(yytext); return token::STRING; }
<PATTERN>\}\*   { pattern_count--; yylval->sval = strdup(yytext);
                  if (param_count == 0) {BEGIN(INITIAL); return token::END_PTRN;}
                  else return token::STRING;
                }
<PATTERN>\}     { pattern_count--; yylval->sval = strdup(yytext);
                  if (param_count == 0) {BEGIN(INITIAL); return token::END_PTRN;}
                  else return token::STRING;
                }
<PATTERN>[^\{\}]* { yylval->sval = strdup(yytext); return token::STRING;}
[a-zA-Z]*       { yylval->sval = strdup(yytext);
                  if (RuleNames::getInstance().isRule(yytext)) {
                    BEGIN(REMAINING);
                    return token::RULE;
                  }
                  else return token::DEADRULE;
                }
<REMAINING>.*   { yylval->sval = strdup(yytext); return token::ACTIONS;}
\n              { lineNum++; }

%%
