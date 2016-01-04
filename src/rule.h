#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "node.h"

using namespace std;

class Rule {
public:
  Rule (string _name, string _actions);
  virtual ~Rule ();

  void addNode(Node* node, const string& actions = string());
  const string& getName() {return name;}
  const list<Node*>& getNodes() const {return affectedNodes;}
  string getActions(Node* node) const {return actions + additionalActions.at(node);}

private:
  string name;
  list<Node*> affectedNodes;
  map<Node*,string> additionalActions;
  string actions;
};

// This list is to be generated and then sent to the action lexer to identify rules
// This is a singleton

class RuleNames {
public:
  static RuleNames& getInstance() {
    static RuleNames instance;
    return instance;
  }

  void addRule(string rule) {rules.insert(rule);}
  bool isRule(char* name) const {return rules.find(string(name)) != rules.end();}

private:
  RuleNames () {}
  RuleNames(RuleNames const&)      = delete;
  void operator=(RuleNames const&) = delete;

  static set<string> rules;
};
