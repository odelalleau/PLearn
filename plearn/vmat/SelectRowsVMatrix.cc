// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
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

#include "SelectRowsVMatrix.h"

namespace PLearn {
using namespace std;

/** SelectRowsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SelectRowsVMatrix,
    "Selects samples from its source according to given vector of indices.",
    ""
);

SelectRowsVMatrix::SelectRowsVMatrix()
    : obtained_inputsize_from_source(false),
      obtained_targetsize_from_source(false),
      obtained_weightsize_from_source(false),
      obtained_extrasize_from_source(false),
      warn_if_all_rows_selected(true),
      rows_to_remove(false)
{}

SelectRowsVMatrix::SelectRowsVMatrix(VMat the_source, TVec<int> the_indices, bool the_rows_to_remove, bool warn)
    : obtained_inputsize_from_source(false),
      obtained_targetsize_from_source(false),
      obtained_weightsize_from_source(false),
      obtained_extrasize_from_source(false),
      warn_if_all_rows_selected(warn),
      indices(the_indices),
      rows_to_remove(the_rows_to_remove)
{
    source = the_source;
    build_();
}

//! Here the indices will be copied locally into an integer vector
SelectRowsVMatrix::SelectRowsVMatrix(VMat the_source, Vec the_indices, bool the_rows_to_remove, bool warn)
    : obtained_inputsize_from_source(false),
      obtained_targetsize_from_source(false),
      obtained_weightsize_from_source(false),
      obtained_extrasize_from_source(false),
      warn_if_all_rows_selected(warn),
      rows_to_remove(the_rows_to_remove)
{
    source = the_source;
    indices.resize(the_indices.length());
    indices << the_indices; // copy to integer indices
    build_();
}

real SelectRowsVMatrix::get(int i, int j) const
{ return source->get(selected_indices[i], j); }

void SelectRowsVMatrix::getSubRow(int i, int j, Vec v) const
{ source->getSubRow(selected_indices[i], j, v); }

real SelectRowsVMatrix::dot(int i1, int i2, int inputsize) const
{ return source->dot(int(selected_indices[i1]), int(selected_indices[i2]), inputsize); }

real SelectRowsVMatrix::dot(int i, const Vec& v) const
{ return source->dot(selected_indices[i],v); }

string SelectRowsVMatrix::getString(int row, int col) const
{ return source->getString(selected_indices[row], col); }

const map<string,real>& SelectRowsVMatrix::getStringToRealMapping(int col) const
{ return source->getStringToRealMapping(col);}

const map<real,string>& SelectRowsVMatrix::getRealToStringMapping(int col) const
{ return source->getRealToStringMapping(col);}

void SelectRowsVMatrix::declareOptions(OptionList &ol)
{
    // Build options.

    declareOption(ol, "indices", &SelectRowsVMatrix::indices, OptionBase::buildoption,
                  "The array of row indices to extract");

    declareOption(ol, "indices_vmat", &SelectRowsVMatrix::indices_vmat, OptionBase::buildoption,
                  "If provided, will override the 'indices' option: the indices will be taken\n"
                  "from the first column of the given VMatrix (taking the closest integer).");

    declareOption(ol, "rows_to_remove", &SelectRowsVMatrix::rows_to_remove, OptionBase::buildoption,
                  "Indication that the rows specified in indices or indices_vmat\n"
                  "should be removed, not selected from the source VMatrix.");

    // Learnt options.

    declareOption(ol, "obtained_inputsize_from_source", &SelectRowsVMatrix::obtained_inputsize_from_source, OptionBase::learntoption,
                  "Set to 1 if the inputsize was obtained from the source VMat.");

    declareOption(ol, "obtained_targetsize_from_source", &SelectRowsVMatrix::obtained_targetsize_from_source, OptionBase::learntoption,
                  "Set to 1 if the targetsize was obtained from the source VMat.");

    declareOption(ol, "obtained_weightsize_from_source", &SelectRowsVMatrix::obtained_weightsize_from_source, OptionBase::learntoption,
                  "Set to 1 if the weightsize was obtained from the source VMat.");

    declareOption(ol, "obtained_extrasize_from_source", &SelectRowsVMatrix::obtained_extrasize_from_source, OptionBase::learntoption,
                  "Set to 1 if the extrasize was obtained from the source VMat.");

    declareOption(ol, "warn_if_all_rows_selected", &SelectRowsVMatrix::warn_if_all_rows_selected, OptionBase::buildoption,
                  "If true, we generate a warning if we select all row.");

    inherited::declareOptions(ol);

    // Hide unused options.
    // Note: it is not obvious that all the options below are useless, one may
    // want to un-hide some in the future. However, the more hidden, the
    // simpler for the beginner.

    redeclareOption(ol, "writable", &SelectRowsVMatrix::writable,
                    OptionBase::nosave, "Not used.");

    redeclareOption(ol, "length", &SelectRowsVMatrix::length_,
                    OptionBase::nosave, "Not used.");

    redeclareOption(ol, "width", &SelectRowsVMatrix::width_,
                    OptionBase::nosave, "Not used.");

}

void SelectRowsVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(selected_indices, copies);
    deepCopyField(indices,      copies);
    deepCopyField(indices_vmat, copies);
}

///////////
// build //
///////////
void SelectRowsVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void SelectRowsVMatrix::build_()
{
    if (indices_vmat) {
        int n = indices_vmat->length();
        indices.resize(n);
        for (int i = 0; i < n; i++)
            indices[i] = int(round(indices_vmat->get(i,0)));
    }

    if(rows_to_remove)
    {
        TVec<bool> tag(source->length());
        tag.fill(true);
        int count_to_remove = 0;
        for(int i=0; i<indices.length(); i++) {
            tag[indices[i]] = false;
            count_to_remove++;
        }
        // Allocate enough memory.
        selected_indices.resize(source->length() - count_to_remove);
        selected_indices.resize(0);
        for(int i=0; i<source->length(); i++)
            if(tag[i]) selected_indices.push_back(i);
        
    }
    else
    {
        selected_indices.resize(indices.length());
        selected_indices << indices;
    }
    //we don't display the warning for SortRowsVMatrix as it always select all row!
    if(warn_if_all_rows_selected && selected_indices.length()==source.length() && source.length()>0)
        PLWARNING("In SelectRowsVMatrix::build_() - We select all row!");

    length_ = selected_indices.length();
    if (source) {
        string error_msg =
            "In SelectRowsVMatrix::build_ - For safety reasons, it is forbidden to "
            "re-use sizes obtained from a previous source VMatrix with a new source "
            "VMatrix having different sizes";
        width_ = source->width();
        if(inputsize_<0) {
            inputsize_ = source->inputsize();
            obtained_inputsize_from_source = true;
        } else if (obtained_inputsize_from_source && inputsize_ != source->inputsize())
            PLERROR(error_msg.c_str());
        if(targetsize_<0) {
            targetsize_ = source->targetsize();
            obtained_targetsize_from_source = true;
        } else if (obtained_targetsize_from_source && targetsize_ != source->targetsize())
            PLERROR(error_msg.c_str());
        if(weightsize_<0) {
            weightsize_ = source->weightsize();
            obtained_weightsize_from_source = true;
        } else if (obtained_weightsize_from_source && weightsize_ != source->weightsize())
            PLERROR(error_msg.c_str());
        if(extrasize_ == 0) {
            extrasize_ = source->extrasize();
            obtained_extrasize_from_source = true;
        } else if (obtained_extrasize_from_source && extrasize_ != source->extrasize())
            PLERROR(error_msg.c_str());
        fieldinfos = source->fieldinfos;
    } else {
        // Restore the original undefined sizes if the current one had been obtained
        // from the source VMatrix.
        if (obtained_inputsize_from_source) {
            inputsize_ = -1;
            obtained_inputsize_from_source = false;
        }
        if (obtained_targetsize_from_source) {
            targetsize_ = -1;
            obtained_targetsize_from_source = false;
        }
        if (obtained_weightsize_from_source) {
            weightsize_ = -1;
            obtained_weightsize_from_source = false;
        }
    }
}

real SelectRowsVMatrix::getStringVal(int col, const string & str) const
{ return source->getStringVal(col, str); }

string SelectRowsVMatrix::getValString(int col, real val) const
{ return source->getValString(col,val); }

PP<Dictionary> SelectRowsVMatrix::getDictionary(int col) const
{
#ifdef BOUNDCHECK
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
#endif
    return source->getDictionary(col);
}


void SelectRowsVMatrix::getValues(int row, int col, Vec& values) const
{
#ifdef BOUNDCHECK
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
#endif
    source->getValues(selected_indices[row],col,values);
}

void SelectRowsVMatrix::getValues(const Vec& input, int col, Vec& values) const
{
#ifdef BOUNDCHECK
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
#endif
    source->getValues(input, col, values);
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
