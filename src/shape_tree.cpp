#include "shape_tree.h"

#include <cerrno>
#include <iostream>
#include <fstream>

using namespace std;

ACT::ShapeTree::ShapeTree() :
	root(this, NULL, true),
	outType(OFF),
	filename("out.off"),
	texCoord(4, Point_2(0,0)) // This stands for no texture
{
}

void ACT::ShapeTree::initFromFile(string path) {
	root.load(path);
}

void ACT::ShapeTree::setOutputFilename(string _filename) {
	filename = _filename;
	string extension = filename.substr(filename.size()-4);

	if (extension == ".off" || extension == ".OFF")
		outType = OFF;
	else if (extension == ".obj" || extension == ".OBJ")
		outType = OBJ;
	else {
		cerr << "Error in setOutputFilename: unknown extension: " << extension << endl;
		outType = OFF;
	}
}

void ACT::ShapeTree::setTextureFile(string path) {
	texturePath = path;
}

void ACT::ShapeTree::addTextureRect(string name, double x0, double y0, double x1, double y1) {
	textures[name] = texCoord.size();

	addTextureCoord(x0, y0, x1, y1);
}

void ACT::ShapeTree::addTextureCoord(double x0, double y0, double x1, double y1) {
	texCoord.push_back(Point_2(x0, y0));
	texCoord.push_back(Point_2(x1, y0));
	texCoord.push_back(Point_2(x1, y1));
	texCoord.push_back(Point_2(x0, y1));
}

void ACT::ShapeTree::outputGeometry() {
	if (outType == OFF)
		outputGeometryOFF();
	else
		outputGeometryOBJ();
}

void ACT::ShapeTree::displayGeometry() {
	if (outType == OFF)
		displayGeometryOFF();
	else
		displayGeometryOBJ();
}

void ACT::ShapeTree::outputGeometryOFF() {
  ofstream output;
	output.open(filename, ios::trunc);
	MeshResult res = root.getSubGeometry();
	output << res.mesh;
	output.close();
}

void ACT::ShapeTree::displayGeometryOFF() {
	outputGeometryOFF();
	if (execl("./viewer", "./viewer", NULL) == -1)
		cerr << strerror(errno) << endl;
}

void ACT::ShapeTree::outputGeometryOBJ() {
	string out(filename);
	out.erase(out.size()-4,4);

	MeshResult res = root.getSubGeometry();

  ofstream objStream, mtlStream;
  objStream.open(out + ".obj", ios::trunc);

  objStream << "# CGA-_interpreter generated building" << endl;
  objStream << "mtllib " << out << ".mtl" << endl;
  objStream << "o Building" << endl;

  map<vertex_descriptor, int> vInt;
  Mesh::Vertex_range::iterator v, v_end;

  int i = 1;
  for (boost::tie(v,v_end) = res.mesh.vertices(); v != v_end ; v++) {
    objStream << 'v' << ' ' << res.mesh.point(*v).x() << ' ' <<
                            	 res.mesh.point(*v).y() << ' ' <<
                            	 res.mesh.point(*v).z() << std::endl;

    vInt.insert(pair<vertex_descriptor, int>(*v,i));
    i++;
  }

	for (unsigned int j = 0 ; j < texCoord.size() ; j++) {
		objStream << "vt" << ' ' << texCoord[j].x() << ' ' <<
                            		texCoord[j].y() << std::endl;
	}

  objStream << "usemtl Texture" << endl;
  objStream << "s off" << endl;

  Mesh::Face_range::iterator f, f_end;
  for (boost::tie(f,f_end) = res.mesh.faces(); f != f_end ; f++) {
    objStream << 'f';
    CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
    boost::tie(v, v_end) = vertices_around_face(res.mesh.halfedge(*f), res.mesh);
		i = res.iTexCoord[*f]+1;
    do {
      objStream << ' ' << vInt[*v] << '/' << i;
			i++;
    } while(++v != v_end);
    objStream << std::endl;
  }

  objStream.close();
  mtlStream.open(out + ".mtl", ios::trunc);

  mtlStream << "newmtl Texture" << endl << "map_Kd " << texturePath << endl;

  mtlStream.close();
}

void ACT::ShapeTree::displayGeometryOBJ() {
	outputGeometryOBJ();
	if (execl("/usr/bin/meshlab", "meshlab", "./out.obj", NULL) == -1)
		cerr << strerror(errno) << endl;
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
		 std::cerr << "Actions: Failed to allocate scanner: (" <<
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
		 std::cerr << "Actions: Failed to allocate parser: (" <<
				ba.what() << "), exiting!!\n";
		 exit( EXIT_FAILURE );
	}
	const int accept( 0 );

	if( parser->parse() != accept )
	{
		 std::cerr << "Actions: Parse failed !\n";
	}
}

void ACT::ShapeTree::addRule(Rule* rule) {
	rules.insert(pair<string, Rule*>(rule->getName(), rule));
}

void ACT::ShapeTree::setInitRule(string ruleName) {
	affectedNode = &root;
	addToRule(ruleName);
}

void ACT::ShapeTree::addToRule(string ruleName, string actions) {
	bool active = false;
	for (auto it = activeRules.begin() ; it != activeRules.end() ; it++) {
		if ((*it)->getName() == ruleName) {
			(*it)->addNode(affectedNode, actions);
			active = true;
		}
	}

	if (!active) {
		if (rules.find(ruleName) != rules.end()) { // else don't do anything
			activeRules.push_back(new Rule(*rules[ruleName]));
			activeRules.back()->addNode(affectedNode);
		}
	}
}

int ACT::ShapeTree::executeRule() {
	if (!activeRules.empty())	{
		for (	auto it = activeRules.front()->getNodes().begin() ;
					it != activeRules.front()->getNodes().end() ; it++) {
			affectedNode = *it;
			executeActions(activeRules.front()->getActions(*it));
		}
		//delete activeRules.front();
		activeRules.pop_front();
		return 0;
	}

	else
		return -1;
}

void ACT::ShapeTree::split(char axis, string pattern) {
	vector<Node*> resultNodes;
	vector<string> resultActions;

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
	if (affectedNode->isFirstTimeSelect())
		affectedNode->selectFace("");

	if (expression == "all")
		affectedNode->selectAllFaces();
	else if (expression == "x") {
		affectedNode->selectFace("xpos");
		affectedNode->selectFace("xneg");
	}
	else if (expression == "y") {
		affectedNode->selectFace("ypos");
		affectedNode->selectFace("yneg");
	}
	else if (expression == "z") {
		affectedNode->selectFace("zpos");
		affectedNode->selectFace("zneg");
	}

	else
		affectedNode->selectFace(expression);
}

void ACT::ShapeTree::setTexture(string texture) {
	if (textures.find(texture) == textures.end())
		affectedNode->noTexture();
	else
		affectedNode->setTexture(textures[texture]);
}

void ACT::ShapeTree::removeFaces() {
	affectedNode = affectedNode->removeFaces();
}

// base[0] = texCoord[texID]

/* base[3] --- base[2]
 * 	 |						|
 * base[0] --- base[1] */

int ACT::ShapeTree::splitTexture(int texID, const vector<double>& weights,
																						Orientation orientation) {
	TexSplitKey tsk;
	tsk.texID = texID;
	tsk.weights = weights;
	tsk.orientation = orientation;

	if (subTextureLocation.find(tsk) == subTextureLocation.end()) {
		int res = texCoord.size();
		double x0 = texCoord[texID].x();
		double x1 = texCoord[texID+1].x();
		double y0 = texCoord[texID].y();
		double y1 = texCoord[texID+2].y();

		double totalWeight = 0;
		for (unsigned int i = 0 ; i < weights.size() ; i++)
			totalWeight += weights[i];

		double ratio;

		switch (orientation) {
			case VERTICAL:
				ratio = (y1 - y0) / totalWeight;
				y1 = y0 + weights[0] * ratio;
				addTextureCoord(x0, y0, x1, y1);
				for (unsigned int i = 1 ; i < weights.size() ; i++) {
					y0 = y1;
					y1 += weights[i] * ratio;
					addTextureCoord(x0, y0, x1, y1);
				}
				break;
			case HORIZONTAL:
				ratio = (x1 - x0) / totalWeight;
				x1 = x0 + weights[0] * ratio;
				addTextureCoord(x0, y0, x1, y1);
				for (unsigned int i = 1 ; i < weights.size() ; i++) {
					x0 = x1;
					x1 += weights[i] * ratio;
					addTextureCoord(x0, y0, x1, y1);
				}
				break;
		}

		subTextureLocation.insert(pair<TexSplitKey, int>(tsk, res));
		return res;
	}

	else {
		return subTextureLocation[tsk];
	}
}
