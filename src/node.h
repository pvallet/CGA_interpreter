#pragma once

#include <cmath>

#include <map>
#include <list>
#include <string>

#include "utils.h"

using namespace std;

typedef struct MeshResult {
	Mesh mesh;
	Mesh roof;
	map<face_descriptor, int> iTexCoord;
	map<face_descriptor, map<vertex_descriptor, int> > iRoofTexCoord;
} MeshResult;

enum Axis {X,Y,Z};

namespace ACT {
	class ShapeTree;
}

class Node {

public:
	Node(ACT::ShapeTree* _shapeTree, Node* _parent);
	~Node();

	void selectAllFaces();
	void noTexture();
	void load(const string& path);
	void setShape(Mesh _shape);
	inline void setParent(Node* _parent) {parent = _parent;}
	void addChild(Node* _child);

	MeshResult getSubGeometry();
	inline bool isFirstTimeSelect() const {return firstTimeSelect;}
	inline void selected() {firstTimeSelect = false;}

	Node* translate(Kernel::RT dx, Kernel::RT dy, Kernel::RT dz);
	Node* extrude(Kernel::RT height); // Returns the new extruded shape, child of the saved old shape
	void split(Axis axis, vector<Node*>& nodes, vector<string>& actions, string pattern);
	void selectFace(const string& face); // Only (x|y|z)(pos|neg), otherwise unselect everything
	void setTexture(int indexFirstCoord);
	void addToRoof(const vector<vector<Point_3> >& ceiling);
	void createRoof(double _roofAngle = 20*M_PI/180, double _roofOffset = 0.3);
	Node* removeFaces();

	void getCeiling(vector<vector<Point_3> >& result);
	void computeRoof();

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
	bool hasRoof;

	Mesh shape; // The shape is supposed to be a cube or a square
	map<string, double> attributes;
	map<face_descriptor, int> iTexCoord;

	double roofAngle;
	double roofOffset;
	map<Kernel::FT, list<Polygon_with_holes_2> > roofLevels;
	Mesh roof;
	map<face_descriptor, map<vertex_descriptor, int> > iRoofTexCoord;

	list<face_descriptor> selectedFaces;
	bool firstTimeSelect;

protected:
	inline void setITexCoord(const map<face_descriptor, int>& _iTexCoord) {iTexCoord = _iTexCoord;}
	void cancelRoof(); // Can be only called by createRoof()
};
