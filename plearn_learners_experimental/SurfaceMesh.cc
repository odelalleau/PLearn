// -*- C++ -*-

// SurfaceMesh.cc
//
// Copyright (C) 2004 Pascal Lamblin 
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

/* *******************************************************      
   * $Id: SurfaceMesh.cc,v 1.18 2005/12/29 13:44:35 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file SurfaceMesh.cc */


#include "SurfaceMesh.h"
#include "geometry.h"

namespace PLearn {
using namespace std;

SurfaceMesh::SurfaceMesh() :
  computed_vtx_norm( false ),
  mesh_type( "PointSet" ),
  verbosity(1)
{
  p_mesh = new Graph_();
  // because graph_ppt field isn't correctly initialized
  mg = new MeshGraph();
  set_property( *p_mesh, graph_ppt, mg );

}

PLEARN_IMPLEMENT_OBJECT(SurfaceMesh,
    "Mesh covering a Surface, with Vertices, Edges, and Faces",
    "" );

SurfaceMesh::SurfaceMesh( Graph g ) :
  computed_vtx_norm( false ),
  p_mesh( g ),
  mesh_type( "PointSet" )
{
  mg = new MeshGraph();
  set_property( *p_mesh, graph_ppt, mg );
}


SurfaceMesh::SurfaceMesh( const SurfaceMesh& m ) :
  computed_vtx_norm( m.computed_vtx_norm ),
  p_mesh( m.p_mesh ),
  mesh_type( m.mesh_type )
{
  mg = new MeshGraph( *(m.mg) ) ;
  set_property( *p_mesh, graph_ppt, mg );
}


SurfaceMesh::~SurfaceMesh()
{}

void SurfaceMesh::declareOptions(OptionList& ol)
{
  declareOption(ol, "mesh_type", &SurfaceMesh::mesh_type,
                OptionBase::learntoption,
                "Type of mesh, describing the kind of information that is available.\n"
                "Supported options are:\n"
                "  - 'FaceSet': Vertices, Faces, and Edges;\n"
                "  - 'LineSet': Vertices and Edges;\n"
                "  - 'PointSet' (default): Vertices.\n" );

  declareOption(ol, "verbosity", &SurfaceMesh::verbosity,
                OptionBase::buildoption,
                "Level of verbosity. If 0 should not write anything on perr.\n"
                "If >0 may write some info on the steps performed"
                " along the way.\n"
                "The level of details written should depend on this value.\n"
               );

  inherited::declareOptions(ol);
}

void SurfaceMesh::build_()
{
//  build_vertex_normals();
  string mt = lowerstring( mesh_type );
  mt = space_to_underscore( mt );
  search_replace( mt, "_", "" );
  if( mt == "faceset" )
    mesh_type = "FaceSet";
  else if( mt == "lineset" )
    mesh_type = "LineSet";
  else if( mt == "pointset" )
    mesh_type = "PointSet";
  else
    PLERROR( "mesh_type '%s' not supported", mesh_type.c_str() );
}

void SurfaceMesh::build()
{
  inherited::build();
  build_();
}

void SurfaceMesh::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  Graph p_mesh2 = new Graph_( *p_mesh );

  property_map< graph, vertex_ppt_t >::type vertex_ppt_map =
    get( vertex_ppt, *p_mesh2 );

  map< vertex_descriptor, vertex_descriptor > vertex_corresp;

  vertex_iterator vi, vi_end;
  vertex_iterator vi2, vi2_end;
  tie( vi, vi_end ) = vertices( *p_mesh );
  tie( vi2, vi2_end ) = vertices( *p_mesh2 );
  for( ; vi != vi_end ; vi++, vi2++ )
  {
    vertex_ppt_map[ *vi2 ] = vertex_ppt_map[*vi2]->deepCopy(copies);
    vertex_corresp[ *vi ] = *vi2;
  }

  property_map< graph, edge_ppt_t >::type edge_ppt_map =
    get( edge_ppt, *p_mesh2 );

  edge_iterator ei2, ei2_end;
  for( tie(ei2,ei2_end) = edges( *p_mesh2 ) ; ei2 != ei2_end ; ei2++ )
  {
    edge_ppt_map[ *ei2 ] = edge_ppt_map[*ei2]->deepCopy(copies);
  }

  mg->makeDeepCopyFromShallowCopy( copies );
  int n_faces = numFaces();

  for( int i=0 ; i<n_faces ; i++ )
  {
    MFace face = getFace(i);
    for( int j=0 ; j<3 ; j++ )
    {
      face->pts[j] = vertex_corresp[ face->pts[j] ];
    }

    for( int j=0 ; j<3 ; j++ )
    {
      int jj = (j+1)%3; // jj == j+1 if j==0 or 1, jj==0 if j=3
      out_edge_iterator oei, oei_end;
      vertex_descriptor src = face->pts[j];
      vertex_descriptor tgt = face->pts[jj];
      for( tie(oei, oei_end)=out_edges(src, *p_mesh2) ; oei != oei_end ; oei++ )
      {
        if( target( *oei, *p_mesh2 ) == tgt )
        {
          face->edges[j] = *oei;
        }
      }
    }
  }

  p_mesh = p_mesh2;

}

vertex_descriptor SurfaceMesh::addVertex( MVertex mv )
{
  vertex_descriptor vtx = add_vertex( mv, *p_mesh );
  return vtx;
}

vertex_descriptor SurfaceMesh::addVertex( Vec coord, Vec features )
{
  MVertex mv = new MeshVertex();

  if( coord.length() != 3 )
    PLERROR("Error in addVertex(Vec coord): coord size is supposed to be 3 (or not to be!)");

  mv->coord << coord;

  mv->features.resize( features.size() );
  mv->features << features;

  vertex_descriptor vtx = addVertex( mv );
  return vtx;
}

vertex_descriptor SurfaceMesh::addVertex( real x, real y, real z )
{
  Vec coord( 3 );
  coord[0] = x;
  coord[1] = y;
  coord[2] = z;
  vertex_descriptor vtx = addVertex( coord );
  return vtx;
}

void SurfaceMesh::delVertex( vertex_descriptor vtx )
{
  remove_vertex( vtx, *p_mesh );
}

MVertex SurfaceMesh::getVertex( vertex_descriptor vtx )
{
  MVertex mv = get( vertex_ppt, *p_mesh, vtx );
  return mv;
}

void SurfaceMesh::setVertex( vertex_descriptor vtx, const MVertex& mv )
{
  put( vertex_ppt, *p_mesh, vtx, mv );
}


Mat SurfaceMesh::getVertexCoords()
{
  int n_pts = num_vertices( *p_mesh );
  Mat coords( n_pts, 3 );

  property_map< graph, vertex_ppt_t >::type
    vertex_ppt_map = get( vertex_ppt, *p_mesh );

  vertex_iterator vi, vi_end;
  tie( vi, vi_end ) = vertices( *p_mesh );
  for( int i=0 ; i<n_pts ; i++, vi++ )
  {
    coords(i) << vertex_ppt_map[ *vi ]->coord;
  }
  return coords;
}

Mat SurfaceMesh::getVertexCoordsAndFeatures()
{
  int n_pts = num_vertices( *p_mesh );
  Mat coords_and_features;

  property_map< graph, vertex_ppt_t >::type
    vertex_ppt_map = get( vertex_ppt, *p_mesh );

  vertex_iterator vi, vi_end;
  tie( vi, vi_end ) = vertices( *p_mesh );

  int feat_size = vertex_ppt_map[ *vi ]->features.size();
  coords_and_features.resize( n_pts, 3+feat_size );

  for( int i=0 ; i<n_pts ; i++, vi++ )
  {
    MVertex mv = vertex_ppt_map[ *vi ];
    coords_and_features(i).subVec(0,3) << mv->coord;
    coords_and_features(i).subVec(3,feat_size) << mv->features;
  }
  return coords_and_features;
}

void SurfaceMesh::setVertexCoords( const Mat& coords )
{
  int n_pts = num_vertices( *p_mesh );
  if( coords.length() != n_pts )
  {
    PLERROR( "setVertexCoords: coords.length()=%d different from n_pts=%d",
             coords.length(), n_pts );
  }
  else
  {
    property_map< graph, vertex_ppt_t >::type
      vertex_ppt_map = get( vertex_ppt, *p_mesh );

    vertex_iterator vi, vi_end;
    tie( vi, vi_end ) = vertices( *p_mesh );
    for( int i=0 ; i<n_pts ; i++, vi++ )
    {
      vertex_ppt_map[ *vi ]->coord << coords(i);
    }
  }
}

void SurfaceMesh::setVertexFeatures( const Mat& features )
{
  int n_pts = num_vertices( *p_mesh );
  if( features.length() != n_pts )
  {
    PLERROR( "setVertexFeatures: features.length()=%d different from n_pts=%d",
             features.length(), n_pts );
  }
  else
  {
    property_map< graph, vertex_ppt_t >::type
      vertex_ppt_map = get( vertex_ppt, *p_mesh );

    vertex_iterator vi, vi_end;
    tie( vi, vi_end ) = vertices( *p_mesh );
    for( int i=0 ; i<n_pts ; i++, vi++ )
    {
      vertex_ppt_map[ *vi ]->features << features(i);
    }
  }
}

void SurfaceMesh::setVertexCoordsAndFeatures( const Mat& coords,
                                              const Mat& features )
{
  int n_pts = num_vertices( *p_mesh );
  if( coords.length() != n_pts )
  {
    PLERROR( "setVertexCoordsAndFeatures:"
             " coords.length()=%d different from n_pts=%d",
             coords.length(), n_pts );
  }
  if( features.length() != n_pts )
  {
    PLERROR( "setVertexCoordsAndFeatures:"
             " features.length()=%d different from n_pts=%d",
             features.length(), n_pts );
  }
  else
  {
    property_map< graph, vertex_ppt_t >::type
      vertex_ppt_map = get( vertex_ppt, *p_mesh );

    vertex_iterator vi, vi_end;
    tie( vi, vi_end ) = vertices( *p_mesh );
    for( int i=0 ; i<n_pts ; i++, vi++ )
    {
      vertex_ppt_map[ *vi ]->coord << coords(i);
      vertex_ppt_map[ *vi ]->features << features(i);
    }
  }
}

int SurfaceMesh::numVertices()
{
  return( num_vertices(*p_mesh) );
}

TVec<vertex_descriptor> SurfaceMesh::getVertexDescriptors()
{
  int n_pts = numVertices();
  TVec<vertex_descriptor> result( n_pts );

  vertex_iterator vi, vi_end;
  tie( vi, vi_end ) = vertices( *p_mesh );
  for( int i=0 ; i<n_pts ; i++, vi++ )
  {
    result[i] = *vi;
  }
  return( result );
}


Mat SurfaceMesh::getVertexNorms()
{
  if( !computed_vtx_norm )
  {
    PLWARNING( "Trying to get vertex normals, but they\'re not initialized" );
  }
  int n_pts = num_vertices( *p_mesh );
  Mat norms( n_pts, 3 );

  property_map< graph, vertex_ppt_t >::type
    vertex_ppt_map = get( vertex_ppt, *p_mesh );

  vertex_iterator vi, vi_end;
  tie( vi, vi_end ) = vertices( *p_mesh );
  for( int i=0 ; i<n_pts ; i++, vi++ )
  {
    norms(i) << vertex_ppt_map[ *vi ]->norm;
  }
  return norms;
}

void SurfaceMesh::setVertexNorms( const Mat& norms )
{
  int n_pts = num_vertices( *p_mesh );
  if( norms.length() != n_pts )
  {
    PLERROR( "setVertexNorms: norms.length()=%d different from n_pts=%d",
             norms.length(), n_pts );
  }
  else
  {
    property_map< graph, vertex_ppt_t >::type
      vertex_ppt_map = get( vertex_ppt, *p_mesh );

    vertex_iterator vi, vi_end;
    tie( vi, vi_end ) = vertices( *p_mesh );
    for( int i=0 ; i<n_pts ; i++, vi++ )
    {
      vertex_ppt_map[ *vi ]->norm << norms(i);
    }
  }
}




edge_descriptor SurfaceMesh::addEdge( vertex_descriptor first,
                                      vertex_descriptor second,
                                      MEdge me )
{
  edge_descriptor ed = add_edge( first, second, me, *p_mesh ).first;
  return ed;
}

void SurfaceMesh::delEdge( edge_descriptor ed )
{
  PLERROR( "SurfaceMesh::delEdge() not implemented yet." );
}

MEdge SurfaceMesh::getEdge( edge_descriptor ed )
{
  MEdge me = get( edge_ppt, *p_mesh, ed );
  return me;
}

void  SurfaceMesh::setEdge( edge_descriptor ed, const MEdge& me )
{
  put( edge_ppt, *p_mesh, ed, me );
}

int SurfaceMesh::numEdges()
{
  return( num_edges(*p_mesh) );
}


int SurfaceMesh::addFace( MFace mf )
{
  mg->faces.append( mf );
  return( mg->faces.length() - 1 );
}

int SurfaceMesh::addFace( TVec<vertex_descriptor> pts )
{
  if( pts.length() != 3 )
    PLERROR("Error in addFace(TVec<...> pts): you can only create a triangle face.");

  MFace mf = new MeshFace();
  mf->pts << pts;
  int face_index = addFace( mf );
  return face_index;
}

int SurfaceMesh::addFace( vertex_descriptor first,
                          vertex_descriptor second,
                          vertex_descriptor third )
{
  TVec<vertex_descriptor> pts( 3 );
  pts[0] = first;
  pts[1] = second;
  pts[2] = third;

  int face_index = addFace( pts );
  return face_index;
}

void SurfaceMesh::delFace( int face_index)
{
  PLERROR( "SurfaceMesh::delFace() not implemented yet." );
}

MFace SurfaceMesh::getFace( int face_index )
{
  MFace mf = mg->faces[ face_index ];
  return mf;
}

void SurfaceMesh::setFace( int face_index, const MFace& mf )
{
  mg->faces[ face_index ] = mf;
}

int SurfaceMesh::numFaces()
{
  return mg->faces.length();
}


TVec< set<int> > SurfaceMesh::cacheNeighborFaces()
{
  int n_pts = numVertices();
  TVec< set<int> > face_cache( n_pts );

  vertex_iterator vi, vi_end;
  tie(vi, vi_end) = vertices( *p_mesh );
  for( int i=0 ; vi != vi_end ; i++, vi++ )
  {
    out_edge_iterator oei, oei_end;
    tie( oei, oei_end ) = out_edges( *vi, *p_mesh );
    for(  ; oei != oei_end ; oei++ )
    {
      MEdge me = getEdge( *oei );
      face_cache[i].insert( me->face1 );
      face_cache[i].insert( me->face2 );
    }
    face_cache[i].erase( -1 );
  }
  return( face_cache );
}


int SurfaceMesh::buildEdges()
{
  TVec<MFace> faces = mg->faces;

  property_map< graph, vertex_ppt_t >::type
    vertex_ppt_map = get( vertex_ppt, *p_mesh );

  property_map< graph, edge_ppt_t >::type
    edge_ppt_map = get( edge_ppt, *p_mesh );

  int n_faces = faces.size();
  int nerrors = 0;

  for( int i=0 ; i<n_faces ; i++ )
  {
    MFace mf = faces[i];
    TVec< vertex_descriptor > pts = mf->pts;

    /* we build 3 edges from each face */
    for( int j=0 ; j<3 ; j++ )
    {
      MEdge me = new MeshEdge();
      me->face1 = i;
      me->bf = 1;

      int jj = (j+1)%3; // jj == j+1 if j==0 or 1, jj==0 if j=3
      me->length = dist( vertex_ppt_map[ pts[j] ]->coord,
                         vertex_ppt_map[ pts[jj] ]->coord,
                         2 );

      edge_descriptor ed;
      bool added;

      /* ed is the descriptor of the edge linking pts[j] and pts[jj],
         added is "true" if it didn't exist and was just created.
         In this last case, the "MeshEdge" property was added too,
         i.e. face1 is set to current face,
         and the edge is considered as a "boundary" (bf=1) */
      tie( ed, added ) = add_edge_ifnew( pts[j], pts[jj], me, *p_mesh );

      mf->edges[j] = ed;

      /* if the edge was not added (it already existed)... */
      if( !added )
      {
        /* either there is no "face2" yet : mf is the other face,
           which is adjacent to "face1",
           and current edge isn't a border any more */
        if( edge_ppt_map[ed]->face2<0 )
        {
          edge_ppt_map[ed]->face2 = i;
          int k = edge_ppt_map[ed]->face1;

          mf->adj_faces[j] = k;
          MFace neighbor_f = faces[k];
          for( int l=0 ; l<3 ; l++ )
          {
            if( neighbor_f->edges[l] == ed )
            {
              neighbor_f->adj_faces[l] = i;
            }
          }

          edge_ppt_map[ed]->bf = 0;
        }
        else
        {
          vertex_descriptor src = source( ed, *p_mesh );
          vertex_descriptor tgt = target( ed, *p_mesh );
          cerr << "Edge between " << src << " and " << tgt 
            << " has more than 2 faces." << endl;

          nerrors++;
        }
      }
    }
  }
  return( nerrors );
}

int SurfaceMesh::buildBoundaries()
{
  int nerrors = 0;
  property_map< graph, vertex_ppt_t >::type
    vertex_ppt_map = get( vertex_ppt, *p_mesh );

  property_map< graph, edge_ppt_t >::type
    edge_ppt_map = get( edge_ppt, *p_mesh );

  /* we mark as "boundary" vertices all vertices belonging to
     at least a "boundary" edge */
  edge_iterator ei, ei_end;
  for( tie(ei,ei_end)=edges( *p_mesh ) ; ei != ei_end ; ei++ )
  {
    if( edge_ppt_map[ *ei ]->bf == 1 )
    {
      vertex_ppt_map[ source( *ei, *p_mesh ) ]->bf = 1;
    }
  }

  /* we check that a "boundary" vertex belongs to, at least,
     two boundary edges (otherwise, it wouldn't make any sense) */
  vertex_iterator vi, vi_end;
  for( tie(vi,vi_end)=vertices( *p_mesh ) ; vi != vi_end ; vi++ )
  {
    if( vertex_ppt_map[ *vi ]->bf )
    {
      int bound_count = 0;
      adjacency_iterator ai, ai_end;
      for( tie(ai,ai_end)=adjacent_vertices( *vi, *p_mesh ) ; ai!=ai_end ; ai++ )
      {
        if( vertex_ppt_map[ *ai ]->bf )
        { bound_count++; }
      }

      if( bound_count < 2 )
      {
        cerr << "Wrong number of points on adjacent boundary to point "
          << *vi << endl;
        nerrors++;
      }
    }
  }
  return nerrors;
}

real SurfaceMesh::getResolution()
{
  return mg->resolution;
}


void SurfaceMesh::setResolution( real resolution )
{
  mg->resolution = resolution;
}


real SurfaceMesh::computeResolution()
{
  /* we compute the mesh resolution :
     it is the median of the vertices' length */
  int n_edges = num_edges( *p_mesh );

  property_map< graph, edge_ppt_t >::type
    edge_ppt_map = get( edge_ppt, *p_mesh );

  Vec lengths( n_edges );
  edge_iterator ei, ei_end;
  tie(ei,ei_end)=edges( *p_mesh );
  for( int i=0 ; i<n_edges ; i++ )
  {
    lengths[i] = edge_ppt_map[*ei]->length;
    ei++;
  }

  real resolution = median( lengths );
  return( resolution );
}

/*
 * Functions to read a mesh from a VRML file, and write to it.
 * protected functions (names ending with a '_') are helper functions,
 * public functions should be used (and added if needed).
 *
 */

/* Protected functions */

// reads coordinates of the points in the "in" stream,
// add them to the mesh, and stores correspondance between indices
// in the file and vertex descriptors (in "vertices")
bool SurfaceMesh::readVRMLCoordinate3_( ifstream& in,
                                        TVec<vertex_descriptor>& vertices,
                                        Mat vtx_features )
{
  string buf;

  /* read from the file until "Coordinate3" is reached */
  while( buf.compare( 0, 11, "Coordinate3" ) && in )
  {
    in >> buf;
  }

  /* read characters until the open bracket */
  getline( in, buf, '[' );

  /* reading coordinates until close bracket */
  getline( in, buf, ']' );
  istringstream all_coords( buf );

  /* read coordinates by comma-separated triplets */
  while( all_coords )
  {
    Vec coord( 3 );
    string coords;
    getline( all_coords, coords );
    coords = removeblanks( coords );
    remove_comments( coords );
    int vertex_index = 0;
    if( coords != "" )
    {
      istringstream point_coords( coords );

      if ( point_coords >>  coord[0] >> coord[1] >> coord[2] )
      {
        vertex_descriptor vtx;
        if( vtx_features.size() == 0 )
          vtx = addVertex( coord );
        else
          vtx = addVertex( coord, vtx_features(vertex_index) );

        /* store the correspondence between order in VRML file
           and vertex descriptor */
        vertices.append( vtx );
        vertex_index++;
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

// reads indices of faces' vertices in the "in" stream,
// add them to the mesh, with the help of the "vertices" correspondence
bool SurfaceMesh::readVRMLIndexedFaceSet_( ifstream& in,
                                           TVec<vertex_descriptor>& vertices )
{
  string buf;

  /* read from file until "IndexedFaceSet" is reached */
  while( buf.compare( 0, 14, "IndexedFaceSet" ) )
  {
    if( !in )
      return false;

    in >> buf;
  }

  /* read characters until the open bracket */
  getline( in, buf, '[' );

  /* reading coordinates until close bracket */
  getline( in, buf, ']' );
  istringstream all_faces( buf );
//  bool end_list = false;

  while( all_faces )
  {
    TVec<int> pts(3);
    int delim;
    string face;
    getline( all_faces, face );
    face = removeblanks( face );
    remove_comments( face );
    if( face != "" )
    {
      istringstream point_indices( face );
//      if( point_indices >> pts[0] >> pts[1] >> pts[2] )
      bool ok = true;
      for( int i=0 ; i<3 ; i++ )
      {
        string pt;
        getline( point_indices, pt, ',' );
        istringstream point( pt );
        ok = (point >> pts[i]) && ok;
      }

      if( ok )
      {
        if( (point_indices >> delim) && (delim != -1) )
          PLERROR( "Non-triangular face found in 'IndexedFaceSet'.\n" );

        addFace( vertices[ pts[0] ], vertices[ pts[1] ], vertices[ pts[2] ] );
      }
      else
      {
        PLERROR( "Parse error in 'IndexedFaceSet': expecting an integer.\n" );
      }
    }
  }

  int nerrors = createSurfaceMesh();
//  delOrphanVertices();

  if( nerrors )
  { return 0; }
  else
  { return 1; }

}

// reads indices of edges' vertices in the "in" stream,
// add them to the mesh, with the help of the "vertices" correspondence
bool SurfaceMesh::readVRMLIndexedLineSet_( ifstream& in,
                                           TVec<vertex_descriptor>& vertices )
{
  string buf;

  /* read from file until "IndexedLineSet" is reached */
  while( buf.compare( 0, 14, "IndexedLineSet" ) )
  {
    if( !in )
      return false;

    in >> buf;
  }

  /* read characters until the open bracket */
  getline( in, buf, '[' );

  /* reading coordinates until close bracket */
  getline( in, buf, ']' );
  istringstream all_edges( buf );

  while( all_edges )
  {
    TVec<int> pts(2);
    int delim;
    string edge;

    getline( all_edges, edge );
    edge = removeblanks( edge );
    remove_comments( edge );
    if( edge != "" )
    {
      istringstream point_indices( edge );
      bool ok = true;
      for( int i=0 ; i<2 ; i++ )
      {
        string pt;
        getline( point_indices, pt, ',' );
        istringstream point( pt );
        ok = (point >> pts[i]) && ok;
      }

      if( ok )
      {
        if( (point_indices >> delim) && (delim != -1) )
          PLERROR( "Edge of more than 2 points found in 'IndexedLineSet'.\n" );

        MEdge me = new MeshEdge();
        addEdge( vertices[ pts[0] ], vertices[ pts [1] ], me );
      }
      else
      {
        PLERROR( "Parse error in 'IndexedLineSet': expecting an integer.\n" );
      }
    }
  }

  return true;
}


// writes the "Coordinate3" VRML node in "out" file stream,
// containing coordinates of the vertices, and store correspondance
// between vertex descriptors and indices in the file (in "pts_indices").
void SurfaceMesh::writeVRMLCoordinate3_(
  ofstream& out,
  map< vertex_descriptor, int >& pts_indices )
{
  out << "  Coordinate3 {\n"
      << "    point [  #line format:\n"
      << "#     point coordinates"
      << " # point index"
      << " # normal vector"
      << " # boundary flag"
      << "\n"
      ;

  vertex_iterator vi, vi_end;
  int vtx_counter=0;
  for( tie( vi, vi_end )=vertices( *p_mesh ) ; vi != vi_end ; vi++ )
  {
    MVertex mv = get( vertex_ppt, *p_mesh, *vi );
    Vec coord = mv->coord;
//    property_map< graph, vertex_index_t >::type pts_indices;
    Vec norm = mv->norm;

    pts_indices[ *vi ] = vtx_counter;
    vtx_counter++ ;

    out << "      "
      << coord[0] << " " << coord[1] << " " << coord[2] << ","
      /* comments */
        // point number
      << " # " << pts_indices[ *vi ]
        // normal vector
      << " # " << norm[0] << " " << norm[1] << " "<< norm[2]
        // boundary flag
      << " # " << mv->bf
      << "\n";

  }

  out << "    ]\n"
      << "  }\n\n";
}

// writes the "IndexedFaceSet" VRML node in "out" file stream,
// with the help of the "pts_indices" correspondance table
void SurfaceMesh::writeVRMLIndexedFaceSet_(
  ofstream& out,
  map< vertex_descriptor, int >& pts_indices )
{
  out << "  IndexedFaceSet {\n"
    << "    coordIndex [  # line format:\n"
    << "#     indices of the points, -1 (end of face)"
    << " # face index"
    << " # adjacent faces' indices"
//    << " # face normal"
    << " # edges' boundary flags"
    << "\n";

  TVec< MFace > faces = mg->faces;
  for( int i=0 ; i < faces.length() ; i++ )
  {
    MFace face = faces[i];
    TVec< vertex_descriptor > pts = face->pts;


    TVec<int> adj_faces = face->adj_faces;
    Vec face_norm = face->face_norm;

    TVec< edge_descriptor > edges = face->edges;

    out << "      "
      << pts_indices[ pts[0] ] << ", "
      << pts_indices[ pts[1] ] << ", "
      << pts_indices[ pts[2] ] << ", -1,"
      /* comments */
        // index
      << " # " << i
        // index of adjacent faces
      << " # " << adj_faces[0] << " " << adj_faces[1] << " " << adj_faces[2]
        // face normal
      /* not calculated yet
      << "# " << face_norm[0] << " " << face_norm[0] << " " << face_norm[0]
       */
        // edges' boundary flags
      << " # " << getEdge( edges[0] )->bf << " "
               << getEdge( edges[1] )->bf << " "
               << getEdge( edges[2] )->bf
      << "\n";
  }

  out << "    ]\n"
    << "  }\n";
}

// writes the "IndexedFaceSet" VRML node in "out" file stream,
// with the help of the "pts_indices" correspondance table
void SurfaceMesh::writeVRMLIndexedLineSet_(
  ofstream& out,
  map< vertex_descriptor, int >& pts_indices )
{
  out << "  IndexedLineSet {\n"
    << "    coordIndex [ # line format:\n"
    << "#     indices of the points, -1 (end of edge)"
    << "\n";

  edge_iterator ei, ei_end;
  for( tie( ei, ei_end )=edges( *p_mesh ) ; ei != ei_end ; ei ++ )
  {
    vertex_descriptor src = source( *ei, *p_mesh );
    vertex_descriptor tgt = target( *ei, *p_mesh );
    out << "      "
      << pts_indices[ src ] <<", "<< pts_indices[ tgt ]
      << ", -1,\n";
  }

  out << "    ]\n"
    << "  }\n";
}

/* Public functions */

// reads from a VRML file, getting at least point coordinates,
// if possible face data (from "IndexedFaceSet" node),
// else if possible edge date (from "IndexedLineSet" node)
bool SurfaceMesh::readVRMLFile( string filename, Mat vtx_features )
{
  ifstream in( filename.c_str() );
  if( !in )
    PLERROR( "Cannot open file: %s.\n", filename.c_str() );

  TVec<vertex_descriptor> vertices;
  if( !readVRMLCoordinate3_( in, vertices, vtx_features ) )
  {
    PLERROR( "Parse error in %s: expecting a floating-point number.\n",
             filename.c_str() );
  }

  ifstream in_( filename.c_str() );
  if( !in_ )
    PLERROR( "Cannot open file: %s.\n", filename.c_str() );

  string buf;

  // check if a "IndexedFaceSet" node is present
  bool face_set = false;
  bool line_set = false;

  while( in_ >> buf )
  {
    if( !buf.compare( 0, 14, "IndexedFaceSet" ) )
      face_set = true;

    if( !buf.compare( 0, 14, "IndexedLineSet" ) )
      line_set = true;
  }

  if( face_set && readVRMLIndexedFaceSet_( in, vertices ) )
    mesh_type = "FaceSet";
  else if( line_set && readVRMLIndexedLineSet_( in, vertices ) )
    mesh_type = "LineSet";
  else
    mesh_type = "PointSet";

  return true;
}

// reads from a VRML file, getting point coordinates, and
// face data from "IndexedFaceSet" node
bool SurfaceMesh::readVRMLIndexedFaceSet( string filename, Mat vtx_features )
{
  if( verbosity >= 1 )
    pout << "\nreadVRMLIndexedFaceSet " << filename << endl;

  ifstream in( filename.c_str() );
  if( !in )
  {
    PLERROR( "Cannot open file: %s.\n", filename.c_str() );
  }

  TVec<vertex_descriptor> vertices;
  if( !readVRMLCoordinate3_( in, vertices, vtx_features ) )
  {
    PLERROR( "Parse error in %s: expecting a floating-point number.\n",
             filename.c_str() );
  }

  if( !readVRMLIndexedFaceSet_( in, vertices ) )
  {
    PLERROR( "Error while parsing 'IndexedFaceSet' in file '%s'.\n",
             filename.c_str() );
  }

  mesh_type = "FaceSet";
  return true;
}

// reads from a VRML file, getting point coordinates, and
// edge data from "IndexedLineSet" node
bool SurfaceMesh::readVRMLIndexedLineSet( string filename, Mat vtx_features )
{
  // probably to be rewritten using PPath
  if( verbosity >= 1 )
    pout << "\nreadVRMLIndexedLineSet " << filename << endl;

  ifstream in( filename.c_str() );
  if( !in )
  {
    PLERROR( "Cannot open file: %s.\n", filename.c_str() );
  }

  TVec<vertex_descriptor> vertices;
  if( !readVRMLCoordinate3_( in, vertices, vtx_features ) )
  {
    PLERROR( "Parse error in %s: expecting a floating-point number.\n",
             filename.c_str() );
  }

  if( !readVRMLIndexedLineSet_( in, vertices ) )
  {
    PLERROR( "Error while parsing 'IndexedLineSet' in file '%s'.\n",
             filename.c_str() );
  }

  mesh_type = "LineSet";
  return true;
}


// writes mesh as a VRML file, using a "Coordinate3" node in any case,
// an "IndexedFaceSet" node if face data is present,
// else an "IndexedLineSet" node if edge data is present.
void SurfaceMesh::writeVRMLFile( string filename )
{
  if( verbosity >= 1 )
    pout << "\nwriteVRMLFile " << filename << endl;

  ofstream out( filename.c_str(), ios::trunc );
  if( !out )
  {
    PLERROR( "Cannot open file: %s.\n", filename.c_str() );
  }

  /* map used to remember points' index in the vrml file */
  map< vertex_descriptor, int > pts_indices;

  out << "#VRML V1.0 ascii\n\n"
      << "Separator {\n"
      << "  Material {\n"
      << "    diffuseColor [ 1 1 1 ]\n"
      << "  }\n\n";

  writeVRMLCoordinate3_( out, pts_indices );

  if( mesh_type == "FaceSet" )
    writeVRMLIndexedFaceSet_( out, pts_indices );
  else if( mesh_type == "LineSet" )
    writeVRMLIndexedLineSet_( out, pts_indices );

  out << "}\n";
}

// writes mesh as a VRML file,
// using an "IndexedFaceSet" to store face information
void SurfaceMesh::writeVRMLIndexedFaceSet( string filename )
{
  if( verbosity >= 1 )
    pout << "\nwriteVRMLIndexedFaceSet " << filename << endl;

  ofstream out( filename.c_str(), ios::trunc );
  if( !out )
  {
    PLERROR( "Cannot open file: %s.\n", filename.c_str() );
  }

  /* map used to remember points' index in the vrml file */
  map< vertex_descriptor, int > pts_indices;


  out << "#VRML V1.0 ascii\n\n"
    << "Separator {\n"
    << "  Material {\n"
    << "    diffuseColor [ 1 1 1 ]\n"
    << "  }\n\n"
    ;

  writeVRMLCoordinate3_( out, pts_indices );

  writeVRMLIndexedFaceSet_( out, pts_indices );

  out << "}\n";
}

// writes mesh as a VRML file,
// using an "IndexedLineSet" to store edge information,
// not storing face information if any
void SurfaceMesh::writeVRMLIndexedLineSet( string filename )
{
  if( verbosity >= 1 )
    pout << "\nwriteVRMLIndexedLineSet " << filename << endl;

  ofstream out( filename.c_str(), ios::trunc );
  if( !out )
  {
    PLERROR( "Cannot open file: %s.\n", filename.c_str() );
  }

  /* map used to remember points' index in the vrml file */
  map< vertex_descriptor, int > pts_indices;

  out << "#VRML V1.0 ascii\n\n"
      << "Separator {\n"
      << "  Material {\n"
      << "    diffuseColor [ 1 1 1 ]\n"
      << "  }\n\n"
    ;

  writeVRMLCoordinate3_( out, pts_indices );

  writeVRMLIndexedLineSet_( out, pts_indices );

  out << "}\n";
}

// writes edges having a boundary flag in a VRML file
void SurfaceMesh::writeVRMLBoundaries( string filename )
{
  if( verbosity >= 1 )
    pout << "\nwriteVRMLBoundaries " << filename << endl;

  ofstream out( filename.c_str(), ios::trunc );
  if( !out )
  {
    PLERROR( "Cannot open file: %s.\n", filename.c_str() );
  }

  /* map used to remember points' index in the vrml file */
  map< vertex_descriptor, int > pts_indices;

  out << "#VRML V1.0 ascii\n\n"
      << "Separator {\n"
      << "  Material {\n"
      << "    diffuseColor [ 1 1 1 ]\n"
      << "  }\n\n";

  writeVRMLCoordinate3_( out, pts_indices );

  out << "  IndexedLineSet {\n"
      << "    coordIndex [  # line format:\n"
/*    << "#     indices of the points, -1 (end of face)"
      << " # face index"
      << " # adjacent faces' indices"
//      << " # face normal"
      << " # edges' boundary flags"
*/    << "\n";

  TVec< MFace > faces = mg->faces;
  for( int i=0 ; i < faces.length() ; i++ )
  {
    MFace face = faces[i];
    TVec< vertex_descriptor > pts = face->pts;


    TVec<int> adj_faces = face->adj_faces;
    Vec face_norm = face->face_norm;

    TVec< edge_descriptor > edges = face->edges;

    if( getEdge( edges[0] )->bf == 1 )
    {
      out << pts_indices[ pts[0] ] <<", "<< pts_indices[ pts[1] ] << ", -1,\n";
    }
    if( getEdge( edges[1] )->bf == 1 )
    {
      out << pts_indices[ pts[1] ] <<", "<< pts_indices[ pts[2] ] << ", -1,\n";
    }
    if( getEdge( edges[2] )->bf == 1 )
    {
      out << pts_indices[ pts[2] ] <<", "<< pts_indices[ pts[0] ] << ", -1,\n";
    }
  }

  out << "    ]\n"
      << "  }\n"
      << "}\n";
}


/* fills all the vertex, edge and faces properties,
   according to the basic properties we already have.
   should probably be in a "build_" method. */
int SurfaceMesh::createSurfaceMesh()
{
  int nerrors = 0;

  /* initialisation from the faces */

  /* create edges and setting their length and boundary flag */
  nerrors += buildEdges();

  /* set boundary flags to appropriate vertices */
  nerrors += buildBoundaries();

  /* compute and set mesh resolution */
  mg->resolution = computeResolution();

  findNormals();

  mg->size = averageSphereRadius();

  return nerrors;
}


int SurfaceMesh::delOrphanVertices()
{
  int number_deleted = 0;
  vertex_iterator vi, vi_end, next;
  tie( vi, vi_end ) = vertices( *p_mesh );
  for (next = vi; vi != vi_end; vi = next)
  {
    ++next;
    if( out_degree( *vi, *p_mesh ) == 0 )
    {
      number_deleted++;
      delVertex( *vi );
    }
  }
  return number_deleted;
}


/*
 ******************************************************************************
 + FUNCTION: FindNormals
 + AUTHOR:   Andrew E. Johnson (aej@ri.cmu.edu)
 + DATE:     6-Jul-94
 + PURPOSE:  Calculate surface normal for each meshpoint in graph by finding
 +           smallest eigenvector of inertial matrix made from the meshpoint's
 +           neighbors. This vector corresponds to the smallest principal
 +           direction.  It also orients all of the mesh surface normals in
 +           the same direction as their neighbors using relaxation
 ******************************************************************************
 */

void SurfaceMesh::findNormals()
{
  // calculate initial normals

  vertex_iterator vi, vi_end;
  for( tie( vi, vi_end )=vertices( *p_mesh ) ; vi != vi_end ; vi++ )
  {
    Vec v(3);
    real fit_error = calcNormal( *p_mesh, *vi, v );

    MVertex mv = get( vertex_ppt, *p_mesh, *vi );
    mv->error = fit_error;

    /* orient the normal based on the orientation
       of the faces adjacent to the point */

    /* find adjacent faces */
    set<int> adj_faces;
    out_edge_iterator ei, ei_end;
    for( tie(ei,ei_end)=out_edges(*vi, *p_mesh) ; ei!=ei_end ; ei++ )
    {
      adj_faces.insert( get(edge_ppt,*p_mesh,*ei)->face1 );
      adj_faces.insert( get(edge_ppt,*p_mesh,*ei)->face2 );
    }
    adj_faces.erase( -1 ); // we don't want to keep "dummy" faces

    int n_opposite = 0;
    int n_same = 0;

    /* determine number of faces with similar normal
       and number of faces with opposite pointing normal */

    for( set<int>::iterator it=adj_faces.begin() ; it!=adj_faces.end() ; it++ )
    {
      MFace mf = get_property( *p_mesh, graph_ppt )->faces[ *it ];

      Vec p0 = get( vertex_ppt, *p_mesh, mf->pts[0] )->coord;
      Vec p1 = get( vertex_ppt, *p_mesh, mf->pts[1] )->coord;
      Vec p2 = get( vertex_ppt, *p_mesh, mf->pts[2] )->coord;

      Vec n( 3 );
      n = cross( p2-p1, p0-p1 );

      if( dot(n,v) > 0 )
      { n_same++ ;}
      else
      { n_opposite++ ;}
    }

    /* if the number opposite is greater than the number the same,
       then flip the normal */

    mv->norm.resize(3);
    if( n_same >= n_opposite )
    { mv->norm << v ; }
    else
    { mv->norm << -v ; }

    /* write vertex property value in the mesh */
    put( vertex_ppt, *p_mesh, *vi, mv );
  }

  computed_vtx_norm = true;
}


real SurfaceMesh::averageSphereRadius()
{
  Vec c( 3 );
  vertex_iterator vi, vi_end;
  for( tie(vi,vi_end)=vertices(*p_mesh) ; vi!=vi_end ; vi++ )
  {
    c += get( vertex_ppt, *p_mesh, *vi )->coord;
  }

  c *= real(1.0)/num_vertices( *p_mesh );

  real r = 0;
  for( tie(vi,vi_end)=vertices(*p_mesh) ; vi!=vi_end ; vi++ )
  {
    r += norm( c - get( vertex_ppt, *p_mesh, *vi )->coord, 2 );
  }

  r *= real(1.0)/num_vertices( *p_mesh );
  return r;
}


Mat SurfaceMesh::boundingBox()
{
  Mat box( 2, 3 );
  Vec mins(3);
  Vec max(3);

  Mat coords = getVertexCoords();
  columnMin( coords, mins );
  columnMax( coords, max );
  box(0) << mins;
  box(1) << max;

  return( box );
}




pair<edge_descriptor,bool> add_edge_ifnew( const vertex_descriptor& u,
                                           const vertex_descriptor& v,
                                           const graph::edge_property_type& ep,
                                           graph& g )
{
  out_edge_iterator ei, ei_end;
  for( tie(ei,ei_end)=out_edges(u,g) ; ei != ei_end ; ei++ )
  {
    if( target( *ei, g ) == v )
    {
      pair< edge_descriptor, bool > result( *ei, false );
      return result;
    }
  }
  pair< edge_descriptor, bool > result = add_edge( u, v, ep, g );
  return result;
}

pair<edge_descriptor,bool> add_edge_ifnew(const vertex_descriptor& u,
                                          const vertex_descriptor& v,
                                          graph& g )
{
  out_edge_iterator ei, ei_end;
  for( tie(ei,ei_end)=out_edges(u,g) ; ei != ei_end ; ei++ )
  {
    if( target( *ei, g ) == v )
    {
      pair< edge_descriptor, bool > result( *ei, false );
      return result;
    }
  }
  pair< edge_descriptor, bool > result = add_edge( u, v, g );
  return result;
}












} // end of namespace PLearn
