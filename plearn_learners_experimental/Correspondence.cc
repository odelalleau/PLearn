// -*- C++ -*-

// Correspondence.cc
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
   * $Id: Correspondence.cc,v 1.2 2004/08/26 05:56:35 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file Correspondence.cc */


#include "Correspondence.h"

namespace PLearn {
using namespace std;

Correspondence::Correspondence() :
  sid( 0 ),
  mid( 0 ),
  model_id( 0 ),
  votes( 0 ),
  correlation( 0 )
{}

Correspondence::Correspondence( const Correspondence& c ) :
  sid( c.sid ),
  mid( c.mid ),
  model_id( c.model_id ),
  votes( c.votes ),
  correlation( c.correlation )
{}

PLEARN_IMPLEMENT_OBJECT(Correspondence,
    "Correspondence / Comparison between points on two meshes",
    ""
);

void Correspondence::declareOptions(OptionList& ol)
{
  declareOption(ol, "sid", &Correspondence::sid, OptionBase::buildoption,
                "dunno yet...");

  declareOption(ol, "mid", &Correspondence::mid, OptionBase::buildoption,
                "dunno yet...");

  declareOption(ol, "model_id", &Correspondence::model_id, 
                OptionBase::buildoption,
                "dunno, probably something to do with a model identifier...");

  declareOption(ol, "votes", &Correspondence::votes, OptionBase::buildoption,
                "dunno, probably something to do with a number of votes...");

  declareOption(ol, "correlation", &Correspondence::correlation, 
                OptionBase::buildoption,
                "dunno, probably the correlation value itself...");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void Correspondence::build_()
{}

void Correspondence::build()
{
  inherited::build();
  build_();
}

void Correspondence::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("Correspondence::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

Correspondence& Correspondence::operator=( const Correspondence& c )
{
  sid = c.sid;
  mid = c.mid;
  model_id = c.model_id;
  votes = c.votes;
  correlation = c.correlation;

  return *this;
}

int Correspondence::compare( const Correspondence& p, const Correspondence& q )
{
  if( p.model_id < q.model_id ) return -1;
  if( p.model_id > q.model_id ) return  1;
  if( p.sid < q.sid ) return -1;
  if( p.sid > q.sid ) return  1;
  if( p.mid < q.mid ) return -1;
  if( p.mid > q.mid ) return  1;

  return 0;
}


bool operator<( const Correspondence& p, const Correspondence& q )
{
  if( p.model_id < q.model_id ) return true;
  if( p.model_id > q.model_id ) return false;
  if( p.sid < q.sid ) return true;
  if( p.sid > q.sid ) return false;
  if( p.mid < q.mid ) return true;
  if( p.mid > q.mid ) return false;

  return false;
}

} // end of namespace PLearn
