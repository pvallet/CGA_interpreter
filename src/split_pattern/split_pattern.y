%skeleton "lalr1.cc"
%require  "3.0"
%debug
%defines "split_pattern_parser.h"
%define api.namespace{SP}
%define parser_class_name {SP_Parser}

%code requires{
	namespace SP {
		class SP_Scanner;
		class SP_Driver;
	}
}

%parse-param { SP_Scanner  &scanner  }
%parse-param { SP_Driver   &driver  }

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

	 #include "split_pattern_driver.h"

#undef yylex
#define yylex scanner.splex

extern int line_num;
}

%define api.prefix {sp}

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
%start block;
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

void SP::SP_Parser::error( const std::string &err_message ) {
   std::cerr << "Split pattern : error: " << err_message << " Line: " << line_num << "\n";
}
