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
  Rule (string _name);
  virtual ~Rule ();

  void addAction(const string& action, double weight);
  void addNode(Node* node, const string& actions = string());
  inline const string& getName() const {return name;}
  inline const list<Node*>& getNodes() const {return affectedNodes;}
  string getActions(Node* node) const; // Chose action at random according to the weights
  void setRecDepth(int depth) {recDepth = depth;}
  inline int getRecDepth() const {return recDepth;}
  void setFallback(const string& _fallback) {fallback = _fallback;}
  void setFallbackMode(bool mode) {fallbackMode = mode;}

private:
  string name;
  list<Node*> affectedNodes;
  map<Node*,string> additionalActions;
  vector<string> actions;
  vector<double> weights;

  bool fallbackMode;
  string fallback;

  double totalWeight;
  int recDepth;
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
