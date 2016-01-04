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
	~ShapeTree();

	void initFromFile(string path) {root.load(path);}
	Node* getRoot() {return &root;}

	void outputGeometry();
	void displayGeometry();
	void addRule(Rule* rule) {rules.push_back(rule);}
	void setInitRule(Rule* rule);

	int executeRule(); // Returns -1 if there is no more rule to be executed

	// Apply actions to the affected node
	void addToRule(string rule, string actions = string());
	void extrude(double value) {affectedNode = affectedNode->extrude(value);}
	void split(char axis, string pattern);

private:
	// Parse function
	void executeActions(const string& actions);

	ACT::ACT_Parser  *parser  = nullptr;
  ACT::ACT_Scanner *scanner = nullptr;

	Node root;
	Node* affectedNode; // To execute actions upon
	std::list<Rule*> rules;
	Rule* initRule;
};

} /* End namespace ACT */
