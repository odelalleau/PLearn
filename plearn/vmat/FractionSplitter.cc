// -*- C++ -*-

// FractionSplitter.cc
//
// Copyright (C) 2003  Pascal Vincent
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

/*! \file FractionSplitter.cc */
#include "FractionSplitter.h"

namespace PLearn {
using namespace std;

//////////////////////
// FractionSplitter //
//////////////////////
FractionSplitter::FractionSplitter():
    one_is_absolute(false),
    round_to_closest(0)
{}

PLEARN_IMPLEMENT_OBJECT(FractionSplitter,
                        "A Splitter that can extract several subparts of a dataset in each split.",
                        "Ranges of the dataset are specified explicitly as start:end positions,\n"
                        "that can be absolute or relative to the number of samples in the training set.");

////////////////////
// declareOptions //
////////////////////
void FractionSplitter::declareOptions(OptionList& ol)
{

    declareOption(ol, "round_to_closest", &FractionSplitter::round_to_closest, OptionBase::buildoption,
                  "If set to 1, then the integer value found when using fractions will\n"
                  "be the closest integer, instead of the integer part.");

    declareOption(ol, "splits", &FractionSplitter::splits, OptionBase::buildoption,
                  "A matrix of start:end pairs. Each row represents a split. \n"
                  "Each start:end element represents a range of samples in the dataset to be splitted. \n"
                  "start and end, which are positions in the datataset, can be specified as either \n"
                  "a fraction of the dataset length (if <=1), or an absolute number of elements (if >1).\n"
                  "The range includes all samples from start to end, but excluding the end sample \n"
                  "(so that, for ex., the same value can be used as the start of the next range \n"
                  "without having the two ranges ovelap). \n"
                  "The value 1 is a bit special as it always means \"until last element inclusive\".\n"
                  "Ex: 1 2 [ 0:0.80, 0.80:1 ]  yields a single split with the first part being the first 80% \n"
                  "of the data, and the second the next 20% \n");

    declareOption(ol, "one_is_absolute",
                  &FractionSplitter::one_is_absolute, OptionBase::buildoption,
        "If true, then 1 is always considered as an absolute index, not as a\n"
        "fraction giving the end of the dataset. This can be useful if you\n"
        "actually want a split with a single element in it.",
        OptionBase::advanced_level);

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void FractionSplitter::build_()
{
}

///////////
// build //
///////////
void FractionSplitter::build()
{
    inherited::build();
    build_();
}

void FractionSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(splits, copies);
}

int FractionSplitter::nsplits() const
{
    return splits.length();
}

int FractionSplitter::nSetsPerSplit() const
{
    return splits.width();
}


TVec<VMat> FractionSplitter::getSplit(int k)
{
    TVec< pair<real,real> > frac_k = splits(k);
    int n = frac_k.length();
    TVec<VMat> vms(n);
    int l = dataset.length();
    int start = 0;
    int end = 0;
    for(int i=0; i<n; i++)
    {
        real fstart = frac_k[i].first;
        real fend = frac_k[i].second;

        if(fstart>1 || (one_is_absolute && is_equal(fstart, 1))) // absolute position
            start = int(round(fstart));
        else {// relative position
            if (round_to_closest) {
                start = int(fstart*l + 0.5);
            } else {
                start = int(fstart*l);
            }
        }

        if(fend>1 || (one_is_absolute && is_equal(fend, 1))) // absolute end position
            end = int(round(fend));
        else if(is_equal(fend,1)) // until last element inclusive
            end = l;
        else {// relative end position
            if (round_to_closest) {
                end = int(fend*l + 0.5);
            } else {
                end = int(fend*l);
            }
        }

        vms[i] = dataset.subMatRows(start, end-start);
    }
    return vms;
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
