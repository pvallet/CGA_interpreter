#include "shape_tree.h"
#include "custom_join.h"

#include <cerrno>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

ACT::ShapeTree::ShapeTree() :
	root(this, NULL, true),
	outType(OFF),
	filename("out.off"),
	texCoord(4, Point_2(0,0)), // This stands for no texture
	roofAngle(30*M_PI/180),
	roofOffset(0.2),
	roofTexture(""),
	roofTexZoom(1),
	roofTexCoord(1, Point_2(0,0))
{
}

ACT::ShapeTree::~ShapeTree() {
	// for (auto it = rules.begin() ; it != rules.end() ; it++)
	// 	delete it->second;
}

void ACT::ShapeTree::initFromFile(const string& path) {
	root.load(path);
}

void ACT::ShapeTree::initFromRect(double x, double y) {
	Mesh axiom;
	vector<vertex_descriptor> vertices;

	vertices.push_back(axiom.add_vertex(Point_3(-x/2, 0, -y/2)));
	vertices.push_back(axiom.add_vertex(Point_3(-x/2, 0,  y/2)));
	vertices.push_back(axiom.add_vertex(Point_3( x/2, 0,  y/2)));
	vertices.push_back(axiom.add_vertex(Point_3( x/2, 0, -y/2)));

	axiom.add_face(	vertices[0],
									vertices[1],
									vertices[2],
									vertices[3]);

	root.setShape(axiom);
}

void ACT::ShapeTree::setOutputFilename(const string& _filename) {
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

void ACT::ShapeTree::setTextureFile(const string& path) {
	texturePath = path;
}

void ACT::ShapeTree::setRoofTexture(const string& path) {
	roofTexture = path;
}

void ACT::ShapeTree::addTextureRect(const string& name, double x0, double y0, double x1, double y1) {
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
	res.mesh += roof;
	output << res.mesh;
	output.close();
}

void ACT::ShapeTree::displayGeometryOFF() {
	outputGeometryOFF();
	if (execl("./mview", "./mview", "filename", NULL) == -1)
		cerr << "Unable to launch viewer: " << strerror(errno) << endl;
}

void ACT::ShapeTree::outputGeometryOBJ() {
	string out(filename);
	out.erase(out.size()-4,4);

	MeshResult res = root.getSubGeometry();

  ofstream objStream, mtlStream;
  objStream.open(out + ".obj", ios::trunc);

  objStream << "# CGA-_interpreter generated building" << endl;

	string mtlFile;
	size_t found = out.rfind("/");

	if (found == string::npos)
		mtlFile = out;
	else
		mtlFile = out.substr(found + 1);

  objStream << "mtllib " << mtlFile << ".mtl" << endl;
  objStream << "o Building " << endl;

  map<vertex_descriptor, int> vInt;
	map<vertex_descriptor, int> roofVInt;
  Mesh::Vertex_range::iterator v, v_end;

  int i = 1;
  for (boost::tie(v,v_end) = res.mesh.vertices(); v != v_end ; v++) {
    objStream << 'v' << ' ' << res.mesh.point(*v).x() << ' ' <<
                            	 res.mesh.point(*v).y() << ' ' <<
                            	 res.mesh.point(*v).z() << std::endl;

    vInt.insert(pair<vertex_descriptor, int>(*v,i));
    i++;
  }

	for (boost::tie(v,v_end) = roof.vertices(); v != v_end ; v++) {
    objStream << 'v' << ' ' << roof.point(*v).x() << ' ' <<
                            	 roof.point(*v).y() << ' ' <<
                            	 roof.point(*v).z() << std::endl;

    roofVInt.insert(pair<vertex_descriptor, int>(*v,i));
    i++;
  }

	for (unsigned int j = 0 ; j < texCoord.size() ; j++) {
		objStream << "vt" << ' ' << texCoord[j].x() << ' ' <<
                            		texCoord[j].y() << std::endl;
	}

	for (unsigned int j = 0 ; j < roofTexCoord.size() ; j++) {
		objStream << "vt" << ' ' << roofTexCoord[j].x() << ' ' <<
                            		roofTexCoord[j].y() << std::endl;
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

	objStream << "usemtl RoofTexture" << endl;
  objStream << "s off" << endl;

  for (boost::tie(f,f_end) = roof.faces(); f != f_end ; f++) {
    objStream << 'f';
    CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
    for (boost::tie(v, v_end) = vertices_around_face(roof.halfedge(*f), roof);
				v != v_end ; v++) {
      objStream << ' ' << roofVInt[*v] << '/' << iRoofTexCoord[*f][*v] + texCoord.size() + 1;
    }
    objStream << std::endl;
  }

  objStream.close();
  mtlStream.open(out + ".mtl", ios::trunc);

  mtlStream << "newmtl Texture" << endl << "map_Kd " << texturePath << endl;
	if (roofTexture != "")
		mtlStream << "newmtl RoofTexture" << endl << "map_Kd " << roofTexture << endl;

  mtlStream.close();
}

void ACT::ShapeTree::displayGeometryOBJ() {
	outputGeometryOBJ();
	if (execl("/usr/bin/meshlab", "meshlab", "./out.obj", NULL) == -1)
		cerr << "Unable to launch meshlab: " << strerror(errno) << endl;
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

void ACT::ShapeTree::setInitRule(const string& ruleName) {
	affectedNode = &root;
	addToRule(ruleName);
}

void ACT::ShapeTree::addToRule(const string& ruleName, const string& actions) {
	bool active = false;
	for (auto it = activeRules.begin() ; it != activeRules.end() ; it++) {
		if ((*it)->getName() == ruleName) {
			(*it)->addNode(affectedNode, actions);
			active = true;
		}
	}

	if (!active) {
		if (rules.find(ruleName) != rules.end()) { // else don't do anything
			if (rules[ruleName]->getRecDepth() > 0) {
				activeRules.push_back(new Rule(*rules[ruleName]));
				activeRules.back()->addNode(affectedNode, actions);
				rules[ruleName]->setRecDepth(rules[ruleName]->getRecDepth() - 1);
			}

			else if (rules[ruleName]->getRecDepth() == -1) {
				activeRules.push_back(new Rule(*rules[ruleName]));
				activeRules.back()->addNode(affectedNode, actions);
			}

			else {
				activeRules.push_back(new Rule(*rules[ruleName]));
				activeRules.back()->setFallbackMode(true);
				activeRules.back()->addNode(affectedNode, actions);
			}
		}
	}
}

int ACT::ShapeTree::executeRule() {
	if (!activeRules.empty())	{
		for (	auto it = activeRules.front()->getNodes().begin() ;
					it != activeRules.front()->getNodes().end() ; it++) {
			affectedNode = *it;
			std::cout << activeRules.front()->getName() << " " <<
									 activeRules.front()->getActions(*it) << std::endl;
			executeActions(activeRules.front()->getActions(*it));
		}

		delete activeRules.front();
		activeRules.pop_front();
		return 0;
	}

	else
		return -1;
}

void ACT::ShapeTree::split(char axis, const string& pattern, const string& actions) {
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
		executeActions(resultActions[i]);// + " " + actions);
	}

	affectedNode = save;
}

void ACT::ShapeTree::selectFaces(const string& expression) {
	if (affectedNode->isFirstTimeSelect()) {
		affectedNode->selectFace("");
		affectedNode->selected();
	}

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

void ACT::ShapeTree::setTexture(const string& texture) {
	if (textures.find(texture) == textures.end())
		affectedNode->noTexture();
	else
		affectedNode->setTexture(textures[texture]);
}

void ACT::ShapeTree::addToRoof() {
	vector<vector<Point_3> > ceiling;
	affectedNode->getCeiling(ceiling);

	for (unsigned int i = 0 ; i < ceiling.size() ; i++) {
		Kernel::FT level = ceiling[i].front().y();

		Polygon_2 newPieceOfRoof;

		for (int j = ceiling[i].size()-1 ; j >= 0 ; j--) {
			newPieceOfRoof.push_back(Point_2(ceiling[i][j].x(), ceiling[i][j].z()));
		}

		roofLevels[level].push_back(Polygon_with_holes_2(newPieceOfRoof));

		std::list<Polygon_with_holes_2> res;
		CstmCGAL::join (roofLevels[level], res);

		roofLevels[level] = res;

		auto it = roofLevels.find(level);
		auto prev = it;

		if (it != roofLevels.begin()) {
			do {
				it--;
				list<Polygon_with_holes_2> res;
				CstmCGAL::join (it->second, prev->second,	res);
				it->second = res;
				prev--;
			} while (it != roofLevels.begin());
		}
	}
}

void ACT::ShapeTree::computeRoof() {
	// We run through the loop in reverse to be able to remove useless geometry in a future version
	for (auto lvl = roofLevels.rbegin() ; lvl != roofLevels.rend() ; lvl++) {
		for (auto it = lvl->second.begin() ; it != lvl->second.end() ; it++) {

			PwhPtr itWithOffset = CstmCGAL::applyOffset(roofOffset, *it);

			SsPtr iss = CGAL::create_interior_straight_skeleton_2(*itWithOffset);
			map<Ss::Vertex_const_handle, vertex_descriptor> vertices;

		  for ( Ss::Vertex_const_iterator v = iss->vertices_begin(); v != iss->vertices_end(); v++ ) {
		    vertices.insert(pair<Ss::Vertex_const_handle, vertex_descriptor>(
		      v, roof.add_vertex(Point_3(	v->point().x(),
																			lvl->first + tan(roofAngle)*(v->time()-roofOffset),
																			v->point().y())) ));
		  }

		  for ( Ss::Face_const_iterator f = iss->faces_begin(); f != iss->faces_end(); ++f ) {
		    vector<vertex_descriptor> face;
		    Ss::Halfedge_const_handle hbegin = f->halfedge();
		    Ss::Halfedge_const_handle h = hbegin;
				Ss::Halfedge_const_handle h_contour;
		    do {
					if (!h->is_bisector())
						h_contour = h;
		      face.push_back(vertices[h->vertex()]);
		      h = h->prev();
		    } while (h != hbegin);

		    face_descriptor newf = roof.add_face(boost::make_iterator_range(face.begin(), face.end()));
				// CstmCGAL::splitFace(roof, newf);

				Point_2 origin = h_contour->opposite()->vertex()->point();
				Vector_2 unit_contour = Vector_2(origin, h_contour->vertex()->point());
				if (unit_contour.squared_length() == 0)
					std::cerr << "Error in computeRoof(): HDS of straight skeleton is ill formed" << std::endl;
				else {
					unit_contour = unit_contour / sqrt(unit_contour.squared_length());

					h = hbegin;
					do {
						Vector_2 OM = Vector_2(origin, h->vertex()->point());
						iRoofTexCoord[newf][vertices[h->vertex()]] = insertRoofITex(Point_2(
							OM * unit_contour / roofTexZoom,
							h->vertex()->time() / cos(roofAngle) / roofTexZoom));

						h = h->prev();
					} while (h != hbegin);
				}
		  }
		}
	}
}

int ACT::ShapeTree::insertRoofITex(Point_2 point) {
	if (roofTexture == "")
		return 0;

	else {
		for (unsigned int i = 0 ; i < roofTexCoord.size() ; i++) {
			if (roofTexCoord[i] == point)
				return i;
		}

		roofTexCoord.push_back(point);

		return roofTexCoord.size()-1;
	}
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
