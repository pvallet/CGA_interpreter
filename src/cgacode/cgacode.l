%{
  #include <iostream>
  #include "cgacode_parser.h"
  #include "cgacode_scanner.h"

  typedef CC::CC_Parser::token token;

  #define YY_NO_UNISTD_H

  namespace CC {
    int line_num = 1;
  }
%}

%option prefix="cc"
%option outfile="cgacode_lexer.cpp" header-file="cgacode_lexer.h"
%option debug
%option noyywrap
%option c++
%option yyclass="CC::CC_Scanner"

%x RULES
%x RULEBODY

%%

[ \r\t]*				    ;
initFromFile        { return token::INIT_FROM_FILE;}
setTextureFile      { return token::SET_TEXTURE_FILE;}
addTextureRect      { return token::ADD_TEXTURE_RECT;}
\"[^\n"]*\"         { yylval->sval = strdup(&yytext[1]);
                      yylval->sval[strlen(yylval->sval) - 1] = '\0';
                      return token::STRING;}
[0-9]+\.?[0-9]*     { yylval->fval = atof(yytext); return token::DOUBLE; }
"%%"               { BEGIN(RULES); return token::SEPARATOR; }
<RULES>[a-zA-Z][a-zA-Z0-9]* {
                      yylval->sval = strdup(yytext); return token::RULE_NAME;}
<RULES>[0-9]+\.?[0-9]* {
		                  yylval->fval = atof(yytext); return token::WEIGHT;}
<RULES>"-->"        { BEGIN(RULEBODY); return token::RULE;}
<RULEBODY>";;"      { BEGIN(RULES);}
<RULEBODY>[^;]*     { yylval->sval = strdup(yytext); return token::RULE_BODY;}
\n                  { CC::line_num++;}
%%
