#include "shape_tree.h"

#include <cerrno>
#include <iostream>
#include <fstream>

using namespace std;

ACT::ShapeTree::ShapeTree() :
	root(NULL, true)
{}

ACT::ShapeTree::~ShapeTree() {
   delete(scanner);
   scanner = nullptr;
   delete(parser);
   parser = nullptr;
}

void ACT::ShapeTree::outputGeometry() {
    ofstream output;
	output.open("out.off", ios::trunc);
	output << root.getSubGeometry();
	output.close();
}

void ACT::ShapeTree::displayGeometry() {
	outputGeometry();
	cout << endl;
	if (execl("./viewer", "./viewer") == -1)
		cout << strerror(errno) << endl;
}

void ACT::ShapeTree::setInitRule(Rule* rule) {
	initRule = rule;
	rule->addNode(&root);
}

void ACT::ShapeTree::executeActions(const string& actions) {
	std::stringstream ss( actions );

	delete(scanner);
	try
	{
		 scanner = new ACT::ACT_Scanner( &ss );
	}
	catch( std::bad_alloc &ba )
	{
		 std::cerr << "Failed to allocate scanner: (" <<
				ba.what() << "), exiting!!\n";
		 exit( EXIT_FAILURE );
	}

	delete(parser);
	try
	{
		 parser = new ACT::ACT_Parser( (*scanner), (*this) );
	}
	catch( std::bad_alloc &ba )
	{
		 std::cerr << "Failed to allocate parser: (" <<
				ba.what() << "), exiting!!\n";
		 exit( EXIT_FAILURE );
	}
	const int accept( 0 );
	cout << "enter parse" << endl;
	if( parser->parse() != accept )
	{
		 std::cerr << "Parse failed !\n";
	}
	cout << "exit parse" << endl;
}

void ACT::ShapeTree::addToRule(string rule, string actions) {
	for (auto it = rules.begin() ; it != rules.end() ; it++) {
		if ((*it)->getName() == rule)
			(*it)->addNode(affectedNode, actions);
	}
}

int ACT::ShapeTree::executeRule() {
	cout << "execute rule: " << rules.front()->getName() << endl;
	if (!rules.empty())	{
		for (	auto it = rules.front()->getNodes().begin() ;
					it != rules.front()->getNodes().end() ; it++) {
			affectedNode = *it;
			executeActions(rules.front()->getActions(*it));
		}
		rules.pop_front();
		return 0;
	}

	else
		return -1;
}

void ACT::ShapeTree::split(char axis, string pattern) {
	vector<Node*> 	resultNodes;
	vector<string> 	resultActions;

	Node* save = affectedNode;

	switch(axis) {
		case 'x': case 'X':
			affectedNode->split(X, resultNodes, resultActions, pattern);
			break;
		case 'y': case 'Y':
			affectedNode->split(Y, resultNodes, resultActions, pattern);
			break;
		case 'z': case 'Z':
			affectedNode->split(Z, resultNodes, resultActions, pattern);
			break;
	}

	for (unsigned int i = 0 ; i < resultNodes.size() ; i++) {
		affectedNode = resultNodes[i];
		executeActions(resultActions[i]);
	}

	affectedNode = save;
}
