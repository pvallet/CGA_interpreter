#pragma once

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "split_pattern_parser.h"

namespace MC {

class MC_Scanner : public yyFlexLexer {
public:
   MC_Scanner(std::istream *in) : yyFlexLexer(in), yylval( nullptr ){};

   int yylex(MC::MC_Parser::semantic_type * const lval) {
      yylval = lval;
      return( yylex() );
   }

private:
   int yylex();
   MC::MC_Parser::semantic_type *yylval;
};

}
