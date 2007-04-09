// -*- C++ -*-

// MeshGraph.cc
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
   * $Id: MeshGraph.cc,v 1.3 2004/11/11 19:54:14 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file MeshGraph.cc */


#include "MeshGraph.h"

namespace PLearn {
using namespace std;

MeshGraph::MeshGraph() :
  id( 0 ),
  resolution( 0 ),
  size( 0 )
{}

/*
MeshGraph::MeshGraph( const MeshGraph& g ) :
  id( g.id ),
  resolution( g.resolution ),
  size( g.size ),
  faces( g.faces ),
  face_norm( g.face_norm )
{}
*/

PLEARN_IMPLEMENT_OBJECT(MeshGraph,
    "Graph properties of a SurfaceMesh",
    ""
);

void MeshGraph::declareOptions(OptionList& ol)
{
  declareOption(ol, "id", &MeshGraph::id, OptionBase::buildoption,
                "Graph identifier");

  declareOption(ol, "resolution", &MeshGraph::resolution,
                OptionBase::buildoption,
                "mesh resolution");

  declareOption(ol, "size", &MeshGraph::size, OptionBase::buildoption,
                "mesh size");

  declareOption(ol, "faces", &MeshGraph::faces, OptionBase::buildoption,
                "MeshFaces of the mesh");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void MeshGraph::build_()
{}

void MeshGraph::build()
{
  inherited::build();
  build_();
}

void MeshGraph::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(faces, copies);
}

/*
MeshGraph& MeshGraph::operator=( const MeshGraph& g )
{
  id = g.id;
  resolution = g.resolution;
  size = g.size;
  faces = g.faces;
  face_norm = g.face_norm;

  return *this;
}
*/

int MeshGraph::compare( const MeshGraph& g, const MeshGraph& h )
{
  return( g.id - h.id );
}

} // end of namespace PLearn
