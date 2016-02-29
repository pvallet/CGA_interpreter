#pragma once

#include <string>

#include "cgacode_scanner.h"
#include "cgacode_parser.h"
#include "../shape_tree.h"
#include "../rule.h"

using namespace std;

namespace CC{

class CC_Driver {
public:
  CC_Driver(ACT::ShapeTree* _shapeTree);
  virtual ~CC_Driver();

  // Returns the name of the init rule
  string parseFile( const string& fileName );

  void initFromFile(const string& path);
  void initFromRect(double x, double y);
  void setOutputFilename(const string& filename);
  void setTextureFile(const string& path);
  void addTextureRect(const string& name, double x0, double y0, double x1, double y1);
  void setRoofAngle(double angle); // Also converts angle from degrees to radians

  void setRecDepth(const string& ruleName, double depth);
  void setFallback(const string& ruleName, const string& fallback);

  void addRule(const string& name, double weight, const string& body);

private:
  CC::CC_Parser  *parser  = nullptr;
  CC::CC_Scanner *scanner = nullptr;

  ACT::ShapeTree* shapeTree;
  map<string, Rule*> rules;

  string initRule;
};

} /* End namespace CC */
