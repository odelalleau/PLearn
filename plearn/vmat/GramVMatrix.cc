// -*- C++ -*-

// GramVMatrix.cc
//
// Copyright (C) 2004 Olivier Delalleau
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

/*! \file GramVMatrix.cc */


#include "GramVMatrix.h"
#include <time.h>         //!< For clock().

namespace PLearn {
using namespace std;

//////////////////
// GramVMatrix //
//////////////////
GramVMatrix::GramVMatrix()
    : verbosity(1)
{
}

PLEARN_IMPLEMENT_OBJECT(GramVMatrix,
                        "Computes the Gram matrix of a given kernel.",
                        "Currently, this class inherits from a MemoryVMatrix, and the Gram matrix\n"
                        "is stored in memory.\n"
    );

////////////////////
// declareOptions //
////////////////////
void GramVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "kernel", &GramVMatrix::kernel, OptionBase::buildoption,
                  "The kernel whose Gram matrix we want to compute.");

    declareOption(ol, "verbosity", &GramVMatrix::verbosity, OptionBase::buildoption,
                  "The level of verbosity.");

    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Hide the 'data' and 'source' options of a MemoryVMatrix, that are
    // not used in this subclass.
    redeclareOption(ol, "data", &GramVMatrix::data, OptionBase::nosave,
                    "Not needed here.");

    redeclareOption(ol, "source", &GramVMatrix::source, OptionBase::nosave,
                    "Not needed here.");

}

///////////
// build //
///////////
void GramVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GramVMatrix::build_()
{
    if (kernel) {
        bool old_report_progress = kernel->report_progress != 0;
        if (verbosity < 1) {
            kernel->report_progress = false;
        }
        int n = kernel->getData()->length();
        data.resize(n, n);
        clock_t time_for_gram = clock();
        kernel->computeGramMatrix(data);
        time_for_gram = clock() - time_for_gram;
        real real_time_for_gram = real(time_for_gram) / real(CLOCKS_PER_SEC);
        if (verbosity >= 2) {
            cout << "Time to compute the Gram matrix: " << real_time_for_gram << endl;
        }
        kernel->report_progress = old_report_progress;
        length_ = width_ = n;
        inherited::build();
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GramVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("GramVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
