#include "event.h"

Event::Event (string _name, Node* _groupNode) :
  name(_name),
  groupNode(_groupNode) {
}

Event::~Event () {}
