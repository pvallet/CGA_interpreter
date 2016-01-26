#include <string>

#include "rule.h"
#include "shape_tree.h"

#include "split_pattern/split_pattern_driver.h"

int main() {

  ACT::ShapeTree shapeTree;

  shapeTree.initFromFile(std::string("plane.off"));

  Rule* Parcel = new Rule("Parcel", "split(\"x\") {~1: BldArea | ~1: GreenSpace | ~1: BldArea}");
  Rule* BldArea = new Rule("BldArea", "extrude(5) extrude(0.5)");//split(\"y\") {0.4: Floor}*");
  //Rule* Floor = new Rule("Floor", "extrude(2)");

  shapeTree.addRule(Parcel);
  shapeTree.addRule(BldArea);
  //shapeTree.addRule(Floor);

  RuleNames::getInstance().addRule("Parcel");
  RuleNames::getInstance().addRule("BldArea");
  //RuleNames::getInstance().addRule("Floor");

  shapeTree.setInitRule(Parcel);

  while (shapeTree.executeRule() != -1);

  shapeTree.displayGeometry();

  return 0;
}
