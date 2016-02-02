#include <string>

#include "rule.h"
#include "shape_tree.h"

#include "split_pattern/split_pattern_driver.h"

int main() {

  ACT::ShapeTree shapeTree;

  // To be automated by a parser

  shapeTree.initFromFile(std::string("plane.off"));
  shapeTree.setTextureFile("res/maisel.png");
  shapeTree.addTextureRect("withoutbalc", 0., 5./8., 1., 1.);
  shapeTree.addTextureRect("withbalcCenter", 56./512., 1./4., 1.-56./512., 5./8.);
  shapeTree.addTextureRect("withbalcLeft", 0., 1./4., 56./512., 5./8.);
  shapeTree.addTextureRect("withbalcRight", 1.-56./512., 1./4., 1., 5./8.);
  shapeTree.addTextureRect("longbalc", 0., 0., 250./512., 112./512.);
  shapeTree.addTextureRect("shortbalc", 243./512., 0., 335./512., 112./512.);

  Rule* GlobalShape = new Rule("GlobalShape", "extrude(4.8) split(\"x\") {~1: ColumnA | ~1: ColumnB }*");
  Rule* ColumnA = new Rule("ColumnA", "split(\"y\") {~0.4: WithBalc | ~0.4: WithoutBalc }*");
  Rule* ColumnB = new Rule("ColumnB", "split(\"y\") {~0.4: WithoutBalc | ~0.4: WithBalc }*");
  Rule* WithoutBalc = new Rule("WithoutBalc",
  "selectFaces(\"zpos\") selectFaces(\"zneg\") setTexture(\"withoutbalc\")");
  Rule* WithBalc = new Rule("WithBalc", "split(\"x\") {~56: selectFaces(\"zpos\") selectFaces(\"zneg\") setTexture(\"withbalcLeft\") | ~200:Balcony | ~200:Balcony | ~56:selectFaces(\"zpos\") selectFaces(\"zneg\") setTexture(\"withbalcRight\")}");
  Rule* Balcony = new Rule("Balcony",
  "selectFaces(\"zpos\") selectFaces(\"zneg\") extrude(0.2) selectFaces(\"zpos\") selectFaces(\"zneg\") setTexture(\"longbalc\") selectFaces(\"\") selectFaces(\"xpos\") selectFaces(\"xneg\") setTexture(\"shortbalc\")");

  shapeTree.addRule(GlobalShape);
  shapeTree.addRule(ColumnA);
  shapeTree.addRule(ColumnB);
  shapeTree.addRule(WithoutBalc);
  shapeTree.addRule(WithBalc);
  shapeTree.addRule(Balcony);

  RuleNames::getInstance().addRule("GlobalShape");
  RuleNames::getInstance().addRule("ColumnA");
  RuleNames::getInstance().addRule("ColumnB");
  RuleNames::getInstance().addRule("WithoutBalc");
  RuleNames::getInstance().addRule("WithBalc");
  RuleNames::getInstance().addRule("Balcony");

  shapeTree.setInitRule(GlobalShape);

  /*Rule* Parcel = new Rule("Parcel", "split(\"x\") {~1: BldArea | ~1: GreenSpace | ~1: BldArea}");
  Rule* BldArea = new Rule("BldArea", "extrude(5) selectFaces(\"zpos\") setTexture(\"window\")");//split(\"y\") {0.4: Floor}*");
  Rule* Floor = new Rule("Floor", "selectFaces(\"zpos\") setTexture(\"window\")");

  shapeTree.addRule(Parcel);
  shapeTree.addRule(BldArea);
  shapeTree.addRule(Floor);

  RuleNames::getInstance().addRule("Parcel");
  RuleNames::getInstance().addRule("BldArea");
  RuleNames::getInstance().addRule("Floor");

  shapeTree.setInitRule(Parcel);*/

  //

  while (shapeTree.executeRule() != -1);

  shapeTree.outputGeometryOBJ();

  return 0;
}
