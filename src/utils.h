#include <boost/shared_ptr.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/create_straight_skeleton_from_polygon_with_holes_2.h>

typedef CGAL::Simple_cartesian<double>     	Kernel;
typedef Kernel::Point_3											Point_3;
typedef Kernel::Point_2											Point_2;
typedef Kernel::Vector_3										Vector_3;
typedef Kernel::Vector_2										Vector_2;

typedef CGAL::Surface_mesh<Point_3>        	Mesh;
typedef Mesh::Vertex_index 									vertex_descriptor;
typedef Mesh::Halfedge_index 								halfedge_descriptor;
typedef Mesh::Face_index 										face_descriptor;

typedef CGAL::Polygon_2<Kernel>            Polygon_2 ;
typedef boost::shared_ptr<Polygon_2>       Polygon_2Ptr ;
typedef CGAL::Polygon_with_holes_2<Kernel> Polygon_with_holes_2 ;
typedef CGAL::Straight_skeleton_2<Kernel>  Ss ;
typedef boost::shared_ptr<Ss> 						 SsPtr ;
typedef boost::shared_ptr<Polygon_with_holes_2> PwhPtr;


namespace CstmCGAL { // Custom CGAL

  PwhPtr applyOffset(double offset, const Polygon_with_holes_2& poly);

  // Sometimes polygons can be the union of 2 polygons that have one vertex in common
  // I should have used nef_polyhedron_2 to avoid this problem, but I only took
  // the time to implement a simple fix. It does not handle holes.

  std::list<Polygon_with_holes_2> splitPoly(const Polygon_with_holes_2& poly);

  /*  These were used to reduce polygons to triangles and square, which were the
    only ones supported by the original OFF viewer. However, the function did not
    always work when the face was concave, so I decided to change the viewer instead
    of investing time in fixing it.
  */

  void splitFace(Mesh& mesh, face_descriptor f);
  void splitFaces(Mesh& mesh);

} // End of namespace CstmCGAL
