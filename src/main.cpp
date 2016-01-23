/*#include <string>

#include "rule.h"
#include "shape_tree.h"

#include "split_pattern/split_pattern_driver.h"

int main() {

  ACT::ShapeTree shapeTree;

  shapeTree.initFromFile(std::string("plane.off"));

  Rule* Parcel = new Rule("Parcel", "split(\"x\") {~1: BldArea | ~1: GreenSpace | ~1: BldArea}");
  Rule* BldArea = new Rule("BldArea", "extrude(5) split(\"y\") {0.4: Floor}*");

  shapeTree.addRule(Parcel);
  shapeTree.addRule(BldArea);

  RuleNames::getInstance().addRule("Parcel");
  RuleNames::getInstance().addRule("BldArea");

  shapeTree.setInitRule(Parcel);

  while (shapeTree.executeRule() != -1);

  shapeTree.displayGeometry();

  return 0;
}*/

#include <fstream>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

using namespace std;

typedef CGAL::Simple_cartesian<double>     	Kernel;
typedef Kernel::Point_3						Point_3;
typedef Kernel::Vector_3					Vector_3;
typedef CGAL::Surface_mesh<Point_3>        	Mesh;
typedef Mesh::Vertex_index 				vertex_descriptor;
typedef Mesh::Face_index 					face_descriptor;

int main() {
  Mesh mesh;
  ifstream input;
	input.open("plane2.off");
	input >> mesh;
	input.close();

  string out("out2");

  ofstream objStream, mtlStream;
  objStream.open(out + ".obj", ios::trunc);

  // OBJ file

  objStream << "# CGA-_interpreter generated building" << endl;
  objStream << "mtllib " << out << ".mtl" << endl;
  objStream << "o Building" << endl;

  map<vertex_descriptor, int> vInt;
  Mesh::Vertex_range::iterator v, v_end;

  int i = 1;
  for (boost::tie(v,v_end) = mesh.vertices(); v != v_end ; v++) {
    objStream << 'v' << ' ' << mesh.point(*v).x() << ' ' <<
                            mesh.point(*v).y() << ' ' <<
                            mesh.point(*v).z() << std::endl;

    vInt.insert(pair<vertex_descriptor, int>(*v,i));
    i++;
  }

  objStream << "vt " << 0 << ' ' << 0 << endl
            << "vt " << 1 << ' ' << 0 << endl
            << "vt " << 1 << ' ' << 1 << endl
            << "vt " << 0 << ' ' << 1 << endl;

  objStream << "usemtl Texture" << endl;
  objStream << "s off" << endl;

  Mesh::Face_range::iterator f, f_end;
  for (boost::tie(f,f_end) = mesh.faces(); f != f_end ; f++) {
    int i = 1;
    objStream << 'f';
    CGAL::Vertex_around_face_iterator<Mesh> v, v_end;
    boost::tie(v, v_end) = vertices_around_face(mesh.halfedge(*f), mesh);
    do
      {objStream << ' ' << vInt[*v] << '/' << i; i++;}
    while(++v != v_end);
    objStream << std::endl;
  }

  objStream.close();
  mtlStream.open(out + ".mtl", ios::trunc);

  mtlStream << "newmtl Texture" << endl <<
    "map_Kd res/window_small.jpg" << endl;

  mtlStream.close();

  return 0;
}
