%{
  #include <iostream>
  #include <string>
  #include "actions_scanner_internal.h"
  #include "actions_parser.h"
  #include "../rule.h"

  typedef ACT::ACT_Parser::token token;

  #define YY_NO_UNISTD_H
  int param_count = 0;
  int pattern_count = 0;
%}

%option prefix="act"
%option outfile="actions_scanner.cpp"
%option debug
%option noyywrap
%option c++
%option yyclass="ACT::ACT_Scanner"

%x PARAM
%x PATTERN
%x REMAINING

%%

[ \r\t\n]*			;
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
<PARAM>-?[0-9]+\.?[0-9]*  { yylval->dval = atof(yytext);   return token::DOUBLE;}
<PARAM>\,                 { return ',';}
translate       { return token::TRANSLATE; }
extrude         { return token::EXTRUDE; }
split           { return token::SPLIT; }
selectFaces     { return token::SELECT_FACES; }
setTexture      { return token::SET_TEXTURE; }
removeFaces     { return token::REMOVE_FACES; }
roof            { return token::ROOF; }
\{              { pattern_count = 1; BEGIN(PATTERN); yylval->sval = strdup(yytext); return token::BEG_PTRN;}
<PATTERN>\{     { pattern_count++; yylval->sval = strdup(yytext); return token::CODE; }
<PATTERN>\}\*   { pattern_count--; yylval->sval = strdup(yytext);
                  if (pattern_count == 0) {BEGIN(REMAINING); return token::END_PTRN;}
                  else return token::CODE;
                }
<PATTERN>\}     { pattern_count--; yylval->sval = strdup(yytext);
                  if (pattern_count == 0) {BEGIN(INITIAL); return token::END_PTRN;}
                  else return token::CODE;
                }
<PATTERN>[^\{\}]* { yylval->sval = strdup(yytext); return token::CODE;}
[a-zA-Z][a-zA-Z0-9]*  { yylval->sval = strdup(yytext);
                  if (RuleNames::getInstance().isRule(yytext)) {
                    BEGIN(REMAINING);
                    return token::RULE;
                  }
                  else return token::DEADRULE;
                }
<REMAINING>.*   { yylval->sval = strdup(yytext); return token::ACTIONS;}

%%
