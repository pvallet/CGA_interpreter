#include <string>

#include "rule.h"
#include "shape_tree.h"

#include "split_pattern/split_pattern_driver.h"

int main() {

  ACT::ShapeTree shapeTree;

  shapeTree.initFromFile(std::string("plane.off"));

  /*Node* root = shapeTree.getRoot();

  std::vector<Node*> split1, split2;
  std::vector<std::string> actions1;
  std::vector<double> weights(3,1.);

  root->split(X, split1, actions1, "{~1: BldArea | ~1: GreenSpace | ~1: BldArea}");

  split1[0]->extrude(5);
  split1[1]->extrude(3);
  split1[2]->extrude(2);

  //split1[0]->split(Y, split2, actions1, "{~1: BldArea | ~1: GreenSpace | ~1: BldArea}");

  shapeTree.displayGeometry();*/

  Rule* Parcel = new Rule("Parcel", "split(\"x\") {~1: BldArea | ~1: GreenSpace | ~1: BldArea}");
  Rule* BldArea = new Rule("BldArea", "extrude(5) split(\"y\") {0.4: Floor}*");

  shapeTree.addRule(Parcel);
  shapeTree.addRule(BldArea);

  RuleNames::getInstance().addRule("Parcel");
  RuleNames::getInstance().addRule("BldArea");

  shapeTree.setInitRule(Parcel);

  while (shapeTree.executeRule() != -1);

  shapeTree.displayGeometry();

  /*SP::SP_Driver driver;

  //driver.parse("{~1 : A | {0.3 : B | ~2 : C }* | 0.3 : B | {~1.5 : EDE | 0.5 : pok}* | ~1 : A}");
  //driver.parse("{15:A}*");
  //driver.parse("{~1: BldArea | ~1: GreenSpace | ~1: BldArea}");
  driver.parse("{0.4: Floor}*");
  driver.computePattern(20);
  std::vector<double> weights = driver.getWeights();
  std::vector<std::string> actions = driver.getActions();

  for (unsigned int i = 0 ; i < weights.size() ; i++) {
    std::cout << weights[i] << " " << actions[i] << std::endl;
  }*/

  return 0;
}
