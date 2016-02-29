#include "custom_join.h"

void CstmCGAL::join(const std::list<Polygon_with_holes_2>& P,
                        std::list<Polygon_with_holes_2>& R) {

  CGAL::join (P.begin(), P.end(), std::back_inserter(R));
}

void CstmCGAL::join(const std::list<Polygon_with_holes_2>& P,
                  const std::list<Polygon_with_holes_2>& Q,
                        std::list<Polygon_with_holes_2>& R) {

  CGAL::join (P.begin(), P.end(), Q.begin(), Q.end(), std::back_inserter(R));
}
