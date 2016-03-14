#include "custom_join.h"
#include "node.h"

#include <cstddef>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <map>

#include <boost/make_shared.hpp>
#include "shape_tree.h"
#include "split_pattern/split_pattern_driver.h"

using namespace std;

typedef pair<vertex_descriptor,vertex_descriptor> Match;

Node::Node(ACT::ShapeTree* _shapeTree, Node* _parent) :
	shapeTree(_shapeTree),
	parent(_parent),
	hasRoof(false),
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

void Node::addChild(Node* _child) {
	_child->setParent(this);
	children.push_back(_child);
}

MeshResult Node::getSubGeometry() {
	MeshResult res;

	if (hasRoof) {
		res.roof = roof;
		res.iRoofTexCoord = iRoofTexCoord;
	}

	if (children.empty()) {
		res.mesh = shape;
		res.iTexCoord = iTexCoord;

		return res;
	}

	else {
		for (auto chld = children.begin() ; chld != children.end() ; chld++) {
			MeshResult partialRes = (*chld)->getSubGeometry();

			// Maps vertices to the appropriate texture coordinates
			for (auto fd = partialRes.iTexCoord.begin() ;
								fd != partialRes.iTexCoord.end() ; fd++) {
				res.iTexCoord.insert(pair<face_descriptor, int>(
					// This addition is specified in the CGAL doc
					(face_descriptor) (fd->first +
														 res.mesh.number_of_faces() +
														 res.mesh.number_of_removed_faces()),
														 fd->second));
			}

			res.mesh += partialRes.mesh;

			// Do the same for the roof
			for (auto fd = partialRes.iRoofTexCoord.begin() ;
									fd != partialRes.iRoofTexCoord.end() ; fd++) {
				map<vertex_descriptor, int> tmpMap;
				for (auto vd = fd->second.begin() ; vd != fd->second.end() ; vd++) {
					tmpMap.insert(pair<vertex_descriptor, int>(
						(vertex_descriptor)  (vd->first +
																	res.roof.number_of_vertices() +
																	res.roof.number_of_removed_vertices()),
																	vd->second));
				}

				res.iRoofTexCoord.insert(pair<face_descriptor, map<vertex_descriptor, int> >(
					(face_descriptor) (fd->first +
														 res.roof.number_of_faces() +
														 res.roof.number_of_removed_faces()),
														 tmpMap));
			}

			res.roof += partialRes.roof;
		}

		return res;
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

	Node* tr = new Node(shapeTree, this);
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

	Node* save = new Node(shapeTree, this);
	Node* extr = new Node(shapeTree, this);
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

			selectFace("ypos"); selectFace("yneg"); selectFace("zpos"); selectFace("zneg");
			break;
		case Y:
			selectFace("yneg"); nodes.front()->selectFace("yneg");
			nodes.front()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.front()->selectFace("");

			selectFace("ypos"); nodes.back()->selectFace("ypos");
			nodes.back()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.back()->selectFace("");

			selectFace("xpos"); selectFace("xneg"); selectFace("zpos"); selectFace("zneg");
			break;
		case Z:
			selectFace("zneg"); nodes.front()->selectFace("zneg");
			nodes.front()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.front()->selectFace("");

			selectFace("zpos"); nodes.back()->selectFace("zpos");
			nodes.back()->setTexture(iTexCoord[selectedFaces.front()]);
			selectFace(""); nodes.back()->selectFace("");

			selectFace("xpos"); selectFace("xneg"); selectFace("ypos"); selectFace("yneg");
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

		Node* subd = new Node(shapeTree, this);
		subd->setShape(shape);

		nodes.resize(weights.size());

		for (unsigned int i = 0 ; i < nShapes.size() ; i++) {
			nodes[i] = new Node(shapeTree, subd);
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

	Node* rm = new Node(shapeTree, this);
	rm->setShape(nShape);
	rm->setITexCoord(nITexCoord);
	addChild(rm);
	return rm;
}

void Node::createRoof(double _roofAngle, double _roofOffset) {
	roofAngle 	= _roofAngle;
	roofOffset 	= _roofOffset;

	hasRoof = true;

	if (parent != NULL)
		parent->cancelRoof();
}

void Node::cancelRoof() {
	hasRoof = false;
	if (parent != NULL)
		parent->cancelRoof();
}

void Node::addToRoof(const vector<vector<Point_3> >& ceiling) {
	if (hasRoof) {
		for (unsigned int i = 0 ; i < ceiling.size() ; i++) {
			Kernel::FT level = ceiling[i].front().y();

			Polygon_2 newPieceOfRoof;

			for (int j = ceiling[i].size()-1 ; j >= 0 ; j--) {
				newPieceOfRoof.push_back(Point_2(ceiling[i][j].x(), ceiling[i][j].z()));
			}

			roofLevels[level].push_back(Polygon_with_holes_2(newPieceOfRoof));

			// std::cout << "size " << roofLevels[level].size() << std::endl;
			// std::cout << "vertices " << roofLevels[level].front().outer_boundary().size() << std::endl;

			std::list<Polygon_with_holes_2> res, treated;
			CstmCGAL::join (roofLevels[level], res);

			for (auto it = res.begin() ; it != res.end() ; it++) {
				std::list<Polygon_with_holes_2> splitted = CstmCGAL::splitPoly(*it);
				if (splitted.size() > 1)
					treated.splice(treated.end(),splitted);
				else
					treated.push_back(*it);
			}

			roofLevels[level] = treated;

			auto it = roofLevels.find(level);
			auto prev = it;

			if (it != roofLevels.begin()) {
				do {
					it--;
					list<Polygon_with_holes_2> res, treated;
					CstmCGAL::join (it->second, prev->second,	res);

					for (auto it2 = res.begin() ; it2 != res.end() ; it2++) {
						std::list<Polygon_with_holes_2> splitted = CstmCGAL::splitPoly(*it2);
						if (splitted.size() > 1)
							treated.splice(treated.end(),splitted);
						else
							treated.push_back(*it2);
					}

					it->second = treated;
					prev--;
				} while (it != roofLevels.begin());
			}
		}
	}

	else {
		if (parent == NULL)
			std::cerr << "Error in addToRoof(): No parent with roof" << std::endl;
		else
			parent->addToRoof(ceiling);
	}
}

void Node::computeRoof() {
	if (hasRoof) {
		// We run through the loop in reverse to be able to remove useless geometry in a future version
		for (auto lvl = roofLevels.rbegin() ; lvl != roofLevels.rend() ; lvl++) {
			for (auto it = lvl->second.begin() ; it != lvl->second.end() ; it++) {

				PwhPtr itWithOffset;

				if (roofOffset > 0)
					itWithOffset = CstmCGAL::applyOffset(roofOffset, *it);
				else
					itWithOffset = boost::make_shared<Polygon_with_holes_2>(*it);

				SsPtr iss = CGAL::create_interior_straight_skeleton_2(*itWithOffset);
				map<Ss::Vertex_const_handle, vertex_descriptor> vertices;

			  for ( Ss::Vertex_const_iterator v = iss->vertices_begin(); v != iss->vertices_end(); v++ ) {
			    vertices.insert(pair<Ss::Vertex_const_handle, vertex_descriptor>(
			      v, roof.add_vertex(Point_3(	v->point().x(),
																				lvl->first + tan(roofAngle)*(v->time()-roofOffset),
																				v->point().y())) ));

						if (lvl->first + tan(roofAngle)*(v->time()-roofOffset) > 100 ) {
						  std::cout << "Failed to compute straight skeleton on polygon:" << std::endl;

							std::vector<Point_2> outerBoundary = std::vector<Point_2>(
				        itWithOffset->outer_boundary().vertices_begin(), itWithOffset->outer_boundary().vertices_end());

							for (unsigned int i = 0 ; i < outerBoundary.size() ; i++) {
								std::cout << outerBoundary[i] << std::endl;
							}
						}
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
							iRoofTexCoord[newf][vertices[h->vertex()]] = shapeTree->insertRoofITex(Point_2(
								OM * unit_contour / shapeTree->getRoofZoom(),
								h->vertex()->time() / cos(roofAngle) / shapeTree->getRoofZoom()));

							h = h->prev();
						} while (h != hbegin);
					}
			  }
			}
		}
	}

	for (auto it = children.begin() ; it != children.end() ; it++)
		(*it)->computeRoof();
}

Node::~Node() {
	for (auto it = children.begin() ; it != children.end() ; it++) {
		delete *it;
	}
}
