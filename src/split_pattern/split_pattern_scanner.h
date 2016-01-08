#pragma once


#undef yyFlexLexer
#define yyFlexLexer spFlexLexer
#include <FlexLexer.h>


#include "split_pattern_parser.h"

namespace SP {

class SP_Scanner : public spFlexLexer {
public:
   SP_Scanner(std::istream *in) : spFlexLexer(in), yylval( nullptr ){};

   int yylex(SP::SP_Parser::semantic_type * const lval) {
      yylval = lval;
      return( yylex() );
   }

private:
   int yylex();
   SP::SP_Parser::semantic_type *yylval;
};

}
