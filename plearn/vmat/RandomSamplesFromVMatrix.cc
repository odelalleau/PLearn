// -*- C++ -*-

// RandomSamplesFromVMatrix.cc
//
// Copyright (C) 2006 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file RandomSamplesFromVMatrix.cc */


#include "RandomSamplesFromVMatrix.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    RandomSamplesFromVMatrix,
    "VMatrix that contains random samples from a VMatrix",
    "The samples are ALWAYS random, i.e. they are not pre-defined at building time.\n"
    "The only exception is when the same row is accessed many consecutive times,\n"
    "in which case the same example is returned. The default length of this VMatrix\n"
    "is the length of the source VMatrix. The user can either specify directly the\n"
    "derised length or specify what should be the ratio of this VMatrix length and \n"
    "the source VMatrix length."
    );

RandomSamplesFromVMatrix::RandomSamplesFromVMatrix():
    flength(-1),
    seed(0), 
    rgen(new PRandom())
{
}

void RandomSamplesFromVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(rgen->uniform_multinomial_sample(source->length()),v);
}

void RandomSamplesFromVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "source", &RandomSamplesFromVMatrix::source,
                  OptionBase::buildoption,
                  "VMatrix from which the samples are taken");
    
    declareOption(ol, "flength", &RandomSamplesFromVMatrix::flength,
                  OptionBase::buildoption,
                  "If provided, will overwrite length by flength * source->length()");
    declareOption(ol, "seed", &RandomSamplesFromVMatrix::seed,
                  OptionBase::buildoption,
                  "The initial seed for the random number generator used in this\n"
                  "VMatrix, for sample selection.\n"
                  "If -1 is provided, then a 'random' seed is chosen based on time\n"
                  "of day, ensuring that different experiments run differently.\n"
                  "If 0 is provided, no (re)initialization of the random number\n"
                  "generator is performed.\n"
                  "With a given positive seed, this VMatrix should always select\n"
                  "the same sequence of samples.");

    

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RandomSamplesFromVMatrix::build_()
{
    if(source)
    {
        if(flength > 0)
            length_ = int(flength * source->length());
        if(length_ < 0)
            length_ = source->length();
        if(seed != 0)
            rgen->manual_seed(seed);
        
        width_ = source->width();
        if(inputsize_ < 0) inputsize_ = source->inputsize();
        if(targetsize_ < 0) targetsize_ = source->targetsize();
        if(weightsize_ < 0) weightsize_ = source->weightsize();
        fieldinfos = source->fieldinfos;
    }
}

// ### Nothing to add here, simply calls build_
void RandomSamplesFromVMatrix::build()
{
    inherited::build();
    build_();
}

void RandomSamplesFromVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(source, copies);
    deepCopyField(rgen, copies);
}


PP<Dictionary> RandomSamplesFromVMatrix::getDictionary(int col) const
{
    return source->getDictionary(col);
}

Vec RandomSamplesFromVMatrix::getValues(int row, int col) const
{
    PLERROR("In RandomSamplesFromVMatrix::getValues(): Cannot give possible values given a row index because samples are independent of the row index.");
    return Vec(0);
}

Vec RandomSamplesFromVMatrix::getValues(const Vec& input, int col) const
{
    return source->getValues(input,col);
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
