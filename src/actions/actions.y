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
%token <dval> DOUBLE
%token				EXTRUDE
%token				SPLIT
%token <sval> BEG_PTRN
%token <sval> END_PTRN
%token <sval> RULE
%token <sval> DEADRULE
%token <sval> ACTIONS

%type <sval> string
%type <dval> double

%%
%start actions;
actions:
	actions action
	| action
	;

action:
	extrude
	| split
	| RULE						{st.addToRule(toStr($1));}
	| RULE ACTIONS		{st.addToRule(toStr($1),toStr($2));}
	| DEADRULE
	;

string:
	string STRING			{char *ss = (char *) malloc (strlen($1) + strlen($2) + 1);
											strcpy(ss,$1); free($1);
											strcat(ss,$2); free($2);
											$$ = ss; }
	| STRING					{$$ = strdup($1); free($1);}
	;

double:
	DOUBLE						{$$ = $1;}
	;

extrude:
	EXTRUDE '(' double ')' {st.extrude($3);}
	;

split:
	SPLIT '(' string ')' BEG_PTRN string END_PTRN
		{	char axis = $3[1];
		 	st.split(axis, toStr($5) + toStr($6) + toStr($7));
	 	}
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
