%{
  #include <iostream>
  #include "split_pattern_parser.h"
  #include "scanner.h"

  typedef MC::MC_Parser::token token;

  #define YY_NO_UNISTD_H
%}

%option outfile="split_pattern_lexer.cpp" header-file="split_pattern_lexer.h"
%option debug
%option noyywrap
%option c++
%option nodefault
%option yyclass="MC::MC_Scanner"

%%

[ \r\n\t]*				;
[\{\}\|\*:]       { return yytext[0];}
\~[0-9]+\.?[0-9]*  { yylval->fval = atof(&yytext[1]); return token::RELWGHT; }
[0-9]+\.?[0-9]*		{ yylval->fval = atof(yytext); return token::ABSWGHT; }
[^\{\}\|\*: \r\n\t]([^\{\}\|\*:][^\{\}\|\*: \r\n\t])*			{
                  yylval->sval = strdup(yytext); return token::ACTIONS;
                } // We cut spaces before and after and keep the sentence
%%
