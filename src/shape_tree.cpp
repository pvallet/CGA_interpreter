#include "shape_tree.h"

#include <cerrno>
#include <iostream>
#include <fstream>

using namespace std;

ShapeTree::ShapeTree() :
	root(NULL, true)
{}

void ShapeTree::outputGeometry() {
    ofstream output;
	output.open("out.off", ios::trunc);
	output << root.getSubGeometry();
	output.close();
}

void ShapeTree::displayGeometry() {
	outputGeometry();
	cout << endl;
	if (execl("./viewer", "./viewer") == -1)
		cout << strerror(errno) << endl;
}

void ShapeTree::executeActions(Node* affectedNode, const string& actions) {

}

int ShapeTree::executeRule() {
	if (!rules.empty())	{
		for (	auto it = rules.front()->getNodes().begin() ;
					it != rules.front()->getNodes().end() ; it++) {
			executeActions(*it, rules.front()->getActions(*it));
		}
		rules.pop_front();
		return 0;
	}

	else
		return -1;
}
