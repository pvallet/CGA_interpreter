#include <cctype>
#include <cmath>
#include <cfloat>
#include <sstream>

#include "split_pattern_driver.h"

SP::SP_Driver::SP_Driver() :
  hasRelWeight(false) {
   currentScope.push(&pattern);
}

SP::SP_Driver::~SP_Driver() {
   delete(scanner);
   scanner = nullptr;
   delete(parser);
   parser = nullptr;

   deleteScope(pattern);
}

void SP::SP_Driver::deleteScope(std::list<Elmt*> scope) {
  for (auto it = scope.begin() ; it != scope.end() ; it++) {
    switch((*it)->type) {
      case RELWGHT:
      case ABSWGHT:
        delete *it;
        break;
      case SCOPE:
        deleteScope((*it)->subElmts);
        delete *it;
        break;
    }
  }
}

void SP::SP_Driver::parse( const char * const string ) {
   std::stringstream ss( string );

   delete(scanner);
   try
   {
      scanner = new SP::SP_Scanner( &ss );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "Split_pattern: Failed to allocate scanner: (" <<
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }

   delete(parser);
   try
   {
      parser = new SP::SP_Parser( (*scanner) /* scanner */,
                                  (*this) /* driver */ );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "Split_pattern: Failed to allocate parser: (" <<
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }
   const int accept( 0 );
   if( parser->parse() != accept )
   {
      std::cerr << "Split_pattern: Parse failed !\n";
   }
}

void SP::SP_Driver::addElement(Elmt* elmt) {
  currentScope.top()->push_back(elmt);

  if (currentScope.top()->back()->type == SCOPE) {
    currentScope.top()->back()->value = repetitions.size();
    repetitions.push_back(0);
  }

  else if (currentScope.top()->back()->type == RELWGHT)
    hasRelWeight = true;
}

void SP::SP_Driver::enterSubScope() {
  if (currentScope.top()->back()->type == SCOPE) {
    currentScope.push(&(currentScope.top()->back()->subElmts));
  }
  else
    std::cerr << "Error entering subscope : not a SCOPE element" << std::endl;
}

void SP::SP_Driver::wasConstScope() { // Append the subscope to the list
  std::list<Elmt*>* subscope = currentScope.top();
  currentScope.pop();
  repetitions[currentScope.top()->back()->value] = -1; // Not to be repeated
  currentScope.top()->pop_back();
  currentScope.top()->splice(currentScope.top()->end(), *subscope);
}

void SP::SP_Driver::computePattern(double _totalLength) {
  totalLength = _totalLength;


  if (!hasRelWeight) {
    /* In this case we only repeat the first star seen, all the other are
     * merely removed. We adapt the repetitions so that it matches the total
     * length
     */
     computeAbsPattern();
  }

  else
    optimizeCoordinate(repetitions.size()-1);

  instantiate();
  computeFinalVectors();
}

void SP::SP_Driver::computeAbsPattern() {
  double remainingLength = totalLength;
  double repeatedLength = 0.f;
  bool once = false;


  for (auto it = pattern.begin() ; it != pattern.end() ; it++) {
    if ((*it)->type == SCOPE) {
      if (!once) {
        once = true;
        for (auto it2 = (*it)->subElmts.begin() ; it2 != (*it)->subElmts.end() ; it2++) {
          if ((*it2)->type == ABSWGHT)
            repeatedLength += (*it2)->value;
        }
      }
    }

    else
      remainingLength -= (*it)->value;
  }


  if (remainingLength < 0.f)
    std::cerr << "Error: total length is too small for given pattern" << std::endl;

  else {
    // The little constant is to avoid rounding errors
    repetitions[0] = remainingLength / repeatedLength + 0.00001; // All the others values are 0
  }
}

// We only find a local minimum
void SP::SP_Driver::optimizeCoordinate(int n) { // n = coord
  if (repetitions[n] != -1) {
    repetitions[n] = 1;
    if (n != 0)
      optimizeCoordinate(n-1);
    else
      instantiate();
    double prevScore = DBL_MAX;
    double newScore = fabs((totalLength - totalAbsLength) / totalRelWeight - 1.f);

    std::vector<int> prevRepetitions;

    while (newScore < prevScore && totalAbsLength <= totalLength) {
      prevRepetitions = repetitions;
      repetitions[n]++;
      if (n != 0)
        optimizeCoordinate(n-1);
      else
        instantiate();
      prevScore = newScore;
      newScore = fabs((totalLength - totalAbsLength) / totalRelWeight - 1.f);
    }

    repetitions = prevRepetitions; // To match with prevScore
  }

  else if (n != 0)
    optimizeCoordinate(n-1);

  else
    instantiate();
}

void SP::SP_Driver::computeFinalVectors() {
  finalWeights.clear();
  finalActions.clear();

  double remainingRelLength = totalLength - totalAbsLength;

  for (auto it = patternInstance.begin() ; it != patternInstance.end() ; it++) {
    finalActions.push_back((*it)->actions);

    if ((*it)->type == ABSWGHT)
      finalWeights.push_back((*it)->value);

    else // type = RELWGHT
      finalWeights.push_back((*it)->value * remainingRelLength / totalRelWeight);
  }
}

void SP::SP_Driver::instantiate() {
  patternInstance.clear();
  totalRelWeight = 0.f;
  totalAbsLength = 0.f;
  instantiateScope(pattern);
}

void SP::SP_Driver::instantiateScope(std::list<Elmt*> scope) {
  for (auto it = scope.begin() ; it != scope.end() ; it++) {
    switch((*it)->type) {
      case RELWGHT:
        totalRelWeight += (*it)->value;
        patternInstance.push_back(*it);
        break;
      case ABSWGHT:
        totalAbsLength += (*it)->value;
        patternInstance.push_back(*it);
        break;
      case SCOPE:
        for (int i = 0 ; i < repetitions[(*it)->value] ; i++)
          instantiateScope((*it)->subElmts);
        break;
    }
  }
}
