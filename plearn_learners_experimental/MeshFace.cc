// -*- C++ -*-

// MeshFace.cc
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
   * $Id: MeshFace.cc,v 1.5 2004/11/17 01:42:02 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file MeshFace.cc */


#include "MeshFace.h"

namespace PLearn {
using namespace std;

MeshFace::MeshFace() :
//  id( 0 ),
  pts( 3 ),
  edges( 3 ),
  adj_faces( 3, -1 ),
  face_norm( 3 )
{}

/*
MeshFace::MeshFace( const MeshFace& f ) :
  pts( 3 ),
  edges( 3 ),
  adj_faces( 3 ),
  face_norm( 3 )
{
//  id = f.id;
  pts << f.pts;
  edges << f.edges;
  adj_faces << f.adj_faces;
  face_norm << f.face_norm;
}
*/

PLEARN_IMPLEMENT_OBJECT(MeshFace,
    "Face element of a SurfaceMesh",
    ""
);

void MeshFace::declareOptions(OptionList& ol)
{
/*  declareOption(ol, "id", &MeshFace::id, OptionBase::buildoption,
                "face identifier");

  declareOption(ol, "pts", &MeshFace::pts, OptionBase::buildoption,
                "identifier of the 3 adjacent points");

  declareOption(ol, "edges", &MeshFace::edges, OptionBase::buildoption,
                "identifier of the 3 adjacent edges");
*/
  declareOption(ol, "adj_faces", &MeshFace::adj_faces, OptionBase::buildoption,
                "identifier of the 3 adjacent faces");

  declareOption(ol, "face_norm", &MeshFace::face_norm, OptionBase::buildoption,
                "face normal");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void MeshFace::build_()
{}

void MeshFace::build()
{
  inherited::build();
  build_();
}

void MeshFace::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  /* "`void*' is not a pointer-to-object type" */
//  deepCopyField(pts, copies);
  pts = pts.copy();

//  deepCopyField(edges, copies);
  edges = edges.copy();

  deepCopyField(adj_faces, copies);
  deepCopyField(face_norm, copies);
}

/*
MeshFace& MeshFace::operator=( const MeshFace& f )
{
//  id = f.id;
  pts << f.pts;
  edges << f.edges;
  adj_faces << f.adj_faces;
  face_norm << f.face_norm;
  return *this;
}
*/

/*
int MeshFace::compare( const MeshFace& e, const MeshFace& f )
{
  return( e.id - f.id );
}
*/

} // end of namespace PLearn
