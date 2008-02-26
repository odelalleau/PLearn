// -*- C++ -*-

// VariablesTest.cc
//
// Copyright (C) 2008 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file VariablesTest.cc */


#include "VariablesTest.h"
#include <plearn/math/PRandom.h>
#include <plearn/var/ArgminVariable.h>
#include <plearn/var/Func.h>
#include <plearn/var/LogAddVariable.h>
#include <plearn/var/UnfoldedFuncVariable.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    VariablesTest,
    "Tests various Variable objects.",
    "Feel free to add more Variable tests in there."
);

//////////////////
// VariablesTest //
//////////////////
VariablesTest::VariablesTest()
{
}

///////////
// build //
///////////
void VariablesTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VariablesTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("VariablesTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void VariablesTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "unfolded_argmin", &VariablesTest::unfolded_argmin,
                   OptionBase::buildoption,
        "Result of the unfolded argmin computation.");

    declareOption(ol, "logadd_binary", &VariablesTest::logadd_binary,
                   OptionBase::buildoption,
        "Result of the logadd on two variables.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void VariablesTest::build_()
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
}

/////////////
// perform //
/////////////
void VariablesTest::perform()
{
    // Declare and initialize variables used in tests.
    int is1 = 5;
    int n_input2 = 3;
    Var input1(1, is1, "input1");
    Var input2(n_input2, is1, "input2");
    PRandom::common(false)->fill_random_uniform(input2->matValue, -1, 1);
    Var input3(input2->length(), input2->width(), "input3");
    PRandom::common(false)->fill_random_uniform(input3->matValue, -1, 1);

    // Test UnfoldedFuncVariable to compute an argmin over a set of row vectors.
    Var argmin1 = argmin(input1);
    Func input1_to_argmin1(input1, argmin1);
    Var unfolded_argmin1 = new UnfoldedFuncVariable(input2, input1_to_argmin1,
                                                    false);
    unfolded_argmin1->fprop();
    unfolded_argmin = unfolded_argmin1->matValue.copy();
    
    // Test LogAddVariable to compute a logadd over two input matrices.
    Var logadd1 = logadd(input2, input3);
    logadd1->fprop();
    logadd_binary = logadd1->matValue.copy();
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
