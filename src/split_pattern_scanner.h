#pragma once

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "split_pattern_parser.h"

namespace SP {

class SP_Scanner : public yyFlexLexer {
public:
   SP_Scanner(std::istream *in) : yyFlexLexer(in), yylval( nullptr ){};

   int yylex(SP::SP_Parser::semantic_type * const lval) {
      yylval = lval;
      return( yylex() );
   }

private:
   int yylex();
   SP::SP_Parser::semantic_type *yylval;
};

}
