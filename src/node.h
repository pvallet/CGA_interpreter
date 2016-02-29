#pragma once

#include <map>
#include <list>
#include <string>

#include "utils.h"

using namespace std;

typedef struct MeshResult {
	Mesh mesh;
	map<face_descriptor, int> iTexCoord;
} MeshResult;

enum Axis {X,Y,Z};

namespace ACT {
	class ShapeTree;
}

class Node {

public:
	Node(ACT::ShapeTree* _shapeTree, Node* _parent, bool _visible);
	~Node();

	void selectAllFaces();
	void noTexture();
	void load(const string& path);
	void setShape(Mesh _shape);
	void setVisible(bool _visible);
	inline void setParent(Node* _parent) {parent = _parent;}
	void addChild(Node* _child);

	inline bool isVisible() {return visible;}
	MeshResult getSubGeometry();
	inline bool isFirstTimeSelect() {return firstTimeSelect;}
	void getCeiling(vector<vector<Point_3> >& result);

	Node* translate(Kernel::RT dx, Kernel::RT dy, Kernel::RT dz);
	Node* extrude(Kernel::RT height); // Returns the new extruded shape, child of the saved old shape
	void split(Axis axis, vector<Node*>& nodes, vector<string>& actions, string pattern);
	void selectFace(const string& face); // Only (x|y|z)(pos|neg), otherwise unselect everything
	void setTexture(int indexFirstCoord);
	Node* removeFaces();

private:

	// Split
	void reconstruct(
		const vector<map<vertex_descriptor, vertex_descriptor> >& matchVertexIn,
		const vector<double>& weights,
		vector<Mesh>* nShapes);
	void preserveTextures(Axis axis, const vector<Node*>& nodes, const vector<double>& weights);

	void distributeX(
		vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
		vector<Mesh>* nShapes,
		const vector<vertex_descriptor>& vertices,
		const vector<double>& weights);
	void distributeY(
		vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
		vector<Mesh>* nShapes,
		const vector<vertex_descriptor>& vertices,
		const vector<double>& weights);
	void distributeZ(
		vector<map<vertex_descriptor, vertex_descriptor> >* matchVertexIn,
		vector<Mesh>* nShapes,
		const vector<vertex_descriptor>& sortedVertices,
		const vector<double>& weights);

	string getFaceString(face_descriptor f);

	ACT::ShapeTree* shapeTree;
	Node* parent;
	list<Node*> children;
	bool visible;

	Mesh shape; // The shape is supposed to be a cube or a square
	map<string, double> attributes;
	map<face_descriptor, int> iTexCoord;

	list<face_descriptor> selectedFaces;
	bool firstTimeSelect;

protected:
	inline void setITexCoord(const map<face_descriptor, int>& _iTexCoord) {iTexCoord = _iTexCoord;}
};
