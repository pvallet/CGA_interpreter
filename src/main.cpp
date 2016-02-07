#include <string>

#include "shape_tree.h"
#include "rule.h"

#include "split_pattern/split_pattern_driver.h"

int main() {

  ACT::ShapeTree shapeTree;

  // To be automated by a parser

  shapeTree.initFromFile(std::string("plane.off"));
  shapeTree.setTextureFile("res/maisel.png");
  shapeTree.addTextureRect("withoutbalc", 0., 5./8., 1., 1.);
  shapeTree.addTextureRect("withbalc", 0., 1./4., 1., 5./8.);
  shapeTree.addTextureRect("longbalc", 0., 0., 250./512., 112./512.);
  shapeTree.addTextureRect("shortbalc", 243./512., 0., 335./512., 112./512.);

  Rule* GlobalShape = new Rule("GlobalShape", "extrude(4.8) split(\"x\") {~1: ColumnA | ~1: ColumnB }*");
  Rule* ColumnA = new Rule("ColumnA", "split(\"y\") {~0.4: WithBalc | ~0.4: WithoutBalc }*");
  Rule* ColumnB = new Rule("ColumnB", "split(\"y\") {~0.4: WithoutBalc | ~0.4: WithBalc }*");
  Rule* WithoutBalc = new Rule("WithoutBalc", "selectFaces(\"z\") setTexture(\"withoutbalc\")");
  Rule* WithBalc = new Rule("WithBalc", "selectFaces(\"z\") setTexture(\"withbalc\") split(\"x\") {~56:Trash | ~200:Balcony | ~200:Balcony | ~56:Trash}");
  Rule* Balcony = new Rule("Balcony", "split(\"y\") {~90: selectFaces(\"z\") extrude(0.15) TextureBalc | ~100:Trash}");
  Rule* TextureBalc = new Rule("TextureBalc", "selectFaces(\"z\") setTexture(\"longbalc\") selectFaces(\"\") selectFaces(\"x\") setTexture(\"shortbalc\")");

  shapeTree.addRule(GlobalShape);
  shapeTree.addRule(ColumnA);
  shapeTree.addRule(ColumnB);
  shapeTree.addRule(WithoutBalc);
  shapeTree.addRule(WithBalc);
  shapeTree.addRule(Balcony);
  shapeTree.addRule(TextureBalc);

  RuleNames::getInstance().addRule("GlobalShape");
  RuleNames::getInstance().addRule("ColumnA");
  RuleNames::getInstance().addRule("ColumnB");
  RuleNames::getInstance().addRule("WithoutBalc");
  RuleNames::getInstance().addRule("WithBalc");
  RuleNames::getInstance().addRule("Balcony");
  RuleNames::getInstance().addRule("TextureBalc");

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
