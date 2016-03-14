#include "utils.h"

#include <CGAL/Polygon_2.h>
#include <CGAL/Straight_skeleton_builder_2.h>
#include <CGAL/Polygon_offset_builder_2.h>
#include <CGAL/compute_outer_frame_margin.h>
#include <CGAL/create_offset_polygons_from_polygon_with_holes_2.h>

#include <boost/make_shared.hpp>

#include <algorithm>
#include <vector>

typedef Ss::Halfedge_iterator Halfedge_iterator;
typedef Ss::Halfedge_handle   Halfedge_handle;
typedef Ss::Vertex_handle     Vertex_handle;

typedef CGAL::Straight_skeleton_builder_traits_2<Kernel>      SsBuilderTraits;
typedef CGAL::Straight_skeleton_builder_2<SsBuilderTraits,Ss> SsBuilder;

typedef CGAL::Polygon_offset_builder_traits_2<Kernel>                     OffsetBuilderTraits;
typedef CGAL::Polygon_offset_builder_2<Ss,OffsetBuilderTraits,Polygon_2>  OffsetBuilder;


PwhPtr CstmCGAL::applyOffset(double offset, const Polygon_with_holes_2& poly) {

  // This code is inspired from the CGAL example Straight_skeleton_2/Low_level_API
  // As the offset can only produce an interior polygon, we need to produce a frame
  // that encloses the polygon and is big enough so that the offset of the contour
  // does not interfere with the one ot the polygon. See CGAL doc page for more info
  boost::optional<double> margin = CGAL::compute_outer_frame_margin(
    poly.outer_boundary().vertices_begin(),poly.outer_boundary().vertices_end(),offset);

  if ( margin ) {

    CGAL::Bbox_2 bbox = CGAL::bbox_2(poly.outer_boundary().vertices_begin(),poly.outer_boundary().vertices_end());

    double fxmin = bbox.xmin() - *margin ;
    double fxmax = bbox.xmax() + *margin ;
    double fymin = bbox.ymin() - *margin ;
    double fymax = bbox.ymax() + *margin ;

    // Create the rectangular frame
    Point_2 frame[4]= { Point_2(fxmin,fymin)
                      , Point_2(fxmax,fymin)
                      , Point_2(fxmax,fymax)
                      , Point_2(fxmin,fymax)
                      } ;

    SsBuilder ssb ;

    ssb.enter_contour(frame,frame+4);

    // We have to revert the orientation of the polygon
    std::vector<Point_2> outerBoundary = std::vector<Point_2>(
      poly.outer_boundary().vertices_begin(),poly.outer_boundary().vertices_end());

    ssb.enter_contour(outerBoundary.rbegin(), outerBoundary.rend());

    SsPtr ss = ssb.construct_skeleton();

    if ( ss ) {
      std::vector<Polygon_2Ptr> offset_contours ;

      OffsetBuilder ob(*ss);

      ob.construct_offset_contours(offset, std::back_inserter(offset_contours));

      // Locate the offset contour that corresponds to the frame
      // That must be the outmost offset contour, which in turn must be the one
      // with the largest unsigned area.
      std::vector<Polygon_2Ptr>::iterator f = offset_contours.end();
      double lLargestArea = 0.0 ;
      for (std::vector<Polygon_2Ptr>::iterator i = offset_contours.begin(); i != offset_contours.end(); ++ i) {
        double lArea = CGAL_NTS abs( (*i)->area() ) ; //Take abs() as  Polygon_2::area() is signed.
        if ( lArea > lLargestArea ) {
          f = i ;
          lLargestArea = lArea ;
        }
      }

      offset_contours.erase(f);

      // Construct result polygon

      std::vector<Point_2> newOuterBoundary = std::vector<Point_2>(
        offset_contours.front()->vertices_begin(), offset_contours.front()->vertices_end());

      Polygon_with_holes_2 result = Polygon_with_holes_2(Polygon_2(newOuterBoundary.rbegin(), newOuterBoundary.rend()));

      // We have to handle the holes separately

      for (auto it = poly.holes_begin() ; it != poly.holes_end() ; it++) {
        std::vector<Point_2> hole = std::vector<Point_2>(it->vertices_begin(),it->vertices_end());

        std::vector<PwhPtr> holeOffsets =
        CGAL::create_interior_skeleton_and_offset_polygons_with_holes_2(offset,
          Polygon_with_holes_2(Polygon_2(hole.begin(), hole.end())));

        for (auto it2 = holeOffsets.begin() ; it2 != holeOffsets.end() ; it++) {
          std::vector<Point_2> revertNewHoles = std::vector<Point_2>(
            (*it2)->outer_boundary().vertices_begin(),(*it2)->outer_boundary().vertices_end());

          result.add_hole(Polygon_2(revertNewHoles.rbegin(), revertNewHoles.rend()));
        }
      }

      return boost::make_shared<Polygon_with_holes_2>(result);
    }
  }

  return NULL;
}

std::list<Polygon_with_holes_2> CstmCGAL::splitPoly(const Polygon_with_holes_2& poly) {
  std::vector<Point_2> outerBoundary = std::vector<Point_2>(
    poly.outer_boundary().vertices_begin(),poly.outer_boundary().vertices_end());

  std::list<Polygon_with_holes_2> result;

  for (unsigned int i = 0 ; i < outerBoundary.size() ; i++) {
    for (unsigned int j = i+1 ; j < outerBoundary.size() ; j++) {
      if (outerBoundary[i] == outerBoundary[j]) {
        result.splice(result.end(), splitPoly(Polygon_with_holes_2(
          Polygon_2(outerBoundary.begin() + i, outerBoundary.begin() + j)
        )));

        for (unsigned int k = i+1 ; k < outerBoundary.size() ; k++) {
          outerBoundary[k] = outerBoundary[k + j - i];
        }

        outerBoundary.resize(outerBoundary.size() - j + i);
        break;
      }
    }
  }

  result.push_back(Polygon_with_holes_2(
    Polygon_2(outerBoundary.begin(), outerBoundary.end())));

  return result;
}

void CstmCGAL::splitFace(Mesh& mesh, face_descriptor f) {
  if (mesh.degree(f) > 4) {
    halfedge_descriptor h = mesh.halfedge(f);

    mesh.set_face(h,Mesh::null_face());
    mesh.set_face(mesh.next(h),Mesh::null_face());

    mesh.add_face(
      mesh.source(h),
      mesh.source(mesh.next(h)),
      mesh.source(mesh.next(mesh.next(h)))
    );

    mesh.set_face(mesh.opposite(mesh.prev(h)),f);
    mesh.set_halfedge(f,mesh.opposite(mesh.prev(h)));

    splitFace(mesh, f);
  }
}

void CstmCGAL::splitFaces(Mesh& mesh) {
  Mesh::Face_range::iterator f, f_end;
  for (boost::tie(f,f_end) = mesh.faces(); f != f_end ; f++) {
    splitFace(mesh,*f);
  }
}
