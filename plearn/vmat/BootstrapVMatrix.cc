// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Olivier Delalleau
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
   * $Id: BootstrapVMatrix.cc,v 1.9 2004/07/21 16:30:55 chrish42 Exp $
   ******************************************************* */

#include "BootstrapVMatrix.h"
#include <plearn/math/random.h>
#include <plearn/math/TMat_sort.h>

namespace PLearn {
using namespace std;

/** BootstrapVMatrix **/

PLEARN_IMPLEMENT_OBJECT(BootstrapVMatrix,
    "A VMatrix that sees a bootstrap subset of its parent VMatrix.\n"
    "This is not a real bootstrap since a sample can only appear once."
    , 
    ""
);

//////////////////////
// BootstrapVMatrix //
//////////////////////
BootstrapVMatrix::BootstrapVMatrix()
  : frac(0.6667),
    shuffle(false)
{}

BootstrapVMatrix::BootstrapVMatrix(VMat m, real frac, bool shuffle)
{
  this->frac = frac;
  this->source = m;
  this->shuffle = shuffle;
  build();
}

////////////////////
// declareOptions //
////////////////////
void BootstrapVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "shuffle", &BootstrapVMatrix::shuffle, OptionBase::buildoption,
        "If set to 1, the indices will be shuffled instead of being sorted.");

    declareOption(ol, "frac", &BootstrapVMatrix::frac, OptionBase::buildoption,
        "The fraction of elements we keep (default = 0.6667).");

    inherited::declareOptions(ol);

    // Hide the 'indices' option, because it will be overridden at build time.
    redeclareOption(ol, "indices", &SelectRowsVMatrix::indices, OptionBase::nosave,"");
}

///////////
// build //
///////////
void BootstrapVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void BootstrapVMatrix::build_()
{
  if (source) {
    indices = TVec<int>(0, source.length()-1, 1); // Range-vector
    shuffleElements(indices);
    indices = indices.subVec(0,int(frac * source.length()));
    if (!shuffle) {
      sortElements(indices);
    }
    // Because we changed the indices, a rebuild may be needed.
    inherited::build();
  }
}

} // end of namespcae PLearn
