// -*- C++ -*-

// MovingAverageVMatrix.cc
//
// Copyright (C) 2004 Yoshua Bengio
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

// Authors: Yoshua Bengio

/*! \file MovingAverageVMatrix.cc */


#include "MovingAverageVMatrix.h"
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;


MovingAverageVMatrix::MovingAverageVMatrix()
    :inherited(), centered_windows(true)
    /* ### Initialise all fields to their default value */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(MovingAverageVMatrix, "Perform moving average of given columns",
                        "The user specifies one or more columns and for each such <column-name>\n"
                        "a moving average window size: a ma<windowsize>-<column-name> column is\n"
                        "created which will contain at row t the moving average from row t-<windowsize>+1\n"
                        "to t inclusively of <column-name>.\n");

void MovingAverageVMatrix::getNewRow(int i, Vec& v) const
{
    source->getRow(i,sourcerow);
    int max_target = centered_windows?min(i+max_window_size/2,length()-1):i;
    if (is_missing(sums(max_target,0)))
    {
        int k=max_target-1;
        while (k>=0 && is_missing(sums(k,0))) k--;
        if (k<0)
        {
            k=0;
            source->getRow(k,previous_sourcerow);
            for (int j=0;j<columns.length();j++)
            {
                real new_value = previous_sourcerow[columns[j]];
                if (is_missing(new_value))
                {
                    sums(k,j) = 0;
                    ma(k,j) = MISSING_VALUE;
                }
                else
                {
                    sums(k,j) = new_value;
                    nnonmissing(k,j) = 1;
                    ma(k,j) = new_value;
                }
            }
        }
        for (;k<max_target;k++)
        {
            source->getRow(k+1,previous_sourcerow);
            for (int j=0;j<columns.length();j++)
            {
                real new_value = previous_sourcerow[columns[j]];
                if (!is_missing(new_value))
                {
                    sums(k+1,j) = sums(k,j) + new_value;
                    nnonmissing(k+1,j) = nnonmissing(k,j) + 1;
                }
                else sums(k+1,j) = sums(k,j);
                int delta = k+1-window_sizes[j];
                int n_at_window_start = (delta>=0)?nnonmissing(delta,j):0;
                int n = nnonmissing(k+1,j) - n_at_window_start;
                if (n>0)
                {
                    if (delta>=0)
                        ma(k+1,j) = (sums(k+1,j) - sums(delta,j))/n;
                    else
                        ma(k+1,j) = sums(k+1,j)/n;
                }
                else
                    ma(k+1,j) = MISSING_VALUE;
            }
        }
    }
    for (int j=0;j<columns_to_average.length();j++)
    {
        int target = centered_windows?min(i+window_sizes[j]/2,length()-1):i;
        row[sourcerow.length()+j]=ma(target,j);
    }
    v << row;
}

void MovingAverageVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "columns_to_average", &MovingAverageVMatrix::columns_to_average, OptionBase::buildoption,
                  "Names of the columns to average.");

    declareOption(ol, "window_sizes", &MovingAverageVMatrix::window_sizes, OptionBase::buildoption,
                  "Sizes (in number of rows) of the moving average windows for each column to average.");

    declareOption(ol, "centered_windows", &MovingAverageVMatrix::centered_windows, OptionBase::buildoption,
                  "Wether or not to center the window around the current example or to average only over previous examples");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void MovingAverageVMatrix::build_()
{
    int nc=columns_to_average.length();
    if (source)
    {
        row.resize(source->width()+nc);
        sourcerow = row.subVec(0,source->width());
        previous_sourcerow.resize(source->width());
        columns.resize(nc);
        max_window_size=0;
        if (window_sizes.length()!=nc)
            PLERROR("MovingAverageVMatrix: the window_sizes option should have the same length as the columns_to_average option (got %d and %d)",
                    window_sizes.length(),nc);
        for (int j=0;j<nc;j++)
        {
            if ((columns[j] = source->fieldIndex(columns_to_average[j])) == -1)
                PLERROR("MovingAverageVMatrix: provided field name %s not found in source VMatrix",columns_to_average[j].c_str());
            if (window_sizes[j]>max_window_size)
                max_window_size=window_sizes[j];
        }

        updateMtime(source);

        // copy length and width from source if not set
        if(length_<0)
            length_ = source->length();
        if(width_<0)
            width_ = source->width() + nc;

        sums.resize(length_,nc);
        sums.fill(MISSING_VALUE);
        nnonmissing.resize(length_,nc);
        nnonmissing.clear();
        ma.resize(length_,nc);
        ma.fill(MISSING_VALUE);

        // copy fieldnames from source if not set and they look good
        if(!hasFieldInfos() && source->hasFieldInfos() )
        {
            Array<VMField>& sinfo = source->getFieldInfos();
            int w=sinfo.size();
            sinfo.resize(w+nc);
            for (int j=0;j<nc;j++)
            {
                sinfo[w+j]=sinfo[columns[j]];
                sinfo[w+j].name = "ma"+tostring(window_sizes[j])+"-"+sinfo[w+j].name;
            }
            setFieldInfos(sinfo);
        }
    }
}

// ### Nothing to add here, simply calls build_
void MovingAverageVMatrix::build()
{
    inherited::build();
    build_();
}

void MovingAverageVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(columns, copies);
    deepCopyField(columns_to_average, copies);
    deepCopyField(sums, copies);
    deepCopyField(window_sizes, copies);

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
