#pragma once

#include <map>
#include <list>
#include <string>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

using namespace std;

typedef CGAL::Simple_cartesian<double>     	Kernel;
typedef Kernel::Point_3						Point_3;
typedef Kernel::Point_2						Point_2;
typedef Kernel::Vector_3					Vector_3;
typedef CGAL::Surface_mesh<Point_3>        	Mesh;
typedef Mesh::Vertex_index 				vertex_descriptor;
typedef Mesh::Halfedge_index 			halfedge_descriptor;
typedef Mesh::Face_index 					face_descriptor;

typedef struct MeshResult {
	Mesh mesh;
	map<face_descriptor, int> iTexCoord;
} MeshResult;

enum Axis {X,Y,Z};

class Node {

public:
	Node(Node* _parent, bool _visible);
	~Node();

	void selectAllFaces();
	void noTexture();
	void load(string path);
	void setShape(Mesh _shape) {shape = _shape; selectAllFaces(); noTexture();}
	void setVisible(bool _visible);
	void setParent(Node* _parent) {parent = _parent;}
	void addChild(Node* _child);

	bool isVisible() {return visible;}
	MeshResult getSubGeometry();
	bool isFirstTimeSelect() {return firstTimeSelect;}

	Node* extrude(Kernel::RT height); // Returns the new extruded shape, child of the saved old shape
	void split(Axis axis, vector<Node*>& nodes, vector<string>& actions, string pattern);
	void selectFace(string face); // Only (x|y|z)(pos|neg), otherwise unselect everything
	void setTexture(int indexFirstCoord);

private:

	// Split
	void reconstruct(
		const vector<map<vertex_descriptor, vertex_descriptor> >& matchVertexIn,
		const vector<vector<vertex_descriptor> >& onBorderBackw,
		const vector<vector<vertex_descriptor> >& onBorderForthw,
		vector<Mesh>* nShapes);

	void distributeX(
		vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
		vector<vector<vertex_descriptor> >* onBorderBackw,
		vector<vector<vertex_descriptor> >* onBorderForthw,
		vector<Mesh>* nShapes,
		const vector<vertex_descriptor>& vertices,
		const vector<double>& weights);
	void distributeY(
		vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
		vector<vector<vertex_descriptor> >* onBorderBackw,
		vector<vector<vertex_descriptor> >* onBorderForthw,
		vector<Mesh>* nShapes,
		const vector<vertex_descriptor>& vertices,
		const vector<double>& weights);
	void distributeZ(
		vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
		vector<vector<vertex_descriptor> >* onBorderBackw,
		vector<vector<vertex_descriptor> >* onBorderForthw,
		vector<Mesh>* nShapes,
		const vector<vertex_descriptor>& sortedVertices,
		const vector<double>& weights);

	Node* parent;
	list<Node*> children;
	bool visible;

	Mesh shape; // The shape is supposed to be a cube or a square
	map<string, double> attributes;
	map<face_descriptor, int> iTexCoord;

	list<face_descriptor> selectedFaces;
	bool firstTimeSelect;
};
