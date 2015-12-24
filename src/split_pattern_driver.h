#pragma once

#include <string>
#include <stack>
#include <list>
#include "split_pattern_scanner.h"
#include "split_pattern_parser.h"

namespace SP{

enum DataType {RELWGHT, ABSWGHT, SCOPE};

typedef struct Elmt{
  SP::DataType      type;
  float             value; // Weight or # of scope if it is repeated
  std::string       actions;
  std::list<Elmt*>  subElmts;
} Elmt;

class SP_Driver{
public:
  SP_Driver();
  virtual ~SP_Driver();

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

  SP::SP_Parser  *parser  = nullptr;
  SP::SP_Scanner *scanner = nullptr;

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

} /* end namespace SP */
