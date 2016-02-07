#pragma once

#include <string>

#include "node.h"
#include "rule.h"
#include "actions/actions_parser.h"
#include "actions/actions_scanner.h"

using namespace std;

enum Orientation {VERTICAL, HORIZONTAL};

// To identify the location of subtextures
typedef struct TexSplitKey {
	int							texID;
	vector<double> 	weights;
	Orientation 		orientation;
} TexSplitKey;

// Lexicographic order
struct tskComp {
	bool operator() (const TexSplitKey& t1, const TexSplitKey& t2) const {
		if 			(t1.texID != t2.texID)
			return t1.texID < t2.texID;
		else if (t1.orientation != t2.orientation)
			return t1.orientation < t2.orientation;
		else if (t1.weights.size() != t2.weights.size())
			return t1.weights.size() < t2.weights.size();
		else {
			for (unsigned int i = 0 ; i < t1.weights.size() ; i++) {
				if 			(t1.weights[i] != t2.weights[i])
				 	return t1.weights[i] < t2.weights[i];
			}
			return false;
		}
	}
};

namespace ACT { // ShapeTree is the driver for actions

class ShapeTree {

public:
	ShapeTree();
	~ShapeTree() {}

	inline Node* getRoot() {return &root;}

	inline void initFromFile(string path) {root.load(path);}
	inline void setTextureFile(string path) {texturePath = path;}
	// Name a sub rectangle of the texture file that will constitute a texture to be applied
	void addTextureRect(string name, double x0, double y0, double x1, double y1);

	void outputGeometryOFF();
	void displayGeometryOFF();
	void outputGeometryOBJ();
	void displayGeometryOBJ();
	inline void addRule(Rule* rule) {rules.push_back(rule);}
	void setInitRule(Rule* rule);

	int executeRule(); // Returns -1 if there is no more rule to be executed

	// Apply actions to the affected node
	void addToRule(string rule, string actions = string());
	inline void extrude(double value) {affectedNode = affectedNode->extrude(value);}
	void split(char axis, string pattern);
	void selectFaces(string expression); // For the moment only [(x|y|z)(pos|neg)] | all
	void setTexture(string texture);
	void removeFaces();

	// weights.size() new textures are created, it returns the index to the 1st
	int splitTexture(int texID, const vector<double>& weights, Orientation orientation);

private:
	// Parse function
	void executeActions(const string& actions);
	void addTextureCoord(double x0, double y0, double x1, double y1);

	Node root;
	Node* affectedNode; // To execute actions on
	std::list<Rule*> rules;
	Rule* initRule;

	string texturePath;
	vector<Point_2> texCoord;
	map<string, int> textures; // The int specifies the first index of the texture coords

	map<TexSplitKey, int, tskComp> subTextureLocation;
};

} /* End namespace ACT */
