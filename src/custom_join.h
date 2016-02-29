/*
 * This is a wrapper for the CGAL join function on polygons
 * This file is needed because boolean set operations conflicts with Surface_mesh
 */

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Boolean_set_operations_2.h>

#include <list>

typedef CGAL::Simple_cartesian<double>     	Kernel;
typedef CGAL::Polygon_with_holes_2<Kernel>  Polygon_with_holes_2 ;

namespace CstmCGAL { // Custom CGAL

void join(const std::list<Polygon_with_holes_2>& P,
                std::list<Polygon_with_holes_2>& R);

void join(const std::list<Polygon_with_holes_2>& P,
          const std::list<Polygon_with_holes_2>& Q,
                std::list<Polygon_with_holes_2>& R);

} // End of namespace Cstm CGAL
