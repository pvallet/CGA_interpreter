#pragma once

#include <string>

#include "node.h"
#include "rule.h"
#include "actions/actions_parser.h"
#include "actions/actions_scanner.h"

using namespace std;

namespace ACT { // ShapeTree is the driver for actions

class ShapeTree {

public:
	ShapeTree();
	~ShapeTree() {}

	Node* getRoot() {return &root;}

	void initFromFile(string path) {root.load(path);}
	void setTextureFile(string path) {texturePath = path;}
	// Name a sub rectangle of the texture file that will constitute a texture to be applied
	void addTextureRect(string name, double x0, double y0, double x1, double y1);

	void outputGeometryOFF();
	void displayGeometryOFF();
	void outputGeometryOBJ();
	void displayGeometryOBJ();
	void addRule(Rule* rule) {rules.push_back(rule);}
	void setInitRule(Rule* rule);

	int executeRule(); // Returns -1 if there is no more rule to be executed

	// Apply actions to the affected node
	void addToRule(string rule, string actions = string());
	void extrude(double value) {affectedNode = affectedNode->extrude(value);}
	void split(char axis, string pattern);
	void selectFaces(string expression); // For the moment only [(x|y|z)(pos|neg)] | all
	void setTexture(string texture);

private:
	// Parse function
	void executeActions(const string& actions);

	Node root;
	Node* affectedNode; // To execute actions on
	std::list<Rule*> rules;
	Rule* initRule;

	string texturePath;
	vector<Point_2> texCoord;
	map<string, int> textures; // the int specifies the first index of the texture coords
};

} /* End namespace ACT */
