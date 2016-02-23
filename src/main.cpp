#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "shape_tree.h"
#include "cgacode/cgacode_driver.h"

#include "split_pattern/split_pattern_driver.h"

int main(int argv, char* argc[]) {
  srand(time(NULL));

  if (argv != 2){
		std::cout << "Usage: " << argc[0] << " <path to source code>" << std::endl;
		return 0;
	}

  std::string sourceCode = argc[1];

  ACT::ShapeTree shapeTree;
  shapeTree.setOutputFilename(sourceCode + ".off");

  // We need to complete the parsing of the code before initializing the queue
  // of active rules.

  CC::CC_Driver cgacodeDriver(&shapeTree);
  std::string initRule = cgacodeDriver.parseFile(sourceCode);
  shapeTree.setInitRule(initRule);

  while (shapeTree.executeRule() != -1);

  shapeTree.outputGeometry();

  return 0;
}
