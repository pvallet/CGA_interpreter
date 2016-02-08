#pragma once

#if ! defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer ccFlexLexer
#include <FlexLexer.h>
#endif

#include "cgacode_parser.h"

namespace CC {

class CC_Scanner : public ccFlexLexer {
public:
   CC_Scanner(std::istream *in) : ccFlexLexer(in), yylval( nullptr ){};

   int yylex(CC::CC_Parser::semantic_type * const lval) {
      yylval = lval;
      return( yylex() );
   }

private:
   int yylex();
   CC::CC_Parser::semantic_type *yylval;
};

}
