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

  CC::CC_Driver cgacodeDriver(&shapeTree);

  cgacodeDriver.parseFile(sourceCode);

  while (shapeTree.executeRule() != -1);

  shapeTree.outputGeometryOBJ();

  return 0;
}
