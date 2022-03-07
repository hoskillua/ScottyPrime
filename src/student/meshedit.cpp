
#include <iostream>
#include <queue>
#include <set>
#include <unordered_map>

#include "../geometry/halfedge.h"
#include "debug.h"

/******************************************************************
*********************** Local Operations **************************
******************************************************************/

/* Note on local operation return types:

    The local operations all return a std::optional<T> type. This is used so that your
    implementation can signify that it does not want to perform the operation for
    whatever reason (e.g. you don't want to allow the user to erase the last vertex).

    An optional can have two values: std::nullopt, or a value of the type it is
    parameterized on. In this way, it's similar to a pointer, but has two advantages:
    the value it holds need not be allocated elsewhere, and it provides an API that
    forces the user to check if it is null before using the value.

    In your implementation, if you have successfully performed the operation, you can
    simply return the required reference:

            ... collapse the edge ...
            return collapsed_vertex_ref;

    And if you wish to deny the operation, you can return the null optional:

            return std::nullopt;

    Note that the stubs below all reject their duties by returning the null optional.
*/

/*
    This method splits the given edge in half, but does not split the
    adjacent faces. Returns an iterator to the new vertex which splits
    the original edge.

    Example function for how to go about implementing local operations
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::bisect_edge(EdgeRef e) {

    if(e->on_boundary()) return std::nullopt;
    // Phase 1: collect all elements
    HalfedgeRef h = (e->halfedge()->is_boundary()) ? e->halfedge()->twin() : e->halfedge();
    HalfedgeRef ht = h->twin();
    HalfedgeRef preh = h;
    HalfedgeRef nexht = ht->next();
    do {
        preh = preh->next();
    } while(preh->next() != h);
    Vec3 vpos = (h->vertex()->pos + ht->vertex()->pos) / 2;

    // Phase 2: Allocate new elements
    VertexRef c = new_vertex();
    c->pos = vpos;
    HalfedgeRef hn = new_halfedge();
    HalfedgeRef hnt = new_halfedge();
    EdgeRef e0 = new_edge();

    // The following elements aren't necessary for the bisect_edge, but they are here to demonstrate
    // phase 4
    FaceRef f_not_used = new_face();
    HalfedgeRef h_not_used = new_halfedge();

    // Phase 3: Reassign elements
    e0->halfedge() = hn;
    hn->twin() = hnt;
    hn->edge() = e0;
    hn->vertex() = h->vertex();
    hn->face() = h->face();
    preh->next() = hn;
    hn->next() = h;
    h->vertex() = c;
    ht->next() = hnt;
    c->halfedge() = h;
    hn->vertex()->halfedge() = hn;
    c->is_new = true;

    // example of set_neighbors:
    // condenses hnt->next() = nexht; hnt->twin() = hn; hnt->vertex() = c; hnt->edge() = e0;
    // hnt->face() = ht->face(); into one line
    hnt->set_neighbors(nexht, hn, c, e0, ht->face());

    // Phase 4: Delete unused elements
    erase(f_not_used);
    erase(h_not_used);

    // Phase 5: Return the correct iterator
    return c;
}

/*
    This method should replace the given vertex and all its neighboring
    edges and faces with a single face, returning the new face.
 */
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::erase_vertex(Halfedge_Mesh::VertexRef v) {
    /// TODO: Deal with some of the boundary cases
    if(v->on_boundary()) return std::nullopt;

    HalfedgeRef h = v->halfedge();

    // totf is the new face to replace delated faces/edges around v
    FaceRef totf = new_face();
    HalfedgeRef itrh = h;
    totf->halfedge() = h->next();

    std::queue<HalfedgeRef> h_to_be_deleted;

    /// pre_itrht -> itrht
    ///              itrh -> nex_itrh

    do {
        HalfedgeRef itrht = itrh->twin();
        HalfedgeRef pre_itrht = itrht;
        HalfedgeRef nex_itrh = itrh->next();

        // getting the edge whose next() is itrht (twin of iterating halfedge)
        do {
            pre_itrht = pre_itrht->next();
        } while(pre_itrht->next() != itrht);

        pre_itrht->next() = nex_itrh;
        nex_itrh->vertex()->halfedge() = nex_itrh;

        h_to_be_deleted.push(itrh);

        itrh = itrh->twin()->next();

    } while(itrh != h);

    // assigning new face as the face for all in-loop half edges

    HalfedgeRef h_to_delete = h_to_be_deleted.front();
    HalfedgeRef face_iter_h = h_to_delete->next();
    do {
        face_iter_h->face() = totf;
        face_iter_h = face_iter_h->next();
    } while(face_iter_h != h_to_delete->next());

    // deleting elements associated with the removed vertex

    while(!h_to_be_deleted.empty()) {
        h_to_delete = h_to_be_deleted.front();
        h_to_be_deleted.pop();

        erase(h_to_delete);
        erase(h_to_delete->twin());
        erase(h_to_delete->edge());
        erase(h_to_delete->face());
    }
    erase(v);

    // totf->boundary = v->on_boundary();

    return totf;

    /// TODO:
    /// Deal with degenerate face (multiple loops of edges/vertices)
}

/*
    This method should erase the given edge and return an iterator to the
    merged face.
 */
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::erase_edge(Halfedge_Mesh::EdgeRef e) {
    /// TODO: Deal with some of the boundary cases
    /// TODO: If degree of one of the vertices is one: we remove the edge but without merging faces
    if(e->on_boundary()) return std::nullopt;

    // getting half edge, its twin and their nexts and prevs
    HalfedgeRef h = e->halfedge();
    HalfedgeRef ht = h->twin();

    HalfedgeRef nexh = h->next();
    HalfedgeRef nexht = ht->next();

    HalfedgeRef preh = h;
    HalfedgeRef preht = ht;

    do {
        preht = preht->next();
    } while(preht->next() != ht);

    do {
        preh = preh->next();

        // appending halfedges from the face to be deleted into the face to be merged
        preh->face() = ht->face();
    } while(preh->next() != h);

    // reassignment
    preh->next() = nexht;
    preht->next() = nexh;
    ht->vertex()->halfedge() = nexh;
    h->vertex()->halfedge() = nexht;
    ht->face()->halfedge() = nexht;

    // deleting elements associated with the removed edge
    erase(h);
    erase(ht);
    erase(h->edge());
    erase(h->face());

    return ht->face();

    /// TODO:
    /// Deal with degenerate face (multiple loops of edges/vertices)
}

/*
    This method should collapse the given edge and return an iterator to
    the new vertex created by the collapse.
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::collapse_edge(Halfedge_Mesh::EdgeRef e) {

    if(e->on_boundary()) return std::nullopt;
    // getting half edge, its twin and their nexts and prevs
    HalfedgeRef h = e->halfedge();
    HalfedgeRef ht = h->twin();

    int degh = h->face()->degree();
    int deght = ht->face()->degree();

    HalfedgeRef nexh = h->next();
    HalfedgeRef nexht = ht->next();

    HalfedgeRef preh = h;
    HalfedgeRef preht = ht;

    VertexRef v_to_delete = ht->vertex();

    HalfedgeRef itrht = ht->twin()->next();

    Vec3 vpos = (h->vertex()->pos + ht->vertex()->pos) / 2;


    do {
        preht = preht->next();
    } while(preht->next() != ht);

    do {
        preh = preh->next();
    } while(preh->next() != h);

    // reassignment

    while(itrht != ht) {
        itrht->vertex() = h->vertex();
        itrht = itrht->twin()->next();
    }
    if(degh > 3)
        h->vertex()->halfedge() = nexh;
    else 
        h->vertex()->halfedge() = nexh->twin()->next();

    h->face()->halfedge() = nexh;

    if(deght > 3)
        ht->vertex()->halfedge() = nexht;
    else
        ht->vertex()->halfedge() = nexht->twin()->next();

    ht->face()->halfedge() = nexht;

    preh->next() = nexh;
    preht->next() = nexht;

    if(degh <= 3)
    {
        preh->vertex()->halfedge() = nexh->twin();
        preh->edge()->halfedge() = preh->twin();
        nexh->twin()->edge() = preh->edge();

        nexh->twin()->twin() = preh->twin();
        preh->twin()->twin() = nexh->twin();

        erase(nexh->edge());
        erase(nexh);
        erase(preh);
        erase(h->face());
    }
    if(deght <= 3) {
        preht->vertex()->halfedge() = nexht->twin();
        preht->twin()->edge() = nexht->edge();
        nexht->edge()->halfedge() = nexht->twin();


        nexht->twin()->twin() = preht->twin();
        preht->twin()->twin() = nexht->twin();

        erase(preht->edge());
        erase(nexht);
        erase(preht);
        erase(ht->face()); 
    }

    h->vertex()->pos = vpos;

    erase(v_to_delete);
    erase(h);
    erase(ht);
    erase(e);


    return h->vertex();
}

/*
    This method should collapse the given face and return an iterator to
    the new vertex created by the collapse.
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::collapse_face(Halfedge_Mesh::FaceRef f) {

    if(f->is_boundary()) return std::nullopt;

    Vec3 c = f->center();

    while(f->degree() > 3) {
        collapse_edge(f->halfedge()->edge());
    }

    HalfedgeRef h_last = f->halfedge()->next()->twin();

    collapse_edge(f->halfedge()->edge());
    VertexRef v = collapse_edge(h_last->edge()).value();

    v->pos = c;

    return v;
    
}

/*
    This method should flip the given edge counter-clockwise and return an iterator to the
    flipped edge.
*/
std::optional<Halfedge_Mesh::EdgeRef> Halfedge_Mesh::flip_edge(Halfedge_Mesh::EdgeRef e) {
    if(e->on_boundary()) return std::nullopt;

    // getting key elements

    HalfedgeRef h = e->halfedge();
    HalfedgeRef ht = h->twin();

    HalfedgeRef nexh = h->next();
    HalfedgeRef nexht = ht->next();

    HalfedgeRef preh = h;
    HalfedgeRef preht = ht;

    do {
        preh = preh->next();
    } while(preh->next() != h);

    do {
        preht = preht->next();
    } while(preht->next() != ht);

    // reassignment

    nexh->face() = ht->face();
    nexht->face() = h->face();

    h->face()->halfedge() = h;
    ht->face()->halfedge() = ht;

    h->vertex()->halfedge() = nexht;
    ht->vertex()->halfedge() = nexh;

    h->vertex() = nexht->next()->vertex();
    ht->vertex() = nexh->next()->vertex();

    preh->next() = nexht;
    preht->next() = nexh;

    h->next() = nexh->next();
    ht->next() = nexht->next();

    nexht->next() = h;
    nexh->next() = ht;

    return e;
}

/*
    This method should split the given edge and return an iterator to the
    newly inserted vertex. The halfedge of this vertex should point along
    the edge that was split, rather than the new edges.
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::split_edge(Halfedge_Mesh::EdgeRef e) {

    // only works on non boundary trangular faces

    if(e->on_boundary() || e->halfedge()->twin()->face()->degree() > 3 ||
       e->halfedge()->face()->degree() > 3)
        return std::nullopt;

    ///    3
    ///    ^
    /// 2<   >1
    ///    v
    ///    4

    // using bisect to cut edge in half and add vertex

    VertexRef vmid = bisect_edge(e).value();

    // getting key elements

    HalfedgeRef h1out = vmid->halfedge();
    HalfedgeRef h2in = h1out->twin();
    HalfedgeRef h2out = h2in->next();
    HalfedgeRef h1in = h2out->twin();

    HalfedgeRef nexh1 = h1out->next();
    HalfedgeRef preh1 = nexh1->next();
    HalfedgeRef nexh2 = h2out->next();
    HalfedgeRef preh2 = nexh2->next();

    // new elements

    HalfedgeRef h3in = new_halfedge();  // <^
    HalfedgeRef h3out = new_halfedge(); // ^>
    HalfedgeRef h4in = new_halfedge();  // v>
    HalfedgeRef h4out = new_halfedge(); // <v
    EdgeRef e3in = new_edge();          // <
    EdgeRef e3out = new_edge();         // >
    FaceRef f3in = new_face();          // <^
    FaceRef f3out = new_face();         // ^>

    e3in->is_new = true;
    e3out->is_new = true;

    // reassignment

    h1out->face()->halfedge() = h1out;
    h2in->face()->halfedge() = h2in;

    h4in->face() = h1out->face();
    h4out->face() = h2in->face();

    h3in->face() = f3in;
    h2out->face() = f3in;
    nexh2->face() = f3in;

    h3out->face() = f3out;
    h1in->face() = f3out;
    preh1->face() = f3out;

    h3in->edge() = e3in;
    h3out->edge() = e3out;
    h4in->edge() = e3out;
    h4out->edge() = e3in;

    h3in->vertex() = preh2->vertex();
    h3out->vertex() = vmid;
    h4in->vertex() = preh1->vertex();
    h4out->vertex() = vmid;

    e3in->halfedge() = h3in;
    e3out->halfedge() = h3out;
    f3in->halfedge() = h3in;
    f3out->halfedge() = h3out;

    h3in->twin() = h4out;
    h3out->twin() = h4in;
    h4out->twin() = h3in;
    h4in->twin() = h3out;

    h3in->next() = h2out;
    h3out->next() = preh1;
    h4in->next() = h1out;
    h4out->next() = preh2;

    nexh2->next() = h3in;
    h2in->next() = h4out;
    nexh1->next() = h4in;
    h1in->next() = h3out;

    return vmid;
}

/*
    This method should insets a vertex into the given face, returning a pointer to the new center
   vertex
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::inset_vertex(FaceRef f) {
    if(f->boundary) return std::nullopt;

    f = bevel_face(f).value();

    return collapse_face(f);

}

/*
    This method should inset a face into the given face, returning a pointer to the new face.
*/
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::inset_face(Halfedge_Mesh::FaceRef f) {

    f = bevel_face(f).value();

    Vec3 center = f->center();

    auto h = f->halfedge();
    do {
        h->vertex()->pos = (h->vertex()->pos + center) / 2.0f;
        h = h->next();
    } while(h != f->halfedge());

    return f;
    // hint: use bevel_face positions as a helper function here
}

/*
    This method should bevel a vertex and inserts a vertex into the new vertex, returning a pointer
   to that vertex
*/
std::optional<Halfedge_Mesh::VertexRef> Halfedge_Mesh::extrude_vertex(VertexRef v) {
    if(v->on_boundary()) return std::nullopt;

    FaceRef f = bevel_vertex(v).value();
    
    std::vector<Vec3> V(f->degree(), v->pos);

    bevel_vertex_positions( V, f, 0.667f);

    f = bevel_face(f).value();

    VertexRef vnew = collapse_face(f).value();
    

    return vnew;
}

/* Note on the beveling process:

    Each of the bevel_vertex, bevel_edge, and bevel_face functions do not represent
    a full bevel operation. Instead, they should update the _connectivity_ of
    the mesh, _not_ the positions of newly created vertices. In fact, you should set
    the positions of new vertices to be exactly the same as wherever they "started from."

    When you click on a mesh element while in bevel mode, one of those three functions
    is called. But, because you may then adjust the distance/offset of the newly
    beveled face, we need another method of updating the positions of the new vertices.

    This is where bevel_vertex_positions, bevel_edge_positions, and
    bevel_face_positions come in: these functions are called repeatedly as you
    move your mouse, the position of which determines the normal and tangent offset
    parameters. These functions are also passed an array of the original vertex
    positions: for bevel_vertex, it has one element, the original vertex position,
    for bevel_edge, two for the two vertices, and for bevel_face, it has the original
    position of each vertex in order starting from face->halfedge. You should use these
    positions, as well as the normal and tangent offset fields to assign positions to
    the new vertices.

    Finally, note that the normal and tangent offsets are not relative values - you
    should compute a particular new position from them, not a delta to apply.
*/

/*
    This method should replace the vertex v with a face, corresponding to
    a bevel operation. It should return the new face.  NOTE: This method is
    only responsible for updating the *connectivity* of the mesh---it does not
    need to update the vertex positions. These positions will be updated in
    Halfedge_Mesh::bevel_vertex_positions (which you also have to
    implement!)
*/
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::bevel_vertex(Halfedge_Mesh::VertexRef v) {
    
    int deg = v->degree();
    if(deg < 3 || v->on_boundary()) return std::nullopt;

    FaceRef bevface = new_face();
    HalfedgeRef itrh = v->halfedge();

    // get outgoing halfedges from original vertex

    std::vector<HalfedgeRef> itrh_vec;
    for(int i = 0; i < deg; i++) {
        itrh_vec.push_back(itrh);
        itrh = itrh->twin()->next();
    }

    // for each face neighbor to the new beveled face:

    for(int i = 0; i < deg; i++) {

        // creating new elements
        VertexRef vi = new_vertex();
        EdgeRef ei = new_edge();
        HalfedgeRef hin = new_halfedge();
        HalfedgeRef hout = new_halfedge();

        // getting face and preh
        FaceRef fout = itrh_vec[i]->face();

        HalfedgeRef preh = itrh_vec[i];
        do {
            preh = preh->next();
        } while(preh->next() != itrh_vec[i]);

        // reassignment
        hout->face() = fout;
        hin->face() = bevface;
        
        ei->halfedge() = hout;
        
        hin->edge() = ei;
        hout->edge() = ei;
        
        hin->vertex() = vi;
        itrh_vec[i]->vertex() = vi;

        // this is implemented in the next loop due to dependency 
        /// hout->vertex()
        
        vi->halfedge() = itrh_vec[i];
        
        hin->twin() = hout;
        hout->twin() = hin;
        
        preh->next() = hout;
        hout->next() = itrh_vec[i];

        // this is implemented in the next loop due to dependency 
        /// hin->next();
        
        //positions:
        vi->pos = v->pos;
        
    }

    for(int i = 0; i < deg; i++) {

        itrh_vec[i]->twin()->next()->vertex() = itrh_vec[i]->vertex();
        
        HalfedgeRef preh = itrh_vec[i];
        do {
            preh = preh->next();
        } while(preh->next() != itrh_vec[i]);
        itrh_vec[i]->twin()->next()->twin()->next() = preh->twin(); 
        
    }

    bevface->halfedge() = itrh_vec[0]->twin()->next()->twin();

    // erasing original vertex
    erase(v);

    return bevface;
    // Reminder: You should set the positions of new vertices (v->pos) to be exactly
    // the same as wherever they "started from."

}

/*
    This method should replace the edge e with a face, corresponding to a
    bevel operation. It should return the new face. NOTE: This method is
    responsible for updating the *connectivity* of the mesh only---it does not
    need to update the vertex positions. These positions will be updated in
    Halfedge_Mesh::bevel_edge_positions (which you also have to
    implement!)
*/
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::bevel_edge(Halfedge_Mesh::EdgeRef e) {

    // Reminder: You should set the positions of new vertices (v->pos) to be exactly
    // the same as wherever they "started from."
    
    if(e->on_boundary()) return std::nullopt;

    HalfedgeRef h = e->halfedge();
    HalfedgeRef ht = e->halfedge()->twin();

    FaceRef f = h->face();
    FaceRef ft = ht->face();

    VertexRef v = h->vertex();
    VertexRef vt = ht->vertex();


    //FaceRef f = bevel_vertex(v).value();
    //FaceRef ft = bevel_vertex(vt).value();

    VertexRef v0 = h->next()->next()->vertex();
    VertexRef v1 = ht->next()->twin()->next()->next()->vertex();
    VertexRef v2 = ht->next()->next()->vertex();
    VertexRef v3 = h->next()->twin()->next()->next()->vertex();

    HalfedgeRef h0 = new_halfedge();
    HalfedgeRef h1 = new_halfedge();
    HalfedgeRef h2 = new_halfedge();
    HalfedgeRef h3 = new_halfedge();

    EdgeRef e01 = new_edge();
    EdgeRef e23 = new_edge();

    v0->halfedge() = h0; 
    v1->halfedge() = h1; 
    v2->halfedge() = h2; 
    v3->halfedge() = h3; 

    h0->vertex() = v0;
    h1->vertex() = v1;
    h2->vertex() = v2;
    h3->vertex() = v3;

    e01->halfedge() = h0;
    e23->halfedge() = h2;

    h0->edge() = e01;
    h1->edge() = e01;
    h2->edge() = e23;
    h3->edge() = e23;

    h0->twin() = h1;
    h1->twin() = h0;
    h2->twin() = h3;
    h3->twin() = h2;

    h1->next() = h->next()->next();



    return erase_vertex(collapse_edge(e).value());

}

/*
    This method should replace the face f with an additional, inset face
    (and ring of faces around it), corresponding to a bevel operation. It
    should return the new face.  NOTE: This method is responsible for updating
    the *connectivity* of the mesh only---it does not need to update the vertex
    positions. These positions will be updated in
    Halfedge_Mesh::bevel_face_positions (which you also have to
    implement!)
*/
std::optional<Halfedge_Mesh::FaceRef> Halfedge_Mesh::bevel_face(Halfedge_Mesh::FaceRef f) {
    
    // Reminder: You should set the positions of new vertices (v->pos) to be exactly
    // the same as wherever they "started from."

    if(f->boundary) return std::nullopt;

    HalfedgeRef itrh = f->halfedge();
    size_t deg = f->degree();
    // get face halfedges from original vertex

    std::vector<HalfedgeRef> itrh_vec;
    for(int i = 0; i < deg; i++) {
        itrh_vec.push_back(itrh);
        itrh = itrh->next();
    }

    // for each face neighbor to the beveled face:

    for(int i = 0; i < deg; i++) {

        // creating new elements
        VertexRef vi = new_vertex();
        EdgeRef ei = new_edge();
        EdgeRef ein = new_edge();
        FaceRef fi = new_face();
        VertexRef v1 = itrh_vec[i]->vertex();
        VertexRef v2 = itrh_vec[i]->twin()->vertex();

        HalfedgeRef h0 = new_halfedge();
        HalfedgeRef h1 = new_halfedge();
        HalfedgeRef h2 = new_halfedge();
        HalfedgeRef h3 = new_halfedge();

        HalfedgeRef h = itrh_vec[i];
        HalfedgeRef ht = itrh_vec[i]->twin();

        // reassignment
        h0->face() = fi;
        h1->face() = fi;
        h2->face() = fi;
        h3->face() = fi;

        h->face() = f;
        f->halfedge() = h;

        fi->halfedge() = h0;

        h3->edge() = ein;
        h0->edge() = ei;
        h1->edge() = ht->edge();
        h->edge() = ein;

        ei->halfedge() = h0;
        ein->halfedge() = h3;
        ht->edge()->halfedge() = ht;

        h0->vertex() = vi;
        h1->vertex() = v1;
        h2->vertex() = v2;
        h->vertex() = vi;

        vi->halfedge() = h0;
        v1->halfedge() = h1;
        v2->halfedge() = h2;
        vi->pos = v1->pos;
        
        h0->next() = h1;
        h1->next() = h2;
        h2->next() = h3;
        h3->next() = h0;
        
        h1->twin() = ht;
        ht->twin() = h1;
        h3->twin() = h;
        h->twin() = h3;

        vi->pos = v1->pos;

        ///h0->twin() old <-> new h2->twin() 
        ///h2->edge() new -> ei old
        ///h3->vertex() new -> vi old 

    }

    for(size_t i = 0; i < deg; i++) {
        size_t p = (i) ? (i - 1) : deg - 1;
        itrh_vec[i]->twin()->next()->twin() = itrh_vec[p]->twin()->next()->next()->next();
        itrh_vec[p]->twin()->next()->next()->next()->twin() = itrh_vec[i]->twin()->next();
    
        itrh_vec[i]->twin()->next()->next()->next()->edge() = itrh_vec[i]->next()->twin()->next()->edge();
    
        itrh_vec[i]->twin()->vertex() = itrh_vec[i]->next()->vertex();

    }
    
    return f;

}

/*
    Compute new vertex positions for the vertices of the beveled vertex.

    These vertices can be accessed via new_halfedges[i]->vertex()->pos for
    i = 1, ..., new_halfedges.size()-1.

    The basic strategy here is to loop over the list of outgoing halfedges,
    and use the original vertex position and its associated outgoing edge
    to compute a new vertex position along the outgoing edge.
*/
void Halfedge_Mesh::bevel_vertex_positions(const std::vector<Vec3>& start_positions,
                                           Halfedge_Mesh::FaceRef face, float tangent_offset) {

    std::vector<HalfedgeRef> new_halfedges;
    auto h = face->halfedge();
    do {
        new_halfedges.push_back(h);
        h = h->next();
    } while(h != face->halfedge());
    
    // make sure vertex beveling is contained inside edges (and not flipped too)
    if(tangent_offset < 0) tangent_offset = -tangent_offset;
    if(tangent_offset > 1) tangent_offset = 1;


    for(int i = 0; i < new_halfedges.size(); i++) {
        // simple linear interpolation
        new_halfedges[i]->vertex()->pos =
            tangent_offset * (new_halfedges[i]->twin()->next()->edge()->center()) +
            (1 - tangent_offset) * start_positions[i];
    }
}

/*
    Compute new vertex positions for the vertices of the beveled edge.

    These vertices can be accessed via new_halfedges[i]->vertex()->pos for
    i = 1, ..., new_halfedges.size()-1.

    The basic strategy here is to loop over the list of outgoing halfedges,
    and use the preceding and next vertex position from the original mesh
    (in the orig array) to compute an offset vertex position.

    Note that there is a 1-to-1 correspondence between halfedges in
    newHalfedges and vertex positions in start_positions. So, you can write
    loops of the form:

    for(size_t i = 0; i < new_halfedges.size(); i++)
    {
            Vector3D pi = start_positions[i]; // get the original vertex
            position corresponding to vertex i
    }
*/
void Halfedge_Mesh::bevel_edge_positions(const std::vector<Vec3>& start_positions,
                                         Halfedge_Mesh::FaceRef face, float tangent_offset) {

    std::vector<HalfedgeRef> new_halfedges;
    auto h = face->halfedge();
    do {
        new_halfedges.push_back(h);
        h = h->next();
    } while(h != face->halfedge());

    (void)new_halfedges;
    (void)start_positions;
    (void)face;
    (void)tangent_offset;
}

/*
    Compute new vertex positions for the vertices of the beveled face.

    These vertices can be accessed via new_halfedges[i]->vertex()->pos for
    i = 1, ..., new_halfedges.size()-1.

    The basic strategy here is to loop over the list of outgoing halfedges,
    and use the preceding and next vertex position from the original mesh
    (in the start_positions array) to compute an offset vertex
    position.

    Note that there is a 1-to-1 correspondence between halfedges in
    new_halfedges and vertex positions in start_positions. So, you can write
    loops of the form:

    for(size_t i = 0; i < new_halfedges.size(); i++)
    {
            Vec3 pi = start_positions[i]; // get the original vertex
            position corresponding to vertex i
    }
*/
void Halfedge_Mesh::bevel_face_positions(const std::vector<Vec3>& start_positions,
                                         Halfedge_Mesh::FaceRef face, float tangent_offset,
                                         float normal_offset) {

    if(flip_orientation) normal_offset = -normal_offset;
    std::vector<HalfedgeRef> new_halfedges;
    auto h = face->halfedge();
    Vec3 FaceCenter(0, 0, 0);
    int i = 0;
    do {
        new_halfedges.push_back(h);
        FaceCenter += (start_positions[i] / (float) face->degree());
        h = h->next();
        i++;
    } while(h != face->halfedge());

    if(tangent_offset < -0.99) tangent_offset = tangent_offset * -1 -  2;

    Vec3 N = cross((new_halfedges[0]->edge()->center() - new_halfedges[1]->vertex()->pos),
                   (new_halfedges[1]->edge()->center() - new_halfedges[1]->vertex()->pos)).normalize();

    for(i = 0; i < new_halfedges.size(); i++) {
        new_halfedges[i]->vertex()->pos = start_positions[i] + 1.5f * normal_offset * N +
                                          tangent_offset * (start_positions[i] - FaceCenter);
    }

    
}

/*
    Updates the position of v using the given start_position
*/
void Halfedge_Mesh::extrude_vertex_position(const Vec3& start_positions,
                                            Halfedge_Mesh::FaceRef face) {
    (void)start_positions;
    (void)face;
}

/******************************************************************
*********************** Global Operations *************************
******************************************************************/

/*
    Splits all non-triangular faces into triangles.
*/
void Halfedge_Mesh::triangulate() {
    
    // For each face...
    std::unordered_map<int, bool> visited;
    std::queue<FaceRef> faces_bfs;

    faces_bfs.push(faces.begin());
    visited[faces.begin()->id()] = true;
    while(!faces_bfs.empty()) {
        FaceRef u = faces_bfs.front();
        faces_bfs.pop();

        if(u->degree() > 3) triangulate(u);

        HalfedgeRef h = u->halfedge();
        do {
            if(!visited[h->twin()->face()->id()]) {
                faces_bfs.push(h->twin()->face());
                visited[h->twin()->face()->id()] = true;
            }
            h = h->next();
        } while(h != u->halfedge());
    }
}

/*
    triangulates a face if it has > 3 degree
    zig-zag pattern
*/
void Halfedge_Mesh::triangulate(Halfedge_Mesh::FaceRef f) {
    std::vector<HalfedgeRef> he_order;

    HalfedgeRef itrh = f->halfedge();

    for(size_t i = 0; i < f->degree(); i++) {
        he_order.push_back(itrh);
        itrh = itrh->next();
    }

    // use 2 pointers to implement zig-zag pattern;
    size_t pointer1 = 1;
    size_t pointer2 = he_order.size() - 1;

    size_t direction = 1;
    FaceRef fi = f;

    while(pointer2 - pointer1 > 1) {
        
        EdgeRef ei = new_edge();
        HalfedgeRef hin = new_halfedge();
        HalfedgeRef hout = new_halfedge();
        ei->is_new = true;

        hout->face() = fi;
        
        fi = new_face();
        
        fi->halfedge() = hin;
        he_order[pointer1]->face() = fi;
        he_order[pointer2 - 1]->face() = fi;
        hin->face() = fi;        

        ei->halfedge() = hin;

        hin->edge() = ei;
        hout->edge() = ei;
        
        hout->vertex() = he_order[pointer1]->vertex();
        
        hin->vertex() = he_order[pointer2]->vertex();
        
        he_order[pointer1 - 1]->next() = hout;
        hout->next() = he_order[pointer2];
        hin->next() = he_order[pointer1];
        he_order[pointer2 - 1]->next() = hin;
        
        hin->twin() = hout;
        hout->twin() = hin;
        
        if (direction)
            he_order[pointer2] = hin;
        else 
            he_order[pointer1-1] = hin;
        

        // this makes the pattern go like
        // 1, n-1 
        // 2, n-1 (+1,0)
        // 2, n-2 (0,-1)
        // 3, n-2 (+1,0)
        pointer1 += (size_t)direction;
        pointer2 -= (1 - (size_t)direction);
        direction = (1-direction);
    }
}

    /* Note on the quad subdivision process:

        Unlike the local mesh operations (like bevel or edge flip), we will perform
        subdivision by splitting *all* faces into quads "simultaneously."  Rather
        than operating directly on the halfedge data structure (which as you've
        seen is quite difficult to maintain!) we are going to do something a bit nicer:
           1. Create a raw list of vertex positions and faces (rather than a full-
              blown halfedge mesh).
           2. Build a new halfedge mesh from these lists, replacing the old one.
        Sometimes rebuilding a data structure from scratch is simpler (and even
        more efficient) than incrementally modifying the existing one.  These steps are
        detailed below.

  Step I: Compute the vertex positions for the subdivided mesh.
        Here we're going to do something a little bit strange: since we will
        have one vertex in the subdivided mesh for each vertex, edge, and face in
        the original mesh, we can nicely store the new vertex *positions* as
        attributes on vertices, edges, and faces of the original mesh. These positions
        can then be conveniently copied into the new, subdivided mesh.
        This is what you will implement in linear_subdivide_positions() and
        catmullclark_subdivide_positions().

  Steps II-IV are provided (see Halfedge_Mesh::subdivide()), but are still detailed
  here:

  Step II: Assign a unique index (starting at 0) to each vertex, edge, and
        face in the original mesh. These indices will be the indices of the
        vertices in the new (subdivided) mesh. They do not have to be assigned
        in any particular order, so long as no index is shared by more than one
        mesh element, and the total number of indices is equal to V+E+F, i.e.,
        the total number of vertices plus edges plus faces in the original mesh.
        Basically we just need a one-to-one mapping between original mesh elements
        and subdivided mesh vertices.

  Step III: Build a list of quads in the new (subdivided) mesh, as tuples of
        the element indices defined above. In other words, each new quad should be
        of the form (i,j,k,l), where i,j,k and l are four of the indices stored on
        our original mesh elements.  Note that it is essential to get the orientation
        right here: (i,j,k,l) is not the same as (l,k,j,i).  Indices of new faces
        should circulate in the same direction as old faces (think about the right-hand
        rule).

  Step IV: Pass the list of vertices and quads to a routine that clears
        the internal data for this halfedge mesh, and builds new halfedge data from
        scratch, using the two lists.
*/

/*
    Compute new vertex positions for a mesh that splits each polygon
    into quads (by inserting a vertex at the face midpoint and each
    of the edge midpoints).  The new vertex positions will be stored
    in the members Vertex::new_pos, Edge::new_pos, and
    Face::new_pos.  The values of the positions are based on
    simple linear interpolation, e.g., the edge midpoints and face
    centroids.
*/
void Halfedge_Mesh::linear_subdivide_positions() {

    // For each vertex, assign Vertex::new_pos to
    // its original position, Vertex::pos.
    for(std::list<Vertex>::iterator v = vertices.begin(); v != vertices.end(); ++v) {
        v->new_pos = v->pos;
    }
    

    // For each edge, assign the midpoint of the two original
    // positions to Edge::new_pos.
    for(std::list<Edge>::iterator e = edges.begin(); e != edges.end(); ++e) {
        e->new_pos = e->center();
    }

    // For each face, assign the centroid (i.e., arithmetic mean)
    // of the original vertex positions to Face::new_pos. Note
    // that in general, NOT all faces will be triangles!
    for(std::list<Face>::iterator f = faces.begin(); f != faces.end(); ++f) {
        f->new_pos = f->center();
    }


}

/*
    Compute new vertex positions for a mesh that splits each polygon
    into quads (by inserting a vertex at the face midpoint and each
    of the edge midpoints).  The new vertex positions will be stored
    in the members Vertex::new_pos, Edge::new_pos, and
    Face::new_pos. The values of the positions are based on
    the Catmull-Clark rules for subdivision.

    Note: this will only be called on meshes without boundary
*/
void Halfedge_Mesh::catmullclark_subdivide_positions() {

    // The implementation for this routine should be
    // a lot like Halfedge_Mesh:linear_subdivide_positions:(),
    // except that the calculation of the positions themsevles is
    // slightly more involved, using the Catmull-Clark subdivision
    // rules. (These rules are outlined in the Developer Manual.)

    // For each face, assign the centroid (i.e., arithmetic mean)
    // of the original vertex positions to Face::new_pos. Note
    // that in general, NOT all faces will be triangles!
    for(std::list<Face>::iterator f = faces.begin(); f != faces.end(); ++f) {
        f->new_pos = f->center();
    }

    // For each edge, assign the midpoint of the two original
    // positions to Edge::new_pos.
    for(std::list<Edge>::iterator e = edges.begin(); e != edges.end(); ++e) {
        e->new_pos = (e->center() * 2 + e->halfedge()->face()->new_pos +
                      e->halfedge()->twin()->face()->new_pos) / 4;
    }

    // For each vertex, assign Vertex::new_pos to
    // its original position, Vertex::pos.
    for(std::list<Vertex>::iterator v = vertices.begin(); v != vertices.end(); ++v) {
        Vec3 R(0), Q(0);
        HalfedgeRef itrh = v->halfedge();
        do {
            R += (itrh->edge()->center()) / (float) (v->degree());
            Q += (itrh->face()->new_pos) / (float) (v->degree());
            itrh = itrh->twin()->next();
        } while(itrh != v->halfedge());
        

        v->new_pos = ((float) (v->degree() - 3) * v->pos + 2 * R + Q) / (float)v->degree();
    }
}

/*
    This routine should increase the number of triangles in the mesh
    using Loop subdivision. Note: this is will only be called on triangle meshes.
    if "linear" is true, loop subdivision only affects connectiviy (like Linear vs Catmull-clark)
*/
void Halfedge_Mesh::loop_subdivide(bool linear) {

    for(std::list<Vertex>::iterator v = vertices.begin(); v != vertices.end(); ++v) {
        v->is_new = false;
        if(!linear) {
            size_t deg = v->degree();
            float u = (deg == 3) ? 3.0f / 16 : 3.0f / (8 * deg);
            Vec3 Usum(0);
            HalfedgeRef itrh = v->halfedge();
            do {
                Usum += (itrh->twin()->vertex()->pos) * (u);
                itrh = itrh->twin()->next();
            } while(itrh != v->halfedge());

            v->new_pos = (1 - deg * u) * v->pos + Usum;
        }
    }
    
    for(std::list<Edge>::iterator e = edges.begin(); e != edges.end(); ++e) {
        e->is_new = false;
        if(!linear) {
            Vec3 opposite = (e->halfedge()->next()->next()->vertex()->pos +
                             e->halfedge()->twin()->next()->next()->vertex()->pos) /
                            2.0f;
            e->new_pos = e->center() * 0.75f + (opposite)*0.25f;
        }
    }

    for(std::list<Edge>::iterator e = edges.begin(); e != edges.end(); ++e) {
        if(!(e->is_new) && !(e->halfedge()->vertex()->is_new) && !(e->halfedge()->twin()->vertex()->is_new)) {
            VertexRef v = split_edge(e).value();
            v->new_pos = e->new_pos;
        }
    }

    for(std::list<Edge>::iterator e = edges.begin(); e != edges.end(); ++e) {
        if(e->is_new && !(e->halfedge()->vertex()->is_new && e->halfedge()->twin()->vertex()->is_new))
            flip_edge(e);
    }

    if(!linear)
        for(std::list<Vertex>::iterator v = vertices.begin(); v != vertices.end(); ++v) {
            v->pos = v->new_pos;
        }



    // Each vertex and edge of the original mesh can be associated with a
    // vertex in the new (subdivided) mesh.
    // Therefore, our strategy for computing the subdivided vertex locations is to
    // *first* compute the new positions
    // using the connectivity of the original (coarse) mesh. Navigating this mesh
    // will be much easier than navigating
    // the new subdivided (fine) mesh, which has more elements to traverse.  We
    // will then assign vertex positions in
    // the new mesh based on the values we computed for the original mesh.

    // Compute new positions for all the vertices in the input mesh using
    // the Loop subdivision rule and store them in Vertex::new_pos.
    //    At this point, we also want to mark each vertex as being a vertex of the
    //    original mesh. Use Vertex::is_new for this.

    // Next, compute the subdivided vertex positions associated with edges, and
    // store them in Edge::new_pos.

    // Next, we're going to split every edge in the mesh, in any order.
    // We're also going to distinguish subdivided edges that came from splitting
    // an edge in the original mesh from new edges by setting the boolean Edge::is_new.
    // Note that in this loop, we only want to iterate over edges of the original mesh.
    // Otherwise, we'll end up splitting edges that we just split (and the
    // loop will never end!)

    // Now flip any new edge that connects an old and new vertex.

    // Finally, copy new vertex positions into the Vertex::pos.
}

/*
    Isotropic remeshing. Note that this function returns success in a similar
    manner to the local operations, except with only a boolean value.
    (e.g. you may want to return false if this is not a triangle mesh)
*/
bool Halfedge_Mesh::isotropic_remesh() {

    // Compute the mean edge length.
    // Repeat the four main steps for 5 or 6 iterations
    // -> Split edges much longer than the target length (being careful about
    //    how the loop is written!)
    // -> Collapse edges much shorter than the target length.  Here we need to
    //    be EXTRA careful about advancing the loop, because many edges may have
    //    been destroyed by a collapse (which ones?)
    // -> Now flip each edge if it improves vertex degree
    // -> Finally, apply some tangential smoothing to the vertex positions

    // Note: if you erase elements in a local operation, they will not be actually deleted
    // until do_erase or validate is called. This is to facilitate checking
    // for dangling references to elements that will be erased.
    // The rest of the codebase will automatically call validate() after each op,
    // but here simply calling collapse_edge() will not erase the elements.
    // You should use collapse_edge_erase() instead for the desired behavior.

    return false;
}

/* Helper type for quadric simplification */
struct Edge_Record {
    Edge_Record() {
    }
    Edge_Record(std::unordered_map<Halfedge_Mesh::VertexRef, Mat4>& vertex_quadrics,
                Halfedge_Mesh::EdgeRef e)
        : edge(e) {

        // Compute the combined quadric from the edge endpoints.
        // -> Build the 3x3 linear system whose solution minimizes the quadric error
        //    associated with these two endpoints.
        // -> Use this system to solve for the optimal position, and store it in
        //    Edge_Record::optimal.
        // -> Also store the cost associated with collapsing this edge in
        //    Edge_Record::cost.
    }
    Halfedge_Mesh::EdgeRef edge;
    Vec3 optimal;
    float cost;
};

/* Comparison operator for Edge_Records so std::set will properly order them */
bool operator<(const Edge_Record& r1, const Edge_Record& r2) {
    if(r1.cost != r2.cost) {
        return r1.cost < r2.cost;
    }
    Halfedge_Mesh::EdgeRef e1 = r1.edge;
    Halfedge_Mesh::EdgeRef e2 = r2.edge;
    return &*e1 < &*e2;
}

/** Helper type for quadric simplification
 *
 * A PQueue is a minimum-priority queue that
 * allows elements to be both inserted and removed from the
 * queue.  Together, one can easily change the priority of
 * an item by removing it, and re-inserting the same item
 * but with a different priority.  A priority queue, for
 * those who don't remember or haven't seen it before, is a
 * data structure that always keeps track of the item with
 * the smallest priority or "score," even as new elements
 * are inserted and removed.  Priority queues are often an
 * essential component of greedy algorithms, where one wants
 * to iteratively operate on the current "best" element.
 *
 * PQueue is templated on the type T of the object
 * being queued.  For this reason, T must define a comparison
 * operator of the form
 *
 *    bool operator<( const T& t1, const T& t2 )
 *
 * which returns true if and only if t1 is considered to have a
 * lower priority than t2.
 *
 * Basic use of a PQueue might look
 * something like this:
 *
 *    // initialize an empty queue
 *    PQueue<myItemType> queue;
 *
 *    // add some items (which we assume have been created
 *    // elsewhere, each of which has its priority stored as
 *    // some kind of internal member variable)
 *    queue.insert( item1 );
 *    queue.insert( item2 );
 *    queue.insert( item3 );
 *
 *    // get the highest priority item currently in the queue
 *    myItemType highestPriorityItem = queue.top();
 *
 *    // remove the highest priority item, automatically
 *    // promoting the next-highest priority item to the top
 *    queue.pop();
 *
 *    myItemType nextHighestPriorityItem = queue.top();
 *
 *    // Etc.
 *
 *    // We can also remove an item, making sure it is no
 *    // longer in the queue (note that this item may already
 *    // have been removed, if it was the 1st or 2nd-highest
 *    // priority item!)
 *    queue.remove( item2 );
 *
 */
template<class T> struct PQueue {
    void insert(const T& item) {
        queue.insert(item);
    }
    void remove(const T& item) {
        if(queue.find(item) != queue.end()) {
            queue.erase(item);
        }
    }
    const T& top(void) const {
        return *(queue.begin());
    }
    void pop(void) {
        queue.erase(queue.begin());
    }
    size_t size() {
        return queue.size();
    }

    std::set<T> queue;
};

/*
    Mesh simplification. Note that this function returns success in a similar
    manner to the local operations, except with only a boolean value.
    (e.g. you may want to return false if you can't simplify the mesh any
    further without destroying it.)
*/
bool Halfedge_Mesh::simplify() {

    std::unordered_map<VertexRef, Mat4> vertex_quadrics;
    std::unordered_map<FaceRef, Mat4> face_quadrics;
    std::unordered_map<EdgeRef, Edge_Record> edge_records;
    PQueue<Edge_Record> edge_queue;

    // Compute initial quadrics for each face by simply writing the plane equation
    // for the face in homogeneous coordinates. These quadrics should be stored
    // in face_quadrics
    // -> Compute an initial quadric for each vertex as the sum of the quadrics
    //    associated with the incident faces, storing it in vertex_quadrics
    // -> Build a priority queue of edges according to their quadric error cost,
    //    i.e., by building an Edge_Record for each edge and sticking it in the
    //    queue. You may want to use the above PQueue<Edge_Record> for this.
    // -> Until we reach the target edge budget, collapse the best edge. Remember
    //    to remove from the queue any edge that touches the collapsing edge
    //    BEFORE it gets collapsed, and add back into the queue any edge touching
    //    the collapsed vertex AFTER it's been collapsed. Also remember to assign
    //    a quadric to the collapsed vertex, and to pop the collapsed edge off the
    //    top of the queue.

    // Note: if you erase elements in a local operation, they will not be actually deleted
    // until do_erase or validate are called. This is to facilitate checking
    // for dangling references to elements that will be erased.
    // The rest of the codebase will automatically call validate() after each op,
    // but here simply calling collapse_edge() will not erase the elements.
    // You should use collapse_edge_erase() instead for the desired behavior.

    return false;
}
