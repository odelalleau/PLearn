// -*- C++ -*-

// UniformizeVMatrix.cc
//
// Copyright (C) 2006 Olivier Delalleau
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

/*! \file UniformizeVMatrix.cc */


#include "UniformizeVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    UniformizeVMatrix,
    "Transforms its source VMatrix so that its features look uniform.",
    "This VMat transforms the features of its source that are obviously non-\n"
    "Gaussian, i.e. when the difference between the maximum and minimum\n"
    "value is too large compared to the standard deviation (the meaning of\n"
    "'too large' being controlled by the 'threshold_ratio' option).\n"
    "\n"
    "It uses an underlying UniformizeLearner to perform uniformization.\n"
);

///////////////////////
// UniformizeVMatrix //
///////////////////////
UniformizeVMatrix::UniformizeVMatrix():
    nquantiles(1000),
    threshold_ratio(10),
    uniformize_input(true),
    uniformize_target(false),
    uniformize_weight(false),
    uniformize_extra(false),
    uniformize_learner( new UniformizeLearner() ),
    uniformized_source( new PLearnerOutputVMatrix() )
{}

////////////////////
// declareOptions //
////////////////////
void UniformizeVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "threshold_ratio", &UniformizeVMatrix::threshold_ratio,
                                         OptionBase::buildoption,
        "A source's feature will be uniformized when the following holds:\n"
        "(max - min) / stddev > threshold_ratio.");

    declareOption(ol, "uniformize_input",
                  &UniformizeVMatrix::uniformize_input,
                  OptionBase::buildoption,
        "Whether or not to uniformize the input part.");

    declareOption(ol, "uniformize_target",
                  &UniformizeVMatrix::uniformize_target,
                  OptionBase::buildoption,
        "Whether or not to uniformize the target part.");

    declareOption(ol, "uniformize_weight",
                  &UniformizeVMatrix::uniformize_weight,
                  OptionBase::buildoption,
        "Whether or not to uniformize the weight part.");

    declareOption(ol, "uniformize_extra",
                  &UniformizeVMatrix::uniformize_extra,
                  OptionBase::buildoption,
        "Whether or not to uniformize the extra part.");

    declareOption(ol, "nquantiles",
                  &UniformizeVMatrix::nquantiles,
                  OptionBase::buildoption,
        "Number of intervals used to divide the sorted values.");

    declareOption(ol, "train_source", &UniformizeVMatrix::train_source,
                                      OptionBase::buildoption,
        "An optional VMat that will be used instead of 'source' to compute\n"
        "the transformation parameters from the distribution statistics.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void UniformizeVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void UniformizeVMatrix::build_()
{
    if (!source)
        return;

    if (train_source) {
        assert( train_source->width() == source->width() );
        assert( train_source->inputsize()  == source->inputsize() &&
                train_source->targetsize() == source->targetsize() &&
                train_source->weightsize() == source->weightsize() &&
                train_source->extrasize()  == source->extrasize() );
    }

    VMat the_source = train_source ? train_source : source;

    assert( the_source->inputsize() >= 0 && the_source->targetsize() >= 0 &&
            the_source->weightsize() >= 0 && the_source->extrasize() >= 0 );

    // Find which dimensions to uniformize.
    TVec<int> features_to_uniformize;
    int col = 0;
    if (uniformize_input)
        features_to_uniformize.append(
                TVec<int>(col, col + the_source->inputsize() - 1, 1));
    col += the_source->inputsize();
    if (uniformize_target)
        features_to_uniformize.append(
                TVec<int>(col, col + the_source->targetsize() - 1, 1));
    col += the_source->targetsize();
    if (uniformize_weight)
        features_to_uniformize.append(
                TVec<int>(col, col + the_source->weightsize() - 1, 1));
    col += the_source->weightsize();
    if (uniformize_extra)
        features_to_uniformize.append(
                TVec<int>(col, col + the_source->extrasize() - 1, 1));
    col += the_source->extrasize();

    // Build the UniformizeLearner and associated PLearnerOutputVMatrix.
    uniformize_learner->forget();
    uniformize_learner->which_fieldnums = features_to_uniformize;
    uniformize_learner->nquantiles = this->nquantiles;
    uniformize_learner->build();
    uniformize_learner->setTrainingSet(the_source);
    uniformize_learner->train();
    TVec< PP<PLearner> > learners;
    learners.append((UniformizeLearner*) uniformize_learner);
    uniformized_source->learners = learners;
    uniformized_source->source = this->source;
    uniformized_source->build();

    // Obtain meta information from source.
    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void UniformizeVMatrix::getNewRow(int i, const Vec& v) const
{
    assert( uniformize_learner->stage > 0 );
    uniformized_source->getRow(i, v);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void UniformizeVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("UniformizeVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
