// -*- C++ -*-

// ExplicitSplitter.cc
//
// Copyright (C) 2002 Pascal Vincent
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

/*! \file ExplicitSplitter.cc */
#include "ExplicitSplitter.h"
#include <plearn/db/getDataSet.h>

namespace PLearn {
using namespace std;

ExplicitSplitter::ExplicitSplitter()
    :Splitter()
{}


PLEARN_IMPLEMENT_OBJECT(ExplicitSplitter, "ONE LINE DESCR",
                        "ExplicitSplitter allows you to define a 'splitter' by giving explicitly the datasets for each split\n"
                        "as a matrix VMatrices.\n"
                        "(This splitter in effect ignores the 'dataset' it is given with setDataSet) \n");

void ExplicitSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "splitsets", &ExplicitSplitter::splitsets, OptionBase::buildoption,
                  "This is a matrix of VMat giving explicitly the datasets for each split.");
    inherited::declareOptions(ol);
}

void ExplicitSplitter::build_()
{
}

// ### Nothing to add here, simply calls build_
void ExplicitSplitter::build()
{
    inherited::build();
    build_();
}

void ExplicitSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Splitter::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(splitsets, copies);
}

int ExplicitSplitter::nsplits() const
{
    return splitsets.length();
}

int ExplicitSplitter::nSetsPerSplit() const
{
    return splitsets.width();
}


TVec<VMat> ExplicitSplitter::getSplit(int k)
{
    return splitsets(k);
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
