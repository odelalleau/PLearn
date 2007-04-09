// -*- C++ -*-

// MeshVertex.cc
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
   * $Id: MeshVertex.cc,v 1.3 2005/12/14 20:41:11 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file MeshVertex.cc */


#include "MeshVertex.h"

namespace PLearn {
using namespace std;

MeshVertex::MeshVertex() :
  region( 0 ),
  bf( 0 ),
  coord( 3 ),
  norm( 3 ),
  error( 0 )
{}

/*
MeshVertex::MeshVertex( const MeshVertex& p ) :
  region( p.region ),
  bf( p.bf ),
  coord( p.coord ),
  norm( p.norm ),
  error( p.error )
{}
*/


PLEARN_IMPLEMENT_OBJECT(MeshVertex,
    "Point element of a SurfaceMesh",
    ""
);

void MeshVertex::declareOptions(OptionList& ol)
{
  declareOption(ol, "region", &MeshVertex::region, OptionBase::buildoption,
                "region the point belongs to");

  declareOption(ol, "bf", &MeshVertex::bf, OptionBase::buildoption,
                "boundary flag");

  declareOption(ol, "coord", &MeshVertex::coord, OptionBase::buildoption,
                "(3D) coordinates of the point");

  declareOption(ol, "norm", &MeshVertex::norm, OptionBase::buildoption,
                "(3D) coordinates of the normal vector at the point");

  declareOption(ol, "error", &MeshVertex::error, OptionBase::buildoption,
                "dunno, sure it has something to do with an error measure...");

  declareOption(ol, "features", &MeshVertex::features, OptionBase::buildoption,
                "Vector containing the (chemical) features of this vertex");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void MeshVertex::build_()
{}

void MeshVertex::build()
{
  inherited::build();
  build_();
}

void MeshVertex::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(coord, copies);
  deepCopyField(norm, copies);
  deepCopyField(features, copies);
}

/*
MeshVertex& MeshVertex::operator=( const MeshVertex& p )
{
  region = p.region;
  bf = p.bf;
  coord = p.coord;
  norm = p.norm;
  error = p.error;
  return *this;
}
*/

int MeshVertex::compare( const MeshVertex& p, const MeshVertex& q )
{
  int answer;

  if( p.error > q.error )
  {  answer = -1; }
  else if( p.error < q.error )
  { answer = 1; }
  else
  { answer = 0; }

  return answer;
}

real dist3D( const MeshVertex& p, const MeshVertex& q )
{
  return dist( p.coord, q.coord, 2 );
}

} // end of namespace PLearn
