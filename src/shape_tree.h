#pragma once

#include <string>

#include "node.h"
#include "rule.h"

using namespace std;

class ShapeTree {

public:
	ShapeTree();
	~ShapeTree() {}

	void initFromFile(string path) {root.load(path);}
	Node* getRoot() {return &root;}

	void outputGeometry();
	void displayGeometry();

	void addRule(Rule* rule) {rules.push_back(rule);}
	void setInitRule(Rule* rule) {initRule = rule;}

	int executeRule(); // Returns -1 if there is no more rule to be executed

private:
	void executeActions(Node* affectedNode, const string& actions);

	Node root;
	std::list<Rule*> rules;
	Rule* initRule;
};
