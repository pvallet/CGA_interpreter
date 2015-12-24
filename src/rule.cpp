#include "rule.h"

Rule::Rule (string _name, string _actions) :
  name(_name),
  actions(_actions)
  {}

Rule::~Rule() {}

void Rule::addNode(Node* node, const string& actions) {
  affectedNodes.push_back(node);
  additionalActions.insert( pair<Node*,string>(node, actions));
}
