#ifndef __MCDRIVER_HPP__
#define __MCDRIVER_HPP__ 1

#include <string>
#include <stack>
#include <list>
#include "scanner.h"
#include "split_pattern_parser.h"

namespace MC{

enum DataType {RELWGHT, ABSWGHT, SCOPE};

typedef struct Elmt{
  MC::DataType      type;
  float             value; // Weight or # of scope if it is repeated
  std::string       actions;
  std::list<Elmt*>  subElmts;
} Elmt;

class MC_Driver{
public:
  MC_Driver();
  virtual ~MC_Driver();

  void parse( const char *string );

  void addElement(Elmt* elmt);
  void enterSubScope();
  void exitSubScope() {currentScope.pop();}
  void wasConstScope();

  void computePattern(float _totalLength); /* Find the optimal repetitions,
                                            * then compute final vectors. */

  const std::vector<float>&        getWeights() const {return finalWeights;}
  const std::vector<std::string>&  getActions() const {return finalActions;}

private:
  void computeAbsPattern();
  void optimizeCoordinate(int n);
  void computeFinalVectors();
  void instantiate();
  void instantiateScope(std::list<Elmt*> scope);
  void deleteScope(std::list<Elmt*> scope);

  MC::MC_Parser  *parser  = nullptr;
  MC::MC_Scanner *scanner = nullptr;

  bool hasRelWeight;
  float totalLength;

  std::stack<std::list<Elmt*>*> currentScope;
  std::list<Elmt*> pattern;
  std::vector<int> repetitions; // 0 0 2 : 1st and 2nd stars are ignored, 3rd is repeated twice
                                // -1 if the scope does not have a star -> not to be repeated

  std::list<Elmt*> patternInstance;
  float totalRelWeight;
  float totalAbsLength;

  std::vector<float> finalWeights;
  std::vector<std::string> finalActions;
};

} /* end namespace MC */
#endif /* END __MCDRIVER_HPP__ */
