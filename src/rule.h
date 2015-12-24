#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>

#include "node.h"

using namespace std;

class ShapeTree;

class Rule {
public:
  Rule (string _name, string _actions);
  virtual ~Rule ();

  void addNode(Node* node, const string& actions = string());
  const string& getName() {return name;}
  const list<Node*>& getNodes() const {return affectedNodes;}
  const string& getActions(Node* node) const {return actions + additionalActions.at(node);}

private:
  string name;
  list<Node*> affectedNodes;
  map<Node*,string> additionalActions;
  string actions;
};
