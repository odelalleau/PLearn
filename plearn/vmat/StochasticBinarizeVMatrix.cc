// -*- C++ -*-

// StochasticBinarizeVMatrix.cc
//
// Copyright (C) 2008 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file StochasticBinarizeVMatrix.cc */


#include "StochasticBinarizeVMatrix.h"
#include <plearn/vmat/ShiftAndRescaleVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StochasticBinarizeVMatrix,
    "Transform its source data into stochastically sampled binary data.",
    "Each column of the source data is first rescaled into the [0, 1] range,\n"
    "then when accessing a sample, each variable is taken to be 1 with\n"
    "probability given by its real value (and 0 otherwise). Constant columns\n"
    "are given a uniform distribution in {0, 1}.\n"
    "Since sampling is performed every time a sample is accessed, one should\n"
    "precompute the data if a constant dataset is desired.\n"
    "In the current implementation, only the input part is binarized."
);

///////////////////////////////
// StochasticBinarizeVMatrix //
///////////////////////////////
StochasticBinarizeVMatrix::StochasticBinarizeVMatrix():
    rescale_to_0_1(true),
    seed(1827),
    random_gen(new PRandom())
{
}

////////////////////
// declareOptions //
////////////////////
void StochasticBinarizeVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "rescale_to_0_1",
                  &StochasticBinarizeVMatrix::rescale_to_0_1,
                  OptionBase::buildoption,
        "Whether to rescale to [0,1] before sampling. If set to False, then\n"
        "the data is assumed to already be in the [0,1] range (no check will\n"
        "be performed to enforce it, though).");

    declareOption(ol, "seed", &StochasticBinarizeVMatrix::seed,
                  OptionBase::buildoption,
        "Seed of random number generator.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void StochasticBinarizeVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void StochasticBinarizeVMatrix::build_()
{
    if (source) {
        if (rescale_to_0_1) {
            PP<ShiftAndRescaleVMatrix> rd =
                new ShiftAndRescaleVMatrix(source, false);
            rd->min_max = Vec(2);
            rd->min_max[1] = 1;
            rd->build();
            rescaled_data = get_pointer(rd);
        } else
            rescaled_data = source;
        setMetaInfoFromSource();
    }
}

///////////////
// getNewRow //
///////////////
void StochasticBinarizeVMatrix::getNewRow(int i, const Vec& v) const
{
    rescaled_data->getRow(i, v);
    for (int j = 0; j < source->inputsize(); j++)
        v[j] = random_gen->uniform_sample() <= v[j] ? 1 : 0;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void StochasticBinarizeVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("StochasticBinarizeVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
