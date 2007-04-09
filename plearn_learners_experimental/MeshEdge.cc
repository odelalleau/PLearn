// -*- C++ -*-

// MeshEdge.cc
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
   * $Id: MeshEdge.cc,v 1.3 2004/11/11 19:54:14 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file MeshEdge.cc */


#include "MeshEdge.h"

namespace PLearn {
using namespace std;

MeshEdge::MeshEdge() :
  error( 0 ),
  length( 0 ),
  face1( -1 ),
  face2( -1 ),
  bf( 0 )
{}

/*
MeshEdge::MeshEdge( const MeshEdge& e ) :
  error( e.error ),
  length( e.length ),
  face1( e.face1 ),
  face2( e.face2 ),
  bf( e.bf )
{}
*/

PLEARN_IMPLEMENT_OBJECT(MeshEdge,
    "Edge element of a SurfaceMesh",
    ""
);

void MeshEdge::declareOptions(OptionList& ol)
{
  declareOption(ol, "error", &MeshEdge::error, OptionBase::buildoption,
                "dunno, sure it has something to do with an error measure...");

  declareOption(ol, "length", &MeshEdge::length, OptionBase::buildoption,
                "edge length");

  declareOption(ol, "face1", &MeshEdge::face1, OptionBase::buildoption,
                "identifier of the first adjacent face");

  declareOption(ol, "face2", &MeshEdge::face2, OptionBase::buildoption,
                "identifier of the second adjacent face");

  declareOption(ol, "bf", &MeshEdge::bf, OptionBase::buildoption,
                "boundary flag");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void MeshEdge::build_()
{}

void MeshEdge::build()
{
  inherited::build();
  build_();
}

void MeshEdge::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

/*
MeshEdge& MeshEdge::operator=( const MeshEdge& e )
{
  error = e.error;
  length = e.length;
  face1 = e.face1;
  face2 = e.face2;
  bf = e.bf;
  return *this;
}
*/

} // end of namespace PLearn
