%skeleton "lalr1.cc"
%require  "3.0"
%debug
%defines "actions_parser.h"
%define api.namespace{ACT}
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

	 #include "../shape_tree.h"

#undef yylex
#define yylex scanner.actlex

extern int line_num;
std::string toStr(char* ptr);
}

%define api.prefix {act}

%output  "actions_parser.cpp"

%union {
	double dval;
	char*	 sval;
}

%token <sval> STRING
%token <sval> CODE
%token <dval> DOUBLE
%token				EXTRUDE
%token				SPLIT
%token				SELECT_FACES
%token				SET_TEXTURE
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
	extrude
	| split
	| setTexture
	| selectFaces
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

extrude:
	EXTRUDE '(' DOUBLE ')' {st.extrude($3);}
	;

split:
	SPLIT '(' STRING ')' BEG_PTRN code END_PTRN
		{	char axis = $3[0];
		 	st.split(axis, toStr($5) + toStr($6) + toStr($7));
	 	}
	;

selectFaces:
	SELECT_FACES '(' STRING ')' { st.selectFaces($3);}
	;

setTexture:
	SET_TEXTURE '(' STRING ')' { st.setTexture($3);}
	;

%%

void ACT::ACT_Parser::error( const std::string &err_message ) {
   std::cerr << "Actions: Error: " << err_message << " Line: " << line_num << "\n";
}

std::string toStr(char* ptr) {
  std::string ret = (ptr != NULL) ? std::string(ptr, (strlen(ptr))) : std::string("");
  delete[] ptr; // Delete memory allocated by lexer.
  return ret;
}
