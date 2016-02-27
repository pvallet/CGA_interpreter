%{
  #include <iostream>
  #include "split_pattern_scanner_internal.h"
  #include "split_pattern_parser.h"

  typedef SP::SP_Parser::token token;

  #define YY_NO_UNISTD_H
%}

%option prefix="sp"
%option outfile="split_pattern_scanner.cpp"
%option debug
%option noyywrap
%option c++
%option nodefault
%option yyclass="SP::SP_Scanner"

%%

[ \r\t\n]*				;
[\{\}\|\*:]       { return yytext[0];}
\~[0-9]+\.?[0-9]* { yylval->fval = atof(&yytext[1]); return token::RELWGHT; }
[0-9]+\.?[0-9]*		{ yylval->fval = atof(yytext); return token::ABSWGHT; }
[^\{\}\|\*: \r\n\t]+([^\{\}\|\*:][^\{\}\|\*: \r\n\t]+)*			{
                  yylval->sval = strdup(yytext); return token::ACTIONS;
                } // We cut spaces before and after and keep the sentence
%%
