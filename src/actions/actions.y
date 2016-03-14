%skeleton "lalr1.cc"
%require  "3.0"
%debug
%defines "actions_parser.h"
%define api.namespace{ACT}
%define api.prefix {act}
%define parser_class_name {ACT_Parser}

%code requires{
	namespace ACT {
		class ACT_Scanner;
		class ShapeTree;
	}
}

%parse-param { ACT_Scanner  &scanner }
%parse-param { ShapeTree &st }

%code{
  #include <iostream>
  #include <cstdlib>
  #include <fstream>
	#include <string>

	#include "actions_scanner.h"
	#include "../shape_tree.h"

#undef yylex
#define yylex scanner.actlex

	namespace ACT {
		std::string toStr(char* ptr);
	}
}

%output  "actions_parser.cpp"

%union {
	double dval;
	char*	 sval;
}

%token <sval> STRING
%token <sval> CODE
%token <dval> DOUBLE
%token				TRANSLATE
%token				EXTRUDE
%token				SPLIT
%token				SELECT_FACES
%token				SET_TEXTURE
%token				REMOVE_FACES
%token				ROOF
%token				CREATE_ROOF
%token <sval> BEG_PTRN
%token <sval> END_PTRN
%token <sval> RULE
%token <sval> DEADRULE
%token <sval> ACTIONS

%type <sval> code

%%
%start actions;
actions:
	actions action
	| action
	;

action:
	translate
	| extrude
	| split
	| setTexture
	| selectFaces
	| removeFaces
	| createRoof
	| ROOF '(' ')'		{st.addToRoof();}
	| RULE						{st.addToRule(toStr($1));}
	| RULE ACTIONS		{st.addToRule(toStr($1),toStr($2));}
	| DEADRULE
	;

code:
	code CODE				{char *ss = (char *) malloc (strlen($1) + strlen($2) + 1);
											strcpy(ss,$1); free($1);
											strcat(ss,$2); free($2);
											$$ = ss; }
	| CODE					{$$ = strdup($1); free($1);}
	;

translate:
	TRANSLATE '(' DOUBLE ',' DOUBLE ',' DOUBLE ')' {st.translate($3,$5,$7);}
	;

extrude:
	EXTRUDE '(' DOUBLE ')' {st.extrude($3);}
	;

split:
	SPLIT '(' STRING ')' BEG_PTRN code END_PTRN
		{	char axis = $3[0];
		 	st.split(axis, toStr($5) + toStr($6) + toStr($7));
	 	}

	;

setTexture:
	SET_TEXTURE '(' STRING ')' { st.setTexture(toStr($3));}
	;

selectFaces:
	SELECT_FACES '(' STRING ')' { st.selectFaces(toStr($3));}
	;

removeFaces:
	REMOVE_FACES '(' ')' { st.removeFaces();}
	;

createRoof:
	CREATE_ROOF '(' DOUBLE ',' DOUBLE ')' { st.createRoof($3, $5);}
	;

%%

void ACT::ACT_Parser::error( const std::string &err_message ) {
   std::cerr << "Actions: Error: " << err_message << "\n";
}

std::string ACT::toStr(char* ptr) {
  std::string ret = (ptr != NULL) ? std::string(ptr, (strlen(ptr))) : std::string("");
  delete[] ptr; // Delete memory allocated by lexer.
  return ret;
}
