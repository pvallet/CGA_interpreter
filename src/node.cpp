#include "node.h"

#include <cstddef>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <map>

#include "shape_tree.h"
#include "split_pattern/split_pattern_driver.h"

using namespace std;

typedef pair<vertex_descriptor,vertex_descriptor> Match;

Node::Node(ACT::ShapeTree* _shapeTree, Node* _parent, bool _visible) :
	shapeTree(_shapeTree),
	parent(_parent),
	visible(_visible),
	firstTimeSelect(true)
{}

void Node::selectAllFaces() {
	selectedFaces.clear();

	Mesh::Face_range::iterator f, f_end;
	for (boost::tie(f,f_end) = shape.faces(); f != f_end ; f++) {
		selectedFaces.push_back(*f);
	}
}

void Node::noTexture() {
	iTexCoord.clear();

	Mesh::Face_range::iterator f, f_end;
	for (boost::tie(f,f_end) = shape.faces(); f != f_end ; f++) {
		iTexCoord[*f] = 0;
	}
}

void Node::load(const string& path) {
	ifstream input;
	input.open(path);
	input >> shape;
	input.close();
	selectAllFaces();
	noTexture();
}

void Node::setShape(Mesh _shape) {
	shape = _shape;
	selectAllFaces();
	noTexture();
}

void Node::setVisible(bool _visible) {
	if (_visible) {
		visible = true;
		if (parent != NULL)
			parent->setVisible(true);
	}

	else {
		visible = false;
		for (auto it = children.begin() ; it != children.end() ; it++)
			(*it)->setVisible(false);
	}
}

void Node::addChild(Node* _child) {
	_child->setParent(this);
	children.push_back(_child);
}

MeshResult Node::getSubGeometry() {
	MeshResult res;

	if (children.empty()) {
		res.mesh = shape;
		res.iTexCoord = iTexCoord;
		return res;
	}

	else {
		bool hasVisibleChild = false;
		for (auto chld = children.begin() ; chld != children.end() ; chld++) {
			if ((*chld)->isVisible()) {
				hasVisibleChild = true;
				MeshResult partialRes = (*chld)->getSubGeometry();

				// Maps vertices to the appropriate texture coordinates
				for (auto vd = partialRes.iTexCoord.begin() ;
									vd != partialRes.iTexCoord.end() ; vd++) {
					res.iTexCoord.insert(pair<face_descriptor, int>(
						// This addition is specified in the CGAL doc
						(face_descriptor) (vd->first +
															 res.mesh.number_of_faces() +
															 res.mesh.number_of_removed_faces()),
															 vd->second));
				}

				// add vertices
				res.mesh += partialRes.mesh;
			}
		}

		if (hasVisibleChild)
			return res;

		else {
			res.mesh = shape;
			res.iTexCoord = iTexCoord;
			return res;
		}
 	}
}

void Node::getCeiling(vector<vector<Point_3> >& result) {
	list<face_descriptor> save = selectedFaces;

	selectFace(""); selectFace("ypos");

	for (auto f = selectedFaces.begin() ; f != selectedFaces.end() ; f++) {
		vector<Point_3> 					innerResult;
		CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
		for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(*f), shape);
				v != v_end; v++){
			innerResult.push_back(shape.point(*v));
		}
		result.push_back(innerResult);
	}

	selectedFaces = save;
}

Node* Node::translate(Kernel::RT dx, Kernel::RT dy, Kernel::RT dz) {
	Mesh nShape;
	map<vertex_descriptor, vertex_descriptor> idxInNewShape;
	CGAL::Vector_3<Kernel> translation(dx,dy,dz);

	Mesh::Vertex_range::iterator v, v_end;
	for (boost::tie(v,v_end) = shape.vertices(); v != v_end ; v++) {
		idxInNewShape.insert(pair<vertex_descriptor, vertex_descriptor>(
			*v, nShape.add_vertex(shape.point(*v) + translation)));
	}

	map<face_descriptor, int> nITexCoord;

	Mesh::Face_range::iterator f, f_end;
	for (boost::tie(f,f_end) = shape.faces(); f != f_end ; f++) {
		vector<vertex_descriptor> vFace;

		CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
    for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(*f), shape);
        v != v_end; v++){
    	vFace.push_back(*v);
    }

		face_descriptor nFace;

		if (vFace.size() == 3)
			nFace = nShape.add_face(idxInNewShape[vFace[0]],
															idxInNewShape[vFace[1]],
															idxInNewShape[vFace[2]]);
		else // vFace.size() == 4
			nFace = nShape.add_face(idxInNewShape[vFace[0]],
															idxInNewShape[vFace[1]],
															idxInNewShape[vFace[2]],
															idxInNewShape[vFace[3]]);

		nITexCoord.insert(pair<face_descriptor, int>( nFace, iTexCoord[*f] ));
	}

	Node* tr = new Node(shapeTree, this, true);
	tr->setShape(nShape);
	tr->setITexCoord(nITexCoord);
	addChild(tr);
	return tr;
}

Node* Node::extrude(Kernel::RT height) {
	Mesh nShape;
  for (auto f = selectedFaces.begin() ; f != selectedFaces.end() ; f++) {

  	vector<vertex_descriptor> iPrevShape;

  	CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
    for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(*f), shape);
        v != v_end; v++){

    	iPrevShape.push_back(*v);
    }

    CGAL::Vector_3<Kernel> normal = CGAL::unit_normal(shape.point(iPrevShape[0]),
													    											 	shape.point(iPrevShape[1]),
													    											 	shape.point(iPrevShape[2]));

    normal = normal * height;

		vector<vertex_descriptor> iNewShape, iNewShapeExtr;

		for (unsigned int i = 0 ; i < iPrevShape.size() ; i++) {
    	iNewShape.push_back(nShape.add_vertex(shape.point(iPrevShape[i])));
			iNewShapeExtr.push_back(nShape.add_vertex(shape.point(iPrevShape[i]) + normal));
    }

		face_descriptor fd;

    for (unsigned int i = 0 ; i < iNewShape.size() ; i++) {
    	fd = nShape.add_face(	iNewShape[i],
							    					iNewShape[(i+1) % iNewShape.size()],
							    					iNewShapeExtr[(i+1) % iNewShapeExtr.size()],
							    					iNewShapeExtr[i]);

    	if (fd == Mesh::null_face())
				cerr << "Extrude: Unable to add face" << endl;
    }

    nShape.add_face(iNewShapeExtr[0], iNewShapeExtr[1], iNewShapeExtr[2], iNewShapeExtr[3]);
  }

	Node* save = new Node(shapeTree, this, true);
	Node* extr = new Node(shapeTree, this, true);
	save->setShape(shape);
	save->setITexCoord(iTexCoord);
	extr->setShape(nShape);
	addChild(save);
	addChild(extr);
	return extr;
}

void Node::distributeX(
	vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
	vector<Mesh>* nShapes,
	const vector<vertex_descriptor>& sortedVertices,
	const vector<double>& weights) {

	double nxtSeparator = shape.point(sortedVertices.front()).x(); // minCoord
	unsigned int j = 0;

	for (unsigned int i = 0 ; i < nShapes->size() ; i++) {
		nxtSeparator += weights[i];

		for (; j < sortedVertices.size() && shape.point(sortedVertices[j]).x() < nxtSeparator; j++) {
			vertex_descriptor indexAdded = (*nShapes)[i].add_vertex(shape.point(sortedVertices[j]));
			(*matchVertexIn)[i].insert( Match(sortedVertices[j], indexAdded) );

			CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
			for(boost::tie(h, h_end) = halfedges_around_target(sortedVertices[j], shape);
					h != h_end; h++){

				if (shape.point(shape.source(*h)).x() > nxtSeparator) {
					double sep = nxtSeparator - weights[i];
					unsigned int k = i;
					Point_3 currentPoint = shape.point(sortedVertices[j]);

					// Put new vertices at each border cutting the edge
					// We need the condition over k when the weights do not some up to maxCoord
					while (shape.point(shape.source(*h)).x() > sep && k < weights.size() - 1) {
						sep += weights[k];

						Vector_3 crossingEdge(currentPoint, shape.point(shape.source(*h)));

						crossingEdge = crossingEdge * Kernel::RT( // Thales
							(sep - currentPoint.x()) /
							(shape.point(shape.source(*h)).x() - currentPoint.x()) );

						currentPoint = currentPoint + crossingEdge;

						(*matchVertexIn)[k].insert( Match(shape.source(*h), (*nShapes)[k].add_vertex(currentPoint)) );
						(*matchVertexIn)[k+1].insert( Match(sortedVertices[j], (*nShapes)[k+1].add_vertex(currentPoint)) );

						k++;
					}

					vertex_descriptor sourceAdded = (*nShapes)[k].add_vertex(shape.point(shape.source(*h)));
					(*matchVertexIn)[k].insert( Match(shape.source(*h), sourceAdded) );
				}
			}
		}
	}
}

void Node::distributeY(
	vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
	vector<Mesh>* nShapes,
	const vector<vertex_descriptor>& sortedVertices,
	const vector<double>& weights) {

	double nxtSeparator = shape.point(sortedVertices.front()).y(); // minCoord
	unsigned int j = 0;

	for (unsigned int i = 0 ; i < nShapes->size() ; i++) {
		nxtSeparator += weights[i];

		for (; j < sortedVertices.size() && shape.point(sortedVertices[j]).y() < nxtSeparator; j++) {
			vertex_descriptor indexAdded = (*nShapes)[i].add_vertex(shape.point(sortedVertices[j]));
			(*matchVertexIn)[i].insert( Match(sortedVertices[j], indexAdded) );

			CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
			for(boost::tie(h, h_end) = halfedges_around_target(sortedVertices[j], shape);
					h != h_end; h++){

				if (shape.point(shape.source(*h)).y() > nxtSeparator) {
					double sep = nxtSeparator - weights[i];
					unsigned int k = i;
					Point_3 currentPoint = shape.point(sortedVertices[j]);

					// Put new vertices at each border cutting the edge
					// We need the condition over k when the weights do not some up to maxCoord
					while (shape.point(shape.source(*h)).y() > sep && k < weights.size() - 1) {
						sep += weights[k];

						Vector_3 crossingEdge(currentPoint, shape.point(shape.source(*h)));

						crossingEdge = crossingEdge * Kernel::RT( // Thales
							(sep - currentPoint.y()) /
							(shape.point(shape.source(*h)).y() - currentPoint.y()) );

						currentPoint = currentPoint + crossingEdge;

						(*matchVertexIn)[k].insert( Match(shape.source(*h), (*nShapes)[k].add_vertex(currentPoint)) );
						(*matchVertexIn)[k+1].insert( Match(sortedVertices[j], (*nShapes)[k+1].add_vertex(currentPoint)) );

						k++;
					}

					vertex_descriptor sourceAdded = (*nShapes)[k].add_vertex(shape.point(shape.source(*h)));
					(*matchVertexIn)[k].insert( Match(shape.source(*h), sourceAdded) );
				}
			}
		}
	}
}

void Node::distributeZ(
	vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
	vector<Mesh>* nShapes,
	const vector<vertex_descriptor>& sortedVertices,
	const vector<double>& weights) {

	double nxtSeparator = shape.point(sortedVertices.front()).z(); // minCoord
	unsigned int j = 0;

	for (unsigned int i = 0 ; i < nShapes->size() ; i++) {
		nxtSeparator += weights[i];

		for (; j < sortedVertices.size() && shape.point(sortedVertices[j]).z() < nxtSeparator; j++) {
			vertex_descriptor indexAdded = (*nShapes)[i].add_vertex(shape.point(sortedVertices[j]));
			(*matchVertexIn)[i].insert( Match(sortedVertices[j], indexAdded) );

			CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
			for(boost::tie(h, h_end) = halfedges_around_target(sortedVertices[j], shape);
					h != h_end; h++){

				if (shape.point(shape.source(*h)).z() > nxtSeparator) {
					double sep = nxtSeparator - weights[i];
					unsigned int k = i;
					Point_3 currentPoint = shape.point(sortedVertices[j]);

					// Put new vertices at each border cutting the edge
					// We need the condition over k when the weights do not some up to maxCoord
					while (shape.point(shape.source(*h)).z() > sep && k < weights.size() - 1) {
						sep += weights[k];

						Vector_3 crossingEdge(currentPoint, shape.point(shape.source(*h)));

						crossingEdge = crossingEdge * Kernel::RT( // Thales
							(sep - currentPoint.z()) /
							(shape.point(shape.source(*h)).z() - currentPoint.z()) );

						currentPoint = currentPoint + crossingEdge;

						(*matchVertexIn)[k].insert( Match(shape.source(*h), (*nShapes)[k].add_vertex(currentPoint)) );
						(*matchVertexIn)[k+1].insert( Match(sortedVertices[j], (*nShapes)[k+1].add_vertex(currentPoint)) );

						k++;
					}

					vertex_descriptor sourceAdded = (*nShapes)[k].add_vertex(shape.point(shape.source(*h)));
					(*matchVertexIn)[k].insert( Match(shape.source(*h), sourceAdded) );
				}
			}
		}
	}
}

void Node::reconstruct(
	const vector<map<vertex_descriptor, vertex_descriptor> >& matchVertexIn,
	const vector<double>& weights, // needed to adapt the textures
	vector<Mesh>* nShapes) {

	for (unsigned int i = 0 ; i < nShapes->size() ; i++) {
		Mesh::Face_range::iterator f, f_end;
    for (boost::tie(f,f_end) = shape.faces(); f != f_end ; f++) {
    	bool addFace = true;

    	vector<vertex_descriptor> aroundNewFace;

    	CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
	    for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(*f), shape);
	        v != v_end; v++) {

	    	if (matchVertexIn[i].find(*v) == matchVertexIn[i].end()) {
	    		addFace = false;
	    		break;
	    	}

	    	else {
	    		aroundNewFace.push_back(matchVertexIn[i].at(*v));
	    	}
	    }

	    if (addFace) {
		    if (aroundNewFace.size() == 4)
		    	(*nShapes)[i].add_face(aroundNewFace[0], aroundNewFace[1],
																 aroundNewFace[2], aroundNewFace[3]);

		    else
    			cerr << "Extrude: This face is not a quad: " << *f << endl;
	    }
    }
	}
}

void Node::preserveTextures(Axis axis, const vector<Node*>& nodes,
																			 const vector<double>& weights) {
	selectFace("");
	switch(axis) { // Textures on ends don't need to be splitted
		case X:
			selectFace("xneg"); nodes.front()->selectFace("xneg");
			nodes.front()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.front()->selectFace("");

			selectFace("xpos"); nodes.back()->selectFace("xpos");
			nodes.back()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.back()->selectFace("");

			selectFace("ypos"); selectFace("yneg"); selectFace("zpos"); selectFace("zpos");
			break;
		case Y:
			selectFace("yneg"); nodes.front()->selectFace("yneg");
			nodes.front()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.front()->selectFace("");

			selectFace("ypos"); nodes.back()->selectFace("ypos");
			nodes.back()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.back()->selectFace("");

			selectFace("xpos"); selectFace("xneg"); selectFace("zpos"); selectFace("zpos");
			break;
		case Z:
			selectFace("zneg"); nodes.front()->selectFace("zneg");
			nodes.front()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.front()->selectFace("");

			selectFace("zpos"); nodes.back()->selectFace("zpos");
			nodes.back()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.back()->selectFace("");

			selectFace("xpos"); selectFace("xpos"); selectFace("ypos"); selectFace("yneg");
			break;
	}

	list<face_descriptor> save = selectedFaces;

	// Split textures on the sides
	for (auto it = selectedFaces.begin() ; it != selectedFaces.end() ; it++) {
		if (iTexCoord[*it] == 0) {
			for (unsigned int i = 0 ; i < nodes.size() ; i++) {
				nodes[i]->selectFace("");
				nodes[i]->selectFace(getFaceString(*it));
				nodes[i]->setTexture(0);
				nodes[i]->selectFace("");
			}
		}

		else {
			int indexFirstNewTexture;
			if (axis == Y)
				indexFirstNewTexture = shapeTree->splitTexture(iTexCoord[*it], weights, VERTICAL);
			else
				indexFirstNewTexture = shapeTree->splitTexture(iTexCoord[*it], weights, HORIZONTAL);

			for (unsigned int i = 0 ; i < nodes.size() ; i++) {
				nodes[i]->selectFace("");
				nodes[i]->selectFace(getFaceString(*it));
				nodes[i]->setTexture(indexFirstNewTexture + i*4);
				nodes[i]->selectFace("");
			}
		}
	}

	// Select faces of children
	for (unsigned int i = 0 ; i < nodes.size() ; i++) {
		nodes[i]->selectAllFaces();
	}

	selectedFaces = save;
}

struct CompX {
	Mesh *shape;
  	bool operator() (vertex_descriptor i,vertex_descriptor j) { return CGAL::compare_x(shape->point(i),shape->point(j)) == -1; }
};

struct CompY {
	Mesh *shape;
  	bool operator() (vertex_descriptor i,vertex_descriptor j) { return CGAL::compare_y(shape->point(i),shape->point(j)) == -1; }
};

struct CompZ {
	Mesh *shape;
  	bool operator() (vertex_descriptor i,vertex_descriptor j) { return CGAL::compare_z(shape->point(i),shape->point(j)) == -1; }
};

void Node::split(Axis axis, vector<Node*>& nodes, vector<string>& actions, string pattern) {
	SP::SP_Driver driver;
	driver.parse(pattern.c_str());

	// Sort vertices according to the axis

	vector<vertex_descriptor> vertices;

	Mesh::Vertex_range::iterator v, v_end;
	for (boost::tie(v,v_end) = shape.vertices(); v != v_end ; v++) {
		vertices.push_back(*v);
	}

	if (!vertices.empty()) {

		switch(axis) {
			case X:
				struct CompX compX; compX.shape = &shape;
				sort(vertices.begin(), vertices.end(), compX);
				driver.computePattern(shape.point(vertices.back()).x()
														- shape.point(vertices.front()).x());
				break;
			case Y:
				struct CompY compY; compY.shape = &shape;
				sort(vertices.begin(), vertices.end(), compY);
				driver.computePattern(shape.point(vertices.back()).y()
														- shape.point(vertices.front()).y());
				break;
			case Z:
				struct CompZ compZ; compZ.shape = &shape;
				sort(vertices.begin(), vertices.end(), compZ);
				driver.computePattern(shape.point(vertices.back()).z()
														- shape.point(vertices.front()).z());
				break;
		}

		// Useful variables

		vector<double> weights = driver.getWeights();
		actions = driver.getActions();

		vector<map<vertex_descriptor, vertex_descriptor> > matchVertexIn; // Matches a vertex of shape to a vertex of a sub shape
		matchVertexIn.resize(weights.size());

		vector<Mesh> nShapes;
		nShapes.resize(weights.size());

		// Distribute Vertices

		switch(axis) {
			case X:
				distributeX(&matchVertexIn, &nShapes, vertices, weights);
				break;
			case Y:
				distributeY(&matchVertexIn, &nShapes, vertices, weights);
				break;
			case Z:
				distributeZ(&matchVertexIn, &nShapes, vertices, weights);
				break;
		}

		// Reconstruct faces

		reconstruct(matchVertexIn, weights, &nShapes);

		// Store results

		Node* subd = new Node(shapeTree, this, true);
		subd->setShape(shape);

		nodes.resize(weights.size());

		for (unsigned int i = 0 ; i < nShapes.size() ; i++) {
			nodes[i] = new Node(shapeTree, subd, true);
			nodes[i]->setShape(nShapes[i]);
			subd->addChild(nodes[i]);
		}

		// Set right textures in subnodes
		preserveTextures(axis, nodes, weights);

		addChild(subd);
	}

	else { // Vertices.emtpy() == true
		nodes.clear();
		actions.clear();
	}
}

string Node::getFaceString(face_descriptor f) {
	CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
	vector<vertex_descriptor> indices;
	for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(f), shape);
			v != v_end; v++) {
		indices.push_back(*v);
	}

	CGAL::Vector_3<Kernel> normal = CGAL::unit_normal(shape.point(indices[0]),
																									 	shape.point(indices[1]),
																									 	shape.point(indices[2]));

	if (normal * CGAL::Vector_3<Kernel>(1,0,0) > 0)
		return "xpos";
	else if (normal * CGAL::Vector_3<Kernel>(-1,0,0) > 0)
		return "xneg";
	else if (normal * CGAL::Vector_3<Kernel>(0,1,0) > 0)
		return "ypos";
	else if (normal * CGAL::Vector_3<Kernel>(0,-1,0) > 0)
		return "yneg";
	else if (normal * CGAL::Vector_3<Kernel>(0,0,1) > 0)
		return "zpos";
	else if (normal * CGAL::Vector_3<Kernel>(0,0,-1) > 0)
		return "zneg";

	return "";
}

void Node::selectFace(const string& face) {
	firstTimeSelect = false;
	bool clear = true;

	Mesh::Face_range::iterator f, f_end;

	for (boost::tie(f,f_end) = shape.faces(); f != f_end ; f++) {
		if (getFaceString(*f) == face) {
			selectedFaces.push_back(*f);
			clear = false;
		}
	}

	if (clear)
		selectedFaces.clear();
}

void Node::setTexture(int indexFirstCoord) {
	for (auto f = selectedFaces.begin() ; f != selectedFaces.end() ; f++) {
		iTexCoord[*f] = indexFirstCoord;
	}
}

Node* Node::removeFaces() {
	Mesh nShape;
	Mesh::Vertex_range::iterator v, v_end;
	for (boost::tie(v,v_end) = shape.vertices(); v != v_end ; v++) {
		nShape.add_vertex(shape.point(*v));
	}

	map<face_descriptor, int> nITexCoord;

	Mesh::Face_range::iterator f, f_end;
	for (boost::tie(f,f_end) = shape.faces(); f != f_end ; f++) {
		// Copy only unselected faces
		bool copyFace = true;
		for (auto it = selectedFaces.begin() ; it != selectedFaces.end() ; it++) {
			if (*it == *f)
				copyFace = false;
		}

		if (copyFace) {
			vector<vertex_descriptor> iPrevShape;
			CGAL::Vertex_around_face_iterator<Mesh> v, v_end;

			for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(*f), shape);
					v != v_end; v++){
				iPrevShape.push_back(*v);
			}
			face_descriptor nf = nShape.add_face(iPrevShape[0], iPrevShape[1],
																					 iPrevShape[2], iPrevShape[3]);
			nITexCoord.insert(pair<face_descriptor, int>(nf, iTexCoord[*f]));
		}
	}

	Node* rm = new Node(shapeTree, this, true);
	rm->setShape(nShape);
	rm->setITexCoord(nITexCoord);
	addChild(rm);
	return rm;
}

Node::~Node() {
	for (auto it = children.begin() ; it != children.end() ; it++) {
		delete *it;
	}
}
