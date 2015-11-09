#include "shape_tree.h"

#include <cerrno>
#include <cfloat>
#include <cstddef>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <map>

Node::Node(Node* _parent, bool _visible) :
	parent(_parent),
	visible(_visible),
	anonymous(false)
{

}

void Node::load(std::string path) {
	std::ifstream input;
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

    	std::vector<vertex_descriptor> indices;
    	
    	CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
	    for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(*f), shape);
	        v != v_end; v++){

	    	indices.push_back(*v);
	    }

	    CGAL::Vector_3<Kernel> normal = CGAL::unit_normal(shape.point(indices[0]),
	    											 shape.point(indices[1]),
	    											 shape.point(indices[2]));

	    normal = normal * height;

	    std::vector<vertex_descriptor> nIndices;

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
				std::cout << "Extrude: Unable to add face" << std::endl;
	    }

	    if (nIndices.size() == 3)
	    	f = shape.add_face(nIndices[0], nIndices[1], nIndices[2]);

	    else if (nIndices.size() == 4)
	    	f = shape.add_face(nIndices[0], nIndices[3], nIndices[2], nIndices[1]);

	    else
	    	std::cout << "Extrude: This face has more than 4 vertices" << std::endl;
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

typedef std::pair<vertex_descriptor,vertex_descriptor> Match;

void Node::split(Axis axis, std::vector<Node*>& actions, std::vector<double> weights) {

	// Sort vertices according to the axis

	std::vector<vertex_descriptor> vertices;

	Mesh::Vertex_range::iterator v, v_end;
	for (boost::tie(v,v_end) = shape.vertices(); v != v_end ; v++) {
		vertices.push_back(*v);
	}

	double minCoord, maxCoord;

	switch(axis) {
		case X:
			struct CompX compX; compX.shape = &shape;
			std::sort(vertices.begin(), vertices.end(), compX);
			minCoord = shape.point(vertices.front()).x();
			maxCoord = shape.point(vertices.back()).x();
			break;
		case Y:
			struct CompY compY; compY.shape = &shape;
			std::sort(vertices.begin(), vertices.end(), compY);
			minCoord = shape.point(vertices.front()).y();
			maxCoord = shape.point(vertices.back()).y();
			break;
		case Z:
			struct CompZ compZ; compZ.shape = &shape;
			std::sort(vertices.begin(), vertices.end(), compZ);
			minCoord = shape.point(vertices.front()).z();
			maxCoord = shape.point(vertices.back()).z();
			break;
	}

	// Useful variables

	std::vector<std::vector<vertex_descriptor> > onBorderForthw, onBorderBackw;
	onBorderForthw.resize(weights.size());
	onBorderBackw.resize(weights.size());
	std::vector<std::map<vertex_descriptor, vertex_descriptor> > matchVertexIn; // Matches a vertex of shape to a vertex of a sub shape
	matchVertexIn.resize(weights.size());
	
	std::vector<Mesh> nShapes;
	nShapes.resize(weights.size());

	double totalWeight = 0.;
	double cumWeight = 0;

	for (auto it = weights.begin() ; it != weights.end() ; it++)
		totalWeight += *it;

	double prvSeparator, nxtSeparator;
	unsigned int j = 0;

	// Distribute Vertices

	for (unsigned int i = 0 ; i < nShapes.size() ; i++) {
		prvSeparator = minCoord + cumWeight * (maxCoord - minCoord) / totalWeight;
		cumWeight += weights[i];
		nxtSeparator = minCoord + cumWeight * (maxCoord - minCoord) / totalWeight;

		switch(axis) {
			case X:
				for (; j < vertices.size() && shape.point(vertices[j]).x() <= nxtSeparator; j++) {
					vertex_descriptor indexAdded = nShapes[i].add_vertex(shape.point(vertices[j]));
					matchVertexIn[i].insert( Match(vertices[j], indexAdded) );

					CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
				    for(boost::tie(h, h_end) = halfedges_around_target(vertices[j], shape);
				        h != h_end; h++){

				    	if (shape.point(shape.source(*h)).x() > nxtSeparator) {

				    		Vector_3 crossingEdge(shape.point(vertices[j]), shape.point(shape.source(*h)));

				    		crossingEdge = crossingEdge * Kernel::RT( // Thales
				    			(nxtSeparator - shape.point(vertices[j]).x()) /
				    			(shape.point(shape.source(*h)).x() - shape.point(vertices[j]).x()) );

				    		onBorderForthw[i].push_back(nShapes[i].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i].insert( Match(shape.source(*h), onBorderForthw[i].back()) );
				    		
				    		onBorderBackw[i+1].push_back(nShapes[i+1].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i+1].insert( Match(shape.source(*h), onBorderBackw[i+1].back()) );
				    	}

				    	else if (shape.point(shape.source(*h)).x() < prvSeparator) {

				    		Vector_3 crossingEdge(shape.point(vertices[j]), shape.point(shape.source(*h)));

				    		crossingEdge = crossingEdge * Kernel::RT( // Thales
				    			(prvSeparator - shape.point(vertices[j]).x()) /
				    			(shape.point(shape.source(*h)).x() - shape.point(vertices[j]).x()) );

				    		onBorderBackw[i].push_back(nShapes[i].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i].insert( Match(shape.source(*h), onBorderBackw[i].back()) );
				    		
				    		onBorderForthw[i-1].push_back(nShapes[i-1].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i-1].insert( Match(shape.source(*h), onBorderForthw[i-1].back()) );
				    	}
				    }
				}
				break;

			case Y:
				for (; j < vertices.size() && shape.point(vertices[j]).y() <= nxtSeparator; j++) {
					vertex_descriptor indexAdded = nShapes[i].add_vertex(shape.point(vertices[j]));
					matchVertexIn[i].insert( Match(vertices[j], indexAdded) );

					CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
				    for(boost::tie(h, h_end) = halfedges_around_target(vertices[j], shape);
				        h != h_end; h++){

				    	if (shape.point(shape.source(*h)).y() > nxtSeparator) {

				    		Vector_3 crossingEdge(shape.point(vertices[j]), shape.point(shape.source(*h)));

				    		crossingEdge = crossingEdge * Kernel::RT( // Thales
				    			(nxtSeparator - shape.point(vertices[j]).y()) /
				    			(shape.point(shape.source(*h)).y() - shape.point(vertices[j]).y()) );

				    		onBorderForthw[i].push_back(nShapes[i].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i].insert( Match(shape.source(*h), onBorderForthw[i].back()) );
				    		
				    		onBorderBackw[i+1].push_back(nShapes[i+1].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i+1].insert( Match(shape.source(*h), onBorderBackw[i+1].back()) );
				    	}

				    	else if (shape.point(shape.source(*h)).y() < prvSeparator) {

				    		Vector_3 crossingEdge(shape.point(vertices[j]), shape.point(shape.source(*h)));

				    		crossingEdge = crossingEdge * Kernel::RT( // Thales
				    			(prvSeparator - shape.point(vertices[j]).y()) /
				    			(shape.point(shape.source(*h)).y() - shape.point(vertices[j]).y()) );

				    		onBorderBackw[i].push_back(nShapes[i].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i].insert( Match(shape.source(*h), onBorderBackw[i].back()) );
				    		
				    		onBorderForthw[i-1].push_back(nShapes[i-1].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i-1].insert( Match(shape.source(*h), onBorderForthw[i-1].back()) );
				    	}
				    }
				}
				break;

			case Z:
				for (; j < vertices.size() && shape.point(vertices[j]).z() <= nxtSeparator; j++) {
					vertex_descriptor indexAdded = nShapes[i].add_vertex(shape.point(vertices[j]));
					matchVertexIn[i].insert( Match(vertices[j], indexAdded) );

					CGAL::Halfedge_around_target_iterator<Mesh> h, h_end;
				    for(boost::tie(h, h_end) = halfedges_around_target(vertices[j], shape);
				        h != h_end; h++){

				    	if (shape.point(shape.source(*h)).z() > nxtSeparator) {

				    		Vector_3 crossingEdge(shape.point(vertices[j]), shape.point(shape.source(*h)));

				    		crossingEdge = crossingEdge * Kernel::RT( // Thales
				    			(nxtSeparator - shape.point(vertices[j]).z()) /
				    			(shape.point(shape.source(*h)).z() - shape.point(vertices[j]).z()) );

				    		onBorderForthw[i].push_back(nShapes[i].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i].insert( Match(shape.source(*h), onBorderForthw[i].back()) );
				    		
				    		onBorderBackw[i+1].push_back(nShapes[i+1].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i+1].insert( Match(shape.source(*h), onBorderBackw[i+1].back()) );
				    	}

				    	else if (shape.point(shape.source(*h)).z() < prvSeparator) {

				    		Vector_3 crossingEdge(shape.point(vertices[j]), shape.point(shape.source(*h)));

				    		crossingEdge = crossingEdge * Kernel::RT( // Thales
				    			(prvSeparator - shape.point(vertices[j]).z()) /
				    			(shape.point(shape.source(*h)).z() - shape.point(vertices[j]).z()) );

				    		onBorderBackw[i].push_back(nShapes[i].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i].insert( Match(shape.source(*h), onBorderBackw[i].back()) );
				    		
				    		onBorderForthw[i-1].push_back(nShapes[i-1].add_vertex(shape.point(vertices[j]) + crossingEdge));
				    		matchVertexIn[i-1].insert( Match(shape.source(*h), onBorderForthw[i-1].back()) );
				    	}
				    }
				}
				break;
		}

	}

	// Reconstruct faces

	for (unsigned int i = 0 ; i < weights.size() ; i++) {
		Mesh::Face_range::iterator f, f_end;
	    for (boost::tie(f,f_end) = shape.faces(); f != f_end ; f++) {
	    	bool addFace = true;

	    	std::vector<vertex_descriptor> aroundNewFace;

	    	CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
		    for(boost::tie(v, v_end) = vertices_around_face(shape.halfedge(*f), shape);
		        v != v_end; v++) {

		    	if (matchVertexIn[i].find(*v) == matchVertexIn[i].end()) {
		    		addFace = false;
		    		break;
		    	}

		    	else {
		    		aroundNewFace.push_back(matchVertexIn[i][*v]);
		    	}
		    }

		    if (addFace) {
		    	if (aroundNewFace.size() == 3)
			    	nShapes[i].add_face(aroundNewFace[0], aroundNewFace[1], aroundNewFace[2]);

			    else if (aroundNewFace.size() == 4)
			    	nShapes[i].add_face(aroundNewFace[0], aroundNewFace[3], aroundNewFace[2], aroundNewFace[1]);
			    
			    else
	    			std::cout << "Extrude: This face has more than 4 vertices" << std::endl;
		    }
	    }
	}

    // Faces on border

	for (unsigned int i = 0 ; i < onBorderForthw.size() ; i++) {
		if (onBorderForthw.size() == 4) {
			nShapes[i].add_face(onBorderForthw[i][0], onBorderForthw[i][3], onBorderForthw[i][2], onBorderForthw[i][1]);
		}

		// TODO : Else, do a delaunay triangulation to create the faces on a separator
	}

	for (unsigned int i = 0 ; i < onBorderBackw.size() ; i++) {
		if (onBorderBackw.size() == 4) {
			nShapes[i].add_face(onBorderBackw[i][0], onBorderBackw[i][3], onBorderBackw[i][2], onBorderBackw[i][1]);
		}

		// TODO : Else, do a delaunay triangulation to create the faces on a separator
	}

	// Store results

	Node* subd = new Node(this, true);
	subd->setShape(shape);

	actions.resize(weights.size());

	for (unsigned int i = 0 ; i < nShapes.size() ; i++) {
		actions[i] = new Node(subd, true);
		actions[i]->setShape(nShapes[i]);
		subd->addChild(actions[i]);
	}

	addChild(subd);
}

Node::~Node() {
	for (auto it = children.begin() ; it != children.end() ; it++) {
		delete *it;
	}
}



ShapeTree::ShapeTree() :
	root(NULL, true)
{
}

void ShapeTree::outputGeometry() {
    std::ofstream output;
	output.open("out.off", std::ios::trunc);
	output << root.getSubGeometry();
	output.close();
}

void ShapeTree::displayGeometry() {
	outputGeometry();
	std::cout << std::endl;
	if (execl("./viewer", "./viewer") == -1)
		std::cout << strerror(errno) << std::endl;
}
