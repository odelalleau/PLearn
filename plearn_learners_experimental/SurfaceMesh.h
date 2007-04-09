// -*- C++ -*-

// SurfaceMesh.h
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
   * $Id: SurfaceMesh.h,v 1.13 2005/12/29 13:44:35 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file SurfaceMesh.h */


#ifndef SurfaceMesh_INC
#define SurfaceMesh_INC

#include <plearn/base/stringutils.h>
#include <plearn/base/Object.h>
#include "mesh_decl.h"
#include "MeshFace.h"


namespace PLearn {
using namespace std;

class SurfaceMesh;
typedef PP<SurfaceMesh> SurfMesh;


class SurfaceMesh: public Object
{

private:
  
  typedef Object inherited;

protected:
  // *********************
  // * protected options *
  // *********************

  MGraph mg;
  bool computed_vtx_norm;

public:

  // ************************
  // * public build options *
  // ************************

  Graph p_mesh;
  string mesh_type;

  //! Level of verbosity. If 0 should not write anything on perr.
  //! If >0 may write some info on the steps performed along the way.
  //! The level of details written should depend on this value.
  int verbosity;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  SurfaceMesh();

  //! Constructor from a graph.
  SurfaceMesh( Graph g );

  //! Copy constructor.
  SurfaceMesh( const SurfaceMesh& m );


  //! Virtual destructor.
  virtual ~SurfaceMesh();


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:
  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(SurfaceMesh);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  // (PLEASE IMPLEMENT IN .cc)
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
  virtual bool readVRMLCoordinate3_( ifstream& in,
                                    TVec<vertex_descriptor>& vertices,
                                    Mat vtx_features = Mat() );
  virtual bool readVRMLIndexedFaceSet_( ifstream& in,
                                        TVec<vertex_descriptor>& vertices );
  virtual bool readVRMLIndexedLineSet_( ifstream& in,
                                        TVec<vertex_descriptor>& vertices );

  virtual void writeVRMLCoordinate3_(
    ofstream& out,
    map< vertex_descriptor, int >& pts_indices );
  virtual void writeVRMLIndexedFaceSet_(
    ofstream& out,
    map< vertex_descriptor, int >& pts_indices );
  virtual void writeVRMLIndexedLineSet_(
    ofstream& out,
    map< vertex_descriptor, int >& pts_indices );

public:
  virtual bool readVRMLFile( string filename, Mat vtx_features = Mat() );
  virtual bool readVRMLIndexedFaceSet( string filename,
                                       Mat vtx_features = Mat() );
  virtual bool readVRMLIndexedLineSet( string filename,
                                       Mat vtx_features = Mat() );

  virtual void writeVRMLFile( string filename );
  virtual void writeVRMLIndexedFaceSet( string filename );
  virtual void writeVRMLIndexedLineSet( string filename );
  virtual void writeVRMLBoundaries( string filename );

  virtual int createSurfaceMesh(); // to fix

  virtual vertex_descriptor addVertex( MVertex mv );
  virtual vertex_descriptor addVertex( Vec coord, Vec features = Vec() );
  virtual vertex_descriptor addVertex( real x, real y, real z );

  virtual void delVertex( vertex_descriptor vtx );
  virtual MVertex getVertex( vertex_descriptor vtx );
  virtual void setVertex( vertex_descriptor vtx, const MVertex& mv );

  virtual Mat getVertexCoords();
  virtual Mat getVertexCoordsAndFeatures();
  virtual void setVertexCoords( const Mat& coords );
  virtual void setVertexFeatures( const Mat& features );
  virtual void setVertexCoordsAndFeatures( const Mat& coords,
                                           const Mat& features );
  virtual int numVertices();
  virtual TVec< vertex_descriptor > getVertexDescriptors();

  virtual Mat getVertexNorms();
  virtual void setVertexNorms( const Mat& coords );

  virtual edge_descriptor addEdge( vertex_descriptor first,
                                   vertex_descriptor second,
                                   MEdge me = new MeshEdge() );

  virtual void delEdge( edge_descriptor ed );
  virtual MEdge getEdge( edge_descriptor ed );
  virtual void setEdge( edge_descriptor ed, const MEdge& me );
  virtual int numEdges();

  virtual int addFace( MFace mf );
  virtual int addFace( TVec<vertex_descriptor> pts );
  virtual int addFace( vertex_descriptor first,
                       vertex_descriptor second,
                       vertex_descriptor third );

  virtual void delFace( int face_index );
  virtual MFace getFace( int face_index );
  virtual void setFace( int face_index, const MFace& mf );
  virtual int numFaces();
  virtual TVec< set<int> > cacheNeighborFaces();

  virtual int buildEdges();
  virtual int buildBoundaries();

  virtual real getResolution();
  virtual void setResolution( real resolution );
  virtual real computeResolution();

  virtual void findNormals();
  virtual real averageSphereRadius();
  virtual int delOrphanVertices();

  virtual Mat boundingBox();


  // ******************
  // * Static methods *
  // ******************

//  static int compare( const SurfaceMesh& e, const SurfaceMesh& f );

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SurfaceMesh);

std::pair< edge_descriptor, bool >
  add_edge_ifnew( const vertex_descriptor& u, const vertex_descriptor& v,
                  const graph::edge_property_type& ep, graph& g );

std::pair< edge_descriptor, bool >
  add_edge_ifnew( const vertex_descriptor& u, const vertex_descriptor& v,
                  graph& g );

} // end of namespace PLearn

#endif
