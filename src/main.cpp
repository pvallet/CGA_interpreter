#include <string>

#include "rule.h"
#include "shape_tree.h"

#include "split_pattern/split_pattern_driver.h"

int main() {

  ACT::ShapeTree shapeTree;

  // To be automated by a parser

  shapeTree.initFromFile(std::string("plane.off"));
  shapeTree.setTextureFile("res/window_small.jpg");
  shapeTree.addTextureRect("window", 0., 0., 1., 1.);

  Rule* Parcel = new Rule("Parcel", "split(\"x\") {~1: BldArea | ~1: GreenSpace | ~1: BldArea}");
  Rule* BldArea = new Rule("BldArea", "extrude(5) selectFaces(\"zpos\") setTexture(\"window\")");//split(\"y\") {0.4: Floor}*");
  Rule* Floor = new Rule("Floor", "selectFaces(\"zpos\") setTexture(\"window\")");

  shapeTree.addRule(Parcel);
  shapeTree.addRule(BldArea);
  shapeTree.addRule(Floor);

  RuleNames::getInstance().addRule("Parcel");
  RuleNames::getInstance().addRule("BldArea");
  RuleNames::getInstance().addRule("Floor");

  shapeTree.setInitRule(Parcel);

  //

  while (shapeTree.executeRule() != -1);

  shapeTree.displayGeometryOBJ();

  return 0;
}
