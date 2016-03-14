%{
  #include <iostream>
  #include "cgacode_scanner_internal.h"
  #include "cgacode_parser.h"

  typedef CC::CC_Parser::token token;

  #define YY_NO_UNISTD_H

  namespace CC {
    int line_num = 1;
    int param_count = 0;
  }
%}

%option prefix="cc"
%option outfile="cgacode_scanner.cpp"
%option debug
%option noyywrap
%option c++
%option yyclass="CC::CC_Scanner"

%x RULES
%x RULEBODY

%%

[ \r\t]*				    ;
initFromFile        { return token::INIT_FROM_FILE;}
initFromRect        { return token::INIT_FROM_RECT;}
setOutputFilename   { return token::SET_OUTPUT_FILENAME;}
setTextureFile      { return token::SET_TEXTURE_FILE;}
addTextureRect      { return token::ADD_TEXTURE_RECT;}
setRoofZoom         { return token::SET_ROOF_ZOOM;}
setRoofTexture      { return token::SET_ROOF_TEXTURE;}
\"[^\n"]*\"         { yylval->sval = strdup(&yytext[1]);
                      yylval->sval[strlen(yylval->sval) - 1] = '\0';
                      return token::STRING;}
[0-9]+\.?[0-9]*     { yylval->fval = atof(yytext); return token::DOUBLE; }
"%%"                { BEGIN(RULES); return token::SEPARATOR; }
<RULES>setRecDepth  { return token::SET_REC_DEPTH;}
<RULES>fallback     { return token::FALLBACK;}
<RULES>[a-zA-Z][a-zA-Z0-9]* {
                      yylval->sval = strdup(yytext); return token::RULE_NAME;}
<RULES>[0-9]+\.?[0-9]* {
		                  yylval->fval = atof(yytext); return token::WEIGHT;}
<RULES>"-->"        { BEGIN(RULEBODY); return token::RULE;}
<RULEBODY>";;"      { BEGIN(RULES);}
<RULEBODY>[^;]*     { yylval->sval = strdup(yytext); return token::RULE_BODY;}
\n                  { CC::line_num++;}
%%
