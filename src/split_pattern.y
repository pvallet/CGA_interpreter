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
	cst_block		{driver.wasConstScope();}
	| var_block	{driver.exitSubScope();}
	;

cst_block:
	'{' {
			Elmt* elmt = new Elmt();
			elmt->type = SCOPE;
			driver.addElement(elmt);
			driver.enterSubScope();
		}
		body '}'
	;

body:
	part
	| body '|' part
	;

part:
	block
	| RELWGHT ':' ACTIONS {
		Elmt* elmt = new Elmt();
		elmt->type = RELWGHT;
		elmt->value = $1;
		elmt->actions = std::string($3);
		driver.addElement(elmt);
	}

	| ABSWGHT ':' ACTIONS {
		Elmt* elmt = new Elmt();
		elmt->type = ABSWGHT;
		elmt->value = $1;
		elmt->actions = std::string($3);
		driver.addElement(elmt);
	}
	;

var_block:
	cst_block '*'
	;

%%

void MC::MC_Parser::error( const std::string &err_message ) {
   std::cerr << "Error: " << err_message << "\n";
}
