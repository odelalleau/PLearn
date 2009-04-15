// -*- C++ -*-

// ValueSelectRowsVMatrix.cc
//
// Copyright (C) 2009 Frederic Bastien
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

// Authors: Frederic Bastien

/*! \file ValueSelectRowsVMatrix.cc */


#include "ValueSelectRowsVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ValueSelectRowsVMatrix,
    "Select rows based on an index column. The value should be in another VMatrix.",
    "MULTI-LINE \nHELP"
    );

//////////////////
// ValueSelectRowsVMatrix //
//////////////////
ValueSelectRowsVMatrix::ValueSelectRowsVMatrix()
{
}

////////////////////
// declareOptions //
////////////////////
void ValueSelectRowsVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "col_name", &ValueSelectRowsVMatrix::col_name,
                  OptionBase::buildoption,
                  "The name of the column to compare values.");

    declareOption(ol, "col_name_sec", &ValueSelectRowsVMatrix::col_name,
                  OptionBase::buildoption,
                  "The name of the column to compare values for the second matrix."
                  " If not provided, will use col_name.");

    declareOption(ol, "second", &ValueSelectRowsVMatrix::second,
                  OptionBase::buildoption,
                  "The matrix that have the value that we keep the columns.");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ValueSelectRowsVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
//    inherited::build();
    build_();
    inherited::build();//must recall as we changed selected_indices
}

////////////
// build_ //
////////////
void ValueSelectRowsVMatrix::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    // ### In a SourceVMatrix, you will typically end build_() with:
    // setMetaInfoFromSource();

    // ### You should keep the line 'updateMtime(0);' if you do not implement
    // ### the update of the mtime. Otherwise you can have an mtime != 0 that
    // ### is not valid.
    // ### Note that setMetaInfoFromSource() updates the mtime to the same as
    // ### the source, but this value will be erased with 'updateMtime(0)'.
    if(col_name_sec.empty())
        col_name_sec=col_name;
    Vec values_src(source->length()), values_sec(second->length());
    int idx = source->getFieldIndex(col_name,false);
    if(idx<0)
        PLERROR("In ValueSelectRowsVMatrix::build_ - the matrix source don't have the column %s",
                col_name.c_str());
    source->getColumn(idx,values_src);
    idx=second->getFieldIndex(col_name_sec);
    if(idx<0)
        PLERROR("In ValueSelectRowsVMatrix::build_ - the matrix second don't have the column %s",
                col_name.c_str());
    second->getColumn(idx,values_sec);

    //sort values_sec as it is shorter.
    sortElements(values_sec);
    for(int i=0;i<values_src.size();i++){
        real val = values_src[i];
        int idx=values_sec.findSorted(val);
        if(values_sec[idx]==val){
            indices.append(i);
        }
    }
    PLCHECK(indices.size()==values_sec.size());
    updateMtime(source);
    updateMtime(second);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ValueSelectRowsVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(second, copies);
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
