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
%option yyclass="ACT::ACT_Scanner"

%x PARAM
%x PATTERN
%x REMAINING

%%

[ \r\t]*				;
\(              { param_count = 1; BEGIN(PARAM); return '(';}
<PARAM>\(       { param_count++; return '('; }
<PARAM>\)       { param_count--;
                  if (param_count == 0) {
                    BEGIN(INITIAL);
                  }
                  return ')';
                }
<PARAM>\"[^\"]*\"         { yylval->sval = strdup(&yytext[1]);
                            yylval->sval[strlen(yylval->sval) - 1] = '\0';
                            return token::STRING;}
<PARAM>[0-9]+\.?[0-9]*    { yylval->dval = atof(yytext);   return token::DOUBLE;}
<PARAM>\,                 { return ',';}
extrude         { return token::EXTRUDE; }
split           { return token::SPLIT; }
selectFaces     { return token::SELECT_FACES; }
setTexture      { return token::SET_TEXTURE; }
\{              { pattern_count = 1; BEGIN(PATTERN); yylval->sval = strdup(yytext); return token::BEG_PTRN;}
<PATTERN>\{     { pattern_count++; yylval->sval = strdup(yytext); return token::CODE; }
<PATTERN>\}\*   { pattern_count--; yylval->sval = strdup(yytext);
                  if (param_count == 0) {BEGIN(INITIAL); return token::END_PTRN;}
                  else return token::CODE;
                }
<PATTERN>\}     { pattern_count--; yylval->sval = strdup(yytext);
                  if (param_count == 0) {BEGIN(INITIAL); return token::END_PTRN;}
                  else return token::CODE;
                }
<PATTERN>[^\{\}]* { yylval->sval = strdup(yytext); return token::CODE;}
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
