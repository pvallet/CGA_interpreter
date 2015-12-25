#include "node.h"

#include <cstddef>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <map>

#include "split_pattern_driver.h"

using namespace std;

typedef pair<vertex_descriptor,vertex_descriptor> Match;

Node::Node(Node* _parent, bool _visible) :
	parent(_parent),
	visible(_visible),
	anonymous(false)
{}

void Node::load(string path) {
	ifstream input;
	input.open(path);
	input >> shape;
	input.close();
}

void Node::setVisible(bool _visible) {
	if (_visible) {
		visible = true;
		if (parent != NULL && !anonymous)
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

Mesh Node::getSubGeometry() {
	if (children.empty())
		return shape;

	else {
		Mesh result;

		bool hasVisibleChild = false;
		for (auto it = children.begin() ; it != children.end() ; it++) {
			if ((*it)->isVisible()) {
				hasVisibleChild = true;
				result += (*it)->getSubGeometry();
			}
		}

		if (hasVisibleChild)
			return result;
		else
			return shape;
	}
}

void Node::extrude(Kernel::RT height) {
    Mesh::Face_range::iterator f, f_end;

    for (boost::tie(f,f_end) = shape.faces(); f != f_end ; f++) {

    	vector<vertex_descriptor> indices;

    	CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
	    for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(*f), shape);
	        v != v_end; v++){

	    	indices.push_back(*v);
	    }

	    CGAL::Vector_3<Kernel> normal = CGAL::unit_normal(shape.point(indices[0]),
	    											 shape.point(indices[1]),
	    											 shape.point(indices[2]));

	    normal = normal * height;

	    vector<vertex_descriptor> nIndices;

	    for (unsigned int i = 0 ; i < indices.size() ; i++) {
	    	nIndices.push_back(shape.add_vertex(shape.point(indices[i]) + normal));
	    }

	    face_descriptor f;

	    for (unsigned int i = 0 ; i < indices.size() ; i++) {
	    	f = shape.add_face(	nIndices[i],
		    					nIndices[(i+1) % nIndices.size()],
		    					indices[(i+1) % indices.size()],
		    					indices[i]);

	    	if (f == Mesh::null_face())
				cout << "Extrude: Unable to add face" << endl;
	    }

	    if (nIndices.size() == 3)
	    	f = shape.add_face(nIndices[0], nIndices[1], nIndices[2]);

	    else if (nIndices.size() == 4)
	    	f = shape.add_face(nIndices[0], nIndices[3], nIndices[2], nIndices[1]);

	    else
	    	cout << "Extrude: This face has more than 4 vertices" << endl;
    }
}

void Node::distributeX(
	vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
	vector<vector<vertex_descriptor> >* onBorderBackw,
	vector<vector<vertex_descriptor> >* onBorderForthw,
	vector<Mesh>* nShapes,
	const vector<vertex_descriptor>& sortedVertices,
	const vector<double>& weights) {

	double minCoord = shape.point(sortedVertices.front()).x();
	double cumWeight = 0;
	double prvSeparator, nxtSeparator;
	unsigned int j = 0;

	for (unsigned int i = 0 ; i < nShapes->size() ; i++) {
		prvSeparator = minCoord + cumWeight;
		cumWeight += weights[i];
		nxtSeparator = minCoord + cumWeight;

		for (; j < sortedVertices.size() && shape.point(sortedVertices[j]).x() <= nxtSeparator; j++) {
			vertex_descriptor indexAdded = (*nShapes)[i].add_vertex(shape.point(sortedVertices[j]));
			(*matchVertexIn)[i].insert( Match(sortedVertices[j], indexAdded) );

			CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
			for(boost::tie(h, h_end) = halfedges_around_target(sortedVertices[j], shape);
					h != h_end; h++){

				if (shape.point(shape.source(*h)).x() > nxtSeparator) {

					Vector_3 crossingEdge(shape.point(sortedVertices[j]), shape.point(shape.source(*h)));

					crossingEdge = crossingEdge * Kernel::RT( // Thales
						(nxtSeparator - shape.point(sortedVertices[j]).x()) /
						(shape.point(shape.source(*h)).x() - shape.point(sortedVertices[j]).x()) );

					(*onBorderForthw)[i].push_back((*nShapes)[i].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i].insert( Match(shape.source(*h), (*onBorderForthw)[i].back()) );

					(*onBorderBackw)[i+1].push_back((*nShapes)[i+1].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i+1].insert( Match(shape.source(*h), (*onBorderBackw)[i+1].back()) );
				}

				else if (shape.point(shape.source(*h)).x() < prvSeparator) {

					Vector_3 crossingEdge(shape.point(sortedVertices[j]), shape.point(shape.source(*h)));

					crossingEdge = crossingEdge * Kernel::RT( // Thales
						(prvSeparator - shape.point(sortedVertices[j]).x()) /
						(shape.point(shape.source(*h)).x() - shape.point(sortedVertices[j]).x()) );

					(*onBorderBackw)[i].push_back((*nShapes)[i].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i].insert( Match(shape.source(*h), (*onBorderBackw)[i].back()) );

					(*onBorderForthw)[i-1].push_back((*nShapes)[i-1].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i-1].insert( Match(shape.source(*h), (*onBorderForthw)[i-1].back()) );
				}
			}
		}
	}
}

void Node::distributeY(
	vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
	vector<vector<vertex_descriptor> >* onBorderBackw,
	vector<vector<vertex_descriptor> >* onBorderForthw,
	vector<Mesh>* nShapes,
	const vector<vertex_descriptor>& sortedVertices,
	const vector<double>& weights) {

	double minCoord = shape.point(sortedVertices.front()).y();
	double cumWeight = 0;
	double prvSeparator, nxtSeparator;
	unsigned int j = 0;

	for (unsigned int i = 0 ; i < nShapes->size() ; i++) {
		prvSeparator = minCoord + cumWeight;
		cumWeight += weights[i];
		nxtSeparator = minCoord + cumWeight;

		for (; j < sortedVertices.size() && shape.point(sortedVertices[j]).y() <= nxtSeparator; j++) {
			vertex_descriptor indexAdded = (*nShapes)[i].add_vertex(shape.point(sortedVertices[j]));
			(*matchVertexIn)[i].insert( Match(sortedVertices[j], indexAdded) );

			CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
			for(boost::tie(h, h_end) = halfedges_around_target(sortedVertices[j], shape);
					h != h_end; h++){

				if (shape.point(shape.source(*h)).y() > nxtSeparator) {

					Vector_3 crossingEdge(shape.point(sortedVertices[j]), shape.point(shape.source(*h)));

					crossingEdge = crossingEdge * Kernel::RT( // Thales
						(nxtSeparator - shape.point(sortedVertices[j]).y()) /
						(shape.point(shape.source(*h)).y() - shape.point(sortedVertices[j]).y()) );

					(*onBorderForthw)[i].push_back((*nShapes)[i].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i].insert( Match(shape.source(*h), (*onBorderForthw)[i].back()) );

					(*onBorderBackw)[i+1].push_back((*nShapes)[i+1].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i+1].insert( Match(shape.source(*h), (*onBorderBackw)[i+1].back()) );
				}

				else if (shape.point(shape.source(*h)).y() < prvSeparator) {

					Vector_3 crossingEdge(shape.point(sortedVertices[j]), shape.point(shape.source(*h)));

					crossingEdge = crossingEdge * Kernel::RT( // Thales
						(prvSeparator - shape.point(sortedVertices[j]).y()) /
						(shape.point(shape.source(*h)).y() - shape.point(sortedVertices[j]).y()) );

					(*onBorderBackw)[i].push_back((*nShapes)[i].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i].insert( Match(shape.source(*h), (*onBorderBackw)[i].back()) );

					(*onBorderForthw)[i-1].push_back((*nShapes)[i-1].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i-1].insert( Match(shape.source(*h), (*onBorderForthw)[i-1].back()) );
				}
			}
		}
	}
}

void Node::distributeZ(
	vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
	vector<vector<vertex_descriptor> >* onBorderBackw,
	vector<vector<vertex_descriptor> >* onBorderForthw,
	vector<Mesh>* nShapes,
	const vector<vertex_descriptor>& sortedVertices,
	const vector<double>& weights) {

	double minCoord = shape.point(sortedVertices.front()).z();
	double cumWeight = 0;
	double prvSeparator, nxtSeparator;
	unsigned int j = 0;

	for (unsigned int i = 0 ; i < nShapes->size() ; i++) {
		prvSeparator = minCoord + cumWeight;
		cumWeight += weights[i];
		nxtSeparator = minCoord + cumWeight;

		for (; j < sortedVertices.size() && shape.point(sortedVertices[j]).z() <= nxtSeparator; j++) {
			vertex_descriptor indexAdded = (*nShapes)[i].add_vertex(shape.point(sortedVertices[j]));
			(*matchVertexIn)[i].insert( Match(sortedVertices[j], indexAdded) );

			CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
			for(boost::tie(h, h_end) = halfedges_around_target(sortedVertices[j], shape);
					h != h_end; h++){

				if (shape.point(shape.source(*h)).z() > nxtSeparator) {

					Vector_3 crossingEdge(shape.point(sortedVertices[j]), shape.point(shape.source(*h)));

					crossingEdge = crossingEdge * Kernel::RT( // Thales
						(nxtSeparator - shape.point(sortedVertices[j]).z()) /
						(shape.point(shape.source(*h)).z() - shape.point(sortedVertices[j]).z()) );

					(*onBorderForthw)[i].push_back((*nShapes)[i].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i].insert( Match(shape.source(*h), (*onBorderForthw)[i].back()) );

					(*onBorderBackw)[i+1].push_back((*nShapes)[i+1].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i+1].insert( Match(shape.source(*h), (*onBorderBackw)[i+1].back()) );
				}

				else if (shape.point(shape.source(*h)).z() < prvSeparator) {

					Vector_3 crossingEdge(shape.point(sortedVertices[j]), shape.point(shape.source(*h)));

					crossingEdge = crossingEdge * Kernel::RT( // Thales
						(prvSeparator - shape.point(sortedVertices[j]).z()) /
						(shape.point(shape.source(*h)).z() - shape.point(sortedVertices[j]).z()) );

					(*onBorderBackw)[i].push_back((*nShapes)[i].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i].insert( Match(shape.source(*h), (*onBorderBackw)[i].back()) );

					(*onBorderForthw)[i-1].push_back((*nShapes)[i-1].add_vertex(shape.point(sortedVertices[j]) + crossingEdge));
					(*matchVertexIn)[i-1].insert( Match(shape.source(*h), (*onBorderForthw)[i-1].back()) );
				}
			}
		}
	}
}

void Node::reconstruct(
	const vector<map<vertex_descriptor, vertex_descriptor> >& matchVertexIn,
	const vector<vector<vertex_descriptor> >& onBorderBackw,
	const vector<vector<vertex_descriptor> >& onBorderForthw,
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
		    	if (aroundNewFace.size() == 3)
			    	(*nShapes)[i].add_face(aroundNewFace[0], aroundNewFace[1], aroundNewFace[2]);

			    else if (aroundNewFace.size() == 4)
			    	(*nShapes)[i].add_face(aroundNewFace[0], aroundNewFace[3], aroundNewFace[2], aroundNewFace[1]);

			    else
	    			cout << "Extrude: This face has more than 4 vertices" << endl;
		    }
	    }
	}

    // Faces on border

	for (unsigned int i = 0 ; i < onBorderForthw.size() ; i++) {
		if (onBorderForthw.size() == 4) {
			(*nShapes)[i].add_face(onBorderForthw[i][0], onBorderForthw[i][3], onBorderForthw[i][2], onBorderForthw[i][1]);
		}

		if (onBorderBackw.size() == 4) {
			(*nShapes)[i].add_face(onBorderBackw[i][0], onBorderBackw[i][3], onBorderBackw[i][2], onBorderBackw[i][1]);
		}

		// TODO : Else, do a delaunay triangulation to create the faces on a separator
	}
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

	vector<double> weights = driver.getWeights();
	actions = driver.getActions();

	// Useful variables

	vector<vector<vertex_descriptor> > onBorderForthw, onBorderBackw;
	onBorderForthw.resize(weights.size());
	onBorderBackw.resize(weights.size());

	vector<map<vertex_descriptor, vertex_descriptor> > matchVertexIn; // Matches a vertex of shape to a vertex of a sub shape
	matchVertexIn.resize(weights.size());

	vector<Mesh> nShapes;
	nShapes.resize(weights.size());

	// Distribute Vertices

	switch(axis) {
		case X:
			distributeX(&matchVertexIn, &onBorderBackw, &onBorderForthw, &nShapes, vertices, weights);
			break;
		case Y:
			distributeY(&matchVertexIn, &onBorderBackw, &onBorderForthw, &nShapes, vertices, weights);
			break;
		case Z:
			distributeZ(&matchVertexIn, &onBorderBackw, &onBorderForthw, &nShapes, vertices, weights);
			break;
	}

	// Reconstruct faces

	reconstruct(matchVertexIn, onBorderBackw, onBorderForthw, &nShapes);

	// Store results

	Node* subd = new Node(this, true);
	subd->setShape(shape);

	nodes.resize(weights.size());

	for (unsigned int i = 0 ; i < nShapes.size() ; i++) {
		nodes[i] = new Node(subd, true);
		nodes[i]->setShape(nShapes[i]);
		subd->addChild(nodes[i]);
	}

	addChild(subd);
}

Node::~Node() {
	for (auto it = children.begin() ; it != children.end() ; it++) {
		delete *it;
	}
}
