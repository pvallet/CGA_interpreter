#include "utils.h"

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
