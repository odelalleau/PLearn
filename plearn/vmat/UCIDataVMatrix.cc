// -*- C++ -*-

// UCIDataVMatrix.cc
//
// Copyright (C) 2005 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file UCIDataVMatrix.cc */


#include "UCIDataVMatrix.h"
#include <plearn/db/databases.h>

namespace PLearn {
using namespace std;


UCIDataVMatrix::UCIDataVMatrix()
{
}

PLEARN_IMPLEMENT_OBJECT(UCIDataVMatrix,
                        "Loads a UCI machine learning dataset.",
                        ""
    );

void UCIDataVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "uci_spec", &UCIDataVMatrix::uci_spec, OptionBase::buildoption,
                  "Specifications of a UCI dataset.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Hide unused options.

    redeclareOption(ol, "vm",          &UCIDataVMatrix::vm,          OptionBase::nosave, "Not used.");
    redeclareOption(ol, "writable",    &UCIDataVMatrix::writable,    OptionBase::nosave, "Not used.");
    redeclareOption(ol, "length",      &UCIDataVMatrix::length_,      OptionBase::nosave, "Not used.");
    redeclareOption(ol, "width",       &UCIDataVMatrix::width_,       OptionBase::nosave, "Not used.");
    redeclareOption(ol, "inputsize",   &UCIDataVMatrix::inputsize_,   OptionBase::nosave, "Not used.");
    redeclareOption(ol, "targetsize",  &UCIDataVMatrix::targetsize_,  OptionBase::nosave, "Not used.");
    redeclareOption(ol, "weightsize",  &UCIDataVMatrix::weightsize_,  OptionBase::nosave, "Not used.");
    redeclareOption(ol, "metadatadir", &UCIDataVMatrix::metadatadir, OptionBase::nosave, "Not used.");
}

void UCIDataVMatrix::build_()
{
    if (uci_spec)
        loadUCISet(vm, uci_spec);
    inherited::build();
}

void UCIDataVMatrix::build()
{
    inherited::build();
    build_();
    updateMtime(0);
//     updateMtime(uci_spec->data_all);
//     updateMtime(uci_spec->data_train);
//     updateMtime(uci_spec->data_test);
//     updateMtime(uci_spec->file_all);
//     updateMtime(uci_spec->file_train);
//     updateMtime(uci_spec->file_test);
}

void UCIDataVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("UCIDataVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
