%skeleton "lalr1.cc"
%require  "3.0"
%debug
%defines "split_pattern_parser.h"
%define api.namespace{MC}
%define parser_class_name {MC_Parser}

%code requires{
	namespace MC {
		class MC_Scanner;
		class MC_Driver;
	}
}

%parse-param { MC_Scanner  &scanner  }
%parse-param { MC_Driver  &driver  }

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

	 #include "driver.h"

#undef yylex
#define yylex scanner.yylex
}

%output  "split_pattern_parser.cpp"

%union {
	float fval;
	int ival;
	char* sval;
}

%token <fval> RELWGHT
%token <fval> ABSWGHT
%token <sval> ACTIONS

%%
block:
	cst_block
	| var_block
	;

cst_block:
	'{' body '}'
	;

body:
	part
	| body '|' part
	;

part:
	block
	| RELWGHT ':' ACTIONS { std::cout << "Relweight: '" << $1 << "' Actions: '" << $3 << "'" << std::endl; }
	| ABSWGHT ':' ACTIONS { std::cout << "Absweight: '" << $1 << "' Actions: '" << $3 << "'" << std::endl; }
	;

var_block:
	cst_block '*' { std::cout << "var_block" << std::endl; }
	;

%%

void MC::MC_Parser::error( const std::string &err_message ) {
   std::cerr << "Error: " << err_message << "\n";
}
