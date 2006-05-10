// -*- C++ -*-

// ShuffleColumnsVMatrix.cc
//
// Copyright (C) 2004 Olivier Delalleau
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
 * $Id$
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ShuffleColumnsVMatrix.cc */


#include "ShuffleColumnsVMatrix.h"
#include <plearn/math/random.h>   //!<  For the seed stuff.

namespace PLearn {
using namespace std;

///////////////////////////
// ShuffleColumnsVMatrix //
///////////////////////////
ShuffleColumnsVMatrix::ShuffleColumnsVMatrix()
    : only_shuffle_inputs(true),
      seed(0)
{}

PLEARN_IMPLEMENT_OBJECT(ShuffleColumnsVMatrix,
                        "Shuffle the columns of its source VMatrix.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void ShuffleColumnsVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "only_shuffle_inputs", &ShuffleColumnsVMatrix::only_shuffle_inputs, OptionBase::buildoption,
                  "Whether we should only shuffle the input part, or also shuffle the target and weight parts.");

    declareOption(ol, "seed", &ShuffleColumnsVMatrix::seed, OptionBase::buildoption,
                  "The random generator seed (-1 = initialized from clock, 0 = no initialization).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ShuffleColumnsVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ShuffleColumnsVMatrix::build_()
{
    if (source) {
        int n_shuffle = source->width();
        if (only_shuffle_inputs) {
            if (source->inputsize() < 0)
                PLERROR("In ShuffleColumnsVMatrix::build_ - Cannot find out the source inputsize");
            n_shuffle = source->inputsize();
        }
        indices = TVec<int>(0, n_shuffle - 1, 1);
        if (seed == -1)
            PLearn::seed();
        else if (seed > 0)
            manual_seed(seed);
        else if (seed != 0)
            PLERROR("In ShuffleColumnsVMatrix::build_ - The seed must be either -1 or >= 0");
        shuffleElements(indices);
        for (int i = 0; i < source->width() - n_shuffle; i++) {
            indices.append(i + n_shuffle);
        }
    }
    inherited::build();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ShuffleColumnsVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ShuffleColumnsVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
