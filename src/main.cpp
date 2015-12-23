#include <string>

#include "rule.h"
#include "shape_tree.h"

#include "driver.h"

int main() {

  ShapeTree shapeTree;

  shapeTree.initFromFile(std::string("plane.off"));
  /*Node* root = shapeTree.getRoot();

  std::vector<Node*> split1, split2;
  std::vector<double> weights(3,1.);

  root->split(X, split1, weights);

  split1[0]->extrude(5);
  split1[1]->extrude(3);
  split1[2]->extrude(2);

  split1[1]->split(Y, split2, weights);*/

  /*Rule* split = new Rule("Parcel",
  "split(\"x\") {~1: BldArea | ~1: GreenSpace | ~1: BldArea}");
  // split("y") { floorH: set("floorIdx", get("split;index")) t(randShift(), 9, randShift()) Floor}*

  shapeTree.addRule(split);
  shapeTree.addRule(new Rule("BldArea", "extrude(5)"));

  shapeTree.setInitRule(split);

  while (shapeTree.executeRule() != -1);

  shapeTree.displayGeometry();*/

  MC::MC_Driver driver;

  driver.parse("{~1 : A | {0.3 : B | ~2 : C }* | 0.3 : B | {~1.5 : EDE | 0.5 : pok}* | ~1 : A}");
  //driver.parse("{15:A}*");
  driver.computePattern(20);
  std::vector<float> weights = driver.getWeights();
  std::vector<std::string> actions = driver.getActions();

  for (unsigned int i = 0 ; i < weights.size() ; i++) {
    std::cout << weights[i] << " " << actions[i] << std::endl;
  }

  return 0;
}
