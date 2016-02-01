#include "shape_tree.h"

#include <cerrno>
#include <iostream>
#include <fstream>

using namespace std;

ACT::ShapeTree::ShapeTree() :
	root(NULL, true)
{}

void ACT::ShapeTree::addTextureRect(string name, double x0, double y0, double x1, double y1) {
	textures[name] = texCoord.size();

	texCoord.push_back(Point_2(x0, y0));
	texCoord.push_back(Point_2(x0+(x1-x0), y0));
	texCoord.push_back(Point_2(x0+(x1-x0), y0+(y1-y0)));
	texCoord.push_back(Point_2(x1, y1));
}

void ACT::ShapeTree::outputGeometryOFF() {
  ofstream output;
	output.open("out.off", ios::trunc);
	MeshResult res = root.getSubGeometry();
	output << res.mesh;
	output.close();
}

void ACT::ShapeTree::displayGeometryOFF() {
	outputGeometryOFF();
	cout << endl;
	if (execl("./viewer", "./viewer") == -1)
		cout << strerror(errno) << endl;
}

void ACT::ShapeTree::outputGeometryOBJ() {
	string out("out");

	MeshResult res = root.getSubGeometry();

  ofstream objStream, mtlStream;
  objStream.open(out + ".obj", ios::trunc);

  objStream << "# CGA-_interpreter generated building" << endl;
  objStream << "mtllib " << out << ".mtl" << endl;
  objStream << "o Building" << endl;

  map<vertex_descriptor, int> vInt;
	map<vertex_descriptor, int> vtInt;
  Mesh::Vertex_range::iterator v, v_end;

  int i = 1;
  for (boost::tie(v,v_end) = res.mesh.vertices(); v != v_end ; v++) {
    objStream << 'v' << ' ' << res.mesh.point(*v).x() << ' ' <<
                            	 res.mesh.point(*v).y() << ' ' <<
                            	 res.mesh.point(*v).z() << std::endl;

    vInt.insert(pair<vertex_descriptor, int>(*v,i));
    i++;
  }

	for (boost::tie(v,v_end) = res.mesh.vertices(); v != v_end ; v++) {
    objStream << "vt" << ' ' << texCoord[res.iTexCoord[*v]].x() << ' ' <<
                            		texCoord[res.iTexCoord[*v]].y() << std::endl;

    vInt.insert(pair<vertex_descriptor, int>(*v,i));
    i++;
  }

  objStream << "usemtl Texture" << endl;
  objStream << "s off" << endl;

  Mesh::Face_range::iterator f, f_end;
  for (boost::tie(f,f_end) = res.mesh.faces(); f != f_end ; f++) {
    objStream << 'f';
    CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
    boost::tie(v, v_end) = vertices_around_face(res.mesh.halfedge(*f), res.mesh);
    do
      objStream << ' ' << vInt[*v] << '/' << res.iTexCoord[*v]+1;
    while(++v != v_end);
    objStream << std::endl;
  }

  objStream.close();
  mtlStream.open(out + ".mtl", ios::trunc);

  mtlStream << "newmtl Texture" << endl <<
    "map_Kd " << texturePath << endl;

  mtlStream.close();
}

void ACT::ShapeTree::displayGeometryOBJ() {
	outputGeometryOBJ();
	cout << endl;
	if (execl("/usr/bin/meshlab", "meshlab", "./out.obj") == -1)
		cout << strerror(errno) << endl;
}

void ACT::ShapeTree::setInitRule(Rule* rule) {
	initRule = rule;
	rule->addNode(&root);
}

void ACT::ShapeTree::executeActions(const string& actions) {
	ACT::ACT_Parser  *parser  = nullptr;
  ACT::ACT_Scanner *scanner = nullptr;
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

	if( parser->parse() != accept )
	{
		 std::cerr << "Parse failed !\n";
	}
}

void ACT::ShapeTree::addToRule(string rule, string actions) {
	for (auto it = rules.begin() ; it != rules.end() ; it++) {
		if ((*it)->getName() == rule)
			(*it)->addNode(affectedNode, actions);
	}
}

int ACT::ShapeTree::executeRule() {
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

void ACT::ShapeTree::selectFaces(string expression) {
	if (expression == "all") {
		affectedNode->selectFace("xpos");
		affectedNode->selectFace("xneg");
		affectedNode->selectFace("ypos");
		affectedNode->selectFace("yneg");
		affectedNode->selectFace("zpos");
		affectedNode->selectFace("zneg");
	}

	else
		affectedNode->selectFace(expression);
}

void ACT::ShapeTree::setTexture(string texture) {
	vector<Point_2> coord;

	if (textures.find(texture) == textures.end())
		affectedNode->noTexture();
	else {
		for (int i = textures[texture] ; i < 4 ; i++) {
			coord.push_back(texCoord[i]);
		}
		affectedNode->setTexture(coord);
	}
}
