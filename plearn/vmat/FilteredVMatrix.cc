// -*- C++ -*-

// FilteredVMatrix.cc
//
// Copyright (C) 2003 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file FilteredVMatrix.cc */

#include "FilteredVMatrix.h"
#include <plearn/base/ProgressBar.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    FilteredVMatrix,
    "A filtered view of its source VMatrix.",
    "The filter is an expression in VPL language.\n"
    "If a metadata directory is provided, the filtered indexes are saved in\n"
    "this directory. Otherwise the filtered indexes are re-computed at each\n"
    "call to build()."
);

/////////////////////
// FilteredVMatrix //
/////////////////////
FilteredVMatrix::FilteredVMatrix():
    build_complete(false),
    allow_repeat_rows(false),
    repeat_id_field_name(""),
    repeat_count_field_name(""),
    report_progress(true)
{}

FilteredVMatrix::FilteredVMatrix( VMat the_source, const string& program_string,
                                  const PPath& the_metadatadir, bool the_report_progress,
                                  bool allow_repeat_rows_, 
                                  const string& repeat_id_field_name_,
                                  const string& repeat_count_field_name_,
                                  bool call_build_):
    inherited(the_source, call_build_),
    allow_repeat_rows(allow_repeat_rows_),
    repeat_id_field_name(repeat_id_field_name_),
    repeat_count_field_name(repeat_count_field_name_),
    report_progress(the_report_progress),
    prg(program_string)
{
    // Note that although VMatrix::build_ would be tempted to call
    // setMetaDataDir when inherited(the_source, true) is called above (if
    // call_build_ is true), the metadatadir is only set below, so it will not
    // happen.
    metadatadir = the_metadatadir;

    if (call_build_)
        build_();
}

////////////////////////////
// computeFilteredIndices //
////////////////////////////
void FilteredVMatrix::computeFilteredIndices()
{
    int l = source.length();
    Vec result(1);
    PP<ProgressBar> pb;
    if (report_progress)
        pb = new ProgressBar("Filtering source vmat", l);
    mem_indices.resize(0);
    for(int i=0; i<l; i++)
    {
        if (report_progress)
            pb->update(i);
        program.run(i,result);
        if(!allow_repeat_rows)
        {
            if(!fast_exact_is_equal(result[0], 0))
                mem_indices.append(i);
        }
        else
            for(int x = int(round(result[0])); x > 0; --x)
                mem_indices.append(i);
    }
    length_ = mem_indices.length();
}

///////////////
// openIndex //
///////////////
void FilteredVMatrix::openIndex()
{
    PLASSERT(hasMetaDataDir());

    PPath idxfname = getMetaDataDir() / "filtered.idx";
    if(!force_mkdir(getMetaDataDir()))
        PLERROR("In FilteredVMatrix::openIndex - Could not create "
                "directory %s", getMetaDataDir().absolute().c_str());

    lockMetaDataDir();
    if(isUpToDate(idxfname))
        indexes.open(idxfname.absolute());
    else  // let's (re)create the index
    {
        computeFilteredIndices();
        rm(idxfname);       // force remove it
        indexes.open(idxfname.absolute(), true);
        for (int i = 0; i < mem_indices.length(); i++)
            indexes.append(mem_indices[i]);
        indexes.close();
        indexes.open(idxfname.absolute());
        mem_indices = TVec<int>();  // Free memory.
    }
    unlockMetaDataDir();

    length_ = indexes.length();
}

////////////////////
// setMetaDataDir //
////////////////////
void FilteredVMatrix::setMetaDataDir(const PPath& the_metadatadir)
{
    inherited::setMetaDataDir(the_metadatadir);
    if (build_complete) {
        // Only call openIndex() if the build has been completed,
        // otherwise the filtering program will not be ready yet.
        openIndex();
        // We call 'setMetaInfoFromSource' only after the index file has been
        // correctly read.
        setMetaInfoFromSource();
    }
}

///////////////
// getNewRow //
///////////////
void FilteredVMatrix::getNewRow(int i, const Vec& v) const
{
    if (mem_indices.isEmpty() && indexes.length() == -1)
        PLERROR("In FilteredVMatrix::getNewRow - Filtered indices are not\n"
                "set yet.");

    int j= source->width();
    int idx = mem_indices.isEmpty() ? indexes[i] : mem_indices[i];

    source->getRow(idx, v.subVec(0, j));

    if(!repeat_id_field_name.empty())
    {
        int k= 1;
        while(k <= i && (mem_indices.isEmpty()
                                ? indexes[i]==indexes[i-k]
                                : mem_indices[i] == mem_indices[i-k]))
            ++k;
        v[j++]= static_cast<real>(k-1);
    }

    if(!repeat_count_field_name.empty())
    {
        int k0= 1;
        while(k0 <= i && (mem_indices.isEmpty()
                                ? indexes[i]==indexes[i-k0]
                                : mem_indices[i] == mem_indices[i-k0]))
            ++k0;
        --k0;
        int k1= 1;
        while(k1+i < length() && (mem_indices.isEmpty()
                                        ? indexes[i]==indexes[i+k1]
                                        : mem_indices[i] == mem_indices[i+k1]))
            ++k1;
        v[j++]= static_cast<real>(k0+k1);
    }

}

////////////////////
// declareOptions //
////////////////////
void FilteredVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "prg", &FilteredVMatrix::prg, OptionBase::buildoption,
                  "The VPL code that should produce a single scalar, indicating whether \n"
                  "we should keep the line (if the produced scalar is non zero) or throw it away (if it's zero)");

    declareOption(ol, "report_progress", &FilteredVMatrix::report_progress, OptionBase::buildoption,
                  "Whether to report the filtering progress or not.");

    declareOption(ol, "allow_repeat_rows", &FilteredVMatrix::allow_repeat_rows, OptionBase::buildoption,
                  "When true, the result of the program indicates the number of times this row should be repeated.\n"
                  "Simple filtering when false.");

    declareOption(ol, "repeat_id_field_name", &FilteredVMatrix::repeat_id_field_name, OptionBase::buildoption,
                  "Field name for the repetition id (0, 1, ..., n-1).  No field is added if empty.");

    declareOption(ol, "repeat_count_field_name", &FilteredVMatrix::repeat_count_field_name, OptionBase::buildoption,
                  "Field name for the number of repetitions (n).  No field is added if empty.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    declareOption(ol, "length", &FilteredVMatrix::length_,
                  OptionBase::nosave,
                  "The number of example. Computed each time from source.");

    redeclareOption(ol, "inputsize", &FilteredVMatrix::inputsize_,
                    OptionBase::nosave,
                    "Taken from source in  FilteredVMatrix.");
    
    redeclareOption(ol, "targetsize", &FilteredVMatrix::targetsize_,
                    OptionBase::nosave,
                    "Taken from source in FilteredVMatrix.");
    
    redeclareOption(ol, "weightsize", &FilteredVMatrix::weightsize_,
                    OptionBase::nosave,
                    "Taken from source in FilteredVMatrix.");
    
    redeclareOption(ol, "extrasize", &FilteredVMatrix::extrasize_,
                    OptionBase::nosave,
                    "Taken from source in FilteredVMatrix.");

    redeclareOption(ol, "width", &FilteredVMatrix::width_,
                    OptionBase::nosave,
                    "Taken from source in FilteredVMatrix.");
}

////////////
// build_ //
////////////
void FilteredVMatrix::build_()
{
    if (source) {
        updateMtime(source);
        vector<string> fieldnames;
        program.setSource(source);
        program.compileString(prg,fieldnames);
        build_complete = true;
        if (hasMetaDataDir())
            // Calling setMetaDataDir() will compute indices and save them
            // in the give metadata directory.
            setMetaDataDir(getMetaDataDir());
        else {
            // Compute selected indices in memory only.
            computeFilteredIndices();
        }

        Array<VMField> finfos= source->getFieldInfos().copy();
            
        if(!repeat_id_field_name.empty())
        {
            finfos.append(VMField(repeat_id_field_name));
            if(0 < width_)
                ++width_;
        }

        if(!repeat_count_field_name.empty())
        {
            finfos.append(VMField(repeat_count_field_name));
            if(0 < width_)
                ++width_;
        }

        setFieldInfos(finfos);
        setMetaInfoFromSource();
    } else
        length_ = width_ = 0;
}

///////////
// build //
///////////
void FilteredVMatrix::build()
{
    build_complete = false;
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void FilteredVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(mem_indices, copies);
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
