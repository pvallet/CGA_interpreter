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
}

%define api.prefix {act}

%output  "actions_parser.cpp"

%union {
	double dval;
	std::string* sval;
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

%destructor { if ($$) { delete ($$); ($$) = nullptr; } } <sval>

%%
%start actions;
actions:
	actions action
	| action
	;

action:
	extrude
	| split
	| RULE						{std::cout<<"rule"<<std::endl;st.addToRule(*$1);}
	| RULE ACTIONS		{std::cout<<"deadrule"<<std::endl;st.addToRule(*$1,*$2);}
	| DEADRULE
	;

string:
	string STRING			{std::cout<<"concat"<<std::endl;$$ = new std::string(*$1 + *$2);}
	| STRING					{std::cout<<"returnstring"<<std::endl;$$ = new std::string(*$1);}
	;

double:
	DOUBLE						{std::cout<<"returndouble"<<std::endl;$$ = $1;}
	;

extrude:
	EXTRUDE '(' double ')' {std::cout<<"extrude"<<std::endl;st.extrude($3);}
	;

split:
	SPLIT '(' string ')' BEG_PTRN string END_PTRN
		{	std::cout<<"split"<<std::endl;char axis = $3->c_str()[1];
		 	st.split(axis, *$5 + *$6 + *$7);
			std::cout<<"endsplit"<<std::endl;
	 	}
	;

%%

void ACT::ACT_Parser::error( const std::string &err_message ) {
   std::cerr << "Actions: Error: " << err_message << " Line: " << line_num << "\n";
}
