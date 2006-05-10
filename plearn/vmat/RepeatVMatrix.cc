// -*- C++ -*-

// RepeatVMatrix.cc
//
// Copyright (C) 2005 Hugo Larochelle
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file RepeatVMatrix.cc */


#include "RepeatVMatrix.h"

namespace PLearn {
using namespace std;

//////////////////
// RepeatVMatrix //
//////////////////
RepeatVMatrix::RepeatVMatrix()
  : repeat_n_times(1)
{}

PLEARN_IMPLEMENT_OBJECT(RepeatVMatrix,
    "Repeats the source VMatrix a certain number of times",
    "This VMatrix simply concatenates repeat_n_times-1 copies of \n"
    "source VMatrix to itself."
);

////////////////////
// declareOptions //
////////////////////
void RepeatVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "repeat_n_times", &RepeatVMatrix::repeat_n_times, OptionBase::buildoption,
                "Number of times to repeat the source VMatrix.\n");

  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void RepeatVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void RepeatVMatrix::build_()
{
  if(source)
  {
    setMetaInfoFromSource();
    length_ = source->length()*repeat_n_times;
    if(repeat_n_times < 1)
      PLERROR("In RepeatVMatrix::build_(): repeat_n_times cannot be < 1");
  }
}

///////////////
// getNewRow //
///////////////
void RepeatVMatrix::getNewRow(int i, const Vec& v) const
{
  source->getRow(i%source->length(),v);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RepeatVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  //PLERROR("RepeatVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

