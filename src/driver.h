#ifndef __MCDRIVER_HPP__
#define __MCDRIVER_HPP__ 1

#include <string>
#include <cstdint>
#include "scanner.h"
#include "split_pattern_parser.h"

namespace MC{

class MC_Driver{
public:
   MC_Driver() = default;

   virtual ~MC_Driver();

   void parse( const char *string );

   void add_weight();

private:
   MC::MC_Parser  *parser  = nullptr;
   MC::MC_Scanner *scanner = nullptr;
};

} /* end namespace MC */
#endif /* END __MCDRIVER_HPP__ */
