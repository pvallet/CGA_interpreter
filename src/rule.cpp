#include "rule.h"
#include <cstdlib>
#include <iostream>

Rule::Rule (string _name) :
  name(_name),
  fallbackMode(false),
  fallback(""),
  recDepth(-1) // infinity
  {}

Rule::~Rule() {}

void Rule::addAction(const string& action, double weight) {
  if (weight < 0)
    std::cerr << "Rule::addAction: negative weight" << std::endl;

  else {
    actions.push_back(string(action));
    weights.push_back(weight);

    totalWeight += weight;
  }
}

void Rule::addNode(Node* node, const string& actions) {
  affectedNodes.push_back(node);
  additionalActions.insert( pair<Node*,string>(node, actions));
}

string Rule::getActions(Node* node) const {
  if (fallbackMode)
    return fallback;

  else {
    double chosenAction = rand() * totalWeight / RAND_MAX;
    double partialWeight = 0;
    int iRes = 0;
    for (unsigned int i = 0 ; i < weights.size() ; i++) {
      partialWeight += weights[i];
      if (partialWeight > chosenAction) {
        iRes = i;
        break;
      }
    }

    if (additionalActions.find(node) == additionalActions.end())
      return actions[iRes];
    else
      return actions[iRes] + additionalActions.at(node);
  }
}

set<string> RuleNames::rules;
