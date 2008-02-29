// -*- C++ -*-

// CumVMatrix.cc
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

/*! \file CumVMatrix.cc */


#include "CumVMatrix.h"

namespace PLearn {
using namespace std;


CumVMatrix::CumVMatrix()
    :inherited(), average(false)
    /* ### Initialise all fields to their default value */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(CumVMatrix, "Add columns that a cumulated values of given columns",
                        "The user specifies one or more columns and for each such <column-name>\n"
                        "a cum-<column-name> column is created which will contain the sum from row 0\n"
                        "to the current row of <column-name>.\n");

void CumVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i,sourcerow);
    if (is_missing(accumulated_columns(i,0)))
    {
        int k=i-1;
        while (k>=0 && is_missing(accumulated_columns(k,0))) k--;
        if (k<0)
        {
            k=0;
            source->getRow(k,previous_sourcerow);
            for (int j=0;j<columns.length();j++)
                accumulated_columns(k,j) = previous_sourcerow[columns[j]];
        }
        for (;k<i;k++)
        {
            source->getRow(k+1,previous_sourcerow);
            for (int j=0;j<columns.length();j++)
                accumulated_columns(k+1,j) = accumulated_columns(k,j) + previous_sourcerow[columns[j]];
        }
    }
    if (average)
    {
        real normalization = 1.0 / (i+1);
        for (int j=0;j<columns_to_accumulate.length();j++)
            row[sourcerow.length()+j]=normalization*accumulated_columns(i,j);
    }
    else
    {
        for (int j=0;j<columns_to_accumulate.length();j++)
            row[sourcerow.length()+j]=accumulated_columns(i,j);
    }
    v << row;
}

void CumVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "columns_to_accumulate", &CumVMatrix::columns_to_accumulate, OptionBase::buildoption,
                  "Names of the columns to accumulate.");

    declareOption(ol, "average", &CumVMatrix::average, OptionBase::buildoption,
                  "whether to report the sum (default, when average=false) or the average");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void CumVMatrix::build_()
{
    int nc=columns_to_accumulate.length();
    if (source)
    {
        row.resize(source->width()+nc);
        sourcerow = row.subVec(0,source->width());
        previous_sourcerow.resize(source->width());
        columns.resize(nc);
        for (int j=0;j<nc;j++)
            if ((columns[j] = source->fieldIndex(columns_to_accumulate[j])) == -1)
                PLERROR("CumVMatrix: provided field name %s not found in source VMatrix",columns_to_accumulate[j].c_str());

        updateMtime(source);

        // copy length and width from source if not set
        if(length_<0)
            length_ = source->length();
        if(width_<0)
            width_ = source->width() + nc;

        accumulated_columns.resize(length_,nc);
        accumulated_columns.fill(MISSING_VALUE);

        // copy fieldnames from source if not set and they look good
        if(!hasFieldInfos() && source->hasFieldInfos() )
        {
            Array<VMField>& sinfo = source->getFieldInfos();
            int w=sinfo.size();
            sinfo.resize(w+nc);
            for (int j=0;j<nc;j++)
            {
                sinfo[w+j]=sinfo[columns[j]];
                sinfo[w+j].name = "cum-"+sinfo[w+j].name;
            }
            setFieldInfos(sinfo);
        }
    }
}

// ### Nothing to add here, simply calls build_
void CumVMatrix::build()
{
    inherited::build();
    build_();
}

void CumVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(columns_to_accumulate, copies);
    deepCopyField(accumulated_columns, copies);
    deepCopyField(columns, copies);

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
