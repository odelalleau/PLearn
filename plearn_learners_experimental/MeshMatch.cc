// -*- C++ -*-

// MeshMatch.cc
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
   * $Id: MeshMatch.cc,v 1.4 2005/01/21 03:58:01 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file MeshMatch.cc */

#include "MeshMatch.h"

namespace PLearn {
using namespace std;

MeshMatch::MeshMatch() :
  id( 0 ),
  model_id( 0 ),
//  correspondences(),
  rot( 3, 3 ),
  trans( 3 ),
  angles( 3 ),
  error( 0 )
{}

/*
MeshMatch::MeshMatch( const MeshMatch& m ) :
  id( m.id ),
  model_id( m.model_id ),
  correspondences(),
  rot( 3, 3 ),
  trans( 3 ),
  angles( 3 ),
  error( m.error )
{
  correspondences.resize( m.correspondences.size() );
  correspondences << m.correspondences;
  rot << m.rot;
  trans << m.trans;
  angles << m.angles;
}
*/


PLEARN_IMPLEMENT_OBJECT(MeshMatch,
    "Object describing the match and alignment between two Meshes",
    ""
);

void MeshMatch::declareOptions(OptionList& ol)
{
  declareOption(ol, "id", &MeshMatch::id, OptionBase::buildoption,
                "match identifier");

  declareOption(ol, "model_id", &MeshMatch::model_id, OptionBase::buildoption,
                "identifier of the model we try to match the mesh to");
/*
  declareOption(ol, "correspondences", &MeshMatch::correspondences, 
                OptionBase::buildoption,
                "list of point-to-point correspondences of the match");
*/
  declareOption(ol, "rot", &MeshMatch::rot, OptionBase::buildoption,
                "rotation matrix of the match"); // maybe learntoption ?

  declareOption(ol, "trans", &MeshMatch::trans, OptionBase::buildoption,
                "translation vector of the match");

  declareOption(ol, "angles", &MeshMatch::angles, OptionBase::buildoption,
                "angles of the rotations of the match");

  declareOption(ol, "error", &MeshMatch::error, OptionBase::buildoption,
                "alignment error between the match and the model");


  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void MeshMatch::build_()
{
}

// ### Nothing to add here, simply calls build_
void MeshMatch::build()
{
  inherited::build();
  build_();
}

void MeshMatch::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("MeshMatch::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/*
MeshMatch& MeshMatch::operator=( const MeshMatch& m )
{
  id = m.id;
  model_id = m.model_id;
  correspondences.resize( m.correspondences.size() );
  correspondences << m.correspondences;
  rot << m.rot;
  trans << m.trans;
  angles << m.angles;
  error = m.error;

  return *this;
}
*/

int MeshMatch::compare( const MeshMatch& p, const MeshMatch& q )
{
//  if( p.correspondences.size() < q.correspondences.size() ) return  1;
//  if( p.correspondences.size() > q.correspondences.size() ) return -1;
  if( p.error > q.error ) return  1;
  if( p.error < q.error ) return -1;
  return 0;
}


} // end of namespace PLearn
