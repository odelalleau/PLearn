// -*- C++ -*-

// ProcessingVMatrix.cc
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

/*! \file ProcessingVMatrix.cc */


#include "ProcessingVMatrix.h"

namespace PLearn {
using namespace std;


ProcessingVMatrix::ProcessingVMatrix()
    :inherited()
    /* ### Initialise all fields to their default value */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(
    ProcessingVMatrix,
    "VMatrix whose rows are processed using a VPL script",
    "See class VMatLanguage for help on VPL syntax."
    );

void ProcessingVMatrix::getNewRow(int i, const Vec& v) const
{
    program.run(i,v);
}

void ProcessingVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "prg", &ProcessingVMatrix::prg, OptionBase::buildoption,
                  "The VPL code to be applied to each row of the vmat");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ProcessingVMatrix::build_()
{
    // Do not do anything until we get the source VMat.
    if (!source)
        return;
    vector<string> fieldnames;
    program.setSource(source);
    program.compileString(prg,fieldnames);
    int nfields = (int)fieldnames.size();
    width_ = nfields;

    // Set field infos.
    fieldinfos.resize(nfields);
    for(int j=0; j<nfields; j++)
        fieldinfos[j] = VMField(fieldnames[j]);

    length_ = source->length();
    setMetaInfoFromSource();
    sourcevec.resize(source->width());
}

void ProcessingVMatrix::build()
{
    inherited::build();
    build_();
}

void ProcessingVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(sourcevec, copies);
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
