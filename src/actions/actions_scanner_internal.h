#pragma once

#include "actions_parser.h"

namespace ACT {

class ACT_Scanner : public actFlexLexer {
public:
   ACT_Scanner(std::istream *in) : actFlexLexer(in), yylval( nullptr ){};

   int yylex(ACT::ACT_Parser::semantic_type * const lval) {
      yylval = lval;
      return( yylex() );
   }

private:
   int yylex();
   ACT::ACT_Parser::semantic_type *yylval;
};

}
