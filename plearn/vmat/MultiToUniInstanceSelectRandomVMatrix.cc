// -*- C++ -*-

// MultiToUniInstanceSelectRandomVMatrix.cc
//
// Copyright (C) 2005 Benoit Cromp
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

// Authors: Benoit Cromp

/*! \file MultiToUniInstanceSelectRandomVMatrix.cc */


#include "MultiToUniInstanceSelectRandomVMatrix.h"
#include <plearn/math/random.h>
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(MultiToUniInstanceSelectRandomVMatrix,
                        "VMat class that selects randomly one row per bags from a multi instances conforming VMatrix and discard the multi instances bag information column. NOTE: doesn't support weight columns (will be discarded)",
                        ""
    );

MultiToUniInstanceSelectRandomVMatrix::MultiToUniInstanceSelectRandomVMatrix()
    : seed(0)
{
}

void MultiToUniInstanceSelectRandomVMatrix::declareOptions(OptionList& ol)
{

    declareOption(ol, "seed", &MultiToUniInstanceSelectRandomVMatrix::seed, OptionBase::buildoption, "Random generator seed (>0) (exceptions : -1 = initialized from clock, 0 = no initialization).");

    inherited::declareOptions(ol);

    // Redeclare some options.
    redeclareOption(ol, "indices", &MultiToUniInstanceSelectRandomVMatrix::indices, OptionBase::nosave, "");
    redeclareOption(ol, "indices_vmat", &MultiToUniInstanceSelectRandomVMatrix::indices_vmat, OptionBase::nosave, "");

    redeclareOption(ol, "source", &MultiToUniInstanceSelectRandomVMatrix::source_, OptionBase::buildoption, "Multi instances conforming source VMatrix");
}

///////////
// build //
///////////

void MultiToUniInstanceSelectRandomVMatrix::build()
{
    inherited::build();
    build_();
}

void MultiToUniInstanceSelectRandomVMatrix::build_()
{

    // Seeding the random number generator
    if (seed == -1)
        PLearn::seed();
    else if (seed > 0)
        PLearn::manual_seed(seed);
    else if (seed != 0)
        PLERROR("In MultiToUniInstanceSelectRandomVMatrix::build_ - The seed must be either -1 or >= 0");

    // Building the source VMatrix (uni instances conforming version of source_)
    source = new SubVMatrix(source_, 0, 0, source_->length(), source_->inputsize()+source_->targetsize() - 1);
    source->defineSizes(source_->inputsize(), source_->targetsize()-1, 0);

    width_ = source->inputsize() + source->targetsize() + source->weightsize();
    inputsize_ = source->inputsize();
    targetsize_ = source->targetsize();
    weightsize_ = source->weightsize();

    // Copy the appropriate fields informations
    if (source->getFieldInfos().size() > 0)
    {
 	fieldinfos.resize(width_);
        this->setFieldInfos(source->getFieldInfos());
    }
// Building the indices list that correspond to choosing randomly one instance per bag.
/* Notes for this task :
   The bag signal values meaning :
   1 means the first instance of the bag
   2 means the last instance of the bag
   3 is for a bag with a single row (= 1+2)
   0 is for intermediate instances.
*/

    int bag_signal_column = source_->inputsize() + source_->targetsize() - 1;
    int first_row = 0;

    indices.resize(0); // This get rid of the user's build option value.
    for(int row=0; row<source_->length(); row++)
    {
        switch(int(source_->get(row, bag_signal_column)))
        {
        case 1:
            first_row = row;
            break;
        case 2:
            indices.push_back(first_row+(int)(uniform_sample()*(row-first_row+1)));
            break;
        case 3:
            indices.push_back(row);
            break;
        };
    }
    inherited::build();

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
