// -*- C++ -*-

// SplitWiseValidationVMatrix.cc
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

/*! \file SplitWiseValidationVMatrix.cc */


#include "SplitWiseValidationVMatrix.h"
#include <plearn/db/getDataSet.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    SplitWiseValidationVMatrix,
    "VMatrix that takes several experiment split_stats.pmat to extract the split statistics and perform validation.",
    "It looks at the validation error, selects the best experiment\n"
    "and only report the associated statistics, for each split."
    );

SplitWiseValidationVMatrix::SplitWiseValidationVMatrix()
    : validation_error_column(-1), add_column_average(false)
{}

void SplitWiseValidationVMatrix::getNewRow(int i, const Vec& v) const
{
    v << data(i);
}

void SplitWiseValidationVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "split_stats_ppaths", &SplitWiseValidationVMatrix::split_stats_ppaths,
                   OptionBase::buildoption,
                   "Help text describing this option");
    declareOption(ol, "validation_error_column", &SplitWiseValidationVMatrix::validation_error_column,
                  OptionBase::buildoption,
                  "Index of the validation error statistic column");
    declareOption(ol, "nsplits", &SplitWiseValidationVMatrix::nsplits,
                  OptionBase::buildoption,
                  "Number of splits to consider, starting from the first one.\n"
                  "If < 0, then all splits are considered (based on the number\n"
                  "of splits of the first expdir).");
    declareOption(ol, "add_column_average", &SplitWiseValidationVMatrix::add_column_average,
                  OptionBase::buildoption,
                  "Indication that the average of the columns should be added.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SplitWiseValidationVMatrix::build_()
{
    if(validation_error_column < 0)
        PLERROR("In SplitWiseValidationVMatrix::build_(): validation_error_column cannot be < 0");
    if(split_stats_ppaths.length() == 0)
        PLERROR("In SplitWiseValidationVMatrix::build_(): split_stats_ppaths is empty");
    TVec<Mat> stats(split_stats_ppaths.length());
    Mat stats_i;
    for(int i=0; i<stats.length(); i++)
    {
        stats_i = getDataSet(split_stats_ppaths[i])->toMat();
        if(i==0)
        {
            if(nsplits<0) nsplits = stats_i.length();
            data.resize(nsplits,stats_i.width());
            data.fill(REAL_MAX);
        }
        if(stats_i.length() <nsplits)
        {
            PLWARNING("In SplitWiseValidationVMatrix::build_(): split_stats_ppaths[%d]=%s does not have enough splits, it will be ignored", i, split_stats_ppaths[i].c_str());
            continue;
        }
        for(int j=0; j<nsplits; j++)
        {
            if(data(j,validation_error_column) > stats_i(j,validation_error_column))
                data(j) << stats_i(j);
        }
    }
    
    length_ = data.length();
    width_ = data.width();
    if(add_column_average)
    {
        data.resize(data.length()+1, data.width());
        data(data.length()-1).fill(0);
        length_++;
        for(int i=0; i<data.width(); i++)
        {
            for(int j=0; j<data.length()-1; j++)
                data(data.length()-1,i) += data(j,i);
            data(data.length()-1,i) = data(data.length()-1,i)/(data.length()-1);
        }
    }
}

// ### Nothing to add here, simply calls build_
void SplitWiseValidationVMatrix::build()
{
    inherited::build();
    build_();
}

void SplitWiseValidationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(split_stats_ppaths, copies);
    deepCopyField(data, copies);

    //PLERROR("SplitWiseValidationVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
