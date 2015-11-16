#pragma once

#include <list>
#include <string>
#include <vector>

#include "node.h"

using namespace std;

class ShapeTree;

class Rule {
private:
  string name;
  list<Node*> affectedNodes;
  string actions;
public:
  Rule (string _name, string _actions);
  virtual ~Rule ();

  void addNode(Node* node) {affectedNodes.push_back(node);}
  const string& getName() {return name;}
  const list<Node*>& getNodes() {return affectedNodes;}
  const string& getActions() {return actions;}
};
