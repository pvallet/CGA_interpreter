#include "cgacode_driver.h"
#include <fstream>

CC::CC_Driver::CC_Driver(ACT::ShapeTree* _shapeTree) :
  shapeTree(_shapeTree)
{}

CC::CC_Driver::~CC_Driver() {
  for (auto it = rules.begin() ; it != rules.end() ; it++) {
    delete it->second;
  }
}

string CC::CC_Driver::parseFile( const string& fileName ) {
   ifstream input;
   input.open(fileName);

   delete(scanner);
   try
   {
      scanner = new CC::CC_Scanner( &input );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "CGAcode: Failed to allocate scanner: (" <<
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }

   delete(parser);
   try
   {
      parser = new CC::CC_Parser( (*scanner) /* scanner */,
                                  (*this) /* driver */ );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "CGAcode: Failed to allocate parser: (" <<
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }
   const int accept( 0 );
   if( parser->parse() != accept )
   {
      std::cerr << "CGAcode: Parse failed !\n";
   }

   input.close();

   return initRule;
}

void CC::CC_Driver::initFromFile(const string& path) {
  shapeTree->initFromFile(path);
}

void CC::CC_Driver::initFromRect(double x, double y) {
  shapeTree->initFromRect(x,y);
}

void CC::CC_Driver::setOutputFilename(const string& filename) {
  shapeTree->setOutputFilename(filename);
}

void CC::CC_Driver::setTextureFile(const string& path) {
  shapeTree->setTextureFile(path);
}

void CC::CC_Driver::addTextureRect(const string& name, double x0, double y0, double x1, double y1) {
  shapeTree->addTextureRect(name, x0, y0, x1, y1);
}

void CC::CC_Driver::setRoofZoom(double zoom) {
  shapeTree->setRoofZoom(zoom);
}

void CC::CC_Driver::setRoofTexture(const string& path) {
  shapeTree->setRoofTexture(path);
}

void CC::CC_Driver::setRecDepth(const string& ruleName, double depth) {
  if (rules.find(ruleName) == rules.end())
    std::cerr << "Error: trying to access to rule: " << ruleName
              << " which is not declared yet." << std::endl;

  else
    rules[ruleName]->setRecDepth(depth);
}

void CC::CC_Driver::setFallback(const string& ruleName, const string& fallback) {
  if (rules.find(ruleName) == rules.end())
    std::cerr << "Error: trying to access to rule: " << ruleName
              << " which is not declared yet." << std::endl;

  else
    rules[ruleName]->setFallback(fallback);
}

void CC::CC_Driver::addRule(const string& name, double weight, const string& body) {
  bool isInitRule = rules.empty();

  if (rules.find(name) == rules.end()) {
    Rule* rule = new Rule(name);
    rules.insert(pair<string, Rule*>(name, rule));
    RuleNames::getInstance().addRule(name);
    shapeTree->addRule(rule);

    if (isInitRule)
      initRule = name;
  }

  rules[name]->addAction(body, weight);

}
