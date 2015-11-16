#pragma once

#include <string>

#include "node.h"

using namespace std;

class Event {
private:
  string name;
  Node* groupNode;
public:
  Event (string _name, Node* _groupNode);
  virtual ~Event ();
};
