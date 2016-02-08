#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Node;

class Rule {
public:
  Rule (string _name);
  virtual ~Rule ();

  void addAction(string action, double weight);
  void addNode(Node* node, const string& actions = string());
  inline const string& getName() {return name;}
  inline const list<Node*>& getNodes() const {return affectedNodes;}
  string getActions(Node* node) const; // Chose action at random according to the weights

private:
  string name;
  list<Node*> affectedNodes;
  map<Node*,string> additionalActions;
  vector<string> actions;
  vector<double> weights;

  double totalWeight;
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
