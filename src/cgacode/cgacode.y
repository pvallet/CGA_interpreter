%skeleton "lalr1.cc"
%require  "3.0"
%debug
%defines "cgacode_parser.h"
%define api.namespace{CC}
%define api.prefix {cc}
%define parser_class_name {CC_Parser}

%code requires{
	namespace CC {
		class CC_Scanner;
		class CC_Driver;
	}
}

%parse-param { CC_Scanner  &scanner  }
%parse-param { CC_Driver   &driver  }

%code{
  #include <iostream>
  #include <cstdlib>
  #include <fstream>

	#include "cgacode_scanner.h"
	#include "cgacode_driver.h"

#undef yylex
#define yylex scanner.cclex

	namespace CC {
		extern int line_num;
		std::string toStr(char* ptr);
	}
}

%output  "cgacode_parser.cpp"

%union {
	float fval;
	int ival;
	char* sval;
}

%token				INIT_FROM_FILE
%token				INIT_FROM_RECT
%token				SET_OUTPUT_FILENAME
%token 				SET_TEXTURE_FILE
%token				ADD_TEXTURE_RECT
%token				SET_ROOF_ANGLE
%token				SET_ROOF_OFFSET
%token				SEPARATOR
%token				SET_REC_DEPTH
%token				FALLBACK
%token <sval> RULE_NAME
%token <fval> WEIGHT
%token				RULE
%token <sval> RULE_BODY
%token <sval> STRING
%token <fval> DOUBLE

%%
%start skeleton;
skeleton:
	commands SEPARATOR rules
	;

commands:
	commands command
	| command
	;

command:
	INIT_FROM_FILE STRING						{driver.initFromFile(toStr($2));}
	| INIT_FROM_RECT DOUBLE DOUBLE	{driver.initFromRect($2,$3);}
	| SET_OUTPUT_FILENAME STRING		{driver.setOutputFilename(toStr($2));}
	| SET_TEXTURE_FILE STRING 			{driver.setTextureFile(toStr($2));}
	| ADD_TEXTURE_RECT STRING DOUBLE DOUBLE DOUBLE DOUBLE
		{driver.addTextureRect(toStr($2), $3, $4, $5, $6);}
	| SET_ROOF_ANGLE DOUBLE					{driver.setRoofAngle($2);}
	| SET_ROOF_OFFSET DOUBLE				{driver.setRoofOffset($2);}
	;

rules:
	rules rule
	| rule
	;

rule:
	SET_REC_DEPTH RULE_NAME WEIGHT 			{driver.setRecDepth(toStr($2), $3);}
	| FALLBACK RULE_NAME RULE RULE_BODY	{driver.setFallback(toStr($2), toStr($4));}
	| rule_declaration
	;

rule_declaration:
	RULE_NAME WEIGHT RULE RULE_BODY			{driver.addRule(toStr($1), $2, toStr($4));}
	| RULE_NAME RULE RULE_BODY					{driver.addRule(toStr($1), 1, toStr($3));}
	;

%%

void CC::CC_Parser::error( const std::string &err_message ) {
   std::cerr << "CGA Code : error: " << err_message << " Line: " << CC::line_num << "\n";
}

std::string CC::toStr(char* ptr) {
  std::string ret = (ptr != NULL) ? std::string(ptr, (strlen(ptr))) : std::string("");
  delete[] ptr; // Delete memory allocated by lexer.
  return ret;
}
